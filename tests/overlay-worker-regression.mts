import assert from 'node:assert/strict'
import fs from 'node:fs'
import path from 'node:path'
import {Worker as NodeWorker} from 'node:worker_threads'
import {runOverlayTask} from '../apps/align/src/overlayTask'
import {createOverlayScheduler} from '../apps/align/src/overlayScheduler'
import {replaceReviewVolumes} from '../apps/align/src/reviewVolumes'
import {imageFromRaw,sharedViewerImage} from '../apps/align/src/volumeImages'
import {imageGeometryAlignment} from '../apps/align/src/geometryInitialization'
import {initialManualState,applyManualAffine} from '../apps/align/src/manualAffine'
import {fitNonlinearSpline,resampleSpline} from '../packages/imaging-math/nonlinearSpline'
import {resampleRegistration} from '../packages/imaging-math/registrationResampling'
import {invertAffine} from '../apps/align/src/roiWarp'
import type {FitState} from '../apps/align/src/appTypes'
import type {Vec3,Mat4} from '../apps/align/src/rigid'
const assets=path.resolve(process.argv[2]??'apps/align/dist','assets'),workerFile=fs.readdirSync(assets).find(f=>/^nonlinearWorker-.*\.js$/.test(f));assert(workerFile,'Emitted overlay/nonlinear worker required')
const code=fs.readFileSync(path.join(assets,workerFile),'utf8');let terminated=0,created=0
const requests:any[]=[]
class ActualWorker{
 onmessage:((event:any)=>void)|null=null;onerror:((event:any)=>void)|null=null;onmessageerror:((event:any)=>void)|null=null
 worker=new NodeWorker(`const {parentPort}=require('node:worker_threads');globalThis.self={postMessage:(m,t)=>parentPort.postMessage(m,t)};parentPort.on('message',data=>self.onmessage({data}));\n${code}`,{eval:true})
 constructor(){created++;this.worker.on('message',data=>this.onmessage?.({data}));this.worker.on('error',e=>this.onerror?.({message:e.message}));this.worker.on('messageerror',e=>this.onmessageerror?.(e))}
 postMessage(data:any){requests.push(data);assert.equal(data.target.values.length,0,'Fixed voxels are unused and must not be cloned');this.worker.postMessage(data)}
 terminate(){terminated++;void this.worker.terminate()}
}
Object.assign(globalThis,{Worker:ActualWorker})
const affine:Mat4=[[0,.7,0,-3],[-1.1,0,0,4],[0,0,1.5,-6],[0,0,0,1]]
const raw={dims:[11,12,13] as Vec3,pixDims:[1.1,.7,1.5] as Vec3,affine,values:Float32Array.from({length:11*12*13},(_,i)=>Math.sin(i*.03)+i%11),frameCount:1,datatypeCode:16,littleEndian:true,slope:1,intercept:0}
const target={...raw,dims:[7,8,9] as Vec3,affine:[[.9,0,0,-2],[0,.8,0,-4],[0,0,1.2,-3],[0,0,0,1]] as Mat4,values:new Float32Array(7*8*9)}
const sourceBefore=raw.values.slice(),targetBefore=target.values.slice(),points:Vec3[]=[]
for(const x of [-8,0,8])for(const y of [-8,8])for(const z of [-8,8])points.push([x,y,z])
const spline=fitNonlinearSpline(points,points.map(([x,y,z])=>[x+.15*Math.sin(y/8),y+.1*Math.cos(z/8),z+.1*Math.cos(x/8)] as Vec3),.05)
const fixture=JSON.parse(fs.readFileSync('tests/fixtures/jip/mapping-fixtures.json','utf8'))[2].result
let cases=0
for(const direction of ['ct-mri','mri-ct'])for(const kind of ['affine','spline','jip'])for(const moving of ['mri','ct'] as const){
 let fit:FitState=imageGeometryAlignment(direction)
 if(kind==='spline')fit={...fit,matrix:spline.matrix,baseMatrix:spline.matrix,inverse:spline.inverse,nonlinear:spline.spline,model:'spline'}
 if(kind==='jip')fit={...fit,matrix:fixture.matrix,baseMatrix:fixture.matrix,inverse:invertAffine(fixture.matrix),nonlinear:fixture.deformation,model:'jip'}
 const state=initialManualState(fit,{mri:[1,2,3],ct:[-2,4,1]},moving);state.parameters[moving]=[.2,-.1,.3,2,-3,4,.01,-.02,.03,.02,-.01,.02];fit=applyManualAffine(fit,state)
 const reverse=moving!==(direction==='ct-mri'?'ct':'mri'),phases:string[]=[]
 const actual=await runOverlayTask({source:raw,target,fit,reverse},new AbortController().signal,text=>phases.push(text))
 const expected=fit.nonlinear?await resampleSpline(raw,target,fit.matrix,fit.nonlinear,()=>{},reverse):resampleRegistration(raw,target,reverse?invertAffine(fit.matrix):fit.matrix)
 assert.deepEqual(actual,expected,`${direction}/${kind}/${moving}`);assert(phases.length);cases++
}
assert.deepEqual(raw.values,sourceBefore);assert.deepEqual(target.values,targetBefore)
const bad=imageGeometryAlignment('ct-mri');bad.matrix=[[0,0,0,0],[0,0,0,0],[0,0,0,0],[0,0,0,1]]
await assert.rejects(runOverlayTask({source:raw,target,fit:bad,reverse:false},new AbortController().signal),/singular|degenerate|invertible|determinant/i)
const controller=new AbortController(),pending=runOverlayTask({source:raw,target:{...target,dims:[250,250,250]},fit:imageGeometryAlignment('ct-mri'),reverse:false},controller.signal);controller.abort();await assert.rejects(pending,/cancelled/)
const already=new AbortController();already.abort();const count=created;await assert.rejects(runOverlayTask({source:raw,target,fit:bad,reverse:false},already.signal),/cancelled/);assert.equal(created,count)
// A real worker can finish before supersession yet its delayed continuation
// still must not publish. Also terminate a genuinely in-flight old worker.
const scheduler=createOverlayScheduler(),request={source:raw,target,fit:imageGeometryAlignment('ct-mri'),reverse:false}
let release!:()=>void,workerDone!:()=>void
const gate=new Promise<void>(resolve=>{release=resolve}),done=new Promise<void>(resolve=>{workerDone=resolve})
const stale=scheduler.run(async signal=>{const value=await runOverlayTask(request,signal);workerDone();await gate;return value})
await done;assert.equal((await scheduler.run(signal=>runOverlayTask(request,signal))).status,'ready');release();assert.equal((await stale).status,'stale')
const inFlight=scheduler.run(signal=>runOverlayTask({...request,target:{...target,dims:[250,250,250]}},signal));assert.equal((await scheduler.run(signal=>runOverlayTask(request,signal))).status,'ready');assert.equal((await inFlight).status,'stale')
assert.equal(created,terminated,'Every emitted worker must release ownership')

