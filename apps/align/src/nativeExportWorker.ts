import {applyNativeProcessing,nativeProcessingValue} from './nativeProcessingExport'
import {openNativeVoxelSource} from '../../../packages/imaging-math/nativeVoxelSource'
import {compressedPagedNativeImage} from '../../../packages/imaging-math/pagedNativeExport'
import {compressedAlignedImage} from '../../../packages/imaging-math/nativeImageExport'
self.onmessage=async({data})=>{
 const progress=(text:string)=>self.postMessage({type:'progress',text})
 let source:Awaited<ReturnType<typeof openNativeVoxelSource>>|undefined
 try{
  let blob:Blob
  if(data.file){
   const process=data.processing?nativeProcessingValue({dims:data.info.dims,affine:data.info.affine},data.processing.base,data.processing.recipe):undefined
   source=await openNativeVoxelSource(data.file,data.info,data.frame,{progress,process,maxResidentEncodedBytes:data.nativeBufferBudgetBytes,residentUncompressed:data.target.dims.reduce((a:number,b:number)=>a*b,1)>data.info.dims.reduce((a:number,b:number)=>a*b,1)/20})
   blob=await compressedPagedNativeImage(source,data.target,data.matrix,data.spline,progress,data.interpolation??'linear')
  }else{
   if(data.processing)applyNativeProcessing(data.raw,data.processing.base,data.processing.recipe,progress)
   blob=await compressedAlignedImage(data.raw,data.target,data.matrix,data.spline,progress,data.interpolation??'linear')
  }
  self.postMessage({type:'result',blob,nativeRead:source?.stats})
 }catch(error){self.postMessage({type:'error',message:error instanceof Error?error.message:String(error)})}
 finally{source?.close()}
}
