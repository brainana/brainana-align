import assert from 'node:assert/strict'
import fs from 'node:fs/promises'
import os from 'node:os'
import path from 'node:path'
import vm from 'node:vm'
const sourceRoot=process.argv[2]?path.resolve(process.argv[2]):new URL('../packages/core-desktop/',import.meta.url)
const sourceFile=name=>typeof sourceRoot==='string'?path.join(sourceRoot,name):new URL(name,sourceRoot)
const folder=await fs.mkdtemp(path.join(os.tmpdir(),'align-preferences-test-'))
const handlers=new Map(),frame={},webContents={mainFrame:frame},win={webContents}
const electron={ipcMain:{handle:(name,fn)=>handlers.set(name,fn)},dialog:{showOpenDialog:async()=>({canceled:false,filePaths:[folder]})}}
const context=vm.createContext({console,JSON,Set,Promise,Error,Buffer});const modules={}
for(const [name,values]of Object.entries({'electron':electron,'node:fs/promises':{default:fs},'node:path':{default:path},'node:os':{default:os}})){modules[name]=new vm.SyntheticModule(Object.keys(values),function(){for(const key of Object.keys(values))this.setExport(key,values[key])},{context})}
const source=new vm.SourceTextModule(await fs.readFile(sourceFile('preferences.mjs'),'utf8'),{context});await source.link(name=>modules[name]);await source.evaluate()
try{
 source.namespace.installPreferenceStorage({directory:folder,window:()=>win})
 const event={sender:webContents,senderFrame:frame};const memory=await handlers.get('align-system-memory')(event);assert(memory.total>0&&memory.available>0);await assert.rejects(handlers.get('align-system-memory')({sender:{},senderFrame:frame}),/Untrusted/);const read=handlers.get('align-preferences-read'),write=handlers.get('align-preferences-write')
 assert.equal(await read(event,'preferences'),null)
 await assert.rejects(read({sender:{},senderFrame:frame},'preferences'),/Untrusted/)
 await assert.rejects(read({sender:webContents,senderFrame:{}},'preferences'),/Untrusted/)
 await assert.rejects(write(event,'../../outside',{value:1}),/Unknown/)
 await assert.rejects(write(event,'preferences',{data:'x'.repeat(2000001)}),/too large/);await assert.rejects(write(event,'preferences',{data:'😀'.repeat(600000)}),/too large/)
 await Promise.all([write(event,'preferences',{theme:'light'}),write(event,'preferences',{theme:'dark'})]);assert.equal((await read(event,'preferences')).theme,'dark');if(process.platform!=='win32')assert.equal((await fs.stat(path.join(folder,'align-preferences.json'))).mode&0o777,0o600)
 await write(event,'window',null);assert.equal(win.preferenceWindowReset,true)
 assert.equal(await handlers.get('align-preferences-export-folder')(event),folder);assert.equal((await read(event,'export')).folder,folder)
 source.namespace.installPreferenceStorage({directory:folder,window:()=>win});assert.equal((await handlers.get('align-preferences-read')(event,'preferences')).theme,'dark','restart uses same user-data storage')
 await fs.writeFile(path.join(folder,'align-guide.json'),'{invalid');assert.equal(await handlers.get('align-preferences-read')(event,'guide'),null)
 // The actual preload exposes only the restricted preferences API; it never exports ipcRenderer or Node.
 let api;vm.runInNewContext(await fs.readFile(sourceFile('preload.cjs'),'utf8'),{require:name=>{assert.equal(name,'electron');return {contextBridge:{exposeInMainWorld:(name,value)=>{if(name==='brainanaPreferences')api=value;else assert(['brainanaDataSources','brainanaScientific'].includes(name))}},ipcRenderer:{invoke:(...args)=>args}}}})
 assert.deepEqual(Object.keys(api).sort(),['chooseExportFolder','memory','read','write']);assert.deepEqual(Array.from(api.read('preferences')),['align-preferences-read','preferences'])
 console.log('Native preferences: trusted main-frame IPC, fixed keys, bounded atomic writes, restrictive file permissions, relaunch persistence, export-folder selection and isolated preload passed.')
}finally{await fs.rm(folder,{recursive:true,force:true})}
