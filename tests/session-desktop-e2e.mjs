/** Run with Electron. Actual sandboxed preload/IPC, browser Files, server ranges,
 * compiled frontend and workers; native picker choices alone are scripted. */
import {app,BrowserWindow,utilityProcess,dialog} from 'electron'
import fs from 'node:fs/promises'
import sync from 'node:fs'
import path from 'node:path'
import os from 'node:os'
import assert from 'node:assert/strict'
import {fileURLToPath} from 'node:url'
import * as nifti from 'nifti-reader-js'
import {gunzipSync} from 'fflate'
import {createSessionFixture} from './helpers/session-fixture.mjs'
import {bootServer} from '../packages/core-launcher/launch.mjs'
import {installExportSaves} from '../packages/core-desktop/exportSaves.mjs'
import {installSessionFiles} from '../apps/align/desktop/sessionFiles.mjs'
import {installPreferenceStorage} from '../packages/core-desktop/preferences.mjs'
import {installDataSourceActions} from '../packages/core-desktop/dataSources.mjs'
import {installJipActions} from '../apps/align/desktop/jipActions.mjs'

const root=path.resolve(path.dirname(fileURLToPath(import.meta.url)),'..'),scratch=sync.mkdtempSync(path.join(os.tmpdir(),'align-native-session-'))
app.setPath('userData',scratch);app.setPath('sessionData',scratch);app.commandLine.appendSwitch('use-angle','swiftshader');app.commandLine.appendSwitch('enable-unsafe-swiftshader')
let boot,win,exitStatus=0;const errors=[];const watchdog=setTimeout(()=>{console.error('Session desktop verification timed out');app.exit(2)},90000)
async function verify(){
try{
 await app.whenReady()
 await fs.rm(path.join(root,'artifacts/validation/session-desktop.json'),{force:true})
 const folder=path.join(scratch,'original data'),{dims,matrix,mriAffine,values,mri,ct,payload,session}=await createSessionFixture(folder)
 const meta=JSON.parse(await fs.readFile(path.join(root,'VERSION.json'),'utf8'))
 boot=await bootServer({serverPath:path.join(root,'apps/align/server.mjs'),root:scratch,cache:path.join(scratch,'cache'),version:meta.version,buildId:meta.buildId,fork:(file,args)=>utilityProcess.fork(file,args,{stdio:'pipe'})})
 win=new BrowserWindow({show:false,width:1540,height:980,webPreferences:{preload:path.join(root,'packages/core-desktop/preload.cjs'),sandbox:true,contextIsolation:true,nodeIntegration:false,webSecurity:true}})
 win.webContents.on('console-message',(_event,_level,message)=>{if(/Uncaught|Error invoking remote|preload.*error/i.test(message))errors.push(message)})
 installPreferenceStorage({directory:scratch,window:()=>win});installExportSaves({directory:scratch,window:()=>win});installSessionFiles({window:()=>win,fileAccess:()=>boot.grantFiles});installDataSourceActions({window:()=>win,canConnect:false,connect:async()=>{}});installJipActions({window:()=>win})
 await win.loadURL(boot.url);win.webContents.debugger.attach('1.3')
 const js=expression=>win.webContents.executeJavaScript(expression)
 async function wait(expression,message){for(let i=0;i<1200;i++){if(await js(expression))return;await new Promise(r=>setTimeout(r,20))}throw Error(message+'; status='+await js('document.querySelector("#status")?.textContent'))}
 async function choose(selector,files){const {root}=await win.webContents.debugger.sendCommand('DOM.getDocument');const {nodeId}=await win.webContents.debugger.sendCommand('DOM.querySelector',{nodeId:root.nodeId,selector});assert(nodeId,selector);await win.webContents.debugger.sendCommand('DOM.setFileInputFiles',{nodeId,files})}
 async function reload(){await new Promise(resolve=>{win.webContents.once('did-finish-load',resolve);win.webContents.reloadIgnoringCache()});await wait('Boolean(document.querySelector("#welcome-screen:not(.hidden)"))','Fresh welcome must appear')}
 async function open(){await js('document.querySelector("#status").textContent=""');await choose('#load-session',[session]);await wait('/Full session restored\\./.test(document.querySelector("#status").textContent)','Full native session must restore')}
 await wait('Boolean(document.querySelector("#welcome-session"))','Welcome must appear')
 assert(await js('!document.querySelector("#welcome-screen").classList.contains("hidden")'));assert(await js('document.querySelector("#save-session").disabled'))
 assert(await js('Boolean(document.createElement("canvas").getContext("webgl2"))'),'Software WebGL2 must be available for the actual compiled viewer')
 await js('const probe=document.createElement("input");probe.type="file";probe.id="native-file-probe";document.body.append(probe)');await choose('#native-file-probe',[mri])
 assert.equal(await js('window.brainanaSessionFiles.path(document.querySelector("#native-file-probe").files[0])'),mri,'The real sandboxed preload must extract native paths from genuine browser Files')
 assert.equal(await js('window.brainanaSessionFiles.path(new File(["x"],"/etc/passwd"))'),null,'A constructed File cannot forge a native path')
 await js('document.querySelector("#native-file-probe").remove()')
 let pickers=0,choices=[]
 dialog.showOpenDialog=async(_win,options)=>{pickers++;assert(options.title.includes('Locate'));return choices.shift()??{canceled:true,filePaths:[]}}
 await open();assert.equal(pickers,0,'Old sessions next to images restore from welcome without manual image loading')
 assert(await js('document.querySelector("#welcome-screen").classList.contains("hidden")'))
 assert(await js('document.querySelector("#mri-name").textContent==="anatomy.nii"&&document.querySelector("#ct-name").textContent==="ct.nii"'))
 // Actual compiled Save buttons use the real sandboxed bridge and Chromium
 // disk download. Only the native dialog's returned choice is scripted.
 let saveDialogs=0,saveChoice=path.join(scratch,'renamed native output.nii.gz')
 dialog.showSaveDialog=async(_win,options)=>{saveDialogs++;assert(options.title.includes('filename and folder'));return saveChoice?{canceled:false,filePath:saveChoice}:{canceled:true}}
 await js('document.querySelector("#export-open").click();document.querySelector("#save-aligned").click()')
 await wait('/Aligned image saved/.test(document.querySelector("#status").textContent)','Combined native save must complete')
 assert.equal(saveDialogs,1,'The volume and metadata use one combined native Save dialog')
 const directBytes=gunzipSync(await fs.readFile(saveChoice)),directHeader=nifti.readHeader(directBytes.buffer),directValues=new Float32Array(nifti.readImage(directHeader,directBytes.buffer))
 assert.deepEqual(directValues,values)
 assert.equal(JSON.parse(await fs.readFile(path.join(scratch,'renamed native output.json'),'utf8')).kind,'transformed-moving-image')
 const nativeSession=path.join(scratch,'my named session.json');saveChoice=nativeSession
 await js('document.querySelector("#save-session-export").click()');await wait('/Session saved/.test(document.querySelector("#status").textContent)','Session save must complete')
 assert.equal(JSON.parse(await fs.readFile(nativeSession,'utf8')).landmarks.length,5)
 saveChoice='';await js('document.querySelector("#save-aligned").click()');await wait('/Save cancelled/.test(document.querySelector("#status").textContent)','Cancelling native Save must propagate')
 assert.equal(saveDialogs,3)
 await js('document.querySelector("#export-close").click()')
 await js('window.__saved=[];window.brainanaAlignSaveBlob=async(blob,filename)=>window.__saved.push({filename,bytes:Array.from(new Uint8Array(await blob.arrayBuffer()))});document.querySelector("#save-session").click()')
 await wait('window.__saved.length===1','Restored session must save');let saved=JSON.parse(Buffer.from((await js('window.__saved[0]')).bytes).toString())
 assert.deepEqual(saved.fit.matrix,matrix);assert.equal(saved.landmarks.length,5);assert.equal(saved.schemaVersion,14);assert.deepEqual(saved.images.mri.sourceLocations,[mri]);assert.deepEqual(saved.images.ct.sourceLocations,[ct]);assert.deepEqual(saved.images.mri.workingDims,[9,10,11]);assert.deepEqual(saved.optimizationWindows,payload.optimizationWindows)
 await js('window.__saved=[];document.querySelector("#save-aligned").click()');await wait('window.__saved.some(s=>s.filename.endsWith(".nii.gz"))','Native export after reopening must finish')
 const exported=await js('window.__saved.find(s=>s.filename.endsWith(".nii.gz"))'),bytes=gunzipSync(Uint8Array.from(exported.bytes)),header=nifti.readHeader(bytes.buffer),output=new Float32Array(nifti.readImage(header,bytes.buffer));assert.deepEqual(header.dims.slice(1,4),dims);assert.deepEqual(header.affine,mriAffine);assert.deepEqual(output,values,'Native re-export must read original pixels through the reopened disk-backed source')
 const moved=path.join(folder,'renamed CT.nii');await fs.rename(ct,moved);choices=[{canceled:false,filePaths:[moved]}];await reload();await open();assert.equal(pickers,1)
 await js('window.__saved=[];window.brainanaAlignSaveBlob=async(blob,filename)=>window.__saved.push({filename,text:await blob.text()});document.querySelector("#save-session").click()');await wait('window.__saved.length===1','Relinked session must save');saved=JSON.parse(await js('window.__saved[0].text'));assert.deepEqual(saved.images.ct.sourceLocations,[moved]);assert.equal(saved.images.ct.name,'renamed CT.nii');assert.deepEqual(saved.fit.matrix,matrix)
 choices=[{canceled:true,filePaths:[]}];await reload();await choose('#load-session',[session]);await wait('/Session opening cancelled/.test(document.querySelector("#status").textContent)','Cancelling native relinking must finish');assert(await js('!document.querySelector("#welcome-screen").classList.contains("hidden")'));assert(await js('document.querySelectorAll(".landmark-row").length===0'));assert(await js('document.querySelector("#alignment-progress").classList.contains("hidden")'))
 const report={status:'passed',version:meta.version,electron:process.versions.electron,realSandboxedPreload:true,combinedNativeFilenameFolderSave:true,oneDialogForVolumeAndRenamedMetadata:true,actualChromiumDiskWritesAndExactVoxels:true,nativeSaveCancelAndNamedSession:true,genuineBrowserFilePaths:true,fabricatedFilePathsRejected:true,oldSessionOpenedFromWelcome:true,workingGridAndFiveLandmarksRestored:true,movedImageRelinkedAndResaved:true,cancelPreservesWelcome:true,fullNativeReExportPixelsExact:true,picker:'Native chooser invocation tested; returned selections scripted',rendering:'Actual compiled viewer using software WebGL2; not physical GPU certification'}
 const outputPath=path.join(root,'artifacts/validation/session-desktop.json');await fs.mkdir(path.dirname(outputPath),{recursive:true});assert.deepEqual(errors,[]);await fs.writeFile(outputPath,JSON.stringify(report,null,2)+'\n');console.log(JSON.stringify(report,null,2))
}catch(error){console.error(error);console.error(errors);exitStatus=1}
finally{clearTimeout(watchdog);win?.destroy();await boot?.stop();await fs.rm(scratch,{recursive:true,force:true});app.exit(exitStatus)}

}
void verify().catch(error=>{console.error(error);app.exit(1)})
