import { multiplyMat4, type Mat4, type Vec3 } from './rigid'
import { determinant3, linearDiagnostics, type LinearParameter, type TransformModel } from './linear'

export type AutomaticMetric = 'nmi' | 'mi' | 'correlation-ratio' | 'normalized-correlation' | 'least-squares'
export type RotationRange = 0 | 15 | 30 | 90 | 180
export type AutomaticLinearSettings = {
  model: TransformModel
  enabledParameters: LinearParameter[]
  rotationRanges: [RotationRange, RotationRange, RotationRange]
  translationLimitMm: number
  scaleLimit: number
  shearLimit: number
  candidatesToRefine: number
  shrinkFactors: number[]
  sweepsPerLevel: number[]
}
export type AutomaticLinearResult = {
  matrix: Mat4
  scoreBefore: number
  scoreAfter: number
  parameters: number[]
  determinant: number
  warnings: string[]
  candidatesEvaluated: number
}

const parameterNames: LinearParameter[]=['tx','ty','tz','rx','ry','rz','sx','sy','sz','shxy','shxz','shyz']
const degrees = (value:number) => value * Math.PI / 180
const yieldToUi = () => new Promise<void>(resolve => setTimeout(resolve, 0))

export function automaticParameterMatrix(parameters:number[], movingCenter:Vec3, fixedCenter:Vec3):Mat4{
  const [tx,ty,tz,rxDeg,ryDeg,rzDeg,lsx,lsy,lsz,shxy,shxz,shyz]=parameters
  const rx=degrees(rxDeg),ry=degrees(ryDeg),rz=degrees(rzDeg)
  const cx=Math.cos(rx),sx=Math.sin(rx),cy=Math.cos(ry),sy=Math.sin(ry),cz=Math.cos(rz),sz=Math.sin(rz)
  const rotation:Mat4=[[cz*cy,cz*sy*sx-sz*cx,cz*sy*cx+sz*sx,0],[sz*cy,sz*sy*sx+cz*cx,sz*sy*cx-cz*sx,0],[-sy,cy*sx,cy*cx,0],[0,0,0,1]]
  // Positive exponential scales make reflection impossible unless explicitly implemented elsewhere.
  const scaleShear:Mat4=[[Math.exp(lsx),shxy,shxz,0],[0,Math.exp(lsy),shyz,0],[0,0,Math.exp(lsz),0],[0,0,0,1]]
  const fromMoving:Mat4=[[1,0,0,-movingCenter[0]],[0,1,0,-movingCenter[1]],[0,0,1,-movingCenter[2]],[0,0,0,1]]
  const toFixed:Mat4=[[1,0,0,fixedCenter[0]+tx],[0,1,0,fixedCenter[1]+ty],[0,0,1,fixedCenter[2]+tz],[0,0,0,1]]
  return multiplyMat4(toFixed,multiplyMat4(rotation,multiplyMat4(scaleShear,fromMoving)))
}

function angles(range:RotationRange):number[]{
  if(range===0)return [0]
  if(range<=30)return [-range,0,range]
  if(range===90)return [-90,-45,0,45,90]
  return [-180,-135,-90,-45,0,45,90,135]
}

function enabledForModel(settings:AutomaticLinearSettings):Set<LinearParameter>{
  if(settings.model==='rigid')return new Set(parameterNames.slice(0,6))
  if(settings.model==='similarity')return new Set([...parameterNames.slice(0,6),'sx'] as LinearParameter[])
  if(settings.model==='anisotropic')return new Set(parameterNames.slice(0,9))
  if(settings.model==='affine')return new Set(parameterNames)
  return new Set(settings.enabledParameters)
}

function normalized(values:number[], uniformScale:boolean):number[]{
  const result=[...values]
  if(uniformScale)result[7]=result[8]=result[6]
  return result
}

function stageAxes(settings:AutomaticLinearSettings, level:number):{axes:number[];uniformScale:boolean}{
  const enabled=enabledForModel(settings), last=settings.shrinkFactors.length-1
  const pose=[0,1,2,3,4,5]
  if(level===0 && [...enabled].some(name=>parameterNames.indexOf(name)>=6)) return {axes:[...pose,6],uniformScale:true}
  if(level<last && [...enabled].some(name=>parameterNames.indexOf(name)>=9)) return {axes:[...pose,6,7,8].filter(axis=>axis<6||settings.model==='affine'||enabled.has(parameterNames[axis])),uniformScale:false}
  return {axes:parameterNames.map((_,axis)=>axis).filter(axis=>enabled.has(parameterNames[axis])),uniformScale:settings.model==='similarity'}
}

