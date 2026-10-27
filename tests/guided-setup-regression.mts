import {userGuideAction} from './helpers/guide-user.mjs'
import {getPreferences} from '../apps/align/src/preferences'
import assert from 'node:assert/strict'
import { createRequire } from 'node:module'
import {guideStep,installGuidedSetup,type GuideState} from '../apps/align/src/guidedSetup.ts'
import {imageGeometryAlignment} from '../apps/align/src/geometryInitialization.ts'
import {fitSummaryHtml} from '../apps/align/src/fitReview.ts'
import {createRegistrationArtifacts} from '../apps/align/src/exportArtifacts.ts'
import {createSessionPayload,parseSessionPayload} from '../apps/align/src/sessionPersistence.ts'
const require=createRequire(new URL('../apps/align/package.json',import.meta.url));const {JSDOM}=require('jsdom')
for(const kind of ['processing','preparation','display','export'] as const){const step=guideStep({mri:true,ct:true,landmarks:[],selectedId:null,minimumPairs:3,fit:null,busy:true,busyKind:kind,error:null,viewerReady:true},'processing');assert(!step.title.includes('Alignment'),'Non-registration tasks must not be called alignment');assert(step.text.includes('registration'));}assert.equal(guideStep({mri:true,ct:true,landmarks:[],selectedId:null,minimumPairs:3,fit:null,busy:true,busyKind:'processing',error:null,viewerReady:true},'processing').title,'Preprocessing in progress');
let s:GuideState={mri:false,ct:false,landmarks:[],selectedId:null,minimumPairs:3,fit:null,busy:false,error:null,viewerReady:true}
assert.equal(guideStep(s,'choose').key,'mri');s.mri=true;assert.equal(guideStep(s,'choose').key,'ct');s.ct=true
assert.equal(guideStep(s,'choose').key,'choose');assert.equal(guideStep(s,'choose').target,'#guided-setup','Method choice must not focus landmark fitting');assert.equal(guideStep(s,'landmarks').key,'pairs')
s.landmarks=[{id:1,mri:null,ct:null,enabled:true}];s.selectedId=1
assert.equal(guideStep(s,'landmarks').key,'point-1-mri');s.landmarks[0].mri=[0,0,0];assert.equal(guideStep(s,'landmarks').key,'point-1-ct')
s.landmarks=[1,2,3].map(id=>({id,mri:[id,0,0] as [number,number,number],ct:[id,0,0] as [number,number,number],enabled:true}))
assert.equal(guideStep(s,'landmarks').key,'pairs');s.minimumPairs=4;assert.equal(guideStep(s,'landmarks').key,'pairs')
assert.equal(guideStep(s,'automatic').key,'initial');assert.equal(guideStep(s,'manual').key,'initial')
const fit=imageGeometryAlignment('ct-mri');s.fit=fit
assert.equal(guideStep(s,'automatic').key,'initial-review');assert.ok(fitSummaryHtml(fit).includes('not available'))
const artifacts=createRegistrationArtifacts({appVersion:'test',fit,landmarks:[]});const json=JSON.parse(await artifacts[0].blob.text());assert.equal(json.rms_mm,null);assert.equal(json.initialization_source,'image-geometry')
const session=createSessionPayload({appVersion:'test',loaded:{mri:null,ct:null},landmarks:[],selectedId:null,nextId:1,direction:'ct-mri',fit,optimizationWindows:{mri:{},ct:{}}});assert.equal(parseSessionPayload(session).fit?.initializationSource,'image-geometry')
s.fit!.proposal={matrix:fit.matrix,params:[0,0,0,0,0,0],scoreBefore:1,scoreAfter:1.1};assert.equal(guideStep(s,'refinement').key,'proposal');assert.equal(guideStep(s,'export').key,'proposal')
s.fit=null;s.landmarks=[];s.mri=false;s.ct=false
const dom=new JSDOM('<details id="initial-processing-panel"><summary>Initial processing</summary><button id="processing-stop">Finish processing tool</button></details><select id="intensity-operation"><option value="global">Global</option><option value="local">Local</option></select><button id="guided-setup">Guide</button><label><input id="mri-file" type="file"></label><label><input id="ct-file" type="file"></label><button id="fit-side">Fit</button><button id="use-image-geometry">Start</button><button id="new-pair">New</button><div id="browser-compatibility"></div>',{pretendToBeVisual:true})
Object.assign(globalThis,{window:dom.window,document:dom.window.document})
const controller=installGuidedSetup(()=>s);const d=dom.window.document
const click=(text:string)=>userGuideAction(d,controller.refresh,text)
d.querySelector<HTMLButtonElement>('#guided-setup')!.click();assert.equal(d.querySelector('#guided-title')!.textContent,'Load your MRI')
s.error='Loading failed';controller.refresh();assert.ok(d.querySelector('#guided-error')!.textContent!.includes('Loading failed'));assert.equal(d.querySelector('#guided-title')!.textContent,'Load your MRI')
s.mri=true;s.error=null;controller.refresh();assert.equal(d.querySelector('#guided-title')!.textContent,'Load your CT')
s.ct=true;controller.refresh();click('Review initial processing');assert.equal(d.querySelector('#guided-title')!.textContent,'Initial processing · image display');s.busy=true;s.busyKind='processing';controller.refresh();assert.equal(d.querySelector('#guided-title')!.textContent,'Preprocessing in progress');s.busy=false;controller.refresh();const processingState=JSON.stringify(s);for(const stage of ['masking','cropping','voxel editing']){click('Continue');assert.equal(d.querySelector('#guided-title')!.textContent,'Initial processing · '+stage)}click('Continue to registration choices');assert.equal(d.querySelector('#guided-title')!.textContent,'Initial processing complete — choose registration');assert.equal(JSON.stringify(s),processingState,'Guide continue cannot change images or start registration');assert(![...d.querySelectorAll('#guided-actions button')].some(b=>b.textContent==='Review initial processing'),'Do not loop back to initial-processing chooser');d.querySelector<HTMLButtonElement>('#guided-back')!.click();assert.equal(d.querySelector('#guided-title')!.textContent,'Initial processing · voxel editing');click('Continue to registration choices');click('Start with landmarks');assert.equal(d.querySelector('#guided-title')!.textContent,'Create your first landmark pair')
click('Use another registration method');click('Skip landmarks · automatic registration');assert.equal((d.querySelector('#intensity-operation') as HTMLSelectElement).value,'global');assert.equal(d.querySelector('#guided-title')!.textContent,'Run linear registration')
s.fit={...imageGeometryAlignment('ct-mri'),initializationSource:'landmarks',landmarkSnapshot:s.landmarks.length?s.landmarks:[1,2,3].map(id=>({id,mri:[id,0,0],ct:[id,0,0],enabled:true}))};controller.refresh();assert.equal(d.querySelector('#guided-title')!.textContent,'Image-based registration options')
for(const decision of ['accept','reject']){click('Guide automatic refinement');assert.equal((d.querySelector('#intensity-operation') as HTMLSelectElement).value,'local');click('Continue to automatic refinement');s.fit!.proposal={matrix:s.fit!.matrix,params:[0,0,0,0,0,0],scoreBefore:1,scoreAfter:1.1};controller.refresh();assert.equal(d.querySelector('#guided-title')!.textContent,'Review the refinement proposal');s.fit!.proposal=null;controller.refresh();assert.equal(d.querySelector('#guided-title')!.textContent,'What would you like to do next?',decision);assert([...d.querySelectorAll('#guided-actions button')].some(b=>b.textContent==='Guide manual adjustment'));assert([...d.querySelectorAll('#guided-actions button')].some(b=>b.textContent==='Guide automatic registration'));click('Guide automatic registration');assert.equal((d.querySelector('#intensity-operation') as HTMLSelectElement).value,'global');d.querySelector<HTMLButtonElement>('#guided-back')!.click()}click('Save or export');assert.equal(d.querySelector('#guided-title')!.textContent,'Save your work');const beforeFinish=JSON.stringify(s);click('Finish guide');assert(d.querySelector('#guided-card')!.classList.contains('hidden'),'Finish closes the guide in one click');assert(d.querySelector('#guided-resume')!.classList.contains('hidden'),'Finishing is not pausing');assert.equal(d.querySelector('#guided-setup')!.textContent,'Guide');assert.equal(d.querySelectorAll('.guided-target').length,0);assert.equal(JSON.stringify(s),beforeFinish,'Finishing preserves images, landmarks and alignment')
assert.ok(d.querySelector('#guided-card')!.classList.contains('hidden'));assert.equal(d.querySelectorAll('.guided-target').length,0)
d.querySelector<HTMLButtonElement>('#guided-setup')!.click();assert.equal(d.querySelector('#guided-title')!.textContent,'How would you like to begin?')
d.dispatchEvent(new dom.window.KeyboardEvent('keydown',{key:'Escape'}));assert.ok(d.querySelector('#guided-card')!.classList.contains('hidden'))
assert.ok(s.fit,'ending guide must preserve alignment')
// Dismissal preserves the route; state changes while hidden are recognized on resume.
s.fit=null;s.landmarks=[{id:1,mri:[1,2,3],ct:null,enabled:true}];s.selectedId=1;s.minimumPairs=3
const start=d.querySelector<HTMLButtonElement>('#guided-setup')!,close=d.querySelector<HTMLButtonElement>('#guided-close')!,resume=d.querySelector<HTMLButtonElement>('#guided-resume')!
start.click();click('Guide me through landmarks');assert.equal(d.querySelector('#guided-title')!.textContent,'Landmark 1: set CT')
close.click();assert.equal(start.textContent,'Resume guide');assert.ok(!resume.classList.contains('hidden'));assert.equal(d.querySelectorAll('.guided-target').length,0)
s.landmarks[0].ct=[1,2,3];resume.click();assert.equal(d.querySelector('#guided-title')!.textContent,'Add or review landmark pairs')
let addClicks=0,fitClicks=0
const add=d.querySelector<HTMLButtonElement>('#new-pair')!,apply=d.querySelector<HTMLButtonElement>('#fit-side')!
add.addEventListener('click',()=>{addClicks++;s.landmarks.push({id:2,mri:null,ct:null,enabled:true});s.selectedId=2})
click('Add landmark pair');assert.equal(addClicks,1);assert.equal(d.querySelector('#guided-title')!.textContent,'Landmark 2: set MRI')
assert.equal(d.querySelector('#guided-actions button.primary')!.textContent,'Show Set MRI control');assert(![...d.querySelectorAll('#guided-actions button')].some(b=>b.textContent==='Apply landmark transformation'))
s.landmarks=[1,2,3].map(id=>({id,mri:[id,0,0],ct:[id,0,0],enabled:true}));controller.refresh()
assert.equal(d.querySelector('#guided-title')!.textContent,'Add or review landmark pairs')
assert.equal(d.querySelector('#guided-actions button.primary'),null,'Meeting the minimum must not promote Apply as a primary action')
assert.deepEqual([...d.querySelectorAll('#guided-actions button')].map(b=>b.textContent),['Show Add landmark pair','Show landmark alignment','Use another registration method']);assert.equal(d.querySelectorAll<HTMLButtonElement>('#guided-actions button')[1].disabled,false)
apply.addEventListener('click',()=>{fitClicks++;s.error='Degenerate landmarks'})
click('Apply landmark transformation');assert.equal(fitClicks,1);assert.equal(d.querySelector('#guided-title')!.textContent,'Add or review landmark pairs');assert.ok(d.querySelector('#guided-error')!.textContent!.includes('Degenerate'))
s.minimumPairs=4;controller.refresh();assert.equal(d.querySelector('#guided-title')!.textContent,'Add or review landmark pairs');s.minimumPairs=3;controller.refresh()
// Dragged coordinates survive refresh/advancement, with keyboard movement and viewport clamping.
const card=d.querySelector<HTMLElement>('#guided-card')!,handle=d.querySelector<HTMLElement>('#guided-move')!
const pointer=(type:string,x:number,y:number,id=7)=>{const e=new dom.window.Event(type,{bubbles:true,cancelable:true});Object.assign(e,{clientX:x,clientY:y,pointerId:id,button:0});handle.dispatchEvent(e)}
const initialX=parseFloat(card.style.left),initialY=parseFloat(card.style.top)
pointer('pointerdown',100,100);pointer('pointermove',120,125);pointer('pointerup',120,125)
assert.equal(parseFloat(card.style.left),initialX+20);assert.equal(parseFloat(card.style.top),initialY+25)
controller.refresh();assert.equal(parseFloat(card.style.left),initialX+20)
handle.dispatchEvent(new dom.window.KeyboardEvent('keydown',{key:'ArrowRight',bubbles:true,cancelable:true}));assert.equal(parseFloat(card.style.left),initialX+30)
pointer('pointerdown',0,0);pointer('pointermove',-10000,-10000);pointer('pointercancel',0,0);assert.equal(card.style.left,'12px');assert.equal(card.style.top,'12px')
pointer('pointermove',100,100);assert.equal(card.style.left,'12px','canceled drag must stop')
Object.defineProperty(dom.window,'innerWidth',{configurable:true,value:400});Object.defineProperty(dom.window,'innerHeight',{configurable:true,value:400})
pointer('pointerdown',0,0);pointer('pointermove',10000,10000);pointer('pointerup',0,0);assert.equal(card.style.left,'38px');assert.equal(card.style.top,'68px')
close.click();start.click();assert.equal(d.querySelector('#guided-title')!.textContent,'Add or review landmark pairs');assert.equal(card.style.left,'38px')
d.querySelector<HTMLButtonElement>('#guided-anchor')!.click();assert.equal(card.dataset.side,'automatic')
s.fit={...imageGeometryAlignment('ct-mri'),initializationSource:'landmarks',landmarkSnapshot:structuredClone(s.landmarks),fittedAt:'new-fit'};s.error=null;controller.refresh();assert.equal(d.querySelector('#guided-title')!.textContent,'What would you like to do next?')
d.querySelector<HTMLButtonElement>('#guided-restart')!.click();assert.equal(d.querySelector('#guided-title')!.textContent,'How would you like to begin?')
controller.close();dom.window.close()
console.log('Guided setup regressions passed: load success/error waiting, existing data, point pairing, model minimum, skips, automatic prerequisites, proposal review, movable/clamped guide, pause/resume with state advancement, primary add/apply actions, restart/Escape cleanup, geometry initialization/session/export and unavailable RMS')

