import { runIntensityRegistration, type IntensityRequest } from '../../../packages/imaging-math/intensityRegistration'
self.onmessage=async(event:MessageEvent<IntensityRequest>)=>{
  try{const result=await runIntensityRegistration(event.data,text=>self.postMessage({type:'progress',text}));self.postMessage({type:'result',result})}
  catch(error){self.postMessage({type:'error',message:error instanceof Error?error.message:String(error)})}
}
