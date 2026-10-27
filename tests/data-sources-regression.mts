import assert from 'node:assert/strict'
import {JSDOM} from 'jsdom'
import fs from 'node:fs'
import path from 'node:path'
import vm from 'node:vm'
import {installDataSourceChooser} from '../apps/align/src/dataSourceChooser'
import {setRegistrationPairing} from '../apps/align/src/registrationPairing'
const dom=new JSDOM('<label tabindex="0"><input id="mri-file" type="file"></label><label tabindex="0"><input id="ct-file" type="file"></label>',{url:'http://localhost'})
Object.assign(globalThis,{window:dom.window,document:dom.window.document})
const $=(id:string)=>document.getElementById(id) as HTMLButtonElement
let remote=false,canConnect=true,connections=0,browsed='',localClicks=0,fail=false
installDataSourceChooser({connection:async()=>({remote,canConnect,label:'workstation'}),browse:async modality=>{browsed=modality},connect:async()=>{connections++;if(fail)throw Error('Authentication failed')}})
const tick=()=>new Promise(resolve=>setTimeout(resolve,0))
$('ct-file').addEventListener('click',()=>localClicks++)
setRegistrationPairing('t1-t2');$('ct-file').click();await tick()
assert.equal($('data-source-title').textContent,'Load T2');assert.equal(localClicks,0)
$('data-source-local').click();assert.equal(localClicks,1);assert.ok($('data-source-modal').classList.contains('hidden'))
$('mri-file').click();await tick();$('data-source-network').click();await tick();assert.equal(connections,1)
fail=true;$('data-source-network').click();await tick();assert.match($('data-source-message').textContent!,/Authentication failed/);assert.equal($('data-source-network').disabled,false)
$('data-source-close').click();remote=true;$('ct-file').click();await tick();assert.match($('data-source-network').textContent!,/Browse connected/);$('data-source-network').click();await tick();assert.equal(browsed,'ct');assert.equal(connections,2)
remote=false;canConnect=false;$('mri-file').click();await tick();$('data-source-network').click();await tick();assert.equal(connections,2);assert.match($('data-source-message').textContent!,/mounted network/)
$('data-source-modal').dispatchEvent(new dom.window.KeyboardEvent('keydown',{key:'Escape',bubbles:true}));assert.ok($('data-source-modal').classList.contains('hidden'))
// Main-frame IPC permits only fixed status/connect actions and rejects subframes/spoofed senders.
let handler:any;const frame={},webContents={mainFrame:frame},win={webContents};let invoked=0
const context=vm.createContext({Error});const electron=new vm.SyntheticModule(['ipcMain'],function(){this.setExport('ipcMain',{handle:(_name:string,fn:any)=>handler=fn})},{context})
const root=process.argv[2]?path.resolve(process.argv[2]):path.resolve('packages/core-desktop')
const module=new vm.SourceTextModule(fs.readFileSync(path.join(root,'dataSources.mjs'),'utf8'),{context});await module.link(()=>electron);await module.evaluate()
module.namespace.installDataSourceActions({window:()=>win,canConnect:true,connect:async()=>{invoked++}})
const event={sender:webContents,senderFrame:frame};assert.equal((await handler(event,'status')).canConnect,true);await handler(event,'connect');assert.equal(invoked,1)
await assert.rejects(handler({...event,senderFrame:{}},'connect'),/Untrusted/);await assert.rejects(handler({...event,sender:{}},'status'),/Untrusted/);await assert.rejects(handler(event,'rm -rf'),/Unknown/)
module.namespace.installDataSourceActions({window:()=>win,canConnect:false,connect:async()=>{invoked++}});await assert.rejects(handler(event,'connect'),/packaged macOS/);assert.equal(invoked,1)
setRegistrationPairing('mri-ct');dom.window.close()
console.log('Data sources: local picker, modality labels, connected SFTP browse, connection retry/cancel, mounted-share guidance, Escape and trusted fixed-action IPC passed.')
