import type {FitState} from './appTypes'
export function installExportDialog(read:()=>{fit:FitState|null;images:boolean;busy:boolean},review:()=>void){
 const modal=document.querySelector<HTMLElement>('#export-modal')!,notice=modal.querySelector<HTMLElement>('#export-notice')!,feedback=modal.querySelector<HTMLElement>('#export-feedback')!,button=modal.querySelector<HTMLButtonElement>('#export-review-proposal')!
 let running=false
 const report=(message:string,error=false)=>{if(modal.classList.contains('hidden'))return;feedback.textContent=message;feedback.classList.toggle('error',error)}
 const refresh=()=>{
  const s=read(),busy=s.busy||running,reason=busy?'Wait for the current alignment or export to finish.':s.fit?.proposal?'This alignment is a pending preview. Review and accept or reject it before exporting aligned images or transforms. You can still save a reloadable session.':!s.fit?'Create or accept an alignment before exporting aligned images or transforms.':''
  const message=(!s.fit&&!busy?'Standalone masks and working images can be exported in the separate section below. Accept an alignment to export a transformed image.':reason)||(!s.images?'Load both MRI and CT to export an aligned image. Transform files and sessions can still be saved.':'')
  notice.textContent=message;notice.classList.toggle('hidden',!message);button.classList.toggle('hidden',!s.fit?.proposal);button.disabled=busy
  modal.querySelector<HTMLButtonElement>('#save-session-export')!.disabled=busy
  modal.querySelector<HTMLButtonElement>('#save-generated-image')!.disabled=busy||!modal.querySelector<HTMLSelectElement>('#generated-image-choice')!.options.length
  modal.querySelector<HTMLButtonElement>('#save-transform')!.disabled=Boolean(reason)
  modal.querySelector<HTMLButtonElement>('#save-aligned')!.disabled=Boolean(reason)||!s.images
  modal.querySelector<HTMLButtonElement>('#save-spline-fields')!.disabled=Boolean(reason)||!s.images
 }
 button.addEventListener('click',review)
 window.addEventListener('align-export-result',event=>{const {message,error}=(event as CustomEvent).detail;report(message,error)})
 const run=async(action:()=>Promise<void>)=>{if(running||read().busy){report('Wait for the current alignment or export to finish.',true);return}running=true;refresh();try{await action()}catch(error){report(error instanceof Error?error.message:String(error),true);throw error}finally{running=false;refresh()}}
 return {refresh,report,run,open(){feedback.textContent='';refresh();modal.classList.remove('hidden')}}
}
