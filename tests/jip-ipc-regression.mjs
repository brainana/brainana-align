import assert from 'node:assert/strict'
import fs from 'node:fs/promises'
import vm from 'node:vm'
import path from 'node:path'
import os from 'node:os'
import {createJipService,scientificEnvironment} from '../apps/align/desktop/jipService.mjs'
const folder=await fs.mkdtemp(path.join(os.tmpdir(),'jip-ipc-')),handlers=new Map(),frame={},webContents={mainFrame:frame},win={webContents},events=new Map();let dialogs=0
const app={getPath:key=>key==='userData'?folder:(process.argv[2]??path.join(folder,'No app/Contents/MacOS/No app')),on:(name,fn)=>events.set(name,fn)},ipcMain={handle:(name,fn)=>handlers.set(name,fn)},dialog={showOpenDialog:async()=>{dialogs++;return {canceled:true}}}
const injected=path.join(folder,'external-worker'),marker=path.join(folder,'external-worker-was-run')
await fs.writeFile(injected,`#!/bin/sh\n/usr/bin/touch '${marker}'\nprintf '%s' '{"protocol":1,"runtime":"brainana-align-registration","version":"fake","actions":["register","itk-register"],"features":["itk-v1"]}'\n`,{mode:0o755})
await fs.writeFile(path.join(folder,'jip-runtime.json'),JSON.stringify({file:injected}))
const module=new vm.SourceTextModule(await fs.readFile('apps/align/desktop/jipActions.mjs','utf8'),{identifier:path.resolve('apps/align/desktop/jipActions.mjs')})
await module.link(async name=>{const imports=name==='electron'?{app,ipcMain,dialog}:name==='./jipService.mjs'?{createJipService,scientificEnvironment}:await import(name);const exports=Object.keys(imports),m=new vm.SyntheticModule(exports,function(){for(const key of exports)this.setExport(key,imports[key])});return m});await module.evaluate();await module.namespace.installJipActions({window:()=>win})
const handle=handlers.get('brainana-scientific');assert(handle);const trusted={sender:webContents,senderFrame:frame}
async function qualifyFrozenItk(runtime){
 assert(runtime.features.includes('itk-v1'),'Embedded backend must advertise the ITK registration capability')
 const dims=[13,15,17],affine=[[Math.sqrt(3)/2,-.6,0,-5],[.5,1.2*Math.sqrt(3)/2,0,-8],[0,0,1.4,-10],[0,0,0,1]],n=dims.reduce((a,b)=>a*b,1)
 const anatomy=(x,y,z)=>4+90*Math.exp(-((x-4.3)**2/8+(y-6.2)**2/12+(z-7.1)**2/18))+50*Math.exp(-((x-8.2)**2/4+(y-10.4)**2/9+(z-11.3)**2/10))+18*Math.exp(-((x-3.1)**2/3+(y-10.1)**2/5+(z-4.3)**2/6))+.4*x+.15*y+.08*z+.03*x*y
 const fixed={dims,affine,values:new Float32Array(n)},moving={dims,affine,values:new Float32Array(n)}
 for(let z=0;z<dims[2];z++)for(let y=0;y<dims[1];y++)for(let x=0;x<dims[0];x++){const i=x+dims[0]*(y+dims[1]*z);fixed.values[i]=anatomy(x,y,z);moving.values[i]=1.2*anatomy(x-.25+.06*Math.sin(z/3),y+.17,z-.11)+2}
 const fixedBefore=fixed.values.slice(),movingBefore=moving.values.slice(),settings={engine:'itk',metric:'mattes-mi',bins:24,sampling_fraction:1,seed:1729,threads:1,nonlinear_iterations:2,bspline_spacing_mm:10,maximum_step_mm:.2,shrink_factors:[1],smoothing_sigmas_mm:[0]}
 const result=await handle(trusted,'run',{action:'itk-register',fixed,moving,start:[[1,0,0,.05],[0,1,0,-.04],[0,0,1,.03],[0,0,0,1]],stages:'nonlinear',settings})
 assert.equal(result.settings.engine,'itk');assert.equal(result.execution.engine,'itk');assert.equal(result.execution.active,'cpu');assert.equal(result.stages,'nonlinear')
 assert.equal(result.deformation.kernel,'jip-dense-pull');assert.equal(result.deformation.settings.engine,'itk');assert.equal(result.deformation.execution.engine,'itk');assert.deepEqual(result.deformation.dims,dims)
 assert(result.deformation.residualValues instanceof Float32Array);assert.equal(result.deformation.residualValues.length,3*n);assert.equal(result.deformation.residualAsset,undefined);assert.equal(result.deformation.residualBase64,undefined)
 assert(Math.max(...result.deformation.residualValues.map(Math.abs))>1e-8,'Asymmetric inputs must exercise a nonzero nonlinear field')
 assert.deepEqual(fixed.values,fixedBefore);assert.deepEqual(moving.values,movingBefore)
 // Load the real shared transform/session/export implementations only for this
 // optional frozen-app qualification. The ordinary IPC trust test needs no TS loader.
 const {register}=await import('tsx/esm/api'),unregister=register()
 try{
  const [{createPointMapping,validateSpline},{applyMat4},{invertAffine},{createSessionPayload,parseSessionPayload},{createRegistrationArtifacts}]=await Promise.all([
   import('../packages/imaging-math/nonlinearSpline.ts'),import('../packages/imaging-math/rigid.ts'),import('../packages/imaging-math/roiWarp.ts'),import('../apps/align/src/sessionPersistence.ts'),import('../apps/align/src/exportArtifacts.ts')
  ])
  validateSpline(result.deformation)
  const mapping=createPointMapping(result.matrix,result.deformation),distance=(a,b)=>Math.hypot(...a.map((v,i)=>v-b[i]))
  const points=[[3,4,5],[6,7,8],[9,10,11]].map(index=>applyMat4(affine,index))
  let nonlinearChange=0
  for(const point of points){const mapped=mapping.forward(point),pulled=mapping.inverse(point);assert(distance(mapping.inverse(mapped),point)<2e-4,'Frozen ITK moving-to-fixed point inverse roundtrip');assert(distance(mapping.forward(pulled),point)<2e-4,'Frozen ITK fixed-to-moving point forward roundtrip');nonlinearChange=Math.max(nonlinearChange,distance(mapped,applyMat4(result.matrix,point)))}
  assert(nonlinearChange>1e-8,'Point mapping must retain the nonlinear field rather than return the affine alone')
  const fit={direction:'ct-mri',model:'jip',nonlinear:result.deformation,parameterCount:3*n,matrix:result.matrix,inverse:invertAffine(result.matrix),baseMatrix:result.matrix,landmarkMatrix:result.matrix,initializationSource:'automatic',rms:0,manual:[0,0,0,0,0,0],landmarkSnapshot:[],landmarksChanged:false,fittedAt:'frozen-itk-ipc-test',proposal:null,jipRegistration:{settings:result.settings,execution:result.execution,diagnostics:result.diagnostics,stages:'nonlinear'}}
  const payload=createSessionPayload({appVersion:'frozen-itk-ipc-test',loaded:{mri:null,ct:null},landmarks:[],selectedId:null,nextId:1,direction:'ct-mri',fit,optimizationWindows:{mri:{},ct:{}}}),sessionJSON=JSON.parse(JSON.stringify(payload)),restored=parseSessionPayload(sessionJSON).fit
  assert.equal(sessionJSON.fit.nonlinear.residualValues,undefined);assert.equal(typeof sessionJSON.fit.nonlinear.residualBase64,'string');assert.equal(restored.jipRegistration.settings.engine,'itk')
  const artifacts=createRegistrationArtifacts({appVersion:'frozen-itk-ipc-test',fit,landmarks:[]});assert.equal(artifacts.length,1,'Full ITK warp cannot be exported as an affine TXT')
  const metadata=JSON.parse(await artifacts[0].blob.text());assert.equal(metadata.registration_engine,'ITK');assert.equal(metadata.transform_model,'itk-bspline');assert.equal(metadata.forward_matrix,null);assert.equal(metadata.inverse_matrix,null);assert.equal(metadata.nonlinear.residualValues,undefined);assert.equal(metadata.nonlinear.residualBase64,sessionJSON.fit.nonlinear.residualBase64);validateSpline(metadata.nonlinear)
  for(const recovered of [createPointMapping(restored.matrix,restored.nonlinear),createPointMapping(metadata.affine_component_forward_matrix,metadata.nonlinear)])for(const point of points)for(const direction of ['forward','inverse'])assert(distance(recovered[direction](point),mapping[direction](point))<1e-9,'Frozen ITK full physical map survives actual session and export serialization')
  console.log('Embedded ITK IPC: advertised capability, asymmetric oblique 13×15×17 nonlinear-only execution from an accepted linear base, CPU engine provenance, nonzero full-map hydration, physical bidirectional roundtrips, immutable inputs and canonical session/export mappings passed')
 }finally{await unregister()}
}
try{for(const action of ['status','choose','run','poll','cancel'])for(const event of [{sender:{},senderFrame:frame},{sender:webContents,senderFrame:{}}])await assert.rejects(()=>handle(event,action,{action:'register'}),/restricted/);assert.equal(dialogs,0);
if(!process.argv[2]){assert.equal((await handle(trusted,'status')).available,false,'Legacy external runtime preferences must be ignored')}
if(process.argv[2]){const runtime=await handle(trusted,'status'),expectedVersion=process.env.JIP_EXPECTED_VERSION??'0.17.7';assert.equal(runtime.available,true);assert.equal(runtime.version,expectedVersion);const raw={dims:[13,15,17],affine:[[1,0,0,0],[0,1,0,0],[0,0,1,0],[0,0,0,1]],values:Float32Array.from({length:13*15*17},(_,i)=>1+i%13)};const result=await handle(trusted,'run',{action:'register',fixed:raw,moving:raw,start:raw.affine,stages:'nonlinear',settings:{model:6,metric:'nc',axes:'z',grid:4,regularization:1}});assert.equal(result.deformation.kernel,'jip-dense-pull');console.log('Embedded JIP discovery and a complete trusted IPC registration passed');assert.equal(runtime.embedded,true);await qualifyFrozenItk(runtime)}
await assert.rejects(()=>fs.stat(marker),/ENOENT/,'An external app/pref must never be executed');await assert.rejects(()=>handle(trusted,'choose'),/Unknown scientific action/);assert.equal(dialogs,0);await assert.rejects(()=>handle(trusted,'arbitrary-file-read','/etc/passwd'),/Unknown/);assert(['idle','complete'].includes((await handle(trusted,'poll')).status));console.log('Exact domain IPC: trusted sender/main frame required for every action, no external runtime selection or discovery, ignored legacy paths and fixed action vocabulary passed')}finally{await events.get('before-quit')?.();await fs.rm(folder,{recursive:true,force:true})}
