import {sampleLinear,type Dims3} from './roiWarp'
export type ExportInterpolation='nearest'|'linear'|'cubic'
/** Identical interpolation arithmetic for a paged native scalar reader. */
export function sampleExportReader(read:(index:number)=>number,dims:Dims3,p:number[],method:ExportInterpolation):number{
 if(p.some((v,a)=>!Number.isFinite(v)||v< -1e-6||v>dims[a]-1+1e-6))return 0
 const q=p.map((v,a)=>Math.max(0,Math.min(dims[a]-1,v)))
 if(method==='nearest'){const [x,y,z]=q.map(Math.round);return read(x+dims[0]*(y+dims[1]*z))}
 if(method==='linear'){
  const [x,y,z]=q,x0=Math.floor(x),y0=Math.floor(y),z0=Math.floor(z),x1=Math.min(x0+1,dims[0]-1),y1=Math.min(y0+1,dims[1]-1),z1=Math.min(z0+1,dims[2]-1),tx=x-x0,ty=y-y0,tz=z-z0
  const at=(a:number,b:number,c:number)=>read(a+dims[0]*(b+dims[1]*c)),mix=(a:number,b:number,t:number)=>t===0?a:t===1?b:a*(1-t)+b*t
  const c00=mix(at(x0,y0,z0),at(x1,y0,z0),tx),c10=mix(at(x0,y1,z0),at(x1,y1,z0),tx),c01=mix(at(x0,y0,z1),at(x1,y0,z1),tx),c11=mix(at(x0,y1,z1),at(x1,y1,z1),tx)
  return mix(mix(c00,c10,ty),mix(c01,c11,ty),tz)
 }
 const base=q.map(Math.floor),weights=q.map((v,a)=>{const t=v-base[a];return [-.5*t+t*t-.5*t*t*t,1-2.5*t*t+1.5*t*t*t,.5*t+2*t*t-1.5*t*t*t,-.5*t*t+.5*t*t*t]});let result=0
 for(let z=0;z<4;z++)for(let y=0;y<4;y++)for(let x=0;x<4;x++){const v=[x,y,z].map((n,a)=>Math.max(0,Math.min(dims[a]-1,base[a]+n-1))),weight=weights[0][x]*weights[1][y]*weights[2][z];if(weight!==0)result+=read(v[0]+dims[0]*(v[1]+dims[1]*v[2]))*weight}
 return result
}

/** Enumerate interpolation support before reading, without touching voxel data. */
export function visitExportNeighbors(dims:Dims3,p:number[],method:ExportInterpolation,visit:(index:number)=>void):void{
 if(p.some((v,a)=>!Number.isFinite(v)||v< -1e-6||v>dims[a]-1+1e-6))return
 const q=p.map((v,a)=>Math.max(0,Math.min(dims[a]-1,v))),base=q.map(Math.floor)
 if(method==='nearest'){const [x,y,z]=q.map(Math.round);visit(x+dims[0]*(y+dims[1]*z));return}
 const size=method==='linear'?2:4,shift=method==='linear'?0:-1
 for(let z=0;z<size;z++)for(let y=0;y<size;y++)for(let x=0;x<size;x++){const v=[x,y,z].map((n,a)=>Math.max(0,Math.min(dims[a]-1,base[a]+n+shift)));visit(v[0]+dims[0]*(v[1]+dims[1]*v[2]))}
}
export function sampleExport(values:Float32Array,dims:Dims3,p:number[],method:ExportInterpolation){
 if(p.some((v,a)=>!Number.isFinite(v)||v< -1e-6||v>dims[a]-1+1e-6))return 0
 const q=p.map((v,a)=>Math.max(0,Math.min(dims[a]-1,v)))
 if(method==='linear')return sampleLinear(values,dims,q as Dims3)
 if(method==='nearest'){const [x,y,z]=q.map(Math.round);return values[x+dims[0]*(y+dims[1]*z)]}
 const base=q.map(Math.floor),weights=q.map((v,a)=>{const t=v-base[a];return [-.5*t+t*t-.5*t*t*t,1-2.5*t*t+1.5*t*t*t,.5*t+2*t*t-1.5*t*t*t,-.5*t*t+.5*t*t*t]});let result=0
 for(let z=0;z<4;z++)for(let y=0;y<4;y++)for(let x=0;x<4;x++){const v=[x,y,z].map((n,a)=>Math.max(0,Math.min(dims[a]-1,base[a]+n-1)));const weight=weights[0][x]*weights[1][y]*weights[2][z];if(weight!==0)result+=values[v[0]+dims[0]*(v[1]+dims[1]*v[2])]*weight}
 return result
}