// A persisted walkthrough is not a restored registration session.
for(const savedRoute of ['landmarks','automatic','manual','review','windows','refinement','export','done']){
 const fresh=new JSDOM('<button id="guided-setup">Guided setup</button><input id="mri-file" type="file"><input id="ct-file" type="file"><button id="fit-side">Fit</button><button id="new-pair">New</button><select id="intensity-operation"><option value="global">Global</option><option value="local">Local</option></select>',{pretendToBeVisual:true})
 Object.assign(globalThis,{window:fresh.window,document:fresh.window.document})
 const writes:any[]=[];fresh.window.brainanaPreferences={read:async()=>({route:savedRoute,history:['choose','landmarks'],paused:true,manualPosition:{x:25,y:25}}),write:async(_key:string,value:any)=>{writes.push(value)}}
 const state:GuideState={mri:false,ct:false,landmarks:[],selectedId:null,minimumPairs:3,fit:null,busy:false,error:null,viewerReady:true}
 const guide=installGuidedSetup(()=>state);if(savedRoute==='refinement'){state.mri=true;state.ct=true}fresh.window.dispatchEvent(new fresh.window.Event('align-preferences-ready'));await new Promise(r=>setTimeout(r,0))
 assert.equal(fresh.window.document.querySelector('#guided-setup')!.textContent,'Guided setup',savedRoute)
 state.mri=false;state.ct=false;fresh.window.document.querySelector<HTMLButtonElement>('#guided-setup')!.click();assert.equal(fresh.window.document.querySelector('#guided-title')!.textContent,'Load your MRI')
 state.mri=true;guide.refresh();assert.equal(fresh.window.document.querySelector('#guided-title')!.textContent,'Load your CT')
 state.ct=true;guide.refresh();assert.equal(fresh.window.document.querySelector('#guided-title')!.textContent,'How would you like to begin?',savedRoute)
 assert([...fresh.window.document.querySelectorAll('#guided-actions button')].some(b=>b.textContent==='Guide me through landmarks'))
 assert(writes.some(v=>v.route==='choose'&&!v.paused),'stale saved progress reset')
 guide.close();fresh.window.close()
}
console.log('Fresh launch with every saved guide route: no false Resume label, MRI/CT loading followed by method choice; same-session pause/resume preserved.')

