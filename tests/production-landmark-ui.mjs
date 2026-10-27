import vm from 'node:vm'
import {Worker as NodeWorker} from 'node:worker_threads'
import {File as NativeFile} from 'node:buffer'
import assert from 'node:assert/strict'
import fs from 'node:fs'
import path from 'node:path'
import { createRequire } from 'node:module'
import { fileURLToPath } from 'node:url'
import {assertRegistrationMethodStructure,exerciseRegistrationMethodDisclosures} from './registration-method-structure.mjs'
const root = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '..')
const require = createRequire(new URL('../apps/align/package.json', import.meta.url))
const { JSDOM } = require('jsdom')
const dist = process.argv[2] ? path.resolve(process.argv[2]) : path.join(root,'apps/align','dist')
const html = fs.readFileSync(path.join(dist,'index.html'),'utf8')
const entry = html.match(/src="\/assets\/([^"]+\.js)"/)?.[1]
assert.ok(entry)
const dom = new JSDOM('<div id="app"></div>', {url:'http://127.0.0.1:12345/#session='+ 'a'.repeat(64),runScripts:'outside-only',pretendToBeVisual:true})
const w = dom.window
// A destroyed real browsing context drops queued work. JSDOM otherwise runs it
// after close(), then its exception reporter accesses a destroyed Location.
// Keep live callback errors fatal; only discard callbacks after context teardown.
w.queueMicrotask=callback=>queueMicrotask(()=>{if(w.document)callback()})
// Reproduce legacy saved settings and every expanded panel. None may reopen at startup.
w.localStorage.setItem('brainana-align-layout',JSON.stringify({workflow:'manual',panels:Object.fromEntries(['initial-processing-panel','landmarks-panel','landmark-alignment-panel','followup-panel','manual-panel','refine-panel','alignment-display-panel'].map(id=>[id,true]))}))
w.localStorage.setItem('brainana-align-preferences',JSON.stringify({schemaVersion:1,saving:{rememberLayout:true}}));
let startupWorkflowClicks=0;w.document.addEventListener('click',e=>{if(e.target.closest?.('[data-workflow]'))startupWorkflowClicks++},true)
w.HTMLCanvasElement.prototype.getContext = () => null
w.fetch = async () => new Response(JSON.stringify({enabled:true,mode:'local',label:'Test'}),{headers:{'content-type':'application/json'}})
w.Headers=Headers;w.Response=Response;w.Request=Request;w.Blob=Blob;w.structuredClone=structuredClone;w.TextDecoder=TextDecoder;w.TextEncoder=TextEncoder
w.ResizeObserver=class {observe(){}disconnect(){}}
w.matchMedia=()=>({matches:false,addEventListener(){},removeEventListener(){}})
const dialogs=[];let allowReflection=false
w.confirm = message => {dialogs.push(message);return !message.includes('MIRRORS ANATOMY') || allowReflection}
const module = new vm.SourceTextModule(fs.readFileSync(path.join(dist,'assets',entry),'utf8'), {context:dom.getInternalVMContext(),initializeImportMeta(meta){meta.url=`http://127.0.0.1:12345/assets/${entry}`}})
await module.link(()=>{throw new Error('Unexpected static import')});await module.evaluate()
await new Promise(r=>setTimeout(r,30))
assert.equal(w.document.querySelector('#welcome-screen').getAttribute('aria-label'),'Image registration setup');assert(!w.document.querySelector('#welcome-screen').textContent.includes('BRAINANA / IMAGE REGISTRATION'));assert(!w.document.querySelector('#welcome-screen').textContent.includes('Bring your brain images into alignment.'));
assert(!w.document.querySelector('#welcome-screen').classList.contains('hidden'));assert(w.document.querySelector('main').classList.contains('welcome-hidden'));assert.equal(w.document.querySelectorAll('[data-registration-pairing]').length,4);
// Use the real compiled guide and welcome together, before choosing a pairing.
for(const launcher of ['#welcome-guided','#guided-setup']){
 w.document.querySelector(launcher).click();assert.equal(w.document.querySelector('#guided-card').dataset.step,'welcome');assert.equal(w.document.querySelectorAll('#guided-text > p').length,3);assert.deepEqual([...w.document.querySelectorAll('#guided-text strong')].map(el=>el.textContent),['Choose a pairing:','Next:','Saved work:']);assert(!w.document.querySelector('#guided-text').textContent.includes('**'));assert.equal(w.document.querySelector('#guided-title').textContent,'Choose your image pairing');assert(!w.document.querySelector('#welcome-screen').classList.contains('hidden'));assert(w.document.querySelector('main').classList.contains('welcome-hidden'));assert(w.document.querySelector('.welcome-pairings').classList.contains('guided-target'));assert(w.document.querySelector('#new-pair').disabled);w.document.querySelector('#welcome-guided').click();assert(!w.document.querySelector('#guided-card').classList.contains('hidden'),'Welcome launcher does not toggle off an active guide');w.document.querySelector('#guided-restart').click();assert.equal(w.document.querySelector('#guided-card').dataset.step,'welcome');w.document.querySelector('#guided-close').click();w.document.querySelector('#guided-resume').click();assert.equal(w.document.querySelector('#guided-card').dataset.step,'welcome');w.document.querySelector('#guided-close').click()
}
w.document.querySelector('[data-registration-pairing="t1-t2"]').click();assert.equal(w.document.querySelector('#ct-name').textContent,'Select T2');assert.equal(w.document.querySelector('#direction option[value="ct-mri"]').textContent,'T2 → T1');w.document.querySelector('[data-registration-pairing="mri-ct"]').click();assert(!w.document.querySelector('main').classList.contains('welcome-hidden'));
// Volume source selection is visible in the exact compiled app; dismissing never changes registration.
const chooserBefore=JSON.stringify(w.document.querySelector('#mri-name').textContent)
w.document.querySelector('#ct-file').dispatchEvent(new w.MouseEvent('click',{bubbles:true,cancelable:true}));await new Promise(r=>setTimeout(r,30));assert.equal(w.document.querySelector('#data-source-modal').classList.contains('hidden'),false)
assert.ok(w.document.querySelector('#data-source-local').textContent.includes('mounted network'))
w.document.querySelector('#data-source-close').click();assert.equal(w.document.querySelector('#data-source-modal').classList.contains('hidden'),true);assert.equal(JSON.stringify(w.document.querySelector('#mri-name').textContent),chooserBefore)
// Network opens the in-app browser even without a desktop connection-launcher bridge.
w.document.querySelector('#ct-file').dispatchEvent(new w.MouseEvent('click',{bubbles:true,cancelable:true}));await new Promise(r=>setTimeout(r,30));w.document.querySelector('#data-source-network').click();await new Promise(r=>setTimeout(r,30));assert.equal(w.document.querySelector('#network-browser-modal').classList.contains('hidden'),false);assert.ok(w.document.querySelector('#network-host'));w.document.querySelector('#network-close').click()
// Reproduce the reported overlap using the exact compiled stylesheet and app.
const guideModalStyle=w.document.createElement('style');guideModalStyle.textContent=fs.readFileSync(path.join(dist,'assets',html.match(/href="\/assets\/([^"]+\.css)"/)[1]),'utf8');w.document.head.append(guideModalStyle)
w.document.querySelector('#guided-setup').click()
for(const modality of ['mri','ct']){
 w.document.querySelector('#'+modality+'-file').dispatchEvent(new w.MouseEvent('click',{bubbles:true,cancelable:true}));await new Promise(r=>setTimeout(r,10));assert.equal(w.getComputedStyle(w.document.querySelector('#guided-card')).visibility,'hidden');assert.equal(w.getComputedStyle(w.document.querySelector('#guided-card')).pointerEvents,'none');w.document.querySelector('#data-source-modal').dispatchEvent(new w.KeyboardEvent('keydown',{key:'Escape',bubbles:true,cancelable:true}));await new Promise(r=>setTimeout(r,0));assert(w.document.querySelector('#data-source-modal').classList.contains('hidden'));assert(!w.document.querySelector('#guided-card').classList.contains('hidden'));assert.equal(w.getComputedStyle(w.document.querySelector('#guided-card')).visibility,'visible')
}
for(const id of ['network-browser-modal','preparation-modal','settings-modal','export-modal','ba-folder-modal']){
 const modal=w.document.getElementById(id);assert(modal,id);modal.classList.remove('hidden');await new Promise(r=>setTimeout(r,0));assert.equal(w.getComputedStyle(w.document.querySelector('#guided-card')).visibility,'hidden',id);modal.classList.add('hidden');await new Promise(r=>setTimeout(r,0));assert.equal(w.getComputedStyle(w.document.querySelector('#guided-card')).visibility,'visible',id)
}
w.document.querySelector('#export-modal').classList.remove('hidden');w.document.dispatchEvent(new w.KeyboardEvent('keydown',{key:'Escape',bubbles:true,cancelable:true}));await new Promise(r=>setTimeout(r,0));assert(w.document.querySelector('#export-modal').classList.contains('hidden'));assert(!w.document.querySelector('#guided-card').classList.contains('hidden'),'Export Escape must not also pause the compiled guide');assert.equal(w.getComputedStyle(w.document.querySelector('#guided-card')).visibility,'visible')
w.document.querySelector('#guided-close').click();guideModalStyle.remove()
assert.equal(startupWorkflowClicks,0,'Startup must not replay workflow clicks from saved layout');assert.equal(w.document.querySelectorAll('[data-workflow][aria-pressed=true]').length,0,'All workflows start neutral even with a saved Manual workflow');assert.equal(w.document.querySelectorAll('.sidebar details[open]').length,0,'Every sidebar section stays collapsed despite legacy all-open saved layout');await new Promise(r=>setTimeout(r,100));assert.equal(w.document.querySelectorAll('.sidebar details[open]').length,0,'Late preference/guide initialization cannot reopen sections');
assertRegistrationMethodStructure(w.document);const registrationStarts=[];for(const id of ['auto-linear','jip-run','itk-run'])w.document.getElementById(id).addEventListener('click',()=>registrationStarts.push(id));await exerciseRegistrationMethodDisclosures(w.document,{requests:()=>registrationStarts.length,state:()=>w.document.querySelector('#fit-summary').innerHTML});assert.equal(w.document.querySelectorAll('.sidebar details[open]').length,0,'Method disclosure testing restores the collapsed launch state');assert(w.document.querySelector('#define-windows').compareDocumentPosition(w.document.querySelector('#linear-registration'))&4,'Shared optimization regions precede the method peers')
assert.equal(w.document.querySelector('#processing-brush-radius').type,'number');assert.deepEqual([...w.document.querySelectorAll('#processing-edit-target option')].map(o=>o.value),['mri','ct','mri-mask','ct-mask']);assert(w.document.querySelector('#processing-editing').contains(w.document.querySelector('#processing-edit-target')));
assert.equal(w.document.querySelectorAll('.preparation-body > .preparation-column').length,2,'Packaged Resolution contains both columns');
assert(w.document.querySelector('.preparation-footer').contains(w.document.querySelector('#prepare-images')),'Prepare stays in the fixed dialog footer');
assert(w.document.querySelector('.preparation-column:first-child').contains(w.document.querySelector('#preparation-images')));
assert(w.document.querySelector('.preparation-column:last-child').contains(w.document.querySelector('#preparation-estimate')));
assert(w.document.querySelector('#refine-panel').contains(w.document.querySelector('#define-windows')));assert(w.document.querySelector('#initial-processing-panel').contains(w.document.querySelector('#processing-voxel')));assert.equal(w.document.querySelector('#processing-value').value,'0');assert.equal(w.document.querySelector('#aligned-image-source').value,'native');
for(const m of ['mri','ct']){for(const k of ['contrast','display-min','display-max'])assert(w.document.querySelector(`#initial-processing-panel #${m}-${k}[type=range]`),'Packaged display sliders are in Initial processing');assert(w.document.querySelector(`#${m}-display-reset`));}assert(w.document.querySelector('#followup-panel').contains(w.document.querySelector('#define-windows-followup')));assert(!w.document.querySelector('#initial-processing-panel').contains(w.document.querySelector('#define-windows')));
for(const m of ['mri','ct']){for(const k of ['min','max'])assert(w.document.querySelector(`#initial-processing-panel #${m}-mask-${k}[type=range]`));for(const action of ['preview','invert','create','apply','clear'])assert(w.document.querySelector(`#${m}-mask-${action}`));}for(const m of ['mri','ct']){const holes=w.document.querySelector(`#${m}-mask-holes`);assert(holes,'Hole filling ships in both mask panels');assert.deepEqual([...holes.options].map(o=>o.value),['','3d','slices']);assert.equal(holes.value,'','Filling starts off');}assert.equal(w.document.querySelectorAll('canvas.threshold-mask-preview').length,6,'Six non-interactive mask preview canvases ship in the compiled app');
assert(w.document.querySelector('#refresh-processed-overlay').classList.contains('hidden'),'Overlay refresh starts hidden');
assert(!w.document.querySelector('#mri-mask-from-display'));assert(!w.document.querySelector('#ct-mask-registration'));assert(w.document.querySelector('#generated-image-choice'));
const ids = [...w.document.querySelectorAll('[id]')].map(e=>e.id)
assert.equal(new Set(ids).size, ids.length, 'Shell controls must retain unique IDs')
// Exercise the Settings window in the exact built frontend, without substituting its controller.
assert.ok(w.document.querySelector('#settings-open'));w.document.querySelector('#settings-open').click()
assert.equal(w.document.querySelector('#settings-modal').classList.contains('hidden'),false)
assert.equal(w.document.querySelectorAll('#settings-modal [data-section]').length,6)
const settingsTheme=w.document.querySelector('[data-group="appearance"][data-key="theme"]');settingsTheme.value='light';settingsTheme.dispatchEvent(new w.Event('change'));await new Promise(r=>setTimeout(r,20))
assert.equal(w.document.documentElement.style.getPropertyValue('--bg'),'#e8edf2');assert.equal(JSON.parse(w.localStorage.getItem('brainana-align-preferences')).appearance.theme,'light')
assert.equal(JSON.parse(w.localStorage.getItem('brainana-align-preferences')).display.movingColor,'#ff00ff','Chrome palette must preserve imaging overlay colors')
w.document.querySelector('[data-section="registration"]').click();const defaultModel=w.document.querySelector('[data-group="registration"][data-key="model"]');defaultModel.value='affine';defaultModel.dispatchEvent(new w.Event('change'));await new Promise(r=>setTimeout(r,20))
assert.equal(w.document.querySelector('#landmark-transform').value,'rigid','Changing saved defaults must not change active registration controls')
w.document.querySelector('#settings-apply-registration').click();assert.equal(w.document.querySelector('#landmark-transform').value,'affine')
w.document.querySelector('#settings-reset-all').click();await new Promise(r=>setTimeout(r,25));w.document.querySelector('#settings-apply-registration').click();assert.equal(w.document.querySelector('#landmark-transform').value,'rigid')
w.document.querySelector('[data-section="guide"]').click();const dock=w.document.querySelector('[data-group="guide"][data-key="dock"]');dock.checked=true;dock.dispatchEvent(new w.Event('change'));await new Promise(r=>setTimeout(r,20));assert.ok(w.document.body.classList.contains('guide-docked'))
w.document.querySelector('#settings-reset-section').click();await new Promise(r=>setTimeout(r,20));assert.equal(w.document.body.classList.contains('guide-docked'),false)
w.document.querySelector('[data-section="saving"]').click();assert.ok(w.document.querySelector('#settings-recover'));assert.ok(w.document.querySelector('#settings-export-folder-choose'));assert.equal(w.document.querySelector('#settings-export-folder-choose').disabled,true,'Browser-only mode uses browser download settings')
w.document.dispatchEvent(new w.KeyboardEvent('keydown',{key:'Escape',bubbles:true}));assert.equal(w.document.querySelector('#settings-modal').classList.contains('hidden'),true)
w.document.dispatchEvent(new w.KeyboardEvent('keydown',{key:',',ctrlKey:true,bubbles:true}));assert.equal(w.document.querySelector('#settings-modal').classList.contains('hidden'),false);w.document.querySelector('#settings-close').click()

assert.equal(w.document.querySelectorAll('.workspace .view-card').length, 9)
assert.ok(w.document.querySelector('.sidebar #fit-side'))
const sidebarSections=[...w.document.querySelectorAll('.sidebar > details.panel')];assert.deepEqual(sidebarSections.map(p=>p.querySelector('summary span').textContent),['Initial processing','Landmarks','Landmark alignment','Landmark refinement','Manual adjustment','Image-based registration','Alignment display']);
for(const panel of sidebarSections){const before=sidebarSections.map(p=>p.open);panel.querySelector('summary').click();assert.equal(panel.open,!before[sidebarSections.indexOf(panel)]);for(let i=0;i<sidebarSections.length;i++)if(sidebarSections[i]!==panel)assert.equal(sidebarSections[i].open,before[i],'Compiled section toggle must preserve siblings');panel.querySelector('summary').click();assert.deepEqual(sidebarSections.map(p=>p.open),before)}

assert.ok(w.document.querySelector('.statusbar #status[role="status"][aria-live="polite"]'))
assert.ok(w.document.querySelector('.topbar .image-loads #mri-file'))
assert.ok(w.document.querySelector('.topbar .session-actions #load-session'))
assert.equal(w.document.querySelector('.brand span').textContent, 'v'+JSON.parse(fs.readFileSync(path.join(root,'VERSION.json'))).version)
const cssEntry=html.match(/href="\/assets\/([^"]+\.css)"/)?.[1]
const css=fs.readFileSync(path.join(dist,'assets',cssEntry),'utf8')
for(const file of ['source-sans-3.woff2','ibm-plex-mono-400.woff2','ibm-plex-mono-600.woff2']) {
 assert.ok(css.includes('/fonts/'+file))
 assert.ok(fs.statSync(path.join(dist,'fonts',file)).size>1000)
}
assert.ok(fs.readFileSync(path.join(dist,'fonts','OFL.txt'),'utf8').includes('SIL OPEN FONT LICENSE'))
assert.ok(fs.readFileSync(path.join(dist,'fonts','NOTICE.txt'),'utf8').includes('Adobe'))
assert.doesNotMatch(css, /https?:\/\//, 'The theme must work without external font/style requests')
const select = w.document.querySelector('#landmark-transform')
assert.ok(select)
assert.equal(select.value,'rigid')
assert.deepEqual([...select.options].map(o=>o.textContent),['Rigid, 6 parameters','Similarity, 7 parameters','Anisotropic scale, 9 parameters','Affine, 12 parameters'])
assert.equal(w.document.querySelector('#spline-settings').classList.contains('hidden'),false);assert.ok(w.document.querySelector('#spline-settings').textContent.includes('12 distributed pairs'));select.value='rigid';select.dispatchEvent(new w.Event('change'));assert.equal(w.document.querySelector('#spline-settings').classList.contains('hidden'),false)
assert.ok(w.document.querySelector('#guided-setup'))
w.document.querySelector('#guided-setup').click()
assert.equal(w.document.querySelector('#guided-card').getAttribute('aria-modal'),'false')
assert.equal(w.document.querySelector('#guided-title').textContent,'Viewer needs attention')
const guideCard=w.document.querySelector('#guided-card'),guideHandle=w.document.querySelector('#guided-move');w.document.body.classList.add('guide-auto-docked');guideCard.dataset.side='docked';for(const [type,x,y] of [['pointerdown',0,0],['pointermove',600,200],['pointerup',600,200]]){const e=new w.Event(type,{bubbles:true,cancelable:true});Object.assign(e,{clientX:x,clientY:y,pointerId:41,button:0,isPrimary:true});guideHandle.dispatchEvent(e)}assert.equal(guideCard.dataset.side,'floating');assert(parseFloat(guideCard.style.left)>400,'Exact compiled guide can drag right across the sidebar');assert(!w.document.body.classList.contains('guide-auto-docked'));assert(w.document.body.classList.contains('guide-manual-floating'));const placedLeft=guideCard.style.left;w.dispatchEvent(new w.Event('resize'));assert.equal(guideCard.style.left,placedLeft);w.document.querySelector('#guided-anchor').click();assert(!w.document.body.classList.contains('guide-manual-floating'),'Follow restores automatic placement in compiled UI')
w.document.querySelector('#guided-close').click()
assert.ok(w.document.querySelector('#guided-card').classList.contains('hidden'))
assert.equal(w.document.querySelector('#guided-setup').textContent,'Resume guide')
assert.ok(!w.document.querySelector('#guided-resume').classList.contains('hidden'))
w.document.querySelector('#guided-resume').click()
assert.equal(w.document.querySelector('#guided-title').textContent,'Viewer needs attention')
w.document.querySelector('#guided-close').click()
assert.ok(w.document.querySelector('#guided-move[tabindex="0"]'))
assert.ok(w.document.querySelector('#guided-restart'))
assert.ok(w.document.querySelector('#use-image-geometry'))
assert.equal(w.document.querySelector('#auto-linear-model').value,'rigid')
assert.deepEqual([...w.document.querySelector('#auto-linear-model').options].map(o=>o.value),['rigid','similarity','anisotropic','affine','custom'])
assert.equal(w.document.querySelector('#intensity-operation').value,'global')
assert(w.document.querySelector('#define-windows').compareDocumentPosition(w.document.querySelector('#intensity-operation')) & 4)
const operation=w.document.querySelector('#intensity-operation');operation.value='local';operation.dispatchEvent(new w.Event('change'))
assert.equal(w.document.querySelector('#local-refinement-settings').classList.contains('hidden'),false)
const preset=w.document.querySelector('#refine-bounds');preset.value='tight';preset.dispatchEvent(new w.Event('change'))
assert.equal(w.document.querySelector('#refine-translation-limit').value,'2');assert.equal(w.document.querySelector('#refine-rotation-limit').value,'1')
assert.equal(w.document.querySelector('#refine-translation-step').value,'1');assert.equal(w.document.querySelector('#refine-rotation-step').value,'0.5')
preset.value='standard';preset.dispatchEvent(new w.Event('change'));assert.equal(w.document.querySelector('#refine-rotation-limit').value,'10')
operation.value='global';operation.dispatchEvent(new w.Event('change'));assert.equal(w.document.querySelector('#local-refinement-settings').classList.contains('hidden'),false,'Local form never disappears when the global workflow is focused')

assert(w.document.querySelector('#landmark-alignment-panel').contains(w.document.querySelector('#spline-alignment')));assert(w.document.querySelector('#spline-availability').textContent.includes('Load both'));
assert.ok(w.document.querySelector('#alignment-cancel'))
assert.ok(fs.readdirSync(path.join(dist,'assets')).some(f=>f.startsWith('registrationWorker-')&&f.endsWith('.js')))
w.document.querySelector('[data-workflow="automatic"]').click()
assert.equal(w.document.querySelector('#landmarks-panel').open,false)
assert.equal(w.document.querySelector('#landmark-alignment-panel').open,false)
assert.equal(w.document.querySelector('#manual-panel').open,false)
assert.equal(w.document.querySelector('#refine-panel').open,true)
w.document.querySelector('[data-workflow="followup"]').click()
assert.equal(w.document.querySelector('#followup-panel').open,true)
assert.equal(operation.value,'local')
assert(w.document.querySelector('#followup-panel').contains(w.document.querySelector('#local-auto-linear')))
assert.equal(w.document.querySelector('#local-auto-linear').disabled,true,'No landmark basis yet')
assert(w.document.querySelector('#landmark-alignment-panel').contains(w.document.querySelector('#spline-settings')))
const bins=w.document.querySelector('#local-auto-bins');bins.value='128'
w.document.querySelector('[data-workflow="followup"]').click();assert.equal(bins.value,'128')
w.document.querySelector('#define-windows-followup').click()
assert.equal(w.document.querySelector('#define-windows').getAttribute('aria-pressed'),'true')
w.document.querySelector('[data-workflow="automatic"]').click()
assert.equal(operation.value,'global');assert.equal(w.document.querySelector('#define-windows').getAttribute('aria-pressed'),'true','Tab change retains shared window editing')
assert.equal(bins.value,'128','Refinement retains its own settings');assert.equal(w.document.querySelector('#auto-bins').value,'64')
w.document.querySelector('#define-windows').click();await Promise.resolve()
assert.equal(w.document.querySelector('#define-windows').getAttribute('aria-pressed'),'false');assert(w.document.querySelector('#refine-panel').contains(w.document.querySelector('#define-windows')))
w.document.querySelector('[data-workflow="landmarks"]').click()
assert.equal(w.document.querySelector('#landmarks-panel').open,true)
assert.equal(w.document.querySelector('#landmark-alignment-panel').open,false)
async function settledFit() {
 for(let i=0;i<100;i++) {if(w.document.querySelector('#alignment-progress').classList.contains('hidden'))return;await new Promise(r=>setTimeout(r,10))}
 throw new Error('Landmark activity did not finish')
}
const source = [[0,0,0],[10,0,0],[0,12,0],[0,0,8],[4,5,6],[9,3,2]]
async function loadLandmarks(linear) {
  const landmarks=source.map((ct,i)=>({id:i+1,ct,mri:linear.map((row,j)=>row.reduce((sum,v,k)=>sum+v*ct[k],0)+[2,-3,7][j]),enabled:true}))
  const payload={application:'Brainana Align',schemaVersion:1,images:{mri:null,ct:null},landmarks,direction:'ct-mri',fit:null,optimizationWindows:{mri:{},ct:{}}}
  const input=w.document.querySelector('#load-session')
  Object.defineProperty(input,'files',{configurable:true,value:[{text:async()=>JSON.stringify(payload)}]})
  input.dispatchEvent(new w.Event('change'))
  await new Promise(r=>setTimeout(r,20))
}
const models={rigid:[[0,-1,0],[1,0,0],[0,0,1]],similarity:[[0,-1.7,0],[1.7,0,0],[0,0,1.7]],anisotropic:[[0,-1.8,0],[0.7,0,0],[0,0,2.3]],affine:[[0.8,0.2,-0.3],[0,1.7,0.4],[0,0,2.1]]}
const saved=[]
w.brainanaAlignSaveBlob=async(blob,filename)=>saved.push({filename,text:await blob.text()})
for(const [model,linear] of Object.entries(models)) {
  await loadLandmarks(linear);select.value=model
  w.document.querySelector('#fit-side').click()
  assert.ok(!w.document.querySelector('#alignment-progress').classList.contains('hidden'))
  await settledFit()
  assert.equal(w.document.querySelector('main').inert,false)
  const summary=w.document.querySelector('#fit-summary').textContent
  assert.ok(summary.includes('RMS error: 0.000 mm'),`${model}: ${summary}`)
  for(const field of ['Determinant:','Scale X/Y/Z:','Shear magnitude:','Reflection:'])assert.ok(summary.includes(field))
  w.document.querySelector('#save-transform').click();await new Promise(r=>setTimeout(r,10))
  assert.equal(JSON.parse(saved.at(-3).text).transform_model,model)
  assert.ok(saved.at(-2).filename.endsWith(`_${model}.txt`))
}
await loadLandmarks([[-1,0,0],[0,1,0],[0,0,1]]);select.value='affine'
w.document.querySelector('#fit-side').click();await settledFit()
assert.equal(w.document.querySelector('#save-transform').disabled,true,'reflection declined must not apply')
assert.ok(dialogs.some(m=>m.includes('MIRRORS ANATOMY')))
allowReflection=true;w.document.querySelector('#fit-side').click();await settledFit()
assert.ok(w.document.querySelector('#fit-summary').textContent.includes('Reflection: YES'))
w.document.querySelector('#save-transform').click();await new Promise(r=>setTimeout(r,10))
assert.equal(JSON.parse(saved.at(-3).text).reflection,true)
assert.equal(JSON.parse(saved.at(-3).text).reflection_confirmed,true)
allowReflection=false
const count=saved.length;w.document.querySelector('#save-transform').click();await new Promise(r=>setTimeout(r,10));assert.equal(saved.length,count)
const beforeReset=w.document.querySelector('#fit-summary').innerHTML;let resetConfirms=0;w.confirm=message=>{assert(message.includes('Reset workspace and start over?'));resetConfirms++;return false}
assert.equal(w.document.querySelector('#guided-reset-workspace'),null,'Guide has no reset shortcut');w.document.querySelector('#reset-workspace').click();await new Promise(r=>setTimeout(r,10));assert.equal(resetConfirms,1,'Only the main toolbar offers Reset workspace');assert.equal(w.document.querySelector('#fit-summary').innerHTML,beforeReset);assert.equal(w.document.querySelector('main').inert,false)
// Verify feedback through the actual bundled click handler, not a substituted UI controller.
await loadLandmarks(models.rigid);select.value='rigid';w.document.querySelector('#fit-side').click();await settledFit()
w.document.querySelector('#export-open').click();assert.equal(w.document.querySelector('#export-modal').classList.contains('hidden'),false)
assert.equal(w.document.querySelector('#save-aligned').disabled,true,'Aligned image needs loaded volumes')
w.brainanaAlignSaveBlob=async()=>{throw new w.Error('Destination write failed')}
w.document.querySelector('#save-transform').click();await new Promise(r=>setTimeout(r,20))
assert.ok(w.document.querySelector('#export-feedback').textContent.includes('Destination write failed'));assert.ok(w.document.querySelector('#export-feedback').classList.contains('error'))
assert.equal(w.document.querySelector('#save-transform').disabled,false,'Failed save must permit retry')
w.brainanaAlignSaveBlob=async(blob,filename)=>saved.push({filename,text:await blob.text()})
const retryCount=saved.length;w.document.querySelector('#save-transform').click();await new Promise(r=>setTimeout(r,20));assert.equal(saved.length,retryCount+3);assert.equal(w.document.querySelector('#export-feedback').textContent,'Transform files saved.')
w.document.querySelector('#export-close').click()
// Exercise staged selection and commit in the exact compiled main composition root.
const emittedWorkerRequests=[],activeOverlayWorkers=new Set(),heldOverlayResults=[];let holdOverlayResults=false
w.DecompressionStream=DecompressionStream
w.Worker=class {
 constructor(url){this.asset=path.basename(new URL(String(url),'http://127.0.0.1').pathname);const workerCode=fs.readFileSync(path.join(dist,'assets',this.asset),'utf8');this.worker=new NodeWorker(`const {parentPort}=require('node:worker_threads');globalThis.self={postMessage:(m,t)=>parentPort.postMessage(m,t)};parentPort.on('message',data=>self.onmessage({data}));\n${workerCode}`,{eval:true});if(this.asset.startsWith('nonlinearWorker-'))activeOverlayWorkers.add(this);this.worker.on('message',data=>{const deliver=()=>this.onmessage?.({data});if(holdOverlayResults&&this.asset.startsWith('nonlinearWorker-')&&data.type==='result')heldOverlayResults.push(deliver);else deliver()});this.worker.on('error',e=>this.onerror?.({message:e.message}));this.worker.on('messageerror',e=>this.onmessageerror?.(e))}
 postMessage(data){emittedWorkerRequests.push({asset:this.asset,type:data.type});this.worker.postMessage(data)}
 terminate(){activeOverlayWorkers.delete(this);void this.worker.terminate()}
}
const waitFor=async(predicate,label)=>{for(let i=0;i<1000;i++){if(predicate())return;await new Promise(r=>setTimeout(r,10))}assert.fail(label)}
const settleOverlay=async()=>{await waitFor(()=>activeOverlayWorkers.size===0&&!w.document.querySelector('#review-label').textContent.includes('interactive preview')&&!w.document.querySelector('main').inert,'Emitted overlay did not settle: '+w.document.querySelector('#status').textContent);await new Promise(r=>setTimeout(r,30))}
const overlayRequestCount=()=>emittedWorkerRequests.filter(r=>r.asset.startsWith('nonlinearWorker-')).length
const nifti=await import('nifti-reader-js'),header=new nifti.NIFTI1();header.dims=[3,19,21,23,1,1,1,1];header.pixDims=[1,.2,.3,.4,1,1,1,1];header.affine=[[.2,0,0,-2],[0,.3,0,-3],[0,0,.4,-4],[0,0,0,1]];header.sform_code=2;header.datatypeCode=16;header.numBitsPerVoxel=32;header.vox_offset=352;header.magic='n+1\0';header.littleEndian=true;header.xyzt_units=2
const voxels=new Float32Array(19*21*23).fill(7),niftiBytes=new Uint8Array(352+voxels.byteLength);niftiBytes.set(new Uint8Array(header.toArrayBuffer()));niftiBytes.set(new Uint8Array(voxels.buffer),352)
for(const modality of ['mri','ct']){const input=w.document.querySelector(`#${modality}-file`);Object.defineProperty(input,'files',{configurable:true,value:[new NativeFile([niftiBytes],`${modality}.nii`)]});input.dispatchEvent(new w.Event('change'));await new Promise(r=>setTimeout(r,30))}
assert.equal(w.document.querySelector('#preparation-modal').classList.contains('hidden'),false)
assert.ok(w.document.querySelector('#mri-name').textContent.includes('Pending'))
const spacing=w.document.querySelector('[data-spacing="mri-0"]');spacing.value='.8';spacing.dispatchEvent(new w.Event('input'))
w.document.querySelector('#prepare-images').click()
for(let i=0;i<1000&&!w.document.querySelector('#preparation-modal').classList.contains('hidden');i++)await new Promise(r=>setTimeout(r,10))
assert.equal(w.document.querySelector('#preparation-modal').classList.contains('hidden'),true,w.document.querySelector('#preparation-error').textContent)
w.document.querySelector('#save-session').click();await new Promise(r=>setTimeout(r,20))
const preparedSession=JSON.parse(saved.at(-1).text);assert.deepEqual(preparedSession.images.mri.dims,[19,21,23]);assert(preparedSession.images.mri.workingDims[0]<19);assert.equal(preparedSession.images.mri.selectedFrame,0);assert.equal(preparedSession.fit,null,'New source preparation clears the prior accepted fit')
assert.equal(w.document.querySelector('#export-interpolation').value,'linear');assert.equal(w.document.querySelector('#export-fov').value,'reference-native');assert.equal(w.document.querySelector('#export-resolution').value,'reference-native');assert(w.document.querySelector('#processing-editing').textContent.includes('Voxel editing'));assert(w.document.querySelector('#opacity-cycle'));assert.equal(w.document.querySelector('#review-comparison').value,'smooth');const inspection=w.document.querySelector('#review-comparison');inspection.value='flip';inspection.dispatchEvent(new w.Event('change'));assert.equal(w.document.querySelector('#review-opacity').disabled,true);assert.equal(w.document.querySelector('#review-flip').disabled,true,'No overlay yet');inspection.value='smooth';inspection.dispatchEvent(new w.Event('change'));assert.equal(w.document.querySelector('#review-opacity').disabled,false);
// Exercise all 12 manual controls through the emitted application, with export direction unchanged.
assert.equal(w.document.querySelectorAll('#nudge-controls input[type="number"]').length,12)
w.document.querySelector('#direction').value='ct-mri';w.document.querySelector('#direction').dispatchEvent(new w.Event('change'))
assert.equal(w.document.querySelector('#manual-start-geometry').textContent,'Begin manual alignment');assert.equal(w.document.querySelector('#manual-start-section').classList.contains('hidden'),false);w.document.querySelector('#manual-start-geometry').click();await settleOverlay()
assert.equal(w.document.querySelector('#manual-start-section').classList.contains('hidden'),true);assert.equal(w.document.querySelector('[data-manual-target="mri"]').disabled,false)
w.document.querySelector('[data-manual-target="mri"]').click();await settleOverlay()
// Exercise the compiled scheduler with real emitted workers. Hold one completed
// response, supersede it through a real nudge, and deliver it after the new map.
const burstCount=overlayRequestCount();holdOverlayResults=true
for(let i=0;i<5;i++){w.document.querySelector('#nudge-controls [data-param="0"][data-sign="1"]').click();await new Promise(r=>setTimeout(r,20))}
await new Promise(r=>setTimeout(r,100));assert.equal(overlayRequestCount(),burstCount,'Nudges must defer full resampling until the pause');assert.equal(w.document.querySelector('main').inert,false)
await waitFor(()=>heldOverlayResults.length===1,'The actual emitted settled worker must complete');assert.equal(overlayRequestCount(),burstCount+1,'A burst starts exactly one settled worker');assert(w.document.querySelector('#review-label').textContent.includes('interactive preview'))
w.document.querySelector('#nudge-controls [data-param="0"][data-sign="1"]').click();holdOverlayResults=false;await settleOverlay();assert.equal(overlayRequestCount(),burstCount+2)
const latestOverlayLabel=w.document.querySelector('#review-label').textContent;assert(latestOverlayLabel.includes('aligned to'));for(const deliver of heldOverlayResults.splice(0))deliver();await new Promise(r=>setTimeout(r,30));assert.equal(w.document.querySelector('#review-label').textContent,latestOverlayLabel,'A stale actual-worker response cannot replace the latest display')
const beforeGuideWorkers=emittedWorkerRequests.length;w.document.querySelector('#guided-setup').click();w.document.querySelector('#guided-restart').click();w.document.querySelector('#guided-close').click();assert.equal(emittedWorkerRequests.length,beforeGuideWorkers,'Guide-only actions must never start a worker')
w.document.querySelector('#nudge-controls [data-param="0"][data-sign="1"]').click();await new Promise(r=>setTimeout(r,200))
assert(w.document.querySelector('#review-label').textContent.includes('interactive preview'),'Manual nudges must update display geometry before the settled full resample')
w.document.querySelector('#save-session').click();await new Promise(r=>setTimeout(r,20))
const manualSession=JSON.parse(saved.at(-1).text);assert.equal(manualSession.fit.direction,'ct-mri');assert(manualSession.fit.matrix[0][3]<0);assert.equal(manualSession.fit.manualAffine.selected,'mri')
const scaleField=w.document.querySelector('#param-6');scaleField.value='110';scaleField.dispatchEvent(new w.Event('change'));await new Promise(r=>setTimeout(r,200))
w.document.querySelector('#save-session').click();await new Promise(r=>setTimeout(r,20));assert.equal(JSON.parse(saved.at(-1).text).fit.model,'affine')
await settleOverlay()
const sourcePreview=w.document.querySelector('#manual-source-preview');sourcePreview.checked=true;sourcePreview.dispatchEvent(new w.Event('change'));await waitFor(()=>w.document.querySelector('#mri-coords').parentElement.querySelector('strong').textContent==='MRI · preview','Source preview must finish updating the source panes')
assert.equal(w.document.querySelector('#mri-coords').parentElement.querySelector('strong').textContent,'MRI · preview')
w.document.querySelector('#define-windows').click();await new Promise(r=>setTimeout(r,30));assert.equal(sourcePreview.checked,false);assert.equal(w.document.querySelector('#mri-coords').parentElement.querySelector('strong').textContent,'MRI')
w.document.querySelector('#reset-manual').click();await new Promise(r=>setTimeout(r,150));w.document.querySelector('#save-session').click();await new Promise(r=>setTimeout(r,20));assert.deepEqual(JSON.parse(saved.at(-1).text).fit.matrix,[[1,0,0,0],[0,1,0,0],[0,0,1,0],[0,0,0,1]])
// Reproduce the reported pending-refinement -> disabled Manual panel in the emitted app.
// The optional ITK adapter requires at least eight voxels per working axis.
w.document.querySelector('#working-resolution').click();w.document.querySelector('[data-preparation-preset="native"]').click();w.document.querySelector('#prepare-images').click();await waitFor(()=>w.document.querySelector('#preparation-modal').classList.contains('hidden'),'Native working preparation must finish');await settleOverlay();w.document.querySelector('#save-session').click();await new Promise(r=>setTimeout(r,20))
const acceptedSession=JSON.parse(saved.at(-1).text)
const cancelLinearSession=structuredClone(acceptedSession);cancelLinearSession.landmarks=source.map((ct,i)=>({id:i+1,ct,mri:models.rigid.map((row,j)=>row.reduce((sum,v,k)=>sum+v*ct[k],0)+[2,-3,7][j]),enabled:true}));cancelLinearSession.landmarkModel='rigid'
const cancelSessionInput=w.document.querySelector('#load-session');Object.defineProperty(cancelSessionInput,'files',{configurable:true,value:[{text:async()=>JSON.stringify(cancelLinearSession)}]});cancelSessionInput.dispatchEvent(new w.Event('change'));await new Promise(r=>setTimeout(r,250));await settleOverlay()
holdOverlayResults=true;w.document.querySelector('#fit-side').click();await waitFor(()=>heldOverlayResults.length===1,'Linear fit must start the actual affine overlay worker');w.document.querySelector('#alignment-cancel').click();holdOverlayResults=false;await settledFit();assert.equal(activeOverlayWorkers.size,0);for(const deliver of heldOverlayResults.splice(0))deliver()
w.document.querySelector('#save-session').click();await new Promise(r=>setTimeout(r,20));assert.deepEqual(JSON.parse(saved.at(-1).text).fit.matrix,acceptedSession.fit.matrix,'Cancelling affine overlay must restore the previous accepted transform for export')
holdOverlayResults=true;w.document.querySelector('#refresh-processed-overlay').click();await waitFor(()=>heldOverlayResults.length===1,'Explicit refresh must start an emitted worker');w.document.querySelector('#alignment-cancel').click();holdOverlayResults=false;await settledFit();for(const deliver of heldOverlayResults.splice(0))deliver();assert(w.document.querySelector('#review-label').textContent.includes('showing the previous display'),'Cancelling explicit refresh must retain the existing review');assert(!w.document.querySelector('#refresh-processed-overlay').classList.contains('hidden'),'Cancelled refresh must permit an explicit retry')
// Exercise shared candidate rollback through the real optional-engine handler.
w.brainanaScientific={status:async()=>({available:true,version:'0.8.4',features:['itk-v1','sampling-v1']}),choose:async()=>({available:true,version:'0.8.4',features:['itk-v1','sampling-v1']}),poll:async()=>({status:'running'}),cancel:async()=>{},run:async(request)=>({matrix:structuredClone(request.start),deformation:{version:1,kernel:'jip-dense-pull',baseAffine:structuredClone(request.start),dims:[8,8,8],affine:[[1,0,0,0],[0,1,0,0],[0,0,1,0],[0,0,0,1]],residualBase64:Buffer.from(new Float32Array(1536).fill(.015).buffer).toString('base64'),settings:{engine:'itk'},execution:{engine:'itk'},warnings:[]},settings:{engine:'itk'},execution:{engine:'itk',active:'cpu'},diagnostics:{relative_jacobian_range:[1,1],warnings:[]}})}
w.dispatchEvent(new w.Event('align-scientific-runtime-changed'));await new Promise(r=>setTimeout(r,30));assert.equal(w.document.querySelector('#itk-run').disabled,false,w.document.querySelector('#itk-availability').textContent);holdOverlayResults=true;w.document.querySelector('#itk-run').click();await waitFor(()=>heldOverlayResults.length===1,'ITK candidate must reach its actual emitted preview worker');assert(w.document.querySelector('#refine-summary').textContent.startsWith('ITK proposal ready.'),'The actual pending ITK preview summary must name ITK');assert(!w.document.querySelector('#refine-summary').textContent.includes('JIP'));w.document.querySelector('#alignment-cancel').click();holdOverlayResults=false;await settledFit();for(const deliver of heldOverlayResults.splice(0))deliver();assert(w.document.querySelector('#review-label').textContent.includes('showing the previous display'),'Cancelling a shared candidate preview must retain the prior overlay');assert(w.document.querySelector('#manual-start-section').classList.contains('hidden'));w.document.querySelector('#save-session').click();await new Promise(r=>setTimeout(r,20));assert.deepEqual(JSON.parse(saved.at(-1).text).fit.matrix,acceptedSession.fit.matrix,'Cancelled optional-engine preview must retain the prior accepted matrix')
// Both public and legacy initializer routes must preserve an existing transform.
w.document.querySelector('#use-image-geometry').click();await new Promise(r=>setTimeout(r,30));w.document.querySelector('#save-session').click();await new Promise(r=>setTimeout(r,20));assert.deepEqual(JSON.parse(saved.at(-1).text).fit,acceptedSession.fit,'The initializer cannot replace an existing alignment')
for(const accept of [true,false]){
 const pending=structuredClone(acceptedSession),candidate=structuredClone(pending.fit);candidate.matrix[0][3]=.25;candidate.inverse[0][3]=-.25;candidate.baseMatrix=structuredClone(candidate.matrix);candidate.fittedAt='pending-compiled-'+accept;candidate.proposal={kind:'automatic',matrix:structuredClone(candidate.matrix),params:Array(12).fill(0),scoreBefore:1,scoreAfter:2,previous:structuredClone(acceptedSession.fit)};candidate.automaticRegistration={operation:'local',model:'rigid',parameterCount:6,metric:'nmi',scoreBefore:1,scoreAfter:2,candidatesEvaluated:1,warnings:[]};pending.fit=candidate
 const sessionInput=w.document.querySelector('#load-session');Object.defineProperty(sessionInput,'files',{configurable:true,value:[{text:async()=>JSON.stringify(pending)}]});sessionInput.dispatchEvent(new w.Event('change'));await new Promise(r=>setTimeout(r,200))
 w.document.querySelector('[data-workflow="manual"]').click();await new Promise(r=>setTimeout(r,30));assert.equal(w.document.querySelector('[data-manual-target="ct"]').disabled,true);assert.equal(w.document.querySelector('#manual-proposal-actions').classList.contains('hidden'),false)
 w.document.querySelector(accept?'#manual-accept-proposal':'#manual-reject-proposal').click();await waitFor(()=>!w.document.querySelector('[data-manual-target="ct"]').disabled,'Deciding a proposal must unlock manual controls after overlay completion');assert.equal(w.document.querySelector('[data-manual-target="ct"]').disabled,false);w.document.querySelector('#save-session').click();await new Promise(r=>setTimeout(r,20));const decided=JSON.parse(saved.at(-1).text);assert.equal(decided.fit.proposal,null);assert.equal(decided.fit.matrix[0][3],accept?.25:0)
 w.document.querySelector('#nudge-controls [data-param="0"][data-sign="1"]').click();await new Promise(r=>setTimeout(r,180));w.document.querySelector('#save-session').click();await new Promise(r=>setTimeout(r,20));assert.notEqual(JSON.parse(saved.at(-1).text).fit.matrix[0][3],decided.fit.matrix[0][3],'The unlocked manual control must actually change the transform')
}
// Audit: exact compiled handler restores legacy pending proposals, including manual pose.
const legacyPending=structuredClone(acceptedSession);legacyPending.fit.manual=[2,0,0,0,0,0];legacyPending.fit.matrix[0][3]=9;legacyPending.fit.inverse[0][3]=-9;legacyPending.fit.proposal={matrix:structuredClone(legacyPending.fit.matrix),scoreBefore:1,scoreAfter:2,params:[7,0,0,0,0,0]}
const legacyInput=w.document.querySelector('#load-session');Object.defineProperty(legacyInput,'files',{configurable:true,value:[{text:async()=>JSON.stringify(legacyPending)}]});legacyInput.dispatchEvent(new w.Event('change'));await new Promise(r=>setTimeout(r,200));w.document.querySelector('#reject-refine').click();await new Promise(r=>setTimeout(r,180));w.document.querySelector('#save-session').click();await new Promise(r=>setTimeout(r,20));assert.equal(JSON.parse(saved.at(-1).text).fit.matrix[0][3],2,'Legacy Reject must restore the accepted manual pose')
// Reset-to-original must undo affine model promotion as well as the matrix.
w.document.querySelector('[data-manual-target="ct"]').click();const auditScale=w.document.querySelector('#param-6');auditScale.value='120';auditScale.dispatchEvent(new w.Event('change'));await new Promise(r=>setTimeout(r,180));w.document.querySelector('#reset-landmark').click();await new Promise(r=>setTimeout(r,180));w.document.querySelector('#save-session').click();await new Promise(r=>setTimeout(r,20));const originalReset=JSON.parse(saved.at(-1).text).fit;assert.equal(originalReset.model,'rigid');assert.equal(originalReset.parameterCount,6);assert.equal(originalReset.manualAffine,undefined);assert.deepEqual(originalReset.matrix,acceptedSession.fit.landmarkMatrix)
// The exact bundle restores pairing metadata without renaming source filenames.
w.document.querySelector('#save-session').click();await new Promise(r=>setTimeout(r,20));const pairingSession=JSON.parse(saved.at(-1).text);pairingSession.registrationPairing='t1-t2';const priorSourceName=w.document.querySelector('#ct-name').textContent;Object.defineProperty(legacyInput,'files',{configurable:true,value:[{text:async()=>JSON.stringify(pairingSession)}]});legacyInput.dispatchEvent(new w.Event('change'));await new Promise(r=>setTimeout(r,200));assert.equal(w.document.querySelector('#ct-name').textContent,priorSourceName);assert.equal(w.document.querySelector('#ct-coords').parentElement.querySelector('strong').textContent,'T2');assert.equal(w.document.querySelector('#direction option[value="ct-mri"]').textContent,'T2 → T1');w.document.querySelector('#save-session').click();await new Promise(r=>setTimeout(r,20));assert.equal(JSON.parse(saved.at(-1).text).registrationPairing,'t1-t2');w.document.querySelector('#save-transform').click();await new Promise(r=>setTimeout(r,80));const pairingExport=JSON.parse(saved.filter(f=>f.filename==='brainana-align_registration.json').at(-1).text);assert.equal(pairingExport.registration_pairing,'t1-t2');assert.equal(pairingExport.moving_space,'T2');assert.equal(pairingExport.fixed_space,'T1')
// JIP's actual compiled review handlers must restore the prior fit or empty state.
for(const accept of [false,true])for(const prior of [null,acceptedSession.fit]){
 const pending=structuredClone(acceptedSession),candidate=structuredClone(acceptedSession.fit),matrix=[[1,0,0,.25],[0,1,0,0],[0,0,1,0],[0,0,0,1]]
 candidate.model='jip';candidate.parameterCount=1536;candidate.initializationSource='automatic';candidate.landmarkSnapshot=[];candidate.landmarkModel=undefined;candidate.matrix=matrix;candidate.landmarkMatrix=matrix;candidate.baseMatrix=matrix;candidate.inverse=[[1,0,0,-.25],[0,1,0,0],[0,0,1,0],[0,0,0,1]];candidate.manual=[0,0,0,0,0,0];delete candidate.manualAffine;delete candidate.automaticRegistration
 candidate.nonlinear={version:1,kernel:'jip-dense-pull',baseAffine:matrix,dims:[8,8,8],affine:[[1,0,0,0],[0,1,0,0],[0,0,1,0],[0,0,0,1]],residualBase64:Buffer.alloc(6144).toString('base64'),settings:{axes:'z'},execution:{active:'cpu'},warnings:[]};candidate.proposal={kind:'jip',matrix,params:[],scoreBefore:0,scoreAfter:0,previous:structuredClone(prior)};pending.fit=candidate
 Object.defineProperty(legacyInput,'files',{configurable:true,value:[{text:async()=>JSON.stringify(pending)}]});legacyInput.dispatchEvent(new w.Event('change'));await new Promise(r=>setTimeout(r,250));assert.equal(w.document.querySelector('[data-manual-target="ct"]').disabled,true)
 w.document.querySelector(accept?'#manual-accept-proposal':'#manual-reject-proposal').click();await new Promise(r=>setTimeout(r,220));w.document.querySelector('#save-session').click();await new Promise(r=>setTimeout(r,30));let decided=JSON.parse(saved.at(-1).text).fit
 if(!accept){assert.deepEqual(decided?.matrix??null,prior?.matrix??null);assert.equal(w.document.querySelector('#manual-start-section').classList.contains('hidden'),!!prior)}else{assert.equal(decided.model,'jip');assert.equal(decided.proposal,null);w.document.querySelector('#nudge-controls [data-param="0"][data-sign="1"]').click();await new Promise(r=>setTimeout(r,220));w.document.querySelector('#save-session').click();await new Promise(r=>setTimeout(r,30));decided=JSON.parse(saved.at(-1).text).fit;assert.equal(decided.model,'jip');assert(decided.manualAffine,'Manual adjustment after JIP must be enabled and serializable');assert.notEqual(decided.matrix[0][3],.25)}
}
// Suggested acquisition settings must cleanly leave the prepared-image profile.
// Exercise defaults through the exact compiled frontend, without proxying a run.
const profileWorkers=emittedWorkerRequests.length,profilePanels=[...w.document.querySelectorAll('.sidebar details')].map(p=>p.open),profileFit=w.document.querySelector('#fit-summary').textContent
for(const [id,model,metric,bins] of [['afni','affine','hellinger','64'],['fsl','affine','correlation-ratio','256'],['ants','affine','parzen-mi','32'],['jip','rigid','mi','50'],['elastix','affine','parzen-mi','32']]){
 const chooser=w.document.querySelector('#linear-defaults-select');chooser.value=id;chooser.dispatchEvent(new w.Event('change',{bubbles:true}));w.document.querySelector('#linear-defaults-apply').click();assert.equal(w.document.querySelector('#auto-linear-model').value,model);assert.equal(w.document.querySelector('#auto-metric').value,metric);assert.equal(w.document.querySelector('#auto-bins').value,bins);for(const input of w.document.querySelectorAll('#linear-registration input[type=number]:enabled'))assert(input.checkValidity(),id+': '+input.id+' must accept exact profile values');assert.equal(emittedWorkerRequests.length,profileWorkers,'Applying defaults cannot run registration');assert.equal(w.document.querySelector('#fit-summary').textContent,profileFit);assert.deepEqual([...w.document.querySelectorAll('.sidebar details')].map(p=>p.open),profilePanels,'Defaults cannot expand or collapse guide/sidebar sections')
}
w.document.querySelector('#auto-bins').value='64';w.document.querySelector('#auto-bins').dispatchEvent(new w.Event('change',{bubbles:true}));assert(w.document.querySelector('#linear-defaults-current').textContent.includes('modified'))
const preparedWorkerCount=emittedWorkerRequests.length;w.document.querySelector('#auto-prepared-settings').click();assert.equal(w.document.querySelector('#auto-search-mode').value,'nearby');assert.equal(w.document.querySelector('#auto-foreground').value,'fixed-nonzero');assert.equal(w.document.querySelector('#auto-normalization').value,'mean');assert(w.document.querySelector('#auto-pyramid').disabled);w.document.querySelector('#acquisition-use-settings').click();assert.equal(w.document.querySelector('#auto-search-mode').value,'broad');assert.equal(w.document.querySelector('#auto-normalization').value,'percentile');assert.equal(w.document.querySelector('#auto-range').value,'180');assert(!w.document.querySelector('#auto-pyramid').disabled);assert.equal(emittedWorkerRequests.length,preparedWorkerCount,'Selecting either settings profile must never execute registration');await new Promise(r=>setTimeout(r,30))
console.log('Production bundle DOM validation passed: all four landmark options, rigid default, exact fits, diagnostics, reflection apply/export confirmation. WebGL rendering is not certified by this DOM test.')
dom.window.close()
