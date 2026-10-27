import fs from 'node:fs'
import path from 'node:path'
import assert from 'node:assert/strict'
import crypto from 'node:crypto'
import {execFileSync} from 'node:child_process'
const root=path.resolve(new URL('..',import.meta.url).pathname)
const app=path.resolve(process.argv[2] || 'generated/Brainana Align Browser.app')
const runtime=path.join(app,'Contents/Resources/runtime')
const hash=file=>crypto.createHash('sha256').update(fs.readFileSync(file)).digest('hex')
const walk=folder=>fs.readdirSync(folder,{withFileTypes:true}).flatMap(e=>e.isDirectory()?walk(path.join(folder,e.name)):[path.join(folder,e.name)])
const meta=JSON.parse(fs.readFileSync(path.join(root,'VERSION.json')))
const plist=JSON.parse(execFileSync('/usr/bin/plutil',['-convert','json','-o','-',path.join(app,'Contents/Info.plist')],{encoding:'utf8'}))
assert.equal(plist.CFBundleIdentifier,'org.brainana.align.browser')
assert.equal(plist.BrainanaReleaseVersion,meta.version);assert.equal(plist.CFBundleVersion,meta.bundleVersion)
assert.equal(plist.LSMinimumSystemVersion,meta.minimumMacOS)
for(const file of walk(path.join(root,'apps/align/dist')))assert.equal(hash(file),hash(path.join(runtime,'dist',path.relative(path.join(root,'apps/align/dist'),file))))
for(const name of ['version.mjs','version.env'])assert.equal(hash(path.join(root,'apps/align',name)),hash(path.join(runtime,name)))
for(const name of ['runtime.mjs','sftpClient.mjs','platformCore.mjs','replaceWithBackup.mjs','networkBrowser.mjs','networkSftpClient.mjs','knownHosts.mjs'])assert.equal(hash(path.join(root,'packages/core-server',name)),hash(path.join(runtime,name)))
assert.equal(fs.readFileSync(path.join(runtime,'server.mjs'),'utf8'),fs.readFileSync(path.join(root,'apps/align/server.mjs'),'utf8').replace('@brainana/core-server/runtime.mjs','./runtime.mjs'))
assert.equal(hash(path.join(root,'packaging/runtime-command.sh')),hash(path.join(runtime,'runtime-command.sh')))
const launcher=path.join(app,'Contents/MacOS/brainana-align-launcher')
assert.equal(hash(path.join(root,'packaging/brainana-align-launcher')),hash(launcher))
execFileSync('/bin/bash',['-n',launcher]);execFileSync('/usr/bin/codesign',['--verify','--deep','--strict',app])
const identities=[]
for(const arch of ['arm64','x64']){const node=path.join(runtime,'darwin-'+arch,'node');execFileSync('/usr/bin/codesign',['--verify',node]);const identity=JSON.parse(execFileSync(node,['-p','JSON.stringify({version:process.version,arch:process.arch})'],{encoding:'utf8'}));assert.equal(identity.arch,arch);assert.equal(execFileSync(node,['-p',`typeof require(${JSON.stringify(path.join(runtime,'node_modules/ssh2'))}).Client`],{encoding:'utf8'}).trim(),'function');identities.push(identity)}
console.log(JSON.stringify({version:meta.version,status:'passed',appSignature:'verified ad-hoc',sourceHashes:'exact frontend/server/launcher/helper match',runtimes:identities,totalBytes:walk(app).filter(file=>!fs.lstatSync(file).isSymbolicLink()).reduce((sum,file)=>sum+fs.statSync(file).size,0),nativeBrowserAndSSH:'pending'},null,2))
