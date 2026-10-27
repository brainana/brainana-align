import {sampleLinear,type Dims3} from './roiWarp'
export type ExportInterpolation='nearest'|'linear'|'cubic'
export function sampleExport(values:Float32Array,dims:Dims3,p:number[],method:ExportInterpolation){
 if(p.some((v,a)=>!Number.isFinite(v)||v< -1e-6||v>dims[a]-1+1e-6))return 0
 const q=p.map((v,a)=>Math.max(0,Math.min(dims[a]-1,v)))
 if(method==='linear')return sampleLinear(values,dims,q as Dims3)
 if(method==='nearest'){const [x,y,z]=q.map(Math.round);return values[x+dims[0]*(y+dims[1]*z)]}
 const base=q.map(Math.floor),weights=q.map((v,a)=>{const t=v-base[a];return [-.5*t+t*t-.5*t*t*t,1-2.5*t*t+1.5*t*t*t,.5*t+2*t*t-1.5*t*t*t,-.5*t*t+.5*t*t*t]});let result=0
 for(let z=0;z<4;z++)for(let y=0;y<4;y++)for(let x=0;x<4;x++){const v=[x,y,z].map((n,a)=>Math.max(0,Math.min(dims[a]-1,base[a]+n-1)));const weight=weights[0][x]*weights[1][y]*weights[2][z];if(weight!==0)result+=values[v[0]+dims[0]*(v[1]+dims[1]*v[2])]*weight}
 return result
}
