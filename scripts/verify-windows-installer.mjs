import fs from 'node:fs'
import path from 'node:path'
import os from 'node:os'
import assert from 'node:assert/strict'
import crypto from 'node:crypto'
import {execFileSync} from 'node:child_process'
const installer=path.resolve(process.argv[2]),app=path.resolve(process.argv[3]),sevenZip=process.argv[4]
if(!sevenZip)throw new Error('Usage: verify-windows-installer.mjs INSTALLER UNPACKED_APP 7ZZ')
const walk=folder=>fs.readdirSync(folder,{withFileTypes:true}).flatMap(e=>e.isDirectory()?walk(path.join(folder,e.name)):[path.join(folder,e.name)])
const hash=f=>crypto.createHash('sha256').update(fs.readFileSync(f)).digest('hex')
const scratch=fs.mkdtempSync(path.join(os.tmpdir(),'brainana-nsis-verify-'))
try {
 const list=execFileSync(sevenZip,['l',installer],{encoding:'utf8'});assert.ok(list.includes('Type = Nsis'))
 execFileSync(sevenZip,['x',installer,'-o'+path.join(scratch,'installer'),'-y'],{encoding:'utf8'})
 const archive=path.join(scratch,'installer/$PLUGINSDIR/app-64.7z');assert.ok(fs.statSync(archive).size>50_000_000)
 const uninstaller=path.join(scratch,'installer/$R0/Uninstall Brainana Align.exe');assert.equal(fs.readFileSync(uninstaller).subarray(0,2).toString(),'MZ')
 const extracted=path.join(scratch,'app');execFileSync(sevenZip,['x',archive,'-o'+extracted,'-y'],{encoding:'utf8'})
 const expected=walk(app).map(f=>path.relative(app,f)).sort(),actual=walk(extracted).map(f=>path.relative(extracted,f)).sort();assert.deepEqual(actual,expected)
 for(const name of expected)assert.equal(hash(path.join(app,name)),hash(path.join(extracted,name)),name)
 console.log(JSON.stringify({status:'passed',installerBytes:fs.statSync(installer).size,format:'NSIS-3 Unicode',embeddedAppBytes:fs.statSync(archive).size,fileCount:expected.length,embeddedFiles:'every extracted file matches verified win-unpacked SHA256',uninstaller:'embedded PE executable present',execution:'installation/uninstallation not executed; Windows host required'},null,2))
}finally{fs.rmSync(scratch,{recursive:true,force:true})}
