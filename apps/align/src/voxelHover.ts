import {voxelAtWorld,thresholdContains,type VoxelEdit} from './imageProcessing'
import type {Loaded,Modality} from './appTypes'
import type {Vec3} from './rigid'
import {imageLabel} from './registrationPairing'
const editCache=new WeakMap<Loaded,Map<number,number|undefined>>()
export function voxelHoverText(item:Loaded,modality:Modality,world:Vec3,mask=false,pending?:VoxelEdit):string|null{
 const visible=voxelAtWorld(item.raw,world),base=item.processing?.base??item.raw,p=voxelAtWorld(base,world);if(!visible||!p)return null
 const value=item.raw.values[visible.index],format=(v:number)=>Number.isFinite(v)?Number(v.toPrecision(7)).toString():String(v)
 let text=`${imageLabel(modality)} · voxel ${visible.voxel.join(', ')}\nIntensity: ${format(value)}`
 if(mask){const r=item.processing?.recipe,rule=r?.binaryMask??r?.thresholdFill;if(!rule)return text;let selection=item.processing?.binarySelection?.[p.index];if(selection===undefined){let edits=editCache.get(item);if(!edits){edits=new Map(r?.edits.map(e=>[e.index,e.value]));editCache.set(item,edits)}selection=thresholdContains(edits.get(p.index)??base.values[p.index],rule)?1:0;const crop=r?.crop;if(crop&&world.some((v,a)=>v<crop.min[a]-1e-6||v>crop.max[a]+1e-6))selection=0}text+=`\nMask: ${selection}`}
 if(pending?.value!==undefined)text+=` · Pending ${mask?'mask':'intensity'}: ${format(pending.value)}`
 else if(pending?.exclude!==undefined)text+=pending.exclude?' · Pending: excluded':' · Pending: included'
 return text
}
