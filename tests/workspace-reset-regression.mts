import assert from 'node:assert/strict'
import {createRequire} from 'node:module'
import {installWorkspaceReset} from '../apps/align/src/workspaceReset'
const {JSDOM}=createRequire(import.meta.url)('jsdom')
async function fixture({confirm=true,busy=false,fail=false}={}){
 const dom=new JSDOM('<button id="reset-workspace">Reset workspace</button><div id="guided-card"></div><button id="guided-resume"></button><div id="settings-modal"></div>',{pretendToBeVisual:true})
 Object.assign(globalThis,{window:dom.window,document:dom.window.document})
 const stored:any={guide:{route:'refinement'},recovery:{current:{fit:'old'},previous:{fit:'older'}},recent:['MRI'],layout:{workflow:'automatic'},preferences:{appearance:'light'},presets:{mine:'colors'},export:{folder:'chosen'}}
 const events:string[]=[],statuses:string[]=[];let reloads=0,confirms=0,starts=0,ends=0
 dom.window.confirm=(text:string)=>{confirms++;assert(text.includes('Unsaved work will be lost'));assert(text.includes('appearance/settings'));return confirm}
 dom.window.brainanaPreferences={read:async(key:string)=>stored[key],write:async(key:string,value:any)=>{events.push(key);await new Promise(r=>setTimeout(r,2));if(fail&&key==='recovery')throw Error('disk full');stored[key]=value}}
 dom.window.addEventListener('align-workspace-reset',()=>events.push('reset-event'))
 dom.window.addEventListener('align-workspace-reset-failed',()=>events.push('failed-event'))
 installWorkspaceReset({activity:{get busy(){return busy},begin:async()=>{starts++;busy=true;return true},end:()=>{ends++;busy=false}},status:text=>statuses.push(text),reload:()=>{assert.deepEqual(events,['reset-event','guide','recovery','recent','layout']);reloads++;events.push('reload')}})
 const click=()=>dom.window.document.querySelector<HTMLButtonElement>('#reset-workspace')!.click()
 click();click();await new Promise(r=>setTimeout(r,40))
 if(!confirm||busy&&starts===0){assert.equal(reloads,0);assert.equal(starts,0);assert.equal(stored.guide.route,'refinement');assert.equal(events.length,0)}
 else if(fail){assert.equal(reloads,0);assert.equal(ends,1);assert(statuses[0].includes('disk full'));assert(events.includes('failed-event'));assert.equal(dom.window.document.querySelector<HTMLElement>('#guided-card')!.inert,false);fail=false;events.length=0;click();await new Promise(r=>setTimeout(r,40));assert.equal(reloads,1,'Retry after storage failure completes');assert.equal(stored.recovery,null)}
 else{assert.equal(reloads,1);assert.equal(confirms,1);assert.equal(starts,1);for(const key of ['guide','recovery','recent','layout'])assert.equal(stored[key],null);assert.equal(dom.window.document.querySelector<HTMLElement>('#guided-card')!.inert,true)}
 assert.deepEqual(stored.preferences,{appearance:'light'});assert.deepEqual(stored.presets,{mine:'colors'});assert.deepEqual(stored.export,{folder:'chosen'})
 dom.window.close()
}
await fixture();await fixture({confirm:false});await fixture({busy:true});await fixture({fail:true})
console.log('Workspace reset: confirmation/cancel, busy and duplicate-click guards, ordered storage clearing before reload, both recovery generations removed, settings/presets/export preserved, failed clear prevents reload and permits retry.')
