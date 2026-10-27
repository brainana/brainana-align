// App-domain worker lifecycle. No renderer-provided executable names or paths.
import fs from 'node:fs/promises'
import path from 'node:path'
import os from 'node:os'
import {spawn} from 'node:child_process'
import {createHash} from 'node:crypto'
import {constants} from 'node:fs'
const MAX=2_000_000
export function scientificEnvironment(){
 const allowed=['SystemRoot','WINDIR','TMPDIR','TEMP','TMP','HOME','USER','LANG','LC_ALL','LC_CTYPE','MallocNanoZone','CUDA_VISIBLE_DEVICES','__CF_USER_TEXT_ENCODING']
 const env=Object.fromEntries(allowed.filter(key=>process.env[key]!==undefined).map(key=>[key,process.env[key]]))
 env.PATH=process.platform==='win32'?path.join(env.SystemRoot??'C:\\Windows','System32'):'/usr/bin:/bin'
 return env
}

export function niftiBuffer(raw){
 if(!raw||!Array.isArray(raw.dims)||raw.dims.length!==3||raw.dims.some(v=>!Number.isInteger(v)||v<8)||raw.dims.reduce((a,b)=>a*b,1)>MAX)throw Error('Choose working images with at least 8 voxels per axis and at most 2 million voxels each using Resolution.')
 const n=raw.dims.reduce((a,b)=>a*b,1),values=raw.values
 if(!(values instanceof Float32Array)||values.length!==n)throw Error('Invalid working-image buffer')
 const a=raw.affine
 if(!Array.isArray(a)||a.length!==4||a.some(r=>!Array.isArray(r)||r.length!==4||!r.every(Number.isFinite))||a[3].some((v,i)=>v!==(i===3?1:0)))throw Error('Invalid physical image geometry')
 const det=a[0][0]*(a[1][1]*a[2][2]-a[1][2]*a[2][1])-a[0][1]*(a[1][0]*a[2][2]-a[1][2]*a[2][0])+a[0][2]*(a[1][0]*a[2][1]-a[1][1]*a[2][0]);if(Math.abs(det)<1e-10)throw Error('Singular physical image geometry')
 const b=Buffer.alloc(352+4*n);b.writeInt32LE(348);b.writeInt16LE(3,40);raw.dims.forEach((d,i)=>b.writeInt16LE(d,42+2*i));b.writeInt16LE(16,70);b.writeInt16LE(32,72);b.writeFloatLE(1,76);for(let j=0;j<3;j++)b.writeFloatLE(Math.hypot(a[0][j],a[1][j],a[2][j]),80+4*j);b.writeFloatLE(352,108);b.writeFloatLE(1,112);b[123]=2;b.writeInt16LE(1,254);for(let i=0;i<3;i++)for(let j=0;j<4;j++)b.writeFloatLE(a[i][j],280+16*i+4*j);b.write('n+1\0',344,'ascii');for(let i=0;i<n;i++)b.writeFloatLE(values[i],352+4*i);return b
}
// Separate inclusion/exclusion data: never modify source image intensities.
export function samplingInputs(spec,fixed,moving){
 if(spec===undefined)return null
 if(!spec||spec.version!==1)throw Error('Invalid registration sampling format')
 const payload={version:1},files={}
 for(const [side,image] of [['fixed',fixed],['moving',moving]]){
  const windows=spec[side+'Windows']??[]
  if(!Array.isArray(windows)||windows.length>3)throw Error('Invalid optimization windows')
  payload[side+'_windows']=windows.map(w=>{
   const axes=w?.axes??[0,1,2]
   if(!Array.isArray(axes)||!axes.length||new Set(axes).size!==axes.length||axes.some(a=>!Number.isInteger(a)||a<0||a>2)||!Array.isArray(w?.min)||!Array.isArray(w?.max)||w.min.length!==3||w.max.length!==3||![...w.min,...w.max].every(Number.isFinite)||axes.some(a=>w.min[a]>w.max[a]))throw Error('Invalid physical optimization window')
   return {min:[...w.min],max:[...w.max],axes:[...axes]}
  })
  const excluded=spec[side+'Excluded']
  if(excluded!==undefined){
   if(!(excluded instanceof Uint8Array)||excluded.length!==image.dims.reduce((a,b)=>a*b,1)||excluded.some(v=>v>1))throw Error('Invalid registration exclusion mask')
   files[side+'_excluded']=Buffer.from(excluded)
  }
 }
 return {payload,files}
}
async function smallJSON(file,limit){const stat=await fs.stat(file);if(stat.size>limit)throw Error('Scientific worker output exceeded the safety limit');return JSON.parse(await fs.readFile(file,'utf8'))}
// A worker can return only this bounded asset, inside our private job directory.
async function hydrateField(result,folder){
 const field=result?.deformation,asset=field?.residualAsset;if(!asset)return result
 const n=field.dims?.reduce?.((a,b)=>a*b,1)
 if(asset.version!==1||asset.file!=='residual.f32'||asset.encoding!=='float32-le-planar'||!Array.isArray(field.dims)||field.dims.length!==3||!field.dims.every(v=>Number.isInteger(v)&&v>=8)||!Number.isSafeInteger(n)||n>MAX||asset.bytes!==12*n||!/^[a-f0-9]{64}$/.test(asset.sha256)||field.residualBase64!==undefined)throw Error('Invalid scientific field asset')
 const file=await fs.open(path.join(folder,'residual.f32'),constants.O_RDONLY|(constants.O_NOFOLLOW??0))
 try{
  const stat=await file.stat();if(!stat.isFile()||stat.size!==asset.bytes)throw Error('Scientific field size does not match its manifest')
  const bytes=await file.readFile();if(bytes.length!==asset.bytes||createHash('sha256').update(bytes).digest('hex')!==asset.sha256)throw Error('Scientific field checksum failed')
  // All supported desktop architectures are little-endian. Do not retain paths
  // or expose buffers beyond the exact field range through the renderer bridge.
  const exact=new Uint8Array(bytes.length);exact.set(bytes);field.residualValues=new Float32Array(exact.buffer)
  for(const v of field.residualValues)if(!Number.isFinite(v))throw Error('Nonfinite scientific displacement field')
  delete field.residualAsset;return result
 }finally{await file.close()}
}
export function createJipService({runtime,temporary=os.tmpdir(),spawnWorker=spawn}){
 let job=null,latest=null,disposed=false,reserved=false,pendingCancellation=false
 const running=()=>Boolean(job)||reserved
 async function run(request){
  if(disposed||job||reserved)throw Error('A scientific operation is already running or the workspace is closing')
  if(!request||!['register','n4','itk-register'].includes(request.action))throw Error('Unknown scientific operation')
  if(['register','itk-register'].includes(request.action)&&(request.stages!=='nonlinear'||!Array.isArray(request.start)||request.start.length!==4||request.start.some(row=>!Array.isArray(row)||row.length!==4||!row.every(Number.isFinite))))throw Error('Nonlinear registration in Align adds deformation to an accepted linear alignment. Run and accept linear alignment first.')
  reserved=true;pendingCancellation=false
  try {
  const command=await runtime();if(disposed||pendingCancellation)throw Error('JIP operation cancelled; workspace unchanged');if(!command)throw Error('The registration backend included in Align is unavailable. Reinstall the complete Align app.')
  const registration=request.action==='register'||request.action==='itk-register'
  if(request.action==='itk-register'&&!command.features?.includes('itk-v1'))throw Error('The included backend does not provide ITK; alignment unchanged.')
  if(!request.settings||typeof request.settings!=='object'||Array.isArray(request.settings)||JSON.stringify(request.settings).length>65536)throw Error('Invalid or oversized scientific settings')
  // Validate and encode before creating a job; source buffers remain untouched.
  const inputs=registration?{fixed:niftiBuffer(request.fixed),moving:niftiBuffer(request.moving)}:{image:niftiBuffer(request.image)}
  const sampling=registration?samplingInputs(request.sampling,request.fixed,request.moving):null
  if(sampling&&!command.features?.includes('sampling-v1'))throw Error('The included backend lacks sampling-v1 support. Current regions are preserved.')
  if(request.action==='itk-register'&&sampling?.payload.moving_windows?.length)throw Error('ITK does not support continuous moving-image windows. Clear them explicitly or choose JIP; current regions are preserved.')
  const folder=await fs.mkdtemp(path.join(temporary,'brainana-jip-'));await fs.chmod(folder,0o700)
  const own={folder,child:null,cancelled:false,settled:false,eventOffset:0,eventPending:'',polling:false};job=own;latest={status:'running',text:'Preparing JIP working images…'}
  try{
   const payload={action:request.action,settings:request.settings,stages:request.stages,start:request.start,field_transport:'binary-v1'}
   for(const [key,bytes] of Object.entries(inputs)){payload[key]=path.join(folder,key+'.nii');await fs.writeFile(payload[key],bytes)}
   if(sampling){payload.sampling=sampling.payload;for(const [key,bytes] of Object.entries(sampling.files)){payload.sampling[key]=path.join(folder,key+'.u8');await fs.writeFile(payload.sampling[key],bytes)}}
   await fs.writeFile(path.join(folder,'request.json'),JSON.stringify(payload))
   if(own.cancelled||pendingCancellation)throw Error('JIP operation cancelled')
   const child=spawnWorker(command.file,[...(command.args??[]),'--align-request',path.join(folder,'request.json'),'--align-output',path.join(folder,'result.json'),'--events',path.join(folder,'events.jsonl'),'--cancel',path.join(folder,'cancel')],{stdio:['ignore','ignore','pipe'],windowsHide:true,env:scientificEnvironment()});own.child=child
   let tail='';child.stderr?.on('data',chunk=>{tail=(tail+chunk.toString()).slice(-8192)})
   const code=await new Promise((resolve,reject)=>{child.once('error',reject);child.once('close',resolve)})
   if(own.cancelled||pendingCancellation)throw Error('JIP operation cancelled; previous alignment preserved')
   const result=await smallJSON(path.join(folder,'result.json'),64*1024*1024).catch(error=>{throw Error('JIP worker could not return a valid result: '+(tail||error.message))})
   if(code!==0||result.status!=='ready')throw Error(result.message||'JIP worker failed')
   if(sampling&&result.result?.settings?.sampling?.version!==1)throw Error('JIP did not confirm the requested sampling regions; proposal discarded')
   if(request.action==='itk-register'&&(result.result?.settings?.engine!=='itk'||result.result?.execution?.engine!=='itk'))throw Error('Scientific worker did not confirm ITK execution; result discarded')
   if(request.action==='itk-register'&&(!result.result?.deformation||JSON.stringify(result.result.matrix)!==JSON.stringify(request.start)||JSON.stringify(result.result.deformation.baseAffine)!==JSON.stringify(request.start)))throw Error('ITK must return a nonlinear field with the accepted linear matrix unchanged; result discarded');
   const value=await hydrateField(result.result,folder);if(own.cancelled||pendingCancellation)throw Error('JIP operation cancelled; previous alignment preserved');latest={status:'complete',text:'JIP proposal ready'};return value
  }finally{
   own.settled=true;clearTimeout(own.timer);if(job===own)job=null
   await fs.rm(folder,{recursive:true,force:true});if(latest?.status==='running')latest={status:'idle',text:'JIP worker finished'}
  }
  }finally{reserved=false}
 }
 async function cancel(){pendingCancellation=true;const own=job;if(!own)return;own.cancelled=true;await fs.writeFile(path.join(own.folder,'cancel'),'').catch(()=>{});own.timer??=setTimeout(()=>{if(!own.settled)own.child?.kill('SIGKILL')},2000)}
 async function poll(){
  const own=job;if(own&&!own.polling){own.polling=true
   try{
    const file=await fs.open(path.join(own.folder,'events.jsonl'),'r')
    try{
     const stat=await file.stat();if(stat.size>4*1024*1024){await cancel();throw Error('Scientific progress output exceeded its safety limit')}
     if(stat.size<own.eventOffset){own.eventOffset=0;own.eventPending=''}
     const length=stat.size-own.eventOffset
     if(length){const bytes=Buffer.alloc(length),read=await file.read(bytes,0,length,own.eventOffset);own.eventOffset+=read.bytesRead
      const text=own.eventPending+bytes.subarray(0,read.bytesRead).toString('utf8'),lines=text.split('\n');own.eventPending=lines.pop()??''
      for(const line of lines){if(!line.trim())continue;try{const event=JSON.parse(line);if(job===own&&!own.settled)latest={status:'running',text:String(event.message??event.stage??'JIP registration in progress…').slice(0,500)}}catch{}}
     }
    }finally{await file.close()}
   }catch{}finally{own.polling=false}
  }
  return latest??{status:'idle'}
 }
 return {run,poll,cancel,running,async dispose(){disposed=true;const child=job?.child;await cancel();child?.kill('SIGTERM')}}
}
