import assert from 'node:assert/strict'
import fs from 'node:fs/promises'
import sync from 'node:fs'
import path from 'node:path'
import os from 'node:os'
import vm from 'node:vm'
import {Worker} from 'node:worker_threads'
import {fileURLToPath} from 'node:url'
import * as nifti from 'nifti-reader-js'
import {gunzipSync} from 'fflate'
import {bootServer} from '../packages/core-launcher/launch.mjs'
import {sessionFileGrants} from '../packages/core-launcher/sessionFileGrants.mjs'
import {serveSessionImage} from '../packages/core-server/sessionImageFiles.mjs'
import {sessionHeader,readSessionHeader,findSessionSources} from '../apps/align/desktop/sessionSourcePaths.mjs'

const root=path.resolve(path.dirname(fileURLToPath(import.meta.url)),'..'),scratch=await fs.mkdtemp(path.join(os.tmpdir(),'align-session-files-'))
let boot
try{
 const browsing=path.join(scratch,'browsing'),source=path.join(scratch,'mounted share with spaces');await fs.mkdir(browsing);await fs.mkdir(source)
 const mri=path.join(source,'anatomy.nii'),ct=path.join(source,'ct.nii'),session=path.join(source,'saved.json')
 await fs.writeFile(mri,Buffer.from('Original MRI bytes'));await fs.writeFile(ct,Buffer.from('Original CT bytes'))
 const payload={application:'Brainana Align',registrationPairing:'mri-ct',note:'Quoted "images": { and } are not keys',images:{mri:{name:'anatomy.nii',sourceFiles:['anatomy.nii']},ct:{name:'ct.nii',sourceFiles:['ct.nii']}},fit:{largeField:'x'.repeat(3*1024**2)}}
 await fs.writeFile(session,JSON.stringify(payload));assert.deepEqual((await readSessionHeader(session)).images,payload.images)
 assert.deepEqual(await findSessionSources(session,payload.images.mri),[mri],'Older sessions find image siblings without stored paths')
 assert.throws(()=>sessionHeader('{"application":"Wrong app","images":{}}'),/Not an Align/)
 assert.throws(()=>sessionHeader('{"application":"Brainana Align","note":"unterminated'),/unavailable/)
 const meta=JSON.parse(await fs.readFile(path.join(root,'VERSION.json'),'utf8'));boot=await bootServer({serverPath:path.join(root,'apps/align/server.mjs'),root:browsing,cache:path.join(scratch,'cache'),version:meta.version,buildId:meta.buildId})
 const auth={'x-brainana-session':boot.handshake.sessionToken},url=id=>new URL('/api/session-image?file='+id,boot.url)
 const [grant]=await boot.grantFiles([mri]);assert.equal((await boot.grantFiles([mri]))[0].id,grant.id)
 assert.equal((await fetch(url(grant.id))).status,401)
 assert.equal((await fetch(url(grant.id),{headers:{...auth,Origin:'https://example.invalid'}})).status,403)
 assert.equal((await fetch(url('0'.repeat(48)),{headers:auth})).status,404)
 assert.equal((await fetch(url(grant.id),{method:'POST',headers:auth})).status,405)
 assert.equal((await fetch(url(grant.id),{headers:{...auth,Range:'bytes=0-1,4-5'}})).status,416)
 assert.equal(await (await fetch(url(grant.id),{headers:auth})).text(),'Original MRI bytes')
 const partial=await fetch(url(grant.id),{headers:{...auth,Range:'bytes=9-11'}});assert.equal(partial.status,206);assert.equal(await partial.text(),'MRI');assert.equal(partial.headers.get('content-range'),'bytes 9-11/18')
 assert((await fetch(new URL('/api/file?path=../mounted%20share%20with%20spaces/anatomy.nii',boot.url),{headers:auth})).status>=400,'Normal browsing confinement is preserved')

 // Run the real main-process handler with only native dialog choices adapted.
 let handler,asked=0;const frame={},webContents={mainFrame:frame},win={webContents};let choice={canceled:false,filePaths:[mri]}
 const context=vm.createContext({console}),modules={}
 modules.electron=new vm.SyntheticModule(['ipcMain','dialog'],function(){this.setExport('ipcMain',{handle:(_name,fn)=>handler=fn});this.setExport('dialog',{showOpenDialog:async()=>{asked++;return choice}})},{context})
 for(const spec of ['node:path','node:fs/promises']){const value=await import(spec);modules[spec]=new vm.SyntheticModule(Object.keys(value),function(){for(const key of Object.keys(value))this.setExport(key,value[key])},{context})}
 const paths=await import('../apps/align/desktop/sessionSourcePaths.mjs');modules['./sessionSourcePaths.mjs']=new vm.SyntheticModule(Object.keys(paths),function(){for(const key of Object.keys(paths))this.setExport(key,paths[key])},{context})
 const module=new vm.SourceTextModule(await fs.readFile(path.join(root,'apps/align/desktop/sessionFiles.mjs'),'utf8'),{context});await module.link(spec=>modules[spec]);await module.evaluate();module.namespace.installSessionFiles({window:()=>win,fileAccess:()=>boot.grantFiles})
 const event={sender:webContents,senderFrame:frame},opened=await handler(event,'open',{path:session,wanted:['mri','ct']})
 assert.equal(opened.mri[0].path,mri);assert.equal(opened.ct[0].path,ct);assert.equal(asked,0,'An older session next to its images must open without a picker')
 await assert.rejects(handler({...event,senderFrame:{}},'open',{path:session,wanted:[]}),/Untrusted/)
 await assert.rejects(handler({...event,sender:{}},'choose',{}),/Untrusted/)
 await assert.rejects(handler(event,'arbitrary-command',{}),/Unknown/)
 await assert.rejects(handler(event,'open',{path:session,wanted:['unknown']}),/Invalid/)
 const moved=path.join(source,'renamed anatomy.nii');await fs.rename(mri,moved);choice={canceled:false,filePaths:[moved]}
 const relinked=await handler(event,'open',{path:session,wanted:['mri']});assert.equal(relinked.mri[0].path,moved);assert.equal(asked,1)
 assert.equal((await fetch(url(grant.id),{headers:auth})).status,409,'A moved/changed source cannot be read through an old grant')
 choice={canceled:true,filePaths:[]};assert.equal(await handler(event,'open',{path:session,wanted:['mri']}),null)
 await fs.appendFile(ct,' changed');assert.equal((await fetch(url(opened.ct[0].id),{headers:auth})).status,409)
 // A request can disappear while stat/open is pending, before stream ownership.
 // Verify the actual opened descriptor is closed on that early cancellation.
 const cancelRegistry=path.join(scratch,'cancel-grants.json'),[cancelGrant]=await sessionFileGrants(cancelRegistry)([moved]),originalOpen=fs.open;let openedHandle
 fs.open=async(...args)=>{const handle=await originalOpen(...args);openedHandle=handle;return handle}
 try{assert.equal(await serveSessionImage({method:'GET',headers:{},aborted:true},{destroyed:true,writeHead(){throw Error('Cancelled response must not write headers')}},url(cancelGrant.id),cancelRegistry),true);assert.equal(openedHandle.fd,-1)}finally{fs.open=originalOpen}

 // Read actual sparse pixels beyond 4 GiB through the real emitted export
 // worker and the desktop-owned HTTP ranges. No full native File/array copy.
 const filename=path.join(source,'large.nii'),identity=[[1,0,0,0],[0,1,0,0],[0,0,1,0],[0,0,0,1]],h=new nifti.NIFTI1();h.dims=[3,1024,1024,1026,1,1,1,1];h.pixDims=[1,1,1,1,1,1,1,1];h.sform_code=2;h.affine=identity;h.datatypeCode=16;h.numBitsPerVoxel=32;h.vox_offset=352;h.magic='n+1\0';h.littleEndian=true;h.xyzt_units=2
 const handle=await fs.open(filename,'w');await handle.truncate(352+1024*1024*1026*4);await handle.write(new Uint8Array(h.toArrayBuffer()),0,352,0)
 for(let z=1021;z<1026;z++)for(let y=1019;y<1024;y++){const row=Buffer.alloc(20);for(let x=1019;x<1024;x++)row.writeFloatLE((x-1019)+10*(y-1019)+100*(z-1021),(x-1019)*4);await handle.write(row,0,20,352+4*(1019+1024*(y+1024*z)))}await handle.close()
 const [large]=await boot.grantFiles([filename]);assert(large.size>2**32)
 const assets=path.resolve(process.argv[2]??path.join(root,'apps/align/dist'),'assets'),workerName=sync.readdirSync(assets).find(n=>/^nativeExportWorker-.*\.js$/.test(n)),code=await fs.readFile(path.join(assets,workerName),'utf8')
 const worker=new Worker(`const {parentPort}=require('node:worker_threads');globalThis.self={postMessage:m=>parentPort.postMessage(m)};parentPort.on('message',data=>self.onmessage({data}));\n${code}`,{eval:true})
 try{
  const result=await new Promise((resolve,reject)=>{worker.on('error',reject);worker.on('message',data=>{if(data.type==='error')reject(Error(data.message));if(data.type==='result')resolve(data)});worker.postMessage({file:{...large,kind:'brainana-range-file-v1',url:url(large.id).href,sessionToken:boot.handshake.sessionToken},info:{name:'large.nii',dims:[1024,1024,1026],spacing:[1,1,1],affine:identity,frames:1,offset:352,bytesPerVoxel:4,datatype:16,littleEndian:true,slope:1,intercept:0},frame:0,target:{dims:[3,3,3],pixDims:[1,1,1],affine:identity},matrix:[[1,0,0,-1020.25],[0,1,0,-1020.125],[0,0,1,-1022.25],[0,0,0,1]],interpolation:'linear'})})
  const bytes=gunzipSync(new Uint8Array(await result.blob.arrayBuffer())),header=nifti.readHeader(bytes.buffer),values=new Float32Array(nifti.readImage(header,bytes.buffer))
  for(let z=0;z<3;z++)for(let y=0;y<3;y++)for(let x=0;x<3;x++)assert.equal(values[x+3*(y+3*z)],(x+1.25)+10*(y+1.125)+100*(z+1.25))
  assert(result.nativeRead.peakCacheBytes<=128*1024**2);assert(result.nativeRead.largestReadBytes<=8*1024**2)
 }finally{await worker.terminate()}
 console.log('Session files: bounded JSON prefix, old sibling lookup, trusted native relinking/cancel, scoped authenticated ranges outside browse root, stale-source refusal and exact emitted export above 4 GiB passed.')
}finally{await boot?.stop();await fs.rm(scratch,{recursive:true,force:true})}
