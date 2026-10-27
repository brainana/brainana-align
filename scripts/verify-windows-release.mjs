import assert from 'node:assert/strict'
import fs from 'node:fs'
import path from 'node:path'
import os from 'node:os'
import crypto from 'node:crypto'
import vm from 'node:vm'
import {execFileSync} from 'node:child_process'
import {extractFile,getRawHeader} from '@electron/asar'
import {NtExecutable,NtExecutableResource,Resource} from 'resedit'
const root=path.resolve(new URL('..',import.meta.url).pathname)
const app=path.resolve(process.argv[2] || 'apps/align/release/win-unpacked')
const meta=JSON.parse(fs.readFileSync(path.join(root,'VERSION.json')))
const asar=path.join(app,'resources/app.asar')
const walk=folder=>fs.readdirSync(folder,{withFileTypes:true}).flatMap(e=>e.isDirectory()?walk(path.join(folder,e.name)):[path.join(folder,e.name)])
const hash=b=>crypto.createHash('sha256').update(b).digest('hex')
function pe(file,machine){const b=fs.readFileSync(file);assert.equal(b.subarray(0,2).toString(),'MZ',file);const offset=b.readUInt32LE(0x3c);assert.equal(b.subarray(offset,offset+4).toString(),'PE\0\0',file);assert.equal(b.readUInt16LE(offset+4),machine,file);return b}
const binary=path.join(app,'Brainana Align.exe'),exe=pe(binary,0x8664)
for(const file of walk(app).filter(f=>f.endsWith('.dll')))pe(file,0x8664)
const resources=NtExecutableResource.from(NtExecutable.from(exe,{ignoreCert:true}))
const versions=Resource.VersionInfo.fromEntries(resources.entries)
assert.ok(versions.some(v=>v.getAllLanguagesForStringValues().some(l=>{const s=v.getStringValues(l);return s.ProductName==='Brainana Align' && s.FileVersion===meta.version})))
const integrity=resources.entries.find(e=>e.type==='INTEGRITY');assert.ok(integrity,'Electron ASAR integrity resource is present')
const recorded=JSON.parse(Buffer.from(integrity.bin).toString()).find(e=>e.file==='resources\\app.asar');assert.equal(recorded.alg,'SHA256');assert.equal(recorded.value,hash(getRawHeader(asar).headerString),'PE integrity resource matches actual ASAR header')
for(const directory of ['workstation-support','browser-support'])assert.equal(fs.existsSync(path.join(app,'resources',directory)),false)
assert.equal(walk(app).some(f=>path.basename(f)==='node.exe'),false)
const frontend=path.join(root,'apps/align/dist')
for(const file of walk(frontend)){const rel=path.relative(frontend,file).replaceAll(path.sep,'/');assert.equal(hash(fs.readFileSync(file)),hash(extractFile(asar,'apps/align/dist/'+rel)),rel)}
for(const name of ['desktop.mjs','server.mjs','version.mjs','desktop/workstation.mjs'])assert.equal(hash(fs.readFileSync(path.join(root,'apps/align',name))),hash(extractFile(asar,'apps/align/'+name)),name)
for(const packageName of ['core-desktop','core-launcher','core-server'])for(const name of fs.readdirSync(path.join(root,'packages',packageName)).filter(n=>/\.(mjs|cjs)$/.test(n)))assert.equal(hash(fs.readFileSync(path.join(root,'packages',packageName,name))),hash(extractFile(asar,'node_modules/@brainana/'+packageName+'/'+name)))
for(const name of ['LICENSE','NOTICE.txt'])assert.equal(hash(fs.readFileSync(path.join(root,name))),hash(extractFile(asar,name)))
for(const dependency of ['ssh2','asn1','bcrypt-pbkdf','safer-buffer','tweetnacl']) {
 const source=path.join(root,'node_modules',dependency),metadata=JSON.parse(fs.readFileSync(path.join(source,'package.json')))
 const packaged=JSON.parse(extractFile(asar,'node_modules/'+dependency+'/package.json'))
 assert.equal(packaged.version,metadata.version,dependency)
 let main=metadata.main||'index.js';if(!path.extname(main))main+='.js'
 assert.equal(hash(fs.readFileSync(path.join(source,main))),hash(extractFile(asar,'node_modules/'+dependency+'/'+main)),dependency+' entry')
}
const packaged=JSON.parse(extractFile(asar,'apps/align/VERSION.json').toString());assert.equal(packaged.version,meta.version);assert.equal(packaged.buildId,meta.buildId)
const scratch=fs.mkdtempSync(path.join(os.tmpdir(),'brainana-windows-asar-'))
try {
 const dir=path.join(scratch,'frontend');fs.mkdirSync(dir)
 for(const file of walk(frontend)){const rel=path.relative(frontend,file);fs.mkdirSync(path.dirname(path.join(dir,rel)),{recursive:true});fs.writeFileSync(path.join(dir,rel),extractFile(asar,'apps/align/dist/'+rel.replaceAll(path.sep,'/')))}
 const preferenceDir=path.join(scratch,'preferences');fs.mkdirSync(preferenceDir)
 for(const name of ['preferences.mjs','preload.cjs','dataSources.mjs'])fs.writeFileSync(path.join(preferenceDir,name),extractFile(asar,'node_modules/@brainana/core-desktop/'+name))
 for(const [test,args] of [['production-landmark-ui.mjs',['--experimental-vm-modules',dir]],['desktop-preferences-regression.mjs',['--experimental-vm-modules',preferenceDir]],['data-sources-regression.mts',['--experimental-vm-modules','--import','tsx',preferenceDir]],['automatic-worker-ui-regression.mts',['--import','tsx',dir]],['nonlinear-worker-regression.mts',['--import','tsx',dir]],['preparation-ui-regression.mts',['--import','tsx',dir]],['native-export-worker-regression.mts',['--import','tsx',dir]],['initial-processing-regression.mts',['--import','tsx',dir]],['threshold-mask-regression.mts',['--import','tsx',dir]],['manual-affine-regression.mts',['--import','tsx',dir]],['guide-export-inspection-regression.mts',['--import','tsx',dir]],['guided-decisions-regression.mts',['--import','tsx',dir]],['audit-regression.mts',['--import','tsx',dir]],['voxel-brush-regression.mts',['--import','tsx',dir]],['welcome-hover-regression.mts',['--import','tsx',dir]],['guide-teaching-regression.mts',['--import','tsx',dir]],['guide-orderings-regression.mts',['--import','tsx',dir]],['guide-startup-regression.mts',['--import','tsx',dir]],['guide-modal-regression.mts',['--import','tsx',dir]]]){const target=args.pop();execFileSync(process.execPath,[...args,path.join(root,'tests',test),target],{cwd:root,encoding:'utf8'})}
 // Execute the actual Windows menu module from ASAR with Electron adapters.
 let template,about=0;const clicks=[];const context=vm.createContext({process:{platform:'win32'}})
 const electron=new vm.SyntheticModule(['Menu','app'],function(){this.setExport('Menu',{buildFromTemplate:t=>{template=t;return t},setApplicationMenu(){}});this.setExport('app',{showAboutPanel:()=>about++})},{context})
 const menu=new vm.SourceTextModule(extractFile(asar,'node_modules/@brainana/core-desktop/menu.mjs').toString(),{context});await menu.link(()=>electron);await menu.evaluate()
 menu.namespace.applyAppMenu({appLabel:'Brainana Align',window:()=>({webContents:{executeJavaScript:s=>clicks.push(s)}}),connectRemote:null})
 const file=template.find(t=>t.label==='File');assert.ok(file.submenu.some(i=>i.role==='quit'));assert.equal(file.submenu.some(i=>i.label==='Connect to workstation…'),false)
 file.submenu.find(i=>i.label==='Settings…').click();file.submenu.find(i=>i.label==='Save session…').click()
 const help=template.find(t=>t.label==='Help');help.submenu.find(i=>i.label==='Guided setup').click();help.submenu.find(i=>i.label==='About Brainana Align').click()
 assert.equal(about,1);assert.ok(clicks.some(s=>s.includes('settings-open')) && clicks.some(s=>s.includes('save-session')) && clicks.some(s=>s.includes('guided-setup')))
}finally{fs.rmSync(scratch,{recursive:true,force:true})}
const totalBytes=walk(app).reduce((sum,file)=>sum+fs.statSync(file).size,0)
assert.ok(totalBytes<400_000_000,'Windows app exceeds 400 MB size budget')
console.log(JSON.stringify({version:meta.version,buildId:meta.buildId,status:'passed',platform:'win32',architecture:'x64',totalBytes,sourceHashes:'exact ASAR/source/frontend match',peHeaders:'x64 executable and DLLs',branding:'Brainana Align and current version',asarIntegrityResource:'SHA256 matches actual ASAR header',standaloneNodeRuntimes:0,macOSPayloads:0,productionUI:'all 6/7/9/12/spline models, guide, settings and proposals passed in extracted DOM/VM',workers:'exact packaged linear and nonlinear workers passed in Node worker threads',windowsMenu:'actual ASAR Windows menu executed with adapters',authenticode:'unsigned',nativeWindowsLaunch:'pending; cross-built on macOS',installerInstallUninstall:'pending'},null,2))
