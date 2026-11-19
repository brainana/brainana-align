import fs from 'node:fs'
import fsp from 'node:fs/promises'
/** Test-only real range reader. Node24 openAsBlob reports file sizes modulo4GiB
 * on this host; the production browser File contract must not inherit that
 * harness bug. Every byte is read from the real sparse file with64-bit offsets. */
export class NativeDiskBlob {
 constructor(filename,start=0,end=fs.statSync(filename).size){this.filename=filename;this.start=start;this.end=end;this.size=end-start}
 slice(start=0,end=this.size){const clamp=v=>Math.max(0,Math.min(this.size,v<0?this.size+v:v));return new NativeDiskBlob(this.filename,this.start+clamp(start),this.start+Math.max(clamp(start),clamp(end)))}
 async arrayBuffer(){if(this.size>8*1024**2)throw Error('Test refused a monolithic native read');const handle=await fsp.open(this.filename,'r');try{const b=Buffer.alloc(this.size);let done=0;while(done<b.length){const n=(await handle.read(b,done,b.length-done,this.start+done)).bytesRead;if(!n)break;done+=n}return b.buffer.slice(b.byteOffset,b.byteOffset+done)}finally{await handle.close()}}
 stream(){const source=this;let at=0;return new ReadableStream({async pull(controller){if(at>=source.size){controller.close();return}const bytes=new Uint8Array(await source.slice(at,Math.min(source.size,at+65536)).arrayBuffer());at+=bytes.length;controller.enqueue(bytes)}})}
}
