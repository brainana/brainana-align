import assert from 'node:assert/strict'
import {runWorkerTask} from '../packages/core-client/workerTask'
for(const path of ['success','error','crash','unreadable','invalid','unknown','callback','callback-null','post','cancel','constructor','already-aborted']){
 let worker:any,terminated=0,attached=0,created=0
 const controller=new AbortController(),signal=controller.signal,add=signal.addEventListener.bind(signal),remove=signal.removeEventListener.bind(signal)
 signal.addEventListener=((...args:any[])=>{attached++;return (add as any)(...args)}) as any
 signal.removeEventListener=((...args:any[])=>{attached--;return (remove as any)(...args)}) as any
 if(path==='already-aborted')controller.abort()
 const promise=runWorkerTask<number>({create:()=>{created++;if(path==='constructor')throw Error('construction failed');return worker={postMessage(){if(path==='post')throw Error('clone failed')},terminate(){terminated++}}},request:{},signal,cancelled:'cancelled',failed:'failed',progress:()=>{if(path==='callback')throw Error('progress failed');if(path==='callback-null')throw null},result:data=>data.result})
 const failure=path==='success'?null:assert.rejects(promise)
 if(worker?.onmessage){
  if(path==='success')worker.onmessage({data:{type:'result',result:42}})
  if(path==='error')worker.onmessage({data:{type:'error',message:'bad'}})
  if(path==='crash')worker.onerror({message:'crash'})
  if(path==='unreadable')worker.onmessageerror({})
  if(path==='invalid')worker.onmessage({data:null})
  if(path==='unknown')worker.onmessage({data:{type:'unexpected'}})
  if(path==='callback'||path==='callback-null')worker.onmessage({data:{type:'progress'}})
  if(path==='cancel')controller.abort()
 }
 if(failure)await failure;else assert.equal(await promise,42)
 assert.equal(terminated,worker?1:0,path);assert.equal(attached,0,path);if(worker){assert.equal(worker.onmessage,null);assert.equal(worker.onerror,null);assert.equal(worker.onmessageerror,null)}
 controller.abort();assert.equal(terminated,worker?1:0);if(path==='already-aborted')assert.equal(created,0)
}
console.log('Worker task: success, worker error/crash, response decoding, unexpected messages, callback/clone/construction failures, early and late cancellation all settle once and release ownership.')
