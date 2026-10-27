import {applyAffine,type RawNifti,type Dims3} from './roiWarp'

/** Bounded percentile samples. Sparse masks must not disappear through stride aliasing. */
export function registrationIntensityRange(values:Float32Array,image?:RawNifti,excluded?:(p:Dims3)=>boolean,percentiles:[number,number]=[.01,.99]):[number,number]{
 if(!Array.isArray(percentiles)||percentiles.length!==2||!percentiles.every(Number.isFinite)||percentiles[0]<0||percentiles[1]>1||percentiles[0]>=percentiles[1])throw Error('Invalid registration intensity percentiles')
 const samples:number[]=[],stride=Math.max(1,Math.floor(values.length/12000))
 const included=(i:number)=>{
  if(!Number.isFinite(values[i]))return false
  if(!excluded||!image)return true
  const d=image.dims
  return !excluded(applyAffine(image.affine,[i%d[0],Math.floor(i/d[0])%d[1],Math.floor(i/(d[0]*d[1]))]))
 }
 for(let i=0;i<values.length;i+=stride)if(included(i))samples.push(values[i])
 if(image&&excluded&&samples.length<64&&stride>1){
  // A thin included structure can lie entirely between every strided sample.
  // Rare fallback: scan all included voxels, retaining a deterministic reservoir
  // rather than allocating a full-volume JS array or declaring the mask empty.
  samples.length=0
  let count=0,seed=0x51f15e
  for(let i=0;i<values.length;i++)if(included(i)){
   count++
   if(samples.length<12000)samples.push(values[i])
   else{seed^=seed<<13;seed^=seed>>>17;seed^=seed<<5;const slot=Math.floor((seed>>>0)/2**32*count);if(slot<samples.length)samples[slot]=values[i]}
  }
 }
 samples.sort((a,b)=>a-b)
 if(!samples.length)throw Error('Image has no included finite intensities. Check exclusion masks.')
 const lo=samples[Math.floor(samples.length*percentiles[0])],hi=samples[Math.min(samples.length-1,Math.floor(samples.length*percentiles[1]))]
 if(!(hi>lo))throw Error('Image has insufficient intensity variation.')
 return [lo,hi]
}
