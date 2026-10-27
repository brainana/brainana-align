import {sampleExport,type ExportInterpolation} from './exportSampling'
import * as nifti from 'nifti-reader-js'
import {Gzip} from 'fflate'
import {createPointMapping,validateSpline,type NonlinearSpline} from './nonlinearSpline'
import {applyAffine,invertAffine,type RawNifti} from './roiWarp'
import type {Mat4} from './rigid'
/** Slab resampling + incremental gzip: no full destination float array or NIfTI byte copy. */
export async function compressedAlignedImage(source:RawNifti,target:Pick<RawNifti,'dims'|'pixDims'|'affine'>,matrix:Mat4,spline?:NonlinearSpline,progress:(text:string)=>void=()=>{},interpolation:ExportInterpolation='linear'):Promise<Blob>{
 return compressImage(source,target,matrix,spline,progress,false,interpolation)
}
/** Serialize a working grid exactly, without interpolating binary or image values. */
export async function compressedWorkingImage(source:RawNifti,progress:(text:string)=>void=()=>{}):Promise<Blob>{return compressImage(source,source,[[1,0,0,0],[0,1,0,0],[0,0,1,0],[0,0,0,1]],undefined,progress,true)}
async function compressImage(source:RawNifti,target:Pick<RawNifti,'dims'|'pixDims'|'affine'>,matrix:Mat4,spline:NonlinearSpline|undefined,progress:(text:string)=>void,direct:boolean,interpolation:ExportInterpolation='linear'):Promise<Blob>{
 if(spline)validateSpline(spline)
 const h=target.dims.some(n=>n>32767)?new nifti.NIFTI2():new nifti.NIFTI1();h.dims=[3,...target.dims,1,1,1,1];h.pixDims=[1,...target.pixDims,1,1,1,1];h.datatypeCode=16;h.numBitsPerVoxel=32;h.scl_slope=1;h.scl_inter=0;h.qform_code=0;h.sform_code=2;h.affine=target.affine.map(row=>[...row]);h.xyzt_units=2;h.littleEndian=true;h.vox_offset=h instanceof nifti.NIFTI2?544:352;h.magic=h instanceof nifti.NIFTI2?'n+2\0\r\n\x1a\n':'n+1\0'
 const parts:BlobPart[]=[],zip=new Gzip({level:3},chunk=>parts.push(chunk.slice().buffer as ArrayBuffer));zip.push(new Uint8Array(h.toArrayBuffer(false)))
 const map=createPointMapping(matrix,spline).inverse,invSource=invertAffine(source.affine),plane=target.dims[0]*target.dims[1],chunkVoxels=Math.max(1,Math.min(2**20,plane)),buffer=new Uint8Array(chunkVoxels*4),view=new DataView(buffer.buffer);let used=0
 for(let z=0;z<target.dims[2];z++){for(let y=0;y<target.dims[1];y++)for(let x=0;x<target.dims[0];x++){
  const value=direct?source.values[x+target.dims[0]*(y+target.dims[1]*z)]:sampleExport(source.values,source.dims,applyAffine(invSource,map(applyAffine(target.affine,[x,y,z]))),interpolation);view.setFloat32(used*4,value,true);used++;if(used===chunkVoxels){zip.push(buffer);used=0}
 }if(z%4===0){progress(`Exporting output grid: slice ${z+1}/${target.dims[2]}…`);await new Promise(r=>setTimeout(r,0))}}
 if(used)zip.push(buffer.subarray(0,used*4));zip.push(new Uint8Array(0),true);return new Blob(parts,{type:'application/gzip'})
}
