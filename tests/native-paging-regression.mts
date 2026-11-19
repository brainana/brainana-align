import assert from 'node:assert/strict'
import fs from 'node:fs'
import {pathToFileURL} from 'node:url'
import {NativeDiskBlob} from './helpers/nativeDiskBlob.mjs'
import fsp from 'node:fs/promises'
import os from 'node:os'
import path from 'node:path'
import {File} from 'node:buffer'
import {Worker} from 'node:worker_threads'
import * as nifti from 'nifti-reader-js'
import {gzipSync,gunzipSync} from 'fflate'
import {inspectVolume,prepareVolume} from '../packages/imaging-math/workingVolume'
import {openNativeVoxelSource} from '../packages/imaging-math/nativeVoxelSource'
import {compressedPagedNativeImage} from '../packages/imaging-math/pagedNativeExport'
import {compressedAlignedImage} from '../packages/imaging-math/nativeImageExport'
import {sampleExport,sampleExportReader,visitExportNeighbors,type ExportInterpolation} from '../packages/imaging-math/exportSampling'
import {fitNonlinearSpline} from '../packages/imaging-math/nonlinearSpline'
import {emptyProcessingRecipe} from '../apps/align/src/imageProcessing'
import {applyNativeProcessing,nativeProcessingValue} from '../apps/align/src/nativeProcessingExport'
import type {RawNifti} from '../packages/imaging-math/roiWarp'
const identity:any=[[1,0,0,0],[0,1,0,0],[0,0,1,0],[0,0,0,1]]
function fixture(datatype:number,gzip:boolean,be:boolean){
 const widths:Record<number,number>={2:1,4:2,8:4,16:4,64:8,256:1,512:2,768:4},width=widths[datatype],h=new nifti.NIFTI1()
 h.dims=[4,17,19,11,2,1,1,1];h.pixDims=[1,.3,.4,.6,1,1,1,1];h.affine=[[0,-.4,.1,5],[.3,0,0,-4],[0,.1,.6,7],[0,0,0,1]];h.sform_code=2;h.xyzt_units=2;h.datatypeCode=datatype;h.numBitsPerVoxel=width*8;h.vox_offset=6624;h.magic='n+1\0';h.littleEndian=!be;h.scl_slope=1.25;h.scl_inter=-4.5
 const count=17*19*11,bytes=new Uint8Array(h.vox_offset+count*2*width);bytes.set(new Uint8Array(h.toArrayBuffer()));const view=new DataView(bytes.buffer)
 for(let i=0;i<count*2;i++){const at=h.vox_offset+i*width,v=i%71+(datatype===256?-35:100*Math.floor(i/count));switch(datatype){case 2:view.setUint8(at,v);break;case 256:view.setInt8(at,v);break;case 4:view.setInt16(at,v,!be);break;case 512:view.setUint16(at,v,!be);break;case 8:view.setInt32(at,v,!be);break;case 768:view.setUint32(at,v,!be);break;case 16:view.setFloat32(at,v+.07,!be);break;case 64:view.setFloat64(at,v+.007,!be)}}
 return new File([gzip?gzipSync(bytes):bytes],gzip?'synthetic.nii.gz':'synthetic.nii')
}
const decode=async(blob:Blob)=>{const b=gunzipSync(new Uint8Array(await blob.arrayBuffer())),h=nifti.readHeader(b.buffer);return {header:h,values:new Float32Array(nifti.readImage(h,b.buffer))}}
let cases=0
for(const type of [2,4,8,16,64,256,512,768])for(const compressed of [false,true])for(const be of [false,true]){
 const file=fixture(type,compressed,be),info=await inspectVolume(file),raw=await prepareVolume(file,info,info.spacing,1),source=await openNativeVoxelSource(file,info,1,{pageVoxels:17,cacheBytes:17*13*64})
 try{
  const points=[[0,0,0],[16,18,10],[8.25,9.125,4.7],[.01,.2,.3],[-2,0,0],[NaN,1,1],[16+5e-7,1,1]]
  for(const method of ['nearest','linear','cubic'] as ExportInterpolation[])for(const p of points){const pages=new Set<number>();visitExportNeighbors(info.dims,p,method,i=>pages.add(source.page(i)));await source.ensure(pages);assert.equal(sampleExportReader(i=>source.read(i),info.dims,p,method),sampleExport(raw.values,raw.dims,p,method))}
  if(type===4||type===16){
   const target={...raw,dims:[19,17,13] as [number,number,number]},matrix:any=[[1.01,.03,0,.12],[0,.98,.01,-.09],[0,0,1.02,.05],[0,0,0,1]]
   for(const method of ['nearest','linear','cubic'] as ExportInterpolation[]){const expected=await decode(await compressedAlignedImage(raw,target,matrix,undefined,()=>{},method)),actual=await decode(await compressedPagedNativeImage(source,target,matrix,undefined,()=>{},method));assert.deepEqual(actual.header.affine,expected.header.affine);assert.deepEqual(actual.values,expected.values,`${type}/${compressed}/${be}/${method}: full exported values must match`)}
  }
  assert(source.stats.peakCacheBytes<=source.stats.cacheBudgetBytes);assert(source.stats.largestReadBytes<=16*17*info.bytesPerVoxel);assert(source.stats.backingBatchBytes<=8*1024**2)
 }finally{source.close()}
 cases++
}
// Nonlinear kernels use the complete inverse map, with the same old writer
// as a numerical reference, including physical obliquity and native processing.
const file=fixture(16,true,false),info=await inspectVolume(file),raw=await prepareVolume(file,info,info.spacing,1),recipe=emptyProcessingRecipe(raw)
recipe.crop={min:[-1,-3,7],max:[6,2,14]};recipe.thresholdFill={min:130,max:190,invert:false,value:-3};recipe.edits=[{index:170,value:777,afterMask:true},{index:200,value:23}]
const expectedRaw={...raw,values:raw.values.slice()};applyNativeProcessing(expectedRaw,raw,recipe)
const processor=nativeProcessingValue(raw,raw,recipe),paged=await openNativeVoxelSource(file,info,1,{process:processor,pageVoxels:29,cacheBytes:29*13*64})
const field=new Float32Array(3*8**3);field.fill(.02,0,8**3)
const nonlinear:any={version:1,kernel:'jip-dense-pull',baseAffine:identity,dims:[8,8,8],affine:[[2,0,0,-4],[0,2,0,-5],[0,0,2,3],[0,0,0,1]],residualValues:field,settings:{axes:'x'},execution:{active:'cpu'},warnings:[]}
const controls:any[]=[];for(const x of [0,3,6])for(const y of [-3,1])for(const z of [8,14])controls.push([x,y,z]);const fitted=fitNonlinearSpline(controls,controls.map(([x,y,z])=>[x+.02*Math.sin(y),y,z]),.05)
for(const warp of [{matrix:identity,nonlinear},{matrix:fitted.matrix,nonlinear:fitted.spline}])for(const method of ['nearest','linear','cubic'] as ExportInterpolation[]){const a=await decode(await compressedPagedNativeImage(paged,raw,warp.matrix,warp.nonlinear,()=>{},method)),b=await decode(await compressedAlignedImage(expectedRaw,raw,warp.matrix,warp.nonlinear,()=>{},method));assert.deepEqual(a.values,b.values)}
assert(raw.values.some((v,i)=>v!==expectedRaw.values[i]));assert.equal(raw.values[170],Math.fround(1.25*Math.fround((170+raw.values.length)%71+100+.07)-4.5));paged.close()
// Zero-weight neighbors containing NaN must preserve exact sample values.
for(const bad of [NaN,Infinity,-Infinity])for(let i=0;i<8;i++)for(const method of ['nearest','linear','cubic'] as ExportInterpolation[]){const values=new Float32Array(8).fill(bad);values[i]=42;const p=[i%2,Math.floor(i/2)%2,Math.floor(i/4)];assert.equal(sampleExportReader(i=>values[i],[2,2,2],p,method),42)}
await assert.rejects(inspectVolume(new File([gzipSync(new Uint8Array(0))],'empty-ct.nii.gz')),/empty-ct.*no complete NIfTI header/)
await assert.rejects(inspectVolume(new File([new Uint8Array(3)],'short.nii')),/no complete NIfTI header/)
const truncated=fixture(4,false,false).slice(0,6800);await assert.rejects(openNativeVoxelSource(truncated,info),/datatype|Truncated/)
const tinyFile=fixture(16,false,false),tinyInfo=await inspectVolume(tinyFile),tiny=await openNativeVoxelSource(tinyFile,tinyInfo,1,{pageVoxels:1,cacheBytes:512});const tinyExpected=await prepareVolume(tinyFile,tinyInfo,tinyInfo.spacing,1);assert.deepEqual((await decode(await compressedPagedNativeImage(tiny,tinyExpected,identity))).values,tinyExpected.values);assert(tiny.stats.pagesRead>128);assert(tiny.stats.peakCacheBytes<=512);tiny.close();
const blobPaged=await openNativeVoxelSource(file,info,1,{maxResidentEncodedBytes:0,pageVoxels:29,cacheBytes:29*13*64});assert.equal(blobPaged.stats.storage,'decompressed-blob');assert.deepEqual((await decode(await compressedPagedNativeImage(blobPaged,raw,identity))).values,raw.values);blobPaged.close();
const abort=new AbortController();await assert.rejects(openNativeVoxelSource(file,info,1,{signal:abort.signal,progress:()=>abort.abort()}),/cancelled/)

