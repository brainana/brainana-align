import type {RawNifti,Dims3} from './roiWarp'
import type {VolumeInfo} from '../../../packages/imaging-math/workingVolume'
import {runWorkerTask} from '../../../packages/core-client/workerTask'
export function prepareInWorker(file:File,info:VolumeInfo,spacing:Dims3,frame:number,progress:(n:number)=>void,signal:AbortSignal):Promise<RawNifti>{
 return runWorkerTask({
  create:()=>new Worker(new URL('./volumeWorker.ts',import.meta.url),{type:'module'}),
  request:{file,info,spacing,frame},signal,
  cancelled:'Preparation cancelled. Existing workspace preserved.',failed:'Volume preparation failed',
  progress:data=>progress(data.fraction),result:data=>data.raw,
 })
}
