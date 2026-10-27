import assert from 'node:assert/strict'
import { createRequire } from 'node:module'
import { pathToFileURL } from 'node:url'
import { rawFromNVImage } from '../apps/align/src/imageGeometry.ts'
import { makeResampledImage } from '../apps/align/src/outputImage.ts'
import { applyMat4, multiplyMat4, rigidDelta, type Mat4, type Vec3 } from '../apps/align/src/rigid.ts'
import { fitLandmarks, type LandmarkModel } from '../apps/align/src/landmarkFit.ts'
const require=createRequire(new URL('../apps/align/package.json',import.meta.url))
const {NVImage}=await import(new URL('../node_modules/@niivue/niivue/build/niivue/index.js',import.meta.url).href)
const nifti=await import(new URL('../node_modules/nifti-reader-js/dist/nifti.js',import.meta.url).href)
function close(a:number[][],b:number[][],eps=1e-5){a.forEach((row,i)=>row.forEach((v,j)=>assert.ok(Math.abs(v-b[i][j])<eps,`${i},${j}: ${v} vs ${b[i][j]}`)))}
const dims=[7,8,9], pix=[0.8,1.1,1.7]
const affine:Mat4=[[0,-0.8,0.15,42],[1.1,0.05,0.2,-37],[-0.1,0.25,1.7,18],[0,0,0,1]]
const voxels:Vec3[]=[[1,1,1],[5,1,2],[2,6,1],[1,2,7],[5,6,7],[3,4,2]]
const vals=Float32Array.from({length:7*8*9},(_,i)=>i+1)
function bytes(){return NVImage.createNiftiArray(dims,pix,affine.flat(),16,vals)}
const sformImage=await NVImage.new(bytes().buffer,'oblique-sform.nii')
const sform=rawFromNVImage(sformImage)
close(sform.affine,affine)
// Independent quaternion formula and qfac generate a known oblique qform.
const q=[0.3,0.4,0.5,Math.sqrt(0.5)]
const [a,b,c,d]=q
const qr=[[a*a+b*b-c*c-d*d,2*b*c-2*a*d,2*b*d+2*a*c],[2*b*c+2*a*d,a*a+c*c-b*b-d*d,2*c*d-2*a*b],[2*b*d-2*a*c,2*c*d+2*a*b,a*a+d*d-c*c-b*b]]
const expectedQ:Mat4=[...qr.map((row,i)=>[...row.map((v,j)=>v*pix[j]*(j===2?-1:1)),[12,-9,5][i]]),[0,0,0,1]]
const qbytes=bytes();const view=new DataView(qbytes.buffer)
view.setInt16(252,1,true);view.setInt16(254,0,true);view.setFloat32(76,-1,true)
;[b,c,d,12,-9,5].forEach((v,i)=>view.setFloat32(256+4*i,v,true))
const qimage=await NVImage.new(qbytes.buffer,'oblique-qform.nii')
const qform=rawFromNVImage(qimage);close(qform.affine,expectedQ)
const truth=rigidDelta([12,-7,4,102,-41,158],[8,-12,27])
for(const raw of [sform,qform]) {
 const world=voxels.map(p=>applyMat4(raw.affine,p)), target=world.map(p=>applyMat4(truth,p))
 close(fitLandmarks(world,target).matrix,truth)
 close(fitLandmarks(target,world).inverse,truth)
}
for(const [units,factor] of [[1,1000],[3,0.001]]) {
 const data=bytes();new DataView(data.buffer).setUint8(123,units)
 const image=await NVImage.new(data.buffer,'units.nii');const raw=rawFromNVImage(image)
 close(raw.affine,affine.map((row,i)=>row.map(v=>i<3?v*factor:v)),factor*1e-5)
 assert.equal(image.hdr.xyzt_units&7,2)
 close(image.getAffine(),raw.affine)
}
const source={name:'moving.nii',raw:sform,nvImage:sformImage,sourceFiles:['moving.nii']}
const rotation=rigidDelta([3,-6,9,87,-73,112],[0,0,0])
for(const [model,linear] of Object.entries({similarity:[[1.7,0,0,0],[0,1.7,0,0],[0,0,1.7,0],[0,0,0,1]],anisotropic:[[0.7,0,0,0],[0,1.8,0,0],[0,0,2.3,0],[0,0,0,1]],affine:[[0.8,0.2,-0.3,0],[0,1.7,0.4,0],[0,0,2.1,0],[0,0,0,1]]})) {
 const transform=multiplyMat4(rotation,linear),targetAffine=multiplyMat4(transform,sform.affine)
 const targetData=NVImage.createNiftiArray(dims,pix,targetAffine.flat(),16,vals)
 const targetImage=await NVImage.new(targetData.buffer,'fixed.nii')
 const target={name:'fixed.nii',raw:rawFromNVImage(targetImage),nvImage:targetImage,sourceFiles:['fixed.nii']}
 const output=makeResampledImage(source,target,transform)
 const data=output.toUint8Array();const header=nifti.readHeader(data.buffer)
 assert.equal(header.qform_code,0);assert.equal(header.sform_code,2)
 assert.equal(header.datatypeCode,16);assert.equal(header.scl_slope,1);assert.equal(header.scl_inter,0)
 close(header.affine,target.raw.affine)
 const restored=await NVImage.new(data.buffer,`${model}.nii`)
 close(restored.getAffine(),target.raw.affine)
 const restoredRaw=rawFromNVImage(restored)
 for(let k=1;k<8;k++)for(let j=1;j<7;j++)for(let i=1;i<6;i++) {
   const idx=i+7*(j+8*k)
   assert.ok(Math.abs(restoredRaw.values[idx]-vals[idx])<1e-3,`${model} resampling at ${idx}`)
 }
}
console.log('NIfTI geometry/export regressions passed: oblique qform, negative qfac, sform, anisotropic voxels, meter/micron units, both directions, similarity/anisotropic/affine NIfTI serialization and reload')
