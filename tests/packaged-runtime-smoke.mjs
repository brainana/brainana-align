import {fileURLToPath} from 'node:url'
import assert from 'node:assert/strict'
import fs from 'node:fs'
import os from 'node:os'
import path from 'node:path'
import { spawn,spawnSync } from 'node:child_process'
import crypto from 'node:crypto'
const app=path.resolve(process.argv[2] || '../macos-package/Brainana Align.app')
const runtime=path.join(app,'Contents/Resources/runtime')
const results=[]
for(const arch of ['darwin-arm64','darwin-x64']) {
 const scratch=fs.mkdtempSync(path.join(os.tmpdir(),'brainana-packaged-smoke-'))
 const token=crypto.randomBytes(32).toString('hex'),portFile=path.join(scratch,'port')
 const node=path.join(runtime,arch,'node')
 const identity=spawnSync(node,['-p','JSON.stringify({version:process.version,arch:process.arch})'],{encoding:'utf8',timeout:60000})
 assert.equal(identity.status,0,identity.error?.message || identity.stderr)
 const signature=spawnSync('codesign',['--verify',node],{encoding:'utf8'})
 assert.equal(signature.status,0,signature.stderr)
 const child=spawn(node,[path.join(runtime,'server.mjs'),'--mode','local','--root',scratch,'--cache',path.join(scratch,'cache'),'--port','0','--port-file',portFile,'--session-token',token],{stdio:['ignore','pipe','pipe']})
 let stderr='';child.stderr.on('data',d=>stderr+=d)
 try {
  for(let i=0;i<300 && !fs.existsSync(portFile);i++) {
   if(child.exitCode!==null)throw new Error(`Packaged server exited: ${stderr}`)
   await new Promise(r=>setTimeout(r,20))
  }
  assert.ok(fs.existsSync(portFile),'packaged server handshake missing')
  const base=`http://127.0.0.1:${Number(fs.readFileSync(portFile,'utf8'))}`
  assert.equal((await fetch(`${base}/api/health`)).status,401)
  const config=await fetch(`${base}/api/config`,{headers:{'x-brainana-session':token}})
  assert.equal(config.status,200)
  assert.equal((await config.json()).mode, 'local')
  const health=await (await fetch(`${base}/api/health`,{headers:{'x-brainana-session':token}})).json()
  assert.equal(health.version,JSON.parse(fs.readFileSync(path.resolve(fileURLToPath(new URL('../apps/align/VERSION.json',import.meta.url))),'utf8')).version)
  const index=await (await fetch(base)).text();const asset=index.match(/src="(\/assets\/[^\"]+\.js)"/)[1]
  const js=await (await fetch(base+asset)).text()
  for(const label of ['Rigid, 6 parameters','Similarity, 7 parameters','Anisotropic scale, 9 parameters','Affine, 12 parameters','guided-setup'])assert.ok(js.includes(label))
  for(const name of ['source-sans-3.woff2','ibm-plex-mono-400.woff2','ibm-plex-mono-600.woff2','OFL.txt','NOTICE.txt']) {
    const response=await fetch(base+'/fonts/'+name)
    assert.equal(response.status,200)
    assert.equal(response.headers.get('content-type'),name.endsWith('.woff2')?'font/woff2':'text/plain; charset=utf-8')
    assert.deepEqual(Buffer.from(await response.arrayBuffer()),fs.readFileSync(path.join(runtime,'dist/fonts',name)),'Bundled font/notice served exactly: '+name)
  }
  results.push({arch,status:'passed',...JSON.parse(identity.stdout),codeSignature:'valid',authenticatedServer:'passed',servedPackagedOptions:'all four'})
 } finally {
  if(child.exitCode===null && child.signalCode===null){const closed=new Promise(r=>child.once('close',r));child.kill('SIGTERM');await closed}fs.rmSync(scratch,{recursive:true,force:true})
 }
}
console.log(JSON.stringify({app,generatedAt:new Date().toISOString(),results},null,2))
