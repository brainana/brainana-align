import assert from 'node:assert/strict'
import {createRequire} from 'node:module'
import {createOperationState,initialOperationState,reduceOperation,projectGuidePriority} from '../apps/align/src/operationState'
import {createOverlayScheduler} from '../apps/align/src/overlayScheduler'
import {createAlignmentActivity} from '../apps/align/src/workflowUI'
const deferred=<T>()=>{let resolve!:(value:T)=>void,reject!:(error:unknown)=>void;const promise=new Promise<T>((a,b)=>{resolve=a;reject=b});return {promise,resolve,reject}}
const store=createOperationState(),scheduler=createOverlayScheduler(store)
const old=deferred<number>(),newer=deferred<number>()
let oldSignal:AbortSignal|undefined
const first=scheduler.run(signal=>{oldSignal=signal;return old.promise}),firstGeneration=scheduler.state.generation
const second=scheduler.run(()=>newer.promise)
assert(oldSignal?.aborted,'Supersession must abort before waiting for the old transport')
newer.resolve(2);assert.deepEqual(await second,{status:'ready',result:2,generation:scheduler.state.generation})
const readyState=store.state;old.resolve(1);assert.equal((await first).status,'stale');assert.equal(store.state,readyState)
store.dispatch({type:'overlay/settle',generation:firstGeneration,status:'failed'});assert.equal(store.state,readyState)
const cancelWait=deferred<number>(),cancelled=scheduler.run(()=>cancelWait.promise);scheduler.cancel();cancelWait.resolve(4);assert.equal((await cancelled).status,'cancelled');assert.equal(scheduler.state.status,'cancelled')
const failed=await scheduler.run(async()=>{throw Error('Worker failure')});assert.equal(failed.status,'failed');assert.equal(scheduler.state.status,'failed')
let starts=0
scheduler.schedule(()=>{starts++},5);scheduler.schedule(()=>{starts++},5)
await new Promise(resolve=>setTimeout(resolve,20));assert.equal(starts,1,'Nudge bursts must start one settled task')
scheduler.schedule(()=>{starts++},5);scheduler.invalidate();await new Promise(resolve=>setTimeout(resolve,20));assert.equal(starts,1,'Reset invalidates deferred work')
const staleError=deferred<number>(),staleTask=scheduler.run(()=>staleError.promise);scheduler.invalidate();staleError.reject(Error('late error'));assert.equal((await staleTask).status,'stale');assert.equal(scheduler.state.status,'idle')

let state=initialOperationState()
state=reduceOperation(state,{type:'activity/begin',kind:'processing',text:'Processing'})
const a=state.activity.id
assert.equal(reduceOperation(state,{type:'activity/begin',kind:'registration',text:'Duplicate'}),state)
state=reduceOperation(state,{type:'activity/cancel',id:a});assert.equal(state.activity.status,'cancelling')
assert.equal(reduceOperation(state,{type:'activity/phase',id:a,text:'late progress'}),state)
state=reduceOperation(state,{type:'activity/end',id:a});assert.equal(state.activity.outcome,'cancelled')
state=reduceOperation(state,{type:'activity/begin',kind:'export',text:'Export'})
assert.equal(reduceOperation(state,{type:'activity/end',id:a,outcome:'failed'}),state)
assert.equal(reduceOperation(state,{type:'activity/phase',id:a,text:'stale'}),state)
state=reduceOperation(state,{type:'proposal/project',pending:true});state=reduceOperation(state,{type:'proposal/decision',decision:'rejected'});assert.equal(state.proposal,'rejected')
assert.equal(reduceOperation(state,{type:'proposal/decision',decision:'accepted'}),state,'A repeated decision cannot decide a new proposal')
state=reduceOperation(state,{type:'proposal/project',pending:true});assert.equal(state.proposal,'pending')
state=reduceOperation(state,{type:'proposal/clear'});assert.equal(state.proposal,'none')

// Exercise all combinations of the guide entry predicates and every busy kind.
// Existing routing/teaching stays in guidedSetup; this projection has no effects.
for(let bits=0;bits<256;bits++)for(const busyKind of ['registration','processing','preparation','export','display'] as const)for(const route of ['choose','manual','done','refinement']){
 const s={welcome:Boolean(bits&1),viewerReady:Boolean(bits&2),mri:Boolean(bits&4),ct:Boolean(bits&8),preparation:{mri:Boolean(bits&16),ct:Boolean(bits&32),ready:Boolean(bits&64)},busy:Boolean(bits&128),busyKind,pendingProposal:true},snapshot=structuredClone(s)
 const actual=projectGuidePriority(s,route)
 const expected=s.welcome?'welcome':!s.viewerReady?'viewer':!s.mri&&!s.preparation.mri?'mri':!s.ct&&!s.preparation.ct?'ct':!s.preparation.ready&&!s.busy?'prepare':s.busy?'working':route!=='choose'&&route!=='done'?'proposal':'route'
 assert.equal(actual.key,expected);assert.deepEqual(s,snapshot)
 if(actual.key==='working')assert.equal(actual.activityKind,busyKind)
}
const ready={viewerReady:true,mri:true,ct:true,busy:false,pendingProposal:true}
assert.equal(projectGuidePriority(ready,'manual').key,'proposal');assert.equal(projectGuidePriority({...ready,pendingProposal:false},'manual').key,'route')

// Real activity DOM checks: cancelling during the initial paint cannot leave an
// inert workspace, and old progress/end callbacks cannot hide the next activity.
const {JSDOM}=createRequire(import.meta.url)('jsdom'),dom=new JSDOM('<main></main><header class="topbar"></header><div id="alignment-progress" class="hidden"><span id="alignment-progress-label"></span><progress></progress><button id="alignment-cancel"></button></div>',{pretendToBeVisual:true})
Object.assign(globalThis,{window:dom.window,document:dom.window.document,requestAnimationFrame:dom.window.requestAnimationFrame.bind(dom.window)})
const activity=createAlignmentActivity(),button=document.querySelector<HTMLButtonElement>('#alignment-cancel')!
const beginning=activity.begin('Preparing','preparation');button.click();assert.equal(await beginning,false);assert.equal(activity.busy,false);assert.equal(document.querySelector('main')!.inert,false)
assert.equal(await activity.begin('Old operation'),true);const oldId=activity.id;activity.end('failed',oldId)
assert.equal(await activity.begin('New operation','export'),true);await activity.phase('Stale progress',.9,oldId);activity.end('completed',oldId)
assert.equal(activity.busy,true);assert.equal(document.querySelector('#alignment-progress-label')!.textContent,'New operation');assert.equal(document.querySelector('main')!.inert,true)
button.click();await activity.phase('After cancellation');assert.equal(document.querySelector('#alignment-progress-label')!.textContent,'New operation');activity.end();assert.equal(activity.state.activity.outcome,'cancelled')
dom.window.close()
console.log('Typed operation/guide projection and latest-overlay scheduling: cancellation, failure, stale completion/progress, duplicate start/decision, coalescing, reset and all entry-priority combinations passed. Bounded ordering checks do not exhaust every UI sequence.')
