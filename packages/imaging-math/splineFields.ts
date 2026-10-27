import {NIFTI1} from 'nifti-reader-js'
import {createPointMapping,validateSpline,type NonlinearSpline} from './nonlinearSpline'
import {applyAffine,type RawNifti} from './roiWarp'
import type {Mat4,Vec3} from './rigid'

// The NIfTI grid affine locates samples in RAS mm. Components are physical RAS
// vectors, not voxel-axis vectors, even on oblique or axis-swapped grids.
export const DISPLACEMENT_FIELD_PROFILE={
 intentCode:1006,intentName:'NIFTI_INTENT_DISPVECT',dimensions:'[I,J,K,1,3]',
 componentDimension:5,componentOrder:['R','A','S'],componentUnits:'mm',
 vectorCoordinateSystem:'RAS',gridCoordinateSystem:'NIfTI sform RAS mm',
 storage:'float32 little-endian; planar components; I fastest',
 forward:{gridSpace:'moving',mapping:'moving_to_fixed',displacement:'F(x) - x'},
 inverse:{gridSpace:'fixed',mapping:'fixed_to_moving',displacement:'F^-1(y) - y'},
 lpsConversion:'On disk, grid geometry and vector components are RAS mm. Ensure the reader converts both to its physical coordinates. ITK NIfTI displacement-intent readers may convert RAS to LPS automatically; do not convert twice.'
} as const
export async function splineDisplacementField(reference:RawNifti,matrix:Mat4,spline:NonlinearSpline,direction:'forward'|'inverse',progress:(text:string)=>void=()=>{}){
 validateSpline(spline);const map=createPointMapping(matrix,spline)[direction],n=reference.dims.reduce((a,b)=>a*b,1),data=new Float32Array(3*n);let i=0
 for(let z=0;z<reference.dims[2];z++){if(z%4===0){progress(`${direction} RAS displacement field: slice ${z+1}/${reference.dims[2]}…`);await new Promise(r=>setTimeout(r,0))}for(let y=0;y<reference.dims[1];y++)for(let x=0;x<reference.dims[0];x++){const p=applyAffine(reference.affine,[x,y,z]),q=map(p as Vec3);for(let a=0;a<3;a++)data[a*n+i]=q[a]-p[a];i++}}
 const hdr=new NIFTI1();Object.assign(hdr,{littleEndian:true,dims:[5,...reference.dims,1,3,1,1],pixDims:[1,...reference.pixDims,1,1,1,1],intent_code:DISPLACEMENT_FIELD_PROFILE.intentCode,intent_name:'RAS displacement',datatypeCode:16,numBitsPerVoxel:32,vox_offset:352,scl_slope:1,scl_inter:0,xyzt_units:2,qform_code:0,sform_code:2,affine:reference.affine.map(r=>[...r]),magic:'n+1\0',description:`Brainana ${direction} ${DISPLACEMENT_FIELD_PROFILE[direction].displacement}; RAS mm; dim5=R,A,S`})
 const header=new Uint8Array(hdr.toArrayBuffer()),bytes=new Uint8Array(352+data.byteLength);bytes.set(header.subarray(0,352));const output=new DataView(bytes.buffer);for(let j=0;j<data.length;j++)output.setFloat32(352+4*j,data[j],true);return bytes
}

// Fields are sampled representations, not the authoritative spline. A configurable
// physical spacing prevents multi-gigabyte vector allocations on fine CT grids.
export function displacementReferenceGrid(reference:RawNifti,spacingMm=2):RawNifti{
 if(!Number.isFinite(spacingMm)||spacingMm<.2||spacingMm>20)throw Error('Displacement field spacing must be 0.2–20 mm')
 const steps=reference.dims.map((_,a)=>{const length=Math.hypot(...reference.affine.slice(0,3).map(r=>r[a]));return Math.max(1,Math.ceil(spacingMm/length))})
 const dims=reference.dims.map((n,a)=>Math.max(1,Math.ceil((n-1)/steps[a])+1)) as [number,number,number],affine=reference.affine.map(r=>[...r]),scales=dims.map((n,a)=>n>1?(reference.dims[a]-1)/(n-1):1)
 for(let a=0;a<3;a++)for(let r=0;r<3;r++)affine[r][a]*=scales[a]
 const bytes=dims.reduce((a,b)=>a*b,1)*12
 if(bytes>256*1024*1024)throw Error(`Requested field needs ${(bytes/1024**2).toFixed(0)} MiB. Choose a coarser field spacing; 2 mm is the default. Full spline coefficients remain available in JSON.`)
 return {...reference,dims,affine,pixDims:[0,1,2].map(a=>Math.hypot(...affine.slice(0,3).map(r=>r[a]))) as [number,number,number],values:new Float32Array(0),frameCount:1}
}
