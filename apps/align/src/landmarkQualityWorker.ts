import {landmarkLeaveOneOutSensitivity,type LandmarkSensitivityInput} from '../../../packages/imaging-math/landmarkSensitivity'

self.onmessage=async({data}:{data:LandmarkSensitivityInput})=>{
 try{
  const report=await landmarkLeaveOneOutSensitivity(data,{onProgress:(completed,total)=>self.postMessage({type:'progress',completed,total})})
  self.postMessage({type:'result',report})
 }catch(error){self.postMessage({type:'error',message:error instanceof Error?error.message:String(error)})}
}
