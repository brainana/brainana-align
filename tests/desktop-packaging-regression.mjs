import {fileURLToPath} from 'node:url'
import assert from 'node:assert/strict'
import fs from 'node:fs'
import os from 'node:os'
import path from 'node:path'
import {spawnSync} from 'node:child_process'
import vm from 'node:vm'
import {EventEmitter} from 'node:events'
import {preparePayload} from '../scripts/prepare-payload.mjs'
const root=path.resolve(fileURLToPath(new URL('..',import.meta.url)))
const scratch=fs.mkdtempSync(path.join(os.tmpdir(),'brainana-slim-packaging-'))
try {
 const executable=path.join(scratch,'Runtime with spaces')
 fs.writeFileSync(executable,'#!/bin/bash\nprintf "%s\\n" "${ELECTRON_RUN_AS_NODE:-unset}" "$@"\n');fs.chmodSync(executable,0o755)
 const payload=path.join(scratch,'payload'),runtime=path.join(payload,'Contents/Resources/runtime')
 fs.mkdirSync(path.join(runtime,'darwin-arm64'),{recursive:true});fs.writeFileSync(path.join(runtime,'darwin-arm64/node'),'stale')
 preparePayload(root,payload)
 assert.equal(fs.existsSync(path.join(runtime,'darwin-arm64')),false,'stale runtime removed')
 assert.equal(fs.existsSync(path.join(runtime,'darwin-x64')),false)
 assert.deepEqual(fs.readFileSync(path.join(runtime,'dist/index.html')),fs.readFileSync(path.join(root,'apps/align/dist/index.html')))
 const helper=path.join(runtime,'runtime-command.sh'),wrapper=path.join(scratch,'wrapper with spaces.sh'),log=path.join(scratch,'server log.txt')
 const script='. "$1"; brainana_select_runtime || exit $?; brainana_write_server_wrapper "$2" "$3" "$4" "$5"'
 const literal='space $() `command` ; "quoted"\nsecond line'
 let result=spawnSync('/bin/bash',['-c',script,'test',helper,wrapper,log,'server with spaces.mjs',literal],{env:{...process.env,BRAINANA_DESKTOP_REMOTE:'1',BRAINANA_DESKTOP_EXECUTABLE:executable},encoding:'utf8'})
 assert.equal(result.status,0,result.stderr)
 result=spawnSync('/bin/bash',[wrapper],{env:{PATH:process.env.PATH},encoding:'utf8'})
 assert.equal(result.status,0,result.stderr)
 assert.equal(fs.readFileSync(log,'utf8'),'1\nserver with spaces.mjs\n'+literal+'\n','launchd wrapper explicitly restores Electron mode and preserves literal arguments')
 result=spawnSync('/bin/bash',['-c','. "$1"; brainana_select_runtime','test',helper],{env:{...process.env,BRAINANA_DESKTOP_REMOTE:'1',BRAINANA_DESKTOP_EXECUTABLE:''},encoding:'utf8'})
 assert.equal(result.status,33,'desktop cannot silently fall back to absent standalone Node')
 preparePayload(root,payload,[['arm64',executable],['x64',executable]])
 for(const [arch,hardware] of [['arm64','arm64'],['x64','x86_64']]) {
  fs.writeFileSync(log,'')
  result=spawnSync('/bin/bash',['-c',script,'test',helper,wrapper,log,'browser-server',literal],{env:{...process.env,BRAINANA_DESKTOP_REMOTE:'0',BRAINANA_DESKTOP_EXECUTABLE:executable,HARDWARE_ARCH:hardware,RES:path.join(payload,'Contents/Resources')},encoding:'utf8'})
  assert.equal(result.status,0,result.stderr)
  assert.ok(fs.readFileSync(wrapper,'utf8').includes('darwin-'+arch+'/node'))
  result=spawnSync('/bin/bash',[wrapper],{env:{PATH:process.env.PATH},encoding:'utf8'});assert.equal(result.status,0,result.stderr)
  assert.equal(fs.readFileSync(log,'utf8'),'unset\nbrowser-server\n'+literal+'\n')
 }
 // Execute the real workstation adapter with process/SSH adapters; cancellation
 // after authentication must leave the trusted embedded executable in the handoff.
 let spawned
 const context=vm.createContext({process:{resourcesPath:path.join(scratch,'App with spaces/Contents/Resources'),execPath:executable,env:{PATH:process.env.PATH}},console,AbortSignal})
 const imports={}
 for(const spec of ['node:fs/promises','node:os','node:path','node:util','node:child_process']) {
  const exports={...await import(spec)}
  if(spec==='node:child_process')exports.spawn=(command,args,options)=>{spawned={command,args,options};const child=new EventEmitter();child.stderr=new EventEmitter();queueMicrotask(()=>child.emit('close',0));return child}
  imports[spec]=new vm.SyntheticModule(Object.keys(exports),function(){for(const key of Object.keys(exports))this.setExport(key,exports[key])},{context})
 }
 imports['@brainana/core-launcher/launch.mjs']=new vm.SyntheticModule(['validateHandshake'],function(){this.setExport('validateHandshake',()=>assert.fail('No handoff was created'))},{context})
 imports['../version.mjs']=new vm.SyntheticModule(['VERSION','BUILD_ID'],function(){this.setExport('VERSION','test');this.setExport('BUILD_ID','test')},{context})
 const adapter=new vm.SourceTextModule(fs.readFileSync(path.join(root,'apps/align/desktop/workstation.mjs'),'utf8'),{context})
 await adapter.link(spec=>imports[spec]);await adapter.evaluate()
 assert.equal(await adapter.namespace.connectWorkstation(),null)
 assert.equal(spawned.options.env.BRAINANA_DESKTOP_EXECUTABLE,executable)
 assert.equal(spawned.options.env.BRAINANA_DESKTOP_REMOTE,'1')
 assert.ok(spawned.args[0].endsWith('workstation-support/Contents/MacOS/brainana-align-launcher'))
 console.log('Slim payload isolation, desktop fail-closed selection, launchd Electron environment, literal quoting, and both browser architectures passed')
} finally {fs.rmSync(scratch,{recursive:true,force:true})}
