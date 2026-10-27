import assert from 'node:assert/strict'
import {JSDOM} from 'jsdom'
import {alignmentLayout} from '../apps/align/src/alignmentLayout'
import {installGuidedSetup,type GuideState} from '../apps/align/src/guidedSetup'
import {installWorkflowFocus} from '../apps/align/src/workflowUI'
import {imageGeometryAlignment} from '../apps/align/src/geometryInitialization'

const tick=async()=>{await Promise.resolve();await Promise.resolve()}
function fixture(){
 const dom=new JSDOM(alignmentLayout('ordering'),{pretendToBeVisual:true,url:'http://localhost'}),w=dom.window,d=w.document
 Object.assign(globalThis,{window:w,document:d});w.HTMLElement.prototype.scrollIntoView=function(){};w.matchMedia=(()=>({matches:true})) as any;installWorkflowFocus()
 const state:GuideState={mri:true,ct:true,landmarks:[],selectedId:null,minimumPairs:3,fit:null,busy:false,error:null,viewerReady:true}
 const guide=installGuidedSetup(()=>state),card=d.querySelector<HTMLElement>('#guided-card')!
 const step=()=>card.dataset.step||'',press=async(selector:string)=>{const el=d.querySelector<HTMLElement>(selector);assert(el,selector);el.click();await tick();guide.refresh()}
 const action=async(label:string)=>{const button=[...d.querySelectorAll<HTMLButtonElement>('#guided-actions button')].find(b=>b.textContent===label);assert(button,`${label} at ${step()}`);button.click();await tick();guide.refresh()}
 const headers=async()=>{for(let i=0;i<15&&/^teach-(open|close)-/.test(step());i++){const heading=d.querySelector<HTMLElement>('summary.guided-target');assert(heading);heading.click();await tick();guide.refresh()}assert(!/^teach-(open|close)-/.test(step()))}
 return {dom,w,d,state,guide,card,step,press,action,headers,close:()=>{guide.close();dom.window.close()}}
}

// Reproduce the user's exact failure: Restart from Close Initial processing.
// Cover every other open section too, and assert the visible step immediately
// without an auto-follow helper that would mask this navigation bug.
for(const id of ['initial-processing-panel','landmarks-panel','landmark-alignment-panel','followup-panel','manual-panel','refine-panel','alignment-display-panel']){
 const f=fixture();f.state.landmarks=[{id:1,enabled:true,mri:[1,2,3],ct:[4,5,6]}];f.state.fit=imageGeometryAlignment('ct-mri');f.d.querySelector<HTMLDetailsElement>('#'+id)!.open=true
 const next=id==='refine-panel'?'Review initial processing':'Skip landmarks · automatic registration'
 await f.press('#guided-setup');assert.equal(f.step(),'choose');await f.action(next);assert.equal(f.step(),'teach-close-'+id)
 const data=JSON.stringify(f.state),panels=[...f.d.querySelectorAll<HTMLDetailsElement>('aside details')].map(p=>p.open)
 await f.press('#guided-restart');assert.equal(f.step(),'choose','Restart must visibly show starting choices before any Close-panel prompt');assert.equal(f.d.querySelector('#guided-title')!.textContent,'How would you like to begin?');assert.equal(JSON.stringify(f.state),data);assert.deepEqual([...f.d.querySelectorAll<HTMLDetailsElement>('aside details')].map(p=>p.open),panels);assert(f.d.querySelector<HTMLButtonElement>('#guided-back')!.disabled,'Restart clears instruction history')
 f.guide.refresh();assert.equal(f.step(),'choose');await f.action(next);assert.equal(f.step(),'teach-close-'+id,'Panel teaching resumes only after choosing a task');f.close()
}

for(const prerequisite of ['mri','ct','prepare']){
 const f=fixture();f.state.mri=prerequisite==='ct';f.state.ct=false;if(prerequisite==='prepare')f.state.preparation={mri:true,ct:true,ready:false};f.d.querySelector<HTMLDetailsElement>('#initial-processing-panel')!.open=true
 await f.press('#guided-setup');assert.equal(f.step(),prerequisite,'An unmet loading prerequisite is not a Close-panel task');await f.press('#guided-restart');assert.equal(f.step(),prerequisite);assert(f.d.querySelector<HTMLDetailsElement>('#initial-processing-panel')!.open);f.close()
}
console.log('Restart: immediate starting choices from all seven Close-panel prompts, preserved data/panels and resumed teaching passed.')

