import assert from 'node:assert/strict'
import fs from 'node:fs/promises'
import os from 'node:os'
import path from 'node:path'
import {EventEmitter} from 'node:events'
import {createHash} from 'node:crypto'
import {createJipService} from '../apps/align/desktop/jipService.mjs'
import {createPointMapping,validateSpline} from '../packages/imaging-math/nonlinearSpline'
import {applyMat4,type Vec3} from '../packages/imaging-math/rigid'

const temporary=await fs.mkdtemp(path.join(os.tmpdir(),'align-itk-service-')),identity=[[1,0,0,0],[0,1,0,0],[0,0,1,0],[0,0,0,1]],raw={dims:[8,9,10],affine:[[.8,-.6,0,7],[.6,.8,0,-4],[0,0,1.2,3],[0,0,0,1]],values:Float32Array.from({length:720},(_,i)=>2+Math.sin(i*.1))}
const original=raw.values.slice(),excluded=new Uint8Array(720);excluded[5]=1
const windows=[{min:[-100,-100,10000],max:[100,100,10001],axes:[0,1]}]
const settings={engine:'itk',model:6,metric:'mattes-mi',iterations:1,nonlinear_iterations:1,threads:1,shrink_factors:[1],smoothing_sigmas_mm:[0],sampling_fraction:1,bspline_spacing_mm:20}
const request={action:'itk-register',fixed:raw,moving:raw,start:identity,stages:'nonlinear',settings}
let features=['itk-v1','sampling-v1'],mode='field',spawned=0,folder='',child:any,lastPayload:any,runtimePause:Promise<void>|null=null
const displacement:Vec3=[.3,-.2,.1],fieldValues=new Float32Array(3*720);for(let a=0;a<3;a++)fieldValues.fill(displacement[a],a*720,(a+1)*720)
const fieldBytes=Buffer.from(fieldValues.buffer),hash=createHash('sha256').update(fieldBytes).digest('hex')
const service=createJipService({temporary,runtime:async()=>{if(runtimePause)await runtimePause;return {file:'trusted-python',args:['trusted-launch.py'],features}},spawnWorker:(file:string,args:string[],options:unknown)=>{
 assert.equal(file,'trusted-python');assert.equal(args[0],'trusted-launch.py');assert.deepEqual({stdio:options.stdio,windowsHide:options.windowsHide},{stdio:['ignore','ignore','pipe'],windowsHide:true});assert(!Object.hasOwn(options.env,'JIP_ENGINE_LIBRARY'));assert(!Object.hasOwn(options.env,'PYTHONPATH'));spawned++
 child=new EventEmitter();child.stderr=new EventEmitter();child.kill=()=>{queueMicrotask(()=>child.emit('close',130));return true}
 folder=path.dirname(args[args.indexOf('--align-output')+1]);const ownChild=child,ownFolder=folder,ownMode=mode
 void(async()=>{
  const payload=JSON.parse(await fs.readFile(path.join(ownFolder,'request.json'),'utf8'));lastPayload=payload
  assert.equal(payload.action,'itk-register');assert.equal(payload.field_transport,'binary-v1');assert.deepEqual(payload.settings,settings)
  assert.deepEqual(payload.start,identity);assert.equal(payload.stages,'nonlinear');assert.equal(payload.command,undefined);assert.equal(payload.executable,undefined)
  for(const name of ['fixed','moving']){assert.equal(payload[name],path.join(ownFolder,name+'.nii'));assert.deepEqual((await fs.readFile(payload[name])).subarray(352),Buffer.from(raw.values.buffer))}
  if(payload.sampling){assert.deepEqual(payload.sampling.fixed_windows,windows);assert.deepEqual(payload.sampling.moving_windows,[]);assert.deepEqual(new Uint8Array(await fs.readFile(payload.sampling.fixed_excluded)),excluded)}
  if(ownMode==='hold'){await fs.writeFile(path.join(ownFolder,'events.jsonl'),JSON.stringify({message:'ITK controlled progress'})+'\n');return}
  const result:any={kind:'registration',matrix:identity,deformation:null,settings:{engine:ownMode==='settings-engine'?'jip':'itk',...(payload.sampling?{sampling:{version:1}}:{})},execution:{engine:ownMode==='execution-engine'?'jip':'itk',active:'cpu'},diagnostics:{warnings:[]}}
  if(ownMode==='no-engine')delete result.settings.engine
  if(ownMode==='sampling-unconfirmed')delete result.settings.sampling
  if(['field','field-path','field-checksum','field-nonfinite','field-changed-matrix','field-changed-base'].includes(ownMode)){
   const bytes=Buffer.from(fieldBytes);if(ownMode==='field-nonfinite')bytes.writeFloatLE(NaN,4)
   await fs.writeFile(path.join(ownFolder,'residual.f32'),bytes)
   result.deformation={version:1,kernel:'jip-dense-pull',baseAffine:identity,dims:raw.dims,affine:raw.affine,settings:{engine:'itk'},execution:{engine:'itk',active:'cpu'},warnings:[],residualAsset:{version:1,file:ownMode==='field-path'?'../escape.f32':'residual.f32',encoding:'float32-le-planar',bytes:bytes.length,sha256:ownMode==='field-checksum'?'0'.repeat(64):ownMode==='field-nonfinite'?createHash('sha256').update(bytes).digest('hex'):hash}}
  }
  if(ownMode==='field-changed-matrix')result.matrix=[[1,0,0,1],[0,1,0,0],[0,0,1,0],[0,0,0,1]];if(ownMode==='field-changed-base')result.deformation.baseAffine=[[1,0,0,1],[0,1,0,0],[0,0,1,0],[0,0,0,1]];
  await fs.writeFile(path.join(ownFolder,'result.json'),JSON.stringify({status:'ready',result}));ownChild.emit('close',0)
 })().catch(error=>ownChild.emit('error',error));return child
}})
const clean=async()=>{assert.equal(service.running(),false);assert.deepEqual(await fs.readdir(temporary),[]);assert.deepEqual(raw.values,original);assert.equal(excluded[5],1)}
async function until(test:()=>Promise<boolean>|boolean){for(let i=0;i<200;i++){if(await test())return;await new Promise(r=>setTimeout(r,5))}throw Error('Service did not reach expected lifecycle state')}
try{
 for(const stages of ['affine','both',undefined])await assert.rejects(()=>service.run({...request,stages}),/accepted linear alignment/);
 await assert.rejects(()=>service.run({...request,start:undefined}),/accepted linear alignment/);assert.equal(spawned,0);
 features=['sampling-v1'];await assert.rejects(()=>service.run(request),/does not provide ITK/);assert.equal(spawned,0);await clean();features=['itk-v1','sampling-v1']
 for(const invalid of [undefined,null,[],42,'x',{oversized:'x'.repeat(65536)}]){await assert.rejects(()=>service.run({...request,settings:invalid}),/Invalid or oversized scientific settings/);assert.equal(spawned,0);await clean()}
 const movingWindows={version:1,fixedWindows:[],movingWindows:windows}
 await assert.rejects(()=>service.run({...request,sampling:movingWindows}),/moving-image windows/);assert.equal(spawned,0);await clean()
 features=['itk-v1'];await assert.rejects(()=>service.run({...request,sampling:{version:1,fixedWindows:windows,movingWindows:[]}}),/sampling-v1/);assert.equal(spawned,0);await clean();features=['itk-v1','sampling-v1']
 mode='linear';await assert.rejects(()=>service.run(request),/must return a nonlinear field/);await clean();mode='field';const field=await service.run({...request,command:'untrusted',executable:'/untrusted'});assert.equal(field.settings.engine,'itk');assert.equal(field.execution.engine,'itk');assert(field.deformation);await clean()
 const sampling={version:1,fixedWindows:windows,movingWindows:[],fixedExcluded:excluded}
 mode='field';const mapped=await service.run({...request,sampling});assert(mapped.deformation.residualValues instanceof Float32Array);assert.equal(mapped.deformation.residualValues.length,2160);assert.equal(mapped.deformation.residualAsset,undefined);assert.equal(mapped.deformation.residualBase64,undefined);assert.equal(mapped.settings.sampling.version,1);assert.deepEqual(lastPayload.sampling.fixed_windows,windows)
 validateSpline(mapped.deformation)
 const map=createPointMapping(mapped.matrix,mapped.deformation),point=applyMat4(raw.affine,[3,3,3]),forward=map.forward(point),inverse=map.inverse(point)
 for(let a=0;a<3;a++){assert(Math.abs(forward[a]-(point[a]-displacement[a]))<1e-6);assert(Math.abs(inverse[a]-(point[a]+displacement[a]))<1e-6)}
 assert(Math.hypot(...forward.map((v,a)=>v-point[a]))>.1,'Hydrated full map must differ from its affine alone');await clean()
 for(mode of ['field-changed-matrix','field-changed-base']){await assert.rejects(()=>service.run(request),/accepted linear matrix unchanged/);await clean()}
 for(mode of ['settings-engine','execution-engine','no-engine']){await assert.rejects(()=>service.run(request),/did not confirm ITK/);await clean()}
 mode='sampling-unconfirmed';await assert.rejects(()=>service.run({...request,sampling}),/did not confirm.*sampling/);await clean()
 for(mode of ['field-path','field-checksum','field-nonfinite']){await assert.rejects(()=>service.run(request),/asset|checksum|Nonfinite/);await clean()}
 let resumeRuntime!:()=>void;runtimePause=new Promise(resolve=>{resumeRuntime=resolve});const reserved=service.run(request),reservedCheck=assert.rejects(reserved,/cancelled/);assert(service.running());await assert.rejects(()=>service.run(request),/already running/);await service.cancel();resumeRuntime();await reservedCheck;runtimePause=null;await clean()
 mode='hold';folder='';const pending=service.run(request),cancelled=assert.rejects(pending,/cancelled/);await until(()=>!!folder&&fs.stat(path.join(folder,'events.jsonl')).then(()=>true).catch(()=>false));assert(service.running());assert.equal((await service.poll()).text,'ITK controlled progress');await assert.rejects(()=>service.run(request),/already running/);await service.cancel();assert(await fs.stat(path.join(folder,'cancel')));child.emit('close',0);await cancelled;await clean()
 // Numeric/pyramid policy belongs to the optional companion. When the real
 // itk-v1 source is supplied, also exercise its refusals through this service.
 if(process.env.JIP_TEST_SOURCE){
  const real=createJipService({temporary,runtime:async()=>({file:process.env.JIP_TEST_PYTHON??'/opt/anaconda3/bin/python',args:[path.resolve(process.env.JIP_TEST_SOURCE!,'frontend/launch.py')],features:['itk-v1','sampling-v1']})})
  try{
   for(const [changed,pattern] of [[{threads:0},/threads/],[{maximum_step_mm:11},/maximum_step_mm/]] as const){await assert.rejects(()=>real.run({...request,settings:{...settings,...changed}}),pattern);assert.deepEqual(await fs.readdir(temporary),[])}
   await assert.rejects(()=>real.run({...request,sampling,settings:{...settings,shrink_factors:[2,1],smoothing_sigmas_mm:[1,0]}}),/dense native-grid/);assert.deepEqual(await fs.readdir(temporary),[])
  }finally{await real.dispose()}
 }
 folder='';const disposing=service.run(request),disposedCheck=assert.rejects(disposing,/cancelled/);await until(()=>!!folder&&fs.stat(path.join(folder,'events.jsonl')).then(()=>true).catch(()=>false));await service.dispose();await disposedCheck;await clean();await assert.rejects(()=>service.run(request),/workspace is closing/)
 console.log('ITK desktop service: trusted dispatch, missing capability, bounded structural settings, physical-window refusal/transport, engine confirmation, nonzero full-map hydration, malformed assets, reservation/in-flight cancellation, disposal and private-file cleanup passed.'+(process.env.JIP_TEST_SOURCE?' Actual companion numeric bounds and fixed-window dense-only refusal passed.':''))
}finally{await service.dispose();await fs.rm(temporary,{recursive:true,force:true})}
