import assert from 'node:assert/strict'
import fs from 'node:fs'
import path from 'node:path'
import ts from 'typescript'
import {JSDOM} from 'jsdom'
import {Niivue,SLICE_TYPE,DRAG_MODE} from '@niivue/niivue'
import {imageFromRaw,sharedViewerImage} from '../apps/align/src/volumeImages'
import {installSliceCrosshairs} from '../apps/align/src/sliceCrosshairs'
import {installWheelZoomAndSlice,observeProjectionDraw} from '../apps/align/src/viewInteraction'
import {SLICE_ZOOM_LIMITS,zoomSliceView} from '../apps/align/src/viewZoom'
import type {RawNifti} from '../apps/align/src/roiWarp'

// Execute Niivue's actual draw2DMain, crosshair geometry and drawLine methods.
// A command recorder verifies geometry/state without claiming native GPU pixels.
const dist=path.resolve(process.argv[2]??'apps/align/dist')
const html=fs.readFileSync(path.join(dist,'index.html'),'utf8'),entry=html.match(/src="\/assets\/([^"]+\.js)"/)![1]
const bundle=fs.readFileSync(path.join(dist,'assets',entry),'utf8'),tree=ts.createSourceFile(entry,bundle,ts.ScriptTarget.Latest,true,ts.ScriptKind.JS)
const functions=tree.statements.filter(ts.isFunctionDeclaration)
const adapterNode=functions.find(n=>n.getText(tree).includes('.drawCrosshairs3D=function'))
const zoomNode=functions.find(n=>n.getText(tree).includes('pan2Dxyzmm')&&n.getText(tree).includes('Math.exp(-Math.max(-240'))
assert(adapterNode&&zoomNode,'The emitted app must contain both viewer safeguards')
const emittedAdapter=new Function('return ('+adapterNode.getText(tree)+')')() as typeof installSliceCrosshairs
const zoomText=zoomNode.getText(tree),boundName=zoomText.match(/Math\.min\(([$\w]+)\.max,Math\.max\(\1\.min/)![1]
let boundNode:ts.VariableDeclaration|undefined
function findBound(n:ts.Node){if(ts.isVariableDeclaration(n)&&n.name.getText(tree)===boundName)boundNode=n;ts.forEachChild(n,findBound)}findBound(tree)
assert(boundNode?.initializer&&ts.isObjectLiteralExpression(boundNode.initializer))
const bounds=Object.fromEntries(boundNode.initializer.properties.map(p=>{assert(ts.isPropertyAssignment(p)&&ts.isNumericLiteral(p.initializer));return [p.name.getText(tree),Number(p.initializer.text)]}))
assert.deepEqual(bounds,SLICE_ZOOM_LIMITS)
const emittedZoom=new Function(boundName,'return ('+zoomText+')')(bounds) as typeof zoomSliceView

const dom=new JSDOM('<body/>',{pretendToBeVisual:true,url:'http://localhost'})
Object.assign(globalThis,{window:dom.window,document:dom.window.document,localStorage:dom.window.localStorage,requestAnimationFrame:dom.window.requestAnimationFrame.bind(dom.window),cancelAnimationFrame:dom.window.cancelAnimationFrame.bind(dom.window)})
type Draw={kind:string;uniforms:Record<string,any>;vertices?:Float32Array;indices?:Uint32Array}
function recordingGL(width=640,height=416){
 let id=0,vao:any=null,array:any=null,kind='',mask=true
 const constants=new Map<string,number>(),buffers=new Map<any,any>(),vaos=new Map<any,any>(),enabled=new Set<number>(),uniforms:Record<string,any>={},parameters=new Map<number,number>()
 const draws:Draw[]=[]
 const gl:any=new Proxy({canvas:{width,height,getBoundingClientRect:()=>({width:width/2,height:height/2})},
  createBuffer:()=>({id:++id}),createVertexArray:()=>({id:++id}),bindVertexArray:(v:any)=>{vao=v},
  bindBuffer:(t:number,b:any)=>{if(t===gl.ARRAY_BUFFER)array=b;else{if(!vaos.has(vao))vaos.set(vao,{});vaos.get(vao).index=b}},
  bufferData:(t:number,data:any)=>buffers.set(t===gl.ARRAY_BUFFER?array:vaos.get(vao)?.index,data),
  vertexAttribPointer:()=>{if(!vaos.has(vao))vaos.set(vao,{});vaos.get(vao).vertices=array},
  uniformMatrix4fv:(key:string,_:any,value:any)=>{uniforms[key]=Array.from(value)},
  uniform4fv:(key:string,value:any)=>{uniforms[key]=Array.from(value)},uniform2fv:(key:string,value:any)=>{uniforms[key]=Array.from(value)},uniform1f:(key:string,value:any)=>{uniforms[key]=value},
  drawElements:()=>draws.push({kind,uniforms:{...uniforms},vertices:buffers.get(vaos.get(vao)?.vertices),indices:buffers.get(vaos.get(vao)?.index)}),
  drawArrays:()=>draws.push({kind,uniforms:{...uniforms}}),
  enable:(p:number)=>enabled.add(p),disable:(p:number)=>enabled.delete(p),isEnabled:(p:number)=>enabled.has(p),
  depthMask:(value:boolean)=>{mask=value},getParameter:(p:number)=>p===gl.DEPTH_WRITEMASK?mask:parameters.get(p)??0,
  blendFunc:(a:number,b:number)=>{for(const p of [gl.BLEND_SRC_RGB,gl.BLEND_SRC_ALPHA])parameters.set(p,a);for(const p of [gl.BLEND_DST_RGB,gl.BLEND_DST_ALPHA])parameters.set(p,b)},
  blendFuncSeparate:(a:number,b:number,c:number,d:number)=>{[gl.BLEND_SRC_RGB,gl.BLEND_DST_RGB,gl.BLEND_SRC_ALPHA,gl.BLEND_DST_ALPHA].forEach((p,i)=>parameters.set(p,[a,b,c,d][i]))}
 },{get:(o,k)=>{if(k in o)return o[k];if(String(k)===String(k).toUpperCase()){if(!constants.has(String(k)))constants.set(String(k),constants.size+1);return constants.get(String(k))}return ()=>{}}})
 const shader=(name:string)=>({use:()=>{kind=name},uniforms:new Proxy({},{get:(_,key)=>key})})
 return {gl,draws,shader}
}
const raw:RawNifti={dims:[67,96,96],pixDims:[1,1,1],affine:[[-1,0,0,33.46],[0,-1,0,53.81],[0,0,1,-19.91],[0,0,0,1]],values:new Float32Array(67*96*96)}
const source=imageFromRaw(raw,'synthetic MRI'),original=structuredClone(raw.affine)
function viewer(type:number,adapter=true,image=source,width=640,height=416){
 const {gl,draws,shader}=recordingGL(width,height),nv:any=new Niivue({show3Dcrosshair:false,isSliceMM:true,isColorbar:false})
 nv.gl=gl;nv.canvas=gl.canvas;nv.canvas.focus=()=>{};nv.volumes=[sharedViewerImage(image)];nv.back=nv.volumes[0];nv.back.pixDims=nv.back.pixDimsRAS;nv.back.dims=nv.back.dimsRAS
 const corners=[];for(const x of [-.5,nv.back.dimsRAS[1]-.5])for(const y of [-.5,nv.back.dimsRAS[2]-.5])for(const z of [-.5,nv.back.dimsRAS[3]-.5])corners.push(nv.back.vox2mm([x,y,z],nv.back.matRAS))
 nv.volumeObject3D={extentsMin:[0,1,2].map(a=>Math.min(...corners.map(p=>p[a]))),extentsMax:[0,1,2].map(a=>Math.max(...corners.map(p=>p[a])))}
 for(const name of ['surfaceShader','sliceMMShader','lineShader'])nv[name]=shader(name)
 nv.drawSliceOrientationText=()=>{};nv.eventInBounds=()=>true;nv.inRenderTile=()=>-1;nv.sync=()=>{};nv.opts.dragMode=DRAG_MODE.pan;nv.opts.crosshairWidth=1
 nv.scene.crosshairPos=[.5,.5,.5]
 nv.drawScene=()=>{nv.screenSlices=[];nv.draw2DMain([0,0,0,0],type)}
 if(adapter)emittedAdapter(nv)
 return {nv,gl,draws}
}
function fullSurfaceFraction(draw:Draw){
 const m=draw.uniforms.mvpMtx,vs=draw.vertices!,idx=draw.indices!,points=[]
 for(let i=0;i<vs.length;i+=3){const x=vs[i],y=vs[i+1],z=vs[i+2];points.push([m[0]*x+m[4]*y+m[8]*z+m[12],m[1]*x+m[5]*y+m[9]*z+m[13]])}
 let covered=0
 for(let yi=0;yi<20;yi++)for(let xi=0;xi<20;xi++){
  const p=[-1+(xi+.5)/10,-1+(yi+.5)/10]
  for(let i=0;i<idx.length;i+=3){const a=points[idx[i]],b=points[idx[i+1]],c=points[idx[i+2]],sign=(u:number[],v:number[])=>(p[0]-v[0])*(u[1]-v[1])-(u[0]-v[0])*(p[1]-v[1]),v=[sign(a,b),sign(b,c),sign(c,a)];if(v.some(x=>x<0)&&v.some(x=>x>0))continue;covered++;break}
 }
 return covered/400
}
// Reproduce the failure using the dependency's actual original wheel handler.
const legacy=viewer(SLICE_TYPE.SAGITTAL,false);legacy.nv.drawScene=()=>{}
for(let i=0;i<100;i++)legacy.nv.wheelListener({deltaY:-.01,clientX:300,clientY:200,preventDefault(){},stopPropagation(){}})
assert(legacy.nv.scene.pan2Dxyzmm[3]>13000)
legacy.nv.draw2DMain([0,0,0,0],SLICE_TYPE.SAGITTAL)
const legacyCoverage=fullSurfaceFraction(legacy.draws.find(d=>d.kind==='surfaceShader'&&d.uniforms.surfaceColor[3]===1)!)
assert.equal(legacyCoverage,1,'The original crosshair geometry covers every sampled pixel in the pane')

// Compare the actual emitted camera function with the source implementation.
for(const initial of [.1,1,64,13743])for(const delta of [-240,-.01,0,.01,240,NaN]){
 const a=viewer(2),b=viewer(2);a.nv.scene.pan2Dxyzmm=[0,0,0,initial];b.nv.scene.pan2Dxyzmm=[0,0,0,initial]
 assert.equal(emittedZoom(a.nv,delta),zoomSliceView(b.nv,delta));assert.deepEqual(Array.from(a.nv.scene.pan2Dxyzmm),Array.from(b.nv.scene.pan2Dxyzmm))
}
let paneCases=0
for(const slot of ['mri','ct','review'])for(const [plane,type] of Object.entries({sagittal:SLICE_TYPE.SAGITTAL,coronal:SLICE_TYPE.CORONAL,axial:SLICE_TYPE.AXIAL})){
 const {nv,gl,draws}=viewer(type),canvas=dom.window.document.createElement('canvas');canvas.width=640;canvas.height=416
 const start=Array.from(nv.scene.crosshairPos),affine=structuredClone(nv.volumes[0].hdr.affine),pixels=nv.volumes[0].img
 const sibling=viewer(type),siblingPan=Array.from(sibling.nv.scene.pan2Dxyzmm)
 let overlayDraws=0;const dispose=observeProjectionDraw(nv,()=>overlayDraws++);installWheelZoomAndSlice(canvas,nv,()=>raw.dims,plane as any)
 let vendorEvents=0;canvas.addEventListener('wheel',()=>vendorEvents++)
 const wheel=(deltaY:number,deltaMode=0,shiftKey=false)=>canvas.dispatchEvent(new dom.window.WheelEvent('wheel',{deltaY,deltaMode,shiftKey,bubbles:true,cancelable:true}))
 for(let i=0;i<100;i++)wheel(-.01)
 assert(Math.abs(nv.scene.pan2Dxyzmm[3]-Math.exp(.0015))<1e-5,`${slot}/${plane}: tiny events must remain tiny`)
 assert.equal(vendorEvents,0,'Do not also deliver wheel zoom to Niivue')
 const tinyZoom=nv.scene.pan2Dxyzmm[3];wheel(0);zoomSliceView(nv,NaN);assert.equal(nv.scene.pan2Dxyzmm[3],tinyZoom)
 // Identical scroll distance split into events has the same effect.
 nv.scene.pan2Dxyzmm=[0,0,0,1];wheel(-120);const one=nv.scene.pan2Dxyzmm[3]
 nv.scene.pan2Dxyzmm=[0,0,0,1];for(let i=0;i<120;i++)wheel(-1);assert(Math.abs(nv.scene.pan2Dxyzmm[3]-one)<1e-5)
 nv.scene.pan2Dxyzmm=[0,0,0,1];wheel(-1,1);const line=nv.scene.pan2Dxyzmm[3];nv.scene.pan2Dxyzmm=[0,0,0,1];wheel(-16);assert.equal(nv.scene.pan2Dxyzmm[3],line);nv.scene.pan2Dxyzmm=[0,0,0,1];wheel(-.1,2);const page=nv.scene.pan2Dxyzmm[3];nv.scene.pan2Dxyzmm=[0,0,0,1];wheel(-80);assert.equal(nv.scene.pan2Dxyzmm[3],page)
 // Hit both bounds, and reverse immediately. Original zoom had no upper bound.
 for(const direction of [-1,1]){for(let i=0;i<70;i++)wheel(direction*240);assert(Math.abs(nv.scene.pan2Dxyzmm[3]-(direction<0?SLICE_ZOOM_LIMITS.max:SLICE_ZOOM_LIMITS.min))<1e-7);wheel(-direction*24);assert(nv.scene.pan2Dxyzmm[3]>SLICE_ZOOM_LIMITS.min&&nv.scene.pan2Dxyzmm[3]<SLICE_ZOOM_LIMITS.max)}
 assert.deepEqual(Array.from(nv.scene.crosshairPos),start);assert.deepEqual(nv.volumes[0].hdr.affine,affine);assert.equal(nv.volumes[0].img,pixels);assert.deepEqual(Array.from(sibling.nv.scene.pan2Dxyzmm),siblingPan)
 // Stable screen thickness at tiny, ordinary, extreme and legacy-corrupted zoom.
 for(const zoom of [.1,1,64,13743])for(const width of [1,5]){
  nv.scene.pan2Dxyzmm=[0,0,0,zoom];const center=nv.frac2mm(nv.scene.crosshairPos),[mn,mx]=nv.sceneExtentsMinMax(true)
  for(let a=0;a<3;a++)nv.scene.pan2Dxyzmm[a]=(mn[a]+mx[a])*.5-zoom*center[a]
  nv.opts.crosshairWidth=width;nv.opts.crosshairColor=slot==='mri'?[.25,.85,1,1]:slot==='ct'?[1,.7,.15,1]:[1,1,1,.85]
  draws.length=0;nv.drawScene()
  const lines=draws.filter(d=>d.kind==='lineShader');assert.equal(lines.length,2,`${slot}/${plane} ${zoom}: one pair of crosshairs`)
  assert.equal(draws.filter(d=>d.kind==='surfaceShader').length,0,'No 3D cylinders in orthogonal slices')
  for(const d of lines){assert.equal(d.uniforms.thickness,width*2);const [x0,y0,x1,y1]=d.uniforms.startXYendXY;assert([x0,y0,x1,y1].every(Number.isFinite));assert(x0>=0&&x1<=640&&y0>=0&&y1<=416);assert(x0===x1||y0===y1);assert.deepEqual(d.uniforms.lineColor,nv.opts.crosshairColor)}
 }
 // Crosshairs never alter or leak depth, blending, or write-mask state.
 gl.disable(gl.DEPTH_TEST);gl.disable(gl.BLEND);gl.depthMask(false);gl.blendFuncSeparate(7,8,9,10)
 nv.drawCrosshairs3D(true,1,null,true,true)
 assert.equal(gl.isEnabled(gl.DEPTH_TEST),false);assert.equal(gl.isEnabled(gl.BLEND),false);assert.equal(gl.getParameter(gl.DEPTH_WRITEMASK),false);assert.deepEqual([gl.BLEND_SRC_RGB,gl.BLEND_DST_RGB,gl.BLEND_SRC_ALPHA,gl.BLEND_DST_ALPHA].map(p=>gl.getParameter(p)),[7,8,9,10])
 nv.opts.crosshairWidth=0;draws.length=0;nv.drawScene();assert(!draws.some(d=>d.kind==='lineShader'))
 nv.opts.crosshairWidth=1;nv.scene.pan2Dxyzmm=[0,0,0,1];const sliceBefore=Array.from(nv.scene.crosshairPos);wheel(24,0,true);const axis={sagittal:0,coronal:1,axial:2}[plane]!;assert(nv.scene.crosshairPos[axis]>sliceBefore[axis]);assert.equal(nv.scene.pan2Dxyzmm[3],1,'Shift wheel changes slice rather than zoom');nv.volumes=[];const emptyPan=Array.from(nv.scene.pan2Dxyzmm);wheel(-100);assert.deepEqual(Array.from(nv.scene.pan2Dxyzmm),emptyPan)
 await new Promise(resolve=>dom.window.requestAnimationFrame(resolve));assert(overlayDraws>0,'Consumed wheel events must still refresh overlay projection after drawing');dispose();paneCases++
}
// Oblique, anisotropic and axis-swapped physical geometry: match Niivue's tile
// projection, including radiological mirroring and a non-square Retina canvas.
for(const affine of [ [[0,-.7,.1,12],[1.2,0,0,-23],[0,.2,2,7],[0,0,0,1]],[[.5,.2,0,-7],[.1,.8,-.3,11],[0,.2,1.3,-19],[0,0,0,1]] ]){
 const image=imageFromRaw({...raw,affine,pixDims:[0,1,2].map(a=>Math.hypot(affine[0][a],affine[1][a],affine[2][a])) as any},'oblique')
 for(const type of [0,1,2])for(const radiological of [false,true]){
  const {nv,draws}=viewer(type,true,image,960,333);nv.opts.isRadiologicalConvention=radiological;nv.drawScene();const tile=nv.screenSlices.at(-1),mm=nv.frac2mm(nv.scene.crosshairPos),axes=[[0,1],[0,2],[1,2]][type],point=[tile.leftTopWidthHeight[0]+(mm[axes[0]]-tile.leftTopMM[0])/tile.fovMM[0]*tile.leftTopWidthHeight[2],tile.leftTopWidthHeight[1]+tile.leftTopWidthHeight[3]-(mm[axes[1]]-tile.leftTopMM[1])/tile.fovMM[1]*tile.leftTopWidthHeight[3]]
  const lines=draws.filter(d=>d.kind==='lineShader');assert.equal(lines.length,2);assert.equal(lines[0].uniforms.startXYendXY[1],point[1]);assert.equal(lines[1].uniforms.startXYendXY[0],point[0])
 }
}
const fallback:any={drawCrosshairs3D(...args:any[]){return args}};installSliceCrosshairs(fallback);assert.deepEqual(fallback.drawCrosshairs3D(true,.4,null,false,true),[true,.4,null,false,true]);assert.deepEqual(raw.affine,original);assert.equal(source.img,raw.values)
console.log(JSON.stringify({status:'passed',emittedBundle:entry,compiledAdapterExecuted:true,compiledZoomExecuted:true,panes:paneCases,original:{tinyWheelEvents:100,deltaPixelsEach:.01,zoom:legacy.nv.scene.pan2Dxyzmm[3],crosshairPaneCoverage:legacyCoverage},checks:['actual Niivue geometry and line draw paths','normalized bounded wheel events','constant CSS crosshair width at13743x','all nine panes','Retina, oblique, anisotropic, axis-swapped, mirrored projection','GL state restoration','source geometry and voxels unchanged'],nativeGPU:'not exercised by command recorder'},null,2))
dom.window.close()
