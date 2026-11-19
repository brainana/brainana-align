import {volumeReader,voxels,type VolumeInfo} from './workingVolume'
import type {Dims3} from './roiWarp'

export const NATIVE_PAGE_VOXELS=65536
export const NATIVE_CACHE_BYTES=128*1024**2
const CHUNK_BYTES=8*1024**2
export type NativeReadStats={storage:'input-file'|'decompressed-blob'|'encoded-memory-chunks';sourceBytes:number;residentEncodedBytes:number;pageVoxels:number;cacheBudgetBytes:number;peakCacheBytes:number;largestReadBytes:number;pagesRead:number;backingBatchBytes:number}
export type NativeValueProcessor=(value:number,index:number)=>number
export type NativeVoxelSource={dims:Dims3;affine:number[][];spacing:Dims3;length:number;stats:NativeReadStats;maxPagesPerBatch:number;page(index:number):number;ensure(pages:Set<number>):Promise<void>;read(index:number):number;close():void}
export type NativeSourceOptions={progress?:(text:string)=>void;process?:NativeValueProcessor;pageVoxels?:number;cacheBytes?:number;signal?:AbortSignal;maxResidentEncodedBytes?:number;residentUncompressed?:boolean}

/** Native samples never require one full-size Float32Array. A caller-budgeted
 * fast path retains ORIGINAL encoded bytes in8MiB chunks; its total is explicit
 * in planning. Larger gzip inputs use browser-managed Blob backing and paged
 * reads. Uncompressed files can be range-read directly. All offsets use Number,
 * never signed32-bit arithmetic. Blob/output storage is not constant-memory. */
