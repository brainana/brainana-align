import assert from 'node:assert/strict'
import fs from 'node:fs'
import {manualPreviewDelta,transformedPreviewImage} from '../apps/align/src/manualPreview'
import {initialManualState,applyManualAffine,registrationModalities,registrationPoint} from '../apps/align/src/manualAffine'
import {imageGeometryAlignment} from '../apps/align/src/geometryInitialization'
import {imageFromRaw} from '../apps/align/src/volumeImages'
import {fitNonlinearSpline} from '../packages/imaging-math/nonlinearSpline'
import {applyAffine,invertAffine} from '../apps/align/src/roiWarp'
import {cloneFitState} from '../apps/align/src/fitSnapshot'
import type {FitState} from '../apps/align/src/appTypes'
import type {Vec3} from '../apps/align/src/rigid'
const centers={mri:[1,2,3] as Vec3,ct:[-2,4,1] as Vec3},points:Vec3[]=[]
for(const x of [-8,0,8])for(const y of [-8,8])for(const z of [-8,8])points.push([x,y,z])
const spline=fitNonlinearSpline(points,points.map(([x,y,z])=>[x+.2*Math.sin(y/8),y+.15*Math.cos(z/8),z+.1*Math.cos(x/8)] as Vec3),.05)
const fixture=JSON.parse(fs.readFileSync('tests/fixtures/jip/mapping-fixtures.json','utf8'))[2].result
for(const direction of ['ct-mri','mri-ct'])for(const kind of ['linear','spline','jip'])for(const target of ['mri','ct'] as const){
 let base:FitState=imageGeometryAlignment(direction)
 if(kind==='spline')base={...base,matrix:spline.matrix,baseMatrix:spline.matrix,inverse:spline.inverse,nonlinear:spline.spline,model:'spline'}
 if(kind==='jip')base={...base,matrix:fixture.matrix,baseMatrix:fixture.matrix,inverse:invertAffine(fixture.matrix),nonlinear:fixture.deformation,model:'jip'}
 let before=base
 for(let step=0;step<3;step++){
  const state=initialManualState(before,centers,target);state.parameters[target]=[step+1,.2,-.3,8,-5,12,.01,-.02,.03,.02,-.01,.02]
  const after=applyManualAffine(before,state),delta=manualPreviewDelta(before,after,target);assert(delta)
  const reference=target==='mri'?'ct':'mri'
  for(const p of [[2,3,4],[-2,1,0]] as Vec3[]){const expected=registrationPoint(after,target,reference,p),actual=applyAffine(delta,registrationPoint(before,target,reference,p));assert(Math.hypot(...expected.map((v,a)=>v-actual[a]))<3e-5)}
  const other=cloneFitState(after);other.manualAffine!.parameters[reference][0]+=1;assert.equal(manualPreviewDelta(before,other,target),null)
  before=after
 }
 const changed=cloneFitState(before);changed.fittedAt='different-fit';assert.equal(manualPreviewDelta(base,changed,target),null)
}
const raw:any={dims:[3,4,5],pixDims:[.7,1,1.2],affine:[[.7,0,0,3],[0,1,0,-2],[0,0,1.2,7],[0,0,0,1]],values:new Float32Array(60).fill(2)},image=imageFromRaw(raw,'test'),geometry=structuredClone(image.hdr!.affine),delta:any=[[1,0,0,1],[0,1,0,2],[0,0,1,3],[0,0,0,1]],preview=transformedPreviewImage(image,delta)
assert.equal(preview.img,image.img);assert.deepEqual(image.hdr!.affine,geometry);assert.deepEqual(raw.affine,geometry);assert.deepEqual(preview.hdr!.affine,[[.7,0,0,4],[0,1,0,0],[0,0,1.2,10],[0,0,0,1]])
console.log('Manual preview: both directions and image targets, all 12 parameters, affine/spline/JIP mapping agreement, repeated changes, unrelated-fit rejection and zero voxel copies passed')
