import fs from 'node:fs'
import assert from 'node:assert/strict'
import {JSDOM} from 'jsdom'
import {alignmentLayout} from '../apps/align/src/alignmentLayout'
import {installGuidedSetup,type GuideState} from '../apps/align/src/guidedSetup'
import {installWorkflowFocus} from '../apps/align/src/workflowUI'
import {renderGuideText} from '../apps/align/src/guideText'
import {guideNavigationStep} from '../apps/align/src/guideTeaching'
import {imageGeometryAlignment} from '../apps/align/src/geometryInitialization'
const dom=new JSDOM(alignmentLayout('test'),{pretendToBeVisual:true,url:'http://localhost'}),w=dom.window,d=w.document
Object.assign(globalThis,{window:w,document:d});w.HTMLElement.prototype.scrollIntoView=function(){};w.matchMedia=(()=>({matches:false})) as any
installWorkflowFocus()
const state:GuideState={mri:true,ct:true,landmarks:[],selectedId:null,minimumPairs:3,fit:null,busy:false,error:null,viewerReady:true}
const g=installGuidedSetup(()=>state),title=()=>d.querySelector('#guided-title')!.textContent,step=()=>d.querySelector<HTMLElement>('#guided-card')!.dataset.step
const guide=(label:string)=>{const button=[...d.querySelectorAll<HTMLButtonElement>('#guided-actions button')].find(b=>b.textContent===label);assert(button,label);button.click();g.refresh()}
const main=(selector:string)=>{d.querySelector<HTMLButtonElement>(selector)!.click();g.refresh()}
let added=0,set=0,fitted=0,accepted=0,resets=0
const heading=(id:string)=>main('#'+id+' > summary')
d.querySelector<HTMLButtonElement>('#new-pair')!.onclick=()=>{added++;state.landmarks.push({id:added,enabled:true,mri:null,ct:null});state.selectedId=added;const row=d.createElement('div');row.dataset.landmarkId=String(added);for(const modality of ['mri','ct'] as const){const b=d.createElement('button');b.className='set-'+modality;b.onclick=()=>{set++;state.landmarks.at(-1)![modality]=[added,2,3]};row.append(b)}d.querySelector('#landmark-list')!.append(row)}
d.querySelector<HTMLButtonElement>('#fit-side')!.onclick=()=>{fitted++;state.fit={...imageGeometryAlignment('ct-mri'),fittedAt:'native-fit',initializationSource:'landmarks',landmarkSnapshot:structuredClone(state.landmarks)}}
d.querySelector<HTMLButtonElement>('#accept-refine-followup')!.onclick=()=>{accepted++;state.fit!.proposal=null}
d.querySelector<HTMLButtonElement>('#reset-workspace')!.onclick=()=>{resets++}
main('#guided-setup');assert.equal(d.querySelectorAll('#guided-text > p').length,4);assert.deepEqual([...d.querySelectorAll('#guided-text strong')].map(el=>el.textContent),['Optional Initial processing','Landmarks','Manual adjustment','Independent image-based registration']);assert(!d.querySelector('#guided-text')!.textContent!.includes('**'))
const literal=d.createElement('div');renderGuideText(literal,'**Label** <img src=x onerror=alert(1)>\n\nKeep <script>literal</script> text.');assert.equal(literal.querySelectorAll('p').length,2);assert.equal(literal.querySelectorAll('strong').length,1);assert.equal(literal.querySelector('img,script'),null);assert(literal.textContent!.includes('<img'))
guide('Guide me through landmarks');assert.equal(title(),'Open Landmarks');assert.equal(d.querySelector<HTMLDetailsElement>('#landmarks-panel')!.open,false)
const headingElement=d.querySelector<HTMLElement>('#landmarks-panel > summary')!;assert(headingElement.classList.contains('guided-target'));assert(headingElement.classList.contains('guided-pulse'))
const style=d.createElement('style');style.textContent=fs.readFileSync(new URL('../apps/align/src/style.css',import.meta.url),'utf8');d.head.append(style)
assert.equal(w.getComputedStyle(d.querySelector('#landmarks-panel')!).overflow,'hidden');assert.equal(w.getComputedStyle(headingElement).outlineOffset,'-4px');assert(w.getComputedStyle(headingElement).boxShadow.includes('inset'),'Heading frame must remain inside its clipped panel');assert.equal(w.getComputedStyle(headingElement).animation,'guide-heading-attention 600ms ease-in-out 3')
await new Promise(r=>setTimeout(r,1850));assert(headingElement.classList.contains('guided-target'));assert(!headingElement.classList.contains('guided-pulse'))
guide('Show heading');assert(headingElement.classList.contains('guided-pulse'),'Explicit Show replays three flashes');w.matchMedia=(()=>({matches:true})) as any;guide('Show heading');assert(!headingElement.classList.contains('guided-pulse'),'Reduced motion keeps only the steady frame');assert(headingElement.classList.contains('guided-target'));w.matchMedia=(()=>({matches:false})) as any
assert.equal(d.querySelector<HTMLDetailsElement>('#landmarks-panel')!.open,false,'Guide cannot open the panel')
heading('landmarks-panel');assert.equal(step(),'pairs');guide('Show Add landmark pair');assert.equal(added,0);assert(d.querySelector('#new-pair')!.classList.contains('guided-target'))
main('#new-pair');assert.equal(added,1);assert.equal(step(),'point-1-mri');guide('Show Set MRI control');assert.equal(set,0);main('[data-landmark-id="1"] .set-mri');assert.equal(step(),'point-1-ct');main('[data-landmark-id="1"] .set-ct');assert.equal(step(),'pairs')
state.landmarks=[1,2,3].map(id=>({id,enabled:true,mri:[id,2,3],ct:[id,2,3]}));g.refresh();assert.equal(step(),'pairs');assert(!d.querySelector<HTMLDetailsElement>('#landmark-alignment-panel')!.open)
guide('Show landmark alignment');assert.equal(title(),'Close Landmarks');assert.equal(fitted,0);assert(d.querySelector<HTMLDetailsElement>('#landmarks-panel')!.open);heading('landmarks-panel');assert.equal(title(),'Open Landmark alignment');assert(!d.querySelector<HTMLDetailsElement>('#landmark-alignment-panel')!.open);heading('landmark-alignment-panel');assert(d.querySelector('#fit-side')!.classList.contains('guided-target'));assert.equal(fitted,0)
main('#fit-side');assert.equal(fitted,1);assert.equal(title(),'Close Landmark alignment');assert(d.querySelector<HTMLDetailsElement>('#landmark-alignment-panel')!.open);heading('landmark-alignment-panel');assert.equal(title(),'What would you like to do next?')
guide('Inspect alignment');assert.equal(title(),'Open Alignment display');heading('alignment-display-panel');assert.equal(title(),'Inspect alignment');guide('Choose next step');assert.equal(title(),'Close Alignment display');heading('alignment-display-panel');assert.equal(title(),'What would you like to do next?')
guide('Guide automatic refinement');assert.equal(title(),'Open Landmark refinement');heading('followup-panel');assert.equal(step(),'windows');guide('Continue to automatic refinement');assert.equal(step(),'refine')
state.fit!.automaticRegistration={operation:'local',metric:'nmi'} as any;state.fit!.proposal={kind:'automatic',matrix:state.fit!.matrix,params:[],scoreBefore:1,scoreAfter:2};g.refresh();const pending=JSON.stringify(state.fit);guide('Show Accept control');assert.equal(accepted,0);assert.equal(JSON.stringify(state.fit),pending);assert(d.querySelector('#accept-refine-followup')!.classList.contains('guided-target'));main('#accept-refine-followup');assert.equal(accepted,1);assert.equal(title(),'Close Landmark refinement');heading('followup-panel')
guide('Explore nonlinear landmark spline');assert.equal(title(),'Open Landmark alignment');heading('landmark-alignment-panel');assert.equal(title(),'Open Nonlinear landmark spline (advanced)');heading('spline-alignment');assert.equal(step(),'spline');assert(d.querySelector('#spline-smoothing')!.classList.contains('guided-target'))
assert.equal(d.querySelector('#guided-reset-workspace'),null,'Reset belongs only in the main toolbar');assert.equal(resets,0);heading('landmark-alignment-panel')
main('#reset-workspace');assert.equal(resets,1)
main('#guided-restart');guide('Review initial processing');heading('initial-processing-panel');guide('Continue');guide('Continue')
let finished=0;d.querySelector('#processing-crop-draw')!.setAttribute('aria-pressed','true');d.querySelector<HTMLButtonElement>('#processing-stop')!.onclick=()=>{finished++;d.querySelector('#processing-crop-draw')!.setAttribute('aria-pressed','false')}
guide('Continue');assert.equal(title(),'Finish the processing tool first');assert.equal(finished,0);state.busy=true;state.busyKind='processing';g.refresh();assert.equal(step(),'working','Busy progress must override a pending location instruction');state.busy=false;g.refresh();main('#processing-stop');assert.equal(finished,1);assert.equal(title(),'Initial processing · voxel editing')
const modal=d.createElement('div');modal.className='modal';d.body.append(modal);const modalStep={key:'prepare',target:'#preparation-title',title:'Review resolution',text:'Review resolution'};assert.equal(guideNavigationStep(modalStep),modalStep,'Never direct a user behind an open modal to close a sidebar');modal.remove()
g.close();dom.window.close()
console.log('Teaching guide: no proxy actions or automatic disclosures; real close/open/field sequence, nested spline, landmark minimum, proposal decisions, inspection and toolbar-only reset passed.')
