import assert from 'node:assert/strict'
import fs from 'node:fs/promises'
import path from 'node:path'
import os from 'node:os'
import {createRequire} from 'node:module'
import * as nifti from 'nifti-reader-js'
import {gunzipSync} from 'fflate'
import {startBrowserHarness} from './browser-harness.mjs'
import {createSessionFixture} from './helpers/session-fixture.mjs'

const require=createRequire(import.meta.url),{chromium}=require('playwright-core'),scratch=await fs.mkdtemp(path.join(os.tmpdir(),'align-browser-session-'))
let browser,harness
try{
 const {session,mri,ct,dims,matrix,mriAffine,values}=await createSessionFixture(path.join(scratch,'data'))
 harness=await startBrowserHarness()
 browser=await chromium.launch({executablePath:process.env.CHROMIUM_PATH||chromium.executablePath(),headless:true,args:['--no-sandbox','--use-angle=swiftshader','--enable-unsafe-swiftshader','--disable-dev-shm-usage']})
 const page=await browser.newPage({viewport:{width:1540,height:980}}),errors=[];page.on('pageerror',e=>errors.push(e.message))
 await page.goto(harness.url+'/#session='+'a'.repeat(64));await page.waitForSelector('#welcome-session');assert(await page.locator('#save-session').isDisabled())
 await page.setInputFiles('#load-session',session);await page.waitForSelector('#session-locate-modal')
 assert.equal(await page.locator('#welcome-screen').isVisible(),true,'Opening a session cannot discard the welcome state before images are ready')
 assert.match(await page.locator('#session-locate-title').textContent(),/MRI/)
 await page.setInputFiles('#session-locate-input',mri);await page.waitForFunction(()=>document.querySelector('#session-locate-title')?.textContent.includes('CT'))
 await page.setInputFiles('#session-locate-input',ct);await page.waitForFunction(()=>document.querySelector('#status').textContent.includes('Full session restored.'))
 assert.equal(await page.locator('#welcome-screen').isVisible(),false)
 await page.evaluate(()=>{window.__saved=[];window.brainanaAlignSaveBlob=async(blob,filename)=>window.__saved.push({filename,bytes:Array.from(new Uint8Array(await blob.arrayBuffer()))});document.querySelector('#save-session').click()})
 await page.waitForFunction(()=>window.__saved.length===1);const saved=JSON.parse(Buffer.from((await page.evaluate(()=>window.__saved[0])).bytes).toString());assert.deepEqual(saved.fit.matrix,matrix);assert.equal(saved.landmarks.length,5);assert.deepEqual(saved.images.mri.workingDims,[9,10,11]);assert.equal(saved.schemaVersion,14)
 await page.evaluate(()=>{window.__saved=[];document.querySelector('#save-aligned').click()});await page.waitForFunction(()=>window.__saved.some(s=>s.filename.endsWith('.nii.gz')))
 const output=await page.evaluate(()=>window.__saved.find(s=>s.filename.endsWith('.nii.gz'))),bytes=gunzipSync(Uint8Array.from(output.bytes)),header=nifti.readHeader(bytes.buffer);assert.deepEqual(header.dims.slice(1,4),dims);assert.deepEqual(header.affine,mriAffine);assert.deepEqual(new Float32Array(nifti.readImage(header,bytes.buffer)),values)
 // Cancelling a later import retains the current complete alignment and files.
 const different=path.join(scratch,'different.json'),payload=JSON.parse(await fs.readFile(session,'utf8'));payload.images.mri.name='different-mri.nii';payload.images.mri.sourceFiles=['different-mri.nii'];await fs.writeFile(different,JSON.stringify(payload));await page.setInputFiles('#load-session',different);await page.waitForSelector('#session-locate-modal');await page.locator('#session-locate-cancel').click();await page.waitForFunction(()=>document.querySelector('#status').textContent.includes('Session opening cancelled'))
 assert.equal(await page.locator('#mri-name').textContent(),'anatomy.nii');await page.evaluate(()=>{window.__saved=[];document.querySelector('#save-session').click()});await page.waitForFunction(()=>window.__saved.length===1);assert.deepEqual(JSON.parse(Buffer.from((await page.evaluate(()=>window.__saved[0])).bytes).toString()).fit.matrix,matrix)
 assert.deepEqual(errors,[])
 const report={status:'passed',browser:await browser.version(),compiledFrontend:true,welcomeWithoutPairingSelection:true,explicitOriginalImagePickers:true,savedWorkingGrid:true,fiveLandmarksAndTransform:true,nativeExportAllPixelsExact:true,cancelPreservesPriorAlignment:true,rendering:'Software WebGL2; no physical GPU certification'}
 await fs.mkdir('artifacts/validation',{recursive:true});await fs.writeFile('artifacts/validation/session-browser.json',JSON.stringify(report,null,2)+'\n');console.log(JSON.stringify(report,null,2))
}finally{await browser?.close();await harness?.close();await fs.rm(scratch,{recursive:true,force:true})}
