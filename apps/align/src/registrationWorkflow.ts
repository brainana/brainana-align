import type {FitState} from './appTypes'

/** Older landmark sessions have no initializationSource; their fitted pairs identify them. */
export function hasLandmarkBasis(fit:FitState|null):boolean {
  return Boolean(fit && fit.initializationSource!=='image-geometry' && (fit.initializationSource!=='automatic'||fit.automaticRegistration?.operation==='local') && fit.landmarkSnapshot.filter(p=>p.enabled&&p.mri&&p.ct).length>=3)
}
/** Proposal review stays with its workflow, including older local sessions. */
export function isLocalRegistration(fit:FitState|null):boolean {
  return Boolean(fit&&((fit.nonlinear&&fit.nonlinear.kernel!=='jip-dense-pull')||fit.automaticRegistration?.operation==='local'||(fit.proposal&&fit.proposal.kind!=='automatic'&&hasLandmarkBasis(fit))))
}
/** Two workflow-local editors share one physical region state and capture controller. */
export function installSharedRegionControls(){
 const canonical=document.querySelector<HTMLButtonElement>('#define-windows')!
 const mirror=document.querySelector<HTMLButtonElement>('#define-windows-followup')!
 const target=document.querySelector<HTMLSelectElement>('#window-target')!
 const mirrorTarget=document.querySelector<HTMLSelectElement>('#window-target-followup')!
 const sync=()=>{if(!canonical.isConnected||!mirror.isConnected)return;mirror.textContent=canonical.textContent;mirror.setAttribute('aria-pressed',canonical.getAttribute('aria-pressed')??'false');mirrorTarget.value=target.value;document.querySelector('#window-summary-followup')!.textContent=document.querySelector('#window-summary')!.textContent}
 let forwarding=false
 const operation=(value:string)=>{const select=document.querySelector<HTMLSelectElement>('#intensity-operation');if(select&&select.value!==value){select.value=value;select.dispatchEvent(new window.Event('change',{bubbles:true}))}}
 canonical.addEventListener('click',()=>{if(!forwarding)operation('global');sync()})
 mirror.addEventListener('click',()=>{operation('local');forwarding=true;try{canonical.click()}finally{forwarding=false;sync()}})
 document.querySelector('#clear-windows-followup')!.addEventListener('click',()=>{document.querySelector<HTMLButtonElement>('#clear-windows')!.click();sync()})
 mirrorTarget.addEventListener('change',()=>{target.value=mirrorTarget.value;target.dispatchEvent(new window.Event('change',{bubbles:true}));sync()})
 target.addEventListener('change',sync)
 const observer=new window.MutationObserver(sync);observer.observe(canonical.closest('.optimization-window-controls')!,{subtree:true,childList:true,characterData:true,attributes:true,attributeFilter:['aria-pressed']});window.addEventListener('pagehide',()=>observer.disconnect(),{once:true})
 sync()
}