// Direct tool interaction and guide actions share completion rules. Model/parameter
// edits do not complete a multi-action registration or require an extra Next click.
{
 const fresh=new JSDOM('<button id="guided-setup">Guide</button><button data-workflow="landmarks">Landmarks</button><button data-workflow="manual">Manual</button><button data-workflow="automatic">Automatic</button><button id="new-pair">New</button><button id="fit-side">Fit</button><button id="define-windows" aria-pressed="false">Define windows</button><button id="auto-linear">Run</button><button id="accept">Accept</button><button id="reject">Reject</button><select id="intensity-operation"><option value="global">Global</option><option value="local">Local</option></select><input id="parameter" value="1"><div id="manual-panel"></div><div id="fit-summary"></div>',{pretendToBeVisual:true})
 Object.assign(globalThis,{window:fresh.window,document:fresh.window.document})
 const state:GuideState={mri:true,ct:true,landmarks:[],selectedId:null,minimumPairs:3,fit:{...imageGeometryAlignment('ct-mri'),initializationSource:'landmarks',landmarkSnapshot:[1,2,3].map(id=>({id,mri:[id,0,0],ct:[id,0,0],enabled:true}))},busy:false,error:null,viewerReady:true}
 const g=installGuidedSetup(()=>state),doc=fresh.window.document
 const title=()=>doc.querySelector('#guided-title')!.textContent
 const press=async(selector:string)=>{doc.querySelector<HTMLButtonElement>(selector)!.click();await Promise.resolve()}
 const action=async(text:string)=>{userGuideAction(doc,g.refresh,text);await Promise.resolve()}
 const op=doc.querySelector<HTMLSelectElement>('#intensity-operation')!
 await press('#guided-setup')
 await press('[data-workflow="manual"]');assert.equal(title(),'Adjust the overlay manually')
 await action('Continue to refinement options');assert.equal(title(),'Optional: restrict refinement','guide-generated operation changes do not skip windows')
 const windows=doc.querySelector<HTMLButtonElement>('#define-windows')!
 windows.addEventListener('click',()=>windows.setAttribute('aria-pressed',windows.getAttribute('aria-pressed')==='true'?'false':'true'))
 await press('#define-windows');assert.equal(title(),'Draw optimization windows')
 const parameter=doc.querySelector<HTMLInputElement>('#parameter')!;parameter.value='2';parameter.dispatchEvent(new fresh.window.Event('input',{bubbles:true}));await Promise.resolve();assert.equal(title(),'Draw optimization windows')
 await press('#define-windows');assert.equal(title(),'Try automatic refinement','finishing window editing advances without Next')
 await action('Adjust manually first');await action('Continue to refinement options');await press('#define-windows');await action('Continue to automatic refinement');assert.equal(title(),'Try automatic refinement','guide-proxied finish does not queue a reverse transition')
 op.value='global';op.dispatchEvent(new fresh.window.Event('change',{bubbles:true}));await Promise.resolve();assert.equal(title(),'Image-based registration options');assert.equal(op.value,'global')
 const makeProposal=()=>{state.fit!.proposal={kind:'automatic',matrix:state.fit!.matrix,params:[0,0,0,0,0,0],scoreBefore:1,scoreAfter:1.1}}
 for(const decision of ['accept','reject']){
  makeProposal();g.refresh();assert.equal(title(),'Review the refinement proposal')
  const b=doc.querySelector<HTMLButtonElement>('#'+decision)!;b.onclick=()=>{state.fit!.proposal=null}
  await press('#'+decision);assert.equal(title(),'What would you like to do next?','global '+decision+' advances immediately')
  await press('[data-workflow="automatic"]')
 }
 await press('#define-windows');assert.equal(title(),'Draw optimization windows');await press('#define-windows');assert.equal(title(),'Image-based registration options','global window editing preserves global operation')
 await press('[data-workflow="landmarks"]');assert.equal(title(),'Create your first landmark pair')
 state.landmarks=[1,2,3].map(id=>({id,mri:[id,0,0],ct:[id,0,0],enabled:true}));g.refresh()
 doc.querySelector<HTMLButtonElement>('#fit-side')!.onclick=()=>{state.fit={...imageGeometryAlignment('ct-mri'),initializationSource:'landmarks',landmarkSnapshot:structuredClone(state.landmarks),fittedAt:'direct-fit'}}
 await press('#fit-side');assert.equal(title(),'What would you like to do next?')
 state.landmarkModel='affine';state.minimumPairs=4;parameter.dispatchEvent(new fresh.window.Event('change',{bubbles:true}));await Promise.resolve();assert.equal(title(),'What would you like to do next?','Changing a parameter must not reopen a completed workflow from the next-step hub')
 state.landmarkModel='rigid';state.minimumPairs=3;g.refresh();assert.equal(title(),'What would you like to do next?')
 for(const decision of ['accept','reject']){
  makeProposal();g.refresh();assert.equal(title(),'Review the refinement proposal')
  state.fit!.proposal!.kind='spline';g.refresh();assert.equal(title(),'Review the nonlinear spline proposal','same-key proposal text is refreshed')
  await press('#'+decision);assert.equal(title(),'What would you like to do next?','spline '+decision+' advances')
 }
 g.close();await press('[data-workflow="manual"]');await press('#guided-resume');assert.equal(title(),'What would you like to do next?','hidden guide does not follow unrelated tool clicks')
 g.close();fresh.window.close()
}
console.log('Direct workflow/operation synchronization, window completion, parameter waiting, global/spline decisions and internal-click feedback regressions passed.')

