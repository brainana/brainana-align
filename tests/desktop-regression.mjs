import assert from 'node:assert/strict'
import fs from 'node:fs'
import fsp from 'node:fs/promises'
import os from 'node:os'
import path from 'node:path'
import {fileURLToPath} from 'node:url'
import vm from 'node:vm'
import {EventEmitter} from 'node:events'
import {bootServer,validateHandshake} from '../packages/core-launcher/launch.mjs'
import {isInternalUrl,isExternalWebUrl,WINDOW_PREFERENCES} from '../packages/core-desktop/navigation.mjs'
const root=path.resolve(path.dirname(fileURLToPath(import.meta.url)),'..')
const base='http://127.0.0.1:8765/#session='+ 'a'.repeat(64)
for(const target of ['http://127.0.0.1:8765/','http://127.0.0.1:8765/api/config'])assert.ok(isInternalUrl(base,target))
for(const target of ['http://127.0.0.1:8766/','https://127.0.0.1:8765/','file:///etc/passwd','javascript:alert(1)','data:text/html,x','https://example.org',null])assert.equal(isInternalUrl(base,target),false)
for(const target of ['file:///x','javascript:x','http://localhost:42','http://[::1]/'])assert.equal(isExternalWebUrl(target),false)
assert.ok(isExternalWebUrl('https://github.com/brainana/brainana-viewer'))
assert.deepEqual(WINDOW_PREFERENCES,{contextIsolation:true,nodeIntegration:false,sandbox:true,webSecurity:true})
for(const data of [{port:0,pid:1,sessionToken:'a'.repeat(64)},{port:80,pid:0,sessionToken:'a'.repeat(64)},{port:80,pid:1,sessionToken:'weak'}])assert.throws(()=>validateHandshake(data))
// Execute the real window module with a minimal Electron adapter, then exercise its event policies.
let options,opened=[],handlers={},permission,checkPermission,windowOpen,download
class FakeWindow {
 constructor(opts){options=opts;this.webContents={setWindowOpenHandler:fn=>windowOpen=fn,on:(event,fn)=>handlers[event]=fn,session:{setPermissionRequestHandler:fn=>permission=fn,setPermissionCheckHandler:fn=>checkPermission=fn,on:(event,fn)=>download=fn}}}
 once(){} on(){} getNormalBounds(){return {width:1540,height:980}} show(){} isDestroyed(){return false} loadURL(){return Promise.resolve()}
}
const context=vm.createContext({URL,console});const electron=new vm.SyntheticModule(['app','BrowserWindow','shell','dialog'],function(){this.setExport('app',{getPath:()=>os.tmpdir()});this.setExport('BrowserWindow',FakeWindow);this.setExport('shell',{openExternal:url=>opened.push(url)});this.setExport('dialog',{showMessageBox:()=>Promise.resolve({})})},{context})
const navigation=new vm.SourceTextModule(fs.readFileSync(path.join(root,'packages/core-desktop/navigation.mjs'),'utf8'),{context})
const windowModule=new vm.SourceTextModule(fs.readFileSync(path.join(root,'packages/core-desktop/window.mjs'),'utf8'),{context,initializeImportMeta(meta){meta.url=new URL('../packages/core-desktop/window.mjs',import.meta.url).href}})
const nodeModules={};for(const spec of ['node:path','node:url','node:fs']){const mod=await import(spec);nodeModules[spec]=new vm.SyntheticModule(Object.keys(mod),function(){for(const key of Object.keys(mod))this.setExport(key,mod[key])},{context})}
await windowModule.link(spec=>spec==='electron'?electron:nodeModules[spec]??navigation);await windowModule.evaluate()
const win=windowModule.namespace.createMainWindow(base,{appLabel:'Test'})
assert.equal(options.webPreferences.nodeIntegration,false);assert.equal(options.webPreferences.sandbox,true)
for(const target of ['https://example.org','file:///x','http://127.0.0.1:99/']) {let prevented=false;handlers['will-navigate']({preventDefault:()=>prevented=true},target);assert.ok(prevented)}
let prevented=false;handlers['will-navigate']({preventDefault:()=>prevented=true},base);assert.equal(prevented,false)
assert.equal(windowOpen({url:'https://example.org'}).action,'deny');assert.equal(opened.length,1)
windowOpen({url:'file:///x'});assert.equal(opened.length,1)
let granted;permission(null,'camera',value=>granted=value);assert.equal(granted,false);assert.equal(checkPermission(),false)
let saveOptions;download(null,{getFilename:()=>'alignment.json',setSaveDialogOptions:o=>saveOptions=o},win.webContents);assert.equal(saveOptions.defaultPath,'alignment.json')
// Execute the actual desktop lifecycle with adapters: focus, successful/failed switching, and quit ownership.
const appEvents=new EventEmitter();let quitCount=0,windowId=0,currentMenu,bootCount=0,stopCount=0,remoteStopped=0
const appMock=Object.assign(appEvents,{setName(){},setAboutPanelOptions(){},requestSingleInstanceLock:()=>true,quit:()=>quitCount++,whenReady:()=>Promise.resolve(),getPath:name=>{assert.equal(name,'userData','Desktop must request a supported Electron path');return os.tmpdir()}})
const windows=[]
function makeWindow() {const w={id:++windowId,destroyed:false,isDestroyed(){return this.destroyed},isMinimized:()=>true,restore:()=>w.restored=true,show:()=>w.shown=true,focus:()=>w.focused=true,destroy:()=>w.destroyed=true};windows.push(w);return w}
const lifeContext=vm.createContext({process:{argv:[]},console,AbortController,Promise,setTimeout})
const lifeElectron=new vm.SyntheticModule(['app','BrowserWindow','dialog','utilityProcess'],function(){this.setExport('app',appMock);this.setExport('BrowserWindow',{});this.setExport('dialog',{showMessageBox:()=>Promise.resolve({response:1})});this.setExport('utilityProcess',{})},{context:lifeContext})
const lifeWindow=new vm.SyntheticModule(['createMainWindow'],function(){this.setExport('createMainWindow',makeWindow)},{context:lifeContext})
const lifeMenu=new vm.SyntheticModule(['applyAppMenu'],function(){this.setExport('applyAppMenu',options=>currentMenu=options)},{context:lifeContext})
const lifeBoot=new vm.SyntheticModule(['bootServer'],function(){this.setExport('bootServer',async()=>({url:base,child:new EventEmitter(),stop:async()=>stopCount++,number:++bootCount}))},{context:lifeContext})
const lifeFs=new vm.SyntheticModule(['default'],function(){this.setExport('default',{})},{context:lifeContext})
const lifePreferences=new vm.SyntheticModule(['installPreferenceStorage'],function(){this.setExport('installPreferenceStorage',()=>{})},{context:lifeContext})
const lifeDataSources=new vm.SyntheticModule(['installDataSourceActions'],function(){this.setExport('installDataSourceActions',()=>{})},{context:lifeContext})
const lifeModule=new vm.SourceTextModule(fs.readFileSync(path.join(root,'packages/core-desktop/main.mjs'),'utf8'),{context:lifeContext})
await lifeModule.link(spec=>({'electron':lifeElectron,'node:fs/promises':lifeFs,'./preferences.mjs':lifePreferences,'./dataSources.mjs':lifeDataSources,'./window.mjs':lifeWindow,'./menu.mjs':lifeMenu,'@brainana/core-launcher/launch.mjs':lifeBoot})[spec]);await lifeModule.evaluate()
let remoteFails=false
await lifeModule.namespace.runDesktop({appLabel:'Test',version:'test',buildId:'test',serverPath:'test',connectRemote:async()=>{if(remoteFails)throw new Error('Cancelled connection');return {url:base,stop:async()=>remoteStopped++}}})
assert.equal(bootCount,1);appEvents.emit('second-instance');assert.ok(windows[0].restored&&windows[0].focused)
remoteFails=true;await currentMenu.connectRemote();assert.equal(windows.length,1);assert.equal(stopCount,0)
remoteFails=false;await currentMenu.connectRemote();assert.equal(windows.length,2);assert.ok(windows[0].destroyed);assert.equal(stopCount,1)
await currentMenu.returnLocal();assert.equal(bootCount,2);assert.equal(windows.length,3);assert.equal(remoteStopped,1)
let preventedQuit=false;appEvents.emit('before-quit',{preventDefault:()=>preventedQuit=true});await new Promise(resolve=>setTimeout(resolve,0))
assert.ok(preventedQuit);assert.equal(stopCount,2);assert.equal(quitCount,1)
appEvents.emit('before-quit',{preventDefault:()=>assert.fail('Repeated quit was blocked')});assert.equal(stopCount,2)

