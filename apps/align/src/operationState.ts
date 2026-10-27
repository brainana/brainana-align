export type ActivityKind='registration'|'processing'|'preparation'|'export'|'display'
export type OperationState={
 activity:{id:number;status:'idle'|'running'|'cancelling';kind:ActivityKind;text:string;fraction?:number;outcome?:'completed'|'cancelled'|'failed'}
 overlay:{generation:number;status:'idle'|'scheduled'|'running'|'ready'|'cancelled'|'failed'}
 proposal:'none'|'pending'|'accepted'|'rejected'
}
export type OperationEvent=
 |{type:'activity/begin';kind:ActivityKind;text:string}
 |{type:'activity/phase';id:number;text:string;fraction?:number}
 |{type:'activity/cancel';id:number}
 |{type:'activity/end';id:number;outcome?:'completed'|'cancelled'|'failed'}
 |{type:'overlay/request';status:'idle'|'scheduled'|'running'}
 |{type:'overlay/settle';generation:number;status:'ready'|'cancelled'|'failed'}
 |{type:'proposal/project';pending:boolean}
 |{type:'proposal/decision';decision:'accepted'|'rejected'}
 |{type:'proposal/clear'}
export const initialOperationState=():OperationState=>({activity:{id:0,status:'idle',kind:'registration',text:''},overlay:{generation:0,status:'idle'},proposal:'none'})
/** Pure transitions. A late worker/paint callback cannot finish a newer task. */
export function reduceOperation(state:OperationState,event:OperationEvent):OperationState{
 switch(event.type){
  case 'activity/begin':return state.activity.status!=='idle'?state:{...state,activity:{id:state.activity.id+1,status:'running',kind:event.kind,text:event.text}}
  case 'activity/phase':return state.activity.id!==event.id||state.activity.status!=='running'?state:{...state,activity:{...state.activity,text:event.text,fraction:event.fraction}}
  case 'activity/cancel':return state.activity.id!==event.id||state.activity.status!=='running'?state:{...state,activity:{...state.activity,status:'cancelling'}}
  case 'activity/end':return state.activity.id!==event.id||state.activity.status==='idle'?state:{...state,activity:{...state.activity,status:'idle',outcome:event.outcome??(state.activity.status==='cancelling'?'cancelled':'completed')}}
  case 'overlay/request':return {...state,overlay:{generation:state.overlay.generation+1,status:event.status}}
  case 'overlay/settle':return event.generation!==state.overlay.generation||state.overlay.status!=='running'?state:{...state,overlay:{...state.overlay,status:event.status}}
  case 'proposal/project':return event.pending?(state.proposal==='pending'?state:{...state,proposal:'pending'}):state.proposal==='pending'?{...state,proposal:'none'}:state
  case 'proposal/decision':return state.proposal!=='pending'?state:{...state,proposal:event.decision}
  case 'proposal/clear':return state.proposal==='none'?state:{...state,proposal:'none'}
 }
}
export function createOperationState(){
 let state=initialOperationState()
 return {get state(){return state},dispatch(event:OperationEvent){state=reduceOperation(state,event);return state}}
}
export type OperationStore=ReturnType<typeof createOperationState>

export type GuidePriorityInput={welcome?:boolean;viewerReady:boolean;mri:boolean;ct:boolean;preparation?:{mri:boolean;ct:boolean;ready:boolean;open?:boolean};busy:boolean;busyKind?:ActivityKind;pendingProposal:boolean}
export type GuidePriority={key:'welcome'|'viewer'|'mri'|'ct'|'prepare'|'proposal'|'route'}|{key:'working';activityKind:ActivityKind}
/** Read-only guide projection. Prerequisites always outrank remembered routes;
 * busy work outranks decisions, and a pending proposal outranks tool teaching. */
export function projectGuidePriority(s:GuidePriorityInput,route:string):GuidePriority{
 if(s.welcome)return {key:'welcome'}
 if(!s.viewerReady)return {key:'viewer'}
 if(!s.mri&&!s.preparation?.mri)return {key:'mri'}
 if(!s.ct&&!s.preparation?.ct)return {key:'ct'}
 if(s.preparation&&!s.preparation.ready&&!s.busy)return {key:'prepare'}
 if(s.busy)return {key:'working',activityKind:s.busyKind??'registration'}
 if(s.pendingProposal&&route!=='choose'&&route!=='done')return {key:'proposal'}
 return {key:'route'}
}
export const isGuidePrerequisite=(key:string)=>['welcome','viewer','mri','ct','prepare','working'].includes(key)
