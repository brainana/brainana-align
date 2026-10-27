import type {Loaded,View} from './appTypes'
import {applyAffine,invertAffine} from './roiWarp'
import {windowPointFromCanvas} from './optimizationWindowGeometry'
import {thresholdContains,type BinaryMaskRule} from './imageProcessing'
export type ThresholdSlice={width:number;height:number;values:Float32Array;valid:Uint8Array;selection?:Uint8Array;indices?:Int32Array}
/** Bounded, nearest-voxel preview in the same physical slice planes as landmarks/regions.
 * Cache this slice across slider events: do not scan/copy a full 3D volume per drag. */
export function sampleThresholdSlice(view:View,item:Loaded,limit=512,includeIndices=false):ThresholdSlice|null{
 const tile=view.nv.screenSlices?.find(s=>s.axCorSag==={axial:0,coronal:1,sagittal:2}[view.plane]);if(!tile)return null
 const [l,t,w,h]=tile.leftTopWidthHeight;if(![l,t,w,h].every(Number.isFinite)||!w||h<=0||!view.canvas.width||!view.canvas.height)return null
 const left=Math.min(l,l+w),width=Math.abs(w),x0=left+width*.25,y0=t+h*.25
 const p=windowPointFromCanvas(view,x0,y0),px=windowPointFromCanvas(view,left+width*.75,y0),py=windowPointFromCanvas(view,x0,t+h*.75);if(!p||!px||!py)return null
 const base=item.processing?.base??item.raw,inv=invertAffine(base.affine),q=applyAffine(inv,p),qx=applyAffine(inv,px),qy=applyAffine(inv,py),dx=qx.map((v,a)=>(v-q[a])/(width*.5)),dy=qy.map((v,a)=>(v-q[a])/(h*.5))
 const scale=Math.min(1,limit/view.canvas.width,limit/view.canvas.height),outWidth=Math.max(1,Math.round(view.canvas.width*scale)),outHeight=Math.max(1,Math.round(view.canvas.height*scale)),values=new Float32Array(outWidth*outHeight),valid=new Uint8Array(values.length),selection=item.processing?.binarySelection?new Uint8Array(values.length):undefined,indices=includeIndices?new Int32Array(values.length).fill(-1):undefined,edits=new Map(item.processing?.recipe.edits.map(e=>[e.index,e.value])??[]),crop=item.processing?.recipe.crop
 for(let y=0;y<outHeight;y++)for(let x=0;x<outWidth;x++){
  const cx=(x+.5)*view.canvas.width/outWidth,cy=(y+.5)*view.canvas.height/outHeight;if(cx<left||cx>left+width||cy<t||cy>t+h)continue
  const xx=cx-x0,yy=cy-y0
  const vx=Math.round(q[0]+dx[0]*xx+dy[0]*yy),vy=Math.round(q[1]+dx[1]*xx+dy[1]*yy),vz=Math.round(q[2]+dx[2]*xx+dy[2]*yy),d=base.dims;if(vx<0||vy<0||vz<0||vx>=d[0]||vy>=d[1]||vz>=d[2])continue
  if(crop){const world=applyAffine(base.affine,[vx,vy,vz]);if(world.some((v,a)=>v<crop.min[a]-1e-6||v>crop.max[a]+1e-6))continue}
  const i=vx+d[0]*(vy+d[1]*vz),j=x+outWidth*y;if(indices)indices[j]=i;values[j]=edits.get(i)??base.values[i];valid[j]=Number.isFinite(values[j])?1:0;if(selection)selection[j]=item.processing!.binarySelection![i]
 }
 return {width:outWidth,height:outHeight,values,valid,selection,indices}
}
export function thresholdPixels(slice:ThresholdSlice,range:BinaryMaskRule,pixels:Uint8ClampedArray=new Uint8ClampedArray(slice.values.length*4),useSelection=false){const overrides=new Map(range.edits?.map(e=>[e.index,e.value])??[]);pixels.fill(0);for(let i=0;i<slice.values.length;i++)if(slice.valid[i]&&(useSelection&&slice.selection?slice.selection[i]:(slice.indices&&overrides.has(slice.indices[i])?Boolean(overrides.get(slice.indices[i])):thresholdContains(slice.values[i],range)))){pixels[i*4]=32;pixels[i*4+1]=240;pixels[i*4+2]=128;pixels[i*4+3]=100}return pixels}
