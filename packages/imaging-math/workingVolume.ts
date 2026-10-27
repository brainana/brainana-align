import * as nifti from 'nifti-reader-js'
import {validateAffine} from './landmarkFit'
import type {RawNifti,Dims3} from './roiWarp'
export type VolumeInfo={name:string;fileBytes:number;residentFileBytes?:number;dims:Dims3;spacing:Dims3;affine:number[][];frames:number;bytesPerVoxel:number;datatype:number;littleEndian:boolean;offset:number;slope:number;intercept:number;header:Uint8Array}
export const voxels=(d:number[])=>d.reduce((a,b)=>a*b,1)
export async function volumeReader(file:Blob){const prefix=new Uint8Array(await file.slice(0,2).arrayBuffer());const compressed=prefix[0]===31&&prefix[1]===139;return (compressed?file.stream().pipeThrough(new DecompressionStream('gzip')):file.stream()).getReader()}
export async function inspectVolume(file:Blob&{name?:string}):Promise<VolumeInfo>{
 const reader=await volumeReader(file),parts:Uint8Array[]=[];let count=0
 try{while(count<544){const {value,done}=await reader.read();if(done)break;const take=value.subarray(0,544-count);parts.push(take);count+=take.length}}finally{await reader.cancel().catch(()=>{})}
 const bytes=new Uint8Array(count);let at=0;for(const part of parts){bytes.set(part,at);at+=part.length}
 // Ignore extensions for metadata inspection; their payload is not part of the image geometry.
 const clean=bytes.slice();const view=new DataView(clean.buffer);const n1=view.getInt32(0,true)===348||view.getInt32(0,false)===348;const extension=n1?348:540;if(clean.length>extension)clean[extension]=0
 const h=nifti.readHeader(clean.buffer,true);const dims=h.dims.slice(1,4).map(Number) as Dims3
 if(!dims.every(v=>Number.isSafeInteger(v)&&v>0&&v<=65535)||!Number.isSafeInteger(voxels(dims)))throw Error('Invalid or oversized image dimensions')
 const affine=h.affine.map(row=>row.map(Number)),units=h.xyzt_units&7;if(![0,1,2,3].includes(units))throw Error('Unsupported spatial units');const factor=units===1?1000:units===3?.001:1;for(let r=0;r<3;r++)for(let c=0;c<4;c++)affine[r][c]*=factor;validateAffine(affine)
 const spacing=[0,1,2].map(a=>Math.hypot(affine[0][a],affine[1][a],affine[2][a])) as Dims3
 const widths:Record<number,number>={2:1,4:2,8:4,16:4,64:8,256:1,512:2,768:4};const bytesPerVoxel=widths[h.datatypeCode];if(!bytesPerVoxel)throw Error(`Unsupported scalar NIfTI datatype ${h.datatypeCode}`)
 const frames=h.dims.slice(4,Math.max(4,h.dims[0]+1)).reduce((a,b)=>a*Math.max(1,b),1);if(!Number.isSafeInteger(frames)||frames<1)throw Error('Invalid frame count')
 if(!Number.isSafeInteger(h.vox_offset)||h.vox_offset<(n1?348:540))throw Error('Invalid NIfTI voxel offset')
 const payloadBytes=voxels(dims)*frames*bytesPerVoxel;if(!Number.isSafeInteger(payloadBytes)||!Number.isSafeInteger(payloadBytes+h.vox_offset))throw Error('Image byte count is not safely representable');if(!(new Uint8Array(await file.slice(0,2).arrayBuffer())[0]===31)&&file.size<h.vox_offset+payloadBytes)throw Error('Image file is truncated');
 return {name:file.name??'volume.nii',fileBytes:file.size,dims,spacing,affine,frames,bytesPerVoxel,datatype:h.datatypeCode,littleEndian:h.littleEndian,offset:h.vox_offset,slope:h.scl_slope||1,intercept:h.scl_slope?(h.scl_inter||0):0,header:clean}
}
export function workingGrid(info:Pick<VolumeInfo,'dims'|'spacing'|'affine'>,requested:Dims3){
 if(!requested.every(v=>Number.isFinite(v)&&v>0))throw Error('Working spacing must be positive millimeters')
 const dims=info.dims.map((n,a)=>n===1?1:Math.max(2,Math.floor((n-1)/Math.max(1,requested[a]/info.spacing[a]))+1)) as Dims3
 const scale=dims.map((n,a)=>n===1?1:(info.dims[a]-1)/(n-1)) as Dims3,affine=info.affine.map(row=>[...row]);for(let r=0;r<3;r++)for(let a=0;a<3;a++)affine[r][a]*=scale[a]
 return {dims,scale,affine,spacing:info.spacing.map((v,a)=>v*scale[a]) as Dims3}
}
/** Streaming separable triangular low-pass + decimation: never allocate the native volume.
 * Centers, world origin, axis directions and endpoint extents are preserved. Coarse native
 * axes are never upsampled. Precomputed normalized taps also handle clipped boundary kernels. */
