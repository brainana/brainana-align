import assert from 'node:assert/strict'
import {JSDOM} from 'jsdom'
import {landmarkLeaveOneOutSensitivity,LANDMARK_SENSITIVITY_MIN_PAIRS,type SensitivityPair,type SensitivityModel} from '../packages/imaging-math/landmarkSensitivity'
import {fitLandmarks} from '../packages/imaging-math/landmarkFit'
import {fitSummaryHtml} from '../apps/align/src/fitReview'
import {installLandmarkSensitivityReview,landmarkSensitivityAvailability,landmarkSensitivityInput} from '../apps/align/src/landmarkQuality'
import type {FitState} from '../apps/align/src/appTypes'
import type {Vec3} from '../packages/imaging-math/rigid'

const points=(n:number,scale=20):Vec3[]=>Array.from({length:n},(_,i)=>{const z=1-2*(i+.5)/n,r=Math.sqrt(1-z*z),angle=i*2.399963229728653;return [scale*r*Math.cos(angle),scale*r*Math.sin(angle),scale*z]})
const pairs=(n:number,scale=20,noise=0):SensitivityPair[]=>points(n,scale).map((source,i)=>({id:i+1,source,target:source.map((v,a)=>v+[2,-1,3][a]+noise*Math.sin(i*1.7+a*2.1)) as Vec3}))
for(const [model,minimum] of Object.entries(LANDMARK_SENSITIVITY_MIN_PAIRS) as [SensitivityModel,number][]){
 const below=await landmarkLeaveOneOutSensitivity({model,pairs:pairs(minimum-1)})
 assert.equal(below.status,'unavailable');assert.equal(below.minimumPairs,minimum);assert.equal(below.omissions.length,0)
 const at=await landmarkLeaveOneOutSensitivity({model,pairs:pairs(minimum)})
 assert.equal(at.status,'complete',`${model} at its conservative count gate: ${JSON.stringify(at)}`);assert.equal(at.validOmissions,minimum);assert(at.heldOutRmsMm!<1e-6)
}
const noisy=pairs(12,20,.15),snapshot=structuredClone(noisy),probePoints:Vec3[]=[[0,0,0],[60,40,-30]],probeSnapshot=structuredClone(probePoints)
const noise=await landmarkLeaveOneOutSensitivity({model:'rigid',pairs:noisy,probePoints})
assert.equal(noise.status,'complete');assert(noise.heldOutRmsMm!>.1);assert(noise.maximumMappingChangeMm!>0);assert.deepEqual(noisy,snapshot);assert.deepEqual(probePoints,probeSnapshot)
const bad=pairs(12);bad[7].target[0]+=8
const badReport=await landmarkLeaveOneOutSensitivity({model:'rigid',pairs:bad})
const worst=[...badReport.omissions].sort((a,b)=>b.heldOutErrorMm!-a.heldOutErrorMm!)[0]
assert.equal(worst.id,bad[7].id,'A known incorrect pair has the largest omitted-point prediction error');assert(worst.heldOutErrorMm!>7.9)
const dispersed=await landmarkLeaveOneOutSensitivity({model:'rigid',pairs:pairs(12,20,.02),probePoints}),clustered=await landmarkLeaveOneOutSensitivity({model:'rigid',pairs:pairs(12,.2,.02),probePoints})
assert(clustered.maximumMappingChangeMm!>dispersed.maximumMappingChangeMm!*20,'Small localization noise in a tight cluster destabilizes distant probe mappings');assert(clustered.sourceRadiusMm!<dispersed.sourceRadiusMm!/50)
const planar:Vec3[]=[[-3,-2,0],[2,-2,0],[3,2,0],[-2,3,0],[1,0,0],[-1,-1,0],[0,2,0],[0,0,5]]
const degenerates=planar.map((source,i)=>({id:i+1,source,target:source.map((v,a)=>v+[2,3,4][a]) as Vec3}))
const unstable=await landmarkLeaveOneOutSensitivity({model:'affine',pairs:degenerates})
assert.equal(unstable.status,'unstable');assert.equal(unstable.validOmissions,7);assert.equal(unstable.omissions.find(row=>row.id===8)!.valid,false);assert.match(unstable.omissions.find(row=>row.id===8)!.reason!,/coplanar|degenerate|collinear/);assert(unstable.warnings.some(w=>w.includes('7/8')))
const line=pairs(6).map((p,i)=>({...p,source:[i,0,0] as Vec3,target:[i+1,0,0] as Vec3}))
assert.equal((await landmarkLeaveOneOutSensitivity({model:'rigid',pairs:line})).status,'unavailable','Count alone cannot validate a degenerate full fit')
assert.equal((await landmarkLeaveOneOutSensitivity({model:'spline',pairs:pairs(65)})).status,'unavailable','Spline work is explicitly capped')
let progress=0
await assert.rejects(()=>landmarkLeaveOneOutSensitivity({model:'rigid',pairs:noisy},{shouldCancel:()=>progress===2,onProgress:done=>{progress=done}}),/cancelled/);assert.equal(progress,2);assert.deepEqual(noisy,snapshot)