// Actual guide placement against controls and the currently required image row.
{
 const fresh=new JSDOM('<button id="guided-setup">Guide</button><button id="new-pair">New</button><button id="define-windows">Define</button><select id="intensity-operation"><option value="local">Local</option></select><div data-landmark-id="1"><button class="set-mri">MRI</button><button class="set-ct">CT</button></div><div class="view-card" data-modality="mri"></div><div class="view-card" data-modality="ct"></div>',{pretendToBeVisual:true})
 Object.assign(globalThis,{window:fresh.window,document:fresh.window.document})
 Object.defineProperty(fresh.window,'innerWidth',{value:1200,configurable:true});Object.defineProperty(fresh.window,'innerHeight',{value:900,configurable:true})
 const rect=(left:number,top:number,width:number,height:number)=>({left,top,width,height,right:left+width,bottom:top+height,x:left,y:top,toJSON(){}})
 const setRect=(el:Element,r:any)=>Object.defineProperty(el,'getBoundingClientRect',{value:()=>r,configurable:true})
 const doc=fresh.window.document
 setRect(doc.querySelector('.set-mri')!,rect(20,550,260,30));setRect(doc.querySelector('.set-ct')!,rect(20,610,260,30));setRect(doc.querySelector('#define-windows')!,rect(20,650,280,30))
 setRect(doc.querySelector('[data-modality="mri"]')!,rect(320,150,860,240));setRect(doc.querySelector('[data-modality="ct"]')!,rect(320,410,860,230))
 const state:GuideState={mri:true,ct:true,landmarks:[{id:1,mri:null,ct:null,enabled:true}],selectedId:1,minimumPairs:3,fit:null,busy:false,error:null,viewerReady:true}
 const g=installGuidedSetup(()=>state);const card=doc.querySelector<HTMLElement>('#guided-card')!;setRect(card,rect(0,0,350,300))
 doc.querySelector<HTMLButtonElement>('#guided-setup')!.click();[...doc.querySelectorAll<HTMLButtonElement>('#guided-actions button')].find(b=>b.textContent==='Guide me through landmarks')!.click()
 const clear=(selector:string)=>{const r=doc.querySelector(selector)!.getBoundingClientRect(),x=parseFloat(card.style.left),y=parseFloat(card.style.top);assert(x+350<=r.left-8||x>=r.right+8||y+300<=r.top-8||y>=r.bottom+8,selector+' must not be covered')}
 clear('.set-mri');clear('[data-modality="mri"]');assert(doc.querySelector('.set-mri')!.classList.contains('guided-pulse'))
 const move=doc.querySelector<HTMLElement>('#guided-move')!,pointer=(type:string,x:number,y:number,id=11)=>{const e=new fresh.window.Event(type,{bubbles:true,cancelable:true});Object.assign(e,{clientX:x,clientY:y,pointerId:id,button:0,isPrimary:true});move.dispatchEvent(e)}
 const beforeDrag={x:parseFloat(card.style.left),y:parseFloat(card.style.top)};pointer('pointerdown',100,100);pointer('pointermove',100+650-beforeDrag.x,100+250-beforeDrag.y);pointer('pointerup',0,0,99);pointer('pointermove',100+660-beforeDrag.x,100+260-beforeDrag.y);pointer('pointerup',0,0)
 assert.equal(card.style.left,'660px','Manual drag crosses sidebar into the protected MRI pane');assert.equal(card.style.top,'260px');assert.equal(card.dataset.side,'floating');assert(doc.body.classList.contains('guide-manual-floating'));g.refresh();fresh.window.dispatchEvent(new fresh.window.Event('resize'));doc.dispatchEvent(new fresh.window.Event('scroll'));assert.equal(card.style.left,'660px','Timer, resize and scroll cannot snap manual placement back')
 const initial={x:card.style.left,y:card.style.top};state.landmarks[0].mri=[1,2,3];g.refresh();assert(!doc.body.classList.contains('guide-manual-floating'),'New steps restore automatic avoidance');clear('.set-ct');clear('[data-modality="ct"]');assert.notDeepEqual({x:card.style.left,y:card.style.top},initial,'next modality gets a new safe position')
 assert(!doc.querySelector('.set-mri')!.classList.contains('guided-pulse'));assert(doc.querySelector('.set-ct')!.classList.contains('guided-pulse'))
 await new Promise(r=>setTimeout(r,1850));assert(!doc.querySelector('.set-ct')!.classList.contains('guided-pulse'));assert(doc.querySelector('.set-ct')!.classList.contains('guided-target'))
 Object.defineProperty(fresh.window,'matchMedia',{value:()=>({matches:true})});state.landmarks[0].mri=null;g.refresh();assert(!doc.querySelector('.set-mri')!.classList.contains('guided-pulse'),'reduced motion skips pulses')
 state.fit=imageGeometryAlignment('ct-mri');doc.querySelector<HTMLButtonElement>('#define-windows')!.click();await Promise.resolve();assert(doc.body.classList.contains('guide-auto-docked'),'no clear floating position reserves dedicated guide space')
 const beforeState=JSON.stringify(state);pointer('pointerdown',0,0);pointer('pointermove',600,200);pointer('pointerup',0,0);assert.equal(card.dataset.side,'floating');assert(!doc.body.classList.contains('guide-auto-docked'),'A docked guide can be dragged out');assert.equal(card.style.left,'612px');assert.equal(JSON.stringify(state),beforeState,'Dragging cannot change registration or landmark state');doc.querySelector<HTMLButtonElement>('#guided-anchor')!.click();assert(doc.body.classList.contains('guide-auto-docked'),'Follow restores automatic docking when crowded')
 move.dispatchEvent(new fresh.window.KeyboardEvent('keydown',{key:'ArrowRight',bubbles:true,cancelable:true}));assert.equal(card.dataset.side,'floating');assert(Number.isFinite(parseFloat(card.style.left)),'Keyboard undocking uses the visible card position')
 // Explicit dock preference survives the temporary manual override.
 const dockPreference=getPreferences().guide.dock;getPreferences().guide.dock=true;doc.body.classList.add('guide-docked');doc.querySelector<HTMLButtonElement>('#guided-anchor')!.click();assert.equal(card.dataset.side,'docked');pointer('pointerdown',0,0);pointer('pointermove',600,200);pointer('pointerup',0,0);assert.equal(card.dataset.side,'floating');assert(doc.body.classList.contains('guide-manual-floating'));assert.equal(getPreferences().guide.dock,true);doc.querySelector<HTMLButtonElement>('#guided-anchor')!.click();assert.equal(card.dataset.side,'docked');assert(!doc.body.classList.contains('guide-manual-floating'));getPreferences().guide.dock=dockPreference;doc.body.classList.remove('guide-docked')
 g.close();assert(!doc.body.classList.contains('guide-auto-docked'));assert(!doc.body.classList.contains('guide-manual-floating'));fresh.window.close()
}
console.log('Guide avoids active controls/image rows, moves on modality change, docks when crowded, pulses then retains outline, and honors reduced motion.')

