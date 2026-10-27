import type {FitState} from './appTypes'
import {LANDMARK_SENSITIVITY_MIN_PAIRS,LANDMARK_SENSITIVITY_MAX_PAIRS,type LandmarkSensitivityInput,type LandmarkSensitivityReport,type SensitivityModel} from '../../../packages/imaging-math/landmarkSensitivity'

const escapeHtml=(value:string)=>value.replace(/[&<>"']/g,c=>({'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;',"'":'&#39;'}[c]!))
function modelFor(fit:FitState):SensitivityModel|null{
 if(fit.initializationSource==='automatic'||fit.initializationSource==='image-geometry'||fit.nonlinear?.kernel==='jip-dense-pull')return null
 const model=fit.landmarkModel??fit.model??'rigid'
 return Object.hasOwn(LANDMARK_SENSITIVITY_MIN_PAIRS,model)?model as SensitivityModel:null
}
export function landmarkSensitivityAvailability(fit:FitState){
 const model=modelFor(fit),count=fit.landmarkSnapshot.filter(p=>p.enabled&&p.mri&&p.ct).length
 if(!model)return {available:false,model,count,reason:'Leave-one-out refits apply to landmark alignments.'}
 if(fit.proposal)return {available:false,model,count,reason:'Accept or reject the registration proposal before running landmark sensitivity.'}
 const minimum=LANDMARK_SENSITIVITY_MIN_PAIRS[model],maximum=LANDMARK_SENSITIVITY_MAX_PAIRS[model]
 if(count<minimum)return {available:false,model,count,reason:`${model} sensitivity needs at least ${minimum} complete enabled pairs (${count} fitted). This is an application heuristic, not a statistical guarantee.`}
 if(count>maximum)return {available:false,model,count,reason:`On-demand ${model} sensitivity is limited to ${maximum} pairs to bound refit work.`}
 if(model==='spline'&&fit.nonlinear?.kernel!=='3d-polyharmonic-r')return {available:false,model,count,reason:'The original landmark spline smoothing setting is unavailable.'}
 return {available:true,model,count,reason:''}
}
export function landmarkSensitivityInput(fit:FitState):LandmarkSensitivityInput{
 const gate=landmarkSensitivityAvailability(fit)
 if(!gate.available||!gate.model)throw Error(gate.reason)
 return {model:gate.model,smoothing:fit.nonlinear?.kernel==='3d-polyharmonic-r'?fit.nonlinear.smoothing:undefined,pairs:fit.landmarkSnapshot.filter(p=>p.enabled&&p.mri&&p.ct).map(p=>({id:p.id,source:[...(fit.direction==='ct-mri'?p.ct!:p.mri!)] as [number,number,number],target:[...(fit.direction==='ct-mri'?p.mri!:p.ct!)] as [number,number,number]}))}
}
/** Markup is cheap: refitting occurs only after the user requests it. */
export function landmarkSensitivityHtml(fit:FitState):string{
 const gate=landmarkSensitivityAvailability(fit)
 if(!gate.model)return ''
 return `<details class="landmark-sensitivity"><summary>Landmark refit sensitivity</summary><p>Omit each fitted pair, refit the original landmark model, and compare predictions. This does not measure target accuracy or independently validate the alignment.</p><p>Analysis gates: rigid/similarity 6 pairs; anisotropic/affine 8; spline 20. These are application heuristics; retained fits must remain valid.</p>${gate.available?'<button type="button" data-landmark-sensitivity-start>Calculate leave-one-out sensitivity</button><button type="button" data-landmark-sensitivity-cancel hidden>Cancel sensitivity</button>':`<p>${escapeHtml(gate.reason)}</p>`}<div data-landmark-sensitivity-report role="status" aria-live="polite"></div></details>`
}
export function landmarkSensitivityReportHtml(report:LandmarkSensitivityReport):string{
 if(report.status==='unavailable')return `<p>${escapeHtml(report.reason??'Sensitivity unavailable.')}</p>`
 const mm=(value:number|undefined)=>value===undefined?'unavailable':`${value.toFixed(3)} mm`
 const rows=report.omissions.map(row=>row.valid?`<tr><td>${row.id}</td><td>${mm(row.heldOutErrorMm)}</td><td>${mm(row.rmsMappingChangeMm)}</td><td>${mm(row.maximumMappingChangeMm)}</td></tr>`:`<tr><td>${row.id}</td><td colspan="3">Invalid subset: ${escapeHtml(row.reason??'unstable fit')}</td></tr>`).join('')
 return `<p><strong>${report.status==='unstable'?'Unstable leave-one-out analysis':'Leave-one-out refit sensitivity'}</strong> · ${report.validOmissions}/${report.pairCount} valid refits of the original ${escapeHtml(report.model)} landmark model.</p><p>Omitted-point prediction RMS: ${mm(report.heldOutRmsMm)}. Largest mapping change at evaluated locations: ${mm(report.maximumMappingChangeMm)}.</p><p>Mapping changes use ${report.probeCount} ${escapeHtml(report.probeLocations)}. Manual and image-based refinements are excluded from these landmark refits.</p><table><thead><tr><th>Omitted pair</th><th>Prediction error</th><th>RMS map change</th><th>Max map change</th></tr></thead><tbody>${rows}</tbody></table>${report.warnings.map(w=>`<p>${escapeHtml(w)}</p>`).join('')}`
}

/** One launch-local controller; no FitState/session fields and no automatic refits. */
export function installLandmarkSensitivityReview(container:HTMLElement,getFit:()=>FitState|null){
 type CacheEntry={snapshot:FitState['landmarkSnapshot'];model:SensitivityModel|null;direction:string;smoothing?:number;report:LandmarkSensitivityReport}
 const cache=new WeakMap<FitState,CacheEntry>()
 let job:{worker:Worker;fit:FitState;snapshot:FitState['landmarkSnapshot'];direction:string;model:SensitivityModel;smoothing?:number;timer:ReturnType<typeof setTimeout>}|null=null,lastPanel:HTMLElement|null=null,disposed=false
 const panel=()=>container.querySelector<HTMLElement>('[data-landmark-sensitivity-report]')
 const controls=(running:boolean)=>{const start=container.querySelector<HTMLButtonElement>('[data-landmark-sensitivity-start]'),cancel=container.querySelector<HTMLButtonElement>('[data-landmark-sensitivity-cancel]');if(start)start.disabled=running;if(cancel)cancel.hidden=!running}
 const stop=(message?:string)=>{if(job){clearTimeout(job.timer);job.worker.terminate();job=null}controls(false);const output=panel();if(output&&message)output.textContent=message}
 const changed=()=>Boolean(job&&(getFit()!==job.fit||job.fit.landmarkSnapshot!==job.snapshot||job.fit.direction!==job.direction||modelFor(job.fit)!==job.model||(job.fit.nonlinear?.kernel==='3d-polyharmonic-r'?job.fit.nonlinear.smoothing:undefined)!==job.smoothing||job.fit.proposal))
 const sync=()=>{
  if(disposed)return
  const fit=getFit(),output=panel()
  if(changed())stop('Alignment changed; sensitivity cancelled.')
  if(output===lastPanel)return
  lastPanel=output
  if(job){controls(true);if(output)output.textContent='Landmark sensitivity is running…';return}
  if(!fit||!output)return
  const saved=cache.get(fit),smoothing=fit.nonlinear?.kernel==='3d-polyharmonic-r'?fit.nonlinear.smoothing:undefined
  if(saved&&saved.snapshot===fit.landmarkSnapshot&&saved.model===modelFor(fit)&&saved.direction===fit.direction&&saved.smoothing===smoothing)output.innerHTML=landmarkSensitivityReportHtml(saved.report)
 }
 const click=(event:Event)=>{
  const target=event.target as Element|null
  if(target?.closest('[data-landmark-sensitivity-cancel]')){stop('Landmark sensitivity cancelled. The alignment is unchanged.');return}
  if(!target?.closest('[data-landmark-sensitivity-start]')||job)return
  const fit=getFit(),output=panel();if(!fit||!output)return
  let input:LandmarkSensitivityInput
  try{input=landmarkSensitivityInput(fit)}catch(error){output.textContent=error instanceof Error?error.message:String(error);return}
  const saved=cache.get(fit)
  if(saved&&saved.snapshot===fit.landmarkSnapshot&&saved.model===input.model&&saved.direction===fit.direction&&saved.smoothing===input.smoothing){output.innerHTML=landmarkSensitivityReportHtml(saved.report);return}
  let worker:Worker
  try{worker=new Worker(new URL('./landmarkQualityWorker.ts',import.meta.url),{type:'module'})}catch(error){output.textContent=error instanceof Error?error.message:String(error);return}
  job={worker,fit,snapshot:fit.landmarkSnapshot,direction:fit.direction,model:input.model,smoothing:input.smoothing,timer:setTimeout(()=>stop('Landmark sensitivity exceeded its time budget. The alignment is unchanged.'),20000)}
  controls(true);output.textContent=`Refitting ${input.pairs.length} omitted-pair subsets…`
  worker.onerror=event=>{if(job?.worker===worker)stop(event.message||'Landmark sensitivity failed. The alignment is unchanged.')}
  worker.onmessage=({data})=>{
   if(job?.worker!==worker)return
   if(changed()){stop('Alignment changed; sensitivity cancelled.');return}
   const current=panel()
   if(data.type==='progress'){if(current)current.textContent=`Landmark sensitivity: ${data.completed}/${data.total} refits…`;return}
   if(data.type==='error'){stop(data.message);return}
   if(data.type==='result'){
    cache.set(fit,{snapshot:fit.landmarkSnapshot,model:input.model,direction:fit.direction,smoothing:input.smoothing,report:data.report})
    stop();if(current)current.innerHTML=landmarkSensitivityReportHtml(data.report)
   }
  }
  try{worker.postMessage(input)}catch(error){stop(error instanceof Error?error.message:String(error))}
 }
 const observer=new MutationObserver(sync);observer.observe(container,{childList:true,subtree:true});container.addEventListener('click',click)
 const dispose=()=>{disposed=true;stop();observer.disconnect();container.removeEventListener('click',click);window.removeEventListener('pagehide',dispose)}
 window.addEventListener('pagehide',dispose,{once:true});sync()
 return {dispose,cancel:()=>stop('Landmark sensitivity cancelled. The alignment is unchanged.')}
}
