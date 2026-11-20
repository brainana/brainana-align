import assert from 'node:assert/strict'
import fs from 'node:fs'
import path from 'node:path'
import {fileURLToPath} from 'node:url'
import {Worker as NodeWorker} from 'node:worker_threads'
import {File} from 'node:buffer'
import {JSDOM} from 'jsdom'
import * as nifti from 'nifti-reader-js'
import {gzipSync} from 'fflate'
import {inspectVolume,prepareVolume} from '../packages/imaging-math/workingVolume'
import {createSessionPayload,parseSessionPayload,imageSignature} from '../apps/align/src/sessionPersistence'
import {prepareSessionWorkspace,sessionImageReady} from '../apps/align/src/sessionWorkspace'
import {processImage,emptyProcessingRecipe,imageWorldBounds} from '../apps/align/src/imageProcessing'
import {imageFromRaw} from '../apps/align/src/volumeImages'
import {imageGeometryAlignment} from '../apps/align/src/geometryInitialization'
import type {Loaded,Modality} from '../apps/align/src/appTypes'

const root=path.resolve(path.dirname(fileURLToPath(import.meta.url)),'..'),assets=path.resolve(process.argv[2]??path.join(root,'apps/align/dist'),'assets')
const dom=new JSDOM('<main></main>',{url:'http://localhost/#session='+'a'.repeat(64)});Object.assign(globalThis,{window:dom.window,document:dom.window.document})
let fail='',hold=false,started=0,terminated=0;const live=new Set<ActualWorker>(),seen:string[]=[]
class ActualWorker{
 onmessage:any;onerror:any;worker:NodeWorker;dead=false;kind:string
 constructor(url:URL){this.kind=url.pathname.includes('volumeWorker')?'volumeWorker-':url.pathname.includes('processingWorker')?'processingWorker-':'';assert(this.kind,'Session preparation must not start a registration optimizer');seen.push(this.kind);started++;const name=fs.readdirSync(assets).find(n=>n.startsWith(this.kind)&&n.endsWith('.js'))!;assert(name);this.worker=new NodeWorker(`const {parentPort}=require('node:worker_threads');globalThis.self={postMessage:m=>parentPort.postMessage(m)};parentPort.on('message',data=>self.onmessage({data}));\n${fs.readFileSync(path.join(assets,name),'utf8')}`,{eval:true});live.add(this);this.worker.on('message',data=>this.onmessage?.({data}));this.worker.on('error',e=>this.onerror?.({message:e.message}))}
 postMessage(data:any){if(fail===this.kind){queueMicrotask(()=>this.onmessage?.({data:{type:'error',message:'Injected session stage failure'}}));return}if(hold)setTimeout(()=>{if(!this.dead)this.worker.postMessage(data)},100);else this.worker.postMessage(data)}
 terminate(){if(this.dead)return;this.dead=true;terminated++;live.delete(this);void this.worker.terminate()}
}
Object.assign(globalThis,{Worker:ActualWorker})
function file(name:string,offset=0,dims=[25,27,29]){const h=new nifti.NIFTI1();h.dims=[4,...dims,2,1,1,1];h.pixDims=[1,.2,.3,.4,1,1,1,1];h.affine=[[0,-.3,0,4],[.2,0,.1,-3],[0,0,.4,-4],[0,0,0,1]];h.sform_code=2;h.xyzt_units=2;h.datatypeCode=16;h.numBitsPerVoxel=32;h.vox_offset=352;h.magic='n+1\0';h.littleEndian=true;const v=Float32Array.from({length:dims.reduce((a,b)=>a*b,2)},(_,i)=>offset+i%71+100*Math.floor(i/dims.reduce((a,b)=>a*b,1))),bytes=new Uint8Array(352+v.byteLength);bytes.set(new Uint8Array(h.toArrayBuffer()));bytes.set(new Uint8Array(v.buffer),352);return new File([gzipSync(bytes)],name)}
const mriFile=file('anatomy.nii.gz'),ctFile=file('ct.nii.gz',5),files={mri:mriFile,ct:ctFile},loaded={} as Record<Modality,Loaded>
for(const m of ['mri','ct'] as Modality[]){const f=files[m],info=await inspectVolume(f),raw=await prepareVolume(f,info,[.51,.72,.92],1,()=>{},[10,12,14]);loaded[m]={name:f.name,sourceFiles:[f.name],raw,nvImage:imageFromRaw(raw,f.name),native:{files:[f as any],info,frame:1,requested:[.51,.72,.92],streamable:true}}}
const recipe=emptyProcessingRecipe(loaded.mri.raw);recipe.version=6;recipe.crop=imageWorldBounds(loaded.mri.raw);recipe.crop.min[0]+=.5;recipe.binaryMask={min:105,max:150,invert:false,fillHoles:'3d',edits:[{index:50,value:1}]};recipe.thresholdFill={...recipe.binaryMask,value:0};recipe.edits=[{index:600,value:777,afterMask:true}]
const expected=processImage(loaded.mri.raw,recipe),processed={...loaded.mri,raw:expected.raw,nvImage:imageFromRaw(expected.raw,loaded.mri.name),processing:{base:loaded.mri.raw,recipe,excluded:expected.excluded,binarySelection:expected.binarySelection}}
const original={mri:processed,ct:loaded.ct},fit=imageGeometryAlignment('ct-mri'),payload=createSessionPayload({appVersion:'test',loaded:original,landmarks:[],selectedId:null,nextId:1,direction:'ct-mri',fit,optimizationWindows:{mri:{axial:{min:[-2,-3,-4],max:[2,3,4]}},ct:{}}})
assert.deepEqual(payload.images.mri!.workingDims,loaded.mri.raw.dims,'Cropped sessions save the original working grid, not cropped dimensions')
assert.deepEqual(payload.images.mri!.requestedSpacing,[.51,.72,.92]);assert(payload.images.mri!.workingDims!.some((n,a)=>n!==processed.raw.dims[a]))
const session=parseSessionPayload(JSON.parse(JSON.stringify(payload))),sessionFile=new File([JSON.stringify(session)],'saved.json') as any
assert(sessionImageReady(session,'mri',processed));assert(sessionImageReady(session,'ct',loaded.ct));assert(!sessionImageReady(session,'ct',{...loaded.ct,sourceFiles:['wrong-subject.nii.gz']}))
const unchanged=loaded.mri.raw.values.slice(),snapshot=JSON.stringify(session),empty={mri:null,ct:null},tick=()=>new Promise(r=>setTimeout(r,2))
async function picker(file:File){for(let i=0;i<2000;i++){const input=document.querySelector<HTMLInputElement>('#session-locate-input');if(input){Object.defineProperty(input,'files',{configurable:true,value:[file]});input.dispatchEvent(new dom.window.Event('change'));return}await tick()}throw Error('Missing session file picker')}

