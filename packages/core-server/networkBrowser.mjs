// In-process SFTP browsing, adapted to Align from Brainana Viewer's current source flow.
// All calls pass through the server's session/Origin/method guards. Browse tokens stay in headers.
import fs from 'node:fs'
import os from 'node:os'
import path from 'node:path'
import crypto from 'node:crypto'
import {SftpClient} from './networkSftpClient.mjs'
export function readSshHosts() {
  let text
  try {
    text = fs.readFileSync(path.join(os.homedir(), '.ssh', 'config'), 'utf8')
  } catch {
    return [] // no config, or not readable
  }
  return parseSshConfig(text)
}

// Pure parser for ~/.ssh/config text (split out so it's testable without touching the
// filesystem or $HOME). Splits on LF or CRLF so Windows-authored configs parse identically.
export function parseSshConfig(text) {
  const hosts = []
  let current = null // { host, hostName, user, port } for the block being read
  const flush = () => {
    if (current) hosts.push(current)
    current = null
  }
  for (const rawLine of String(text).split(/\r?\n/)) {
    const line = rawLine.trim()
    if (!line || line.startsWith('#')) continue
    // Keyword and value split on whitespace or a single '=' (both are valid ssh_config syntax).
    const match = line.match(/^(\w+)\s*=?\s*(.*)$/)
    if (!match) continue
    const keyword = match[1].toLowerCase()
    const value = match[2].trim()
    if (keyword === 'host') {
      flush()
      // A Host line may list several patterns; take the first concrete (non-wildcard) alias.
      const alias = value.split(/\s+/).find((p) => p && !p.includes('*') && !p.includes('?'))
      if (alias) current = { host: alias }
      continue
    }
    if (!current) continue // a keyword before any Host block — ignore
    if (keyword === 'hostname') current.hostName = value
    else if (keyword === 'user') current.user = value
    else if (keyword === 'port') {
      const n = Number(value)
      if (Number.isFinite(n)) current.port = n
    }
  }
  flush()
  return hosts
}

const HEADER='x-brainana-remote-token'
export function createNetworkBrowser({readJson,json,isAllowed,Client=SftpClient,agent=process.env.SSH_AUTH_SOCK}) {
  const sessions=new Map();let pending=0,closed=false
  const clients=new Set()
  const safePath=value=>{if(typeof value!=='string'||value.length>4096||value.includes('\0'))throw Error('Invalid remote path');return value||'.'}
  const close=()=>{closed=true;for(const client of clients)void client.close().catch(()=>{});clients.clear();sessions.clear()}
  const timer=setInterval(()=>{for(const [token,s]of sessions)if(Date.now()-s.used>20*60*1000){sessions.delete(token);clients.delete(s.client);void s.client.close().catch(()=>{})}},60000);timer.unref?.()
  return {
    close(){clearInterval(timer);close()},
    async handle(req,res,url){
      if(!url.pathname.startsWith('/api/network/'))return false
      const action=url.pathname.slice('/api/network/'.length)
      if(action==='profiles'&&req.method==='GET'){json(res,200,readSshHosts());return true}
      const expected=action==='connect'||action==='disconnect'?'POST':'GET'
      if(req.method!==expected){json(res,405,{error:'Method not allowed'});return true}
      if(action==='connect') {
        if(closed||sessions.size+pending>=4)throw Error('Close another network connection before opening a new one')
        pending++
        let client
        try {
          const b=await readJson(req)
          if(typeof b.host!=='string'||!b.host.trim()||b.host.length>253||/\s|\0/.test(b.host)||typeof b.username!=='string'||!b.username.trim()||b.username.length>256)throw Error('Enter a valid hostname and username')
          const profile=readSshHosts().find(p=>p.host===b.host.trim())
          if(profile?.hostName)b.host=profile.hostName
          const port=Number(b.port||profile?.port||22);if(!Number.isInteger(port)||port<1||port>65535)throw Error('Invalid SSH port')
          for(const k of ['password','passphrase','identityFile'])if(b[k]!==undefined&&(typeof b[k]!=='string'||b[k].length>4096))throw Error('Invalid authentication field')
          let privateKey
          if(b.identityFile){const file=b.identityFile.startsWith('~/')?path.join(os.homedir(),b.identityFile.slice(2)):b.identityFile;const stat=fs.statSync(file);if(!stat.isFile()||stat.size>65536)throw Error('Invalid private key file');privateKey=fs.readFileSync(file)}
          client=new Client({host:b.host.trim(),port,username:b.username.trim(),password:b.password||undefined,privateKey,passphrase:b.passphrase||undefined,agent:privateKey||b.password?undefined:agent})
          clients.add(client)
          await client.connect()
          if(closed||req.aborted||res.destroyed)throw Error('Connection cancelled')
          const directory=await client.realpath(safePath(b.path||'.'))
          const token=crypto.randomBytes(32).toString('hex');sessions.set(token,{client,used:Date.now()})
          json(res,200,{token,path:directory,label:`${b.username.trim()}@${b.host.trim()}`})
        } catch(error){if(client){clients.delete(client);await client.close().catch(()=>{})}throw error}
        finally {pending--}
        return true
      }
      const token=req.headers[HEADER];const session=typeof token==='string'?sessions.get(token):null
      if(!session){json(res,401,{error:'Network connection expired or disconnected. Connect again.'});return true}
      session.used=Date.now()
      if(action==='disconnect'){sessions.delete(token);clients.delete(session.client);await session.client.close();json(res,200,{ok:true});return true}
      const filename=safePath(url.searchParams.get('path')||'.')
      if(action==='list') {
        const directory=await session.client.realpath(filename)
        const entries=(await session.client.list(directory)).filter(e=>e.name!=='.'&&e.name!=='..'&&!e.name.includes('/')&&(e.type==='directory'||e.type==='file'&&isAllowed(e.name))).map(e=>({name:e.name,path:path.posix.join(directory,e.name),directory:e.type==='directory'})).sort((a,b)=>Number(b.directory)-Number(a.directory)||a.name.localeCompare(b.name))
        json(res,200,{path:directory,parent:directory==='/'?null:path.posix.dirname(directory),entries});return true
      }
      if(action==='file') {
        if(!isAllowed(filename)){json(res,415,{error:'Unsupported image type'});return true}
        const stat=await session.client.stat(filename)
        if(!stat.isFile||!Number.isSafeInteger(stat.size)||stat.size<1||stat.size>1_500_000_000)throw Error('Select a supported image file smaller than 1.5 GB')
        const stream=session.client.createReadStream(filename)
        stream.on('error',error=>{if(!res.headersSent)json(res,400,{error:error.message});else res.destroy()})
        res.on('close',()=>stream.destroy())
        res.writeHead(200,{'Content-Type':'application/octet-stream','Content-Length':stat.size,'Cache-Control':'no-store'});stream.pipe(res);return true
      }
      json(res,404,{error:'Unknown network operation'});return true
    },
  }
}
