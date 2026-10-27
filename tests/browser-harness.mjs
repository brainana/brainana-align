/** Serve the complete emitted app so module chunks, workers, fonts and origins
 * behave as in a real release. This fixture exposes no host files/data APIs. */
import http from 'node:http'
import fs from 'node:fs/promises'
import path from 'node:path'
import {fileURLToPath} from 'node:url'
export async function startBrowserHarness(){
 const dist=path.resolve(path.dirname(fileURLToPath(import.meta.url)),'../apps/align/dist')
 const requests=[]
 const types={'.html':'text/html','.js':'text/javascript','.css':'text/css','.json':'application/json','.woff2':'font/woff2','.png':'image/png','.svg':'image/svg+xml'}
 const server=http.createServer((req,res)=>{void(async()=>{
  const url=new URL(req.url,'http://127.0.0.1')
  if(url.pathname==='/api/config'){
   requests.push({path:url.pathname,token:req.headers['x-brainana-session']})
   res.writeHead(200,{'content-type':'application/json'});res.end(JSON.stringify({enabled:true,mode:'local',label:'CI fixture'}));return
  }
  const file=path.resolve(dist,'.'+decodeURIComponent(url.pathname==='/'?'/index.html':url.pathname))
  if(!file.startsWith(dist+path.sep)){res.writeHead(403);res.end();return}
  const data=await fs.readFile(file);res.writeHead(200,{'content-type':types[path.extname(file)]??'application/octet-stream'});res.end(data)
 })().catch(()=>{res.writeHead(404);res.end()})})
 await new Promise((resolve,reject)=>{server.once('error',reject);server.listen(0,'127.0.0.1',resolve)})
 return {url:`http://127.0.0.1:${server.address().port}`,requests,close:()=>new Promise(resolve=>{server.closeAllConnections();server.close(resolve)})}
}
