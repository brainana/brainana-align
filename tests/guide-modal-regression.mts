import assert from 'node:assert/strict'
import fs from 'node:fs'
import {JSDOM} from 'jsdom'
import {alignmentLayout} from '../apps/align/src/alignmentLayout'
import {installGuidedSetup,type GuideState} from '../apps/align/src/guidedSetup'
import {installExportDestination} from '../apps/align/src/exportDestination'
import {installDataSourceChooser} from '../apps/align/src/dataSourceChooser'
const dom=new JSDOM(alignmentLayout('modal'),{pretendToBeVisual:true,url:'http://localhost'}),w=dom.window,d=w.document
Object.assign(globalThis,{window:w,document:d});let scrolls=0;w.HTMLElement.prototype.scrollIntoView=function(){scrolls++};w.matchMedia=(()=>({matches:true})) as any
const style=d.createElement('style');style.textContent=fs.readFileSync(new URL('../apps/align/src/style.css',import.meta.url),'utf8');d.head.append(style)
const state:GuideState={mri:false,ct:false,landmarks:[],selectedId:null,minimumPairs:3,fit:null,busy:false,error:null,viewerReady:true},guide=installGuidedSetup(()=>state),card=d.querySelector<HTMLElement>('#guided-card')!
const tick=async()=>{await Promise.resolve();await Promise.resolve()}
const click=async(selector:string)=>{d.querySelector<HTMLElement>(selector)!.click();await tick();guide.refresh()}
const visible=()=>{assert.equal(w.getComputedStyle(card).visibility,'visible');assert.notEqual(w.getComputedStyle(card).pointerEvents,'none')}
const suspended=()=>{assert.equal(w.getComputedStyle(card).visibility,'hidden','No guide pixels may cover an open dialog');assert.equal(w.getComputedStyle(card).pointerEvents,'none','Guide cannot intercept dialog input')}
installDataSourceChooser({connection:async()=>({remote:false,canConnect:false}),browse:async()=>{},connect:async()=>{}})
await click('#guided-setup');visible()
for(const modality of ['mri','ct']){
 state.mri=modality==='ct';guide.refresh();const expected=card.dataset.step
 await click('#'+modality+'-file');const modal=d.querySelector<HTMLElement>('#data-source-modal')!;assert(!modal.classList.contains('hidden'));suspended();assert(Number(w.getComputedStyle(modal).zIndex)>Number(w.getComputedStyle(card).zIndex),'Dialog also has a higher stacking layer')
 const focused=d.activeElement,priorScrolls=scrolls;guide.refresh();assert.equal(d.activeElement,focused);assert.equal(scrolls,priorScrolls,'Guide must not scroll controls behind a modal')
 modal.dispatchEvent(new w.KeyboardEvent('keydown',{key:'Escape',bubbles:true,cancelable:true}));await tick();assert(modal.classList.contains('hidden'));assert(!card.classList.contains('hidden'),'Dialog Escape must not pause the guide');visible();assert.equal(card.dataset.step,expected)
}
let checked=0
for(const id of ['data-source-modal','network-browser-modal','server-browser-modal','preparation-modal','settings-modal','export-modal']){
 let modal=d.getElementById(id);const created=!modal;if(!modal){modal=d.createElement('div');modal.id=id;modal.className='modal hidden';modal.innerHTML='<input aria-label="Dialog control">';d.body.append(modal)}
 modal.classList.remove('hidden');await tick();suspended();const before=card.dataset.step;state.ct=true;guide.refresh();assert.equal(card.dataset.step,before,'No route projection while a modal owns interaction')
 // Another dialog remains open: closing only one must not reveal the guide.
 const second=d.createElement('div');second.className='modal';d.body.append(second);modal.classList.add('hidden');await tick();suspended();second.remove();await tick();visible();checked++
 // Respect hidden attribute as well as the application hidden class.
 modal.hidden=true;modal.classList.remove('hidden');await tick();visible();modal.hidden=false;await tick();suspended();modal.classList.add('hidden');await tick();visible();if(created)modal.remove()
}
// The workstation export folder used a separate overlay; it now shares the
// same modal lifecycle, including if its parent Export closes before it does.
Object.assign(globalThis,{CustomEvent:w.CustomEvent,fetch:async(url:string)=>new Response(JSON.stringify(url==='/api/config'?{mode:'remote'}:{path:'',entries:[]}))})
installExportDestination();await new Promise(r=>setTimeout(r,0));d.querySelector('#export-modal')!.classList.remove('hidden');const destination=d.querySelector<HTMLSelectElement>('#ba-destination')!;destination.value='workstation';destination.dispatchEvent(new w.Event('change'));await click('#ba-folder-button');await new Promise(r=>setTimeout(r,0));assert(d.querySelector('#ba-folder-modal')!.matches('.modal:not(.hidden)'));suspended();d.querySelector('#export-modal')!.classList.add('hidden');await tick();suspended();await click('#ba-remote-cancel');visible()
// New and removed dialogs, native dialogs, and manually moved guide positions.
card.style.left='200px';card.style.top='100px';const native=d.createElement('dialog');native.setAttribute('open','');d.body.append(native);await tick();suspended();native.remove();await tick();visible()
// Legacy handlers can close on Escape without preventDefault. Ownership must
// have been captured while the dialog was still open.
const legacy=d.createElement('div');legacy.className='modal';legacy.addEventListener('keydown',()=>legacy.classList.add('hidden'));d.body.append(legacy);await tick();legacy.dispatchEvent(new w.KeyboardEvent('keydown',{key:'Escape',bubbles:true}));await tick();assert(!card.classList.contains('hidden'));visible();legacy.remove()
await click('#guided-close');const resume=d.querySelector<HTMLElement>('#guided-resume')!,blocking=d.createElement('div');blocking.className='modal';d.body.append(blocking);await tick();assert.equal(w.getComputedStyle(resume).visibility,'hidden');blocking.remove();await tick();assert.equal(w.getComputedStyle(resume).visibility,'visible');assert(card.classList.contains('hidden'),'Closing a dialog must not unpause an intentionally hidden guide')
await click('#guided-resume');visible();guide.close();dom.window.close()
console.log(`Guide dialogs: real MRI/CT loading and Escape, ${checked+1} dialog types, stacking/CSS visibility, focus/scroll protection, nested/replaced/removed/hidden/native dialogs and paused resume passed.`)
