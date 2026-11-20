import {diskFile,diskFilePath,type DiskFileDescriptor} from '../../../packages/core-client/rangeFile'
import {getSessionToken} from './runtimeClient'
import type {ImageSignature} from './sessionPersistence'
import type {Modality} from './appTypes'

type NativeSessionFiles={path:(file:File)=>string|null;open:(file:File,wanted:Modality[])=>Promise<Partial<Record<Modality,DiskFileDescriptor[]>>|{unavailable:true}|null>;choose:(signature:ImageSignature,label:string,problem?:string)=>Promise<DiskFileDescriptor[]|{unavailable:true}|null>}
declare global{interface Window{brainanaSessionFiles?:NativeSessionFiles}}
export const sessionCancelled=()=>new DOMException('Session opening cancelled. Current workspace preserved.','AbortError')
export function sourceFilePath(file:File):string|null{
 const disk=diskFilePath(file);if(disk)return disk
 try{return typeof window!=='undefined'?window.brainanaSessionFiles?.path(file)??null:null}catch{return null}
}
const convert=(files:DiskFileDescriptor[])=>files.map(file=>diskFile(file,window.location.origin,getSessionToken()))

/** Browser edition and synthetic recovery snapshots have no native path grant.
 * Their explicit picker preserves the same restoration transaction. */
function browserPick(signature:ImageSignature,label:string,signal:AbortSignal,problem=''):Promise<File[]>{
 return new Promise((resolve,reject)=>{
  const modal=document.createElement('div');modal.className='modal';modal.id='session-locate-modal';modal.setAttribute('role','dialog');modal.setAttribute('aria-modal','true');modal.setAttribute('aria-labelledby','session-locate-title')
  modal.innerHTML='<div class="modal-card"><div class="modal-head"><h2 id="session-locate-title"></h2><button id="session-locate-close" aria-label="Cancel opening session">×</button></div><p id="session-locate-message"></p><p id="session-locate-problem" class="error" role="alert"></p><label>Original image files<input id="session-locate-input" type="file" multiple></label><p class="hint">Select the original input, including any companion files. The saved working resolution and processing will be restored automatically.</p><button id="session-locate-cancel">Cancel opening session</button></div>'
  modal.querySelector('#session-locate-title')!.textContent=`Locate ${label} for this session`
  modal.querySelector('#session-locate-message')!.textContent=`Original image: ${signature.name}. Select its current location${signature.sourceLocations?.some(Boolean)?' if it has moved':'; this older session has no saved file location'}.`
  modal.querySelector('#session-locate-problem')!.textContent=problem
  const input=modal.querySelector<HTMLInputElement>('#session-locate-input')!;input.multiple=signature.sourceFiles.length>1;input.accept='.nii,.nii.gz,.hdr,.img,.img.gz,.head,.brik,.brik.gz,.mgh,.mgz,.nrrd,.nhdr,.mif,.mha,.mhd,.raw,.v,.v16,.vmr,.npy,.npz,.fib,.src'
  const cleanup=()=>{signal.removeEventListener('abort',cancel);modal.remove()};const cancel=()=>{cleanup();reject(sessionCancelled())}
  for(const id of ['session-locate-close','session-locate-cancel'])modal.querySelector('#'+id)!.addEventListener('click',cancel)
  modal.addEventListener('keydown',event=>{if(event.key==='Escape'){event.preventDefault();cancel()}})
  input.addEventListener('change',()=>{const files=Array.from(input.files??[]);if(files.length){cleanup();resolve(files)}})
  document.body.append(modal);input.focus();signal.addEventListener('abort',cancel,{once:true});if(signal.aborted)cancel()
 })
}
export async function locateSessionImage(signature:ImageSignature,label:string,signal:AbortSignal,problem=''):Promise<File[]>{
 if(signal.aborted)throw sessionCancelled()
 const bridge=window.brainanaSessionFiles
 if(bridge){const result=await bridge.choose(signature,label,problem);if(result===null)throw sessionCancelled();if(!('unavailable' in result)){if(signal.aborted)throw sessionCancelled();return convert(result)}}
 return browserPick(signature,label,signal,problem)
}
export async function reopenSessionImages(file:File,wanted:Modality[],signal:AbortSignal):Promise<Partial<Record<Modality,File[]>>>{
 if(!wanted.length||!window.brainanaSessionFiles)return {}
 let result:Awaited<ReturnType<NativeSessionFiles['open']>>
 try{result=await window.brainanaSessionFiles.open(file,wanted)}catch{if(signal.aborted)throw sessionCancelled();return {}}
 if(result===null||signal.aborted)throw sessionCancelled()
 if('unavailable' in result)return {}
 return Object.fromEntries(Object.entries(result).map(([m,files])=>[m,convert(files)]))
}
