import {cloneFitState} from './fitSnapshot'
import type {FitState,Modality} from './appTypes'
import type {Mat4,Vec3} from './rigid'
import {multiplyMat4} from './rigid'
import {invertAffine} from './roiWarp'
import {automaticParameterMatrix} from '../../../packages/imaging-math/automaticLinear'
import {diagnoseTransform,validateAffine} from './landmarkFit'
import {createPointMapping} from '../../../packages/imaging-math/nonlinearSpline'
export type Manual12=[number,number,number,number,number,number,number,number,number,number,number,number]
export type ManualAffineState={version:1;direction:string;selected:Modality;baseMatrix:Mat4;baseModel:FitState['model'];baseParameterCount:number;priorSplineInput?:Mat4;centers:Record<Modality,Vec3>;parameters:Record<Modality,Manual12>}
export const zeroManual=():Manual12=>[0,0,0,0,0,0,0,0,0,0,0,0]
export function registrationModalities(direction:string){return direction==='ct-mri'?{moving:'ct' as const,fixed:'mri' as const}:{moving:'mri' as const,fixed:'ct' as const}}
export function registrationPoint(fit:FitState,from:Modality,to:Modality,point:Vec3):Vec3{if(from===to)return point;const map=createPointMapping(fit.matrix,fit.nonlinear);return from===registrationModalities(fit.direction).moving?map.forward(point):map.inverse(point)}
export function manualDelta(parameters:Manual12,center:Vec3):Mat4{return automaticParameterMatrix(parameters,center,center)}
export function validateManualAffine(state:ManualAffineState){
 if(!state||state.version!==1||!['ct-mri','mri-ct'].includes(state.direction)||!['mri','ct'].includes(state.selected)||!['rigid','similarity','anisotropic','affine','spline','jip'].includes(state.baseModel??'')||(!Number.isInteger(state.baseParameterCount)||state.baseParameterCount<6))throw Error('Invalid manual affine state')
 validateAffine(state.baseMatrix);if(state.priorSplineInput)validateAffine(state.priorSplineInput)
 for(const m of ['mri','ct'] as const){const p=state.parameters?.[m],c=state.centers?.[m];if(!Array.isArray(p)||p.length!==12||!p.every(Number.isFinite)||p.slice(0,3).some(v=>Math.abs(v)>1e5)||p.slice(3,6).some(v=>Math.abs(v)>360000)||p.slice(6,9).some(v=>Math.abs(v)>Math.log(20)+1e-10)||p.slice(9).some(v=>Math.abs(v)>3)||!Array.isArray(c)||c.length!==3||!c.every(Number.isFinite))throw Error('Manual affine parameters are outside supported bounds (scale 5–2000%, shear ±3).');validateAffine(manualDelta(p,c))}
}
export function initialManualState(fit:FitState,centers:Record<Modality,Vec3>,selected:Modality):ManualAffineState{
 if(fit.manualAffine)return structuredClone(fit.manualAffine)
 const parameters={mri:zeroManual(),ct:zeroManual()},moving=registrationModalities(fit.direction).moving;parameters[moving].splice(0,6,...fit.manual)
 return {version:1,direction:fit.direction,selected,baseMatrix:structuredClone(fit.baseMatrix),baseModel:fit.model??'rigid',baseParameterCount:fit.parameterCount??6,priorSplineInput:fit.nonlinear?.inputAffine?structuredClone(fit.nonlinear.inputAffine):undefined,centers:structuredClone(centers),parameters}
}
export function manualComposition(state:ManualAffineState){validateManualAffine(state);const {moving,fixed}=registrationModalities(state.direction),post=manualDelta(state.parameters[moving],state.centers[moving]),pre=invertAffine(manualDelta(state.parameters[fixed],state.centers[fixed]));return {matrix:multiplyMat4(post,multiplyMat4(state.baseMatrix,pre)),pre}}
export function applyManualAffine(fit:FitState,state:ManualAffineState):FitState{
 if(state.direction!==fit.direction)throw Error('Manual affine direction mismatch')
 const {matrix,pre}=manualComposition(state),next=cloneFitState(fit),diagnostics=diagnoseTransform(matrix),{moving}=registrationModalities(fit.direction)
 next.manualAffine=structuredClone(state);next.matrix=matrix;next.inverse=invertAffine(matrix);next.manual=state.parameters[moving].slice(0,6) as FitState['manual'];next.proposal=null
 if(next.nonlinear){next.nonlinear.inputAffine=state.priorSplineInput?multiplyMat4(state.priorSplineInput,pre):pre;next.model=next.nonlinear.kernel==='jip-dense-pull'?'jip':'spline'}else{const base=state.baseModel??'rigid',incompatible=(base==='rigid'&&diagnostics.scales.some(v=>Math.abs(v-1)>1e-4))||(base==='similarity'&&Math.max(...diagnostics.scales)-Math.min(...diagnostics.scales)>1e-4)||(!['affine','spline'].includes(base)&&diagnostics.shear>1e-4);next.model=incompatible?'affine':base;next.parameterCount=next.model==='affine'?12:state.baseParameterCount}
 next.diagnostics={...diagnostics,warnings:[...new Set([...(fit.diagnostics?.warnings??[]),...diagnostics.warnings])]};return next
}
export function resetManualAffine(fit:FitState):FitState{const next=cloneFitState(fit),state=fit.manualAffine;next.matrix=structuredClone(state?.baseMatrix??fit.baseMatrix);next.inverse=invertAffine(next.matrix);next.manual=[0,0,0,0,0,0];if(state){next.model=state.baseModel;next.parameterCount=state.baseParameterCount;if(next.nonlinear){if(state.priorSplineInput)next.nonlinear.inputAffine=structuredClone(state.priorSplineInput);else delete next.nonlinear.inputAffine}}delete next.manualAffine;next.diagnostics=diagnoseTransform(next.matrix);return next}
