import {manualComposition} from './manualAffine'
import {multiplyMat4,rigidDelta,type Vec3} from './rigid'
import type {FitState} from './appTypes'
import {diagnoseTransform,LANDMARK_MODELS} from './landmarkFit'
import {invertAffine} from './roiWarp'
/** Reset matrix AND model metadata; manual scaling may have promoted rigid to affine. */
export function resetInitialAlignment(fit:FitState):FitState{
 const next=structuredClone(fit)
 next.matrix=structuredClone(fit.landmarkMatrix);next.baseMatrix=structuredClone(next.matrix)
 next.inverse=invertAffine(next.matrix);next.manual=[0,0,0,0,0,0];delete next.manualAffine
 if(next.nonlinear)delete next.nonlinear.inputAffine
 next.model=next.nonlinear?(next.nonlinear.kernel==='jip-dense-pull'?'jip':'spline'):fit.landmarkModel??fit.manualAffine?.baseModel??fit.model??'rigid'
 next.parameterCount=next.nonlinear?(next.nonlinear.kernel==='jip-dense-pull'?3*next.nonlinear.dims.reduce((a,b)=>a*b,1):next.nonlinear.controls.length*3):LANDMARK_MODELS[next.model==='jip'||next.model==='spline'?'affine':next.model].parameters
 next.diagnostics=diagnoseTransform(next.matrix);next.reflectionConfirmed=Boolean(next.diagnostics.reflection&&fit.reflectionConfirmed)
 if(next.automaticRegistration?.operation==='local')delete next.automaticRegistration
 return next
}

/** Old schema proposals have no previous FitState. Reconstruct their accepted pose. */
export function rejectLegacyRefinement(fit:FitState,center:Vec3):FitState{
 const next=structuredClone(fit)
 next.matrix=fit.manualAffine?manualComposition(fit.manualAffine).matrix:multiplyMat4(rigidDelta(fit.manual,center),fit.baseMatrix)
 next.inverse=invertAffine(next.matrix);next.proposal=null;next.diagnostics=diagnoseTransform(next.matrix)
 return next
}
