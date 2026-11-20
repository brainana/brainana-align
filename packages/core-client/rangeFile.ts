/** Disk-backed files reopened by the desktop. Descriptors cross worker boundaries;
 * native voxel data stays on disk and is read through authenticated HTTP ranges. */
export type DiskFileDescriptor={id:string;name:string;path:string;size:number;lastModified:number}
export type RangeFileDescriptor=DiskFileDescriptor&{kind:'brainana-range-file-v1';url:string;sessionToken:string}

class RangeFile {
 readonly type='application/octet-stream'
 readonly webkitRelativePath=''
 constructor(readonly descriptor:RangeFileDescriptor,private start=0,private end=descriptor.size){}
 get name(){return this.descriptor.name}
 get size(){return this.end-this.start}
 get lastModified(){return this.descriptor.lastModified}
 slice(start=0,end=this.size){const clamp=(n:number)=>{n=Number.isNaN(n)?0:Math.trunc(n);return Math.max(0,Math.min(this.size,n<0?this.size+n:n))};return new RangeFile(this.descriptor,this.start+clamp(start),this.start+Math.max(clamp(start),clamp(end)))}
 private async response(signal?:AbortSignal){
  const response=await fetch(this.descriptor.url,{headers:{'x-brainana-session':this.descriptor.sessionToken,Range:`bytes=${this.start}-${this.end-1}`},signal})
  if(!response.ok)throw Error(response.status===409?'An original image changed on disk. Reopen the session and locate the original file.':'The original image is no longer available. Reopen the session to locate it.')
  if(Number(response.headers.get('content-length'))!==this.size)throw Error('Incomplete original-image range response')
  return response
 }
 async arrayBuffer(){return this.size?(await this.response()).arrayBuffer():new ArrayBuffer(0)}
 async text(){return new TextDecoder().decode(await this.arrayBuffer())}
 stream():ReadableStream<Uint8Array>{
  if(!this.size)return new Blob().stream()
  const abort=new AbortController();let reader:ReadableStreamDefaultReader<Uint8Array>|undefined,cancelled=false
  const ready=this.response(abort.signal).then(response=>{if(!response.body)throw Error('Original image response has no data');reader=response.body.getReader();if(cancelled)void reader.cancel().catch(()=>{});return reader})
  // A consumer can cancel before the fetch resolves. Keep that rejection owned.
  void ready.catch(()=>{})
  return new ReadableStream({async pull(controller){try{const r=await ready;if(cancelled)return;const next=await r.read();if(next.done)controller.close();else controller.enqueue(next.value)}catch(error){if(!cancelled)controller.error(error)}},async cancel(){cancelled=true;abort.abort();await reader?.cancel().catch(()=>{})}})
 }
}

export function diskFile(descriptor:DiskFileDescriptor,origin:string,sessionToken:string):File{
 if(!/^[a-f0-9]{48}$/.test(descriptor.id)||!Number.isSafeInteger(descriptor.size)||descriptor.size<0)throw Error('Invalid original-image file descriptor')
 return new RangeFile({...descriptor,kind:'brainana-range-file-v1',url:new URL('/api/session-image?file='+descriptor.id,origin).href,sessionToken}) as unknown as File
}
export function workerFile(file:File|undefined):File|RangeFileDescriptor|undefined{return file instanceof RangeFile?file.descriptor:file}
export function restoreWorkerFile(file:File|RangeFileDescriptor):File{return (file as RangeFileDescriptor)?.kind==='brainana-range-file-v1'?new RangeFile(file as RangeFileDescriptor) as unknown as File:file as File}
export function diskFilePath(file:File):string|undefined{return file instanceof RangeFile?file.descriptor.path:undefined}
export async function materializeFile(file:File):Promise<File>{return file instanceof RangeFile?new File([await file.arrayBuffer()],file.name,{lastModified:file.lastModified,type:file.type}):file}
