import {app,ipcMain} from 'electron'
import fs from 'node:fs/promises'
import path from 'node:path'
import {fileURLToPath} from 'node:url'
import {spawn} from 'node:child_process'
import {createJipService,scientificEnvironment} from './jipService.mjs'
export function embeddedRuntimeFolder(executable,platform=process.platform,resources=process.resourcesPath){
 return path.join(resources??(platform==='darwin'?path.resolve(path.dirname(executable),'../Resources'):path.join(path.dirname(executable),'resources')),'scientific-runtime')
}
export async function installJipActions({window}){
 // App-owned resources only: no picker, saved path, sibling discovery or PATH search.
 const folder=app.isPackaged===false?path.resolve(path.dirname(fileURLToPath(import.meta.url)),'../../../generated/scientific-runtime'):embeddedRuntimeFolder(app.getPath('exe'))
 const file=path.join(folder,process.platform==='win32'?'BrainanaAlignRegistration.exe':'BrainanaAlignRegistration')
 let probe=null,pending=null
 async function runtime(){
  if(probe)return {file,features:probe.features??[]}
  if(pending)return pending
  pending=(async()=>{
   try{const [root,actual]=await Promise.all([fs.realpath(folder),fs.realpath(file)]);if(!actual.startsWith(root+path.sep))return null}catch{return null}
   const info=await new Promise(resolve=>{let out='',done=false;const finish=value=>{if(done)return;done=true;clearTimeout(timer);resolve(value)},child=spawn(file,['--align-capabilities'],{stdio:['ignore','pipe','ignore'],windowsHide:true,env:scientificEnvironment()}),timer=setTimeout(()=>{child.kill();finish(null)},10000)
    child.stdout.on('data',c=>{out+=c.toString();if(out.length>8192){child.kill();finish(null)}});child.on('error',()=>finish(null));child.on('close',code=>{try{const value=JSON.parse(out);finish(code===0&&value.runtime==='brainana-align-registration'&&value.protocol===1&&value.actions?.includes('register')&&value.actions?.includes('itk-register')?value:null)}catch{finish(null)}})})
   if(info)probe=info
   return info?{file,features:info.features??[]}:null
  })()
  try{return await pending}finally{pending=null}
 }
 const service=createJipService({runtime});app.on('before-quit',()=>{void service.dispose()});app.on('browser-window-created',(_event,win)=>win.on('closed',()=>{void service.cancel()}))
 ipcMain.handle('brainana-scientific',async(event,action,request)=>{
  const win=window();if(!win||event.sender!==win.webContents||event.senderFrame!==win.webContents.mainFrame)throw Error('Scientific action is restricted to the main application window')
  if(action==='status'){const command=await runtime();return {available:!!command,embedded:true,version:probe?.version??null,features:probe?.features??[],busy:service.running()}}
  if(action==='run')return service.run(request)
  if(action==='poll')return service.poll()
  if(action==='cancel')return service.cancel()
  throw Error('Unknown scientific action')
 })
}
