import assert from 'node:assert/strict'
import {createRequire} from 'node:module'
import {alignmentLayout} from '../apps/align/src/alignmentLayout.ts'
import {installExportDialog} from '../apps/align/src/exportDialog.ts'
import {installExportDestination} from '../apps/align/src/exportDestination.ts'
import {imageGeometryAlignment} from '../apps/align/src/geometryInitialization.ts'
import {saveArtifacts} from '../apps/align/src/exportArtifacts.ts'
const require=createRequire(new URL('../apps/align/package.json',import.meta.url));const {JSDOM}=require('jsdom')
const dom=new JSDOM('<div id="app">'+alignmentLayout('test')+'</div>',{url:'http://localhost/#session='+ 'a'.repeat(64),pretendToBeVisual:true})
Object.assign(globalThis,{window:dom.window,document:dom.window.document,CustomEvent:dom.window.CustomEvent})
let fit=imageGeometryAlignment('ct-mri'),busy=false,reviewed=0
fit.proposal={kind:'automatic',matrix:fit.matrix,params:[0,0,0,0,0,0],scoreBefore:1,scoreAfter:1.1}
const ui=installExportDialog(()=>({fit,images:true,busy}),()=>reviewed++)
ui.open();const d=dom.window.document
assert(d.querySelector<HTMLButtonElement>('#save-aligned')!.disabled);assert(d.querySelector<HTMLButtonElement>('#save-transform')!.disabled)
assert(!d.querySelector<HTMLButtonElement>('#save-session-export')!.disabled,'pending previews may be saved in sessions')
assert(d.querySelector('#export-notice')!.textContent!.includes('pending preview'))
d.querySelector<HTMLButtonElement>('#export-review-proposal')!.click();assert.equal(reviewed,1)
fit.proposal=null;ui.refresh();assert(!d.querySelector<HTMLButtonElement>('#save-transform')!.disabled);assert(d.querySelector('#export-notice')!.classList.contains('hidden'))
let release!:()=>void
const work=ui.run(()=>new Promise<void>(r=>release=r));assert(d.querySelector<HTMLButtonElement>('#save-aligned')!.disabled);assert(d.querySelector<HTMLButtonElement>('#save-session-export')!.disabled);release();await work;assert(!d.querySelector<HTMLButtonElement>('#save-transform')!.disabled)
await assert.rejects(ui.run(async()=>{throw new Error('Resampling failed')}),/Resampling failed/);assert.equal(d.querySelector('#export-feedback')!.textContent,'Resampling failed');assert(d.querySelector('#export-feedback')!.classList.contains('error'))
let fail=true;const attempted:string[]=[]
Object.assign(globalThis,{fetch:async(url:string,options:any)=>{
 if(url==='/api/config')return new Response(JSON.stringify({mode:'local'}))
 if(url==='/api/local-export-folder/select')return new Response(JSON.stringify({selected:true,path:'/exports'}))
 if(url.startsWith('/api/local-export-file')){assert.equal(options.headers.get('x-brainana-session'),'a'.repeat(64));const name=new URL(url,'http://localhost').searchParams.get('filename')!;attempted.push(name);return new Response(JSON.stringify(fail?{error:'Disk full'}:{path:'/exports/'+name}),{status:fail?500:200})}
 throw new Error(url)
}})
installExportDestination();d.querySelector<HTMLButtonElement>('#ba-folder-button')!.click();await new Promise(r=>setTimeout(r,0))
const artifacts=['first.json','second.txt'].map(filename=>({filename,blob:new Blob(['content'])}))
await assert.rejects(ui.run(()=>saveArtifacts(artifacts)),/Disk full/);assert.deepEqual(attempted,['first.json'],'multi-file export stops after failure');assert.equal(d.querySelector('#export-feedback')!.textContent,'Disk full')
fail=false;await ui.run(()=>saveArtifacts(artifacts));assert.deepEqual(attempted,['first.json','first.json','second.txt'],'queue recovers for the next export');assert(d.querySelector('#export-feedback')!.textContent!.includes('/exports/second.txt'))
busy=true;ui.refresh();assert(d.querySelector<HTMLButtonElement>('#save-transform')!.disabled)
dom.window.close();console.log('Pending-proposal export gates, review route, session preservation, busy guards, visible failures, batch short-circuit and queue recovery passed.')
