import os from 'node:os'
import fs from 'node:fs/promises'
import path from 'node:path'
import {ipcMain,dialog} from 'electron'
export function installPreferenceStorage({directory,window}) {
  const keys=new Set(['preferences','presets','guide','recovery','recent','export','layout','window'])
  const target=key=>{if(!keys.has(key))throw new Error('Unknown preference key');return path.join(directory,`align-${key}.json`)}
  const verify=event=>{const win=window();if(!win||event.sender!==win.webContents||event.senderFrame!==win.webContents.mainFrame)throw new Error('Untrusted preferences sender')}
  let writes=Promise.resolve()
  const save=(key,value)=>{
    const file=target(key),data=JSON.stringify(value)
    if(typeof data!=='string'||Buffer.byteLength(data,'utf8')>2000000)throw new Error('Preference data is too large or invalid')
    const write=writes.catch(()=>{}).then(async()=>{await fs.mkdir(directory,{recursive:true});await fs.writeFile(file+'.tmp',data,{mode:0o600});await fs.rename(file+'.tmp',file)})
    writes=write;return write
  }
  ipcMain.handle('align-system-memory',async event=>{verify(event);const available=typeof process!=='undefined'&&typeof process.availableMemory==='function'?process.availableMemory():os.freemem();return {total:os.totalmem(),available:Math.min(os.totalmem(),available),sampledAt:Date.now(),source:'OS available-memory estimate (conservative)'}})
  ipcMain.handle('align-preferences-read',async(event,key)=>{verify(event);try{return JSON.parse(await fs.readFile(target(key),'utf8'))}catch(error){if(error.code==='ENOENT'||error.name==='SyntaxError')return null;throw error}})
  ipcMain.handle('align-preferences-write',async(event,key,value)=>{verify(event);await save(key,value);if(key==='window'&&value===null)window().preferenceWindowReset=true})
  ipcMain.handle('align-preferences-export-folder',async(event)=>{verify(event);const result=await dialog.showOpenDialog(window(),{title:'Default export folder',properties:['openDirectory','createDirectory']});if(result.canceled||!result.filePaths[0])return null;await save('export',{folder:result.filePaths[0]});return result.filePaths[0]})
}
