import {createJipMapping,validateJip,type JipDeformation} from './jipDeformation'
import {fitLandmarks,leastSquares,validateAffine} from './landmarkFit'
import {applyMat4,multiplyMat4,type Mat4,type Vec3} from './rigid'
import {determinant3,invertAffine,applyAffine,frame,sampleLinear,type RawNifti} from './roiWarp'

// 3D polyharmonic/TPS kernel is r, not the 2D r² log r. We use -r so a
// positive ridge is added to the conditionally positive definite system.
// F(x)=D[G(Ax)], G(q)=q+u(q). A proper affine base and ||Du||<1 guarantee
// a globally one-to-one residual map. Never export A or D*A as the full warp.
export const SPLINE_MIN_PAIRS=12,SPLINE_MAX_PAIRS=128,SPLINE_LIMIT=.65
export type LandmarkSpline={version:1;kernel:'3d-polyharmonic-r';baseAffine:Mat4;inputAffine?:Mat4;center:Vec3;scale:number;controls:Vec3[];weights:Vec3[];polynomial:Vec3[];smoothing:number;lipschitzBound:number;limitingFactor:number;warnings:string[]}
export type NonlinearSpline=LandmarkSpline|JipDeformation
export const isJip=(s:NonlinearSpline):s is JipDeformation=>s.kernel==='jip-dense-pull'
const norm=(p:number[])=>Math.hypot(...p)
const distance=(a:Vec3,b:Vec3)=>Math.hypot(a[0]-b[0],a[1]-b[1],a[2]-b[2])
export function splineBound(s:LandmarkSpline){return (Math.sqrt(s.polynomial.slice(1).flat().reduce((sum,v)=>sum+v*v,0))+s.weights.reduce((sum,w)=>sum+norm(w),0))/s.scale}
export function validateSpline(value:unknown):asserts value is NonlinearSpline{
 if((value as JipDeformation)?.kernel==='jip-dense-pull'){validateJip(value as JipDeformation);return}
 const s=value as LandmarkSpline;if(!s||s.version!==1||s.kernel!=='3d-polyharmonic-r')throw Error('Unsupported nonlinear spline')
 if(s.inputAffine){validateAffine(s.inputAffine);if(determinant3(s.inputAffine)<=0)throw Error('Nonlinear input affine must preserve handedness')}
 validateAffine(s.baseAffine);if(determinant3(s.baseAffine)<=0)throw Error('Nonlinear spline requires a proper, nonreflected affine base')
 const vec=(p:unknown)=>Array.isArray(p)&&p.length===3&&p.every(v=>typeof v==='number'&&Number.isFinite(v))
 if(!vec(s.center)||!Number.isFinite(s.scale)||s.scale<=1e-6||s.scale>1e6||!Array.isArray(s.controls)||s.controls.length<SPLINE_MIN_PAIRS||s.controls.length>SPLINE_MAX_PAIRS||!s.controls.every(vec)||!Array.isArray(s.weights)||s.weights.length!==s.controls.length||!s.weights.every(vec)||!Array.isArray(s.polynomial)||s.polynomial.length!==4||!s.polynomial.every(vec)||!Number.isFinite(s.smoothing)||s.smoothing<0||s.smoothing>10||!Number.isFinite(s.limitingFactor)||s.limitingFactor<=0||s.limitingFactor>1)throw Error('Invalid nonlinear spline coefficients')
 if(s.controls.some(c=>norm(c)>Math.sqrt(s.controls.length)*1.01)||[...s.weights,...s.polynomial].some(p=>norm(p)>1e7))throw Error('Spline coefficients are implausibly large')
 const bound=splineBound(s);if(!Number.isFinite(bound)||bound>SPLINE_LIMIT+1e-9)throw Error('Nonlinear deformation exceeds the invertibility safety bound')
 s.lipschitzBound=bound;s.warnings=Array.isArray(s.warnings)?s.warnings.filter(w=>typeof w==='string'):[]
}
export function splineDisplacement(s:LandmarkSpline,q:Vec3):Vec3{
 const x=(q[0]-s.center[0])/s.scale,y=(q[1]-s.center[1])/s.scale,z=(q[2]-s.center[2])/s.scale,p=s.polynomial
 let u=p[0][0]+x*p[1][0]+y*p[2][0]+z*p[3][0],v=p[0][1]+x*p[1][1]+y*p[2][1]+z*p[3][1],w=p[0][2]+x*p[1][2]+y*p[2][2]+z*p[3][2]
 for(let i=0;i<s.controls.length;i++){const c=s.controls[i],r=-Math.hypot(x-c[0],y-c[1],z-c[2]),a=s.weights[i];u+=r*a[0];v+=r*a[1];w+=r*a[2]}
 return [u,v,w]
}
export function inverseSplineResidual(s:LandmarkSpline,y:Vec3):Vec3{
 let q=y
 for(let i=0;i<128;i++){const u=splineDisplacement(s,q),next:Vec3=[y[0]-u[0],y[1]-u[1],y[2]-u[2]];if(distance(next,q)<=1e-7){return next}q=next}
 throw Error('Nonlinear inverse failed to converge; result was not used')
}
export function createPointMapping(matrix:Mat4,spline?:NonlinearSpline){
 if(spline&&isJip(spline))return createJipMapping(matrix,spline)
 const inverse=invertAffine(matrix)
 if(!spline)return {forward:(x:Vec3)=>applyMat4(matrix,x),inverse:(y:Vec3)=>applyMat4(inverse,y)}
 const a=spline.inputAffine?multiplyMat4(spline.baseAffine,spline.inputAffine):spline.baseAffine,ai=invertAffine(a),d=multiplyMat4(matrix,ai),di=multiplyMat4(a,inverse)
 return {forward:(x:Vec3)=>{const q=applyMat4(a,x),u=splineDisplacement(spline,q);return applyMat4(d,[q[0]+u[0],q[1]+u[1],q[2]+u[2]])},inverse:(y:Vec3)=>applyMat4(ai,inverseSplineResidual(spline,applyMat4(di,y)))}
}
export function fitNonlinearSpline(source:Vec3[],target:Vec3[],smoothing=.01){
 const n=source.length;if(n<SPLINE_MIN_PAIRS||n>SPLINE_MAX_PAIRS||target.length!==n)throw Error(`Nonlinear spline needs ${SPLINE_MIN_PAIRS}–${SPLINE_MAX_PAIRS} complete, distributed landmark pairs`)
 if(!Number.isFinite(smoothing)||smoothing<0||smoothing>10)throw Error('Spline smoothing must be between 0 and 10')
 const base=fitLandmarks(source,target,'affine');if(base.diagnostics.reflection)throw Error('The affine base mirrors anatomy. Verify left–right conventions or use the explicit affine workflow; nonlinear fitting cannot silently reflect anatomy.')
 const q=source.map(x=>applyMat4(base.matrix,x)),center=[0,1,2].map(a=>q.reduce((sum,p)=>sum+p[a],0)/n) as Vec3,scale=Math.sqrt(q.reduce((sum,p)=>sum+distance(p,center)**2,0)/n)
 if(!(scale>1e-6))throw Error('Nonlinear landmarks are coincident')
 for(let i=0;i<n;i++)for(let j=0;j<i;j++)if(distance(q[i],q[j])<scale*.001||distance(target[i],target[j])<scale*.001)throw Error('Nonlinear landmarks are coincident or too closely clustered; spread distinct pairs through the volume')
 const controls=q.map(p=>p.map((v,a)=>(v-center[a])/scale) as Vec3),p=controls.map(c=>[1,...c]),design=controls.map((c,i)=>[...controls.map((d,j)=>-distance(c,d)+(i===j?smoothing:0)),...p[i]])
 for(let a=0;a<4;a++)design.push([...p.map(row=>row[a]),0,0,0,0])
 const coefficients=[0,1,2].map(a=>leastSquares(design,[...q.map((point,i)=>target[i][a]-point[a]),0,0,0,0])),condition=Math.max(...coefficients.map(c=>c.condition))
 if(condition>1e9)throw Error('Spline system is numerically unstable; improve landmark distribution or increase smoothing')
 const vectors=Array.from({length:n+4},(_,i)=>coefficients.map(c=>c.solution[i]) as Vec3)
 const spline:LandmarkSpline={version:1,kernel:'3d-polyharmonic-r',baseAffine:base.matrix,center,scale,controls,weights:vectors.slice(0,n),polynomial:vectors.slice(n),smoothing,lipschitzBound:0,limitingFactor:1,warnings:[...base.diagnostics.warnings]}
 const rawBound=splineBound(spline);if(rawBound>SPLINE_LIMIT){spline.limitingFactor=SPLINE_LIMIT/rawBound;for(const row of [...spline.weights,...spline.polynomial])for(let a=0;a<3;a++)row[a]*=spline.limitingFactor;spline.warnings.push(`Deformation reduced to ${(100*spline.limitingFactor).toFixed(1)}% to prevent folding. Residual errors remain; verify landmarks rather than forcing an exact fit.`)}
 spline.lipschitzBound=splineBound(spline)
 spline.warnings.push('Landmarks constrain only sampled anatomy. Sparse coverage and extrapolation can distort unsampled regions; inspect all slices and use independent validation points.')
 if(n<20)spline.warnings.push('Fewer than 20 pairs: use more well-distributed landmarks for nonlinear work. The 12-pair minimum is an application safeguard, not a guarantee of accuracy.')
 if(smoothing===0)spline.warnings.push('Exact interpolation requested: mistaken points can drive local distortion. Smoothing is recommended.')
 if(condition>1e5)spline.warnings.push('Spline fit is poorly conditioned; increase smoothing or redistribute landmarks.')
 validateSpline(spline)
 const mapping=createPointMapping(base.matrix,spline),residuals=source.map((x,i)=>distance(mapping.forward(x),target[i])),rms=Math.sqrt(residuals.reduce((sum,v)=>sum+v*v,0)/n)
 return {matrix:base.matrix,inverse:base.inverse,spline,rms,residuals,condition,affineRms:base.rms,parameterCount:3*n,diagnostics:{...base.diagnostics,warnings:[...base.diagnostics.warnings,...spline.warnings]}}
}
export function splineJacobian(s:LandmarkSpline,q:Vec3){
 const h=Math.max(1e-4,s.scale*1e-5),j=[[1,0,0],[0,1,0],[0,0,1]]
 for(let a=0;a<3;a++){const lo=[...q] as Vec3,hi=[...q] as Vec3;lo[a]-=h;hi[a]+=h;const u=splineDisplacement(s,lo),v=splineDisplacement(s,hi);for(let b=0;b<3;b++)j[b][a]+=(v[b]-u[b])/(2*h)}return determinant3(j)
}
export function splineDiagnostics(s:LandmarkSpline,matrix:Mat4){
 const d=determinant3(matrix),samples=s.controls.map(c=>c.map((v,a)=>v*s.scale+s.center[a]) as Vec3)
 for(let x=-2;x<=2;x++)for(let y=-2;y<=2;y++)for(let z=-2;z<=2;z++)samples.push([s.center[0]+x*s.scale,s.center[1]+y*s.scale,s.center[2]+z*s.scale])
 const jac=samples.map(q=>d*splineJacobian(s,q));return {minimumSampledJacobian:Math.min(...jac),maximumSampledJacobian:Math.max(...jac),globalJacobianLowerBound:d*(1-splineBound(s))**3,lipschitzBound:splineBound(s),maxLandmarkDisplacementMm:Math.max(...samples.slice(0,s.controls.length).map(q=>norm(splineDisplacement(s,q))))}
}
export async function resampleSpline(source:RawNifti,target:RawNifti,matrix:Mat4,spline:NonlinearSpline|undefined,progress:(text:string)=>void=()=>{},reverse=false){
 if(spline)validateSpline(spline);const mapping=createPointMapping(matrix,spline),inverse=reverse?mapping.forward:mapping.inverse,invSource=invertAffine(source.affine),values=frame(source,0),out=new Float32Array(target.dims.reduce((a,b)=>a*b,1));let i=0
 for(let z=0;z<target.dims[2];z++){if(z%4===0){progress(`Nonlinear resampling: slice ${z+1}/${target.dims[2]}…`);await new Promise(r=>setTimeout(r,0))}for(let y=0;y<target.dims[1];y++)for(let x=0;x<target.dims[0];x++){const p=inverse(applyAffine(target.affine,[x,y,z])),v=applyAffine(invSource,p);out[i++]=sampleLinear(values,source.dims,v)}}return out
}
