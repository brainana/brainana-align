import assert from 'node:assert/strict'
import fs from 'node:fs'
import path from 'node:path'
import crypto from 'node:crypto'
import os from 'node:os'
import {execFileSync} from 'node:child_process'
import {fileURLToPath,pathToFileURL} from 'node:url'
import {extractFile} from '@electron/asar'
const root=path.resolve(path.dirname(fileURLToPath(import.meta.url)),'..')
const app=path.resolve(process.argv[2] || 'apps/align/release/mac-arm64/Brainana Align.app')
const asar=path.join(app,'Contents/Resources/app.asar'),meta=JSON.parse(fs.readFileSync(path.join(root,'VERSION.json'),'utf8'))
const plist=JSON.parse(execFileSync('/usr/bin/plutil',['-convert','json','-o','-',path.join(app,'Contents/Info.plist')],{encoding:'utf8'}))
assert.equal(plist.CFBundleIdentifier,meta.bundleIdentifier)
assert.equal(plist.CFBundleShortVersionString,meta.version.split('-')[0]);assert.equal(plist.CFBundleVersion,meta.bundleVersion)
assert.equal(plist.BrainanaReleaseVersion,meta.version);assert.equal(plist.BrainanaBuildID,meta.buildId)
assert.equal(plist.LSMinimumSystemVersion,meta.minimumMacOS)
const hash=b=>crypto.createHash('sha256').update(b).digest('hex')
const frontend=path.join(root,'apps/align/dist')
const walk=folder=>fs.readdirSync(folder,{withFileTypes:true}).flatMap(e=>e.isDirectory()?walk(path.join(folder,e.name)):[path.join(folder,e.name)])
for(const file of walk(frontend)){const rel=path.relative(frontend,file).replaceAll(path.sep,'/');assert.equal(hash(fs.readFileSync(file)),hash(extractFile(asar,'apps/align/dist/'+rel)),rel)}
for(const name of ['desktop.mjs','server.mjs','version.mjs','desktop/workstation.mjs','desktop/jipActions.mjs','desktop/jipService.mjs'])assert.equal(hash(fs.readFileSync(path.join(root,'apps/align',name))),hash(extractFile(asar,'apps/align/'+name)),name)
for(const packageName of ['core-desktop','core-launcher','core-server'])for(const name of fs.readdirSync(path.join(root,'packages',packageName)).filter(n=>/\.(mjs|cjs)$/.test(n)))assert.equal(hash(fs.readFileSync(path.join(root,'packages',packageName,name))),hash(extractFile(asar,'node_modules/@brainana/'+packageName+'/'+name)),packageName+'/'+name)
const js=walk(frontend).filter(f=>f.endsWith('.js')).map(f=>extractFile(asar,'apps/align/dist/'+path.relative(frontend,f).replaceAll(path.sep,'/')).toString()).join('\n')
for(const removed of ['BRAINANA / IMAGE REGISTRATION','Bring your brain images into alignment.'])assert.ok(!js.includes(removed),'Removed welcome text must not ship: '+removed)
assert.ok(!js.includes('guided-reset-workspace'),'Removed guide reset shortcut must not ship')
assert.ok(!js.includes('End guide'),'Guide completion must not require a second End guide action')
for(const label of ['review-comparison','review-flip','Flip between images','Auto flip · 1 s','linear-defaults-select','linear-defaults-apply','AFNI · 3dAllineate defaults','FSL · FLIRT default affine','ANTs · antsRegistrationSyN.sh affine (-t a)','JIP · 0.8.4 linear defaults','Elastix · default affine parameter map','linear-registration','Linear registration','ITK nonlinear B-spline','itk-registration','itk-run','Acquisition details and suggested settings','Calculate leave-one-out sensitivity','landmarkQualityWorker-','affine-resample','jip-registration','jip-run','JIP registration','jip-dense-pull','welcome-screen','data-registration-pairing','voxel-hover-readout','Paint specified intensity','processing-edit-target','processing-mask-value','processing-brush-radius','processing-edit-apply','processing-edit-discard','voxel-edit-preview','contrast-value','intensity-controls','threshold-controls','threshold-mask-preview','mask-create','mask-apply','generated-image-choice','save-generated-image','display-reset','Initial processing','processing-crop-apply','processing-voxel','processing-export','processingWorker-','processingExportWorker-','aligned-image-source','spline-alignment','spline-actions','accept-spline','reject-spline','local-auto-linear','landmark-refinement-availability','spline-availability','refine-actions-followup','Run image-based refinement','Finish guide','Continue with current progress','Back changes instructions only','Downsampling presets','Resolution · pending','preparation-estimate','registration-spacing','nativeExportWorker-','volumeWorker-','Nonlinear landmark spline (advanced)','Landmark refinement','Image-based registration','define-windows-followup','run-landmark-spline','spline-smoothing','save-spline-fields','nonlinearWorker-','Rigid, 6 parameters','Similarity, 7 parameters','Anisotropic scale, 9 parameters','Affine, 12 parameters','auto-linear-model','Custom parameter groups','Run linear registration','Refine current alignment','Constrained refinement','refine-translation-step','refine-rotation-step','local-refinement-settings','registrationWorker-','guided-setup','settings-open','reset-workspace','Window was not changed: drag a rectangle at least 4 pixels wide and high.','Reset workspace','settings-modal','High contrast','Export settings','settings-recover','Begin manual alignment','data-workflow','alignment-progress','guided-resume','Apply landmark transformation','movingAlpha=clamp(movingSample.a*overlayAlphaShader'])assert.ok(js.includes(label),label)
const binary=path.join(app,'Contents/MacOS',plist.CFBundleExecutable)
const archs=execFileSync('/usr/bin/lipo',['-archs',binary],{encoding:'utf8'}).trim().split(/\s+/)
const expected=process.argv[3] || 'arm64'
assert.deepEqual(archs,[expected==='x64'?'x86_64':expected])
execFileSync('/usr/bin/codesign',['--verify','--deep','--strict',app])
const runtime=path.join(app,'Contents/Resources/workstation-support/Contents/Resources/runtime')
for(const file of walk(frontend)){const rel=path.relative(frontend,file);assert.equal(hash(fs.readFileSync(file)),hash(fs.readFileSync(path.join(runtime,'dist',rel))),'browser adapter frontend '+rel)}
for(const name of ['platformCore.mjs','sftpClient.mjs','replaceWithBackup.mjs','runtime.mjs','networkBrowser.mjs','networkSftpClient.mjs','knownHosts.mjs'])assert.equal(hash(fs.readFileSync(path.join(root,'packages/core-server',name))),hash(fs.readFileSync(path.join(runtime,name))))
assert.equal(hash(fs.readFileSync(path.join(root,'apps/align/version.mjs'))),hash(fs.readFileSync(path.join(runtime,'version.mjs'))))
assert.equal(fs.existsSync(path.join(runtime,'darwin-arm64')),false)
assert.equal(fs.existsSync(path.join(runtime,'darwin-x64')),false)
assert.equal(hash(fs.readFileSync(path.join(root,'packaging/runtime-command.sh'))),hash(fs.readFileSync(path.join(runtime,'runtime-command.sh'))))
const identity=JSON.parse(execFileSync(binary,['-p','JSON.stringify({version:process.version,arch:process.arch})'],{env:{...process.env,ELECTRON_RUN_AS_NODE:'1'},encoding:'utf8'}))
assert.equal(identity.arch,expected)
assert.equal(execFileSync(binary,['--input-type=module','-e',`const m=await import(${JSON.stringify(pathToFileURL(path.join(runtime,'runtime.mjs')).href)});console.log(typeof m.runServerCli)`],{env:{...process.env,ELECTRON_RUN_AS_NODE:'1'},encoding:'utf8'}).trim(),'function','Slim workstation runtime resolves every server dependency')
for(const dependency of [path.join(runtime,'node_modules/ssh2'),path.join(asar,'node_modules/ssh2')])assert.equal(execFileSync(binary,['-p',`typeof require(${JSON.stringify(dependency)}).Client`],{env:{...process.env,ELECTRON_RUN_AS_NODE:'1'},encoding:'utf8'}).trim(),'function','Packaged JavaScript SSH dependency resolves')
for(const name of ['LICENSE','NOTICE.txt'])assert.equal(hash(fs.readFileSync(path.join(root,name))),hash(extractFile(asar,name)))
const totalBytes=walk(app).filter(file=>!fs.lstatSync(file).isSymbolicLink()).reduce((sum,file)=>sum+fs.statSync(file).size,0)
const scientificFolder=path.join(app,'Contents/Resources/scientific-runtime')
assert.ok(fs.existsSync(path.join(scientificFolder,'BrainanaAlignRegistration')),'Align must contain its own registration worker')
const scientificBytes=walk(scientificFolder).filter(f=>!fs.lstatSync(f).isSymbolicLink()).reduce((sum,f)=>sum+fs.statSync(f).size,0)
assert.ok(totalBytes-scientificBytes<350_000_000,'Core desktop exceeds its350MB budget')
assert.ok(scientificBytes<90_000_000&&totalBytes<350_000_000,'Integrated scientific runtime exceeds measured size budgets')
for(const f of walk(scientificFolder))assert.ok(!/SimpleITK|PyQt|PySide|QtCore|QtWidgets|jip_workbench|\.app(?:\/|$)/.test(path.relative(scientificFolder,f)),'Standalone GUI dependency must not ship in Align')
for(const removed of ['jip-choose-runtime','itk-choose-runtime','Choose JIP Registration app…','Choose registration companion…'])assert.ok(!js.includes(removed),'External runtime selection must not ship: '+removed)
// Execute the exact ASAR bundle in the same Node version supplied by Electron, without requiring a GUI session.
const scratch=fs.mkdtempSync(path.join(os.tmpdir(),'brainana-asar-proof-'))
try {
 const dir=path.join(scratch,'frontend');fs.mkdirSync(dir)
 for(const file of walk(frontend)){const rel=path.relative(frontend,file);fs.mkdirSync(path.dirname(path.join(dir,rel)),{recursive:true});fs.writeFileSync(path.join(dir,rel),extractFile(asar,'apps/align/dist/'+rel.replaceAll(path.sep,'/')))}
 const preferenceDir=path.join(scratch,'desktop-preferences');fs.mkdirSync(preferenceDir);for(const name of ['preferences.mjs','preload.cjs','dataSources.mjs'])fs.writeFileSync(path.join(preferenceDir,name),extractFile(asar,'node_modules/@brainana/core-desktop/'+name))
 execFileSync(process.execPath,['--experimental-vm-modules',path.join(root,'tests/desktop-preferences-regression.mjs'),preferenceDir],{encoding:'utf8'})
 execFileSync(process.execPath,['--experimental-vm-modules','--import','tsx',path.join(root,'tests/data-sources-regression.mts'),preferenceDir],{cwd:root,encoding:'utf8'})
 execFileSync(process.execPath,['--experimental-vm-modules',path.join(root,'tests/production-landmark-ui.mjs'),dir],{encoding:'utf8'})
 execFileSync(process.execPath,['--import','tsx',path.join(root,'tests/flip-inspection-regression.mts')],{cwd:root,encoding:'utf8'})
 execFileSync(process.execPath,['--import','tsx',path.join(root,'tests/viewer-rendering-regression.mts'),dir],{cwd:root,encoding:'utf8'})
 execFileSync(process.execPath,['--import','tsx',path.join(root,'tests/linear-defaults-worker.mts'),dir],{cwd:root,encoding:'utf8'})
 execFileSync(process.execPath,['--import','tsx',path.join(root,'tests/automatic-worker-ui-regression.mts'),dir],{cwd:root,encoding:'utf8'})
 execFileSync(process.execPath,['--import','tsx',path.join(root,'tests/nonlinear-worker-regression.mts'),dir],{cwd:root,encoding:'utf8'})
 for(const test of ['preparation-ui-regression.mts','native-export-worker-regression.mts','initial-processing-regression.mts','threshold-mask-regression.mts','manual-affine-regression.mts','guide-export-inspection-regression.mts','guided-decisions-regression.mts','audit-regression.mts','voxel-brush-regression.mts','welcome-hover-regression.mts','guide-teaching-regression.mts','guide-orderings-regression.mts','guide-startup-regression.mts','guide-modal-regression.mts','overlay-worker-regression.mts','landmark-quality-worker-regression.mts'])execFileSync(process.execPath,['--import','tsx',path.join(root,'tests',test),dir],{cwd:root,encoding:'utf8'})
}finally{fs.rmSync(scratch,{recursive:true,force:true})}
console.log(JSON.stringify({version:meta.version,buildId:meta.buildId,app,generatedAt:new Date().toISOString(),status:'passed',architectures:archs,appSignature:'verified ad-hoc',sourceHashes:'exact match with ASAR and slim workstation adapter',embeddedNode:identity,totalBytes,standaloneNodeRuntimes:0,embeddedScientificRuntime:{bytes:scientificBytes,coreBytes:totalBytes-scientificBytes,noStandaloneGui:true},packagedProductionUI:'passed: Settings themes/persistence, seven sections, display sliders and live mask controls, defaults separation/docking/reset/keyboard; four landmark models, five automatic models, rigid defaults, reflection decisions and guide',packagedPreferenceStorage:'passed: exact ASAR handler and preload executed with Electron adapters; fixed-key authorization, atomic persistence and relaunch checks',packagedNonlinearWorker:'passed: exact ASAR fit/resample/field worker and task controller, transfer-safe buffers, errors/progress/cancellation',packagedRegistrationWorker:'passed: exact ASAR worker in Node worker threads, model routes, direction, proposals, session restore, cancellation and stale-input discard; native browser launch pending',packagedOverlayWorker:'passed: exact ASAR affine and nonlinear background resampling, source immutability and cancellation',packagedLandmarkSensitivityWorker:'passed: exact ASAR bounded leave-one-out refits',packagedSliceRenderer:'passed: exact emitted crosshair adapter and zoom function; actual Niivue geometry/line commands in nine panes; GPU pixels pending',nativeGUI:'pending'},null,2))
