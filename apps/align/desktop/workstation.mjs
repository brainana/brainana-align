// macOS-only adapter retaining the tested system-SSH interactive authentication workflow.
// The launcher returns a private handshake instead of opening an external browser.
import fs from 'node:fs/promises'
import os from 'node:os'
import path from 'node:path'
import {spawn,execFile} from 'node:child_process'
import {promisify} from 'node:util'
import {validateHandshake} from '@brainana/core-launcher/launch.mjs'
import {VERSION,BUILD_ID} from '../version.mjs'
const run=promisify(execFile)
export async function connectWorkstation({signal}={}) {
  const scratch=await fs.mkdtemp(path.join(os.tmpdir(),'brainana-workstation-'))
  await fs.chmod(scratch,0o700)
  const handoff=path.join(scratch,'connection.json')
  const launcher=path.join(process.resourcesPath,'workstation-support/Contents/MacOS/brainana-align-launcher')
  let data=null,stopped=false
  const stop=async()=>{
    if(stopped)return;stopped=true
    if(data) {
      // Only this connection's private instance can be stopped; browser sessions use a separate directory.
      if(/^org\.brainana\.align\.server\.[0-9.]+$/.test(data.launchLabel))await run('/bin/launchctl',['remove',data.launchLabel]).catch(()=>{})
      if(data.controlSocket && data.sshTarget)await run('/usr/bin/ssh',['-S',data.controlSocket,'-O','exit',data.sshTarget],{timeout:3000}).catch(()=>{})
    }
    await fs.rm(scratch,{recursive:true,force:true})
  }
  try {
    const child=spawn('/bin/bash',[launcher],{env:{...process.env,BRAINANA_DESKTOP_REMOTE:'1',BRAINANA_DESKTOP_EXECUTABLE:process.execPath,BRAINANA_DESKTOP_HANDOFF:handoff,BRAINANA_DESKTOP_INSTANCE_DIR:path.join(scratch,'instance')},stdio:['ignore','ignore','pipe'],signal})
    let errorText='';child.stderr.on('data',d=>{errorText=(errorText+d).slice(-2000)})
    const code=await new Promise((resolve,reject)=>{child.once('error',reject);child.once('close',resolve)})
    try {data=JSON.parse(await fs.readFile(handoff,'utf8'))}catch(error){if(error.code==='ENOENT' && code===0){await stop();return null}throw error}
    if(code!==0)throw new Error(errorText || 'Workstation authentication did not complete')
    const url=validateHandshake(data)
    if(data.version!==VERSION || data.buildId!==BUILD_ID || data.mode!=='remote')throw new Error('Workstation connection identity mismatch')
    const health=await fetch(new URL('/api/health',url),{headers:{'x-brainana-session':data.sessionToken},signal:AbortSignal.timeout(3000)})
    if(!health.ok)throw new Error('Workstation data server is unavailable')
    return {url,stop}
  } catch(error){
    // A handoff may have completed concurrently with cancellation.
    if(!data)try{data=JSON.parse(await fs.readFile(handoff,'utf8'))}catch{}
    await stop();throw error
  }
}
