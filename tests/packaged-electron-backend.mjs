import assert from 'node:assert/strict'
import fs from 'node:fs/promises'
import os from 'node:os'
import path from 'node:path'
import {spawn} from 'node:child_process'
import {fileURLToPath} from 'node:url'
const root=path.resolve(path.dirname(fileURLToPath(import.meta.url)),'..')
const app=path.resolve(process.argv[2]),bin=path.join(app,'Contents/MacOS/Brainana Align'),asar=path.join(app,'Contents/Resources/app.asar')
const scratch=await fs.mkdtemp(path.join(os.tmpdir(),'brainana-electron-backend-')),handshake=path.join(scratch,'handshake')
const requestedArch=process.argv[3]
const child=spawn(requestedArch==='x64'?'/usr/bin/arch':bin,[...(requestedArch==='x64'?['-x86_64',bin]:[]),path.join(asar,'apps/align/server.mjs'),'--root',scratch,'--cache',path.join(scratch,'cache'),'--handshake-file',handshake],{env:{...process.env,ELECTRON_RUN_AS_NODE:'1'},stdio:['ignore','pipe','pipe']})
let errors='';child.stderr.on('data',d=>errors+=d)
try {
 let data
 for(let i=0;i<600;i++) {
  if(child.exitCode!==null)throw new Error('Packaged Electron backend stopped: '+errors)
  try{data=JSON.parse(await fs.readFile(handshake,'utf8'));break}catch(error){if(error.code!=='ENOENT')throw error}
  await new Promise(resolve=>setTimeout(resolve,50))
 }
 assert.ok(data,'Packaged Electron server handshake unavailable: '+errors)
 const base=`http://127.0.0.1:${data.port}`
 assert.equal((await fetch(base+'/api/config')).status,401)
 const health=await (await fetch(base+'/api/health',{headers:{'x-brainana-session':data.sessionToken}})).json()
 assert.equal(health.version,JSON.parse(await fs.readFile(path.join(root,'VERSION.json'),'utf8')).version)
 const html=await (await fetch(base)).text(),asset=html.match(/src="([^\"]+\.js)"/)[1]
 const js=await (await fetch(base+asset)).text()
 for(const text of ['Rigid, 6 parameters','Similarity, 7 parameters','Anisotropic scale, 9 parameters','Affine, 12 parameters','guided-setup','Run linear registration','linear-registration','Linear registration','jip-registration','JIP registration','itk-registration','ITK nonlinear B-spline','Custom parameter groups','registrationWorker-'])assert.ok(js.includes(text))
 const workerAsset=js.match(/registrationWorker-[A-Za-z0-9_-]+\.js/)?.[0];assert.ok(workerAsset);assert.equal((await fetch(base+'/assets/'+workerAsset)).status,200)
  for(const name of ['source-sans-3.woff2','ibm-plex-mono-400.woff2','ibm-plex-mono-600.woff2','OFL.txt','NOTICE.txt']) {
    const response=await fetch(base+'/fonts/'+name)
    assert.equal(response.status,200)
    assert.equal(response.headers.get('content-type'),name.endsWith('.woff2')?'font/woff2':'text/plain; charset=utf-8')
    assert.deepEqual(Buffer.from(await response.arrayBuffer()),await fs.readFile(path.join(root,'apps/align/dist/fonts',name)),'Bundled font/notice served exactly: '+name)
  }
 console.log(JSON.stringify({status:'passed',version:health.version,architecture:requestedArch || process.arch,backend:'Actual ASAR server executed by bundled Electron Node',authenticatedAPI:'passed',servedFrontend:'four models and guide',stderr:errors.trim()},null,2))
}finally {
 if(child.exitCode===null){const closed=new Promise(resolve=>child.once('exit',resolve));child.kill('SIGTERM');await closed}
 await fs.rm(scratch,{recursive:true,force:true})
}
