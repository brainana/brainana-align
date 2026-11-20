/** Session-local fitting choice; changing it never applies a transform. */
export function installLandmarkReflectionControls(){
 const model=document.querySelector<HTMLSelectElement>('#landmark-transform')!,choice=document.querySelector<HTMLInputElement>('#landmark-allow-reflection')!,hint=document.querySelector<HTMLElement>('#landmark-reflection-hint')!,row=document.querySelector<HTMLElement>('#landmark-reflection-choice')!
 const refresh=()=>{const affine=model.value==='affine';row.classList.remove('hidden');choice.disabled=false;hint.textContent=affine?'This choice also controls affine fitting. If its best-fitting matrix mirrors anatomy while reflection is off, the fit is rejected without changing the alignment. Allowing reflection still requires confirmation before applying a mirrored result.':'Optional: compare ordinary and left–right reflected fits using the same transform model. Reflection is a discrete choice, adding no scale or shear to rigid fitting. Four or more pairs spanning 3D are needed to infer handedness; planar ties keep the ordinary fit. Confirm any mirrored result before applying.'}
 model.addEventListener('change',refresh);refresh()
 return {get enabled(){return choice.checked},restore(value:boolean){choice.checked=value;refresh()},refresh}
}
