import assert from 'node:assert/strict'
import {downsampleByFactor,downsampleForRegistration,resolutionLevels,shrinkFactorLevels} from '../packages/imaging-math/imagePyramid'
import {applyAffine,type RawNifti,type Dims3} from '../packages/imaging-math/roiWarp'
function image(dims:Dims3):RawNifti {
 const affine=[[0,-1.7,.2,23],[.8,0,.3,-42],[.1,.2,2.5,11],[0,0,0,1]]
 return {dims,affine,pixDims:[0,1,2].map(a=>Math.hypot(...affine.slice(0,3).map(r=>r[a]))) as Dims3,values:Float32Array.from({length:dims.reduce((a,b)=>a*b,1)},(_,i)=>Math.sin(i*.3)*100+i*.01),frameCount:1,slope:1,intercept:0,datatypeCode:16,littleEndian:true}
}
// Independent reference: convolve the full line at each integer source location,
// then interpolate that filtered line onto the endpoint-preserving output grid.
function reference(source:RawNifti,factor:number,sigma:number){
 let values=source.values,dims=[...source.dims] as Dims3
 const target=dims.map(n=>n===1?1:Math.max(2,Math.floor((n-1)/factor)+1)) as Dims3
 const radius=sigma<.25?0:Math.max(1,Math.ceil(sigma*3)),kernel=Float32Array.from({length:radius*2+1},(_,i)=>Math.exp(-.5*((i-radius)/(sigma||1))**2)),sum=kernel.reduce((a,b)=>a+b,0)
 // Production normalization uses the unrounded exponential sum.
 let exactSum=0;for(let k=-radius;k<=radius;k++)exactSum+=Math.exp(-.5*(k/(sigma||1))**2)
 for(let i=0;i<kernel.length;i++)kernel[i]/=exactSum
 assert(sum>0)
 for(let axis=0;axis<3;axis++){
  const next=[...dims] as Dims3;next[axis]=target[axis]
  if(next[axis]===dims[axis]&&radius===0)continue
  const result=new Float32Array(next.reduce((a,b)=>a*b,1)),spacing=target[axis]===1?1:(source.dims[axis]-1)/(target[axis]-1)
  const index=(p:number[])=>p[0]+dims[0]*(p[1]+dims[1]*p[2])
  let out=0
  for(let z=0;z<next[2];z++)for(let y=0;y<next[1];y++)for(let x=0;x<next[0];x++){
   const p=[x,y,z],coordinate=p[axis]*spacing,lo=Math.floor(coordinate),f=coordinate-lo
   const filtered=(center:number)=>{let v=0;for(let k=-radius;k<=radius;k++){p[axis]=Math.max(0,Math.min(dims[axis]-1,center+k));v+=values[index(p)]*kernel[k+radius]}return v}
   const a=filtered(lo);result[out++]=f===0?a:a*(1-f)+filtered(lo+1)*f
  }
  values=result;dims=next
 }
 return values
}
for(const dims of [[17,14,11],[1,13,9],[7,1,1],[2,2,2],[1,1,1]] as Dims3[])for(const factor of [1,2,4,8])for(const sigma of [0,.1,.5,2]){
 const source=image(dims),original=source.values.slice(),result=downsampleByFactor(source,factor,sigma),expected=reference(source,factor,sigma)
 assert.equal(result.values.length,expected.length)
 for(let i=0;i<expected.length;i++)assert(Math.abs(result.values[i]-expected[i])<3e-5,`${dims}/${factor}/${sigma}/${i}`)
 assert.deepEqual(source.values,original)
 assert.deepEqual(applyAffine(result.affine,[0,0,0]),applyAffine(source.affine,[0,0,0]))
 assert.deepEqual(applyAffine(result.affine,result.dims.map(n=>n-1) as Dims3),applyAffine(source.affine,source.dims.map(n=>n-1) as Dims3))
}
for(const invalid of [NaN,Infinity,-Infinity])for(const axis of [0,1,2]){
 const source=image([5,5,5]);source.values.fill(42);source.values[[1,5,25][axis]]=invalid
 assert.equal(downsampleByFactor(source,2,0).values[0],42,'zero-weight neighbor must not propagate')
 assert(Number.isNaN(downsampleByFactor(source,2,.5).values[0])||!Number.isFinite(downsampleByFactor(source,2,.5).values[0]),'positive support must retain invalidity')
 source.values[0]=invalid;assert(!Number.isFinite(downsampleByFactor(source,2,0).values[0]))
}
for(const custom of ['8,nope,4,1','8,Infinity,1','8,,1','',',','8,NaN,1'])assert.throws(()=>shrinkFactorLevels('custom',custom))
for(const custom of ['4,nope,2,0','4,Infinity,0','4,,0','',',','NaN'])assert.throws(()=>resolutionLevels('custom',custom))
assert.deepEqual(shrinkFactorLevels('custom','8,4,2,1'),[8,4,2,1]);assert.deepEqual(resolutionLevels('custom','4,2,0'),[4,2,0])
for(const sigma of [-1,NaN,Infinity])assert.throws(()=>downsampleByFactor(image([5,5,5]),2,sigma))
const source=image([17,14,11]);assert.equal(downsampleForRegistration(source,0),source);assert(downsampleForRegistration(source,3).pixDims.every((n,a)=>n>=source.pixDims[a]))
console.log('Pyramid audit: 80 oblique/anisotropic/singleton finite reference cases, exact physical endpoints, immutable input, nonfinite support, invalid custom levels and smoothing validation passed.')

// A physical inclusion plane at x=1 is missed by every 10th flat sample.
// The old range estimator declared this nonempty, varying region empty.
const maskImage=image([128,32,32]);maskImage.affine=[[1,0,0,0],[0,1,0,0],[0,0,1,0],[0,0,0,1]];maskImage.pixDims=[1,1,1]
const {registrationIntensityRange}=await import('../packages/imaging-math/registrationIntensityRange')
const {makeIntensityScorer}=await import('../packages/imaging-math/intensityRegistration')
const excluded=new Uint8Array(maskImage.values.length).fill(1)
for(let z=0;z<32;z++)for(let y=0;y<32;y++)excluded[1+128*(y+32*z)]=0
const range=registrationIntensityRange(maskImage.values,maskImage,p=>p[0]!==1);assert(range[1]>range[0])
const mask={...maskImage,values:excluded},score=makeIntensityScorer(maskImage,maskImage,'least-squares',32,[],[],18000,undefined,undefined,mask,mask)
assert(Math.abs(score([[1,0,0,0],[0,1,0,0],[0,0,1,0],[0,0,0,1]]))<1e-12)
assert.throws(()=>registrationIntensityRange(maskImage.values,maskImage,()=>true),/no included finite/)
assert.throws(()=>registrationIntensityRange(new Float32Array(maskImage.values.length).fill(1),maskImage,p=>p[0]!==1),/insufficient intensity variation/)
console.log('Masked intensity range: sparse physical-plane stride aliasing falls back to bounded included-voxel sampling; nonempty varying masks register, empty/constant masks still reject.')
