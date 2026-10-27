import assert from 'node:assert/strict'
import fs from 'node:fs'
import path from 'node:path'
import {Worker} from 'node:worker_threads'
import type {LandmarkSensitivityInput,LandmarkSensitivityReport} from '../packages/imaging-math/landmarkSensitivity'
import type {Vec3} from '../packages/imaging-math/rigid'

const assets=path.resolve(process.argv[2]??'apps/align/dist','assets'),filename=fs.readdirSync(assets).find(name=>/^landmarkQualityWorker-.*\.js$/.test(name))
assert(filename,'Build must emit the on-demand landmark sensitivity worker')
const source=fs.readFileSync(path.join(assets,filename),'utf8'),workers:Worker[]=[]
const spawn=()=>{const worker=new Worker(`const {parentPort}=require('node:worker_threads');globalThis.self={postMessage:message=>parentPort.postMessage(message)};parentPort.on('message',data=>self.onmessage({data}));\n${source}`,{eval:true});workers.push(worker);return worker}
async function run(input:LandmarkSensitivityInput){
 const worker=spawn(),progress:number[]=[]
 try{
  const report=await new Promise<LandmarkSensitivityReport>((resolve,reject)=>{const timer=setTimeout(()=>reject(Error('Worker timeout')),20000);worker.on('error',error=>{clearTimeout(timer);reject(error)});worker.on('message',message=>{if(message.type==='progress'){progress.push(message.completed);return}clearTimeout(timer);message.type==='error'?reject(Error(message.message)):resolve(message.report)});worker.postMessage(input)})
  return {report,progress}
 }finally{await worker.terminate()}
}
try{
 const points:Vec3[]=[];for(const x of [-10,0,10])for(const y of [-10,0,10])for(const z of [-10,0,10])points.push([x,y,z])
 const input:LandmarkSensitivityInput={model:'spline',smoothing:.01,pairs:points.map((source,i)=>({id:i+1,source,target:source.map((v,a)=>v+.03*Math.sin(i+a)) as Vec3}))},snapshot=structuredClone(input)
 const {report,progress}=await run(input)
 assert.equal(report.status,'complete');assert.equal(report.validOmissions,27);assert.deepEqual(progress,Array.from({length:27},(_,i)=>i+1));assert.deepEqual(input,snapshot)
 const below=await run({...input,pairs:input.pairs.slice(0,19)});assert.equal(below.report.status,'unavailable');assert.equal(below.progress.length,0)
 await assert.rejects(()=>run({...input,model:'unknown' as any}),/Unsupported sensitivity model/)
 const cancellable=spawn();let messages=0;cancellable.on('message',()=>messages++);cancellable.postMessage(input);await cancellable.terminate();await new Promise(r=>setTimeout(r,25));assert.equal(messages,0,'Terminating a requested worker prevents later results');assert.deepEqual(input,snapshot)
 console.log('Actual emitted landmark-quality worker: bounded nonlinear leave-one-out refits, progress, count gate, error propagation, immediate termination and immutable caller state passed.')
}finally{await Promise.all(workers.map(worker=>worker.terminate()))}
