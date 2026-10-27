import assert from 'node:assert/strict'
import {createRequire} from 'node:module'
import {installOptimizationWindowCapture} from '../apps/align/src/optimizationWindowInteraction'
import {windowPointFromCanvas,windowRectangleOnCanvas} from '../apps/align/src/optimizationWindowGeometry'
import {planeAxes,registrationWindows,withinOptimizationWindows} from '../apps/align/src/optimizationWindows'
import {createSessionPayload,parseSessionPayload} from '../apps/align/src/sessionPersistence'
import type {View,Plane,Modality,WindowBounds} from '../apps/align/src/appTypes'
const {JSDOM}=createRequire(import.meta.url)('jsdom')
for(const modality of ['mri','ct'] as Modality[])for(const plane of ['axial','coronal','sagittal'] as Plane[])for(const mirrored of [false,true]){
 const dom=new JSDOM('<canvas></canvas><div id="layer"></div>',{pretendToBeVisual:true})
 Object.assign(globalThis,{window:dom.window,document:dom.window.document,AbortController:dom.window.AbortController})
 const layer=dom.window.document.querySelector<HTMLDivElement>('#layer')!,canvas=dom.window.document.querySelector<HTMLCanvasElement>('canvas')!
 canvas.width=800;canvas.height=600
 Object.defineProperty(layer,'getBoundingClientRect',{value:()=>({left:0,top:0,width:400,height:300})})
 const [a,b,depth]=planeAxes[plane],slice:any={axCorSag:{axial:0,coronal:1,sagittal:2}[plane],leftTopWidthHeight:mirrored?[700,40,-600,480]:[100,40,600,480],leftTopMM:[-30,-40],fovMM:[120,100]}
 // Oblique, anisotropic physical crosshair; the fractional depth is not a world axis.
 const view:View={modality,plane,canvas,windowLayer:layer,overlay:null as any,nv:{screenSlices:[slice],scene:{crosshairPos:[.2,.4,.7]},frac2mm:(f:number[])=>[.8*f[0]-.4*f[1]+.1*f[2]+11,.6*f[0]+1.2*f[1]+.2*f[2]-8,.1*f[0]+.2*f[1]+1.8*f[2]+25],canvasPos2frac:()=>[-1,-1,-1]} as any}
 const center=Array.from(view.nv.frac2mm(view.nv.scene.crosshairPos))
 const inside=windowPointFromCanvas(view,300,200)!;assert.equal(inside[depth],center[depth]);assert.equal(windowPointFromCanvas(view,-100,-100),null)
 const edge=windowPointFromCanvas(view,2000,2000,true)!;assert.equal(edge[a],mirrored?-30:90);assert.equal(edge[b],-40)
 let saved:WindowBounds|undefined,rejected=0,commits=0,enabled=true
 const dispose=installOptimizationWindowCapture({view,isEnabled:()=>enabled,getPixDims:()=>[.3,.5,10],eventToMm:e=>windowPointFromCanvas(view,e.clientX*2,e.clientY*2,true),onCommit:bounds=>{saved=bounds;commits++},onReject:()=>{rejected++}})
 const pointer=(type:string,x:number,y:number,id=1,button=0)=>{const e=new dom.window.Event(type,{bubbles:true,cancelable:true});Object.assign(e,{clientX:x,clientY:y,pointerId:id,button,isPrimary:true});(type==='pointerdown'?layer:dom.window.document).dispatchEvent(e)}
 pointer('pointerdown',100,80);pointer('pointermove',270,190);pointer('pointerup',1000,1000)
 assert.equal(commits,1,`${modality} ${plane} release outside panel`);assert(saved)
 assert.equal(saved.min[depth],center[depth]);assert.equal(saved.max[depth],center[depth]);assert.deepEqual(registrationWindows({[plane]:saved})[0].axes,[a,b])
 const expected=saved;assert(windowRectangleOnCanvas(view,saved));const windows={mri:modality==='mri'?{[plane]:saved}:{},ct:modality==='ct'?{[plane]:saved}:{}}
 const payload=createSessionPayload({appVersion:'test',loaded:{mri:null,ct:null},landmarks:[],selectedId:null,nextId:1,direction:'ct-mri',fit:null,optimizationWindows:windows})
 assert.deepEqual(parseSessionPayload(JSON.parse(JSON.stringify(payload))).optimizationWindows,windows)
 pointer('pointerdown',100,80);pointer('pointerup',101,81);assert.equal(rejected,1);assert.equal(saved,expected,'Rejected replacement preserves existing window')
 pointer('pointerdown',100,80);pointer('pointermove',220,170);pointer('pointercancel',0,0);assert.equal(commits,2,'Cancel commits last valid drag, ignoring cancellation coordinates')
 const rect=windowRectangleOnCanvas(view,saved!)!;assert(rect.width>0&&rect.height>0)
 // Zoom/pan puts both physical corners outside the field of view: render a
 // full-size rectangle and let CSS clip it; do not erase or clamp its bounds.
 const before=structuredClone(saved);slice.leftTopMM=[0,0];slice.fovMM=[5,5]
 const zoomed=windowRectangleOnCanvas(view,saved!)!;assert(zoomed.width>slice.leftTopWidthHeight[2]||mirrored);assert.deepEqual(saved,before)
 assert(withinOptimizationWindows([(saved!.min[0]+saved!.max[0])/2,(saved!.min[1]+saved!.max[1])/2,(saved!.min[2]+saved!.max[2])/2],{[plane]:saved}))
 enabled=false;pointer('pointerdown',100,80);pointer('pointerup',270,190);assert.equal(commits,2)
 enabled=true;pointer('pointerdown',100,80,2);dispose();pointer('pointerup',270,190,2);assert.equal(commits,2,'Disposal removes pending document capture')
 dom.window.close()
}
console.log('Real DOM capture: all six MRI/CT planes, mirrored display, CSS/backing scaling, physical oblique depth, outside release, cancel, tiny replacement preservation, zoom/pan persistence, worker axes, session restore and capture disposal passed.')
