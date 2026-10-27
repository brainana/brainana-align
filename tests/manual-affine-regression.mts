import assert from 'node:assert/strict'
import fs from 'node:fs'
import path from 'node:path'
import {Worker as NodeWorker} from 'node:worker_threads'
import {JSDOM} from 'jsdom'
import {imageGeometryAlignment} from '../apps/align/src/geometryInitialization'
import {initialManualState,applyManualAffine,manualDelta,resetManualAffine,registrationPoint,registrationModalities,zeroManual} from '../apps/align/src/manualAffine'
import {installManualAdjustment} from '../apps/align/src/manualAdjustmentUI'
import {installManualSourcePreview} from '../apps/align/src/manualSourcePreview'
import {createPointMapping,fitNonlinearSpline} from '../packages/imaging-math/nonlinearSpline'
import {createSessionPayload,parseSessionPayload} from '../apps/align/src/sessionPersistence'
import {createRegistrationArtifacts} from '../apps/align/src/exportArtifacts'
import {alignmentLayout} from '../apps/align/src/alignmentLayout'
import {imageFromRaw} from '../apps/align/src/volumeImages'
import {applyMat4,multiplyMat4,type Vec3} from '../apps/align/src/rigid'
import {invertAffine,type RawNifti} from '../apps/align/src/roiWarp'
import {runNonlinearTask} from '../apps/align/src/nonlinearTasks'
import type {FitState,Modality,View,Plane} from '../apps/align/src/appTypes'
const close=(a:number[],b:number[],eps=1e-6)=>assert(a.every((v,i)=>Math.abs(v-b[i])<eps),`${a} != ${b}`),centers={mri:[0,0,0] as Vec3,ct:[0,0,0] as Vec3}
close(applyMat4(manualDelta([1,2,3,0,0,90,Math.log(2),Math.log(3),Math.log(4),.2,.3,.4],[0,0,0]),[1,2,3]),[-6.2,5.3,15])
for(const direction of ['ct-mri','mri-ct']){
 const base=imageGeometryAlignment(direction);base.matrix=manualDelta([5,-2,3,70,89,179,0,0,0,0,0,0],[1,2,3]);base.inverse=invertAffine(base.matrix);base.baseMatrix=base.matrix;base.landmarkMatrix=base.matrix
 const {moving,fixed}=registrationModalities(direction),state=initialManualState(base,centers,moving);state.parameters[moving][0]=2;state.parameters[fixed][1]=-3;state.parameters[fixed][5]=90
 const adjusted=applyManualAffine(base,state),point:Vec3=[7,4,-2],expected=applyMat4(manualDelta(state.parameters[moving],centers[moving]),applyMat4(base.matrix,applyMat4(invertAffine(manualDelta(state.parameters[fixed],centers[fixed])),point)))
 close(createPointMapping(adjusted.matrix).forward(point),expected);close(registrationPoint(adjusted,moving,fixed,point),expected);close(registrationPoint(adjusted,fixed,moving,expected),point)
 assert.equal(adjusted.direction,direction);assert.deepEqual(resetManualAffine(adjusted).matrix,base.baseMatrix);assert.deepEqual(base.manual,[0,0,0,0,0,0])
 const malformed=structuredClone(state);malformed.parameters[moving][6]=Infinity;assert.throws(()=>applyManualAffine(base,malformed))
}
const points:Vec3[]=[];for(const x of [-8,0,8])for(const y of [-8,8])for(const z of [-8,8])points.push([x,y,z]);const targets=points.map(([x,y,z])=>[x+.12*Math.sin(y/8),y+.1*Math.cos(z/8),z+.1*Math.cos(x/8)] as Vec3),spline=fitNonlinearSpline(points,targets,.05)
for(const direction of ['ct-mri','mri-ct']){
 const fit:FitState={...imageGeometryAlignment(direction),matrix:spline.matrix,baseMatrix:spline.matrix,inverse:spline.inverse,landmarkMatrix:spline.matrix,model:'spline',parameterCount:36,nonlinear:spline.spline,initializationSource:'landmarks',landmarkSnapshot:points.map((p,i)=>({id:i+1,enabled:true,[direction==='ct-mri'?'ct':'mri']:p,[direction==='ct-mri'?'mri':'ct']:targets[i]})) as any},state=initialManualState(fit,centers,'mri'),{moving,fixed}=registrationModalities(direction)
 state.parameters[moving]=[1,2,3,3,-4,10,.02,-.01,.03,.01,-.02,.01];state.parameters[fixed]=[-2,1,0,-3,2,7,-.01,.02,.01,-.02,.01,.03]
 const adjusted=applyManualAffine(fit,state),mapping=createPointMapping(adjusted.matrix,adjusted.nonlinear),original=createPointMapping(fit.matrix,fit.nonlinear),dm=manualDelta(state.parameters[moving],centers[moving]),dfi=invertAffine(manualDelta(state.parameters[fixed],centers[fixed]))
 for(const p of [...points,[2,3,4] as Vec3]){const expected=applyMat4(dm,original.forward(applyMat4(dfi,p)));close(mapping.forward(p),expected);close(mapping.inverse(expected),p)}
 assert.deepEqual(adjusted.nonlinear!.weights,spline.spline.weights);assert.deepEqual(adjusted.nonlinear!.controls,spline.spline.controls)
 const payload=createSessionPayload({appVersion:'test',loaded:{mri:null,ct:null},landmarks:fit.landmarkSnapshot,selectedId:null,nextId:13,direction,fit:adjusted,optimizationWindows:{mri:{},ct:{}}}),restored=parseSessionPayload(JSON.parse(JSON.stringify(payload))).fit!;close(createPointMapping(restored.matrix,restored.nonlinear).forward([2,3,4]),mapping.forward([2,3,4]));const bad=structuredClone(payload);bad.fit!.nonlinear!.inputAffine![0][3]+=1;assert.throws(()=>parseSessionPayload(bad))
 const metadata=JSON.parse(await createRegistrationArtifacts({appVersion:'test',fit:adjusted,landmarks:fit.landmarkSnapshot})[0].blob.text());assert(metadata.manual_adjustment.affine);assert(metadata.nonlinear.inputAffine);assert(metadata.nonlinear_mapping.includes('A P x'));assert.deepEqual(resetManualAffine(adjusted).nonlinear!.inputAffine,undefined)
}
const dom=new JSDOM('<main></main>',{pretendToBeVisual:true});Object.assign(globalThis,{window:dom.window,document:dom.window.document});document.querySelector('main')!.innerHTML=alignmentLayout('test')
const raw:RawNifti={dims:[7,7,7],pixDims:[1,1,1],affine:[[1,0,0,-3],[0,1,0,-3],[0,0,1,-3],[0,0,0,1]],values:new Float32Array(343),datatypeCode:16,slope:1,intercept:0,littleEndian:true,frameCount:1};raw.values[3+7*(3+7*3)]=1;const originalVoxels=raw.values.slice(),loaded:any={mri:{raw,nvImage:imageFromRaw(raw,'MRI'),name:'MRI',sourceFiles:[]},ct:{raw,nvImage:imageFromRaw(raw,'CT'),name:'CT',sourceFiles:[]}}
let fit:FitState|null=imageGeometryAlignment('ct-mri'),busy=false,changes=0,targetChanges=0,previewChanges=0;const messages:string[]=[]
const ui=installManualAdjustment({getFit:()=>fit,setFit:f=>{fit=f},getLoaded:()=>loaded,busy:()=>busy,changed:()=>changes++,targetChanged:()=>targetChanges++,previewChanged:()=>previewChanges++,status:s=>messages.push(s)}),click=(selector:string)=>document.querySelector<HTMLButtonElement>(selector)!.click(),input=(i:number,v:number)=>{const field=document.querySelector<HTMLInputElement>(`#param-${i}`)!;field.value=String(v);field.dispatchEvent(new window.Event('change'))}
assert(document.querySelector('#manual-start-section')!.classList.contains('hidden'));click('[data-manual-target=mri]');assert.equal(ui.target,'mri');click('[data-param="0"][data-sign="1"]');assert.equal(fit!.matrix[0][3],-.5);assert.equal(fit!.inverse[0][3],.5);assert.equal(fit!.direction,'ct-mri');assert.equal(fit!.manualAffine!.parameters.ct[0],0)
click('[data-manual-target=ct]');input(6,110);input(9,.2);assert.equal(fit!.model,'affine');assert.equal(fit!.parameterCount,12);assert(Math.abs(fit!.manualAffine!.parameters.ct[6]-Math.log(1.1))<1e-9);const before=fit;input(6,0);assert.equal(fit,before);assert(messages.length);busy=true;ui.refresh();assert(document.querySelector<HTMLButtonElement>('[data-param="0"][data-sign="1"]')!.disabled);busy=false;ui.refresh();click('#reset-manual');assert.deepEqual(fit!.matrix,imageGeometryAlignment('ct-mri').matrix);assert.equal(fit!.manualAffine,undefined)
// Preview replaces viewer buffers only, reuses the review buffer, restores camera
// state, and never mutates loaded/raw/header arrays or fit/landmark coordinates.
const views:Record<Modality,View[]>={mri:[],ct:[]},nativeMm:Record<Modality,Vec3>={mri:[1,2,0],ct:[0,0,0]};let state:any=null
for(const m of ['mri','ct'] as const)for(const plane of ['sagittal','coronal','axial'] as Plane[]){const nv:any={volumes:[loaded[m].nvImage],scene:{pan2Dxyzmm:[1,2,3,1.2]},removeVolumeByIndex(i:number){this.volumes.splice(i,1)},addVolume(image:any){this.volumes.push(image)},setVolumeAffine(i:number,a:any){this.volumes[i].setAffine(a)}};views[m].push({nv,modality:m,plane,canvas:document.querySelector(`#${m}-${plane}`),overlay:null,windowLayer:null} as any)}
const preview=installManualSourcePreview({loaded:m=>loaded[m],views:m=>views[m],current:m=>nativeMm[m],gotoNative:(m,p)=>{nativeMm[m]=p},state:s=>{state=s},beforeShow:()=>{},afterShow:()=>{}}),image=imageFromRaw({...raw,values:new Float32Array(343).fill(2)},'aligned')
fit=imageGeometryAlignment('ct-mri');fit.matrix[0][3]=1;fit.inverse=invertAffine(fit.matrix);const fitSnapshot=JSON.stringify(fit);preview.show('ct','mri',image,fit);assert.equal(state.modality,'ct');close(nativeMm.ct,[0,2,0]);for(const view of views.ct){assert.equal(view.nv.volumes[0].img,image.img);assert.deepEqual(view.nv.scene.pan2Dxyzmm,[0,0,0,1]);assert(view.canvas.closest('.view-card')!.classList.contains('source-transform-preview'))}views.ct[0].nv.scene.pan2Dxyzmm=[4,5,6,2];preview.show('ct','mri',image,fit);assert.deepEqual(views.ct[0].nv.scene.pan2Dxyzmm,[4,5,6,2]);preview.show('mri','ct',image,fit);assert.equal(state.modality,'mri');for(const view of views.ct){assert.equal(view.nv.volumes[0].img,raw.values);assert.deepEqual(view.nv.scene.pan2Dxyzmm,[1,2,3,1.2])}preview.clear();assert.equal(state,null);assert.equal(JSON.stringify(fit),fitSnapshot);assert.deepEqual(raw.values,originalVoxels)
// Execute the actual packaged worker's forward and reverse pull maps.
const assets=path.resolve(process.argv[2]??'apps/align/dist','assets'),workerFile=fs.readdirSync(assets).find(n=>/^nonlinearWorker-.*\.js$/.test(n))!
class ActualWorker{worker:NodeWorker;onmessage:any;onerror:any;constructor(){this.worker=new NodeWorker(`const {parentPort}=require('node:worker_threads');globalThis.self={postMessage:m=>parentPort.postMessage(m)};parentPort.on('message',data=>self.onmessage({data}));\n${fs.readFileSync(path.join(assets,workerFile),'utf8')}`,{eval:true});this.worker.on('message',data=>this.onmessage?.({data}));this.worker.on('error',e=>this.onerror?.({message:e.message}))}postMessage(data:any){this.worker.postMessage(data)}terminate(){void this.worker.terminate()}}
Object.assign(globalThis,{Worker:ActualWorker});const matrix=manualDelta([1,0,0,0,0,0,0,0,0,0,0,0],[0,0,0]);for(const reverse of [false,true]){const out=await runNonlinearTask<Float32Array>({type:'resample',source:raw,target:raw,matrix,reverse},()=>{});assert.equal(out[(reverse?2:4)+7*(3+7*3)],1);assert.deepEqual(raw.values,originalVoxels)}
dom.window.close();console.log('Manual affine: 12 controls, both explicit targets/directions, centered affine algebra, nonlinear pre/post inverse, strict session/export state, UI guards/reset, source-preview camera/data isolation and actual forward/reverse worker passed.')