// Actual shared boot process, token authentication, production options, and server ownership cleanup.
const scratch=await fsp.mkdtemp(path.join(os.tmpdir(),'brainana-desktop-test-'))
try {
 const metadata=JSON.parse(fs.readFileSync(path.join(root,'VERSION.json'),'utf8'))
 const boot=await bootServer({serverPath:path.join(root,'apps/align/server.mjs'),root:scratch,cache:path.join(scratch,'cache'),version:metadata.version,buildId:metadata.buildId})
 assert.equal((await fetch(new URL('/api/config',boot.url))).status,401)
 const response=await fetch(new URL('/api/config',boot.url),{headers:{'x-brainana-session':boot.handshake.sessionToken}});assert.equal(response.status,200)
 const html=await (await fetch(boot.url)).text();const asset=html.match(/src="([^\"]+\.js)"/)[1]
 const js=await (await fetch(new URL(asset,boot.url))).text()
 for(const label of ['Rigid, 6 parameters','Similarity, 7 parameters','Anisotropic scale, 9 parameters','Affine, 12 parameters','guided-setup'])assert.ok(js.includes(label))
 const pid=boot.child.pid;await boot.stop();await boot.stop();assert.throws(()=>process.kill(pid,0))
 await assert.rejects(bootServer({serverPath:path.join(scratch,'missing.mjs'),root:scratch,timeoutMs:2000}))
}finally{await fsp.rm(scratch,{recursive:true,force:true})}
// Shared workspaces must never depend on Align entry points or application state.
for(const dir of fs.readdirSync(path.join(root,'packages')))for(const file of fs.readdirSync(path.join(root,'packages',dir)).filter(f=>/\.(mjs|ts)$/.test(f))) {
 const contents=fs.readFileSync(path.join(root,'packages',dir,file),'utf8')
 assert.doesNotMatch(contents,/(?:from|import\()\s*['"][^'"]*(?:apps\/|@brainana\/align)/,dir+'/'+file+' imports app domain')
}
console.log('Desktop boundaries, window security, native downloads, authenticated boot, shutdown and workspace purity passed')
