import {NVImage} from '@niivue/niivue'
import * as nifti from 'nifti-reader-js'
import type {RawNifti} from './roiWarp'
export function imageFromRaw(raw:RawNifti,name:string):NVImage{
 const h=new nifti.NIFTI1();h.dims=[3,...raw.dims,1,1,1,1];h.pixDims=[1,...raw.pixDims,1,1,1,1];h.datatypeCode=16;h.numBitsPerVoxel=32;h.scl_slope=1;h.scl_inter=0;h.xyzt_units=2;h.sform_code=2;h.qform_code=0;h.affine=raw.affine.map(row=>[...row]);h.vox_offset=352;h.magic='n+1\0';h.littleEndian=true
 const image=new NVImage();image.hdr=h;image.img=raw.values;image.name=name;image.calculateRAS();image.calMinMax();return image
}
/** Independent viewer metadata/IDs, shared immutable voxel storage. Never edit img in place. */
export function sharedViewerImage(image:NVImage):NVImage{
 const copy=new NVImage(),id=copy.id;Object.assign(copy,image);copy.id=id;copy.hdr=Object.assign(Object.create(Object.getPrototypeOf(image.hdr!)),image.hdr)
 for(const key of ['dims','pixDims','affine']){const value=(image.hdr as any)[key];(copy.hdr as any)[key]=structuredClone(value)}
 copy.img=image.img;copy.calculateRAS();return copy
}

export function nativeGeometry(item:import('./appTypes').Loaded):RawNifti{const info=item.native?.info;return info?{...item.raw,dims:info.dims,pixDims:info.spacing,affine:info.affine,values:new Float32Array(0)}:(item.processing?.base??item.raw)}
