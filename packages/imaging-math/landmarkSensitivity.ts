import {fitLandmarks,type LandmarkModel} from './landmarkFit'
import {createPointMapping,fitNonlinearSpline} from './nonlinearSpline'
import type {Vec3} from './rigid'

export type SensitivityModel=LandmarkModel|'spline'
// Application heuristics: these counts do not establish statistical validity.
export const LANDMARK_SENSITIVITY_MIN_PAIRS={rigid:6,similarity:6,anisotropic:8,affine:8,spline:20} as const
export const LANDMARK_SENSITIVITY_MAX_PAIRS={rigid:128,similarity:128,anisotropic:128,affine:128,spline:64} as const
export type SensitivityPair={id:number;source:Vec3;target:Vec3}
export type LandmarkSensitivityInput={model:SensitivityModel;pairs:readonly SensitivityPair[];smoothing?:number;probePoints?:readonly Vec3[]}
export type OmissionSensitivity={id:number;valid:boolean;reason?:string;heldOutErrorMm?:number;rmsMappingChangeMm?:number;maximumMappingChangeMm?:number}
export type LandmarkSensitivityReport={
 method:'leave-one-out-refit';status:'complete'|'unstable'|'unavailable';model:SensitivityModel;pairCount:number;minimumPairs:number;
 probeCount:number;probeLocations:'provided moving RAS points'|'fitted moving landmarks';sourceRadiusMm?:number;
 trainingRmsMm?:number;heldOutRmsMm?:number;maximumMappingChangeMm?:number;validOmissions:number;omissions:OmissionSensitivity[];
 reason?:string;warnings:string[]
}
const distance=(a:Vec3,b:Vec3)=>Math.hypot(...a.map((v,i)=>v-b[i]))
const errorText=(error:unknown)=>error instanceof Error?error.message:String(error)
const finitePoint=(p:Vec3)=>Array.isArray(p)&&p.length===3&&p.every(Number.isFinite)
const limitations=[
 'Counts are application heuristics, not statistical guarantees. Every retained subset must also fit successfully.',
 'Training RMS and leave-one-out refit sensitivity are not independent validation or an estimate of target accuracy.',
 'Clustered landmarks and extrapolation can leave unsampled anatomy weakly constrained; low values here do not establish anatomical correspondence.'
]

function refit(input:LandmarkSensitivityInput,pairs:SensitivityPair[]){
 const source=pairs.map(p=>p.source),target=pairs.map(p=>p.target)
 const fitted=input.model==='spline'?fitNonlinearSpline(source,target,input.smoothing??.01):fitLandmarks(source,target,input.model)
 if(fitted.diagnostics.conditionNumber>1000||fitted.diagnostics.warnings.some(w=>/numerically unstable|poorly conditioned/i.test(w)))throw Error('Retained landmark geometry is numerically unstable')
 const mapping=createPointMapping(fitted.matrix,'spline' in fitted?fitted.spline:undefined)
 return {mapping,rms:fitted.rms}
}

