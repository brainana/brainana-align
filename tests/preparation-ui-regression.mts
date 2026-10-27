import assert from 'node:assert/strict'
import fs from 'node:fs'
import path from 'node:path'
import {Worker as NodeWorker} from 'node:worker_threads'
import {File} from 'node:buffer'
import {JSDOM} from 'jsdom'
import * as nifti from 'nifti-reader-js'
import {installImagePreparation} from '../apps/align/src/imagePreparation'
import {alignmentLayout} from '../apps/align/src/alignmentLayout'
import {installGuidedSetup} from '../apps/align/src/guidedSetup'
const dom=new JSDOM('<div id="app"></div>',{pretendToBeVisual:true});Object.assign(globalThis,{window:dom.window,document:dom.window.document});document.querySelector('#app')!.innerHTML=alignmentLayout('test')
const assets=path.resolve(process.argv[2]??'apps/align/dist','assets'),code=fs.readFileSync(path.join(assets,fs.readdirSync(assets).find(f=>/^volumeWorker-.*\.js$/.test(f))!),'utf8')
let starts=0,cancels=0,fail=false
class ActualWorker{onmessage:any;onerror:any;worker:NodeWorker;constructor(){starts++;this.worker=new NodeWorker(`const {parentPort}=require('node:worker_threads');globalThis.self={postMessage:(m)=>parentPort.postMessage(m)};parentPort.on('message',data=>self.onmessage({data}));\n${code}`,{eval:true});this.worker.on('message',data=>this.onmessage?.({data}));this.worker.on('error',error=>this.onerror?.({message:error.message}))}postMessage(data:any){if(fail){queueMicrotask(()=>this.onmessage?.({data:{type:'error',message:'Test decoding failure'}}));return}this.worker.postMessage(data)}terminate(){cancels++;void this.worker.terminate()}}
Object.assign(globalThis,{Worker:ActualWorker})
;(window as any).brainanaPreferences={memory:async()=>({total:16*1024**3,available:8*1024**3,sampledAt:Date.now(),source:'test'})}
function volume(name:string,frames=1){const h=new nifti.NIFTI1();h.dims=[frames>1?4:3,25,27,29,frames,1,1,1];h.pixDims=[1,.2,.3,.4,1,1,1,1];h.affine=[[.2,0,0,-2],[0,.3,0,-3],[0,0,.4,-4],[0,0,0,1]];h.sform_code=2;h.datatypeCode=16;h.numBitsPerVoxel=32;h.vox_offset=352;h.magic='n+1\0';h.littleEndian=true;h.xyzt_units=2;const v=new Float32Array(25*27*29*frames).fill(7),b=new Uint8Array(352+v.byteLength);b.set(new Uint8Array(h.toArrayBuffer()));b.set(new Uint8Array(v.buffer),352);return new File([b],name)}
let loaded:any={mri:null,ct:null},busy=false,commits=0,lastChanged:any;const statuses:string[]=[]
const prep=installImagePreparation({getLoaded:()=>loaded,busy:()=>busy,begin:async()=>{if(busy)return false;busy=true;return true},phase:async()=>{},end:()=>{busy=false},commit:async(items,changed)=>{loaded=items;lastChanged=changed;commits++},status:text=>statuses.push(text)})
const guide=installGuidedSetup(()=>({mri:Boolean(loaded.mri),ct:Boolean(loaded.ct),preparation:prep.state(),landmarks:[],selectedId:null,minimumPairs:3,fit:null,busy,error:null,viewerReady:true}));document.querySelector<HTMLButtonElement>('#guided-setup')!.click()
const mri=volume('mri.nii',2),ct=volume('ct.nii');await Promise.all([prep.select('mri',[mri as any]),prep.select('ct',[ct as any])]);assert.equal(starts,0,'Header selection must not start voxel preparation');assert.equal(commits,0);assert.equal(loaded.mri,null);guide.refresh();assert.equal(document.querySelector('#guided-title')!.textContent,'Load your MRI','Guide projection is suspended while Resolution owns the screen');document.querySelector<HTMLButtonElement>('#preparation-close')!.click();guide.refresh();assert.equal(document.querySelector('#guided-title')!.textContent,'Review resolution');document.querySelector<HTMLButtonElement>('#working-resolution')!.click();assert(document.querySelector('#preparation-capacity')!.textContent!.includes('Installed RAM'))
// Toolbar ownership and real preset clicks, including the single/multi-volume UI.
const style=document.createElement('style');style.textContent=fs.readFileSync('apps/align/src/style.css','utf8');document.head.append(style);assert.equal(window.getComputedStyle(document.querySelector('.topbar')!).flexWrap,'nowrap');assert.equal(window.getComputedStyle(document.querySelector('.preparation-spacing')!).gridTemplateColumns,'repeat(3,minmax(0,1fr))');
// Resolution uses two semantic columns, with primary actions outside the scrolling body.
assert.equal(document.querySelectorAll('.preparation-body > .preparation-column').length,2);
assert(document.querySelector('.preparation-column:first-child')!.contains(document.querySelector('#preparation-images')));
assert(document.querySelector('.preparation-column:last-child')!.contains(document.querySelector('#preparation-estimate')));
assert(document.querySelector('.preparation-column:last-child')!.contains(document.querySelector('#preparation-budget')));
assert(document.querySelector('.preparation-footer')!.contains(document.querySelector('#prepare-images')));
assert(!document.querySelector('.preparation-body')!.contains(document.querySelector('#prepare-images')));
assert.equal(window.getComputedStyle(document.querySelector('.preparation-body')!).gridTemplateColumns,'repeat(2,minmax(0,1fr))');
const cluster=(id:string)=>document.querySelector(id)!.closest('.toolbar-cluster')!;
assert.equal(cluster('#mri-file'),cluster('#working-resolution'));assert.equal(cluster('#ct-file'),cluster('#working-resolution'));assert.notEqual(cluster('#new-pair'),cluster('#save-session'));
assert.equal(document.querySelector('#preparation-title')!.textContent,'Resolution');assert.equal(document.querySelectorAll('.preparation-frame').length,1);assert.equal(document.querySelectorAll('.preparation-spacing').length,2);
const nativeSpacing=[.2,.3,.4];for(const factor of [1,2,4,8]){document.querySelector<HTMLButtonElement>(`[data-preparation-preset="${factor===1?'native':factor}"]`)!.click();for(let axis=0;axis<3;axis++)assert(Math.abs(Number(document.querySelector<HTMLInputElement>(`[data-spacing=mri-${axis}]`)!.value)-nativeSpacing[axis]*factor)<1e-6)}
document.querySelector<HTMLButtonElement>('[data-preparation-preset=recommended]')!.click();
let field=document.querySelector<HTMLInputElement>('[data-spacing=mri-0]')!,before=document.querySelector('#preparation-working-mri')!.textContent;field.value='1';field.dispatchEvent(new dom.window.Event('input'));assert.notEqual(document.querySelector('#preparation-working-mri')!.textContent,before)
const click=()=>document.querySelector<HTMLButtonElement>('#prepare-images')!.click(),wait=async()=>{for(let i=0;i<1000;i++){await new Promise(r=>setTimeout(r,10));if(!busy&&commits>0)return}throw Error('Preparation timed out')};click();await wait();assert.equal(commits,1);assert.deepEqual(lastChanged,['mri','ct']);assert.equal(loaded.mri.native.info.dims[0],25);assert(loaded.mri.raw.dims[0]<25);assert.equal(loaded.mri.raw.frameCount,1);assert.equal(loaded.mri.raw.values,loaded.mri.nvImage.img);assert(prep.state().ready)
const acquisitionMri={version:1,source:'manual',readout:'epi',phaseEncodingDirection:'i-',totalReadoutTimeSeconds:.03},acquisitionCt={version:1,source:'manual',readout:'cartesian'};loaded.mri.acquisition=acquisitionMri;loaded.ct.acquisition=acquisitionCt
const old=loaded;document.querySelector<HTMLButtonElement>('#working-resolution')!.click();field=document.querySelector<HTMLInputElement>('[data-spacing=mri-0]')!;field.value='.6';field.dispatchEvent(new dom.window.Event('input'));click();for(let i=0;i<1000&&commits<2;i++)await new Promise(r=>setTimeout(r,10));assert.equal(commits,2);assert.deepEqual(lastChanged,[],'Resolution-only changes preserve physical registration/window state through the commit contract')
assert.deepEqual(loaded.mri.acquisition,acquisitionMri);assert.deepEqual(loaded.ct.acquisition,acquisitionCt);assert.notEqual(loaded.mri.acquisition,acquisitionMri,'New working metadata must not alias the prior snapshot')
// Worker failure and cancellation cannot partially replace either loaded input.
const beforeFailure=loaded;fail=true;document.querySelector<HTMLButtonElement>('#working-resolution')!.click();click();await new Promise(r=>setTimeout(r,100));assert.equal(loaded,beforeFailure);assert.equal(commits,2);assert(document.querySelector('#preparation-error')!.textContent!.includes('Test decoding failure'));fail=false
click();for(let i=0;i<100&&!busy;i++)await new Promise(r=>setTimeout(r,2));document.querySelector<HTMLButtonElement>('#preparation-cancel')!.click();await new Promise(r=>setTimeout(r,30));assert.equal(loaded,beforeFailure);assert.equal(commits,2);assert.equal(busy,false);assert(cancels>=3)
// Replacement clears only that image's acquisition; changing a selected frame
// also requires new acquisition details, while the other input retains them.
await prep.select('ct',[volume('new-ct.nii') as any]);click();for(let i=0;i<1000&&commits<3;i++)await new Promise(r=>setTimeout(r,10));assert.equal(commits,3);assert.deepEqual(lastChanged,['ct']);assert.deepEqual(loaded.mri.acquisition,acquisitionMri);assert.equal(loaded.ct.acquisition,undefined)
loaded.ct.acquisition=acquisitionCt;document.querySelector<HTMLButtonElement>('#working-resolution')!.click();const frame=document.querySelector<HTMLInputElement>('.preparation-frame input')!;frame.value='2';frame.dispatchEvent(new dom.window.Event('input'));click();for(let i=0;i<1000&&commits<4;i++)await new Promise(r=>setTimeout(r,10));assert.equal(commits,4);assert.deepEqual(lastChanged,['mri']);assert.equal(loaded.mri.acquisition,undefined);assert.deepEqual(loaded.ct.acquisition,acquisitionCt)
guide.close();dom.window.close();console.log('Preparation DOM + actual emitted worker: header-only selections, joint planning/live memory, guide waiting, edited resolution, shared viewer data, source vs resolution changes, failure rollback and cancellation passed.')
