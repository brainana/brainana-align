import {runWorkerTask} from '../../../packages/core-client/workerTask'
import type {RawNifti} from './roiWarp'
import {invertAffine} from './roiWarp'
import type {FitState} from './appTypes'
export type OverlayRequest={source:RawNifti;target:RawNifti;fit:FitState;reverse:boolean}
/** Pixel ownership stays in the workspace. Do not clone the unused fixed voxels
 * or detach either input; the worker transfers only its newly allocated output. */
export function runOverlayTask(request:OverlayRequest,signal:AbortSignal,progress:(text:string)=>void=()=>{}):Promise<Float32Array>{
 const {source,target,fit,reverse}=request
 return runWorkerTask({
  create:()=>new Worker(new URL('./nonlinearWorker.ts',import.meta.url),{type:'module'}),signal,
  request:{type:fit.nonlinear?'resample':'affine-resample',source,target:{...target,values:new Float32Array(0)},matrix:fit.nonlinear?fit.matrix:reverse?invertAffine(fit.matrix):fit.matrix,spline:fit.nonlinear,reverse},
  cancelled:'Overlay refresh cancelled.',failed:'Overlay worker failed',progress:data=>progress(data.text),
  result:data=>{if(!ArrayBuffer.isView(data.result)||Object.prototype.toString.call(data.result)!=='[object Float32Array]'||data.result.length!==target.dims.reduce((a,b)=>a*b,1))throw Error('Overlay worker returned an invalid image.');return data.result as Float32Array},
 })
}
