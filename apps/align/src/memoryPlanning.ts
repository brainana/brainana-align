import {workingGrid,voxels,type VolumeInfo} from '../../../packages/imaging-math/workingVolume'
import type {Dims3} from './roiWarp'
export type MemoryReading={total:number;available:number;sampledAt:number;source:string}
export const GiB=1024**3,MAX_WORKING_BYTES=1.5*GiB
export function memoryPlan(infos:VolumeInfo[],spacings:Dims3[],reading:MemoryReading|null,budgetGiB=0){
 const grids=infos.map((info,i)=>workingGrid(info,spacings[i])),counts=grids.map(g=>voxels(g.dims)),sum=counts.reduce((a,b)=>a+b,0),largest=Math.max(0,...counts)
 // Six source panes and three two-volume review panes each have separate GPU resources.
 // Includes immutable CPU images, one worker copy, one active pyramid, review and headroom.
 const low=.5*GiB+20*sum+20*largest,high=.75*GiB+32*sum+32*largest+infos.reduce((n,i)=>n+(i.residentFileBytes??Math.min(i.fileBytes,64*1024**2)),0)
 // Free/available readings exclude buffers already resident in this app and may
 // exclude reclaimable caches. Never infer OS pressure or allocation failure from
 // comparing the whole planning envelope to that instantaneous reading.
 const reserve=reading?Math.max(.5*GiB,.15*reading.total):0,budget=budgetGiB>0?budgetGiB*GiB:null,capacity=reading?Math.max(0,reading.total-reserve):null,usable=capacity===null?budget:budget===null?capacity:Math.min(capacity,budget)
 const unsupported=grids.some(g=>voxels(g.dims)*4>MAX_WORKING_BYTES||g.dims.some(n=>n>2048))
 const rating=unsupported?'Exceeds supported working size':usable===null?'Capacity unavailable':low>usable?'Above planning allowance':high>usable||largest>64e6?'Potentially slow':'Comfortable'
 const native=infos.map(i=>voxels(i.dims)),exportHigh=high+(4*Math.max(0,...native)+12*Math.max(0,...native))
 return {grids,low,high,usable,reserve,rating,unsupported,exportHigh,workingVoxels:sum,nativeExportLimited:native.some(n=>n*4>MAX_WORKING_BYTES)}
}
export function recommendSpacings(infos:VolumeInfo[],reading:MemoryReading|null,budget=0,multiplier=1):Dims3[]{
 let spacing=Math.max(...infos.map(i=>Math.min(...i.spacing)),...infos.map(i=>Math.max(...i.dims.map((n,a)=>(n-1)*i.spacing[a]))/320))*multiplier
 let result=infos.map(i=>i.spacing.map(v=>Math.max(v,spacing)) as Dims3)
 for(let n=0;n<20;n++){const p=memoryPlan(infos,result,reading,budget);if(!p.unsupported&&(p.usable===null||p.high<=p.usable*.65))break;const maximum=Math.max(...infos.map(i=>Math.max(...i.dims.map((n,a)=>(n-1)*i.spacing[a]))/96));if(spacing>=maximum)break;spacing=Math.min(maximum,spacing*1.25);result=infos.map(i=>i.spacing.map(v=>Math.max(v,spacing)) as Dims3)}return result
}
export async function readMemory():Promise<MemoryReading|null>{try{const api=(window as any).brainanaPreferences;if(api?.memory){const r=await api.memory();if(Number.isFinite(r.total)&&r.total>0&&Number.isFinite(r.available)&&r.available>=0)return r}return null}catch{return null}}
export const memoryText=(n:number)=>`${(n/GiB).toFixed(2)} GiB`
