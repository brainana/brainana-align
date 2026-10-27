import assert from 'node:assert/strict'
import {JSDOM} from 'jsdom'
import {alignmentLayout} from '../apps/align/src/alignmentLayout'
import {installGuidedSetup,type GuideState} from '../apps/align/src/guidedSetup'
import {guideNavigationStep} from '../apps/align/src/guideTeaching'
import {installWorkflowFocus} from '../apps/align/src/workflowUI'
import {imageGeometryAlignment} from '../apps/align/src/geometryInitialization'

const dom=new JSDOM(alignmentLayout('test'),{pretendToBeVisual:true,url:'http://localhost'}),w=dom.window,d=w.document
Object.assign(globalThis,{window:w,document:d});w.HTMLElement.prototype.scrollIntoView=function(){};w.matchMedia=(()=>({matches:true})) as any
installWorkflowFocus()
const state:GuideState={mri:true,ct:true,landmarks:[],selectedId:null,minimumPairs:3,fit:null,busy:false,error:null,viewerReady:true}
const windows={mri:{axial:{min:[-3,-4,5],max:[7,8,5],axes:[0,1]}},ct:{}},runs={linear:0,jip:0,itk:0}
const guide=installGuidedSetup(()=>state),q=<T extends HTMLElement>(selector:string)=>{const el=d.querySelector<T>(selector);assert(el,selector);return el},step=()=>q('#guided-card').dataset.step,title=()=>q('#guided-title').textContent
const tick=async()=>{await Promise.resolve();await Promise.resolve();guide.refresh()}
const main=async(selector:string)=>{q(selector).click();await tick()}
const action=async(label:string)=>{const button=[...d.querySelectorAll<HTMLButtonElement>('#guided-actions button')].find(b=>b.textContent===label);assert(button,`${label} at ${step()}`);button.click();await tick()}
const methodIds=['linear-registration','jip-registration','itk-registration']
const panels=()=>methodIds.map(id=>q<HTMLDetailsElement>('#'+id).open)
const choices=()=>['#auto-linear-model','#auto-bins','#jip-metric','#jip-grid','#itk-metric','#itk-bins'].map(selector=>(q<HTMLSelectElement>(selector)).value)
for(const [method,id] of [['linear','auto-linear'],['jip','jip-run'],['itk','itk-run']] as const){const button=q<HTMLButtonElement>('#'+id);button.disabled=false;button.addEventListener('click',()=>{runs[method]++;state.busy=true;state.busyKind='registration'})}
q<HTMLSelectElement>('#auto-bins').value='128';q<HTMLSelectElement>('#jip-metric').value='nc';q<HTMLSelectElement>('#itk-metric').value='local-correlation'
const originalChoices=choices(),originalData=JSON.stringify({state,windows})
assert.deepEqual(panels(),[false,false,false],'All three peer methods start collapsed')
await main('#guided-setup');assert.equal(step(),'choose');assert.deepEqual(panels(),[false,false,false])
await action('Skip landmarks · automatic registration');assert.equal(step(),'teach-open-refine-panel');assert(!q<HTMLDetailsElement>('#refine-panel').open)
await action('Show heading');assert(!q<HTMLDetailsElement>('#refine-panel').open,'Guide-only actions cannot open the outer panel')
await main('#refine-panel > summary');assert.equal(step(),'teach-open-linear-registration');assert.deepEqual(panels(),[false,false,false])
await action('Show heading');await main('#guided-anchor');assert.deepEqual(panels(),[false,false,false],'Show and Follow must not expand the Linear section')
await main('#linear-registration > summary');assert.equal(step(),'initial');assert.equal(title(),'Run linear registration');assert(q('#auto-linear').classList.contains('guided-target'))
for(const name of ['Linear registration','JIP nonlinear registration','ITK nonlinear B-spline'])assert(q('#guided-text').textContent!.includes(name),'Initial instructions name all peer methods')
assert(q('#guided-text').textContent!.includes('B-spline'));assert(q('#guided-text').textContent!.includes('CPU alternative'))
await main('#linear-registration > summary');assert.equal(step(),'teach-open-linear-registration');guide.refresh();await main('#guided-anchor');assert(!q<HTMLDetailsElement>('#linear-registration').open,'A collapsed method stays collapsed on refresh and Follow')
await main('#linear-registration > summary');assert.equal(step(),'initial')