// Reproduce the app's deferred landmark-row rendering, rather than prebuilding
// targets before the guide advances. This was the launch-button highlight trap.
{
 const fresh=new JSDOM('<main><button id="guided-setup">Guide</button><button id="new-pair">New</button><details open><div id="landmark-list"></div></details><button id="fit-side">Fit</button></main>',{pretendToBeVisual:true});Object.assign(globalThis,{window:fresh.window,document:fresh.window.document});const doc=fresh.window.document;
 const state:GuideState={mri:true,ct:true,landmarks:[],selectedId:null,minimumPairs:3,fit:null,busy:false,error:null,viewerReady:true};let next=1;
 const paint=()=>{const list=doc.querySelector('#landmark-list')!;list.replaceChildren();for(const point of state.landmarks){const row=doc.createElement('div');row.className='landmark-row';row.dataset.landmarkId=String(point.id);for(const modality of ['mri','ct'] as const){const b=doc.createElement('button');b.className=`set-${modality}`;b.textContent=point[modality]?'Replace':'Set';b.onclick=()=>{point[modality]=[point.id,2,3];fresh.window.requestAnimationFrame(paint)};row.append(b)}list.append(row)}};
 doc.querySelector<HTMLButtonElement>('#new-pair')!.onclick=()=>{state.selectedId=next;state.landmarks.push({id:next++,mri:null,ct:null,enabled:true});fresh.window.requestAnimationFrame(paint)};
 const g=installGuidedSetup(()=>state),action=(text:string)=>userGuideAction(doc,g.refresh,text),settle=()=>new Promise(r=>fresh.window.requestAnimationFrame(()=>queueMicrotask(r)));
 doc.querySelector<HTMLButtonElement>('#guided-setup')!.click();action('Guide me through landmarks');
 for(let id=1;id<=2;id++){
  action('Add landmark pair');assert.equal(doc.querySelector('#guided-title')!.textContent,`Landmark ${id}: set MRI`);assert(!doc.querySelector('#guided-setup')!.classList.contains('guided-target'),'Missing row cannot highlight launcher');await settle();
  const selector=`[data-landmark-id="${id}"] .set-mri`;assert(doc.querySelector(selector)!.classList.contains('guided-target'),'Deferred row acquires current highlight');const detached=doc.querySelector(selector)!;paint();await Promise.resolve();assert(!detached.isConnected);assert(doc.querySelector(selector)!.classList.contains('guided-target'),'Rebuilt rows reacquire highlight');assert(!doc.querySelector(selector)!.classList.contains('guided-pulse'),'Rebuilding the same target must not repeatedly flash it');
  action(`Set MRI point for landmark ${id}`);await settle();assert(doc.querySelector(`[data-landmark-id="${id}"] .set-ct`)!.classList.contains('guided-target'));action(`Set CT point for landmark ${id}`);await settle();
 }
 action('Add landmark pair');await settle();assert.equal(doc.querySelector('#guided-title')!.textContent,'Landmark 3: set MRI');const saved=JSON.stringify(state.landmarks);
 doc.querySelector<HTMLButtonElement>('#guided-back')!.click();assert.equal(doc.querySelector('#guided-title')!.textContent,'Landmark pair already added');assert(![...doc.querySelectorAll('#guided-actions button')].some(b=>b.textContent==='Add landmark pair'),'Back cannot create a duplicate unfinished pair');doc.querySelector<HTMLButtonElement>('#guided-back')!.click();assert.equal(doc.querySelector('#guided-title')!.textContent,'Landmark 2: review CT');assert(doc.querySelector('[data-landmark-id="2"] .set-ct')!.classList.contains('guided-target'));assert.equal(JSON.stringify(state.landmarks),saved,'Back never rolls back placed coordinates');
 doc.querySelector<HTMLButtonElement>('#guided-back')!.click();assert.equal(doc.querySelector('#guided-title')!.textContent,'Landmark 2: review MRI');action('Continue with current progress');assert.equal(doc.querySelector('#guided-title')!.textContent,'Landmark 3: set MRI');doc.querySelector<HTMLButtonElement>('#guided-close')!.click();doc.querySelector<HTMLButtonElement>('#guided-resume')!.click();assert.equal(doc.querySelector('#guided-title')!.textContent,'Landmark 3: set MRI');g.close();fresh.window.close();
}
console.log('Deferred/rebuilt landmark targets, explicit point actions, per-point Back and non-destructive resume passed.');
