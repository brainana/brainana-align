import assert from 'node:assert/strict'
import {JSDOM} from 'jsdom'
import {alignmentLayout} from '../apps/align/src/alignmentLayout'
import {installWelcomeScreen} from '../apps/align/src/welcomeScreen'
import {installGuidedSetup,type GuideState} from '../apps/align/src/guidedSetup'
import {installWorkflowFocus} from '../apps/align/src/workflowUI'
import {installDataSourceChooser} from '../apps/align/src/dataSourceChooser'
import {REGISTRATION_PAIRINGS,setRegistrationPairing,getRegistrationPairing} from '../apps/align/src/registrationPairing'
const tick=async()=>{await Promise.resolve();await Promise.resolve()}
let cases=0
for(const pairing of Object.keys(REGISTRATION_PAIRINGS) as (keyof typeof REGISTRATION_PAIRINGS)[])for(const launcher of ['#welcome-guided','#guided-setup'])for(const order of ['welcome-first','guide-first'])for(const first of ['mri','ct'] as const){
 const dom=new JSDOM(alignmentLayout('startup'),{pretendToBeVisual:true,url:'http://localhost'}),w=dom.window,d=w.document
 Object.assign(globalThis,{window:w,document:d});w.HTMLElement.prototype.scrollIntoView=function(){};w.matchMedia=(()=>({matches:true})) as any
 setRegistrationPairing('mri-ct');installWorkflowFocus()
 const state:GuideState={mri:false,ct:false,landmarks:[],selectedId:null,minimumPairs:3,fit:null,busy:false,error:null,viewerReady:true,preparation:{mri:false,ct:false,ready:false,open:false}}
 let welcome:ReturnType<typeof installWelcomeScreen>|undefined
 const read=()=>({...state,welcome:welcome?.isWelcome()??true})
 const installWelcome=()=>welcome=installWelcomeScreen(()=>Boolean(state.mri||state.ct||state.preparation?.mri||state.preparation?.ct))
 let guide:ReturnType<typeof installGuidedSetup>
 if(order==='welcome-first'){installWelcome();guide=installGuidedSetup(read)}else{guide=installGuidedSetup(read);installWelcome()}
 installDataSourceChooser({connection:async()=>({remote:false,canConnect:false}),browse:async()=>{},connect:async()=>{}})
 let fileActions=0;for(const m of ['mri','ct'])d.querySelector('#'+m+'-file')!.addEventListener('click',()=>fileActions++)
 const click=async(selector:string)=>{d.querySelector<HTMLElement>(selector)!.click();await tick();guide.refresh()}
 const step=()=>d.querySelector<HTMLElement>('#guided-card')!.dataset.step
 await click(launcher)
 assert.equal(step(),'welcome');assert(welcome!.isWelcome());assert(d.querySelector('main')!.classList.contains('welcome-hidden'));assert(d.querySelector('.welcome-pairings')!.classList.contains('guided-target'));assert.equal(fileActions,0);assert.equal(getRegistrationPairing(),'mri-ct')
 await click('#welcome-guided');assert(!d.querySelector('#guided-card')!.classList.contains('hidden'),'Repeated welcome Guide must remain active')
 await click('#guided-anchor');await click('#guided-restart');assert.equal(step(),'welcome');assert.equal(fileActions,0)
 await click('#guided-close');await click('#guided-resume');assert.equal(step(),'welcome');assert(welcome!.isWelcome())
 await click(`[data-registration-pairing="${pairing}"]`);assert.equal(getRegistrationPairing(),pairing);assert(!welcome!.isWelcome());assert.equal(step(),'mri');assert.equal(d.querySelector('#guided-title')!.textContent,'Load your '+REGISTRATION_PAIRINGS[pairing].first);assert.equal(fileActions,0,'Choosing a pairing must not load images');assert(d.querySelector<HTMLButtonElement>('#guided-back')!.disabled,'Do not offer Back into a welcome screen that has already been left')
 // Opening/cancelling source selection and the local picker does not satisfy loading.
 await click('#mri-file');assert(!d.querySelector('#data-source-modal')!.classList.contains('hidden'));await click('#data-source-close');assert.equal(step(),'mri');assert.equal(fileActions,0)
 await click('#mri-file');await click('#data-source-local');assert.equal(fileActions,1);assert.equal(step(),'mri')
 // Real selection notifications are emitted only after successful header reads.
 state.preparation![first]=true;w.dispatchEvent(new w.Event('align-volume-selection'));guide.refresh();assert.equal(step(),first==='mri'?'ct':'mri')
 const second=first==='mri'?'ct':'mri';state.preparation![second]=true;w.dispatchEvent(new w.Event('align-volume-selection'));guide.refresh();assert.equal(step(),'prepare')
 await click('#guided-restart');assert.equal(step(),'prepare','Restart cannot bypass pending preparation or reopen welcome')
 state.busy=true;state.busyKind='preparation';guide.refresh();assert.equal(step(),'working');state.busy=false;state.error='Preparation cancelled';guide.refresh();assert.equal(step(),'prepare','Failed/cancelled preparation cannot advance')
 state.preparation!.ready=true;state.preparation!.open=false;state.mri=state.ct=true;state.error=null;guide.refresh();assert.equal(step(),'choose');assert.equal(d.querySelectorAll('aside details[open]').length,0)
 guide.close();dom.window.close();cases++
}
console.log(`Integrated startup: ${cases} sequences covering both launchers, both install orders, all pairings, either header order, cancel/retry, pause/resume, restart and preparation completion passed.`)

// Opening a session is an alternative welcome path; cancellation stays here,
// successful restore chooses the saved pairing and must not re-request images.
for(const pairing of Object.keys(REGISTRATION_PAIRINGS) as (keyof typeof REGISTRATION_PAIRINGS)[]){
 const dom=new JSDOM(alignmentLayout('session-startup'),{pretendToBeVisual:true,url:'http://localhost'}),w=dom.window,d=w.document;Object.assign(globalThis,{window:w,document:d});w.HTMLElement.prototype.scrollIntoView=function(){};w.matchMedia=(()=>({matches:true})) as any
 setRegistrationPairing('mri-ct');const state:GuideState={mri:false,ct:false,landmarks:[],selectedId:null,minimumPairs:3,fit:null,busy:false,error:null,viewerReady:false}
 const welcome=installWelcomeScreen(()=>state.mri||state.ct),guide=installGuidedSetup(()=>({...state,welcome:welcome.isWelcome()}));let sessionPicks=0;d.querySelector('#load-session')!.addEventListener('click',()=>sessionPicks++)
 d.querySelector<HTMLElement>('#welcome-guided')!.click();assert.equal(d.querySelector<HTMLElement>('#guided-card')!.dataset.step,'welcome','Pairing precedes even a pending WebGL warning')
 d.querySelector<HTMLElement>('#welcome-session')!.click();guide.refresh();assert.equal(sessionPicks,1);assert(welcome.isWelcome(),'Cancelled session selection retains welcome');assert.equal(d.querySelector<HTMLElement>('#guided-card')!.dataset.step,'welcome')
 state.mri=state.ct=true;state.viewerReady=true;welcome.restore(pairing);guide.refresh();assert(!welcome.isWelcome());assert.equal(getRegistrationPairing(),pairing);assert.equal(d.querySelector<HTMLElement>('#guided-card')!.dataset.step,'choose');guide.close();dom.window.close()
}
console.log('Startup session alternatives: cancelled selection, four restored pairings and welcome before viewer-readiness warnings passed.')
