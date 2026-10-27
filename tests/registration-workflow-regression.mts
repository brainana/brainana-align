import {userGuideAction,followGuidePanels} from './helpers/guide-user.mjs'
import assert from 'node:assert/strict'
import {JSDOM} from 'jsdom'
import {alignmentLayout} from '../apps/align/src/alignmentLayout'
import {installWorkflowFocus} from '../apps/align/src/workflowUI'
import {hasLandmarkBasis,installSharedRegionControls} from '../apps/align/src/registrationWorkflow'
import {imageGeometryAlignment} from '../apps/align/src/geometryInitialization'
import {installAutomaticRegistration} from '../apps/align/src/automaticRegistrationUI'
import {guideStep,installGuidedSetup} from '../apps/align/src/guidedSetup'
const dom=new JSDOM('<div id="app"></div>',{pretendToBeVisual:true})
Object.assign(globalThis,{window:dom.window,document:dom.window.document})
document.querySelector('#app')!.innerHTML=alignmentLayout('test')
assert.equal(document.querySelectorAll('.sidebar details[open]').length,0,'Every tool section starts collapsed');
assert.equal(document.querySelectorAll('[data-workflow][aria-pressed=true]').length,0,'A new workspace must not preselect a workflow');
installWorkflowFocus();installSharedRegionControls()
const d=document,click=(id:string)=>d.querySelector<HTMLButtonElement>(id)!.click(),op=d.querySelector<HTMLSelectElement>('#intensity-operation')!,bins=d.querySelector<HTMLSelectElement>('#auto-bins')!
let fit:any=imageGeometryAlignment('ct-mri')
const pairs=[1,2,3].map(id=>({id,mri:[id,0,0],ct:[id,0,0],enabled:true}))
assert.equal(hasLandmarkBasis(fit),false)
fit={...fit,initializationSource:undefined,landmarkSnapshot:pairs};assert.equal(hasLandmarkBasis(fit),true,'Legacy landmark fit stays usable')
assert.equal(hasLandmarkBasis({...fit,initializationSource:'automatic'}),false,'Old global results with inherited pairs are not landmark refinement bases')
const controller=installAutomaticRegistration({getLoaded:()=>({mri:{} as any,ct:{} as any}),getFit:()=>fit,getDirection:()=> 'ct-mri',getWindows:()=>[],activity:{busy:false,begin:async()=>false,phase:async()=>{},end(){}},status(){},onResult(){}})
click('[data-workflow=followup]');assert.equal(op.value,'local');assert(d.querySelector('#followup-panel')!.contains(d.querySelector('#local-auto-linear')));assert(!d.querySelector('#auto-linear-model')!.closest('label')!.classList.contains('hidden'),'Global controls remain permanently visible in their section')
const localBins=d.querySelector<HTMLSelectElement>('#local-auto-bins')!;localBins.value='128';click('[data-workflow=followup]');assert.equal(localBins.value,'128','Repeated tab/guide selection keeps chosen bins')
click('[data-workflow=automatic]');assert.equal(op.value,'global');assert(d.querySelector('#refine-panel')!.contains(d.querySelector('#auto-linear')));assert.equal(bins.value,'64');assert.equal(d.querySelector('#auto-linear-model')!.closest('label')!.classList.contains('hidden'),false)
bins.value='32';click('[data-workflow=followup]');assert.equal(localBins.value,'128');click('[data-workflow=automatic]');assert.equal(bins.value,'32','Each operation retains its own bins')
click('#followup-panel > summary');assert.equal(op.value,'local','Direct accordion opening selects its workflow');assert(d.querySelector('#followup-panel')!.contains(d.querySelector('#local-auto-linear')));click('#refine-panel > summary');assert.equal(op.value,'local','Closing the global section must not select another operation');click('#refine-panel > summary');assert.equal(op.value,'global');
// Real native disclosure clicks must change only their own section, in either
// direction. Sibling state includes the initial landmark/alignment pair.
const sections=[...d.querySelectorAll<HTMLDetailsElement>('.sidebar > details.panel')];assert.deepEqual(sections.map(p=>p.querySelector('summary span')!.textContent),['Initial processing','Landmarks','Landmark alignment','Landmark refinement','Manual adjustment','Image-based registration','Alignment display']);
for(const panel of sections){for(const open of [false,true]){panel.open=open;const before=sections.map(p=>p.open);panel.querySelector<HTMLElement>('summary')!.click();assert.equal(panel.open,!open);for(let i=0;i<sections.length;i++)if(sections[i]!==panel)assert.equal(sections[i].open,before[i],`${panel.id} must not toggle ${sections[i].id}`)}}
// Closing the last section for a workflow clears its highlight, without opening siblings.
for(const panel of sections)panel.open=false;
click('#manual-panel > summary');assert.equal(d.querySelector('[data-workflow=manual]')!.getAttribute('aria-pressed'),'true');click('#manual-panel > summary');await Promise.resolve();assert.equal(d.querySelector('[data-workflow=manual]')!.getAttribute('aria-pressed'),'false');assert.equal(sections.filter(p=>p.open).length,0);
// Both complete forms remain in place across every section/workflow interaction.
const follow=d.querySelector<HTMLDetailsElement>('#followup-panel')!,global=d.querySelector<HTMLDetailsElement>('#refine-panel')!;follow.open=global.open=true;
const localNode=d.querySelector('#local-auto-linear')!,globalNode=d.querySelector('#auto-linear')!;for(const route of ['automatic','followup','manual','landmarks']){click(`[data-workflow=${route}]`);assert(follow.contains(localNode));assert(global.contains(globalNode));assert.equal(localBins.value,'128');assert.equal(bins.value,'32')}
assert(d.querySelector('#landmark-alignment-panel')!.contains(d.querySelector('#spline-alignment')));assert(!follow.contains(d.querySelector('#spline-alignment')));assert.equal(d.querySelectorAll('[data-registration-focus]').length,0);
// Separate eligibility messages and local review controls work without moving DOM.
fit={...fit,proposal:null};controller.refresh();assert.equal(d.querySelector<HTMLButtonElement>('#local-auto-linear')!.disabled,false);assert.equal(d.querySelector<HTMLButtonElement>('#run-landmark-spline')!.disabled,true);assert(d.querySelector('#spline-availability')!.textContent!.includes('At least 12'));
let accepted=0,rejected=0;d.querySelector('#accept-refine')!.addEventListener('click',()=>{accepted++;fit.proposal=null});d.querySelector('#reject-refine')!.addEventListener('click',()=>{rejected++;fit.proposal=null});
for(const action of ['accept','reject']){fit.proposal={kind:'automatic'};fit.automaticRegistration={operation:'local'};if(action==='reject'){delete fit.automaticRegistration;fit.proposal.kind=undefined}controller.refresh();assert(!d.querySelector('#refine-actions-followup')!.classList.contains('hidden'));assert(d.querySelector('#refine-actions')!.classList.contains('hidden'));click(`#${action}-refine-followup`);controller.refresh();assert(d.querySelector('#refine-actions-followup')!.classList.contains('hidden'))}assert.equal(accepted,1);assert.equal(rejected,1);delete fit.automaticRegistration;
// Spline decisions stay beside the landmark spline, never in image refinement.
for(const action of ['accept','reject']){fit.proposal={kind:'spline'};fit.nonlinear={} as any;controller.refresh();assert(!d.querySelector('#spline-actions')!.classList.contains('hidden'));assert(d.querySelector('#refine-summary')!.classList.contains('hidden'));assert(d.querySelector('#spline-result-summary')!.textContent!.includes('Spline landmark RMS'));assert(d.querySelector('#refine-actions-followup')!.classList.contains('hidden'));assert(d.querySelector('#refine-actions')!.classList.contains('hidden'));click(`#${action}-spline`);controller.refresh();assert(d.querySelector('#spline-actions')!.classList.contains('hidden'))}assert.equal(accepted,2);assert.equal(rejected,2);delete fit.nonlinear;
// Restored proposal scores come from their own FitState, not the other form's text.
const savedFit=fit;fit={...fit,automaticRegistration:{operation:'local',metric:'nmi',scoreBefore:1.234,scoreAfter:1.456,candidatesEvaluated:12},proposal:{kind:'automatic'}};d.querySelector('#refine-summary')!.textContent='Wrong global score 999';controller.refresh();assert(d.querySelector('#local-refine-summary')!.textContent!.includes('1.23400'));assert(!d.querySelector('#local-refine-summary')!.textContent!.includes('999'));fit={...savedFit,automaticRegistration:{operation:'global',metric:'mi',scoreBefore:7,scoreAfter:8,candidatesEvaluated:20},proposal:null};controller.refresh();assert(d.querySelector('#refine-summary')!.textContent!.includes('7.00000'));fit=savedFit;controller.refresh();
let editing=false,clears=0;const canonical=d.querySelector('#define-windows')!
canonical.addEventListener('click',()=>{editing=!editing;canonical.setAttribute('aria-pressed',String(editing));canonical.textContent=editing?'Finish defining windows':'Define windows'})
d.querySelector('#clear-windows')!.addEventListener('click',()=>{clears++;d.querySelector('#window-summary')!.textContent='Cleared'})
click('#define-windows-followup');assert.equal(editing,true);assert(d.querySelector('#followup-panel')!.contains(d.querySelector('#define-windows-followup')))
click('[data-workflow=automatic]');assert.equal(editing,true,'Switching workflow does not clear or stop region capture');click('#define-windows');await Promise.resolve();assert.equal(d.querySelector('#define-windows')!.getAttribute('aria-pressed'),'false')
const target=d.querySelector<HTMLSelectElement>('#window-target')!;target.value='mri';target.dispatchEvent(new dom.window.Event('change'));assert.equal(d.querySelector<HTMLSelectElement>('#window-target')!.value,'mri')
click('#clear-windows');await Promise.resolve();assert.equal(clears,1);assert.equal(d.querySelector('#window-summary')!.textContent,'Cleared')
const state:any={mri:true,ct:true,viewerReady:true,fit:null,landmarks:[],selectedId:null,minimumPairs:3,busy:false,error:null}
assert.equal(guideStep({...state,fit:{...fit,proposal:{kind:'spline'}}},'spline').target,'#spline-actions','Spline guide review stays in landmark alignment');
const guide=installGuidedSetup(()=>state);const originalRefresh=guide.refresh;guide.refresh=()=>{originalRefresh();followGuidePanels(d,originalRefresh)};click('#guided-setup')
const action=(label:string)=>userGuideAction(d,guide.refresh,label)
action('Review initial processing');assert.equal(d.querySelector('#guided-title')!.textContent,'Initial processing · image display');assert(d.querySelector<HTMLDetailsElement>('#initial-processing-panel')!.open);action('Skip remaining processing');
assert.equal(d.querySelector('#guided-title')!.textContent,'Initial processing complete — choose registration');action('Use image-based registration');action('Guide optimization regions');assert.equal(d.querySelector('#guided-title')!.textContent,'Optional: restrict image-based registration');assert.equal(op.value,'global')
click('#guided-close');click('#guided-resume');assert.equal(op.value,'global','Resuming global region guide must not select landmark refinement')
action('Continue to image-based registration');assert.equal(op.value,'global')
state.fit=imageGeometryAlignment('ct-mri');guide.refresh();const globalState={...state,landmarks:pairs,fit:{...fit,initializationSource:'automatic'}};assert.equal(guideStep(globalState,'landmarks').key,'pairs','A global result must not be described as a completed landmark fit');assert(![...d.querySelectorAll('#guided-actions button')].some(b=>b.textContent==='Guide automatic refinement'),'Nonlandmark starts do not offer landmark refinement')
state.fit={...fit,initializationSource:'landmarks'};guide.refresh();action('Explore nonlinear landmark spline');assert.equal(d.querySelector('#guided-title')!.textContent,'Optional nonlinear landmark spline');assert(d.querySelector('#landmark-alignment-panel')!.contains(d.querySelector('#spline-settings')));assert(d.querySelector<HTMLDetailsElement>('#landmark-alignment-panel')!.open,'Spline guide reveals the outer section');assert(d.querySelector<HTMLDetailsElement>('#spline-alignment')!.open,'Spline guide reveals the inner section');
action('Continue to landmark refinement');assert.equal(op.value,'local');assert(d.querySelector('#define-windows-followup')!.classList.contains('guided-target'))
action('Continue to automatic refinement');action('Adjust manually first');action('Guide automatic registration');assert.equal(op.value,'global');click('#guided-back');guide.refresh();assert(d.querySelector('#manual-panel')!.open,'Back reopens the actual manual workflow');action('Continue to refinement options');action('Continue to automatic refinement');action('Skip refinement');click('#guided-back');assert.equal(op.value,'local');assert(d.querySelector('#followup-panel')!.contains(d.querySelector('#local-auto-linear')),'Back keeps local controls in landmark follow-up');
// Repainting the same guide target must respect a manually collapsed section.
guide.refresh=originalRefresh;
click('#guided-restart');state.fit=null;state.landmarks=[{id:7,mri:null,ct:null,enabled:true}];state.selectedId=7;
const list=d.querySelector('#landmark-list')!;list.innerHTML='<div class="landmark-row" data-landmark-id="7"><button class="set-mri">Set</button><button class="set-ct">Set</button></div>';action('Guide me through landmarks');
const landmarks=d.querySelector<HTMLDetailsElement>('#landmarks-panel')!;assert(landmarks.open);const siblings=sections.filter(p=>p!==landmarks).map(p=>p.open);click('#landmarks-panel > summary');assert(!landmarks.open);list.innerHTML=list.innerHTML;await Promise.resolve();guide.refresh();assert(!landmarks.open,'Guide refresh cannot reopen the section just collapsed by the user');assert.deepEqual(sections.filter(p=>p!==landmarks).map(p=>p.open),siblings);click('#guided-anchor');assert(!landmarks.open,'Show control must teach the heading without opening its panel');assert.equal(d.querySelector<HTMLElement>('#guided-card')!.dataset.step,'teach-open-landmarks-panel');click('#landmarks-panel > summary');guide.refresh();assert(landmarks.open);
for(const panel of sections){panel.open=false;const before=sections.map(p=>p.open);panel.querySelector<HTMLElement>('summary')!.click();await Promise.resolve();for(let i=0;i<sections.length;i++)if(sections[i]!==panel)assert.equal(sections[i].open,before[i],`Guide must not open ${sections[i].id} when ${panel.id} is disclosed`)}
guide.close();dom.window.close()
console.log('Registration workflows: separate global/local tabs, legacy landmark eligibility, shared region actions/state, per-operation bins, guide global windows/resume and landmark-only spline/refinement passed.')
