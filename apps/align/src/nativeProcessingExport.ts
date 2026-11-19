import {createBinarySelection} from './maskHoles'
import {applyAffine,invertAffine,type RawNifti} from './roiWarp'
import type {ProcessingRecipe} from './imageProcessing'
/** Replay the saved working-grid segmentation/edit footprints in native physical space.
 * Mutates only a worker-owned source buffer. Registration exclusions do not erase data. */
export function nativeProcessingValue(source:Pick<RawNifti,'dims'|'affine'>,base:RawNifti,recipe:ProcessingRecipe){
 const finalEdits=new Set(recipe.edits.filter(e=>e.afterMask).map(e=>e.index)),inv=invertAffine(base.affine),edits=new Map(recipe.edits.filter(e=>e.value!==undefined).map(e=>[e.index,e.value!])),selection=recipe.thresholdFill?createBinarySelection(base,recipe,recipe.thresholdFill):null,[nx,ny]=source.dims
 return (value:number,i:number):number=>{
  const world=applyAffine(source.affine,[i%nx,Math.floor(i/nx)%ny,Math.floor(i/(nx*ny))]);if(recipe.crop&&world.some((v,a)=>v<recipe.crop!.min[a]-1e-6||v>recipe.crop!.max[a]+1e-6))return 0
  const q=applyAffine(inv,world).map(Math.round);if(q.some((v,a)=>v<0||v>=base.dims[a]))return selection?recipe.thresholdFill!.value:value
  const j=q[0]+base.dims[0]*(q[1]+base.dims[1]*q[2]),edited=edits.get(j);if(edited!==undefined)value=edited
  if(selection&&!selection[j])value=recipe.thresholdFill!.value;if(finalEdits.has(j))value=edits.get(j)!
  return value
 }
}
export function applyNativeProcessing(source:RawNifti,base:RawNifti,recipe:ProcessingRecipe,progress:(s:string)=>void=()=>{}){
 const process=nativeProcessingValue(source,base,recipe),[nx,ny,nz]=source.dims
 for(let z=0;z<nz;z++){for(let y=0;y<ny;y++)for(let x=0;x<nx;x++){const i=x+nx*(y+ny*z);source.values[i]=process(source.values[i],i)}if(z%8===0)progress(`Applying working-grid mask/crop/edits to native intensities: ${z+1}/${nz}…`)}
 return source
}
