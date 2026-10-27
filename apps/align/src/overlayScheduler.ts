import {createOperationState,type OperationStore} from './operationState'
export type OverlayOutcome<T>={status:'ready';result:T;generation:number}|{status:'stale';generation:number}|{status:'cancelled';generation:number}|{status:'failed';error:unknown;generation:number}
/** One worker at a time; supersession aborts immediately, before the debounce.
 * Generation checks also protect against transports that finish after abort. */
export function createOverlayScheduler(store:OperationStore=createOperationState()){
 let controller:AbortController|null=null,timer:ReturnType<typeof setTimeout>|null=null
 function request(status:'idle'|'scheduled'|'running'){
  if(timer!==null){clearTimeout(timer);timer=null}
  controller?.abort();controller=null
  return store.dispatch({type:'overlay/request',status}).overlay.generation
 }
 return {
  get state(){return store.state.overlay},
  invalidate(){request('idle')},
  cancel(){controller?.abort()},
  schedule(start:()=>void,delay:number){const generation=request('scheduled');timer=setTimeout(()=>{timer=null;if(store.state.overlay.generation===generation)start()},delay)},
  isCurrent(generation:number){return store.state.overlay.generation===generation},
  async run<T>(work:(signal:AbortSignal,generation:number)=>Promise<T>):Promise<OverlayOutcome<T>>{
   const generation=request('running'),own=new AbortController();controller=own
   try{
    const result=await work(own.signal,generation)
    if(store.state.overlay.generation!==generation)return {status:'stale',generation}
    if(own.signal.aborted){store.dispatch({type:'overlay/settle',generation,status:'cancelled'});return {status:'cancelled',generation}}
    store.dispatch({type:'overlay/settle',generation,status:'ready'});return {status:'ready',result,generation}
   }catch(error){
    if(store.state.overlay.generation!==generation)return {status:'stale',generation}
    const status=own.signal.aborted?'cancelled':'failed';store.dispatch({type:'overlay/settle',generation,status})
    return status==='cancelled'?{status,generation}:{status,error,generation}
   }finally{if(controller===own)controller=null}
  },
 }
}