// Exercise the actual installed Niivue volume methods with GL updates stubbed.
const {Niivue}=await import('@niivue/niivue'),nv=new Niivue({forceDevicePixelRatio:1});let uploads=0;nv.updateGLVolume=()=>{uploads++};nv.drawScene=()=>{};nv.setVolume=()=>{}
const fixed=imageFromRaw(target,'fixed'),moving=imageFromRaw(target,'moving');nv.volumes=[sharedViewerImage(fixed),sharedViewerImage(moving)]
const retained=nv.volumes[0],ids=nv.volumes.map(v=>v.id),next=imageFromRaw({...target,values:new Float32Array(target.values.length).fill(3)},'new')
replaceReviewVolumes(nv,fixed,next,fixed);assert.equal(nv.volumes.length,2);assert.equal(nv.volumes[0],retained);assert.equal(nv.volumes[0].id,ids[0]);assert.equal(nv.volumes[1].img,next.img);assert.notEqual(nv.volumes[1].id,ids[1]);assert(uploads)
const different=imageFromRaw(target,'changed-fixed');replaceReviewVolumes(nv,different,next,fixed);assert.notEqual(nv.volumes[0],retained);assert.equal(nv.volumes[0].img,different.img)
assert(requests.some(r=>r.type==='affine-resample'));assert(requests.some(r=>r.type==='resample'))
console.log(`Exact emitted overlay worker: ${cases} affine/spline/JIP direction/target/manual cases, immutable inputs, bounded target transport, errors, cancellation, stale continuations and fixed-viewer retention passed. Native GPU responsiveness remains unverified.`)
