// CPU preparation benchmark only. This does not measure OpenGL upload/drawing.
import {performance} from 'node:perf_hooks'
import fs from 'node:fs'
import {imageFromRaw} from '../apps/align/src/volumeImages'
import {makeResampledImage} from '../apps/align/src/outputImage'
import {transformedPreviewImage} from '../apps/align/src/manualPreview'
const raw:any={dims:[96,112,120],pixDims:[.5,.7,1],affine:[[.5,0,0,-24],[0,.7,0,-39],[0,0,1,-60],[0,0,0,1]],values:Float32Array.from({length:96*112*120},(_,i)=>1+Math.sin(i*.001)),frameCount:1,datatypeCode:16,slope:1,intercept:0,littleEndian:true}
const item:any={raw,nvImage:imageFromRaw(raw,'synthetic'),name:'synthetic'},delta:any=[[1,0,0,.5],[0,1,0,-.2],[0,0,1,.1],[0,0,0,1]],records=[]
for(let repeat=-1;repeat<5;repeat++){
 let start=performance.now();const old=makeResampledImage(item,item,delta),full=performance.now()-start
 start=performance.now();const next=transformedPreviewImage(item.nvImage,delta),preview=performance.now()-start
 if(next.img!==item.nvImage.img||old.img===item.nvImage.img)throw Error('Invalid benchmark ownership')
 records.push({repeat,warmup:repeat<0,full_cpu_resampling_ms:full,cached_display_geometry_ms:preview})
}
const median=(key:string)=>records.filter(r=>!r.warmup).map(r=>(r as any)[key]).sort((a,b)=>a-b)[2]
const result={status:'passed',voxels:raw.values.length,records,medians:{full_cpu_resampling_ms:median('full_cpu_resampling_ms'),cached_display_geometry_ms:median('cached_display_geometry_ms')},scope:'Synthetic CPU image preparation only. The transient display uses cached pixels and must be followed by full resampling; GPU uploads/drawing and native responsiveness are unmeasured.'}
fs.writeFileSync(process.argv[2]??'manual-preview-benchmark.json',JSON.stringify(result,null,2)+'\n');console.log(JSON.stringify(result.medians))
