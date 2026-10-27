import assert from 'node:assert/strict'
import fs from 'node:fs/promises'
import os from 'node:os'
import path from 'node:path'
import {EventEmitter} from 'node:events'
import {createHash} from 'node:crypto'
import {createJipService} from '../apps/align/desktop/jipService.mjs'
const temporary=await fs.mkdtemp(path.join(os.tmpdir(),'jip-binary-test-'))
const raw={dims:[8,8,8],affine:[[1,0,0,0],[0,1,0,0],[0,0,1,0],[0,0,0,1]],values:new Float32Array(512).fill(1)}
const bytes=Buffer.alloc(512*12),hash=createHash('sha256').update(bytes).digest('hex')
let mode='valid',child,folder
const service=createJipService({temporary,runtime:async()=>({file:'test-only-worker'}),spawnWorker:(_file,args)=>{
 child=new EventEmitter();child.stderr=new EventEmitter();child.kill=()=>child.emit('close',130)
 folder=path.dirname(args[args.indexOf('--align-output')+1])
 void (async()=>{
  const request=JSON.parse(await fs.readFile(path.join(folder,'request.json'),'utf8'));assert.equal(request.field_transport,'binary-v1')
  if(mode==='progress'){await fs.writeFile(path.join(folder,'events.jsonl'),'{"message":"first"}\n{"message":');return}
  const asset={version:1,file:'residual.f32',bytes:bytes.length,sha256:hash,encoding:'float32-le-planar'}
  if(mode==='path')asset.file='../secret.f32'
  if(mode==='checksum')asset.sha256='0'.repeat(64)
  await fs.writeFile(path.join(folder,'residual.f32'),mode==='short'?bytes.subarray(4):bytes)
  await fs.writeFile(path.join(folder,'result.json'),JSON.stringify({status:'ready',result:{deformation:{dims:[8,8,8],residualAsset:asset}}}))
  child.emit('close',0)
 })().catch(error=>child.emit('error',error));return child
}})
try{
 const result=await service.run({action:'register',fixed:raw,moving:raw,start:raw.affine,stages:'nonlinear',settings:{}})
 assert(result.deformation.residualValues instanceof Float32Array);assert.equal(result.deformation.residualValues.length,1536);assert.equal(result.deformation.residualAsset,undefined)
 for(mode of ['path','short','checksum']){await assert.rejects(()=>service.run({action:'register',fixed:raw,moving:raw,start:raw.affine,stages:'nonlinear',settings:{}}),/asset|size|checksum/);assert.deepEqual(await fs.readdir(temporary),[])}
 mode='progress';const pending=service.run({action:'register',fixed:raw,moving:raw,start:raw.affine,stages:'nonlinear',settings:{}})
 while(!folder||!await fs.stat(path.join(folder,'events.jsonl')).catch(()=>null))await new Promise(r=>setTimeout(r,5))
 assert.equal((await service.poll()).text,'first')
 await fs.appendFile(path.join(folder,'events.jsonl'),'"second"}\n')
 assert.equal((await service.poll()).text,'second');assert.equal((await service.poll()).text,'second')
 await service.cancel();child.emit('close',130);await assert.rejects(()=>pending,/cancelled/)
 assert.deepEqual(await fs.readdir(temporary),[])
 console.log('Bounded binary assets, checksum/path/length rejection, cleanup and incremental partial-event polling passed')
}finally{await service.dispose();await fs.rm(temporary,{recursive:true,force:true})}
