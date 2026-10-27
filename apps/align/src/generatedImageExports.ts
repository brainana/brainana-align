import {imageLabel,imageKey,getRegistrationPairing} from './registrationPairing'
import type {Loaded,Modality} from './appTypes'
import {emptyProcessingRecipe} from './imageProcessing'
import {saveArtifacts} from './exportArtifacts'
export function generatedImageChoices(loaded:Partial<Record<Modality,Loaded|null>>){return (['mri','ct'] as const).flatMap(m=>{const item=loaded[m];if(!item)return [];const r=item.processing?.recipe;return [...(r?.binaryMask||r?.thresholdFill?[{value:`${m}:mask`,label:`${imageLabel(m)} · Binary mask (1 selected, 0 excluded)`}]:[]),...(r&&(r.thresholdFill||r.crop||r.edits.some(e=>e.value!==undefined))?[{value:`${m}:masked`,label:`${imageLabel(m)} · Processed working image (mask/crop/edits)`}]:[])]})}

export function installGeneratedImageExports(o:{getLoaded:()=>Partial<Record<Modality,Loaded|null>>;activity:{busy:boolean;begin:(s:string,kind?:import('./workflowUI').ActivityKind)=>Promise<boolean>;end:()=>void};run:(fn:()=>Promise<void>)=>Promise<void>;report:(s:string,e?:boolean)=>void}){
 const select=document.querySelector<HTMLSelectElement>('#generated-image-choice')!,button=document.querySelector<HTMLButtonElement>('#save-generated-image')!
 function refresh(){const previous=select.value,choices=generatedImageChoices(o.getLoaded()),key=JSON.stringify(choices);if(select.dataset.choices!==key){select.replaceChildren(...choices.map(c=>new window.Option(c.label,c.value)));select.dataset.choices=key;if(choices.some(c=>c.value===previous))select.value=previous}button.disabled=o.activity.busy||!choices.length;select.disabled=o.activity.busy||!choices.length}
 async function save(){const [m,kind]=select.value.split(':') as [Modality,string],item=o.getLoaded()[m];if(!item)throw Error('This image is no longer available.');if(!generatedImageChoices(o.getLoaded()).some(c=>c.value===select.value))throw Error('This generated image is no longer available.')
  if(!await o.activity.begin('Exporting selected source or generated image…','export'))return
  refresh();const cancel=document.querySelector<HTMLButtonElement>('#alignment-cancel')!;cancel.classList.add('hidden')
  try{
   const recipe=item.processing?.recipe,threshold=recipe?.binaryMask??recipe?.thresholdFill
   cancel.classList.remove('hidden')
   const blob=await new Promise<Blob>((resolve,reject)=>{const worker=new Worker(new URL('./processingExportWorker.ts',import.meta.url),{type:'module'}),finish=()=>{worker.terminate();cancel.removeEventListener('click',stop)},stop=()=>{finish();reject(Error('Image export cancelled.'))};cancel.addEventListener('click',stop,{once:true});worker.onerror=e=>{finish();reject(Error(e.message))};worker.onmessage=({data})=>{finish();data.type==='error'?reject(Error(data.message)):resolve(kind==='mask'?data.mask:data.image)};try{worker.postMessage(kind==='mask'?{base:item.processing?.base??item.raw,recipe:recipe??emptyProcessingRecipe(item.raw),threshold}:{raw:item.raw,imageOnly:true})}catch(error){finish();reject(error)}})
   cancel.classList.add('hidden')
   const name=kind==='mask'?'binary-mask':'processed-working';const raw=item.raw
   await saveArtifacts([{filename:`${imageKey(m)}_${name}.nii.gz`,blob},{filename:`${imageKey(m)}_${name}.json`,blob:new Blob([JSON.stringify({source:item.name,kind,imageLabel:imageLabel(m),registrationPairing:getRegistrationPairing(),coordinateSystem:'NIfTI RAS physical millimeters',geometry:{dims:raw.dims,pixDims:raw.pixDims,affine:raw.affine},maskConvention:kind==='mask'?'1 selected, 0 excluded':undefined,threshold:kind==='mask'?threshold:undefined,recipe:recipe,displayAdjustmentsApplied:false},null,2)],{type:'application/json'})}]);o.report(`${kind==='mask'?'Binary mask':kind==='masked'?'Processed working image':'Working image'} saved without display adjustments.`)
  }finally{cancel.classList.add('hidden');o.activity.end();refresh()}
 }
 button.addEventListener('click',()=>{void o.run(save).catch(()=>{}).finally(refresh)});refresh();return {refresh}
}
