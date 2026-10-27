import {fitNonlinearSpline,resampleSpline} from '../../../packages/imaging-math/nonlinearSpline'
import {splineDisplacementField} from '../../../packages/imaging-math/splineFields'
import {resampleRegistration} from '../../../packages/imaging-math/registrationResampling'
const progress=(text:string)=>self.postMessage({type:'progress',text})
self.onmessage=async({data})=>{try{let result:any
 if(data.type==='fit'){progress('Solving regularized 3D landmark spline…');result=fitNonlinearSpline(data.source,data.target,data.smoothing)}
 else if(data.type==='affine-resample'){progress('Preparing transformed overlay…');result=resampleRegistration(data.source,data.target,data.matrix)}
 else if(data.type==='resample')result=await resampleSpline(data.source,data.target,data.matrix,data.spline,progress,Boolean(data.reverse))
 else if(data.type==='field')result=await splineDisplacementField(data.reference,data.matrix,data.spline,data.direction,progress)
 else throw Error('Unknown nonlinear task')
 const transfers:Transferable[]=ArrayBuffer.isView(result)?[result.buffer]:[];(self as unknown as {postMessage:(message:unknown,transfer:Transferable[])=>void}).postMessage({type:'result',result},transfers)
 }catch(error){self.postMessage({type:'error',message:error instanceof Error?error.message:String(error)})}}
