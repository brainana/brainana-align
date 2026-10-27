import {imageLabel} from './registrationPairing'
import type {Landmark,Modality} from './appTypes'

type Options = {
 getLandmarks:()=>Landmark[]
 getSelected:()=>number|null
 select:(id:number)=>void
 toggle:(id:number,enabled:boolean)=>void
 remove:(id:number)=>void
 setPoint:(id:number,modality:Modality)=>void
 go:(id:number,modality:Modality)=>void
}

/** Point-list presentation only. Model changes and navigation belong to the app. */
export function installLandmarkList(o:Options){
 const list=document.querySelector<HTMLDivElement>('#landmark-list')!
 const count=document.querySelector('#pair-count')!
 const summary=document.querySelector<HTMLDivElement>('#selected-landmark-summary')!
 let rendered:string|null=null
 return function render(){
  const landmarks=o.getLandmarks(),selectedId=o.getSelected()
  const signature=JSON.stringify([imageLabel('mri'),imageLabel('ct'),selectedId,landmarks])
  // Slice/crosshair/opacity updates do not alter this list. Preserve focus and
  // guide targets instead of replacing every row/listener on every frame.
  if(signature===rendered)return
  rendered=signature
  count.textContent=`${landmarks.length} pair${landmarks.length===1?'':'s'}`
  const selected=landmarks.find(lm=>lm.id===selectedId)
  summary.textContent=selected
   ? `Landmark ${selected.id} · ${imageLabel('mri')} ${selected.mri?'set':'not set'} · ${imageLabel('ct')} ${selected.ct?'set':'not set'} · ${selected.enabled?'included':'excluded'}${selected.residual!==undefined?` · residual ${selected.residual.toFixed(2)} mm`:''}`
   : 'No landmark selected'
  list.innerHTML=landmarks.length?'':'<div class="empty">No landmarks yet.</div>'
  for(const lm of landmarks){
   const row=document.createElement('div')
   row.dataset.landmarkId=String(lm.id)
   row.className=`landmark-row ${selectedId===lm.id?'selected':''}`
   const coord=(p:Landmark['mri'])=>p?p.map(v=>v.toFixed(1)).join(', '):'not set'
   row.innerHTML=`
    <div class="landmark-head"><button class="select">Landmark ${lm.id}</button><label><input class="enabled" type="checkbox" ${lm.enabled?'checked':''}> use</label><button class="delete" title="Delete">×</button></div>
    <div class="point-row"><span>${imageLabel('mri')}</span><code>${coord(lm.mri)}</code><button class="set-mri">${lm.mri?'Replace':'Set'}</button><button class="go-mri" ${lm.mri?'':'disabled'}>Go</button></div>
    <div class="point-row"><span>${imageLabel('ct')}</span><code>${coord(lm.ct)}</code><button class="set-ct">${lm.ct?'Replace':'Set'}</button><button class="go-ct" ${lm.ct?'':'disabled'}>Go</button></div>
    ${lm.residual!==undefined?`<div class="residual">Residual: ${lm.residual.toFixed(2)} mm</div>`:''}`
   // Capture IDs only: session/Undo may replace objects with identical values.
   row.querySelector('.select')!.addEventListener('click',()=>o.select(lm.id))
   row.querySelector('.enabled')!.addEventListener('change',event=>o.toggle(lm.id,(event.target as HTMLInputElement).checked))
   row.querySelector('.delete')!.addEventListener('click',()=>o.remove(lm.id))
   for(const modality of ['mri','ct'] as const){
    row.querySelector(`.set-${modality}`)!.addEventListener('click',()=>o.setPoint(lm.id,modality))
    row.querySelector(`.go-${modality}`)!.addEventListener('click',()=>o.go(lm.id,modality))
   }
   list.append(row)
  }
 }
}
