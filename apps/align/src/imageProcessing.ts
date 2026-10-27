import {createBinarySelection,sameBinaryRule} from './maskHoles'
import {applyAffine,invertAffine,type RawNifti,type Dims3} from './roiWarp'
import {validateAffine} from './landmarkFit'
import type {WindowBounds} from './appTypes'
export type ProcessingGeometry=Pick<RawNifti,'dims'|'affine'|'pixDims'>
export type VoxelEdit={index:number;value?:number;exclude?:boolean;afterMask?:true}
export type ThresholdRange={min:number;max:number;invert:boolean}
export type BinaryMaskRule=ThresholdRange&{fillHoles?:'3d'|'slices';edits?:{index:number;value:0|1}[]}
export type ProcessingRecipe={version:1|2|3|4|5|6;base:ProcessingGeometry;crop:WindowBounds|null;edits:VoxelEdit[];binaryMask?:BinaryMaskRule;thresholdMask?:ThresholdRange;thresholdFill?:(BinaryMaskRule&{value:number})}
export function thresholdContains(value:number,range:ThresholdRange){return Number.isFinite(value)&&(range.invert?value<range.min||value>range.max:value>=range.min&&value<=range.max)}
export function validateThresholdRange(value:unknown):ThresholdRange{const r=value as ThresholdRange;if(!r||!Number.isFinite(r.min)||!Number.isFinite(r.max)||r.min>r.max||typeof r.invert!=='boolean')throw Error('Invalid intensity mask range');return {min:r.min,max:r.max,invert:r.invert}}
export function validateBinaryMaskRule(value:unknown):BinaryMaskRule{const rule=validateThresholdRange(value),input=value as BinaryMaskRule,mode=input.fillHoles;if(mode!==undefined&&mode!=='3d'&&mode!=='slices')throw Error('Invalid hole-filling mode');const result:BinaryMaskRule=mode?{...rule,fillHoles:mode}:rule;if(input.edits!==undefined){if(!Array.isArray(input.edits)||input.edits.length>20000)throw Error('Up to 20,000 mask edits are supported');const seen=new Set<number>();for(const e of input.edits){if(!e||!Number.isSafeInteger(e.index)||e.index<0||seen.has(e.index)||(e.value!==0&&e.value!==1))throw Error('Invalid binary mask edit');seen.add(e.index)}result.edits=input.edits.map(e=>({...e}))}return result}

