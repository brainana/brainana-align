import assert from 'node:assert/strict'
import fs from 'node:fs/promises'
import os from 'node:os'
import path from 'node:path'
import {EventEmitter} from 'node:events'
import {createJipService,samplingInputs} from '../apps/align/desktop/jipService.mjs'
const temporary=await fs.mkdtemp(path.join(os.tmpdir(),'jip-regions-')),raw={dims:[8,9,10],affine:[[.8,-.6,0,7],[.6,.8,0,-4],[0,0,1.2,3],[0,0,0,1]],values:Float32Array.from({length:720},(_,i)=>2+Math.sin(i*.1))},mask=new Uint8Array(720),windows=[{min:[-100,-100,10000],max:[100,100,10001],axes:[0,1]}]
mask[3]=1;const sampling={version:1,fixedWindows:windows,movingWindows:windows,fixedExcluded:mask,movingExcluded:mask}
assert.deepEqual(samplingInputs(sampling,raw,raw).payload.fixed_windows,windows)
for(const spec of [{...sampling,movingExcluded:Uint8Array.of(0)},{...sampling,fixedExcluded:new Uint8Array(720).fill(2)},{...sampling,movingWindows:[{...windows[0],axes:[1,1]}]},{...sampling,movingWindows:[{...windows[0],min:[NaN,0,0]}]}])assert.throws(()=>samplingInputs(spec,raw,raw))
let confirms=true,spawned=0,features=[]
const service=createJipService({temporary,runtime:async()=>({file:'test-only',features}),spawnWorker:(_file,args)=>{
 spawned++;const child=new EventEmitter();child.stderr=new EventEmitter();child.kill=()=>child.emit('close',130)
 void(async()=>{const folder=path.dirname(args[args.indexOf('--align-output')+1]),request=JSON.parse(await fs.readFile(path.join(folder,'request.json'),'utf8'))
  assert.deepEqual(request.sampling.moving_windows,windows)
  for(const key of ['fixed_excluded','moving_excluded']){assert.equal(path.dirname(request.sampling[key]),folder);assert.deepEqual(new Uint8Array(await fs.readFile(request.sampling[key])),mask)}
  assert.deepEqual((await fs.readFile(request.fixed)).subarray(352),Buffer.from(raw.values.buffer))
  await fs.writeFile(path.join(folder,'result.json'),JSON.stringify({status:'ready',result:{settings:confirms?{sampling:{version:1}}:{}}}));child.emit('close',0)
 })().catch(e=>child.emit('error',e));return child
}})
try{
 await assert.rejects(()=>service.run({action:'register',fixed:raw,moving:raw,start:raw.affine,stages:'nonlinear',sampling,settings:{}}),/sampling-v1/);assert.equal(spawned,0);assert.deepEqual(await fs.readdir(temporary),[])
 features=['sampling-v1'];const result=await service.run({action:'register',fixed:raw,moving:raw,start:raw.affine,stages:'nonlinear',sampling,settings:{}});assert.equal(result.settings.sampling.version,1)
 confirms=false;await assert.rejects(()=>service.run({action:'register',fixed:raw,moving:raw,start:raw.affine,stages:'nonlinear',sampling,settings:{}}),/did not confirm/);assert.deepEqual(await fs.readdir(temporary),[])
 if(process.env.JIP_TEST_SOURCE){
  const real=createJipService({temporary,runtime:async()=>({file:process.env.JIP_TEST_PYTHON??'/opt/anaconda3/bin/python',args:[path.resolve(process.env.JIP_TEST_SOURCE,'frontend/launch.py')],features})})
  try{const result=await real.run({action:'register',fixed:raw,moving:raw,sampling,start:[[1,0,0,0],[0,1,0,0],[0,0,1,0],[0,0,0,1]],stages:'nonlinear',settings:{model:6,grid:4,metric:'nc',axes:'z',regularization:1,backend:'threads'}});assert.equal(result.settings.sampling.version,1);assert(result.deformation.residualValues instanceof Float32Array);assert.equal(result.settings.sampling.fixed_exclusion.voxels,1);assert.deepEqual(result.settings.sampling.moving_windows,windows)}finally{await real.dispose()}
 }
 assert.equal(mask[3],1);assert.deepEqual(await fs.readdir(temporary),[])
 console.log('JIP regions: bounded independent mask transport, physical axes, old-runtime refusal, confirmed sampling, original-intensity preservation, cleanup and actual native worker passed')
}finally{await service.dispose();await fs.rm(temporary,{recursive:true,force:true})}
