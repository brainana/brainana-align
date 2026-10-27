import type {Modality} from './appTypes'
export const REGISTRATION_PAIRINGS={
 'mri-ct':{title:'MRI–CT registration',first:'MRI',second:'CT'},
 't1-t2':{title:'T1–T2 registration',first:'T1',second:'T2'},
 't1-t1':{title:'T1–T1 registration',first:'T1 · 1',second:'T1 · 2'},
 'mri-epi':{title:'MRI–EPI registration',first:'MRI',second:'EPI'},
} as const
export type RegistrationPairing=keyof typeof REGISTRATION_PAIRINGS
let pairing:RegistrationPairing='mri-ct'
export const validPairing=(v:unknown):v is RegistrationPairing=>typeof v==='string'&&Object.hasOwn(REGISTRATION_PAIRINGS,v)
export const getRegistrationPairing=()=>pairing
export function setRegistrationPairing(value:RegistrationPairing){if(!validPairing(value))throw Error('Unknown registration pairing');pairing=value}
export const imageLabel=(m:Modality,p:RegistrationPairing=pairing)=>REGISTRATION_PAIRINGS[p][m==='mri'?'first':'second']
export const imageKey=(m:Modality)=>imageLabel(m).toLowerCase().replace(' · ','-')
/** Only static instructions are formatted; never run this over filenames or user data. */
export const imagingText=(text:string)=>text.replace(/\bMRI\b/g,imageLabel('mri')).replace(/\bCT\b/g,imageLabel('ct'))
const staticCopies=new WeakMap<Document,{node:Text;source:string}[]>()
export function applyImageLabels(){
 let copies=staticCopies.get(document);if(!copies){copies=[];const walker=document.createTreeWalker(document.body,4);let node:Node|null;while((node=walker.nextNode())){const text=node as Text;if(/\b(MRI|CT)\b/.test(text.data)&&!text.parentElement?.closest('#welcome-screen,#mri-name,#ct-name,script,style'))copies.push({node:text,source:text.data})}staticCopies.set(document,copies)}for(const copy of copies)if(copy.node.isConnected)copy.node.data=imagingText(copy.source)

 document.querySelectorAll<HTMLElement>('[data-image-label]').forEach(el=>{if(!el.textContent?.includes(' · preview'))el.textContent=imageLabel(el.dataset.imageLabel as Modality)})
 const direction=document.querySelector<HTMLSelectElement>('#direction');if(direction)for(const option of direction.options)option.textContent=option.value==='ct-mri'?`${imageLabel('ct')} → ${imageLabel('mri')}`:`${imageLabel('mri')} → ${imageLabel('ct')}`
 for(const id of ['processing-target','processing-edit-target']){const select=document.querySelector<HTMLSelectElement>('#'+id);if(select)for(const option of select.options){const m=option.value.startsWith('ct')?'ct':'mri';option.textContent=imageLabel(m)+(id==='processing-edit-target'?(option.value.endsWith('-mask')?' binary mask':' image'):'')}}
 document.querySelectorAll<HTMLButtonElement>('[data-manual-target]').forEach(b=>b.textContent='Adjust '+imageLabel(b.dataset.manualTarget as Modality))
 const volumes=document.querySelector('.toolbar-cluster .cluster-label');if(volumes)volumes.textContent='Volumes · '+REGISTRATION_PAIRINGS[pairing].title.replace(' registration','')
 const workspace=document.querySelector('.workspace');workspace?.setAttribute('aria-label',`${imageLabel('mri')}, ${imageLabel('ct')} and aligned overlay in three anatomical planes`)
}