// Sparse 4.30 GB source: a real file with real pixels above the signed-32-bit
// AND 4 GiB boundaries, without reserving a 4 GB source array or disk allocation.
const dir=await fsp.mkdtemp(path.join(os.tmpdir(),'align-native-large-'))
try{
 const filename=path.join(dir,'large.nii'),h=new nifti.NIFTI1();h.dims=[3,1024,1024,1026,1,1,1,1];h.pixDims=[1,1,1,1,1,1,1,1];h.sform_code=2;h.affine=identity;h.datatypeCode=16;h.numBitsPerVoxel=32;h.vox_offset=352;h.magic='n+1\0';h.littleEndian=true;h.xyzt_units=2
 const handle=await fsp.open(filename,'w');await handle.truncate(352+1024*1024*1026*4);await handle.write(new Uint8Array(h.toArrayBuffer()),0,352,0)
 for(let z=1021;z<1026;z++)for(let y=1019;y<1024;y++){const row=Buffer.alloc(5*4);for(let x=1019;x<1024;x++)row.writeFloatLE((x-1019)+10*(y-1019)+100*(z-1021),(x-1019)*4);await handle.write(row,0,row.length,352+4*(1019+1024*(y+1024*z)))}await handle.close()
 const blob=new NativeDiskBlob(filename),largeInfo=await inspectVolume(blob as any),target={dims:[3,3,3],pixDims:[1,1,1],affine:identity},matrix=[[1,0,0,-1020.25],[0,1,0,-1020.125],[0,0,1,-1022.25],[0,0,0,1]]
 assert(blob.size>2**32)
 const assets=path.resolve(process.argv[2]??'apps/align/dist','assets'),workerFile=fs.readdirSync(assets).find(n=>/^nativeExportWorker-.*\.js$/.test(n))!,code=fs.readFileSync(path.join(assets,workerFile),'utf8')
 const worker=new Worker(`const {parentPort}=require('node:worker_threads');globalThis.self={postMessage:m=>parentPort.postMessage(m)};parentPort.on('message',async data=>{const {NativeDiskBlob}=await import(${JSON.stringify(pathToFileURL(path.resolve('tests/helpers/nativeDiskBlob.mjs')).href)});data.file=new NativeDiskBlob(data.inputPath);delete data.inputPath;self.onmessage({data})});\n${code}`,{eval:true})
 try{
  const result:any=await new Promise((resolve,reject)=>{worker.on('message',data=>{if(data.type==='error')reject(Error(data.message));if(data.type==='result')resolve(data)});worker.on('error',reject);worker.postMessage({inputPath:filename,info:largeInfo,frame:0,target,matrix,interpolation:'linear'})})
  const out=await decode(result.blob);assert.deepEqual(out.header.dims.slice(1,4),[3,3,3]);for(let z=0;z<3;z++)for(let y=0;y<3;y++)for(let x=0;x<3;x++)assert.equal(out.values[x+3*(y+3*z)],(x+1.25)+10*(y+1.125)+100*(z+1.25))
  assert(result.nativeRead.peakCacheBytes<=128*1024**2);assert(result.nativeRead.largestReadBytes<=8*1024**2);assert.equal(result.nativeRead.storage,'input-file');console.log('Large exact emitted worker:',JSON.stringify({sourceBytes:blob.size,...result.nativeRead}))
 }finally{await worker.terminate()}
}finally{await fsp.rm(dir,{recursive:true,force:true})}
console.log(`Native paging: ${cases} datatype/endian/compression/frame combinations; complete old/new exported arrays for all interpolators; tiny-cache eviction/subdivision; nonlinear native processing; NaN, truncation and abort; actual emitted worker reading above4GiB passed.`)
