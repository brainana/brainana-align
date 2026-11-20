import {NVImage} from '@niivue/niivue'
import {materializeFile} from '../../../packages/core-client/rangeFile'
import {runWorkerTask} from '../../../packages/core-client/workerTask'
import {inspectVolume,workingGrid,type VolumeInfo} from '../../../packages/imaging-math/workingVolume'
import {geometryDifference,imageSignature,type ImageSignature,type SessionPayload} from './sessionPersistence'
import {geometryMatches} from './imageProcessing'
import {reopenSessionImages,locateSessionImage,sessionCancelled} from './sessionFileAccess'
import {prepareInWorker} from './volumeTasks'
import {imageFromRaw} from './volumeImages'
import {rawFromNVImage} from './imageGeometry'
import {REGISTRATION_PAIRINGS} from './registrationPairing'
import type {Loaded,Modality} from './appTypes'
import type {Dims3,RawNifti} from './roiWarp'

const modalities:Modality[]=['mri','ct']
function check(signal:AbortSignal){if(signal.aborted)throw sessionCancelled()}
function desiredGrid(payload:SessionPayload,m:Modality,info:Pick<VolumeInfo,'dims'|'spacing'|'affine'>){
 const signature=payload.images[m]!,base=payload.processing?.[m]?.base,spacing=base?.pixDims??signature.requestedSpacing??signature.workingSpacing??signature.pixDims,dims=base?.dims??signature.workingDims
 const grid=workingGrid(info,spacing,dims)
 if(base&&!geometryMatches(base,{dims:grid.dims,affine:grid.affine,pixDims:grid.spacing}))throw Error('Saved preprocessing grid does not match the original image geometry')
 return {grid,spacing,dims}
}
export function sessionImageReady(payload:SessionPayload,m:Modality,item:Loaded|null):boolean{
 const saved=payload.images[m],current=imageSignature(item)
 if(!saved||!current||!item||geometryDifference(saved,current).length||saved.sourceFiles.join('\0')!==item.sourceFiles.join('\0'))return false
 try{const target=desiredGrid(payload,m,{dims:current.dims,spacing:current.pixDims,affine:current.affine}),base=item.processing?.base??item.raw;return geometryMatches(base,{dims:target.grid.dims,affine:target.grid.affine,pixDims:target.grid.spacing})}catch{return false}
}
type Source={files:File[];info:VolumeInfo;raw?:RawNifti;streamable:boolean}
async function inspectSource(files:File[],signature:ImageSignature):Promise<Source>{
 if(!files.length)throw Error('Select the original image')
 if(files.length!==signature.sourceFiles.length)throw Error(`Select ${signature.sourceFiles.length} original file(s) for ${signature.name}; this selection contains ${files.length}.`)
 if(files.length===1&&/\.nii(\.gz)?$/i.test(files[0].name)){
  try{const info=await inspectVolume(files[0]);info.residentFileBytes=0;return {files,info,streamable:true}}
  catch(error){if(!String(error).includes('Unsupported scalar NIfTI datatype'))throw error}
 }
 if((signature.selectedFrame??0)!==0)throw Error('This format cannot restore a selected 3D frame')
 const decoded=await Promise.all(files.map(materializeFile)),image=await NVImage.loadFromFile({file:decoded.length===1?decoded[0]:decoded,name:files[0].name}),raw=rawFromNVImage(image)
 return {files,raw,streamable:false,info:{name:files[0].name,fileBytes:files.reduce((n,f)=>n+f.size,0),dims:raw.dims,spacing:raw.pixDims,affine:raw.affine,frames:1,bytesPerVoxel:4,datatype:16,littleEndian:true,offset:0,slope:1,intercept:0,header:new Uint8Array(0)}}
}
/** Stage the entire pair and preprocessing without mutating the current workspace.
 * The composition root publishes these immutable results only after overlay success. */
export async function prepareSessionWorkspace(file:File,payload:SessionPayload,current:Record<Modality,Loaded|null>,signal:AbortSignal,progress:(text:string)=>void):Promise<Record<Modality,Loaded>>{
 const labels=REGISTRATION_PAIRINGS[payload.registrationPairing],label=(m:Modality)=>m==='mri'?labels.first:labels.second
 if(!payload.images.mri||!payload.images.ct)throw Error('This session has no complete image references')
 const wanted=modalities.filter(m=>!sessionImageReady(payload,m,current[m])),found=await reopenSessionImages(file,wanted,signal),sources:Partial<Record<Modality,Source>>={}
 for(const m of wanted){
  check(signal);const saved=payload.images[m]!;let files=found[m]??await locateSessionImage(saved,label(m),signal)
  while(true){
   check(signal);progress(`Checking ${label(m)} original image…`)
   try{
    const source=await inspectSource(files,saved);check(signal)
    const issues=geometryDifference(saved,{...saved,dims:source.info.dims,pixDims:source.info.spacing,affine:source.info.affine})
    if((saved.selectedFrame??0)>=source.info.frames)issues.push('the selected 3D frame is unavailable')
    if(issues.length)throw Error(`This ${label(m)} does not match the session: ${issues.join('; ')}`)
    sources[m]=source;break
   }catch(error){check(signal);files=await locateSessionImage(saved,label(m),signal,error instanceof Error?error.message:String(error))}
  }
  desiredGrid(payload,m,sources[m]!.info)
 }
 const items={} as Record<Modality,Loaded>
 for(const m of modalities){
  check(signal);const saved=payload.images[m]!,source=sources[m];let item:Loaded
  if(source){
   const target=desiredGrid(payload,m,source.info),frame=saved.selectedFrame??0
   const raw=source.streamable?await prepareInWorker(source.files[0],source.info,target.spacing,frame,n=>progress(`Restoring ${label(m)} working image ${(n*100).toFixed(0)}%…`),signal,target.dims):source.raw!
   if(!geometryMatches(raw,{dims:target.grid.dims,affine:target.grid.affine,pixDims:target.grid.spacing}))throw Error(`${label(m)} working grid could not be restored exactly`)
   item={name:source.files[0].name,sourceFiles:source.files.map(f=>f.name),raw,nvImage:imageFromRaw(raw,source.files[0].name),native:{files:source.files,info:source.info,frame,requested:[...target.spacing],streamable:source.streamable}}
  }else{const prior=current[m]!,raw=prior.processing?.base??prior.raw;item={...prior,raw,processing:undefined,nvImage:prior.processing?imageFromRaw(raw,prior.name):prior.nvImage}}
  item.acquisition=saved.acquisition?structuredClone(saved.acquisition):undefined
  const recipe=payload.processing?.[m]
  if(recipe){
   progress(`Restoring ${label(m)} masks, crop and voxel edits…`)
   if(!geometryMatches(item.raw,recipe.base))throw Error(`${label(m)} preprocessing requires its saved working grid`)
   const result=await runWorkerTask<{raw:RawNifti;excluded:Uint8Array;focus:Dims3;binarySelection?:Uint8Array}>({create:()=>new Worker(new URL('./processingWorker.ts',import.meta.url),{type:'module'}),request:{base:item.raw,recipe},signal,cancelled:'Session opening cancelled. Current workspace preserved.',failed:'Saved image processing could not be restored',result:data=>data})
   item={...item,raw:result.raw,nvImage:imageFromRaw(result.raw,item.name),processing:{base:item.raw,recipe:structuredClone(recipe),excluded:result.excluded,focus:result.focus,binarySelection:result.binarySelection}}
  }
  items[m]=item
 }
 check(signal);return items
}