type Scored = {p:number[];score:number}

/** Original bounded derivative-free line minimizer: bracket locally, then golden-section refine. */
function maximizeLine(current:Scored, axis:number, initialStep:number, lower:number, upper:number, uniformScale:boolean, evaluate:(p:number[])=>number):Scored{
  if(!(upper>lower)||initialStep<=0)return current
  const cache=new Map<number,Scored>()
  const at=(value:number):Scored=>{
    const bounded=Math.max(lower,Math.min(upper,value)),key=Number(bounded.toPrecision(12)),found=cache.get(key)
    if(found)return found
    const p=normalized([...current.p],uniformScale);p[axis]=bounded
    const result={p:normalized(p,uniformScale),score:evaluate(normalized(p,uniformScale))};cache.set(key,result);return result
  }
  const center=current.p[axis], left=at(center-initialStep), middle=at(center), right=at(center+initialStep)
  let best=[left,middle,right].sort((a,b)=>b.score-a.score)[0]
  let lo=Math.max(lower,center-initialStep),hi=Math.min(upper,center+initialStep)
  if(best!==middle){
    const direction=best===right?1:-1
    let previous=center,position=best.p[axis],step=initialStep
    for(let expansion=0;expansion<5;expansion++){
      step*=1.8;const next=at(position+direction*step)
      if(next.score<=best.score){lo=Math.min(previous,next.p[axis]);hi=Math.max(previous,next.p[axis]);break}
      previous=position;position=next.p[axis];best=next
      lo=Math.min(lo,position);hi=Math.max(hi,position)
      if(position===lower||position===upper)break
    }
  }
  const ratio=(Math.sqrt(5)-1)/2
  let x1=hi-ratio*(hi-lo),x2=lo+ratio*(hi-lo),f1=at(x1),f2=at(x2)
  for(let iteration=0;iteration<10&&hi-lo>Math.max(1e-7,initialStep*.08);iteration++){
    if(f1.score<f2.score){lo=x1;x1=x2;f1=f2;x2=lo+ratio*(hi-lo);f2=at(x2)}
    else{hi=x2;x2=x1;f2=f1;x1=hi-ratio*(hi-lo);f1=at(x1)}
  }
  return [best,f1,f2].sort((a,b)=>b.score-a.score)[0]
}

function distinctCandidates(candidates:Scored[],limit:number):Scored[]{
  const sorted=[...candidates].filter(item=>Number.isFinite(item.score)).sort((a,b)=>b.score-a.score),result:Scored[]=[]
  for(const candidate of sorted){
    const duplicate=result.some(existing=>Math.hypot(...candidate.p.slice(0,3).map((value,index)=>value-existing.p[index]))<.5&&Math.hypot(...candidate.p.slice(3,6).map((value,index)=>value-existing.p[index]))<1)
    if(!duplicate)result.push(candidate)
    if(result.length>=limit)break
  }
  return result
}

