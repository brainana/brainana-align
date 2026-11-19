import {Gzip} from 'fflate'
import {alignedImageHeader} from './nativeImageExport'
import {sampleExportReader,visitExportNeighbors,type ExportInterpolation} from './exportSampling'
import {createPointMapping,validateSpline,type NonlinearSpline} from './nonlinearSpline'
import {applyAffine,invertAffine,type RawNifti} from './roiWarp'
import type {Mat4,Vec3} from './rigid'
import type {NativeVoxelSource} from './nativeVoxelSource'

/** Fill bounded output strips in spatial tiles, prefetching just their native
 * interpolation pages. Split tiles whose transformed neighbors exceed cache.
 * Compression/output Blob storage remains budgeted separately from page memory. */
export async function compressedPagedNativeImage(source:NativeVoxelSource,target:Pick<RawNifti,'dims'|'pixDims'|'affine'>,matrix:Mat4,spline?:NonlinearSpline,progress:(text:string)=>void=()=>{},interpolation:ExportInterpolation='linear',signal?:AbortSignal):Promise<Blob>{
 if(spline)validateSpline(spline)
 const chunks:BlobPart[]=[],zip=new Gzip({level:3},part=>chunks.push(part.slice().buffer));zip.push(alignedImageHeader(target))
 const pull=createPointMapping(matrix,spline).inverse,inv=invertAffine(source.affine),[nx,ny,nz]=target.dims
 const rows=Math.max(1,Math.min(ny,Math.floor(8*1024**2/nx))),values=new Float32Array(nx*rows),pagesPerTile=source.maxPagesPerBatch
 const output=new Uint8Array(1024**2),outputView=new DataView(output.buffer);let used=0
 const read=(index:number)=>source.read(index),check=()=>{if(signal?.aborted)throw Error('Native export cancelled. No output saved.')}
 const fill=async(points:Vec3[],slots:number[])=>{
  check();const pages=new Set<number>();if(Number.isFinite(pagesPerTile))for(const point of points)visitExportNeighbors(source.dims,point,interpolation,index=>pages.add(source.page(index)))
  if(pages.size>pagesPerTile){if(points.length<2)throw Error('Native interpolation support exceeds the page cache');const middle=Math.floor(points.length/2);await fill(points.slice(0,middle),slots.slice(0,middle));await fill(points.slice(middle),slots.slice(middle));return}
  await source.ensure(pages);for(let i=0;i<points.length;i++)values[slots[i]]=sampleExportReader(read,source.dims,points[i],interpolation)
 }
 let yielded=performance.now()
 for(let z=0;z<nz;z++)for(let y0=0;y0<ny;y0+=rows){const height=Math.min(rows,ny-y0);if(y0===0)progress(`Exporting output grid: slice ${z+1}/${nz}…`)
  for(let x0=0;x0<nx;x0+=32)for(let yTile=0;yTile<height;yTile+=16){const points:Vec3[]=[],slots:number[]=[];for(let y=yTile;y<Math.min(height,yTile+16);y++)for(let x=x0;x<Math.min(nx,x0+32);x++){points.push(applyAffine(inv,pull(applyAffine(target.affine,[x,y0+y,z]))));slots.push(x+nx*y)}await fill(points,slots)
   if(performance.now()-yielded>30){await new Promise(r=>setTimeout(r,0));yielded=performance.now();check()}
  }
  // DataView guarantees little-endian output even on a big-endian host.
  for(let i=0;i<nx*height;i++){outputView.setFloat32(used,values[i],true);used+=4;if(used===output.length){zip.push(output);used=0}}
 }
 check();if(used)zip.push(output.subarray(0,used));zip.push(new Uint8Array(0),true);return new Blob(chunks,{type:'application/gzip'})
}
