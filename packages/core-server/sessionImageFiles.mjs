import fs from 'node:fs'

/** Called only after the normal same-origin/token authentication. Never accepts
 * paths from an HTTP caller: grants are written by the owning desktop process. */
export async function serveSessionImage(req,res,url,grantFile){
 if(url.pathname!=='/api/session-image')return false
 const fail=(status,message)=>{res.writeHead(status,{'content-type':'application/json','cache-control':'no-store'});res.end(JSON.stringify({error:message}));return true}
 if(req.method!=='GET')return fail(405,'Method not allowed')
 const id=url.searchParams.get('file')
 if(!grantFile||!id||!/^[a-f0-9]{48}$/.test(id))return fail(404,'Original image not available')
 let record
 try{const stat=await fs.promises.stat(grantFile);if(stat.size>2*1024**2)return fail(500,'Invalid image grant registry');record=JSON.parse(await fs.promises.readFile(grantFile,'utf8'))[id]}catch{return fail(404,'Original image not available')}
 if(!record)return fail(404,'Original image not available')
 let file
 try{file=await fs.promises.open(record.path,'r')}catch{return fail(409,'Original image moved or became unreadable')}
 try{
  const stat=await file.stat()
  if(req.aborted||res.destroyed)return true
  if(!stat.isFile()||stat.size!==record.size||stat.mtimeMs!==record.modified||stat.ino!==record.ino||stat.dev!==record.dev)return fail(409,'Original image changed on disk')
  const range=req.headers.range;let start=0,end=stat.size-1
  if(range){const match=/^bytes=(\d+)-(\d*)$/.exec(range);if(!match)return fail(416,'Invalid image byte range');start=Number(match[1]);end=match[2]?Number(match[2]):end}
  if(!Number.isSafeInteger(start)||!Number.isSafeInteger(end)||start<0||end<start||end>=stat.size)return fail(416,'Image byte range is outside the file')
  res.writeHead(range?206:200,{'content-type':'application/octet-stream','content-length':end-start+1,'cache-control':'no-store','accept-ranges':'bytes','x-content-type-options':'nosniff',...(range?{'content-range':`bytes ${start}-${end}/${stat.size}`}:{})})
  const stream=file.createReadStream({start,end,autoClose:true});file=undefined
  stream.on('error',error=>res.destroy(error));res.once('close',()=>stream.destroy());stream.pipe(res)
  return true
 }finally{await file?.close().catch(()=>{})}
}
