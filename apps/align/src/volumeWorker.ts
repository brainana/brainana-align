import {prepareVolume} from '../../../packages/imaging-math/workingVolume'
import {restoreWorkerFile} from '../../../packages/core-client/rangeFile'
self.onmessage=async({data})=>{try{let last=-1;const raw=await prepareVolume(restoreWorkerFile(data.file),data.info,data.spacing,data.frame,n=>{const pct=Math.floor(n*100);if(pct!==last){last=pct;self.postMessage({type:'progress',fraction:n})}},data.exactDims);(self as any).postMessage({type:'result',raw},[raw.values.buffer])}catch(error){self.postMessage({type:'error',message:error instanceof Error?error.message:String(error)})}}
