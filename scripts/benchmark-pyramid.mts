/** Deterministic CPU microbenchmark, not a claim about native GUI responsiveness. */
import {performance} from 'node:perf_hooks'
import {createHash} from 'node:crypto'
import {pathToFileURL} from 'node:url'
import * as current from '../packages/imaging-math/imagePyramid'
const baseline=process.argv[2]?await import(pathToFileURL(process.argv[2]).href):null
const source:any={dims:[144,128,112],pixDims:[.4,.6,.8],affine:[[.4,0,0,-30],[0,.6,0,20],[0,0,.8,-10],[0,0,0,1]],values:Float32Array.from({length:144*128*112},(_,i)=>Math.sin(i*.01)+Math.cos(i*.0001)),frameCount:1,slope:1,intercept:0,datatypeCode:16,littleEndian:true}
const records=[]
for(const factor of [2,4,8]){
 let before:any=null
 for(const [name,module] of baseline?[['baseline',baseline],['current',current]] as const:[['current',current]] as const){
  for(let i=0;i<2;i++)module.downsampleByFactor(source,factor)
  const timings=[];let output:any
  for(let i=0;i<7;i++){const start=performance.now();output=module.downsampleByFactor(source,factor);timings.push(performance.now()-start)}
  timings.sort((a,b)=>a-b)
  const record={implementation:name,factor,inputVoxels:source.values.length,outputDims:output.dims,medianMs:Number(timings[3].toFixed(3)),minMs:Number(timings[0].toFixed(3)),outputSHA256:createHash('sha256').update(new Uint8Array(output.values.buffer,output.values.byteOffset,output.values.byteLength)).digest('hex')}
  if(name==='baseline')before=record
  records.push({...record,...(before&&name==='current'?{speedup:before.medianMs/record.medianMs,bitIdentical:before.outputSHA256===record.outputSHA256}:{})})
 }
}
console.log(JSON.stringify({node:process.version,platform:process.platform,arch:process.arch,method:'2 warmups, 7 timed runs, median; synthetic finite Float32 volume; CPU only',records},null,2))
