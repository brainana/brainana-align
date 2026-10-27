import fs from 'node:fs'
import os from 'node:os'
import path from 'node:path'
import assert from 'node:assert/strict'
import crypto from 'node:crypto'
import {execFileSync} from 'node:child_process'
import {extractFile} from '@electron/asar'
const platform=process.platform,arch=process.arch,root=process.cwd(),release=path.join(root,'apps/align/release')
const candidates=fs.readdirSync(release).map(name=>path.join(release,name)).filter(p=>fs.statSync(p).isDirectory())
let app,resources,exe
if(platform==='darwin'){
 const folder=candidates.find(p=>p.endsWith('mac-'+arch));assert(folder,'Missing native macOS unpacked output')
 app=path.join(folder,'Brainana Align.app');resources=path.join(app,'Contents/Resources');exe=path.join(app,'Contents/MacOS/Brainana Align')
 execFileSync('/usr/bin/codesign',['--verify','--deep','--strict',app]);const slices=execFileSync('/usr/bin/lipo',['-archs',exe],{encoding:'utf8'}).trim();assert.equal(slices,arch==='x64'?'x86_64':arch)
}else{
 app=candidates.find(p=>p.endsWith(platform==='win32'?'win-unpacked':'linux-unpacked'));assert(app,'Missing unpacked native output')
 resources=path.join(app,'resources');exe=path.join(app,platform==='win32'?'Brainana Align.exe':'brainana-align')
}
const asar=path.join(resources,'app.asar'),meta=JSON.parse(fs.readFileSync('VERSION.json','utf8'))
assert.equal(JSON.parse(extractFile(asar,'apps/align/VERSION.json').toString()).version,meta.version)
const hash=b=>crypto.createHash('sha256').update(b).digest('hex')
const walk=p=>fs.readdirSync(p,{withFileTypes:true}).flatMap(e=>e.isDirectory()?walk(path.join(p,e.name)):[path.join(p,e.name)])
const dist=path.join(root,'apps/align/dist'),scratch=fs.mkdtempSync(path.join(os.tmpdir(),'align-package-proof-'))
try{
 for(const file of walk(dist)){
  const rel=path.relative(dist,file),data=extractFile(asar,'apps/align/dist/'+rel.split(path.sep).join('/'))
  assert.equal(hash(data),hash(fs.readFileSync(file)),rel);const target=path.join(scratch,rel);fs.mkdirSync(path.dirname(target),{recursive:true});fs.writeFileSync(target,data)
 }
 const js=walk(scratch).filter(p=>p.endsWith('.js')).map(p=>fs.readFileSync(p,'utf8')).join('\n')
 for(const label of ['Rigid, 6 parameters','Similarity, 7 parameters','Anisotropic scale, 9 parameters','Affine, 12 parameters','Flip between images','JIP registration','ITK nonlinear B-spline'])assert(js.includes(label),label)
 const science=path.join(resources,'scientific-runtime')
 const scientificFiles=walk(science)
 assert(!scientificFiles.some(p=>/SimpleITK|PyQt|PySide|QtWidgets|jip_workbench/.test(path.relative(science,p))),'GUI/large external scientific runtime must not ship')
 const size=folder=>walk(folder).filter(p=>!fs.lstatSync(p).isSymbolicLink()).reduce((sum,p)=>sum+fs.statSync(p).size,0)
 const appBytes=size(app),scientificBytes=size(science)
 assert(scientificBytes<=90_000_000,'Scientific runtime exceeds90MB budget')
 if(platform==='darwin'&&arch==='arm64')assert(appBytes<=350_000_000,'Apple Silicon app exceeds350MB budget')
 const worker=path.join(resources,'scientific-runtime','BrainanaAlignRegistration'+(platform==='win32'?'.exe':''))
 const scientific=JSON.parse(execFileSync(worker,['--align-capabilities'],{encoding:'utf8',timeout:60000}));assert.equal(scientific.runtime,'brainana-align-registration')
 const identity=JSON.parse(execFileSync(exe,['-p','JSON.stringify({arch:process.arch,version:process.version})'],{encoding:'utf8',timeout:60000,env:{...process.env,ELECTRON_RUN_AS_NODE:'1'}}));assert.equal(identity.arch,arch)
 for(const test of ['production-landmark-ui.mjs','viewer-rendering-regression.mts','linear-defaults-worker.mts','automatic-worker-ui-regression.mts','nonlinear-worker-regression.mts','native-export-worker-regression.mts','initial-processing-regression.mts','threshold-mask-regression.mts','overlay-worker-regression.mts']){
  execFileSync(process.execPath,['--experimental-vm-modules','--import','tsx',path.join(root,'tests',test),scratch],{cwd:root,stdio:'inherit',timeout:180000})
 }
 execFileSync(process.execPath,['--experimental-vm-modules',path.join(root,'tests/jip-ipc-regression.mjs'),exe],{cwd:root,stdio:'inherit',timeout:180000})
 const out=path.join(root,'artifacts/validation');fs.mkdirSync(out,{recursive:true});fs.writeFileSync(path.join(out,'package-'+platform+'-'+arch+'.json'),JSON.stringify({version:meta.version,platform,arch,status:'passed',exactFrontend:true,appBytes,scientificBytes,embeddedNode:identity,scientific,app,nativeGUIPixels:'not certified by headless tests'},null,2)+'\n')
}finally{fs.rmSync(scratch,{recursive:true,force:true})}
