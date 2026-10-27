import {createOperationState,type OperationStore,type ActivityKind} from './operationState'
export type {ActivityKind} from './operationState'
/** Reveal an explicitly requested control, including every collapsed ancestor. */
export function revealControl(control:Element|null|undefined){
  for(let node=control;node;node=node.parentElement)if(node instanceof window.HTMLDetailsElement)node.open=true
}
export function installWorkflowFocus() {
  const panels:Record<string,string[]>= {processing:['#initial-processing-panel'],landmarks:['#landmarks-panel','#landmark-alignment-panel'],followup:['#followup-panel'],manual:['#manual-panel'],automatic:['#refine-panel']}
  const activate=(mode:string,source='workflow')=>{
    if(mode==='automatic'||mode==='followup'){
      const operation=document.querySelector<HTMLSelectElement>('#intensity-operation')!
      operation.value=mode==='followup'?'local':'global';operation.dispatchEvent(new window.CustomEvent('change',{detail:{source}}))
    }
    for(const other of document.querySelectorAll<HTMLButtonElement>('[data-workflow]'))other.setAttribute('aria-pressed',String(other.dataset.workflow===mode))
  }
  // Native section toggles change that section only. Closing a section must not
  // click a workflow shortcut, change operation, or open/close any sibling.
  for(const [mode,selectors] of Object.entries(panels))for(const selector of selectors){
    const panel=document.querySelector<HTMLDetailsElement>(selector)!;panel.dataset.workflowPanel=mode
    panel.querySelector('summary')!.addEventListener('click',()=>{
      if(!panel.open)activate(mode,'disclosure')
      else queueMicrotask(()=>{if(!selectors.some(id=>document.querySelector<HTMLDetailsElement>(id)?.open))document.querySelector<HTMLButtonElement>(`[data-workflow="${mode}"]`)?.setAttribute('aria-pressed','false')})
    })
  }
  // Explicit workflow shortcuts retain focused navigation. Ordinary disclosure
  // headings above are independent and never delegate to these shortcuts.
  for(const button of document.querySelectorAll<HTMLButtonElement>('[data-workflow]'))button.addEventListener('click',()=>{
    const mode=button.dataset.workflow!;activate(mode)
    for(const [name,selectors] of Object.entries(panels))for(const selector of selectors)document.querySelector<HTMLDetailsElement>(selector)!.open=name===mode&&selector!=='#landmark-alignment-panel'
  })
}
export function createAlignmentActivity(operations:OperationStore=createOperationState(),beforeBegin:()=>void=()=>{}) {
  const banner=document.querySelector<HTMLElement>('#alignment-progress')!,label=banner.querySelector<HTMLElement>('#alignment-progress-label')!,bar=banner.querySelector<HTMLProgressElement>('progress')!
  const show=(text:string,fraction?:number)=>{label.textContent=text;if(fraction===undefined)bar.removeAttribute('value');else bar.value=Math.max(0,Math.min(1,fraction))}
  const paint=()=>new Promise<void>(resolve=>requestAnimationFrame(()=>setTimeout(resolve,0)))
  const running=(id:number)=>operations.state.activity.id===id&&operations.state.activity.status==='running'
  const end=(outcome?:'completed'|'cancelled'|'failed',id=operations.state.activity.id)=>{operations.dispatch({type:'activity/end',id,outcome});if(operations.state.activity.id!==id||operations.state.activity.status!=='idle')return;banner.classList.add('hidden');document.querySelector('main')!.inert=false;document.querySelector<HTMLElement>('.topbar')!.inert=false}
  document.querySelector('#alignment-cancel')!.addEventListener('click',()=>operations.dispatch({type:'activity/cancel',id:operations.state.activity.id}))
  return {
    get busy(){return operations.state.activity.status!=='idle'},
    get kind(){return operations.state.activity.kind},
    get id(){return operations.state.activity.id},
    get state(){return operations.state},
    async begin(text:string,nextKind:ActivityKind='registration'){if(operations.state.activity.status!=='idle')return false;beforeBegin();const {id,kind}=operations.dispatch({type:'activity/begin',kind:nextKind,text}).activity;banner.dataset.activity=kind;banner.querySelector('progress')!.setAttribute('aria-label',kind==='registration'?'Alignment progress':kind==='processing'?'Preprocessing progress':'Task progress');document.querySelector('#alignment-cancel')!.textContent=kind==='registration'?'Cancel registration':kind==='processing'?'Cancel preprocessing':kind==='export'?'Cancel export':kind==='preparation'?'Cancel preparation':'Cancel overlay refresh';show(text);banner.classList.remove('hidden');document.querySelector('main')!.inert=true;document.querySelector<HTMLElement>('.topbar')!.inert=true;await paint();if(!running(id)){end('cancelled',id);return false}return true},
    async phase(text:string,fraction?:number,id=operations.state.activity.id){const previous=operations.state;operations.dispatch({type:'activity/phase',id,text,fraction});if(operations.state===previous)return;show(text,fraction);await paint()},
    end,
  }
}
