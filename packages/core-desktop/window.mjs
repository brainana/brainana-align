import path from 'node:path'
import {fileURLToPath} from 'node:url'
import fs from 'node:fs'
import {app,BrowserWindow,shell,dialog} from 'electron'
import {isInternalUrl,isExternalWebUrl,WINDOW_PREFERENCES} from './navigation.mjs'
export function createMainWindow(url,{appLabel='Brainana',onFailure=()=>{}}={}) {
  const boundsFile=path.join(app.getPath('userData'),'align-window.json');let bounds={}
  try{const saved=JSON.parse(fs.readFileSync(boundsFile,'utf8'));if(Number.isFinite(saved.width)&&Number.isFinite(saved.height))bounds={width:Math.round(Math.max(1000,Math.min(3000,saved.width))),height:Math.round(Math.max(680,Math.min(2000,saved.height)))}}catch{}
  const win=new BrowserWindow({width:1540,height:980,...bounds,minWidth:1000,minHeight:680,title:appLabel,backgroundColor:'#14120d',show:false,autoHideMenuBar:true,webPreferences:{...WINDOW_PREFERENCES,preload:path.join(path.dirname(fileURLToPath(import.meta.url)),'preload.cjs')}})
  win.webContents.setWindowOpenHandler(({url:target})=>{
    if(isExternalWebUrl(target))void shell.openExternal(target)
    return {action:'deny'}
  })
  win.webContents.on('will-navigate',(event,target)=>{if(!isInternalUrl(url,target))event.preventDefault()})
  win.webContents.on('will-redirect',(event,target)=>{if(!isInternalUrl(url,target))event.preventDefault()})
  win.webContents.on('will-attach-webview',event=>event.preventDefault())
  win.webContents.session.setPermissionRequestHandler((_contents,_permission,callback)=>callback(false))
  win.webContents.session.setPermissionCheckHandler(()=>false)
  win.once('ready-to-show',()=>win.show())
  win.webContents.on('render-process-gone',(_event,details)=>{
    onFailure(details)
    if(!win.isDestroyed())void dialog.showMessageBox(win,{type:'error',message:'The display process stopped',detail:'Reopen Brainana Align to restore a saved session. Reason: '+details.reason})
  })
  const session=win.webContents.session
  const download=(_event,item,contents)=>{
    if(contents!==win.webContents)return
    let folder='';try{const saved=JSON.parse(fs.readFileSync(path.join(app.getPath('userData'),'align-export.json'),'utf8'));if(typeof saved.folder==='string'&&fs.statSync(saved.folder).isDirectory())folder=saved.folder}catch{}
    item.setSaveDialogOptions({title:'Save Brainana export',defaultPath:folder?path.join(folder,path.basename(item.getFilename())):item.getFilename()})
    item.once?.('done',(_event,state)=>{if(state==='completed')try{const file=path.join(app.getPath('userData'),'align-export.json');fs.writeFileSync(file+'.download-tmp',JSON.stringify({folder:path.dirname(item.getSavePath())}),{mode:0o600});fs.renameSync(file+'.download-tmp',file)}catch{}})
  }
  session.on('will-download',download)
  win.once('closed',()=>session.removeListener('will-download',download))
  win.on('resize',()=>{win.preferenceWindowReset=false})
  win.on('close',()=>{if(win.preferenceWindowReset)return;try{const {width,height}=win.getNormalBounds();fs.mkdirSync(path.dirname(boundsFile),{recursive:true});fs.writeFileSync(boundsFile,JSON.stringify({width,height}),{mode:0o600})}catch{}})
  win.loadURL(url).catch(error=>{
    if(!win.isDestroyed()){win.show();void dialog.showMessageBox(win,{type:'error',message:'Unable to open Brainana Align',detail:error.message})}
  })
  return win
}