export async function optimizeAutomaticLinear(options:{
  settings:AutomaticLinearSettings
  movingCenter:Vec3
  fixedCenter:Vec3
  score:(matrix:Mat4,level:number)=>number
  progress?:(message:string)=>void
  cancelled?:()=>boolean
}):Promise<AutomaticLinearResult>{
  const {settings,movingCenter,fixedCenter}=options, enabled=enabledForModel(settings)
  if(!['tx','ty','tz','rx','ry','rz'].every(name=>enabled.has(name as LinearParameter)))throw new Error('Automatic initialization requires translation and rotation. Custom models may hold scale or shear fixed, but not the six pose parameters.')
  if(!settings.shrinkFactors.length||settings.shrinkFactors.length!==settings.sweepsPerLevel.length)throw new Error('Every registration level must have a matching optimization-sweep count.')
  const zero=Array(12).fill(0), matrix=(p:number[])=>automaticParameterMatrix(p,movingCenter,fixedCenter)
  let evaluations=0
  const evaluate=(p:number[],level:number)=>{evaluations++;return options.score(matrix(p),level)}
  const baseline=evaluate(zero,settings.shrinkFactors.length-1)
  const orientationSeeds:Scored[]=[]
  const ax=angles(settings.rotationRanges[0]),ay=angles(settings.rotationRanges[1]),az=angles(settings.rotationRanges[2]),total=ax.length*ay.length*az.length
  let searched=0
  for(const rx of ax)for(const ry of ay)for(const rz of az){
    if(options.cancelled?.())throw new Error('Automatic linear alignment cancelled.')
    const p=[0,0,0,rx,ry,rz,0,0,0,0,0,0],score=evaluate(p,0);searched++
    if(Number.isFinite(score))orientationSeeds.push({p,score})
    if(searched%24===0){options.progress?.(`Global orientation search: ${searched}/${total} candidates…`);await yieldToUi()}
  }
  let candidates=distinctCandidates(orientationSeeds,Math.max(12,settings.candidatesToRefine*4))
  if(!candidates.length)throw new Error('The global orientation search found no candidate with sufficient image overlap.')
  const limits=[settings.translationLimitMm,settings.translationLimitMm,settings.translationLimitMm,...settings.rotationRanges,Math.log(1+settings.scaleLimit),Math.log(1+settings.scaleLimit),Math.log(1+settings.scaleLimit),settings.shearLimit,settings.shearLimit,settings.shearLimit]
  // Optimize translation at every retained coarse orientation before final candidate selection.
  candidates=candidates.map(candidate=>{
    let result=candidate
    for(const axis of [0,1,2])result=maximizeLine(result,axis,Math.max(1,settings.shrinkFactors[0]),-limits[axis],limits[axis],false,p=>evaluate(p,0))
    return result
  })
  candidates=distinctCandidates(candidates,settings.candidatesToRefine)
  for(let level=0;level<settings.shrinkFactors.length;level++){
    const factor=settings.shrinkFactors[level],stage=stageAxes(settings,level),sweeps=settings.sweepsPerLevel[level]
    options.progress?.(`Automatic linear alignment: ${factor}× level, ${candidates.length} candidate${candidates.length===1?'':'s'}…`)
    const refined:Scored[]=[]
    for(let candidateIndex=0;candidateIndex<candidates.length;candidateIndex++){
      let result=candidates[candidateIndex]
      for(let sweep=0;sweep<sweeps;sweep++){
        const before=result.score
        for(const axis of stage.axes){
          const tolerance=[.2,.2,.2,.3,.3,.3,.002,.002,.002,.001,.001,.001][axis]*factor
          const step=[Math.max(.5,factor),Math.max(.5,factor),Math.max(.5,factor),Math.max(1,factor*2),Math.max(1,factor*2),Math.max(1,factor*2),Math.min(.12,.015*factor),Math.min(.12,.015*factor),Math.min(.12,.015*factor),Math.min(.08,.008*factor),Math.min(.08,.008*factor),Math.min(.08,.008*factor)][axis]
          result=maximizeLine(result,axis,Math.max(step,tolerance),-limits[axis],limits[axis],stage.uniformScale,p=>evaluate(p,level))
        }
        if(result.score-before<1e-7)break
        if(options.cancelled?.())throw new Error('Automatic linear alignment cancelled.')
        await yieldToUi()
      }
      refined.push(result)
    }
    candidates=distinctCandidates(refined,Math.max(1,level===0?settings.candidatesToRefine:Math.ceil(settings.candidatesToRefine/2)))
    // FLIRT-inspired perturb-and-retry: retain nearby rotation starts after the coarse fit.
    if(level===0&&settings.shrinkFactors.length>1){
      const perturbed:Scored[]=[...candidates]
      for(const candidate of candidates.slice(0,2))for(const axis of [3,4,5])for(const sign of [-1,1]){
        if(limits[axis]<=0)continue
        const p=[...candidate.p];p[axis]=Math.max(-limits[axis],Math.min(limits[axis],p[axis]+sign*Math.min(15,limits[axis]/2)))
        perturbed.push({p,score:evaluate(p,level)})
      }
      candidates=distinctCandidates(perturbed,settings.candidatesToRefine)
    }
  }
  const best=candidates[0],resultMatrix=matrix(best.p),determinant=determinant3(resultMatrix)
  if(!(determinant>1e-6))throw new Error('The automatic solution was singular or reflected and was rejected. Automatic fitting never enables reflection implicitly.')
  const diagnostics=linearDiagnostics(resultMatrix,enabled.size),warnings=[...diagnostics.warnings]
  if(best.p.slice(3,6).some((value,index)=>settings.rotationRanges[index]>0&&Math.abs(value)>=settings.rotationRanges[index]-1e-6))warnings.push('A rotation reached its search boundary; consider a wider angular search.')
  if(best.score-baseline<1e-4)warnings.push('The intensity metric improved only slightly over center alignment.')
  return {matrix:resultMatrix,scoreBefore:baseline,scoreAfter:best.score,parameters:best.p,determinant,warnings,candidatesEvaluated:evaluations}
}
