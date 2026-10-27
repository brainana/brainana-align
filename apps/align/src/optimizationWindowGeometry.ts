import type {View,WindowBounds} from './appTypes'
import type {Vec3} from './rigid'
import {planeAxes} from './optimizationWindows'

// WINDOW CONTRACT: these are RAS millimeter display planes (setSliceMM(true)),
// not voxel axes. Never overwrite a fractional voxel axis on an oblique image.
// Niivue's frac2canvasPos rejects off-screen corners and canvasPos2frac returns
// the finite sentinel [-1,-1,-1]. Neither is suitable for an unbounded ROI drag.
const sliceTypes={axial:0,coronal:1,sagittal:2}
function tile(view:View){
  return view.nv.screenSlices?.find(s=>s.axCorSag===sliceTypes[view.plane]&&s.leftTopWidthHeight?.length>=4&&s.leftTopMM?.length>=2&&s.fovMM?.length>=2&&[...s.leftTopWidthHeight,...s.leftTopMM,...s.fovMM].every(Number.isFinite)&&s.leftTopWidthHeight[2]!==0&&s.leftTopWidthHeight[3]>0&&s.fovMM[0]!==0&&s.fovMM[1]!==0)
}
export function windowPointFromCanvas(view:View,x:number,y:number,clampToSlice=false):Vec3|null{
  const slice=tile(view)
  if(!slice||!Number.isFinite(x)||!Number.isFinite(y))return null
  let [left,top,width,height]=slice.leftTopWidthHeight
  const mirrored=width<0;if(mirrored){left+=width;width=-width}
  let fx=(x-left)/width,fy=1-(y-top)/height
  if(!clampToSlice&&(fx<0||fx>1||fy<0||fy>1))return null
  fx=Math.max(0,Math.min(1,fx));fy=Math.max(0,Math.min(1,fy));if(mirrored)fx=1-fx
  const center=Array.from(view.nv.frac2mm(view.nv.scene.crosshairPos),Number)
  if(!center.slice(0,3).every(Number.isFinite)||center.length<3)return null
  const [a,b]=planeAxes[view.plane],point=center.slice(0,3) as Vec3
  point[a]=slice.leftTopMM[0]+fx*slice.fovMM[0];point[b]=slice.leftTopMM[1]+fy*slice.fovMM[1]
  return point
}
export function windowRectangleOnCanvas(view:View,bounds:WindowBounds):{left:number;top:number;width:number;height:number}|null{
  const slice=tile(view);if(!slice)return null
  let [left,top,width,height]=slice.leftTopWidthHeight
  const mirrored=width<0;if(mirrored){left+=width;width=-width}
  const [a,b]=planeAxes[view.plane]
  const project=(point:Vec3)=>{let fx=(point[a]-slice.leftTopMM[0])/slice.fovMM[0];if(mirrored)fx=1-fx;const fy=(point[b]-slice.leftTopMM[1])/slice.fovMM[1];return [left+fx*width,top+(1-fy)*height]}
  const first=project(bounds.min),second=project(bounds.max)
  // Do not clamp each corner: the layer clips the rectangle without changing
  // its physical bounds, even when both corners are outside the visible tile.
  return {left:Math.min(first[0],second[0]),top:Math.min(first[1],second[1]),width:Math.abs(second[0]-first[0]),height:Math.abs(second[1]-first[1])}
}
