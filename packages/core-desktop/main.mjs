import {installDataSourceActions} from './dataSources.mjs'
import {app,BrowserWindow,dialog,utilityProcess} from 'electron'
import fs from 'node:fs/promises'
import {installPreferenceStorage} from './preferences.mjs'
import {createMainWindow} from './window.mjs'
import {applyAppMenu} from './menu.mjs'
import {bootServer} from '@brainana/core-launcher/launch.mjs'

export async function runDesktop(options) {
  const {appLabel,version,buildId,serverPath,connectRemote}=options
  app.setName(appLabel)
  app.setAboutPanelOptions({applicationName:appLabel,applicationVersion:version,version:buildId,copyright:'Brainana'})
  if(!app.requestSingleInstanceLock()){app.quit();return}
  let win=null,boot=null,quitting=false,switching=false,connectionAbort=null
  app.on('second-instance',()=>{if(win){if(win.isMinimized())win.restore();win.show();win.focus()}})
  app.on('window-all-closed',()=>{if(!switching)app.quit()})
  app.on('before-quit',event=>{
    if(quitting)return
    event.preventDefault();quitting=true;connectionAbort?.abort()
    Promise.resolve(boot?.stop()).finally(()=>app.quit())
  })
  const watch=owned=>owned.child?.once('exit',()=>{if(!quitting && !switching && boot===owned && win && !win.isDestroyed())void dialog.showMessageBox(win,{type:'error',message:'The local data server stopped',detail:'Save your session using the toolbar and reopen the application.'})})
  const local=()=>bootServer({serverPath,version,buildId,cache:app.getPath('userData')+'/cache/'+version,fork:(file,args)=>utilityProcess.fork(file,args,{stdio:'pipe',serviceName:appLabel+' data server'})})
  const switchTo=async(factory)=>{
    if(switching || quitting)return
    const result=await dialog.showMessageBox(win,{type:'question',message:'Change data connection?',detail:'Save your session first if you want to keep the current images, landmarks and registration.',buttons:['Cancel','Change connection'],defaultId:0,cancelId:0})
    if(result.response!==1)return
    switching=true;connectionAbort=new AbortController()
    let next
    try {
      next=await factory(connectionAbort.signal)
      if(!next)return
      if(quitting){await next.stop();return}
      const previous=boot,old=win
      boot=next;win=createMainWindow(boot.url,{appLabel});watch(boot);old.destroy();await previous.stop()
    } catch(error) {await next?.stop();await dialog.showMessageBox(win,{type:'error',message:'Connection could not be changed',detail:error.message})}
    finally {switching=false;connectionAbort=null}
  }
  try {
    await app.whenReady()
    installDataSourceActions({window:()=>win,canConnect:!!connectRemote,connect:()=>switchTo(signal=>connectRemote({window:win,signal}))})
    installPreferenceStorage({directory:app.getPath('userData'),window:()=>win})
    await options.installDomainActions?.({window:()=>win})
    boot=await local();win=createMainWindow(boot.url,{appLabel})
    watch(boot)
    applyAppMenu({appLabel,window:()=>win,connectRemote:connectRemote?()=>switchTo(signal=>connectRemote({window:win,signal})):null,returnLocal:()=>switchTo(local)})
    // Explicit release smoke mode, invoked by the packager. It records actual Chromium/WebGL readiness.
    const flag=process.argv.indexOf('--smoke-report')
    if(flag>=0 && process.argv[flag+1]) {
      win.webContents.once('did-finish-load',async()=>{
        try {
          await new Promise(resolve=>setTimeout(resolve,1200))
          const checks=await win.webContents.executeJavaScript(`(()=>{const gl=document.createElement('canvas').getContext('webgl2');const models=[...document.querySelectorAll('#landmark-transform option')].map(e=>e.textContent);document.getElementById('guided-setup')?.click();return {title:document.title,models,guide:!!document.querySelector('[role="dialog"]'),webgl2:!!gl,nodeAvailable:typeof window.require!=='undefined',userAgent:navigator.userAgent}})()`)
          await fs.writeFile(process.argv[flag+1],JSON.stringify({version,buildId,...checks},null,2)+'\n')
          const image=await win.webContents.capturePage();await fs.writeFile(process.argv[flag+1]+'.png',image.toPNG())
        }catch(error){await fs.writeFile(process.argv[flag+1],JSON.stringify({version,error:error.message}))}
        app.quit()
      })
    }
  } catch(error) {await dialog.showMessageBox({type:'error',message:appLabel+' could not start',detail:error.message});app.quit()}
}
