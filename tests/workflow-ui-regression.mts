import {finishWorkingImageCommit} from '../apps/align/src/workingImageCommit'
import { REVIEW_BLEND_SHADER } from '../apps/align/src/reviewBlendShader.ts'
import assert from 'node:assert/strict'
import { createRequire } from 'node:module'
import { configureReviewAppearance, updateReviewOpacity } from '../apps/align/src/reviewAppearance.ts'
import { createAlignmentActivity, installWorkflowFocus } from '../apps/align/src/workflowUI.ts'
import { alignmentLayout } from '../apps/align/src/alignmentLayout.ts'
import type { Niivue } from '@niivue/niivue'
const require=createRequire(import.meta.url),{JSDOM}=require('jsdom')
let uploads=0,draws=0,shaderInstalls=0
const views=Array.from({length:3},()=>({volumes:[{opacity:.2,colormap:'gray'},{opacity:.3,colormap:'gray'}],overlayAlphaShader:1,setCustomSliceShader(source:string){assert.equal(source,REVIEW_BLEND_SHADER);shaderInstalls++},updateGLVolume(){uploads++},drawScene(){draws++}}))
const viewers=views as unknown as Niivue[]
configureReviewAppearance(viewers,'falsecolor',.55)
assert.equal(uploads,3);assert.ok(views.every(v=>v.volumes[0].colormap==='green'&&v.volumes[1].colormap==='brainanaMagenta'&&v.volumes[1].opacity===1))
for(const alpha of [0,.01,.25,.9,1]){updateReviewOpacity(viewers,alpha);assert.ok(views.every(v=>v.overlayAlphaShader===alpha))}
assert.equal(uploads,3,'Opacity gestures must never rebuild textures');assert.equal(draws,15)
configureReviewAppearance(viewers,'opacity',.4);assert.equal(uploads,6);assert.equal(shaderInstalls,3,'Compile once per viewer, not on slider/mode changes');assert.ok(views.every(v=>v.volumes.every(i=>i.colormap==='gray'&&i.opacity===1)))
updateReviewOpacity(viewers,2);assert.ok(views.every(v=>v.overlayAlphaShader===1));updateReviewOpacity(viewers,-1);assert.ok(views.every(v=>v.overlayAlphaShader===0))
// Evaluate the actual shader's scalar blend statements, rather than a separate implementation.
// This catches using the opacity control on fixedAlpha, which gives incorrect 0/1 endpoints.
const statements=REVIEW_BLEND_SHADER.slice(REVIEW_BLEND_SHADER.indexOf('  float fixedAlpha='),REVIEW_BLEND_SHADER.indexOf('  color=')).replace(/float /g,'let ')
const scalarBlend=new Function('fixedSample','movingSample','overlayAlphaShader','opacity','isAlphaClipDark','backgroundMasksOverlays','clamp',statements+'return {fixedAlpha,movingAlpha,outAlpha,movingWeight}')
const blend=(fixedA:number,movingA:number,alpha:number)=>scalarBlend({a:fixedA},{a:movingA},alpha,1,false,0,(x:number,l:number,h:number)=>Math.max(l,Math.min(h,x)))
for(const [alpha,expected] of [[0,0],[.25,.25],[1,1]]){const v=blend(1,1,alpha);assert.equal(v.fixedAlpha,1);assert.equal(v.movingWeight,expected);assert.equal(v.outAlpha,1)}
assert.equal(blend(1,.5,.5).movingWeight,.25)
assert.equal(blend(1,0,1).movingWeight,0,'Empty moving samples retain the fixed image')
for(const name of ['volume','overlay','drawing','paqd','paqdUniforms','colormap','drawOpacity','drawRimOpacity','overlayOutlineWidth','overlayAlphaShader','isAlphaClipDark','backgroundMasksOverlays','opacity','axCorSag','overlays'])assert.ok(new RegExp('uniform[^;]+ '+name+';').test(REVIEW_BLEND_SHADER),'Niivue uniform contract: '+name)
const fallback={volumes:[{opacity:1,colormap:'gray'},{opacity:1,colormap:'gray'}],overlayAlphaShader:1,setCustomSliceShader(source:string){if(source)throw new Error('Test shader rejection')},updateGLVolume(){},drawScene(){}}
const warn=console.warn;console.warn=()=>{}
try{configureReviewAppearance([fallback as unknown as Niivue],'opacity',.6);updateReviewOpacity([fallback as unknown as Niivue],0);assert.equal(fallback.volumes[1].opacity,0);assert.equal(fallback.overlayAlphaShader,1);updateReviewOpacity([fallback as unknown as Niivue],1);assert.equal(fallback.volumes[1].opacity,1)}finally{console.warn=warn}
const dom=new JSDOM('<div id="app"></div>',{pretendToBeVisual:true})
Object.assign(globalThis,{window:dom.window,document:dom.window.document,requestAnimationFrame:dom.window.requestAnimationFrame.bind(dom.window)})
document.querySelector('#app')!.innerHTML=alignmentLayout('test');installWorkflowFocus()
const click=(mode:string)=>document.querySelector<HTMLButtonElement>(`[data-workflow=${mode}]`)!.click()
click('automatic');assert.equal(document.querySelector<HTMLDetailsElement>('#refine-panel')!.open,true)
for(const id of ['landmarks-panel','landmark-alignment-panel','manual-panel'])assert.equal(document.querySelector<HTMLDetailsElement>('#'+id)!.open,false)
click('manual');assert.equal(document.querySelector<HTMLDetailsElement>('#manual-panel')!.open,true);assert.equal(document.querySelector<HTMLDetailsElement>('#refine-panel')!.open,false)
click('landmarks');assert.equal(document.querySelector<HTMLDetailsElement>('#landmarks-panel')!.open,true)
const activity=createAlignmentActivity(),banner=document.querySelector('#alignment-progress')!,bar=banner.querySelector('progress')!
assert.equal(await activity.begin('Fitting'),true);assert.equal(activity.busy,true);assert.ok(!banner.classList.contains('hidden'));assert.equal(document.querySelector('main')!.inert,true)
assert.equal(await activity.begin('Duplicate'),false);assert.equal(document.querySelector('#alignment-progress-label')!.textContent,'Fitting')
await activity.phase('Search',.5);assert.equal(bar.value,.5)
await activity.phase('Overlay');assert.equal(bar.hasAttribute('value'),false)
activity.end();assert.equal(activity.busy,false);assert.equal(document.querySelector('main')!.inert,false);assert.ok(banner.classList.contains('hidden'))
await activity.begin('Updating working CT image…','processing');assert.equal(activity.kind,'processing');assert.equal(document.querySelector('#alignment-cancel')!.textContent,'Cancel preprocessing');assert.equal(bar.getAttribute('aria-label'),'Preprocessing progress');activity.end();await activity.begin('Refreshing overlay','display');assert.equal(document.querySelector('#alignment-cancel')!.textContent,'Cancel overlay refresh');activity.end();
let clears=0,refreshes=0,invalidations=0;const hooks={clearFit(){clears++},async refreshOverlay(){refreshes++},invalidateOverlay(){invalidations++}};
await finishWorkingImageCommit('processing',[],true,hooks);assert.deepEqual([clears,refreshes,invalidations],[0,0,1],'Processing must never launch/refit/resample an overlay');await finishWorkingImageCommit('processing',[],false,hooks);assert.deepEqual([clears,refreshes,invalidations],[0,0,1],'No alignment can be created by preprocessing');await finishWorkingImageCommit('preparation',[],true,hooks);assert.equal(refreshes,1);await finishWorkingImageCommit('preparation',['ct'],true,hooks);assert.equal(clears,1);
dom.window.close()
console.log('Workflow regressions passed: stable panel focus, visible/painted progress and operation guard, three-pane opacity at 0/1 with zero texture rebuilds, mode-specific texture setup.')
