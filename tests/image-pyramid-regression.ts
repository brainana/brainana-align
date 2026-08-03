import assert from 'node:assert/strict'
import type { RawNifti } from '../source/src/domain/imaging/roiWarp.ts'
import { downsampleByFactor, downsampleForRegistration, resolutionLevels, shrinkFactorLevels } from '../source/src/domain/registration/imagePyramid.ts'

const dims:[number,number,number]=[17,13,9]
const values=new Float32Array(dims[0]*dims[1]*dims[2])
for(let z=0;z<dims[2];z++)for(let y=0;y<dims[1];y++)for(let x=0;x<dims[0];x++)values[x+dims[0]*(y+dims[1]*z)]=x+10*y+100*z
const raw:RawNifti={dims,frameCount:1,affine:[[1,0,0,12],[0,2,0,-8],[0,0,3,4],[0,0,0,1]],pixDims:[1,2,3],values,datatypeCode:16,littleEndian:true,slope:1,intercept:0}
const reduced=downsampleForRegistration(raw,4)
assert.deepEqual(reduced.dims,[5,7,7])
const apply=(matrix:number[][],point:[number,number,number])=>[0,1,2].map(row=>matrix[row][0]*point[0]+matrix[row][1]*point[1]+matrix[row][2]*point[2]+matrix[row][3])
assert.deepEqual(apply(reduced.affine,[0,0,0]),apply(raw.affine,[0,0,0]))
const oldEnd=apply(raw.affine,[dims[0]-1,dims[1]-1,dims[2]-1])
const newEnd=apply(reduced.affine,reduced.dims.map(value=>value-1) as [number,number,number])
assert.ok(Math.hypot(...oldEnd.map((value,index)=>value-newEnd[index]))<1e-6)
assert.deepEqual(resolutionLevels('fast'),[4])
assert.deepEqual(resolutionLevels('balanced'),[4,2])
assert.deepEqual(resolutionLevels('precise'),[4,2,0])
assert.deepEqual(resolutionLevels('custom','2, 4, 0, 2'),[4,2,0])
assert.throws(()=>resolutionLevels('custom','20'),/0 through 12/)
const factorReduced=downsampleByFactor(raw,4,0)
assert.deepEqual(factorReduced.dims,[5,4,3])
assert.deepEqual(apply(factorReduced.affine,[0,0,0]),apply(raw.affine,[0,0,0]))
const factorEnd=apply(factorReduced.affine,factorReduced.dims.map(value=>value-1) as [number,number,number])
assert.ok(Math.hypot(...oldEnd.map((value,index)=>value-factorEnd[index]))<1e-6)
assert.deepEqual(shrinkFactorLevels('standard'),[8,4,2,1])
console.log('physical-space image pyramid regression checks passed')