// A cancelled continuation must never revive after Restart, Back, or reset.
for(const cancel of ['restart','back','reset']){
 const f=fixture();await f.press('#guided-setup');await f.action('Review initial processing');await f.headers();await f.action('Continue');await f.action('Continue')
 f.d.querySelector('#processing-crop-draw')!.setAttribute('aria-pressed','true')
 await f.action('Continue');await f.headers();assert.equal(f.d.querySelector('#guided-title')!.textContent,'Finish the processing tool first')
 if(cancel==='restart')await f.press('#guided-restart')
 else if(cancel==='back')await f.press('#guided-back')
 else {f.w.dispatchEvent(new f.w.Event('align-guide-reset'));f.guide.refresh()}
 const expected=cancel==='back'?'processing-crop':'choose'
 f.d.querySelector('#processing-crop-draw')!.setAttribute('aria-pressed','false');f.guide.refresh();await f.headers()
 assert.equal(f.step(),expected,`${cancel} must cancel the previously requested next processing stage`)
 f.close()
}
console.log('Guide ordering: cancelled processing continuations passed.')

// A result can arrive after the user has requested a different control.
{
 const f=fixture();await f.press('#guided-setup');await f.action('Guide me through landmarks');await f.headers();await f.action('Show Add landmark pair')
 f.state.fit=imageGeometryAlignment('ct-mri');f.state.fit.automaticRegistration={operation:'global',metric:'nmi'} as any;f.state.fit.proposal={kind:'automatic',matrix:f.state.fit.matrix,params:[],scoreBefore:1,scoreAfter:2};f.guide.refresh();await f.headers()
 assert.equal(f.step(),'proposal','A newly arriving proposal must supersede an earlier Show control instruction')
 f.close()
}

// Enumerate every top-level disclosure combination against controls in each
// workflow, including the nested spline. Projection must converge through real
// summary clicks and leave all data and unrelated disclosures alone.
const {guideNavigationStep}=await import('../apps/align/src/guideTeaching')
let navigationCases=0,walkActions=0
{
 const f=fixture(),panels=[...f.d.querySelectorAll<HTMLDetailsElement>('aside details')].filter(p=>!p.parentElement?.closest('details'))
 assert.equal(panels.length,7)
 const targets=['#processing-display','#processing-masking','#processing-cropping','#processing-editing','#new-pair','#fit-side','#spline-smoothing','#local-auto-linear','#manual-start-geometry','#auto-linear','#alignment-display-panel','#export']
 for(let mask=0;mask<2**panels.length;mask++)for(const target of targets){
  panels.forEach((panel,i)=>panel.open=Boolean(mask&(1<<i)));f.d.querySelector<HTMLDetailsElement>('#spline-alignment')!.open=false
  const stateBefore=JSON.stringify(f.state),base={key:target==='#new-pair'?'pairs':'ordering',target,title:'Ordering',text:'Ordering'}
  for(let n=0;n<12;n++){
   const before=panels.map(p=>p.open),step=guideNavigationStep(base)
   assert.deepEqual(panels.map(p=>p.open),before,'Projection cannot mutate panels')
   if(!step.key.startsWith('teach-')){assert.equal(step.target,target);break}
   assert(n<11,'Disclosure teaching must converge');const summary=f.d.querySelector<HTMLElement>(step.target)!;assert(summary);assert.equal(summary.tagName,'SUMMARY');summary.click()
  }
  const projected=guideNavigationStep(base);assert.equal(projected.target,target)
  assert.equal(JSON.stringify(f.state),stateBefore);navigationCases++
 }
 f.close()
}

