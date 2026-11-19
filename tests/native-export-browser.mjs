import assert from 'node:assert/strict'
import fs from 'node:fs'
import fsp from 'node:fs/promises'
import path from 'node:path'
import os from 'node:os'
import http from 'node:http'
import {createRequire} from 'node:module'
import * as nifti from 'nifti-reader-js'
import {gunzipSync} from 'fflate'
const require=createRequire(import.meta.url),{chromium}=require('playwright-core')
const dist=path.resolve(process.argv[2]??'apps/align/dist'),assets=path.join(dist,'assets'),workerName=fs.readdirSync(assets).find(n=>/^nativeExportWorker-.*\.js$/.test(n))
assert(workerName)
const executablePath=process.env.CHROMIUM_PATH||(process.platform==='darwin'?'/Applications/Google Chrome.app/Contents/MacOS/Google Chrome':chromium.executablePath())
const dir=await fsp.mkdtemp(path.join(os.tmpdir(),'align-native-browser-'));let browser,server
try{
 const identity=[[1,0,0,0],[0,1,0,0],[0,0,1,0],[0,0,0,1]],filename=path.join(dir,'large.nii'),h=new nifti.NIFTI1();h.dims=[3,1024,1024,1026,1,1,1,1];h.pixDims=[1,1,1,1,1,1,1,1];h.sform_code=2;h.affine=identity;h.datatypeCode=16;h.numBitsPerVoxel=32;h.vox_offset=352;h.magic='n+1\0';h.littleEndian=true;h.xyzt_units=2
 const size=352+1024*1024*1026*4,handle=await fsp.open(filename,'w');await handle.truncate(size);await handle.write(new Uint8Array(h.toArrayBuffer()),0,352,0)
 for(let z=1021;z<1026;z++)for(let y=1019;y<1024;y++){const row=Buffer.alloc(20);for(let x=1019;x<1024;x++)row.writeFloatLE((x-1019)+10*(y-1019)+100*(z-1021),(x-1019)*4);await handle.write(row,0,20,352+4*(1019+1024*(y+1024*z)))}await handle.close()
 server=http.createServer((req,res)=>{if(req.url==='/'){res.setHeader('Content-Type','text/html');res.end('<input id="native" type="file">')}else if(req.url===`/assets/${workerName}`){res.setHeader('Content-Type','text/javascript');fs.createReadStream(path.join(assets,workerName)).pipe(res)}else{res.statusCode=404;res.end()}})
 await new Promise((resolve,reject)=>{server.once('error',reject);server.listen(0,'127.0.0.1',resolve)})
 browser=await chromium.launch({executablePath,headless:true,args:['--no-sandbox','--disable-gpu','--disable-dev-shm-usage']})
 const page=await browser.newPage();await page.goto(`http://127.0.0.1:${server.address().port}`);await page.setInputFiles('#native',filename)
 const result=await page.evaluate(async({workerName,identity,size})=>{
  const file=document.querySelector('#native').files[0];if(file.size!==size)throw Error(`Native browser File size mismatch: ${file.size}`)
  return await new Promise((resolve,reject)=>{const worker=new Worker(`/assets/${workerName}`);worker.onerror=e=>{worker.terminate();reject(Error(e.message))};worker.onmessage=async({data})=>{if(data.type==='error'){worker.terminate();reject(Error(data.message))}if(data.type==='result'){const bytes=Array.from(new Uint8Array(await data.blob.arrayBuffer()));worker.terminate();resolve({bytes,nativeRead:data.nativeRead,fileBytes:file.size})}};worker.postMessage({file,info:{name:'large.nii',dims:[1024,1024,1026],spacing:[1,1,1],affine:identity,frames:1,offset:352,bytesPerVoxel:4,datatype:16,littleEndian:true,slope:1,intercept:0},frame:0,target:{dims:[3,3,3],pixDims:[1,1,1],affine:identity},matrix:[[1,0,0,-1020.25],[0,1,0,-1020.125],[0,0,1,-1022.25],[0,0,0,1]],interpolation:'linear'})})
 },{workerName,identity,size})
 const bytes=gunzipSync(Uint8Array.from(result.bytes)),header=nifti.readHeader(bytes.buffer),values=new Float32Array(nifti.readImage(header,bytes.buffer));assert.deepEqual(header.dims.slice(1,4),[3,3,3])
 for(let z=0;z<3;z++)for(let y=0;y<3;y++)for(let x=0;x<3;x++)assert.equal(values[x+3*(y+3*z)],(x+1.25)+10*(y+1.125)+100*(z+1.25))
 assert(result.nativeRead.peakCacheBytes<=128*1024**2);assert(result.fileBytes>2**32)
 const report={status:'passed',browser:await browser.version(),nativeBrowserFileBytes:result.fileBytes,exactEmittedWorker:workerName,nativeRead:result.nativeRead,all27NativeInterpolationValuesExact:true,gpu:'Not required for CPU/file export test'}
 const reportPath=path.resolve(process.env.BRAINANA_NATIVE_EXPORT_REPORT??'artifacts/validation/native-export-browser.json');await fsp.mkdir(path.dirname(reportPath),{recursive:true});await fsp.writeFile(reportPath,JSON.stringify(report,null,2)+'\n');console.log(JSON.stringify(report,null,2))
}finally{if(browser)await browser.close();if(server)await new Promise(resolve=>server.close(resolve));await fsp.rm(dir,{recursive:true,force:true})}
