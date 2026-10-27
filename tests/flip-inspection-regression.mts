import assert from 'node:assert/strict'
import {JSDOM} from 'jsdom'
import {installOpacityCycle} from '../apps/align/src/opacityCycle'
import {configureReviewAppearance,updateReviewFlip,updateReviewOpacity} from '../apps/align/src/reviewAppearance'
import {REVIEW_BLEND_SHADER} from '../apps/align/src/reviewBlendShader'
const dom=new JSDOM('<select id="mode"><option value="smooth">Smooth</option><option value="flip">Flip</option></select><button id="play"></button><button id="flip"></button><p id="label"></p><input id="slider" type="range" value="55">',{pretendToBeVisual:true})
Object.assign(globalThis,{window:dom.window,document:dom.window.document})
const d=dom.window.document,mode=d.querySelector<HTMLSelectElement>('#mode')!,play=d.querySelector<HTMLButtonElement>('#play')!,flip=d.querySelector<HTMLButtonElement>('#flip')!,label=d.querySelector<HTMLElement>('#label')!,slider=d.querySelector<HTMLInputElement>('#slider')!
let nextId=0;const frames=new Map<number,FrameRequestCallback>();Object.assign(globalThis,{requestAnimationFrame:(cb:FrameRequestCallback)=>{frames.set(++nextId,cb);return nextId},cancelAnimationFrame:(id:number)=>frames.delete(id)})
const tick=(t:number)=>{const [id,cb]=[...frames][0];frames.delete(id);cb(t)}
let uploads=0,draws=0,available=true,names={fixed:'MRI',moving:'CT'}
const viewers:any[]=Array.from({length:3},()=>({volumes:[{img:new Float32Array([1,2]),opacity:1},{img:new Float32Array([3,4]),opacity:1}],scene:{crosshairPos:[.2,.3,.4],pan2Dxyzmm:[1,2,3,2]},setCustomSliceShader:()=>{},addColormap:()=>{},updateGLVolume:()=>uploads++,drawScene:()=>draws++,overlayAlphaShader:1}))
configureReviewAppearance(viewers,'opacity',.55);const before=structuredClone(viewers.map(v=>({volumes:v.volumes,scene:v.scene}))),initialUploads=uploads
const controller=installOpacityCycle(slider,play,()=>updateReviewOpacity(viewers,Number(slider.value)/100),{mode,flip,label,available:()=>available,names:()=>names,show:image=>updateReviewFlip(viewers,image)})
mode.value='flip';mode.dispatchEvent(new dom.window.Event('change'));assert(slider.disabled);assert(viewers.every(v=>v.overlayAlphaShader===-1));assert.equal(label.textContent,'MRI only (fixed)')
flip.click();assert(viewers.every(v=>v.overlayAlphaShader===2));assert.equal(label.textContent,'CT only (moving)');assert.equal(slider.value,'55','Flip never overwrites saved blend opacity')
play.click();tick(0);const stable=draws;tick(999);assert.equal(draws,stable,'No extra draws between flip boundaries');tick(1000);assert(viewers.every(v=>v.overlayAlphaShader===-1));tick(2000);assert(viewers.every(v=>v.overlayAlphaShader===2));assert.equal(frames.size,1)
flip.click();assert.equal(frames.size,0);assert.equal(play.getAttribute('aria-pressed'),'false');assert(viewers.every(v=>v.overlayAlphaShader===-1))
names={fixed:'CT',moving:'MRI'};controller.refresh();assert.equal(label.textContent,'CT only (fixed)','Pair names follow either transform direction')
play.click();dom.window.dispatchEvent(new dom.window.Event('pagehide'));assert.equal(frames.size,0)
play.click();Object.defineProperty(d,'hidden',{configurable:true,value:true});d.dispatchEvent(new dom.window.Event('visibilitychange'));assert.equal(frames.size,0);Object.defineProperty(d,'hidden',{configurable:true,value:false})
play.click();dom.window.dispatchEvent(new dom.window.Event('align-workspace-reset'));assert.equal(frames.size,0)
mode.value='smooth';mode.dispatchEvent(new dom.window.Event('change'));assert(!slider.disabled);assert(viewers.every(v=>v.overlayAlphaShader===.55));assert.equal(uploads,initialUploads,'Repeated flips use uniforms and draws, never texture rebuilds')
assert.deepEqual(viewers.map(v=>({volumes:v.volumes,scene:v.scene})),before,'Pixels, image metadata and all three camera/crosshair states stay unchanged')
play.click();tick(0);assert.equal(slider.value,'25');tick(4000/1.4);assert.equal(slider.value,'75');slider.dispatchEvent(new dom.window.Event('input'));assert.equal(frames.size,0)
available=false;controller.refresh();assert(play.disabled&&flip.disabled);assert(label.textContent!.includes('Create an aligned overlay'))
// Evaluate the exact pure-image shader branches, including transparent moving
// background. It must never reveal the fixed RGB when moving alone is selected.
const branches=REVIEW_BLEND_SHADER.slice(REVIEW_BLEND_SHADER.indexOf('  // Reserved endpoints:'),REVIEW_BLEND_SHADER.indexOf('  float fixedAlpha=')).replaceAll('return;','return color;')
const shader=new Function('fixedSample','movingSample','overlayAlphaShader','opacity','isAlphaClipDark','clamp','vec4','let color;'+branches+'return color;')
const run=(control:number,alpha:number)=>shader({rgb:[.9,.8,.7],a:1},{rgb:[.1,.2,.3],a:alpha},control,1,false,(v:number,l:number,h:number)=>Math.max(l,Math.min(h,v)),(...args:any[])=>Array.isArray(args[0])?[...args[0],args[1]]:args)
assert.deepEqual(run(-1,0),[.9,.8,.7,1]);assert.deepEqual(run(2,0),[0,0,0,1]);assert.deepEqual(run(2,.5),[.1,.2,.3,1]);assert.equal(run(.55,1),undefined)
const fallback:any={volumes:[{opacity:1},{opacity:1}],setCustomSliceShader:(s:string)=>{if(s)throw Error('shader unavailable')},updateGLVolume:()=>{},drawScene:()=>{}};const warn=console.warn;console.warn=()=>{};try{configureReviewAppearance([fallback],'opacity',.55)}finally{console.warn=warn}
updateReviewFlip([fallback],'moving');assert.equal(fallback.volumes[0].opacity,0);assert.equal(fallback.volumes[1].opacity,1);updateReviewOpacity([fallback],.55);assert.equal(fallback.volumes[0].opacity,1);assert.equal(fallback.volumes[1].opacity,.55)
dom.window.close();console.log('Flip inspection: manual/1-second automatic switching, all three panes, either direction, pure-image/background shader branches, opacity restoration, no texture rebuilds/camera/data mutation, lifecycle cleanup and shader fallback passed. Native GPU pixels pending.')
