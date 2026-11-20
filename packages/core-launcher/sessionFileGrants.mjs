import fs from 'node:fs/promises'
import path from 'node:path'
import crypto from 'node:crypto'

/** Main-process-only grants. There is deliberately no HTTP registration action.
 * The owned server can read only these exact immutable file identities, including
 * selected files outside its ordinary browsing root (mounted shares, for example). */
export function sessionFileGrants(filename){
 const records={};let queue=Promise.resolve()
 return paths=>{
  const job=queue.catch(()=>{}).then(async()=>{
   if(!Array.isArray(paths)||!paths.length||paths.length>128)throw Error('Invalid image file selection')
   const next={...records},descriptors=[]
   for(const input of paths){
    if(typeof input!=='string'||input.includes('\0')||!path.isAbsolute(input))throw Error('Invalid image location')
    const canonical=await fs.realpath(input),stat=await fs.stat(canonical)
    if(!stat.isFile()||!Number.isSafeInteger(stat.size)||stat.size<1)throw Error('The image location is not a readable file')
    let id=Object.keys(next).find(id=>{const r=next[id];return r.path===canonical&&r.size===stat.size&&r.modified===stat.mtimeMs&&r.ino===stat.ino&&r.dev===stat.dev})
    if(!id){if(Object.keys(next).length>=512)throw Error('Too many open image sources. Save the session and reopen Align.');id=crypto.randomBytes(24).toString('hex');next[id]={path:canonical,size:stat.size,modified:stat.mtimeMs,ino:stat.ino,dev:stat.dev}}
    descriptors.push({id,path:input,name:path.basename(input),size:stat.size,lastModified:stat.mtimeMs})
   }
   const encoded=JSON.stringify(next);if(Buffer.byteLength(encoded)>2*1024**2)throw Error('Too many image locations. Save the session and reopen Align.')
   await fs.writeFile(filename+'.tmp',encoded,{mode:0o600});await fs.rename(filename+'.tmp',filename);Object.assign(records,next)
   return descriptors
  });queue=job;return job
 }
}
