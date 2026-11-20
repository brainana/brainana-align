import assert from 'node:assert/strict'
import {JSDOM} from 'jsdom'
import {installExportDestination} from '../apps/align/src/exportDestination.ts'
import {saveArtifacts} from '../apps/align/src/exportArtifacts.ts'
const dom=new JSDOM('<div id="export-modal"><div class="modal-card"></div></div>',{url:'http://localhost/#session='+'a'.repeat(64),pretendToBeVisual:true}),w=dom.window,d=w.document
Object.assign(globalThis,{window:w,document:d,CustomEvent:w.CustomEvent})
const tick=()=>new Promise(r=>setTimeout(r,0)),saved:any[]=[],artifacts=[{filename:'volume.nii.gz',blob:new Blob(['voxels'])},{filename:'volume.json',blob:new Blob(['metadata'])}]
let fail=false,cancel=false
;(w as any).brainanaExports={save:async(files:any[])=>{assert.equal(files.length,2);assert(files.every(f=>typeof f.url==='string'&&!('blob' in f)));if(fail)throw Error('Write failed');if(cancel)return {canceled:true};saved.push(files);return {canceled:false,paths:['/chosen/new.nii.gz','/chosen/new.json']}}} as any
Object.assign(globalThis,{fetch:async(url:string,options:any)=>{
 if(url==='/api/config')return new Response(JSON.stringify({mode:'remote'}))
 if(url.startsWith('/api/save-list'))return new Response(JSON.stringify({path:'output',entries:[]}))
 if(url.startsWith('/api/save-file')){assert.equal(options.headers.get('x-brainana-session'),'a'.repeat(64));saved.push({path:new URL(url,'http://localhost').searchParams.get('path'),text:await options.body.text()});return new Response(JSON.stringify({path:new URL(url,'http://localhost').searchParams.get('path')}))}
 throw Error(url)
}})
installExportDestination();await tick();assert.equal(d.querySelector('#ba-folder-button'),null)
await saveArtifacts(artifacts);assert.equal(saved.length,1);assert(d.querySelector('#ba-export-status')!.textContent!.includes('/chosen/new.nii.gz'))
cancel=true;await assert.rejects(saveArtifacts(artifacts),/cancelled/);cancel=false;fail=true;await assert.rejects(saveArtifacts(artifacts),/Write failed/);fail=false;await saveArtifacts(artifacts);assert.equal(saved.length,2)
const select=d.querySelector<HTMLSelectElement>('#ba-destination')!;select.value='workstation';select.dispatchEvent(new w.Event('change'))
const pending=saveArtifacts(artifacts);await tick();assert(!d.querySelector('#ba-folder-modal')!.classList.contains('hidden'));const name=d.querySelector<HTMLInputElement>('#ba-save-name')!;name.value='renamed';name.dispatchEvent(new w.Event('input'));assert(d.querySelector('#ba-save-related')!.textContent!.includes('renamed.json'));d.querySelector<HTMLButtonElement>('#ba-save-submit')!.click();await pending
assert.deepEqual(saved.slice(-2),[{path:'output/renamed.nii.gz',text:'voxels'},{path:'output/renamed.json',text:'metadata'}])
const cancelled=saveArtifacts(artifacts);await tick();d.querySelector<HTMLButtonElement>('#ba-remote-cancel')!.click();await assert.rejects(cancelled,/cancelled/)
delete (w as any).brainanaExports
select.value='local';select.dispatchEvent(new w.Event('change'))
const names:string[]=[],texts:string[]=[]
;(w as any).showSaveFilePicker=async({suggestedName}:{suggestedName:string})=>{names.push(suggestedName);return {name:names.length===1?'browser.nii.gz':suggestedName,createWritable:async()=>({write:async(blob:Blob)=>texts.push(await blob.text()),close:async()=>{}})}}
const browser=saveArtifacts(artifacts);await tick();d.querySelector<HTMLButtonElement>('#ba-save-submit')!.click();await tick();d.querySelector<HTMLButtonElement>('#ba-save-submit')!.click();await browser
assert.deepEqual(names,['volume.nii.gz','browser.json']);assert.deepEqual(texts,['voxels','metadata'])
dom.window.close();console.log('Export destination: no separate folder selector, one native request per image/metadata group, visible saved paths, cancellation/failure/retry, authenticated workstation filename+folder dialog and consistent sidecar names passed.')
