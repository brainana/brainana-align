// Shared process lifecycle. The caller supplies an executable or Electron utility-process factory.
import fs from 'node:fs/promises'
import os from 'node:os'
import path from 'node:path'
import { spawn } from 'node:child_process'

export function validateHandshake(data) {
  if (!data || !Number.isInteger(data.port) || data.port < 1 || data.port > 65535 || !Number.isInteger(data.pid) || data.pid < 1 || !/^[a-f0-9]{64,}$/i.test(data.sessionToken)) throw new Error('Invalid server startup handshake')
  return `http://127.0.0.1:${data.port}/#session=${data.sessionToken}`
}

export async function bootServer({serverPath, root=os.homedir(), cache, version, buildId, fork, timeoutMs=30000, serverArgs=[]}) {
  const scratch=await fs.mkdtemp(path.join(os.tmpdir(),'brainana-desktop-'))
  await fs.chmod(scratch,0o700)
  const handshake=path.join(scratch,'handshake.json'),errorFile=path.join(scratch,'error')
  const args=['--host','127.0.0.1','--port','0','--root',root,'--cache',cache || path.join(scratch,'cache'),'--handshake-file',handshake,'--error-file',errorFile,...serverArgs]
  let child, exited=false, failure='', stopped=false
  const stop=async()=> {
    if(stopped)return
    stopped=true
    if(child && !exited) {
      const closed=new Promise(resolve=>child.once('exit',resolve))
      child.kill()
      await Promise.race([closed,new Promise(resolve=>setTimeout(resolve,2000))])
      if(!exited && child.pid) { try {process.kill(child.pid,'SIGKILL')}catch{} }
    }
    await fs.rm(scratch,{recursive:true,force:true})
  }
  try {
    child=fork ? fork(serverPath,args) : spawn(process.execPath,[serverPath,...args],{stdio:['ignore','pipe','pipe']})
    child.once('exit',()=>{exited=true})
    child.once('error',error=>{failure=error.message})
    child.stderr?.on('data',data=>{failure=(failure+data.toString()).slice(-2000)})
    const start=Date.now()
    while(Date.now()-start<timeoutMs) {
      if(exited)throw new Error(failure || 'The data server stopped during startup')
      try {
        const data=JSON.parse(await fs.readFile(handshake,'utf8'))
        const url=validateHandshake(data)
        if(child.pid!==data.pid || (version && data.version!==version) || (buildId && data.buildId!==buildId))throw new Error('Server startup identity mismatch')
        const response=await fetch(new URL('/api/health',url),{headers:{'x-brainana-session':data.sessionToken},signal:AbortSignal.timeout(2000)})
        if(!response.ok)throw new Error('Server startup health check failed')
        return {url,child,stop,handshake:data}
      } catch(error) {
        if(error.code!=='ENOENT')throw error
      }
      try {const message=await fs.readFile(errorFile,'utf8');throw new Error(message)}catch(error){if(error.code!=='ENOENT')throw error}
      await new Promise(resolve=>setTimeout(resolve,30))
    }
    throw new Error('The data server did not become ready within 30 seconds')
  } catch(error) {await stop();throw error}
}
