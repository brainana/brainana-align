import {createWindowBounds,registrationWindows,withinOptimizationWindows} from '../apps/align/src/optimizationWindows'
import assert from 'node:assert/strict'
import { automaticParameterMatrix,optimizeAutomaticLinear,type AutomaticLinearSettings } from '../packages/imaging-math/automaticLinear'
import { runIntensityRegistration,makeIntensityScorer } from '../packages/imaging-math/intensityRegistration'
import { applyMat4,type Mat4,type Vec3 } from '../packages/imaging-math/rigid'
import { invertAffine,type RawNifti } from '../packages/imaging-math/roiWarp'
import { downsampleByFactor } from '../packages/imaging-math/imagePyramid'
import { parseSessionPayload } from '../apps/align/src/sessionPersistence'
import { createRegistrationArtifacts } from '../apps/align/src/exportArtifacts'
import { diagnoseTransform } from '../packages/imaging-math/landmarkFit'
import type { FitState } from '../apps/align/src/appTypes'
const settings=(model:AutomaticLinearSettings['model']):AutomaticLinearSettings=>({model,enabledParameters:['tx','ty','tz','rx','ry','rz'],rotationRanges:[0,0,0],translationLimitMm:4,scaleLimit:.35,shearLimit:.2,candidatesToRefine:2,shrinkFactors:[2,1],sweepsPerLevel:[4,5]})
const error=(a:Mat4,b:Mat4)=>a.slice(0,3).flatMap((row,r)=>row.map((v,c)=>(v-b[r][c])**2)).reduce((a,b)=>a+b,0)
for(const model of ['rigid','similarity','anisotropic','affine'] as const){
 const p=[2,-1,1,0,0,0,...(model==='rigid'?[0,0,0]:model==='similarity'?[.1,.1,.1]:[.1,-.08,.04]),...(model==='affine'?[.04,-.02,.03]:[0,0,0])],target=automaticParameterMatrix(p,[0,0,0],[0,0,0])
 const result=await optimizeAutomaticLinear({settings:settings(model),movingCenter:[0,0,0],fixedCenter:[0,0,0],score:(matrix,level)=>-error(matrix,target)+(level===0?1000:0)})
 assert(error(result.matrix,target)<.02,`${model} recovery`);assert(result.scoreAfter<=0,'Final score must use native level');assert(result.scoreAfter>=result.scoreBefore)
}
const custom=settings('custom');custom.enabledParameters.push('sy')
const customTarget=automaticParameterMatrix([0,0,0,0,0,0,0,.15,0,0,0,0],[0,0,0],[0,0,0])
const customResult=await optimizeAutomaticLinear({settings:custom,movingCenter:[0,0,0],fixedCenter:[0,0,0],score:m=>-error(m,customTarget)})
assert.equal(customResult.parameters[6],0);assert.equal(customResult.parameters[8],0);assert(customResult.parameters[7]>.1)
const phantom=(p:Vec3)=>{
 let value=1
 for(const [x,y,z,w,s] of [[-4,-2,1,90,2],[3,4,-3,50,3],[1,-5,-4,35,1.7],[-2,3,5,65,2.1]])value+=w*Math.exp(-((p[0]-x)**2+(p[1]-y)**2+(p[2]-z)**2)/(2*s*s))
 return value
}
function image(affine:Mat4,transform?:Mat4):RawNifti{const dims:Vec3=[23,23,23],values=new Float32Array(23**3);let i=0;for(let z=0;z<23;z++)for(let y=0;y<23;y++)for(let x=0;x<23;x++){let p=applyMat4(affine,[x,y,z]);if(transform)p=applyMat4(transform,p);values[i++]=phantom(p)}return {dims,values,affine,pixDims:[1,1,1],frameCount:1,datatypeCode:16,littleEndian:true,slope:1,intercept:0}}
const affine=[[1,0,0,-11],[0,1,0,-11],[0,0,1,-11],[0,0,0,1]],fixed=image(affine)
for(const angle of [90,175]){
 const target=automaticParameterMatrix([0,0,0,0,0,angle,0,0,0,0,0,0],[0,0,0],[0,0,0]),moving=image(affine,target),s=settings('rigid');s.rotationRanges=[0,0,180]
 const result=await runIntensityRegistration({fixed,moving,settings:s,metric:'normalized-correlation',bins:32,operation:'global',fixedWindows:[],movingWindows:[]},()=>{})
 assert(result.scoreAfter>.97,`Image rotation ${angle}: ${result.scoreAfter}`);assert(error(result.matrix,target)<.5,`Image rotation ${angle} matrix error ${error(result.matrix,target)}`)
 for(const metric of ['nmi','mi','correlation-ratio','normalized-correlation','least-squares'] as const){const score=makeIntensityScorer(fixed,moving,metric,32);assert(score(target)>score(affine.map((r,i)=>r.map((_,j)=>i===j?1:0))),metric)}
 const inverseScore=makeIntensityScorer(moving,fixed,'normalized-correlation',32);assert(inverseScore(invertAffine(target))>.98)
}
for(const model of ['similarity','anisotropic','affine'] as const){
 const target=automaticParameterMatrix([0,0,0,0,0,0,...(model==='similarity'?[.08,.08,.08]:[.08,-.06,.03]),...(model==='affine'?[.04,-.02,.03]:[0,0,0])],[0,0,0],[0,0,0]),moving=image(affine,target)
 const result=await runIntensityRegistration({fixed,moving,settings:settings(model),metric:'normalized-correlation',bins:32,operation:'global',fixedWindows:[],movingWindows:[]},()=>{})
 assert(result.scoreAfter>.98,`${model} image score ${result.scoreAfter}`);assert(error(result.matrix,target)<.3,`${model} image matrix ${error(result.matrix,target)}`)
 const local=await runIntensityRegistration({fixed,moving,settings:settings('rigid'),metric:'normalized-correlation',bins:32,operation:'local',start:target,fixedWindows:[],movingWindows:[]},()=>{})
 assert(local.scoreAfter>=local.scoreBefore);assert(error(local.matrix,target)<.1,'Local refinement preserves accepted nonrigid base')
}
// Oblique, anisotropic geometry and exact image transforms: correct physical
// matrix must win in both directions for every supported model.
const oblique=[[.8,-.4,.1,-5],[.6,1.2,.2,-8],[.1,.2,1.8,-12],[0,0,0,1]]
for(const model of ['rigid','similarity','anisotropic','affine'] as const){const target=automaticParameterMatrix([1,-2,1,20,10,90,...(model==='rigid'?[0,0,0]:model==='similarity'?[.1,.1,.1]:[.1,-.07,.05]),...(model==='affine'?[.08,.03,-.04]:[0,0,0])],[0,0,0],[0,0,0]);const f=image(oblique),m=image(oblique,target),score=makeIntensityScorer(f,m,'normalized-correlation',64);assert(score(target)>.98);assert(score(target)>score(automaticParameterMatrix(Array(12).fill(0),[0,0,0],[0,0,0])));assert(makeIntensityScorer(m,f,'normalized-correlation',64)(invertAffine(target))>.97)}
const shrunk=downsampleByFactor(image(oblique),4);assert.deepEqual(applyMat4(shrunk.affine,[0,0,0]),applyMat4(oblique,[0,0,0]));assert(error([[...applyMat4(shrunk.affine,shrunk.dims.map(v=>v-1) as Vec3),0],[0,0,0,0],[0,0,0,0]],[[...applyMat4(oblique,[22,22,22]),0],[0,0,0,0],[0,0,0,0]])<1e-12)
assert.equal(makeIntensityScorer(fixed,fixed,'nmi',32)([[1,0,0,1000],[0,1,0,0],[0,0,1,0],[0,0,0,1]]),-Infinity)
assert.throws(()=>makeIntensityScorer(fixed,fixed,'nmi',32,[{min:[100,100,100],max:[110,110,110]}]),/Too few/)
await assert.rejects(()=>runIntensityRegistration({fixed,moving:fixed,settings:settings('rigid'),metric:'nmi',bins:32,operation:'global',fixedWindows:[],movingWindows:[]},()=>{},()=>true),/cancelled/)
const matrix=automaticParameterMatrix([0,0,0,0,0,0,.1,.1,.1,0,0,0],[0,0,0],[0,0,0]),fit:FitState={direction:'ct-mri',initializationSource:'automatic',model:'similarity',matrix,inverse:invertAffine(matrix),baseMatrix:matrix,landmarkMatrix:matrix,rms:0,manual:[0,0,0,0,0,0],landmarkSnapshot:[],landmarksChanged:false,fittedAt:'test',diagnostics:diagnoseTransform(matrix),proposal:{matrix,scoreBefore:1,scoreAfter:2,params:Array(12).fill(0),kind:'automatic',previous:null},automaticRegistration:{operation:'global',model:'similarity',parameterCount:7,metric:'nmi',bins:32,settings:settings('similarity'),scoreBefore:1,scoreAfter:2,candidatesEvaluated:10,warnings:[]}}
const payload={application:'Brainana Align',schemaVersion:2,landmarks:[],direction:'ct-mri',fit}
assert.equal(parseSessionPayload(JSON.parse(JSON.stringify(payload))).fit!.proposal!.previous,null)
assert.throws(()=>createRegistrationArtifacts({fit,landmarks:[],appVersion:'test'}),/Accept or reject/)
fit.proposal=null;const artifacts=createRegistrationArtifacts({fit,landmarks:[],appVersion:'test'}),json=JSON.parse(await artifacts[0].blob.text());assert.equal(json.initialization_source,'automatic');assert.equal(json.rms_mm,null);assert.equal(json.automatic_registration.parameterCount,7)
console.log('Automatic registration: level score correction, four models, custom held scales, image 90/175°, both directions, five metrics, oblique anisotropic geometry, pyramid extent, windows, cancellation, sessions and exports passed.')

