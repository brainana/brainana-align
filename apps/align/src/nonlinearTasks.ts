import {runWorkerTask} from '../../../packages/core-client/workerTask'
export async function runNonlinearTask<T>(request:unknown,progress:(text:string)=>void):Promise<T>{
 const button=document.querySelector<HTMLButtonElement>('#alignment-cancel')!,controller=new AbortController()
 const cancel=()=>controller.abort()
 button.addEventListener('click',cancel);button.classList.remove('hidden')
 try{return await runWorkerTask<T>({
  create:()=>new Worker(new URL('./nonlinearWorker.ts',import.meta.url),{type:'module'}),
  request,signal:controller.signal,cancelled:'Nonlinear operation cancelled.',failed:'Nonlinear worker failed',
  progress:data=>progress(data.text),result:data=>data.result,
 })}finally{button.removeEventListener('click',cancel);button.classList.add('hidden')}
}
