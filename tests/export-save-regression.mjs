import assert from 'node:assert/strict'
import fs from 'node:fs/promises'
import path from 'node:path'
import os from 'node:os'
import vm from 'node:vm'
import {EventEmitter} from 'node:events'
import {exportFilename,exportFilenames} from '../packages/core-client/exportNames.mjs'
for(const name of ['../escape','bad\\name','.', '', 'bad\0name', 'bad.', 'trailing '])assert.throws(()=>exportFilename(name))
assert.deepEqual(exportFilenames(['image.nii.gz','image.json'],'subject'),['subject.nii.gz','subject.json'])
assert.deepEqual(exportFilenames(['image.nii.gz','image.json'],'subject.nii'),['subject.nii.gz','subject.json'])
assert.deepEqual(exportFilenames(['registration.json','forward.txt','inverse.txt'],'case.json'),['case.json','case_forward.txt','case_inverse.txt'])
const root=await fs.mkdtemp(path.join(os.tmpdir(),'align-save-test-')),handlers=new Map(),choices=[],prompts=[],writes=[]
const context=vm.createContext({URL,console,setTimeout,clearTimeout})
let response=1, failure=null, delay=null
const wc=new EventEmitter();wc.mainFrame={};wc.getURL=()=> 'http://localhost:1234/';wc.session=new EventEmitter()
wc.downloadURL=url=>{const item=new EventEmitter();item.getURL=()=>url;item.setSavePath=output=>writes.push(output);item.cancel=()=>item.emit('done',{},'cancelled');wc.session.emit('will-download',{},item,wc);if(delay)delay=()=>item.emit('done',{},'completed');else queueMicrotask(()=>item.emit('done',{},failure??'completed'))}
const win={webContents:wc,isDestroyed:()=>false}
const electron=new vm.SyntheticModule(['ipcMain','dialog'],function(){this.setExport('ipcMain',{handle:(name,fn)=>handlers.set(name,fn)});this.setExport('dialog',{showSaveDialog:async(_win,options)=>{assert.equal(_win,win);prompts.push(options);return choices.shift()??{canceled:true}},showMessageBox:async()=>({response})})},{context})
const module=new vm.SourceTextModule(await fs.readFile(new URL('../packages/core-desktop/exportSaves.mjs',import.meta.url),'utf8'),{context})
await module.link(async name=>{if(name==='electron')return electron;const value=await import(name);return new vm.SyntheticModule(Object.keys(value),function(){for(const key of Object.keys(value))this.setExport(key,value[key])},{context})});await module.evaluate()
module.namespace.installExportSaves({directory:root,window:()=>win})
const save=files=>handlers.get('brainana-export-save')({sender:wc,senderFrame:wc.mainFrame},files),files=[{filename:'image.nii.gz',url:'blob:http://localhost:1234/one'},{filename:'image.json',url:'blob:http://localhost:1234/two'}]
try{
 await assert.rejects(handlers.get('brainana-export-save')({sender:wc,senderFrame:{}},files),/Untrusted/)
 await assert.rejects(save([{...files[0],url:'file:///etc/passwd'}]),/Blob URL/)
 await assert.rejects(save([{...files[0],url:'blob:http://localhost:4567/one'}]),/Blob URL/)
 await assert.rejects(save([{...files[0],filename:'../escape'}]),/filename/)
 assert((await save(files)).canceled);assert.equal(writes.length,0)
 choices.push({filePath:path.join(root,'custom name.nii.gz')});let result=await save(files)
 assert.deepEqual(Array.from(result.paths),[path.join(root,'custom name.nii.gz'),path.join(root,'custom name.json')]);assert.deepEqual(writes,Array.from(result.paths));assert.equal(JSON.parse(await fs.readFile(path.join(root,'align-export.json'),'utf8')).folder,root)
 choices.push({filePath:path.join(root,'next.nii.gz')});await save(files);assert.equal(prompts.at(-1).defaultPath,path.join(root,'image.nii.gz'),'Last-used folder is a suggestion, never a silent write destination')
 await fs.writeFile(path.join(root,'protected.json'),'preserve');response=0;choices.push({filePath:path.join(root,'protected.nii.gz')});const before=writes.length;assert((await save(files)).canceled);assert.equal(writes.length,before);assert.equal(await fs.readFile(path.join(root,'protected.json'),'utf8'),'preserve')
 await fs.writeFile(path.join(root,'normalized.nii.gz'),'preserve compressed volume');choices.push({filePath:path.join(root,'normalized')});assert((await save(files)).canceled);assert.equal(writes.length,before,'Adding the required extension still checks the actual destination before overwrite')
 response=1;failure='interrupted';choices.push({filePath:path.join(root,'failed.nii.gz')});await assert.rejects(save(files),/interrupted/);assert.equal(writes.length,before+1,'No metadata writes after first-file failure');assert.equal(wc.session.listenerCount('will-download'),0)
 failure=null;delay=true;choices.push({filePath:path.join(root,'pending.nii.gz')});const pending=save([files[0]]);await new Promise(r=>setTimeout(r,10));await assert.rejects(save(files),/already open/);delay();await pending;delay=null
 assert.equal(wc.listenerCount('destroyed'),0);assert.equal(wc.listenerCount('did-start-navigation'),0)
 choices.push({filePath:path.join(root,'recovered.nii.gz')});await save(files)
 console.log('Native save: trusted main frame and same-origin Blob-only IPC, combined filename/folder picker, grouped image/metadata naming, extensions, overwrite cancellation, interrupted-write short-circuit, busy guard, cleanup and retry passed.')
}finally{await fs.rm(root,{recursive:true,force:true})}
