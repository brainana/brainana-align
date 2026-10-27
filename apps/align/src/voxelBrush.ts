import {applyAffine,type Dims3,type RawNifti} from './roiWarp'
import {MAX_VOXEL_EDITS,type ProcessingRecipe,type VoxelEdit} from './imageProcessing'
type Delta=Map<number,VoxelEdit|undefined>
/** Sparse editing transaction: pointer events never copy/reprocess a volume. */
export class VoxelBrushDraft{
 readonly changes=new Map<number,VoxelEdit>();private undo:Delta[]=[];private redo:Delta[]=[];private stroke:Delta|null=null
 private added=0;private original:Map<number,VoxelEdit>
 constructor(readonly base:RawNifti,readonly recipe:ProcessingRecipe,readonly finalPaint=false){this.original=new Map(recipe.edits.map(e=>[e.index,e]))}
 get pending(){return this.changes.size>0}
 get canUndo(){return this.undo.length>0}get canRedo(){return this.redo.length>0}
 begin(){this.stroke=new Map()}
 end(){if(this.stroke?.size){this.undo.push(this.stroke);this.undo=this.undo.slice(-32);this.redo=[]}this.stroke=null}
 private set(index:number,value:VoxelEdit|undefined){if(!this.original.has(index)){if(value&&!this.changes.has(index))this.added++;else if(!value&&this.changes.has(index))this.added--}if(value)this.changes.set(index,value);else this.changes.delete(index)}
 paint(center:Dims3,radius:number,action:string,value:number){
  if(!Number.isInteger(radius)||radius<0||radius>16||!['replace','exclude','include'].includes(action)||action==='replace'&&(!Number.isFinite(value)||Math.abs(value)>3.402823466e38))throw Error('Choose a valid brush size, action and finite replacement value.')
  const d=this.base.dims,crop=this.recipe.crop,candidates:number[]=[]
  for(let z=Math.max(0,center[2]-radius);z<=Math.min(d[2]-1,center[2]+radius);z++)for(let y=Math.max(0,center[1]-radius);y<=Math.min(d[1]-1,center[1]+radius);y++)for(let x=Math.max(0,center[0]-radius);x<=Math.min(d[0]-1,center[0]+radius);x++){
   if(radius&&((x-center[0])**2+(y-center[1])**2+(z-center[2])**2>radius**2))continue
   if(crop&&applyAffine(this.base.affine,[x,y,z]).some((n,a)=>n<crop.min[a]-1e-6||n>crop.max[a]+1e-6))continue
   candidates.push(x+d[0]*(y+d[1]*z))
  }
  const added=candidates.filter(i=>!this.original.has(i)&&!this.changes.has(i)).length
  if(this.original.size+this.added+added>MAX_VOXEL_EDITS)throw Error(`This brush would exceed ${MAX_VOXEL_EDITS.toLocaleString()} edited voxels. Undo/discard part of the batch or use masking for larger regions.`)
  if(!this.stroke)this.begin()
  for(const index of candidates){const prior=this.changes.get(index),edit={...(prior??this.original.get(index)??{index})};if(action==='replace'){edit.value=value;if(this.finalPaint)edit.afterMask=true}else edit.exclude=action==='exclude';if(JSON.stringify(edit)===JSON.stringify(prior??this.original.get(index)))continue;if(!this.stroke!.has(index))this.stroke!.set(index,prior);this.set(index,edit)}
  return candidates.length
 }
 step(undo=true){this.end();const from=undo?this.undo:this.redo,to=undo?this.redo:this.undo,delta=from.pop();if(!delta)return;const reverse:Delta=new Map();for(const [i,e] of delta){reverse.set(i,this.changes.get(i));this.set(i,e)}to.push(reverse)}
 finish():ProcessingRecipe{this.end();const edits=[...new Map([...this.original,...this.changes]).values()];return {...this.recipe,version:edits.some(e=>e.afterMask)?6:this.recipe.version,edits}}
}
