import fs from 'node:fs'
import path from 'node:path'
import {fileURLToPath} from 'node:url'
import {spawnSync} from 'node:child_process'
const root=path.resolve(path.dirname(fileURLToPath(import.meta.url)),'../..')
const pkg=JSON.parse(fs.readFileSync(path.join(root,'package.json'),'utf8'))
// Native macOS launcher/OS commands are separately checked by the macOS jobs.
// Browser GPU suites get a dedicated Linux job with installed browser engines.
const macOnly=new Set(['test:desktop-packaging','test:macos-runtime','test:macho','test:lifecycle','test:lifecycle-detach','test:lifecycle-launchd','test:ssh-cancel','test:ssh-terminal','test:local-startup'])
const posixOnly=new Set(['test:launch-handshake'])
const native=new Set(['test:jip-service','test:displacement-simpleitk'])
const nativeOnly=process.argv.includes('--native-only')
const browser=new Set(['test:browser','test:browser-matrix'])
const results=[]
for(const name of Object.keys(pkg.scripts).filter(n=>n.startsWith('test:')&&(!nativeOnly||native.has(n)))){
 let reason=!nativeOnly&&native.has(name)?'Required in native worker stage':browser.has(name)?'Dedicated browser job':macOnly.has(name)&&process.platform!=='darwin'?'macOS-specific launcher test':posixOnly.has(name)&&process.platform==='win32'?'POSIX Python launcher integration':null
 if(reason){results.push({name,status:'not-applicable',reason});continue}
 const started=Date.now(),r=spawnSync(process.execPath,[process.env.npm_execpath??path.join(path.dirname(process.execPath),'node_modules/npm/bin/npm-cli.js'),'run',name],{cwd:root,stdio:'inherit',env:process.env,timeout:240000})
 const status=r.status===0?'passed':'failed';results.push({name,status,exit:r.status,signal:r.signal,durationMs:Date.now()-started});console.log(name+': '+status)
 // Finish independent suites so one portability failure does not hide others.
 // The final exit code still blocks every packaging/publish stage.
}
const out=path.join(root,'artifacts/validation');fs.mkdirSync(out,{recursive:true});fs.writeFileSync(path.join(out,`tests-${process.platform}-${process.arch}${nativeOnly?'-native':''}.json`),JSON.stringify({platform:process.platform,arch:process.arch,results},null,2)+'\n')
if(results.some(v=>v.status==='failed'))process.exitCode=1