const makeFit=(direction='ct-mri'):FitState=>{const p=pairs(8,20,.03),f=fitLandmarks(p.map(p=>p.source),p.map(p=>p.target));return {direction,model:'rigid',landmarkModel:'rigid',matrix:f.matrix,inverse:f.inverse,baseMatrix:f.matrix,landmarkMatrix:f.matrix,manual:[0,0,0,0,0,0],rms:f.rms,proposal:null,landmarkSnapshot:p.map(p=>({id:p.id,ct:direction==='ct-mri'?p.source:p.target,mri:direction==='ct-mri'?p.target:p.source,enabled:true})),landmarksChanged:false,fittedAt:'test'}}
const dom=new JSDOM('<div id="summary"></div>',{pretendToBeVisual:true});Object.assign(globalThis,{window:dom.window,document:dom.window.document,MutationObserver:dom.window.MutationObserver})
const container=document.querySelector<HTMLElement>('#summary')!;let current=makeFit(),postCount=0,terminated=0
const originalFit=structuredClone(current)
const pending:ControlledWorker[]=[]
class ControlledWorker{
 onmessage:any;onerror:any;input:any
 constructor(){pending.push(this)}
 postMessage(input:any){postCount++;this.input=input}
 terminate(){terminated++}
 async complete(){const report=await landmarkLeaveOneOutSensitivity(this.input);this.onmessage?.({data:{type:'result',report}})}
}
Object.assign(globalThis,{Worker:ControlledWorker})
container.innerHTML=fitSummaryHtml(current)
const controller=installLandmarkSensitivityReview(container,()=>current),start=()=>container.querySelector<HTMLButtonElement>('[data-landmark-sensitivity-start]')!.click(),tick=()=>new Promise(r=>setTimeout(r,0))
assert.equal(postCount,0,'Opening/rebuilding a fit summary never runs refits');assert(container.textContent!.includes('Training RMS error:'));assert(container.textContent!.includes('not independent validation'))
start();assert.equal(postCount,1);assert(container.querySelector<HTMLButtonElement>('[data-landmark-sensitivity-start]')!.disabled)
await pending[0].complete();assert.equal(terminated,1);assert(container.textContent!.includes('8/8 valid refits'));assert(container.textContent!.includes('Omitted-point prediction RMS'));assert.deepEqual(current,originalFit)
container.innerHTML=fitSummaryHtml(current);await tick();assert(container.textContent!.includes('8/8 valid refits'),'Cached report survives a summary redraw');start();assert.equal(postCount,1,'Cached report is reused without more refits')
current=makeFit('mri-ct');container.innerHTML=fitSummaryHtml(current);await tick();const input=landmarkSensitivityInput(current);assert.deepEqual(input.pairs[0].source,current.landmarkSnapshot[0].mri);assert.deepEqual(input.pairs[0].target,current.landmarkSnapshot[0].ct)
start();assert.equal(postCount,2);container.querySelector<HTMLButtonElement>('[data-landmark-sensitivity-cancel]')!.click();assert.equal(terminated,2);assert(container.textContent!.includes('sensitivity cancelled'));await pending[1].complete();assert(container.textContent!.includes('sensitivity cancelled'),'Late result cannot replace cancellation feedback')
start();assert.equal(postCount,3);current=makeFit();container.innerHTML=fitSummaryHtml(current);await tick();assert.equal(terminated,3,'Changing the fitted snapshot terminates work');await pending[2].complete();assert(!container.textContent!.includes('8/8 valid refits'),'Stale result cannot attach to a replacement fit')
start();controller.dispose();assert.equal(terminated,4,'Disposal terminates the worker')
const tooFew=makeFit();tooFew.landmarkSnapshot=tooFew.landmarkSnapshot.slice(0,5);assert.equal(landmarkSensitivityAvailability(tooFew).available,false);assert(!fitSummaryHtml(tooFew).includes('data-landmark-sensitivity-start'))
assert.equal(landmarkSensitivityAvailability({...makeFit(),initializationSource:'automatic'}).available,false)
class FailedWorker{onmessage:any;onerror:any;postMessage(){throw Error('clone failed')}terminate(){terminated++}}
Object.assign(globalThis,{Worker:FailedWorker});container.innerHTML=fitSummaryHtml(current);const failed=installLandmarkSensitivityReview(container,()=>current);start();assert(container.textContent!.includes('clone failed'));assert.equal(terminated,5);assert.equal(container.querySelector<HTMLButtonElement>('[data-landmark-sensitivity-start]')!.disabled,false);failed.dispose()
dom.window.close()
console.log('Landmark sensitivity: conservative gates, noisy/bad/clustered controls, degenerate omitted subsets, immutable inputs, cancellation, bounded work, original-model/both-direction mapping, explicit on-demand UI, cached reports and worker cleanup/stale-result rejection passed.')
