import {ipcMain,dialog} from 'electron'
import path from 'node:path'
import fs from 'node:fs/promises'
import {readSessionHeader,findSessionSources,supportedImage,IMAGE_EXTENSIONS} from './sessionSourcePaths.mjs'

const labels={'mri-ct':['MRI','CT'],'t1-t2':['T1','T2'],'t1-t1':['T1 · 1','T1 · 2'],'mri-epi':['MRI','EPI']}
async function existingFolder(filename){let folder=path.dirname(filename);while(true){try{if((await fs.stat(folder)).isDirectory())return folder}catch{}const next=path.dirname(folder);if(next===folder)return undefined;folder=next}}
export function installSessionFiles({window,fileAccess}){
 ipcMain.handle('align-session-files',async(event,action,request)=>{
  const win=window()
  if(!win||event.sender!==win.webContents||event.senderFrame!==win.webContents.mainFrame)throw Error('Untrusted session-file sender')
  if(!['open','choose'].includes(action))throw Error('Unknown session-file action')
  if(!fileAccess())return {unavailable:true}
  const choose=async(signature,label,problem='')=>{
   const previous=signature?.sourceLocations?.find(p=>typeof p==='string')
   const result=await dialog.showOpenDialog(win,{title:`Locate ${label} image for saved session`,message:`${problem?String(problem).slice(0,4096)+'\n\n':''}Select the original ${label} input${signature?.name?': '+signature.name:'.'}`,defaultPath:previous?await existingFolder(previous):undefined,properties:['openFile',...(signature?.sourceFiles?.length>1?['multiSelections']:[])],filters:[{name:'Image volumes',extensions:[...new Set(IMAGE_EXTENSIONS.map(e=>e.split('.').at(-1)))]}]})
   if(result.canceled||!result.filePaths.length)return null
   if(result.filePaths.some(p=>!supportedImage(p)))throw Error('Select a supported original image file')
   return fileAccess()(result.filePaths)
  }
  if(action==='choose')return choose(request?.signature,String(request?.label??'original').slice(0,80),request?.problem)
  if(action!=='open')throw Error('Unknown session-file action')
  if(!Array.isArray(request?.wanted)||request.wanted.some(m=>!['mri','ct'].includes(m))||new Set(request.wanted).size!==request.wanted.length)throw Error('Invalid session image slots')
  const header=await readSessionHeader(request.path),result={},names=labels[header.registrationPairing]??labels['mri-ct']
  for(const m of request.wanted){
   const signature=header.images?.[m];if(!signature){result[m]=[];continue}
   const found=await findSessionSources(request.path,signature)
   result[m]=found?await fileAccess()(found):await choose(signature,names[m==='mri'?0:1])
   if(result[m]===null)return null
  }
  return result
 })
}
