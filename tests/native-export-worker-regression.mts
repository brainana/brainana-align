import assert from 'node:assert/strict'
import fs from 'node:fs'
import path from 'node:path'
import {Worker as NodeWorker} from 'node:worker_threads'
import {File} from 'node:buffer'
import {JSDOM} from 'jsdom'
import * as nifti from 'nifti-reader-js'
import {gunzipSync} from 'fflate'
import {saveNativeAlignedImage} from '../apps/align/src/nativeExport'
import {inspectVolume,prepareVolume} from '../packages/imaging-math/workingVolume'
import {imageGeometryAlignment} from '../apps/align/src/geometryInitialization'
const dom=new JSDOM('<button id="alignment-cancel"></button>',{pretendToBeVisual:true});Object.assign(globalThis,{window:dom.window,document:dom.window.document});const assets=path.resolve(process.argv[2]??'apps/align/dist','assets'),code=fs.readFileSync(path.join(assets,fs.readdirSync(assets).find(f=>/^nativeExportWorker-.*\.js$/.test(f))!),'utf8')
const requests:any[]=[];class ActualWorker{onmessage:any;onerror:any;worker=new NodeWorker(`const {parentPort}=require('node:worker_threads');globalThis.self={postMessage:m=>parentPort.postMessage(m)};parentPort.on('message',data=>self.onmessage({data}));\n${code}`,{eval:true});constructor(){this.worker.on('message',data=>this.onmessage?.({data}));this.worker.on('error',e=>this.onerror?.({message:e.message}))}postMessage(data:any){requests.push(data);this.worker.postMessage(data)}terminate(){void this.worker.terminate()}}
Object.assign(globalThis,{Worker:ActualWorker});let saved:Blob|null=null,writes=0,busy=false,ends=0;const messages:string[]=[];(window as any).brainanaAlignSaveBlob=async(blob:Blob)=>{saved=blob;writes++};(window as any).brainanaPreferences={memory:async()=>({total:16*1024**3,available:8*1024**3,sampledAt:Date.now(),source:'test'})}
const h=new nifti.NIFTI1();h.dims=[3,19,21,23,1,1,1,1];h.pixDims=[1,.2,.3,.4,1,1,1,1];h.sform_code=2;h.affine=[[.2,0,0,-3],[0,.3,0,-4],[0,0,.4,-5],[0,0,0,1]];h.datatypeCode=16;h.numBitsPerVoxel=32;h.vox_offset=352;h.magic='n+1\0';h.xyzt_units=2;h.littleEndian=true;const vals=Float32Array.from({length:19*21*23},(_,i)=>i%17),bytes=new Uint8Array(352+vals.byteLength);bytes.set(new Uint8Array(h.toArrayBuffer()));bytes.set(new Uint8Array(vals.buffer),352);const file=new File([bytes],'native.nii'),info=await inspectVolume(file as any),raw=await prepareVolume(file as any,info,[1,1,1]);raw.values.fill(999)
const loaded:any={name:'native.nii',raw,nvImage:null,sourceFiles:['native.nii'],native:{files:[file],info,requested:[1,1,1],frame:0,streamable:true}},activity={begin:async()=>{busy=true;return true},phase:async(text:string)=>{messages.push(text)},end:()=>{busy=false;ends++}},fit=imageGeometryAlignment('ct-mri')
assert.equal(await saveNativeAlignedImage(loaded,loaded,fit,'ct','mri',activity,()=>{}),true);assert.equal(writes,1);assert(saved);assert.equal(requests[0].raw,undefined,'Native file is reread rather than sending the filtered workspace');assert.equal(requests[0].target.values,undefined,'Fixed-image voxels are never loaded for export')
const decoded=gunzipSync(new Uint8Array(await (saved as Blob).arrayBuffer())),head=nifti.readHeader(decoded.buffer),out=new Float32Array(nifti.readImage(head,decoded.buffer));assert.deepEqual(head.dims.slice(1,4),info.dims);out.forEach((v,i)=>assert(Math.abs(v-vals[i])<1e-4));assert(messages.some(m=>m.includes('output grid')));assert.equal(busy,false)
const job=saveNativeAlignedImage(loaded,loaded,fit,'ct','mri',activity,()=>{});for(let i=0;i<100&&!busy;i++)await new Promise(r=>setTimeout(r,2));document.querySelector<HTMLButtonElement>('#alignment-cancel')!.click();await assert.rejects(job,/cancelled/);assert.equal(writes,1);assert.equal(busy,false);assert.equal(ends,2)
// Nonstreamable sources also retain originals after processing, while explicit processed mode uses the working grid.
const original={...raw,values:new Float32Array(raw.values.length).fill(7)},processed={...loaded,native:undefined,raw:{...raw,values:new Float32Array(raw.values.length).fill(99)},processing:{base:original}};
await saveNativeAlignedImage(processed,processed,fit,'ct','mri',activity,()=>{});assert.equal(requests.at(-1).file,undefined);assert.equal(requests.at(-1).raw.values[0],7,'Default export cannot silently substitute processed data for a nonstreamable original');
await saveNativeAlignedImage(processed,processed,fit,'ct','mri',activity,()=>{},'processed');assert.equal(requests.at(-1).raw.values[0],99);assert.deepEqual(requests.at(-1).target.dims,raw.dims);
// The sampled JIP pull must survive native export, rather than its affine alone.
const fieldValues=new Float32Array(3*8**3);fieldValues.fill(.2,0,8**3)
const jipFit:any={...imageGeometryAlignment('ct-mri'),model:'jip',parameterCount:3*8**3,nonlinear:{version:1,kernel:'jip-dense-pull',baseAffine:fit.matrix,dims:[8,8,8],affine:[[3,0,0,-6],[0,3,0,-6],[0,0,3,-6],[0,0,0,1]],residualBase64:Buffer.from(fieldValues.buffer).toString('base64'),settings:{axes:'x'},execution:{active:'cpu'},warnings:[]}}
await saveNativeAlignedImage(loaded,loaded,jipFit,'ct','mri',activity,()=>{})
const jipBytes=gunzipSync(new Uint8Array(await (saved as Blob).arrayBuffer())),jipHead=nifti.readHeader(jipBytes.buffer),jipOutput=new Float32Array(nifti.readImage(jipHead,jipBytes.buffer))
for(let z=0;z<23;z++)for(let y=0;y<21;y++)for(let x=0;x<18;x++){const i=x+19*(y+21*z);assert(Math.abs(jipOutput[i]-vals[i+1])<1e-4,'Native JIP pull must shift one original voxel in physical X')}
dom.window.close();console.log('Exact native export worker/controller: native input fidelity despite altered working voxels, native reference grid, no fixed payload, progress, cancellation and no partial destination save passed.')