export async function openNativeVoxelSource(file:Blob,info:VolumeInfo,frame=0,o:NativeSourceOptions={}):Promise<NativeVoxelSource>{
 const length=voxels(info.dims),width=info.bytesPerVoxel,total=length*width,start=info.offset+frame*total
 if(!info.dims.every(n=>Number.isSafeInteger(n)&&n>0)||!Number.isSafeInteger(length)||length<1||!Number.isSafeInteger(total)||!Number.isSafeInteger(start+total)||!Number.isSafeInteger(info.offset)||info.offset<0||!Number.isInteger(frame)||frame<0||frame>=info.frames)throw Error('Invalid native image dimensions, frame or byte count')
 const widths:Record<number,number>={2:1,256:1,4:2,512:2,8:4,768:4,16:4,64:8}
 if(widths[info.datatype]!==width)throw Error('Unsupported or inconsistent native scalar datatype')
 const pageVoxels=o.pageVoxels??NATIVE_PAGE_VOXELS,cacheBytes=o.cacheBytes??NATIVE_CACHE_BYTES
 if(!Number.isSafeInteger(pageVoxels)||pageVoxels<1||(!Number.isSafeInteger(cacheBytes)||cacheBytes<0))throw Error('Invalid native source cache configuration')
 const stats:NativeReadStats={storage:'input-file',sourceBytes:total,residentEncodedBytes:0,pageVoxels,cacheBudgetBytes:cacheBytes,peakCacheBytes:0,largestReadBytes:0,pagesRead:0,backingBatchBytes:0}
 const check=()=>{if(o.signal?.aborted)throw Error('Native export cancelled. No output saved.')}
 check()
 const prefix=new Uint8Array(await file.slice(0,2).arrayBuffer()),compressed=prefix[0]===31&&prefix[1]===139,budget=o.maxResidentEncodedBytes??512*1024**2
 if(!Number.isSafeInteger(budget)||budget<0)throw Error('Invalid native encoded-memory allowance')
 const keepEncoded=total<=budget&&(compressed||Boolean(o.residentUncompressed))
 let backing:Blob|null=file,offset=start,closed=false;const encoded:DataView[]=[]
 if(compressed){
  stats.storage=keepEncoded?'encoded-memory-chunks':'decompressed-blob';offset=0
  const reader=await volumeReader(file);let position=0,received=0,used=0,chunk=new Uint8Array(Math.min(CHUNK_BYTES,total)),result=new Blob(),last=-1
  try{while(received<total){check();const {value,done}=await reader.read();if(done)break;const begin=position;position+=value.length;if(position<=start)continue
    const part=value.subarray(Math.max(0,start-begin),Math.min(value.length,start+total-begin))
    for(let at=0;at<part.length;){const take=Math.min(chunk.length-used,part.length-at);chunk.set(part.subarray(at,at+take),used);used+=take;received+=take;at+=take;stats.backingBatchBytes=Math.max(stats.backingBatchBytes,used)
     if(used===chunk.length){if(keepEncoded)encoded.push(new DataView(chunk.buffer));else result=new Blob([result,chunk.buffer]);used=0;chunk=new Uint8Array(Math.min(CHUNK_BYTES,total-received))}
    }
    const percent=Math.floor(received/total*100);if(percent!==last){last=percent;o.progress?.(`Reading compressed native moving image ${percent}%…`)}
   }
   check();if(received!==total)throw Error(`Truncated native voxel data in ${info.name}: expected ${total} bytes, read ${received}`)
   backing=keepEncoded?null:result;stats.residentEncodedBytes=keepEncoded?total:0
  }finally{await reader.cancel().catch(()=>{})}
 }else{
  if(file.size<start+total)throw Error(`Truncated native voxel data in ${info.name}: expected at least ${start+total} bytes, found ${file.size}`)
  if(keepEncoded){stats.storage='encoded-memory-chunks';for(let at=0;at<total;at+=CHUNK_BYTES){check();const bytes=await file.slice(start+at,Math.min(start+total,start+at+CHUNK_BYTES)).arrayBuffer();if(bytes.byteLength!==Math.min(CHUNK_BYTES,total-at))throw Error('Truncated native input chunk');encoded.push(new DataView(bytes));stats.backingBatchBytes=Math.max(stats.backingBatchBytes,bytes.byteLength);o.progress?.(`Reading native moving image ${Math.floor(Math.min(total,at+CHUNK_BYTES)/total*100)}%…`)}backing=null;stats.residentEncodedBytes=total}
 }
 const resident=stats.storage==='encoded-memory-chunks',pageFootprint=pageVoxels*((resident?0:width)+(o.process?5:0)),capacity=pageFootprint?Math.floor(cacheBytes/pageFootprint):Infinity
 if(capacity<64)throw Error('Native source cache must hold at least 64 interpolation pages')
 const cache=new Map<number,{raw?:DataView;values?:Float32Array;processed?:Uint8Array;bytes:number}>();let cachedBytes=0
 const decode=(view:DataView,at:number)=>{let v:number;switch(info.datatype){case 2:v=view.getUint8(at);break;case 256:v=view.getInt8(at);break;case 4:v=view.getInt16(at,info.littleEndian);break;case 512:v=view.getUint16(at,info.littleEndian);break;case 8:v=view.getInt32(at,info.littleEndian);break;case 768:v=view.getUint32(at,info.littleEndian);break;case 16:v=view.getFloat32(at,info.littleEndian);break;default:v=view.getFloat64(at,info.littleEndian)}return Math.fround(0+v*info.slope+info.intercept)}
 const encodedRead=(index:number)=>{const byte=index*width,chunk=Math.floor(byte/CHUNK_BYTES);return decode(encoded[chunk],byte-chunk*CHUNK_BYTES)}
 const ensure=async(pages:Set<number>)=>{
  check();if(closed)throw Error('Native source is closed');if(pages.size>capacity)throw Error('Native source tile exceeds the page cache; subdivide the output tile');if(resident&&!o.process)return
  const missing:number[]=[]
  for(const p of pages){if(!Number.isSafeInteger(p)||p<0||p*pageVoxels>=length)throw Error('Native page index is outside the image');const hit=cache.get(p);if(hit){cache.delete(p);cache.set(p,hit)}else missing.push(p)}
  for(const [p,entry] of cache){if(cache.size+missing.length<=capacity)break;if(pages.has(p))continue;cachedBytes-=entry.bytes;cache.delete(p)}
  const add=(p:number,raw?:DataView)=>{const count=Math.min(pageVoxels,length-p*pageVoxels),values=o.process?new Float32Array(count):undefined,processed=o.process?new Uint8Array(count):undefined,bytes=(raw?.byteLength??0)+(values?.byteLength??0)+(processed?.byteLength??0);cache.set(p,{raw,values,processed,bytes});cachedBytes+=bytes;stats.pagesRead++;stats.peakCacheBytes=Math.max(stats.peakCacheBytes,cachedBytes)}
  if(resident){for(const p of missing)add(p)}else{
   missing.sort((a,b)=>a-b);const groups:number[][]=[];for(const p of missing){const group=groups.at(-1);if(group&&group.length<16&&group.at(-1)!+1===p)group.push(p);else groups.push([p])}
   for(let at=0;at<groups.length;at+=4)await Promise.all(groups.slice(at,at+4).map(async group=>{
    check();const first=group[0]*pageVoxels,last=Math.min(length,(group.at(-1)!+1)*pageVoxels),data=await backing!.slice(offset+first*width,offset+last*width).arrayBuffer();check();stats.largestReadBytes=Math.max(stats.largestReadBytes,data.byteLength)
    if(data.byteLength!==(last-first)*width)throw Error(`Truncated native page in ${info.name}`)
    for(const p of group){const count=Math.min(pageVoxels,length-p*pageVoxels),begin=(p*pageVoxels-first)*width;add(p,new DataView(data.slice(begin,begin+count*width)))}
   }))
  }
  if(cachedBytes>cacheBytes)throw Error('Native page cache exceeded its memory budget')
 }
 return {dims:info.dims,affine:info.affine,spacing:info.spacing,length,stats,maxPagesPerBatch:capacity,page:index=>Math.floor(index/pageVoxels),ensure,
  read(index){if(!Number.isSafeInteger(index)||index<0||index>=length||closed)throw Error('Native voxel index is outside the image');if(resident&&!o.process)return encodedRead(index)
   const p=Math.floor(index/pageVoxels),entry=cache.get(p);if(!entry)throw Error('Native voxel page was not prefetched');const at=index-p*pageVoxels
   if(entry.processed){if(!entry.processed[at]){const v=resident?encodedRead(index):decode(entry.raw!,at*width);entry.values![at]=o.process!(v,index);entry.processed[at]=1}return entry.values![at]}
   return decode(entry.raw!,at*width)},
  close(){closed=true;cache.clear();encoded.length=0;cachedBytes=0;backing=null}
 }
}