// Real browser-style selections from an empty workspace, wrong-image retry,
// selected frame, exact saved dimensions, original-grid crop/mask/paint replay.
const abort=new AbortController(),opening=prepareSessionWorkspace(sessionFile,session,empty,abort.signal,()=>{})
await picker(file('wrong-size.nii.gz',0,[19,20,21]));for(let i=0;i<1000&&!document.querySelector('#session-locate-problem')?.textContent;i++)await tick();assert(document.querySelector('#session-locate-problem')?.textContent?.includes('does not match'))
await picker(mriFile);await picker(ctFile);const restored=await opening
assert.deepEqual(empty,{mri:null,ct:null});assert.deepEqual(restored.mri.raw.values,expected.raw.values);assert.deepEqual(restored.mri.raw.affine,expected.raw.affine);assert.deepEqual(restored.mri.processing!.base.values,loaded.mri.raw.values);assert.deepEqual(restored.mri.processing!.excluded,expected.excluded);assert.deepEqual(restored.mri.processing!.binarySelection,expected.binarySelection);assert.deepEqual(restored.ct.raw.values,loaded.ct.raw.values);assert.equal(restored.ct.native!.frame,1);assert.deepEqual(restored.ct.raw.dims,[10,12,14]);assert.deepEqual(loaded.mri.raw.values,unchanged);assert.equal(JSON.stringify(session),snapshot);assert.equal(live.size,0)

// Cancellation between file selections must not prepare either image.
const beforeCancel=started,cancel=new AbortController(),cancelled=prepareSessionWorkspace(sessionFile,session,empty,cancel.signal,()=>{});const cancelledAssertion=assert.rejects(cancelled,/cancelled/);await picker(mriFile);for(let i=0;i<1000&&!document.querySelector('#session-locate-cancel');i++)await tick();(document.querySelector('#session-locate-cancel') as HTMLButtonElement).click();await cancelledAssertion;assert.equal(started,beforeCancel);assert.equal(document.querySelector('#session-locate-modal'),null)

// Reuse an already loaded correct pair without reparsing native data. A failure
// in the later processing stage cannot alter its images, recipes or alignment.
fail='processingWorker-';await assert.rejects(prepareSessionWorkspace(sessionFile,session,original,new AbortController().signal,()=>{}),/Injected session stage failure/);assert.deepEqual(original.mri.raw.values,expected.raw.values);assert.equal(live.size,0);fail=''
const beforeReuse=seen.length;await prepareSessionWorkspace(sessionFile,session,original,new AbortController().signal,()=>{});assert(!seen.slice(beforeReuse).includes('volumeWorker-'));assert.equal(live.size,0)

// Mid-worker cancellation settles/terminates exactly once and permits retry.
const delayed=new AbortController();hold=true;const inFlight=prepareSessionWorkspace(sessionFile,session,original,delayed.signal,()=>{}),stopped=assert.rejects(inFlight,/cancelled/);await tick();delayed.abort();await stopped;hold=false;assert.equal(live.size,0);assert.equal(started,terminated)

// Persist only source paths, never transient HTTP IDs, URLs or auth tokens.
window.brainanaSessionFiles={path:f=>'/mounted/renamed/'+f.name,open:async()=>({unavailable:true}),choose:async()=>({unavailable:true})}
const signature=imageSignature(loaded.ct)!;assert.deepEqual(signature.sourceLocations,['/mounted/renamed/ct.nii.gz'])
const resaved=createSessionPayload({appVersion:'test',loaded,landmarks:[],selectedId:null,nextId:1,direction:'ct-mri',fit,optimizationWindows:{mri:{},ct:{}}});assert.equal(parseSessionPayload(JSON.parse(JSON.stringify(resaved))).images.ct!.sourceLocations![0],'/mounted/renamed/ct.nii.gz')
for(const bad of [['bad\0path'],['a','b'],[{}]]){const tampered=JSON.parse(JSON.stringify(resaved));tampered.images.ct.sourceLocations=bad;assert.throws(()=>parseSessionPayload(tampered),/locations/)}
assert.equal(JSON.stringify(resaved.images).includes('sessionToken'),false);delete window.brainanaSessionFiles
const legacy=JSON.parse(JSON.stringify(session));legacy.schemaVersion=13;delete legacy.images.mri.sourceLocations;delete legacy.images.ct.sourceLocations;assert.equal(parseSessionPayload(legacy).schemaVersion,14)
dom.window.close();console.log('Session workspace: empty startup, wrong-file retry, exact saved grids/frame, full processing replay, original immutability, reuse, failure/cancel/retry, legacy schemas and safe path-only metadata passed.')