for(const [name,id,target] of [['Linear','linear-registration','linear-defaults-select'],['JIP nonlinear','jip-registration','jip-metric'],['ITK nonlinear','itk-registration','itk-metric']]){
 const before=panels(),data=JSON.stringify({state,windows})
 await action(`Show ${name} registration`)
 if(!before[methodIds.indexOf(id)]){
  assert.equal(step(),'teach-open-'+id);assert.equal(title(),id==='itk-registration'?'Open ITK nonlinear B-spline':'Open '+name+' registration')
  await action('Show heading');await main('#guided-anchor');assert.deepEqual(panels(),before,'A method-location action only teaches its actual heading')
  await main('#'+id+' > summary')
 }
 assert(step()!.startsWith('teach-control-'));assert(q('#'+target).classList.contains('guided-target'))
 assert.deepEqual(runs,{linear:0,jip:0,itk:0});assert.equal(JSON.stringify({state,windows}),data);assert.deepEqual(choices(),originalChoices)
 await main('#'+id+' > summary');assert.equal(step(),'teach-open-'+id);guide.refresh();assert(!q<HTMLDetailsElement>('#'+id).open)
 await action('Show heading');assert(!q<HTMLDetailsElement>('#'+id).open);await main('#'+id+' > summary');assert(q('#'+target).classList.contains('guided-target'))
 await action('Return to instructions');assert.equal(title(),'Run linear registration')
}

// Advanced descendants without IDs need the same native heading teaching.
for(const [target,heading] of [['#auto-range','Additional search settings'],['#itk-bins','ITK settings']]){
 const base={key:'method-setting',target,title:'Review method setting',text:'Review this setting'},before=panels(),data=JSON.stringify({state,windows}),projection=guideNavigationStep(base)
 assert.equal(projection.title,'Open '+heading);const summary=q(projection.target);assert.equal(summary.tagName,'SUMMARY');assert(!summary.parentElement!.hasAttribute('open'))
 assert.deepEqual(panels(),before);assert.equal(JSON.stringify({state,windows}),data);summary.click();assert.equal(guideNavigationStep(base).target,target)
}
assert.equal(JSON.stringify({state,windows}),originalData);assert.deepEqual(choices(),originalChoices)

// A native engine Run owns execution. Observe its completed handler and show
// registration progress even when the guide was teaching another workflow.
for(const [method,id] of [['linear','auto-linear'],['jip','jip-run'],['itk','itk-run']] as const){
 await main('#guided-restart');assert.equal(step(),'choose');const before=panels();await action('Start with manual adjustment');assert.deepEqual(panels(),before)
 const count=runs[method];await main('#'+id);assert.equal(runs[method],count+1);assert.equal(step(),'working');assert.equal(title(),'Alignment in progress')
 state.busy=false;state.busyKind=undefined;guide.refresh();assert.equal(title(),'Run linear registration','The actual engine action selected image-based registration context')
}
await main('#guided-restart');assert.equal(step(),'choose');assert.deepEqual(choices(),originalChoices);assert.deepEqual(windows,{mri:{axial:{min:[-3,-4,5],max:[7,8,5],axes:[0,1]}},ct:{}})
// A real proposal decision returns through visible peer Close instructions,
// then the outer heading. A guide refresh must never perform those clicks.
for(const target of ['#auto-range','#itk-bins']){const nested=q(target).closest('details')!;if(nested.open){nested.querySelector<HTMLElement>(':scope > summary')!.click();await tick()}}
state.fit={...imageGeometryAlignment('ct-mri'),automaticRegistration:{operation:'global',metric:'nmi'} as any,proposal:{kind:'automatic',matrix:imageGeometryAlignment('ct-mri').matrix,params:[],scoreBefore:1,scoreAfter:2}};guide.refresh();assert.equal(step(),'proposal')
q('#accept-refine').addEventListener('click',()=>{state.fit!.proposal=null});await main('#accept-refine')
const decisionState=JSON.stringify({state,windows}),decisionRuns={...runs}
for(const id of methodIds){assert.equal(step(),'teach-close-'+id);const before=panels();await action('Show heading');await main('#guided-anchor');assert.deepEqual(panels(),before);await main('#'+id+' > summary');assert.equal(q<HTMLDetailsElement>('#'+id).open,false)}
assert.equal(step(),'teach-close-refine-panel');await main('#refine-panel > summary');assert.equal(step(),'review');assert.equal(q<HTMLDetailsElement>('#linear-registration').open,false,'Completed proposal hub closes the formerly open Linear peer through its real heading');assert.equal(d.querySelectorAll('aside details[open]').length,0);assert.equal(JSON.stringify({state,windows}),decisionState);assert.deepEqual(runs,decisionRuns)
// Ordinary parent collapse retains its independent nested state. Once hidden,
// those peers must not cause cleanup instructions to reopen the outer section.
await main('#refine-panel > summary');for(const id of methodIds)await main('#'+id+' > summary');assert.deepEqual(panels(),[true,true,true]);await main('#refine-panel > summary');assert.equal(step(),'review');assert.deepEqual(panels(),[true,true,true]);guide.refresh();await main('#guided-anchor');assert.equal(q<HTMLDetailsElement>('#refine-panel').open,false,'Hidden peer state must not reopen its parent at the hub');assert.equal(step(),'review');assert.deepEqual(panels(),[true,true,true]);assert.equal(JSON.stringify({state,windows}),decisionState)
guide.close();dom.window.close()
console.log('Registration method guide: collapsed startup, real outer/nested headings, Linear/JIP/ITK location-only actions, native close/reopen/Follow, advanced headings without IDs, preserved settings/physical windows and main-engine run routing passed.')
