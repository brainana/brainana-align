import assert from 'node:assert/strict'
import {JSDOM} from 'jsdom'
import {alignmentLayout} from '../apps/align/src/alignmentLayout'
import {saveNativeAlignedImage} from '../apps/align/src/nativeExport'
import {installGeneratedImageExports} from '../apps/align/src/generatedImageExports'
import {installInitialProcessing} from '../apps/align/src/initialProcessingUI'
import {emptyProcessingRecipe} from '../apps/align/src/imageProcessing'
import {rawFromNVImage} from '../apps/align/src/imageGeometry'
import {sampleExport} from '../packages/imaging-math/exportSampling'
import {sampleLinear} from '../packages/imaging-math/roiWarp'
import {processingViewPoint,voxelAtWorld} from '../apps/align/src/imageProcessing'
import {prepareInWorker} from '../apps/align/src/volumeTasks'
import {imageGeometryAlignment} from '../apps/align/src/geometryInitialization'
import {applyManualAffine,initialManualState} from '../apps/align/src/manualAffine'
import {resetInitialAlignment,rejectLegacyRefinement} from '../apps/align/src/resetInitialAlignment'
import {createRegistrationArtifacts} from '../apps/align/src/exportArtifacts'
import {parseSessionPayload,createSessionPayload} from '../apps/align/src/sessionPersistence'
import {fitLandmarks} from '../apps/align/src/landmarkFit'
import {rigidDelta,applyMat4,multiplyMat4} from '../apps/align/src/rigid'
import {replaceWithBackup} from '../packages/core-server/replaceWithBackup.mjs'
const affine=[[2,0,0,0],[0,3,0,0],[0,0,4,0],[0,0,0,1]]
const geometry={dims:[2,2,2],pixDims:[2,3,4],affine} as any
assert.deepEqual(rawFromNVImage({img:new Float32Array(8),hdr:{dims:[3,2,2,2],pixDims:[1,1,1,1],xyzt_units:2},getAffine:()=>affine} as any).pixDims,[2,3,4])
for(const nonfinite of [NaN,Infinity,-Infinity])for(let index=0;index<8;index++){
 const values=new Float32Array(8).fill(nonfinite);values[index]=42
 const point=[index%2,Math.floor(index/2)%2,Math.floor(index/4)]
 for(const method of ['nearest','linear','cubic'] as const)assert.equal(sampleExport(values,[2,2,2],point,method),42)
}
assert(Number.isNaN(sampleLinear(new Float32Array([1,NaN]),[2,1,1],[.5,0,0])),'nonzero NaN contribution must remain invalid')
assert.deepEqual(processingViewPoint(geometry,geometry,[0,0,0],{min:[1,1,1],max:[5,5,5]},[2,3,4]),[2,3,4])
assert.equal(voxelAtWorld(geometry,[NaN,0,0]),null)
let terminated=0,attached=0;const controller=new AbortController(),signal=controller.signal
const add=signal.addEventListener.bind(signal),remove=signal.removeEventListener.bind(signal)
signal.addEventListener=((...args:any[])=>{attached++;return add(...args as Parameters<typeof add>)}) as any
signal.removeEventListener=((...args:any[])=>{attached--;return remove(...args as Parameters<typeof remove>)}) as any
Object.assign(globalThis,{Worker:class{postMessage(){throw Error('clone failed')}terminate(){terminated++}}})
await assert.rejects(prepareInWorker({} as any,{} as any,[1,1,1],0,()=>{},signal),/clone failed/)
assert.equal(terminated,1);assert.equal(attached,0);controller.abort();assert.equal(terminated,1)
// Exercise each real export controller's synchronous failure path, including UI
// busy-state recovery; no files are written and no original arrays are transferred.
const dom=new JSDOM('<div id="app"></div>');Object.assign(globalThis,{window:dom.window,document:dom.window.document});document.querySelector('#app')!.innerHTML=alignmentLayout('test')
const raw={...geometry,values:new Float32Array(8),frameCount:1,slope:1,intercept:0,datatypeCode:16,littleEndian:true},recipe=emptyProcessingRecipe(raw);recipe.binaryMask={minimum:0,maximum:1,invert:false} as any
const item:any={raw,name:'test.nii',sourceFiles:[],processing:{base:raw,recipe,excluded:new Uint8Array(8)}}
let busy=false,ends=0;const activity={get busy(){return busy},begin:async()=>{busy=true;return true},phase:async()=>{},end:()=>{busy=false;ends++}}
await assert.rejects(saveNativeAlignedImage(item,item,imageGeometryAlignment('ct-mri'),'ct','mri',activity,()=>{}),/clone failed/);assert(!busy);assert.equal(terminated,2)
const generated=installGeneratedImageExports({getLoaded:()=>({ct:item,mri:item}),activity,run:async fn=>{await fn()},report:()=>{}})
document.querySelector<HTMLButtonElement>('#save-generated-image')!.click()
for(let i=0;i<100&&ends<2;i++)await new Promise(r=>setTimeout(r,1))
assert.equal(ends,2);assert(!busy);assert.equal(terminated,3)
const processing=installInitialProcessing({getLoaded:()=>({ct:item,mri:item}),pending:()=>false,activity,commit:async()=>{},stopRegion:()=>{},redraw:()=>{},status:()=>{}})
document.querySelector<HTMLButtonElement>('#processing-export')!.click()
for(let i=0;i<100&&ends<3;i++)await new Promise(r=>setTimeout(r,1))
assert.equal(ends,3);assert(!busy);assert.equal(terminated,4);dom.window.close()
for(const direction of ['ct-mri','mri-ct']){
 const fit=imageGeometryAlignment(direction),state=initialManualState(fit,{mri:[0,0,0],ct:[0,0,0]},'ct');state.parameters.ct[6]=Math.log(2)
 const adjusted=applyManualAffine(fit,state);assert.equal(adjusted.model,'affine')
 const reset=resetInitialAlignment(adjusted);assert.equal(reset.model,'rigid');assert.equal(reset.parameterCount,6);assert.deepEqual(reset.matrix,fit.matrix);assert.deepEqual(reset.diagnostics!.scales,[1,1,1]);assert.equal(reset.manualAffine,undefined)
 const payload=createSessionPayload({appVersion:'test',loaded:{mri:null,ct:null},landmarks:[],selectedId:null,nextId:1,direction,fit:reset,optimizationWindows:{mri:{},ct:{}}});assert.equal(parseSessionPayload(payload).fit!.model,'rigid')
 const metadata=JSON.parse(await createRegistrationArtifacts({appVersion:'test',fit:adjusted,landmarks:[]})[0].blob.text());assert.equal(metadata.scale_estimates_xyz[0],direction==='ct-mri'?2:.5);assert.deepEqual(metadata.landmark_fit_diagnostics.scales,[1,1,1])
}
const legacy=imageGeometryAlignment('ct-mri');legacy.manual=[2,3,4,0,0,0];legacy.matrix=rigidDelta([12,13,14,0,0,0],[0,0,0]);legacy.proposal={matrix:legacy.matrix,scoreBefore:1,scoreAfter:2,params:[10,10,10,0,0,0]}
assert.deepEqual(rejectLegacyRefinement(legacy,[0,0,0]).matrix,rigidDelta([2,3,4,0,0,0],[0,0,0]));assert.equal(rejectLegacyRefinement(legacy,[0,0,0]).proposal,null)
// Fault injection: replacement, recovery and cleanup failures must never erase
// the only surviving original, for both local and structured SFTP adapters.
for(const failure of ['none','install','rollback','cleanup']){
 const files=new Map([['destination','original'],['temporary','new']]);let installs=0
 const io={exists:async(f:string)=>files.has(f),rename:async(a:string,b:string)=>{if(a==='temporary'&&failure!=='none'&&failure!=='cleanup'){installs++;throw Error('install failure')}if(a==='backup'&&failure==='rollback')throw Error('rollback failure');assert(files.has(a));files.set(b,files.get(a)!);files.delete(a)},remove:async(f:string)=>{if(f==='backup'&&failure==='cleanup')throw Error('cleanup failure');files.delete(f)}}
 if(failure==='none')await replaceWithBackup(io,'temporary','destination','backup');else await assert.rejects(replaceWithBackup(io,'temporary','destination','backup'),failure==='rollback'?/Original file retained at backup/:failure==='cleanup'?/New file saved/:/install failure/)
 if(failure==='rollback')assert.equal(files.get('backup'),'original');else if(failure==='install')assert.equal(files.get('destination'),'original');else assert.equal(files.get('destination'),'new')
 if(failure==='cleanup')assert.equal(files.get('backup'),'original')
}
// Seeded all-pairs recovery beyond the fixed hand-picked scientific fixtures.
let seed=166970;const random=()=>{seed=(Math.imul(seed,1664525)+1013904223)>>>0;return seed/2**32}
for(let run=0;run<80;run++){
 const points=Array.from({length:9},()=>Array.from({length:3},()=>200*(random()-.5)) as [number,number,number]),rotation=rigidDelta([10*random(),20*random(),-5*random(),360*random(),360*random(),360*random()],[0,0,0])
 for(const model of ['rigid','similarity','anisotropic','affine'] as const){const scales=model==='rigid'?[1,1,1]:model==='similarity'?Array(3).fill(.5+random()):[.5+random(),.5+random(),.5+random()],linear=[[scales[0],model==='affine'?.15:0,model==='affine'?-.2:0,0],[0,scales[1],model==='affine'?.1:0,0],[0,0,scales[2],0],[0,0,0,1]],truth=multiplyMat4(rotation,linear),target=points.map(p=>applyMat4(truth,p)),result=fitLandmarks(points,target,model);assert(result.rms<1e-4,`${model} run ${run}: ${result.rms}`);for(let i=0;i<4;i++)for(let j=0;j<4;j++)assert(Math.abs(result.matrix[i][j]-truth[i][j])<1e-4)}
}
console.log('Audit regressions passed: affine spacing, nonfinite interpolation, unchanged-grid crop navigation, worker clone cleanup, original-fit model/reset/export, failed overwrite recovery and 320 seeded transform recoveries.')