/** Bounded, read-only refits. Run in a worker for UI use; cancellation is checked between omissions. */
export async function landmarkLeaveOneOutSensitivity(input:LandmarkSensitivityInput,options:{
 shouldCancel?:()=>boolean;onProgress?:(completed:number,total:number)=>void;timeBudgetMs?:number;
}={}):Promise<LandmarkSensitivityReport>{
 if(!Object.hasOwn(LANDMARK_SENSITIVITY_MIN_PAIRS,input.model))throw Error('Unsupported sensitivity model')
 const count=input.pairs.length,minimum=LANDMARK_SENSITIVITY_MIN_PAIRS[input.model]
 const report:LandmarkSensitivityReport={method:'leave-one-out-refit',status:'unavailable',model:input.model,pairCount:count,minimumPairs:minimum,probeCount:0,probeLocations:input.probePoints?'provided moving RAS points':'fitted moving landmarks',validOmissions:0,omissions:[],warnings:[...limitations]}
 if(count<minimum){report.reason=`At least ${minimum} complete enabled pairs are required for this ${input.model} sensitivity analysis.`;return report}
 if(count>LANDMARK_SENSITIVITY_MAX_PAIRS[input.model]){report.reason=`This on-demand ${input.model} analysis is limited to ${LANDMARK_SENSITIVITY_MAX_PAIRS[input.model]} pairs to bound refit work. The alignment is unchanged.`;return report}
 if(input.pairs.some(p=>!Number.isInteger(p.id)||!finitePoint(p.source)||!finitePoint(p.target))||new Set(input.pairs.map(p=>p.id)).size!==count){report.reason='Sensitivity needs distinct pair IDs and finite RAS-mm coordinates.';return report}
 // Own all coordinates: neither the accepted snapshot nor caller probes can
 // be changed by refits, cancellation, or a future fitting implementation.
 const pairs=input.pairs.map(p=>({id:p.id,source:[...p.source] as Vec3,target:[...p.target] as Vec3})),probes=(input.probePoints??pairs.map(p=>p.source)).map(p=>[...p] as Vec3)
 if(!probes.length||probes.length>256||probes.some(p=>!finitePoint(p))){report.reason='Use 1–256 finite moving RAS-mm probe locations.';return report}
 report.probeCount=probes.length
 const center=pairs.reduce((sum,p)=>sum.map((v,a)=>v+p.source[a]/count) as Vec3,[0,0,0] as Vec3)
 report.sourceRadiusMm=Math.sqrt(pairs.reduce((sum,p)=>sum+distance(p.source,center)**2,0)/count)
 const started=Date.now(),budget=options.timeBudgetMs??15000
 const check=()=>{if(options.shouldCancel?.())throw Error('Landmark sensitivity cancelled.');if(Date.now()-started>budget)throw Error('Landmark sensitivity exceeded its time budget; the alignment is unchanged.')}
 check()
 let baseline:ReturnType<typeof refit>,original:Vec3[]
 try{baseline=refit(input,pairs);original=probes.map(p=>baseline.mapping.forward(p));report.trainingRmsMm=baseline.rms}catch(error){report.reason=`Full landmark refit is invalid: ${errorText(error)}`;return report}
 for(let omitted=0;omitted<count;omitted++){
  check()
  const pair=pairs[omitted]
  try{
   const retained=refit(input,pairs.filter((_,i)=>i!==omitted)),prediction=retained.mapping.forward(pair.source),changes=probes.map((p,i)=>distance(retained.mapping.forward(p),original[i]))
   const row:OmissionSensitivity={id:pair.id,valid:true,heldOutErrorMm:distance(prediction,pair.target),rmsMappingChangeMm:Math.sqrt(changes.reduce((sum,v)=>sum+v*v,0)/changes.length),maximumMappingChangeMm:Math.max(...changes)}
   if(![row.heldOutErrorMm,row.rmsMappingChangeMm,row.maximumMappingChangeMm].every(Number.isFinite))throw Error('Retained fit produced nonfinite point mappings')
   report.omissions.push(row);report.validOmissions++
  }catch(error){report.omissions.push({id:pair.id,valid:false,reason:errorText(error)})}
  options.onProgress?.(omitted+1,count)
  // Also allows cancellation in non-worker callers. The app terminates its
  // worker immediately on Cancel, including during an individual refit.
  await new Promise<void>(resolve=>setTimeout(resolve,0))
 }
 check()
 const valid=report.omissions.filter(row=>row.valid)
 report.status=valid.length===count?'complete':'unstable'
 if(valid.length){report.heldOutRmsMm=Math.sqrt(valid.reduce((sum,row)=>sum+row.heldOutErrorMm!**2,0)/valid.length);report.maximumMappingChangeMm=Math.max(...valid.map(row=>row.maximumMappingChangeMm!))}
 if(valid.length!==count)report.warnings.unshift(`${count-valid.length} omitted-pair subset(s) could not produce a stable valid fit. Aggregates cover only ${valid.length}/${count} valid refits; this analysis is unstable.`)
 return report
}