// A linear objective must reach, but never exceed, the user bounds at the exact start-step schedule.
const {optimizeConstrainedRefinement}=await import('../packages/imaging-math/automaticLinear')
const constrained=await optimizeConstrainedRefinement({settings:{model:'rigid',enabledParameters:[],rotationRanges:[1,1,1] as any,translationLimitMm:2,scaleLimit:.35,shearLimit:.3,candidatesToRefine:1,shrinkFactors:[1],sweepsPerLevel:[2],refinement:{translationStepMm:1,rotationStepDegrees:.5}},center:[0,0,0],start:[[1,0,0,0],[0,1,0,0],[0,0,1,0],[0,0,0,1]],score:m=>m[0][3],progress:()=>{},cancelled:()=>false})
assert.equal(constrained.parameters[0],2);assert(constrained.parameters.slice(0,3).every(v=>Math.abs(v)<=2));assert(constrained.parameters.slice(3,6).every(v=>Math.abs(v)<=1));assert(constrained.warnings.some(v=>v.includes('bound')))

// Real drawn windows have different slice depths; only their visible axes restrict samples.
const drawn={sagittal:createWindowBounds([4,-50,-50],[4,50,50],'sagittal'),coronal:createWindowBounds([-50,-3,-50],[50,-3,50],'coronal'),axial:createWindowBounds([-50,-50,7],[50,50,7],'axial')}
const windows=registrationWindows(drawn),identity=automaticParameterMatrix(Array(12).fill(0),[0,0,0],[0,0,0])
assert(withinOptimizationWindows([10,9,8],drawn));assert.deepEqual(windows.map(w=>w.axes),[[1,2],[0,2],[0,1]])
assert.throws(()=>makeIntensityScorer(fixed,fixed,'nmi',32,windows.map(({min,max})=>({min,max}))),/Too few/,'Reproduce the previous zero-thickness worker failure')
for(const geometry of [affine,oblique])for(const reverse of [false,true])for(const operation of ['global','local'] as const){
 const first=image(geometry),second=image(geometry),s=settings('rigid');s.shrinkFactors=[4,2,1];s.sweepsPerLevel=[1,1,1]
 if(operation==='local'){s.refinement={translationStepMm:1,rotationStepDegrees:.5};s.rotationRanges=[1,1,1]}
 const result=await runIntensityRegistration({fixed:reverse?second:first,moving:reverse?first:second,settings:s,metric:'normalized-correlation',bins:32,operation,start:identity,fixedWindows:windows,movingWindows:windows},()=>{})
 assert(Number.isFinite(result.scoreAfter),`${operation} plane windows, oblique geometry, reverse=${reverse}`)
}
for(const plane of ['sagittal','coronal','axial'] as const)assert(makeIntensityScorer(fixed,fixed,'normalized-correlation',32,registrationWindows({[plane]:drawn[plane]}))(identity)>.99)
assert.equal(makeIntensityScorer(fixed,fixed,'nmi',32,windows,[{min:[100,100,100],max:[110,110,110],axes:[0,1]}])(identity),-Infinity,'Moving windows remain enforced')
assert.throws(()=>makeIntensityScorer(fixed,fixed,'nmi',32,[{min:[100,100,100],max:[110,110,110],axes:[1,2]}]),/Too few/,'Genuinely empty fixed windows remain rejected')
console.log('Drawn plane windows: prior failure reproduced; all planes, both directions, oblique/anisotropic physical geometry, global pyramids, constrained refinement and fixed/moving rejection passed.')

const coarseInput:RawNifti={...fixed,dims:[41,41,41],affine:[[1,0,0,-20],[0,1,0,-20],[0,0,1,-20],[0,0,0,1]],values:Float32Array.from({length:41**3},(_,i)=>phantom([i%41-20,Math.floor(i/41)%41-20,Math.floor(i/1681)-20]))}
const coarse=downsampleByFactor(coarseInput,8)
assert(Number.isFinite(makeIntensityScorer(coarse,coarse,'normalized-correlation',32,windows,windows)(identity)),'8× preparation respects physical drawn-window axes')
