import type {FitState,Modality} from './appTypes'
import type {Mat4} from './rigid'
import {multiplyMat4} from './rigid'
import {invertAffine} from './roiWarp'
import {manualDelta,registrationModalities,zeroManual,type Manual12} from './manualAffine'
import type {NVImage} from '@niivue/niivue'
import {sharedViewerImage} from './volumeImages'
const same=(a:unknown,b:unknown)=>JSON.stringify(a)===JSON.stringify(b)
/** A one-sided manual change post-composes the displayed source-to-reference
 * map, also when viewing the inverse nonlinear direction. This only moves the
 * cached display image. A settled rebuild samples ORIGINAL source voxels once. */
export function manualPreviewDelta(before:FitState,after:FitState,target:Modality):Mat4|null{
 const state=after.manualAffine;if(!state||before.direction!==after.direction||before.fittedAt!==after.fittedAt||before.proposal||after.proposal)return null
 const other:Modality=target==='mri'?'ct':'mri'
 if(!same(before.manualAffine?.baseMatrix??before.baseMatrix,state.baseMatrix))return null
 if(before.nonlinear?.kernel!==after.nonlinear?.kernel)return null
 if(before.nonlinear?.kernel==='jip-dense-pull'&&after.nonlinear?.kernel==='jip-dense-pull'){
  if(before.nonlinear.residualValues!==after.nonlinear.residualValues||before.nonlinear.residualBase64!==after.nonlinear.residualBase64||!same(before.nonlinear.baseAffine,after.nonlinear.baseAffine))return null
 }else if(before.nonlinear&&after.nonlinear){
  // Coefficients/control points are small immutable model data, distinct from
  // changing manual inputAffine. Never approximate a different fitted warp.
  const {inputAffine:_a,...a}=before.nonlinear,{inputAffine:_b,...b}=after.nonlinear
  if(!same(a,b))return null
 }
 const prior=(m:Modality):Manual12=>{if(before.manualAffine)return before.manualAffine.parameters[m];const p=zeroManual();if(registrationModalities(before.direction).moving===m)p.splice(0,6,...before.manual);return p}
 if(!same(prior(other),state.parameters[other])||before.manualAffine&&!same(before.manualAffine.centers,state.centers))return null
 return multiplyMat4(manualDelta(state.parameters[target],state.centers[target]),invertAffine(manualDelta(prior(target),state.centers[target])))
}
export function transformedPreviewImage(image:NVImage,delta:Mat4):NVImage{
 const copy=sharedViewerImage(image);copy.setAffine(multiplyMat4(delta,image.hdr!.affine as Mat4));return copy
}
