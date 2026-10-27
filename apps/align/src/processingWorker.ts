import {processImage} from './imageProcessing'
import {fillMaskHoles} from './maskHoles'
self.onmessage=({data})=>{try{
 if(data.binaryOnly){const binarySelection=fillMaskHoles(data.selection,data.dims,data.mode);for(const e of data.edits??[])binarySelection[e.index]=e.value;(self as any).postMessage({type:'result',binarySelection},[binarySelection.buffer]);return}
 const result=processImage(data.base,data.recipe),transfer=[result.raw.values.buffer,result.excluded.buffer];if(result.binarySelection)transfer.push(result.binarySelection.buffer)
 ;(self as any).postMessage({type:'result',...result},transfer)
}catch(error){self.postMessage({type:'error',message:error instanceof Error?error.message:String(error)})}}