// Deterministic bounded state walks: arbitrary guide choices mixed with manual
// panel changes, busy transitions, new results, pause/resume, Back and Restart.
// Mocks supply state transitions only; scientific workers have separate suites.
for(let seed=1;seed<=16;seed++){
 const f=fixture();let rng=seed;const random=(n:number)=>{rng=(Math.imul(rng,1664525)+1013904223)>>>0;return rng%n}
 await f.press('#guided-setup')
 for(let i=0;i<100;i++){
  if(f.card.classList.contains('hidden'))await f.press('#guided-setup')
  const choice=random(9)
  if(choice===0){const headings=[...f.d.querySelectorAll<HTMLElement>('aside details > summary')];headings[random(headings.length)].click()}
  else if(choice===1){await f.press('#guided-close');await f.press('#guided-setup')}
  else if(choice===2)await f.press('#guided-restart')
  else if(choice===3){const back=f.d.querySelector<HTMLButtonElement>('#guided-back')!;if(!back.disabled)back.click()}
  else if(choice===4){f.state.busy=!f.state.busy;f.state.busyKind='registration'}
  else if(choice===5&&!f.state.busy){
   if(f.state.fit?.proposal)f.state.fit.proposal=null
   else {f.state.fit=imageGeometryAlignment('ct-mri');f.state.fit.automaticRegistration={operation:random(2)?'global':'local',metric:'nmi'} as any;f.state.fit.proposal={kind:random(2)?'automatic':'spline',matrix:f.state.fit.matrix,params:[],scoreBefore:1,scoreAfter:2}}
  }else{
   const actions=[...f.d.querySelectorAll<HTMLButtonElement>('#guided-actions button')].filter(b=>!b.disabled)
   if(actions.length){const button=actions[random(actions.length)],stateBefore=JSON.stringify(f.state),openBefore=[...f.d.querySelectorAll<HTMLDetailsElement>('aside details')].map(p=>p.open)
    button.click();assert.equal(JSON.stringify(f.state),stateBefore,'Guide actions never execute data tools');assert.deepEqual([...f.d.querySelectorAll<HTMLDetailsElement>('aside details')].map(p=>p.open),openBefore,'Guide actions never toggle panels')}
  }
  await tick();f.guide.refresh()
  if(f.state.busy&&!f.card.classList.contains('hidden'))assert.equal(f.step(),'working','Progress must win regardless of route/history/location requests')
  const targets=f.d.querySelectorAll('.guided-target');assert(targets.length<=1,'No duplicate stale highlights')
  for(const target of targets)assert(target.isConnected)
  walkActions++
 }
 f.close()
}
console.log(`Guide ordering: ${navigationCases} disclosure/target combinations and ${walkActions} seeded mixed actions passed, plus cancelled continuation and result-arrival regressions.`)

// Hide/Resume is intentionally different from cancellation: preserve the user's
// pending destination, then continue only after the actual main Finish control.
{
 const f=fixture();await f.press('#guided-setup');await f.action('Review initial processing');await f.headers();await f.action('Continue');await f.action('Continue')
 f.d.querySelector('#processing-crop-draw')!.setAttribute('aria-pressed','true');await f.action('Continue');await f.press('#guided-close');await f.press('#guided-setup');await f.headers();assert.equal(f.d.querySelector('#guided-title')!.textContent,'Finish the processing tool first')
 f.d.querySelector('#processing-crop-draw')!.setAttribute('aria-pressed','false');f.guide.refresh();await f.headers();assert.equal(f.step(),'processing-voxels');f.close()
}

// Users may request one decision control, then choose the other in the main UI.
// Rejecting a first-ever result restores an empty alignment, not stale Accept.
{
 const f=fixture();await f.press('#guided-setup');f.state.fit=imageGeometryAlignment('ct-mri');f.state.fit.automaticRegistration={operation:'global',metric:'nmi'} as any;f.state.fit.proposal={kind:'automatic',matrix:f.state.fit.matrix,params:[],scoreBefore:1,scoreAfter:2};f.guide.refresh();await f.headers();await f.action('Show Accept control');await f.headers()
 f.d.querySelector<HTMLButtonElement>('#reject-refine')!.onclick=()=>{f.state.fit=null};await f.press('#reject-refine');await f.headers();assert.equal(f.step(),'choose','Rejecting a first result clears stale Accept-location instructions');f.close()
}
