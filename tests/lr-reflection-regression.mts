import fs from 'node:fs'
import assert from 'node:assert/strict'
import {JSDOM} from 'jsdom'
import {optimizeConstrainedRefinement} from '../packages/imaging-math/automaticLinear'
import {createAlignmentActivity} from '../apps/align/src/workflowUI'
import {fitLandmarks,leftRightReflection,diagnoseTransform,type LandmarkModel} from '../packages/imaging-math/landmarkFit'
import {applyMat4,multiplyMat4,rigidDelta,type Vec3} from '../packages/imaging-math/rigid'
import {invertAffine,type RawNifti} from '../packages/imaging-math/roiWarp'
import {createPointMapping,fitNonlinearSpline} from '../packages/imaging-math/nonlinearSpline'
import {landmarkLeaveOneOutSensitivity} from '../packages/imaging-math/landmarkSensitivity'
import {initialManualState,applyManualAffine,resetManualAffine,manualDelta,registrationModalities} from '../apps/align/src/manualAffine'
import {manualPreviewDelta} from '../apps/align/src/manualPreview'
import {createSessionPayload,parseSessionPayload} from '../apps/align/src/sessionPersistence'
import {createRegistrationArtifacts} from '../apps/align/src/exportArtifacts'
import {landmarkSensitivityInput} from '../apps/align/src/landmarkQuality'
import {imageGeometryAlignment} from '../apps/align/src/geometryInitialization'
import {installManualAdjustment} from '../apps/align/src/manualAdjustmentUI'
import {installLandmarkReflectionControls} from '../apps/align/src/landmarkReflectionUI'
import {alignmentLayout} from '../apps/align/src/alignmentLayout'
import {imageFromRaw} from '../apps/align/src/volumeImages'
import type {FitState,Modality} from '../apps/align/src/appTypes'

const close=(a:number[],b:number[],e=1e-6)=>assert(a.every((v,i)=>Math.abs(v-b[i])<e),`${a} != ${b}`)
const points:Vec3[]=[[-18,-12,-9],[21,-13,4],[-12,22,6],[7,8,25],[3,-4,-18],[-23,7,12],[15,17,-6],[4,1,5]],mirror=leftRightReflection([11,-3,2]),models:LandmarkModel[]=['rigid','similarity','anisotropic']
const scale=(s:number[])=>[[s[0],0,0,0],[0,s[1],0,0],[0,0,s[2],0],[0,0,0,1]]
const oracle=JSON.parse(fs.readFileSync(new URL('./fixtures/lr-procrustes-reference.json',import.meta.url),'utf8'))
for(const item of oracle.cases)for(const branch of ['proper','allow'] as const){const actual=fitLandmarks(item.source,item.target,item.model,{reflection:branch}),expected=item.expected[branch];close(actual.matrix.flat(),expected.matrix.flat(),1e-6);assert(Math.abs(actual.rms-expected.rms)<1e-8);assert.equal(actual.diagnostics.reflection,expected.reflected)}
const sourceSnapshot=structuredClone(points)
const localStart=leftRightReflection([0,0,0]),localTruth=multiplyMat4(rigidDelta([1,0,0,0,0,0],[0,0,0]),localStart)
const local=await optimizeConstrainedRefinement({settings:{model:'rigid',enabledParameters:['tx','ty','tz','rx','ry','rz'],rotationRanges:[15,15,15],translationLimitMm:2,scaleLimit:0,shearLimit:0,candidatesToRefine:1,shrinkFactors:[1],sweepsPerLevel:[1],refinement:{translationStepMm:1,rotationStepDegrees:1}},center:[0,0,0],start:localStart,score:m=>-m.flat().reduce((sum,v,i)=>sum+(v-localTruth.flat()[i])**2,0),progress:()=>{},cancelled:()=>false})
assert(local.determinant<0);close(local.matrix.flat(),localTruth.flat())

