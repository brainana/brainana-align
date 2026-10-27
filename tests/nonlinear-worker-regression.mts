import assert from 'node:assert/strict'
import fs from 'node:fs'
import path from 'node:path'
import {Worker as NodeWorker} from 'node:worker_threads'
import {createRequire} from 'node:module'
import {runNonlinearTask} from '../apps/align/src/nonlinearTasks'
import {createPointMapping} from '../packages/imaging-math/nonlinearSpline'
const {JSDOM}=createRequire(import.meta.url)('jsdom'),dom=new JSDOM('<button id="alignment-cancel" class="hidden"></button>')
Object.assign(globalThis,{window:dom.window,document:dom.window.document})
const assets=path.resolve(process.argv[2]??'apps/align/dist','assets'),workerFile=fs.readdirSync(assets).find(f=>/^nonlinearWorker-.*\.js$/.test(f));assert(workerFile,'Nonlinear worker must exist in emitted app')
const code=fs.readFileSync(path.join(assets,workerFile),'utf8');let terminated=0
class ActualWorker{
 onmessage:((event:any)=>void)|null=null;onerror:((event:any)=>void)|null=null
 worker=new NodeWorker(`const {parentPort}=require('node:worker_threads');globalThis.self={postMessage:(m,t)=>parentPort.postMessage(m,t)};parentPort.on('message',data=>self.onmessage({data}));\n${code}`,{eval:true})
 constructor(){this.worker.on('message',data=>this.onmessage?.({data}));this.worker.on('error',e=>this.onerror?.({message:e.message}))}
 postMessage(data:unknown){this.worker.postMessage(data)}terminate(){terminated++;void this.worker.terminate()}
}
Object.assign(globalThis,{Worker:ActualWorker})
const points:number[][]=[];for(const x of [-8,0,8])for(const y of [-8,0,8])for(const z of [-8,0,8])points.push([x,y,z])
const phases:string[]=[],progress=(text:string)=>phases.push(text)
const fit:any=await runNonlinearTask({type:'fit',source:points,target:points.map(([x,y,z])=>[x+.2*Math.sin(y/5),y+.2*Math.sin(z/5),z+.2*Math.sin(x/5)]),smoothing:.01},progress)
assert(fit.rms<fit.affineRms);assert(fit.spline.lipschitzBound<1)
const raw={dims:[17,17,17],affine:[[1,0,0,-8],[0,1,0,-8],[0,0,1,-8],[0,0,0,1]],pixDims:[1,1,1],values:Float32Array.from({length:17**3},(_,i)=>i%17),frameCount:1,datatypeCode:16,littleEndian:true,slope:1,intercept:0}
const output=await runNonlinearTask<Float32Array>({type:'resample',source:raw,target:raw,matrix:fit.matrix,spline:fit.spline},progress);assert(output instanceof Float32Array);assert.equal(output.length,17**3);assert(raw.values.byteLength>0,'Worker must not detach loaded image buffers')
for(const direction of ['forward','inverse']){const bytes=await runNonlinearTask<Uint8Array>({type:'field',reference:raw,matrix:fit.matrix,spline:fit.spline,direction},progress);assert(bytes instanceof Uint8Array);assert.equal(bytes.length,352+12*17**3)}
await assert.rejects(()=>runNonlinearTask({type:'fit',source:points.slice(0,3),target:points.slice(0,3),smoothing:.01},progress),/12/)
const pending=runNonlinearTask({type:'resample',source:raw,target:{...raw,dims:[80,80,80]},matrix:fit.matrix,spline:fit.spline},progress)
dom.window.document.querySelector<HTMLButtonElement>('#alignment-cancel')!.click();await assert.rejects(()=>pending,/cancelled/)
assert(dom.window.document.querySelector('#alignment-cancel')!.classList.contains('hidden'));assert(terminated>=6);assert(phases.some(p=>p.includes('Solving')));assert(phases.some(p=>p.includes('slice')))
const mapping=createPointMapping(fit.matrix,fit.spline);assert(Math.hypot(...mapping.inverse(mapping.forward([3,4,5])).map((v,i)=>v-[3,4,5][i]))<1e-6)
console.log('Exact emitted nonlinear worker and task controller: spline fitting, transfer-safe resampling, forward/inverse field export, validation error propagation, progress, cancellation/termination and same-map inverse passed. GPU/native visual output remains pending.')
dom.window.close()
