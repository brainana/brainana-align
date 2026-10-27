import {preferenceStorage} from './preferences'

// Reload after clearing recovery state so all volumes, WebGL resources, workers,
// pending file reads, selection history and in-memory session state are discarded together.
export function installWorkspaceReset(options:{activity:{busy:boolean;begin:(text:string)=>Promise<boolean>;end:()=>void};status:(text:string,error?:boolean)=>void;reload?:()=>void}){
  const button=document.querySelector<HTMLButtonElement>('#reset-workspace')!
  let resetting=false
  button.addEventListener('click',()=>{void (async()=>{
    if(resetting||options.activity.busy)return
    if(!window.confirm('Reset workspace and start over?\n\nThis unloads MRI and CT, clears landmarks, alignment and proposals, optimization windows, undo history, guide progress, recovery snapshots and recent dataset names. Unsaved work will be lost.\n\nYour appearance/settings, original image files and saved exports are kept. Cancel and Save session first if you need this work.'))return
    resetting=true
    if(!await options.activity.begin('Resetting workspace…')){resetting=false;return}
    try{
      window.dispatchEvent(new window.Event('align-workspace-reset'))
      for(const id of ['guided-card','guided-resume','settings-modal']){const el=document.getElementById(id);if(el)el.inert=true}
      for(const key of ['guide','recovery','recent','layout'])await preferenceStorage.write(key,null)
      ;(options.reload??(()=>window.location.reload()))()
    }catch(error){
      resetting=false;options.activity.end();window.dispatchEvent(new window.Event('align-workspace-reset-failed'))
      for(const id of ['guided-card','guided-resume','settings-modal']){const el=document.getElementById(id);if(el)el.inert=false}
      options.status(`Workspace reset could not finish: ${error instanceof Error?error.message:String(error)}. Your loaded workspace remains open; retry Reset workspace.`,true)
    }
  })()})
}
