import type {RawNifti} from './roiWarp'
import {applyAffine} from './roiWarp'
import type {ProcessingRecipe,BinaryMaskRule} from './imageProcessing'
import {thresholdContains} from './imageProcessing'

/** Build original-working-grid selection in bounded chunks. State 3 protects
 * nonfinite values/outside-crop voxels from ever becoming selected by filling. */
export function thresholdSelection(base:RawNifti,recipe:ProcessingRecipe,rule:BinaryMaskRule,mask:Uint8Array=new Uint8Array(base.values.length),start=0,end=mask.length):Uint8Array{
 const edits=new Map(recipe.edits.map(e=>[e.index,e.value])),[nx,ny]=base.dims,crop=recipe.crop
 for(let i=start;i<Math.min(end,mask.length);i++){
  const value=edits.get(i)??base.values[i]
  if(!Number.isFinite(value)){mask[i]=3;continue}
  if(crop){const world=applyAffine(base.affine,[i%nx,Math.floor(i/nx)%ny,Math.floor(i/(nx*ny))]);if(world.some((v,a)=>v<crop.min[a]-1e-6||v>crop.max[a]+1e-6)){mask[i]=3;continue}}
  mask[i]=thresholdContains(value,rule)?1:0
 }
 return mask
}

/** Exterior flood fill with face connectivity (6 in 3D / 4 within voxel-Z
 * slices). Scanline spans avoid a voxel-sized integer queue. Work-stack pages
 * are capped at 32 MiB; exceeding this rejects transactionally, never approximates.
 * No dilation: original foreground and outer boundaries are preserved. */
export function fillMaskHoles(mask:Uint8Array,dims:readonly number[],mode?:BinaryMaskRule['fillHoles']):Uint8Array{
 const [nx,ny,nz]=dims,nxy=nx*ny
 if(dims.length!==3||dims.some(n=>!Number.isSafeInteger(n)||n<1)||mask.length!==nxy*nz||mask.length>0xffffffff)throw Error('Invalid binary-mask geometry')
 if(!mode){for(let i=0;i<mask.length;i++)if(mask[i]===3)mask[i]=0;return mask}
 if(mode!=='3d'&&mode!=='slices')throw Error('Unknown hole-filling mode')
 const pages:Uint32Array[]=[],pageSize=65536,maxEntries=8*1024*1024;let top=0
 const put=(v:number)=>{if(top>=maxEntries)throw Error('Hole filling exceeded its 32 MiB work-stack limit. Use a coarser working resolution.');const page=Math.floor(top/pageSize);if(!pages[page])pages[page]=new Uint32Array(pageSize);pages[page][top++%pageSize]=v}
 const pop=()=>{--top;return pages[Math.floor(top/pageSize)][top%pageSize]}
 const open=(i:number)=>mask[i]===0||mask[i]===3
 const seed=(i:number)=>{if(!open(i))return i;const row=Math.floor(i/nx)*nx;let left=i,right=i;while(left>row&&open(left-1))left--;while(right<row+nx-1&&open(right+1))right++;mask.fill(2,left,right+1);put(left);put(right);return right}
 for(let z=0;z<nz;z++){
  for(let y=0;y<ny;y++){const row=z*nxy+y*nx;seed(row);seed(row+nx-1)}
  for(let x=0;x<nx;x++){seed(z*nxy+x);seed(z*nxy+(ny-1)*nx+x)}
 }
 if(mode==='3d')for(const z of [0,nz-1])for(let y=0;y<ny;y++)for(let x=0;x<nx;x++)x=seed(z*nxy+y*nx+x)-z*nxy-y*nx
 // Protected voxels are exterior roots even if enclosed by a threshold shell.
 for(let i=0;i<mask.length;i++)if(mask[i]===3)i=seed(i)
 while(top){const right=pop(),left=pop(),y=Math.floor(left/nx)%ny,z=Math.floor(left/nxy)
  const offsets:number[]=[];if(y>0)offsets.push(-nx);if(y<ny-1)offsets.push(nx);if(mode==='3d'){if(z>0)offsets.push(-nxy);if(z<nz-1)offsets.push(nxy)}
  for(const offset of offsets)for(let i=left+offset;i<=right+offset;i++)if(open(i))i=seed(i)
 }
 for(let i=0;i<mask.length;i++)mask[i]=mask[i]===2?0:1
 return mask
}
/** Manual corrections are applied AFTER hole filling, independently for each saved mask snapshot. */
export function eligibleMaskEdits(base:RawNifti,recipe:ProcessingRecipe,rule:BinaryMaskRule){const values=new Map(recipe.edits.map(e=>[e.index,e.value])),crop=recipe.crop,[nx,ny]=base.dims;return (rule.edits??[]).filter(e=>{if(!e.value)return true;if(!Number.isFinite(values.get(e.index)??base.values[e.index]))return false;if(!crop)return true;const p=applyAffine(base.affine,[e.index%nx,Math.floor(e.index/nx)%ny,Math.floor(e.index/(nx*ny))]);return p.every((v,a)=>v>=crop.min[a]-1e-6&&v<=crop.max[a]+1e-6)})}
export function createBinarySelection(base:RawNifti,recipe:ProcessingRecipe,rule:BinaryMaskRule):Uint8Array{const selection=fillMaskHoles(thresholdSelection(base,recipe,rule),base.dims,rule.fillHoles);for(const e of eligibleMaskEdits(base,recipe,rule))selection[e.index]=e.value;return selection}
export function sameBinaryRule(a:BinaryMaskRule,b:BinaryMaskRule){return a.min===b.min&&a.max===b.max&&a.invert===b.invert&&a.fillHoles===b.fillHoles&&JSON.stringify(a.edits??[])===JSON.stringify(b.edits??[])}
