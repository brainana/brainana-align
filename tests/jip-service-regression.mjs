import assert from 'node:assert/strict'
import path from 'node:path'
import fs from 'node:fs/promises'
import os from 'node:os'
import {createJipService,niftiBuffer} from '../apps/align/desktop/jipService.mjs'
const temporary=await fs.mkdtemp(path.join(os.tmpdir(),'jip-service-test-')),embedded=path.resolve('generated/scientific-runtime',process.platform==='win32'?'BrainanaAlignRegistration.exe':'BrainanaAlignRegistration')
const identity=[[1,0,0,0],[0,1,0,0],[0,0,1,0],[0,0,0,1]],raw={dims:[13,15,17],affine:identity,values:new Float32Array(13*15*17)}
for(let z=0;z<17;z++)for(let y=0;y<15;y++)for(let x=0;x<13;x++)raw.values[x+13*(y+15*z)]=1+70*Math.exp(-((x-5)**2/12+(y-6)**2/14+(z-8)**2/16))
const snapshot=raw.values.slice(),runtime=async()=>({file:embedded,args:[]}),service=createJipService({temporary,runtime})
const request={action:'register',fixed:raw,moving:raw,stages:'nonlinear',start:identity,settings:{model:6,metric:'nc',axes:'z',regularization:1,grid:4}}
try{
 for(const invalid of [{...request,stages:'both'},{...request,stages:'affine'},{...request,start:undefined},{...request,start:[[NaN]]}])await assert.rejects(()=>service.run(invalid),/accepted linear alignment/);assert.deepEqual(await fs.readdir(temporary),[]);assert.equal(service.running(),false)
 const b=niftiBuffer(raw);assert.equal(b.readInt32LE(),348);assert.equal(b.length,352+4*raw.values.length);assert.equal(b.readFloatLE(352),raw.values[0]);assert.throws(()=>niftiBuffer({...raw,affine:[[0,0,0,0],[0,1,0,0],[0,0,1,0],[0,0,0,1]]}),/Singular/)
 const pending=service.run(request);await assert.rejects(()=>service.run(request),/already/);const result=await pending;assert.equal(result.deformation.kernel,'jip-dense-pull');assert.deepEqual(raw.values,snapshot);assert.equal(service.running(),false);assert.deepEqual(await fs.readdir(temporary),[])
 await assert.rejects(()=>service.run({...request,settings:{model:99}}),/6-|parameter/);assert.deepEqual(await fs.readdir(temporary),[])
 const cancelled=service.run(request);await new Promise(r=>setTimeout(r,25));await service.cancel();await assert.rejects(()=>cancelled,/cancelled/);assert.deepEqual(await fs.readdir(temporary),[])
 await assert.rejects(()=>service.run({...request,action:'shell',command:'anything'}),/Unknown/)
 console.log('JIP domain service real native run, process exit, input immutability, singular execution, validation/error/cancel cleanup and fixed actions passed')
}finally{await service.dispose();await fs.rm(temporary,{recursive:true,force:true})}