export const MAX_VOXEL_EDITS=20000
export function geometryMatches(a:ProcessingGeometry,b:ProcessingGeometry){return a.dims.every((n,i)=>n===b.dims[i])&&a.affine.every((r,i)=>r.every((n,j)=>Math.abs(n-b.affine[i][j])<1e-5))}
export function imageWorldBounds(raw:ProcessingGeometry):WindowBounds{const corners:number[][]=[];for(const x of [0,raw.dims[0]-1])for(const y of [0,raw.dims[1]-1])for(const z of [0,raw.dims[2]-1])corners.push(applyAffine(raw.affine,[x,y,z]));return {min:[0,1,2].map(a=>Math.min(...corners.map(p=>p[a]))) as Dims3,max:[0,1,2].map(a=>Math.max(...corners.map(p=>p[a]))) as Dims3}}
export function emptyProcessingRecipe(raw:RawNifti):ProcessingRecipe{return {version:1,base:{dims:[...raw.dims],pixDims:[...raw.pixDims],affine:raw.affine.map(r=>[...r])},crop:null,edits:[]}}
export function validateProcessingRecipe(value:unknown):ProcessingRecipe{
 const r=value as ProcessingRecipe;if(!r||![1,2,3,4,5,6].includes(r.version)||!r.base||!Array.isArray(r.base.dims)||r.base.dims.length!==3||!r.base.dims.every(n=>Number.isInteger(n)&&n>0)||!Array.isArray(r.base.pixDims)||r.base.pixDims.length!==3||!r.base.pixDims.every(n=>Number.isFinite(n)&&n>0))throw Error('Invalid processing geometry')
 validateAffine(r.base.affine);if(r.base.dims.reduce((a,b)=>a*b,1)>1.5*1024**3/4)throw Error('Processing image exceeds the 1.5 GiB array limit')
 if(r.crop&&(!Array.isArray(r.crop.min)||!Array.isArray(r.crop.max)||r.crop.min.length!==3||r.crop.max.length!==3||!r.crop.min.every((n,a)=>Number.isFinite(n)&&Number.isFinite(r.crop!.max[a])&&n<=r.crop!.max[a])))throw Error('Invalid crop bounds')
 if(!Array.isArray(r.edits)||r.edits.length>MAX_VOXEL_EDITS)throw Error(`Up to ${MAX_VOXEL_EDITS} individual voxel edits are supported`)
 const seen=new Set<number>();for(const e of r.edits){if(!Number.isInteger(e.index)||e.index<0||e.index>=r.base.dims.reduce((a,b)=>a*b,1)||seen.has(e.index)||(e.value===undefined&&e.exclude===undefined)||(e.value!==undefined&&(!Number.isFinite(e.value)||Math.abs(e.value)>3.402823466e38))||(e.exclude!==undefined&&typeof e.exclude!=='boolean'))throw Error('Invalid voxel edit');if(e.afterMask!==undefined&&(e.afterMask!==true||r.version<6||e.value===undefined))throw Error('Final paint overrides require recipe6 and a finite intensity');seen.add(e.index)}
 if(r.version===1&&(r.thresholdMask!==undefined||r.thresholdFill!==undefined))throw Error('Intensity masks require processing recipe version 2')
 if(r.thresholdMask!==undefined)validateThresholdRange(r.thresholdMask)
 if(r.thresholdFill!==undefined){validateThresholdRange(r.thresholdFill);if(!Number.isFinite(r.thresholdFill.value)||Math.abs(r.thresholdFill.value)>3.402823466e38)throw Error('Invalid mask fill value')}
 if(r.binaryMask!==undefined){if(r.version!==3&&r.version!==4&&r.version!==5&&r.version!==6)throw Error('Binary masks require processing recipe version 3');validateThresholdRange(r.binaryMask)}
 for(const rule of [r.binaryMask,r.thresholdFill])if(rule){validateBinaryMaskRule(rule);if(rule.edits&&(r.version<5||rule.edits.some(e=>e.index>=r.base.dims.reduce((a,b)=>a*b,1))))throw Error('Mask edits require recipe5 and matching original working geometry');if(rule.fillHoles&&r.version<4)throw Error('Hole filling requires processing recipe version 4')}
 return structuredClone(r)
}
export function processImage(base:RawNifti,recipe:ProcessingRecipe):{raw:RawNifti;excluded:Uint8Array;focus:Dims3;binarySelection?:Uint8Array}{
 const r=validateProcessingRecipe(recipe);if(!geometryMatches(base,r.base)||base.values.length!==base.dims.reduce((a,b)=>a*b,1))throw Error('Processing requires the original working resolution and geometry. Restore original before changing resolution.')
 const binarySelection=(r.binaryMask?.fillHoles||r.binaryMask?.edits?.length)?createBinarySelection(base,r,r.binaryMask!):undefined,fillSelection=(r.thresholdFill?.fillHoles||r.thresholdFill?.edits?.length)?(binarySelection&&r.binaryMask&&sameBinaryRule(r.binaryMask,r.thresholdFill)?binarySelection:createBinarySelection(base,r,r.thresholdFill!)):undefined
 const low:Dims3=[0,0,0],high=base.dims.map(n=>n-1) as Dims3
 if(r.crop){const inv=invertAffine(base.affine),corners:number[][]=[];for(const x of [r.crop.min[0],r.crop.max[0]])for(const y of [r.crop.min[1],r.crop.max[1]])for(const z of [r.crop.min[2],r.crop.max[2]])corners.push(applyAffine(inv,[x,y,z]));for(let a=0;a<3;a++){low[a]=Math.max(0,Math.ceil(Math.min(...corners.map(p=>p[a]))-1e-6));high[a]=Math.min(high[a],Math.floor(Math.max(...corners.map(p=>p[a]))+1e-6))}}
 const dims=low.map((n,a)=>high[a]-n+1) as Dims3;if(dims.some(n=>n<1))throw Error('Crop contains no voxel centers. Enlarge the crop.')
 const affine=base.affine.map(row=>[...row]),origin=applyAffine(base.affine,low);for(let a=0;a<3;a++)affine[a][3]=origin[a]
 const edits=new Map(r.edits.map(e=>[e.index,e])),values=new Float32Array(dims.reduce((a,b)=>a*b,1)),excluded=new Uint8Array(values.length);let kept=0,bestDistance=Infinity;let focus:Dims3=[0,0,0]
 for(let z=0;z<dims[2];z++)for(let y=0;y<dims[1];y++)for(let x=0;x<dims[0];x++){
  const q:[number,number,number]=[x+low[0],y+low[1],z+low[2]],i=q[0]+base.dims[0]*(q[1]+base.dims[1]*q[2]),j=x+dims[0]*(y+dims[1]*z),world=r.crop?applyAffine(base.affine,q):null
  if(world&&r.crop&&world.some((v,a)=>v<r.crop!.min[a]-1e-6||v>r.crop!.max[a]+1e-6)){excluded[j]=1;continue}
  kept++;const distance=[x,y,z].reduce((sum,n,a)=>sum+(n-(dims[a]-1)/2)**2,0);if(distance<bestDistance){bestDistance=distance;focus=applyAffine(affine,[x,y,z])}
  const edit=edits.get(i);const intensity=edit?.value??base.values[i];values[j]=r.thresholdFill&&!(fillSelection?fillSelection[i]:thresholdContains(intensity,r.thresholdFill))?r.thresholdFill.value:intensity;if(edit?.afterMask)values[j]=edit.value!;excluded[j]=edit?.exclude||r.thresholdMask&&!thresholdContains(intensity,r.thresholdMask)?1:0
 }
 if(!kept)throw Error('Crop contains no voxel centers. Enlarge the crop.')
 return {raw:{...base,dims,affine,pixDims:[...base.pixDims],values,datatypeCode:16,slope:1,intercept:0,frameCount:1},excluded,focus,binarySelection:binarySelection??(!r.binaryMask?fillSelection:undefined)}
}
export function voxelAtWorld(raw:ProcessingGeometry,world:[number,number,number]){const q=applyAffine(invertAffine(raw.affine),world).map(Math.round) as Dims3;if(q.some((n,a)=>!Number.isFinite(n)||n<0||n>=raw.dims[a]))return null;return {voxel:q,index:q[0]+raw.dims[0]*(q[1]+raw.dims[1]*q[2])}}

/** Keep a visible crosshair; when a crop removes it, use a real retained voxel. */
export function processingViewPoint(previous:ProcessingGeometry,next:ProcessingGeometry,world:Dims3,crop:WindowBounds|null,focus:Dims3):Dims3{
 // An oblique physical crop can remove corners without changing the voxel grid.
 if(geometryMatches(previous,next)&&!crop&&world.every(Number.isFinite))return world
 const q=applyAffine(invertAffine(next.affine),world)
 return q.some((v,a)=>!Number.isFinite(v)||v< -1e-6||v>next.dims[a]-1+1e-6)||(crop&&world.some((v,a)=>v<crop.min[a]-1e-6||v>crop.max[a]+1e-6))?[...focus]:world
}