export async function prepareVolume(file:Blob,info:VolumeInfo,requested:Dims3,frame=0,progress:(n:number)=>void=()=>{}):Promise<RawNifti>{
 if(!Number.isInteger(frame)||frame<0||frame>=info.frames)throw Error('Selected frame is unavailable')
 const grid=workingGrid(info,requested),size=voxels(grid.dims);if(size*4>1536*1024**2)throw Error('Working array exceeds the 1.5 GiB allocation limit. Choose coarser spacing.')
 const output=new Float32Array(size),taps=info.dims.map((count,a)=>{
  const s=grid.scale[a],n=grid.dims[a],sums=new Float64Array(n),lists=Array.from({length:count},(_,i)=>{const list:Array<[number,number]>=[];if(s===1){list.push([i,1]);sums[i]=1;return list}for(let j=Math.max(0,Math.ceil(i/s-1));j<=Math.min(n-1,Math.floor(i/s+1));j++){const w=Math.max(0,1-Math.abs(i-j*s)/s);if(w>0){list.push([j,w]);sums[j]+=w}}return list});for(const list of lists)for(const tap of list)tap[1]/=sums[tap[0]];return lists
 })
 const reader=await volumeReader(file),start=info.offset+frame*voxels(info.dims)*info.bytesPerVoxel,total=voxels(info.dims),end=start+total*info.bytesPerVoxel;let position=0,index=0,tail=new Uint8Array(0),lastPlane=-1
 const read=(v:DataView,o:number)=>{switch(info.datatype){case 2:return v.getUint8(o);case 256:return v.getInt8(o);case 4:return v.getInt16(o,info.littleEndian);case 512:return v.getUint16(o,info.littleEndian);case 8:return v.getInt32(o,info.littleEndian);case 768:return v.getUint32(o,info.littleEndian);case 16:return v.getFloat32(o,info.littleEndian);default:return v.getFloat64(o,info.littleEndian)}}
 try{while(position<end){const {value,done}=await reader.read();if(done)break;const begin=position;position+=value.length;if(position<=start)continue;const selected=value.subarray(Math.max(0,start-begin),Math.min(value.length,end-begin)),data=new Uint8Array(tail.length+selected.length);data.set(tail);data.set(selected,tail.length);const view=new DataView(data.buffer);let o=0
  for(;o+info.bytesPerVoxel<=data.length;o+=info.bytesPerVoxel,index++){
   const x=index%info.dims[0],y=Math.floor(index/info.dims[0])%info.dims[1],z=Math.floor(index/(info.dims[0]*info.dims[1])),v=read(view,o)*info.slope+info.intercept
   for(const [oz,wz] of taps[2][z])for(const [oy,wy] of taps[1][y])for(const [ox,wx] of taps[0][x])output[ox+grid.dims[0]*(oy+grid.dims[1]*oz)]+=v*wx*wy*wz
   if(z!==lastPlane){lastPlane=z;progress(z/info.dims[2])}
  }tail=data.slice(o)
 }}finally{await reader.cancel().catch(()=>{})}
 if(index!==total)throw Error(`Truncated voxel data: expected ${total}, read ${index}`);progress(1)
 return {dims:grid.dims,pixDims:grid.spacing,affine:grid.affine,values:output,frameCount:1,datatypeCode:16,littleEndian:true,slope:1,intercept:0}
}
