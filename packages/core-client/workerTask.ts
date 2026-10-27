/** One-shot worker ownership: settle once and release every listener on all exits. */
export type WorkerTaskOptions<T> = {
 create:()=>Worker
 request:unknown
 signal?:AbortSignal
 cancelled:string
 failed:string
 progress?:(data:any)=>void
 result:(data:any)=>T
}
export function runWorkerTask<T>(options:WorkerTaskOptions<T>):Promise<T>{
 return new Promise((resolve,reject)=>{
  if(options.signal?.aborted){reject(Error(options.cancelled));return}
  let worker:Worker|undefined,settled=false,listening=false
  const finish=(error:unknown,result?:T,successful=false)=>{
   if(settled)return
   settled=true
   if(listening)options.signal?.removeEventListener('abort',cancel)
   if(worker){worker.onmessage=null;worker.onerror=null;worker.onmessageerror=null;worker.terminate()}
   if(successful)resolve(result as T);else reject(error)
  }
  const cancel=()=>finish(Error(options.cancelled))
  try{
   worker=options.create()
   options.signal?.addEventListener('abort',cancel,{once:true});listening=Boolean(options.signal)
   worker.onerror=event=>finish(Error(event.message||options.failed))
   worker.onmessageerror=()=>finish(Error(options.failed+': unreadable worker response.'))
   worker.onmessage=({data})=>{
    if(settled)return
    try{
     if(!data||typeof data!=='object')throw Error(options.failed+': invalid worker response.')
     if(data.type==='progress')options.progress?.(data)
     else if(data.type==='error')finish(Error(data.message||options.failed))
     else if(data.type==='result')finish(null,options.result(data),true)
     else throw Error(options.failed+': unknown worker response.')
    }catch(error){finish(error)}
   }
   worker.postMessage(options.request)
  }catch(error){finish(error)}
 })
}