for(const model of models)for(const angle of [2,90,137,179.999])for(const reverse of [false,true]){
 const s=model==='rigid'?[1,1,1]:model==='similarity'?[1.13,1.13,1.13]:[.85,1.19,1.08],truth=multiplyMat4(rigidDelta([7,-11,5,angle,-22,41],[3,6,-4]),multiplyMat4(scale(s),mirror)),target=points.map(p=>applyMat4(truth,p)),from=reverse?target:points,to=reverse?points:target,expected=reverse?invertAffine(truth):truth
 // R diag(s) is direction-dependent: the reverse of a rotated anisotropic
 // scale need not be in the same no-shear model. Test its forward recovery;
 // its general affine inverse is checked separately below.
 if(model==='anisotropic'&&reverse)continue
 const fit=fitLandmarks(from,to,model,{reflection:'allow'})
 assert(fit.diagnostics.reflection);assert.equal(fit.parameterCount,model==='rigid'?6:model==='similarity'?7:9);assert(fit.reflectionSelection!.applied);assert(!fit.reflectionSelection!.ambiguous)
 close(fit.matrix.flat(),expected.flat());assert(fit.rms<1e-6);assert(fit.diagnostics.shear<1e-6)
 for(const p of from)close(applyMat4(fit.inverse,applyMat4(fit.matrix,p)),p)
 if(model==='rigid')close(fit.diagnostics.scales,[1,1,1]);if(model==='similarity')assert(Math.max(...fit.diagnostics.scales)-Math.min(...fit.diagnostics.scales)<1e-8)
 const legacy=fitLandmarks(from,to,model);assert(!legacy.diagnostics.reflection);assert.equal(legacy.reflectionSelection,undefined)
 const ordinary=fitLandmarks(points,points.map(p=>applyMat4(rigidDelta([1,2,3,angle,8,-14],[0,0,0]),p)),model,{reflection:'allow'});assert(!ordinary.diagnostics.reflection)
}
assert.deepEqual(points,sourceSnapshot)
const planar:Vec3[]=[[0,0,3],[12,0,3],[0,18,3],[12,18,3]]
const ambiguous=fitLandmarks(planar,planar.map(p=>applyMat4(mirror,p)),'rigid',{reflection:'allow'});assert(!ambiguous.diagnostics.reflection);assert(ambiguous.reflectionSelection!.ambiguous);assert(ambiguous.diagnostics.warnings.some(w=>w.includes('cannot reliably distinguish')))
assert(fitLandmarks(planar.slice(0,3),planar.slice(0,3).map(p=>applyMat4(mirror,p)),'rigid',{reflection:'require'}).diagnostics.reflection)
assert.throws(()=>fitLandmarks(points,points.map(p=>applyMat4(mirror,p)),'affine',{reflection:'proper'}),/reflection is off/)
assert(fitLandmarks(points,points.map(p=>applyMat4(mirror,p)),'affine',{reflection:'allow'}).diagnostics.reflection)
const reflected=fitLandmarks(points,points.map(p=>applyMat4(mirror,p)),'rigid',{reflection:'allow'})
const landmarks=points.map((ct,i)=>({id:i+1,enabled:true,ct,mri:applyMat4(mirror,ct)}))
const fit:FitState={...imageGeometryAlignment('ct-mri'),...reflected,landmarkAllowReflection:true,landmarkReflection:reflected.reflectionSelection,initializationSource:'landmarks',landmarkModel:'rigid',landmarkMatrix:reflected.matrix,baseMatrix:reflected.matrix,reflectionConfirmed:true,landmarkSnapshot:landmarks}
const payload=createSessionPayload({appVersion:'test',landmarkAllowReflection:true,loaded:{mri:null,ct:null},landmarks,selectedId:1,nextId:9,direction:fit.direction,fit,optimizationWindows:{mri:{},ct:{}}}),saved=parseSessionPayload(JSON.parse(JSON.stringify(payload)))
assert(saved.landmarkAllowReflection);assert.equal(saved.fit!.model,'rigid');assert.equal(saved.fit!.parameterCount,6);assert(saved.fit!.diagnostics!.reflection);assert.equal(saved.schemaVersion,15)
const meta=JSON.parse(await createRegistrationArtifacts({appVersion:'test',fit:saved.fit!,landmarks})[0].blob.text());assert.equal(meta.parameter_count,6);assert(meta.reflection);assert(meta.landmark_reflection_selection.applied)
assert.throws(()=>createRegistrationArtifacts({appVersion:'test',fit:{...fit,reflectionConfirmed:false},landmarks}),/confirmation/)
const legacy=structuredClone(payload);legacy.schemaVersion=14;delete legacy.landmarkAllowReflection;legacy.fit=null;assert.equal(parseSessionPayload(legacy).landmarkAllowReflection,false)
const bad=structuredClone(payload);bad.fit!.landmarkReflection!.applied=false;assert.throws(()=>parseSessionPayload(bad),/reflection provenance/)
const sensitivity=landmarkSensitivityInput(fit);assert.equal(sensitivity.reflection,'require');const quality=await landmarkLeaveOneOutSensitivity(sensitivity);assert.equal(quality.status,'complete');assert(quality.heldOutRmsMm!<1e-6)
const centers={mri:[11,7,-2] as Vec3,ct:[-3,4,8] as Vec3}
for(const direction of ['ct-mri','mri-ct'])for(const target of ['mri','ct'] as Modality[]){
 const base=imageGeometryAlignment(direction),state=initialManualState(base,centers,target),{moving,fixed}=registrationModalities(direction);state.version=2;state.reflections={mri:false,ct:false};state.reflections[target]=true;state.parameters[target][0]=2;state.parameters[target][4]=15
 const adjusted=applyManualAffine(base,state),dm=manualDelta(state.parameters[moving],centers[moving],state.reflections[moving]),df=invertAffine(manualDelta(state.parameters[fixed],centers[fixed],state.reflections[fixed]))
 assert(adjusted.diagnostics!.reflection);assert.equal(adjusted.model,'rigid');assert.equal(adjusted.parameterCount,6);assert.equal(adjusted.reflectionConfirmed,false,'Algebra does not imply consent')
 for(const p of points)close(createPointMapping(adjusted.matrix).forward(p),applyMat4(dm,applyMat4(base.matrix,applyMat4(df,p))))
 close(manualPreviewDelta(base,adjusted,target)!.flat(),manualDelta(state.parameters[target],centers[target],true).flat())
 adjusted.reflectionConfirmed=true;const restored=parseSessionPayload(createSessionPayload({...payload,loaded:{mri:null,ct:null},direction,fit:adjusted})).fit!;close(restored.matrix.flat(),adjusted.matrix.flat());assert(restored.manualAffine!.reflections![target])
 const unflipped=structuredClone(state);unflipped.reflections![target]=false;const twice=applyManualAffine(adjusted,unflipped);assert(!twice.diagnostics!.reflection);assert(!twice.reflectionConfirmed);assert.deepEqual(resetManualAffine(adjusted).matrix,base.matrix)
 const metadata=JSON.parse(await createRegistrationArtifacts({appVersion:'test',fit:restored,landmarks:[]})[0].blob.text());assert(metadata.manual_adjustment.left_right_flips[target]);assert.equal(metadata.parameter_count,6)
}
const dom=new JSDOM(alignmentLayout('reflection'),{pretendToBeVisual:true});Object.assign(globalThis,{window:dom.window,document:dom.window.document})
const controls=installLandmarkReflectionControls();assert(!controls.enabled);document.querySelector<HTMLInputElement>('#landmark-allow-reflection')!.checked=true;assert(controls.enabled);controls.restore(false);assert(!controls.enabled)
const raw:RawNifti={dims:[5,7,9],pixDims:[.7,1.2,2],affine:[[0,0,2,-8],[.7,0,0,-1.4],[0,-1.2,0,3.6],[0,0,0,1]],values:new Float32Array(315),datatypeCode:16,slope:1,intercept:0,littleEndian:true,frameCount:1},voxels=raw.values.slice(),loaded:any={mri:{raw,nvImage:imageFromRaw(raw,'MRI')},ct:{raw,nvImage:imageFromRaw(raw,'CT')}}
let current=imageGeometryAlignment('ct-mri'),busy=false,changes=0,confirm=false
window.confirm=()=>confirm
const ui=installManualAdjustment({getFit:()=>current,setFit:f=>{current=f},getLoaded:()=>loaded,busy:()=>busy,changed:()=>changes++,targetChanged:()=>{},previewChanged:()=>{},status:()=>{}}),button=document.querySelector<HTMLButtonElement>('#manual-flip-lr')!
const initial=current;button.click();assert.equal(current,initial);assert.equal(changes,0)
confirm=true;button.click();assert(current.diagnostics!.reflection);assert(current.reflectionConfirmed);assert(current.manualAffine!.reflections!.ct);assert.equal(current.parameterCount,6);assert.equal(button.getAttribute('aria-pressed'),'true');button.click();assert(!current.diagnostics!.reflection)
document.querySelector<HTMLButtonElement>('[data-manual-target=mri]')!.click();button.click();assert(current.manualAffine!.reflections!.mri);assert(!current.manualAffine!.reflections!.ct);assert.equal(current.direction,'ct-mri');assert.deepEqual(raw.values,voxels)
busy=true;window.dispatchEvent(new window.Event('align-activity-change'));assert(button.disabled);busy=false;window.dispatchEvent(new window.Event('align-activity-change'));assert(!button.disabled)
const activity=createAlignmentActivity();let activityChanges=0;window.addEventListener('align-activity-change',()=>activityChanges++);Object.assign(globalThis,{requestAnimationFrame:(fn:any)=>setTimeout(fn,0)})
busy=true;await activity.begin('Exporting…','export');assert.equal(activityChanges,1);assert(button.disabled);busy=false;activity.end();assert.equal(activityChanges,2);assert(!button.disabled)
current.proposal={matrix:current.matrix,params:[],scoreBefore:0,scoreAfter:0,kind:'spline'};ui.refresh();assert(button.disabled)
const pts:Vec3[]=[];for(const x of [-4,0,4])for(const y of [-4,4])for(const z of [-4,4])pts.push([x,y,z]);const spline=fitNonlinearSpline(pts,pts,.1),nonlinear={...imageGeometryAlignment('ct-mri'),model:'spline' as const,nonlinear:spline.spline,matrix:spline.matrix,baseMatrix:spline.matrix};current=nonlinear;ui.refresh();assert(button.disabled);assert(document.querySelector('#manual-flip-status')!.textContent!.includes('linear alignment'))
const illegal=initialManualState(nonlinear,centers,'ct');illegal.version=2;illegal.reflections={mri:false,ct:true};assert.throws(()=>applyManualAffine(nonlinear,illegal),/linear alignment/)
dom.window.close();console.log('LR reflection: exact 6/7/9 fits and inverse maps, large rotations/directions, proper defaults, planar ambiguity, fixed-branch sensitivity, schema15, named export provenance/consent, both manual targets, preview composition, involution/reset, source immutability, UI decline/busy/proposal guards and nonlinear preservation passed.')
