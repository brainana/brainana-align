import {registrationIntensityRange as range} from './registrationIntensityRange'
import {createPointMapping,validateSpline,type NonlinearSpline} from './nonlinearSpline'
import { applyAffine, invertAffine, frame, sampleLinear, type RawNifti } from './roiWarp'
import { type Mat4, type Vec3 } from './rigid'
import { downsampleByFactor,downsampleForRegistration } from './imagePyramid'
import { optimizeAutomaticLinear, optimizeConstrainedRefinement, type AutomaticLinearSettings, type AutomaticMetric } from './automaticLinear'

export type RegistrationWindow = {min:Vec3;max:Vec3;axes?:[number,number]}
export type IntensityMask=Pick<RawNifti,'dims'|'affine'>&{values:Uint8Array}
export type LinearScoringOptions={strictSampleBudget?:boolean;sampleFraction?:number;histogramRule?:'fixed'|'cube-root'|'pyramid';rangePercentiles?:[number,number]}
export type IntensityRequest = {scoring?:LinearScoringOptions;  foreground?:'all'|'fixed-nonzero';normalization?:'percentile'|'mean'; sampling?:'grid'|'jittered';sampleSeed?:number; fixedMask?:IntensityMask; movingMask?:IntensityMask; fixed:RawNifti; moving:RawNifti; settings:AutomaticLinearSettings; metric:AutomaticMetric; bins:number; operation:'global'|'local'; start?:Mat4; nonlinear?:NonlinearSpline; fixedWindows:RegistrationWindow[]; movingWindows:RegistrationWindow[];resolutionMm?:number;maxSamples?:number }
const inside=(p:Vec3,windows:RegistrationWindow[])=>windows.every(w=>(w.axes??[0,1,2]).every(a=>p[a]>=w.min[a]&&p[a]<=w.max[a]))
const center=(image:RawNifti)=>applyAffine(image.affine,image.dims.map(v=>(v-1)/2) as Vec3)


/** Bound fixed-grid iteration by the physical ROI intersection, without cropping data or
 * changing sample phase. Window axes constrain only their two visible RAS directions. */
function samplingBounds(image:RawNifti,windows:RegistrationWindow[]){
 if(!windows.length)return {low:[0,0,0],high:image.dims.map(n=>n-1)}
 const corners:Vec3[]=[];for(const x of [0,image.dims[0]-1])for(const y of [0,image.dims[1]-1])for(const z of [0,image.dims[2]-1])corners.push(applyAffine(image.affine,[x,y,z]))
 const low=[0,1,2].map(a=>Math.min(...corners.map(p=>p[a]))),high=[0,1,2].map(a=>Math.max(...corners.map(p=>p[a])))
 for(const w of windows)for(const a of w.axes??[0,1,2]){low[a]=Math.max(low[a],w.min[a]);high[a]=Math.min(high[a],w.max[a])}
 if(low.some((v,a)=>v>high[a]))return null
 const inverse=invertAffine(image.affine),projected:Vec3[]=[];for(const x of [low[0],high[0]])for(const y of [low[1],high[1]])for(const z of [low[2],high[2]])projected.push(applyAffine(inverse,[x,y,z]))
 return {low:[0,1,2].map(a=>Math.max(0,Math.floor(Math.min(...projected.map(p=>p[a]))-1e-6))),high:[0,1,2].map(a=>Math.min(image.dims[a]-1,Math.ceil(Math.max(...projected.map(p=>p[a]))+1e-6)))}
}
export function makeIntensityScorer(fixed:RawNifti,moving:RawNifti,metric:AutomaticMetric,bins:number,fixedWindows:RegistrationWindow[]=[],movingWindows:RegistrationWindow[]=[],maxSamples=18000,start?:Mat4,nonlinear?:NonlinearSpline,fixedMask?:IntensityMask,movingMask?:IntensityMask,sampling:'grid'|'jittered'='grid',seed=1729,foreground?:RawNifti,normalization:'percentile'|'mean'='percentile',options:LinearScoringOptions={}){
  if(!Number.isInteger(maxSamples)||maxSamples<64||maxSamples>250000)throw Error('Invalid metric sample budget')
  validateLinearScoring(options);const requestedBins=bins;let eligible=0,selected=0
  if(!Number.isInteger(bins)||bins<16||bins>256)throw Error('Histogram bins must be an integer from 16 to 256')
  if(!['grid','jittered'].includes(sampling)||!Number.isInteger(seed)||seed<0||seed>0xffffffff)throw Error('Invalid sampling policy or seed')
  if(!['percentile','mean'].includes(normalization)||normalization==='mean'&&!['mi','nmi'].includes(metric))throw Error('Mean-normalized histograms require MI or NMI')
  const random=(x:number,y:number,z:number,a:number)=>{let h=(seed^Math.imul(x+1,73856093)^Math.imul(y+1,19349663)^Math.imul(z+1,83492791)^Math.imul(a+1,2654435761))>>>0;h=Math.imul(h^(h>>>16),2246822507);h=Math.imul(h^(h>>>13),3266489909);return ((h^(h>>>16))>>>0)/4294967296}
  const lookup=(mask:IntensityMask|undefined)=>{if(!mask)return ()=>false;if(mask.dims.length!==3||mask.dims.some(n=>!Number.isInteger(n)||n<1)||mask.values.length!==mask.dims.reduce((a,b)=>a*b,1))throw Error('Invalid exclusion mask');const inverse=invertAffine(mask.affine);return (world:Vec3)=>{const q=applyAffine(inverse,world).map(Math.round);return q.some((n,a)=>n<0||n>=mask.dims[a])||Boolean(mask.values[q[0]+mask.dims[0]*(q[1]+mask.dims[1]*q[2])])}}
  const originalFixedExcluded=lookup(fixedMask),movingExcluded=lookup(movingMask),foregroundInverse=foreground?invertAffine(foreground.affine):null
  const fixedExcluded=(p:Vec3)=>{if(originalFixedExcluded(p))return true;if(!foreground||!foregroundInverse)return false;const q=applyAffine(foregroundInverse,p).map(Math.round);if(q.some((v,a)=>v<0||v>=foreground.dims[a]))return true;const v=foreground.values[q[0]+foreground.dims[0]*(q[1]+foreground.dims[1]*q[2])];return !Number.isFinite(v)||v===0}
  const fv=frame(fixed,0),mv=frame(moving,0),invMoving=invertAffine(moving.affine),fr=range(fv,fixed,fixedMask||foreground?fixedExcluded:undefined,options.rangePercentiles),mr=range(mv,moving,movingMask?movingExcluded:undefined,options.rangePercentiles)
  const bounds=samplingBounds(fixed,fixedWindows)
  const startInverse=start?createPointMapping(start,nonlinear).inverse:null
  let points:Array<[Vec3,number]>=[],stride=Math.max(1,Math.ceil(Math.cbrt(fv.length/maxSamples)))
  // Global support belongs to the fixed image, never to the initial moving pose.
  const collect=()=>{points=[];if(!bounds)return;const first=bounds.low.map(v=>Math.ceil(v/stride)*stride);
  for(let z=first[2];z<=bounds.high[2];z+=stride)for(let y=first[1];y<=bounds.high[1];y+=stride)for(let x=first[0];x<=bounds.high[0];x+=stride){const q=(sampling==='grid'?[x,y,z]:[x,y,z].map((v,a)=>Math.max(0,Math.min(fixed.dims[a]-1,v+(random(x,y,z,a)-.5)*stride*.9)))) as Vec3,p=applyAffine(fixed.affine,q),v=sampling==='grid'?fv[x+fixed.dims[0]*(y+fixed.dims[1]*z)]:sampleLinear(fv,fixed.dims,q);if(Number.isFinite(v)&&!fixedExcluded(p)&&inside(p,fixedWindows)&&(!startInverse||applyAffine(invMoving,startInverse(p)).every((v,a)=>v>=0&&v<=moving.dims[a]-1)))points.push([p,v])}
  };
  if(options.sampleFraction!==undefined){
    // Visit included original lattice points once. Fraction selection is evenly
    // distributed; a deterministic reservoir enforces the independent memory cap.
    stride=1;points=[];const fraction=options.sampleFraction,limit=maxSamples
    if(bounds)for(let z=Math.max(0,Math.ceil(bounds.low[2]));z<=bounds.high[2];z++)for(let y=Math.max(0,Math.ceil(bounds.low[1]));y<=bounds.high[1];y++)for(let x=Math.max(0,Math.ceil(bounds.low[0]));x<=bounds.high[0];x++){
      const q=(sampling==='grid'?[x,y,z]:[x,y,z].map((v,a)=>Math.max(0,Math.min(fixed.dims[a]-1,v+(random(x,y,z,a)-.5)*.9)))) as Vec3,p=applyAffine(fixed.affine,q),v=sampling==='grid'?fv[x+fixed.dims[0]*(y+fixed.dims[1]*z)]:sampleLinear(fv,fixed.dims,q)
      if(!Number.isFinite(v)||fixedExcluded(p)||!inside(p,fixedWindows)||startInverse&&!applyAffine(invMoving,startInverse(p)).every((v,a)=>v>=0&&v<=moving.dims[a]-1))continue
      eligible++;if(Math.floor(eligible*fraction)===Math.floor((eligible-1)*fraction))continue
      selected++;if(points.length<limit)points.push([p,v]);else{const slot=Math.floor(random(x,y,z,7)*selected);if(slot<limit)points[slot]=[p,v]}
    }
  }else collect();while(points.length<1000&&stride>1){stride=Math.max(1,Math.floor(stride/2));collect()}
  if(options.strictSampleBudget&&options.sampleFraction===undefined){selected=points.length;eligible=selected;if(points.length>maxSamples){for(let i=maxSamples;i<points.length;i++){const slot=Math.floor(random(i,0,0,9)*(i+1));if(slot<maxSamples)points[slot]=points[i]}points.length=maxSamples}}
  if(points.length<64)throw new Error('Too few fixed-image samples. Enlarge optimization windows or choose finer working/registration resolution.');
  if(options.histogramRule==='cube-root')bins=Math.max(16,Math.min(256,Math.round(Math.cbrt(points.length))))
  const smooth=metric==='parzen-mi',joint=smooth?new Float64Array(bins*bins):new Uint32Array(bins*bins),fh=new Uint32Array(bins),mh=smooth?new Float64Array(bins):new Uint32Array(bins)
  const meanMode=normalization==='mean',pairedFixed=meanMode?new Float64Array(points.length):null,pairedMoving=meanMode?new Float64Array(points.length):null
  let lastN=0
  const score=(matrix:Mat4)=>{
    const inverse=createPointMapping(matrix,nonlinear).inverse;joint.fill(0);fh.fill(0);mh.fill(0)
    let n=0,sf=0,sm=0,sff=0,smm=0,sfm=0,squared=0
    for(const [p,f] of points){const world=inverse(p);if(!inside(world,movingWindows)||movingExcluded(world))continue;const voxel=applyAffine(invMoving,world);if(voxel.some((v,a)=>v<0||v>moving.dims[a]-1))continue;const m=sampleLinear(mv,moving.dims,voxel);if(!Number.isFinite(m)||(!options.rangePercentiles&&!meanMode&&f<=fr[0]&&m<=mr[0]))continue;if(meanMode){pairedFixed![n]=f;pairedMoving![n]=m;n++;sf+=f;sm+=m;continue}const fn=(f-fr[0])/(fr[1]-fr[0]),mn=(m-mr[0])/(mr[1]-mr[0]),fb=Math.max(0,Math.min(bins-1,Math.floor(fn*bins))),mb=Math.max(0,Math.min(bins-1,Math.floor(mn*bins)));if(smooth){const position=Math.max(0,Math.min(bins-1,mn*(bins-1))),base=Math.floor(position),t=position-base,t2=t*t,t3=t2*t;for(let k=0;k<4;k++){const j=Math.max(0,Math.min(bins-1,base+k-1)),w=k===0?(1-3*t+3*t2-t3)/6:k===1?(4-6*t2+3*t3)/6:k===2?(1+3*t+3*t2-3*t3)/6:t3/6;joint[fb*bins+j]+=w;mh[j]+=w}}else{joint[fb*bins+mb]++;mh[mb]++}fh[fb]++;n++;sf+=f;sm+=m;sff+=f*f;smm+=m*m;sfm+=f*m;squared+=(fn-mn)**2}
    lastN=n
    if(meanMode&&n){
      const af=sf/n,am=sm/n
      if(!Number.isFinite(af)||!Number.isFinite(am)||Math.abs(af)<1e-12||Math.abs(am)<1e-12)return -Infinity
      let low=Infinity,high=-Infinity
      for(let i=0;i<n;i++){const f=pairedFixed![i]/af,m=pairedMoving![i]/am;low=Math.min(low,f,m);high=Math.max(high,f,m)}
      if(!(high>low))return -Infinity
      for(let i=0;i<n;i++){const fb=Math.max(0,Math.min(bins-1,Math.floor(bins*(pairedFixed![i]/af-low)/(high-low)))),mb=Math.max(0,Math.min(bins-1,Math.floor(bins*(pairedMoving![i]/am-low)/(high-low))));joint[fb*bins+mb]++;fh[fb]++;mh[mb]++}
    }
    lastN=n
    if(n<(start?Math.min(500,Math.max(64,Math.floor(points.length*.25))):Math.max(64,Math.floor(points.length*.1))))return -Infinity
    if(metric==='least-squares')return -squared/n
    if(metric==='normalized-correlation'){const denominator=Math.sqrt(Math.max(0,(n*sff-sf*sf)*(n*smm-sm*sm)));return denominator>0?(n*sfm-sf*sm)/denominator:-Infinity}
    if(metric==='hellinger'){let affinity=0;for(let f=0;f<bins;f++)for(let m=0;m<bins;m++){const c=joint[f*bins+m];if(c)affinity+=Math.sqrt(c*fh[f]*mh[m])/(n*Math.sqrt(n))}return Math.max(0,2*(1-affinity))}
    const entropy=(hist:Uint32Array|Float64Array)=>{let h=0;for(const count of hist)if(count){const p=count/n;h-=p*Math.log(p)}return h},hf=entropy(fh),hm=entropy(mh),hj=entropy(joint)
    if(metric==='mi'||metric==='parzen-mi')return hf+hm-hj
    if(metric==='correlation-ratio'){let overall=0;for(let mb=0;mb<bins;mb++)overall+=mh[mb]*(mb+.5)/bins;overall/=n;let between=0,total=0;for(let fb=0;fb<bins;fb++){let sum=0,count=0;for(let mb=0;mb<bins;mb++){const c=joint[fb*bins+mb];sum+=c*(mb+.5)/bins;count+=c}if(count)between+=count*(sum/count-overall)**2}for(let mb=0;mb<bins;mb++)total+=mh[mb]*((mb+.5)/bins-overall)**2;return total>0?between/total:-Infinity}
    return hj>0?(hf+hm)/hj:-Infinity
  }
  return Object.assign(score,{diagnostics(matrix:Mat4){score(matrix);let occupied=0;for(const value of joint)if(value>0)occupied++;return {fixedSamples:points.length,includedSamples:lastN,overlapFraction:lastN/points.length,occupiedJointBins:occupied,bins,sampling,seed,...(options.sampleFraction!==undefined||options.strictSampleBudget?{requestedSampleFraction:options.sampleFraction,eligibleSamples:eligible,sampleBudgetClipped:selected>maxSamples}:{}),...(options.histogramRule?{histogramRule:options.histogramRule,requestedBins}:{}),foreground:foreground?'fixed-nonzero':'all',normalization}}})
}
export async function runIntensityRegistration(request:IntensityRequest,progress:(text:string)=>void,cancelled:()=>boolean=()=>false){
  validateLinearScoring(request.scoring);
  if(!['hellinger','parzen-mi','nmi','mi','correlation-ratio','normalized-correlation','least-squares'].includes(request.metric)||!Number.isInteger(request.bins)||request.bins<16||request.bins>256)throw new Error('Invalid image metric or histogram bins.')
  if(request.foreground!==undefined&&!['all','fixed-nonzero'].includes(request.foreground))throw Error('Unknown foreground matching policy')
  if(request.normalization!==undefined&&(!['percentile','mean'].includes(request.normalization)||request.normalization==='mean'&&!['mi','nmi'].includes(request.metric)))throw Error('Mean-normalized histograms require MI or NMI')
  if(request.settings.positionInitialization!==undefined&&!['physical','geometry','moments'].includes(request.settings.positionInitialization))throw Error('Unknown starting position')
  if(request.settings.smoothingSigmasVox!==undefined&&(!Array.isArray(request.settings.smoothingSigmasVox)||request.settings.smoothingSigmasVox.length!==request.settings.shrinkFactors.length||request.settings.smoothingSigmasVox.some(v=>!Number.isFinite(v)||v<0||v>16)))throw Error('Specify one finite smoothing sigma (0–16 voxels) per pyramid level')
  const foreground=request.foreground==='fixed-nonzero'?request.fixed:undefined
  if(request.resolutionMm!==undefined&&(!Number.isFinite(request.resolutionMm)||request.resolutionMm<0))throw Error('Invalid registration spacing');if(request.maxSamples!==undefined&&(!Number.isInteger(request.maxSamples)||request.maxSamples<1000||request.maxSamples>250000))throw Error('Invalid sampling count');
  if(request.resolutionMm){request={...request,fixed:downsampleForRegistration(request.fixed,request.resolutionMm),moving:downsampleForRegistration(request.moving,request.resolutionMm)}}
  const sampling=request.sampling??'grid',seed=request.sampleSeed??1729
  const annotate=(result:Awaited<ReturnType<typeof optimizeAutomaticLinear>>,scorer:ReturnType<typeof makeIntensityScorer>)=>{const diagnostics=scorer.diagnostics(result.matrix);return {...result,samplingDiagnostics:diagnostics,warnings:[...result.warnings,...(diagnostics.overlapFraction<.25?['Less than25% of fixed sample support contributes to the final score. Inspect partial overlap.']:[]),...(diagnostics.sampleBudgetClipped?['The requested sample fraction exceeded the sample budget; a deterministic bounded subset was used.']:[]),...(request.metric==='parzen-mi'&&diagnostics.includedSamples<request.bins*request.bins/2?['The joint histogram has relatively few samples for the selected bin count. Consider more samples or fewer bins.']:[])]}}
  const s=request.settings;if(![s.translationLimitMm,s.scaleLimit,s.shearLimit,...s.rotationRanges].every(v=>Number.isFinite(v)&&v>=0)||s.translationLimitMm>500||s.scaleLimit>1||s.shearLimit>1||s.rotationRanges.some(v=>v>180)||!s.shrinkFactors.length||s.shrinkFactors.at(-1)!==1)throw new Error('Invalid registration search limits or pyramid.')
  if(request.nonlinear){validateSpline(request.nonlinear);if(request.operation!=='local'||!s.refinement)throw Error('A nonlinear base requires constrained local refinement')}
  if(request.operation==='local'&&!request.start)throw new Error('Refinement needs an accepted starting transform.')
  if(request.operation==='local'&&s.refinement){
    if(![s.refinement.translationStepMm,s.refinement.rotationStepDegrees].every(v=>Number.isFinite(v)&&v>0)||s.translationLimitMm<=0||s.rotationRanges.some(v=>v<=0))throw new Error('Refinement limits and steps must be positive numbers.')
    progress('Preparing constrained overlap samples…')
    const score=makeIntensityScorer(request.fixed,request.moving,request.metric,request.bins,request.fixedWindows,request.movingWindows,request.maxSamples??30000,request.start,request.nonlinear,request.fixedMask,request.movingMask,sampling,seed,foreground,request.normalization??'percentile')
    return annotate(await optimizeConstrainedRefinement({settings:s,center:center(request.fixed),start:request.start!,score,progress,cancelled}),score)
  }
  // Keep only the active level; baseline/final comparisons still use the finest level.
  let active=-1,scorer:ReturnType<typeof makeIntensityScorer>|null=null
  const score=(matrix:Mat4,level:number)=>{
    if(active!==level){
      let factor=request.settings.searchMode==='nearby'?1:request.settings.shrinkFactors[level]
      progress(`Preparing ${factor}× working image resolution…`);scorer=null
      for(;;){
        try{scorer=makeIntensityScorer(downsampleByFactor(request.fixed,factor,request.settings.smoothingSigmasVox?.[level]??(factor===1?0:factor*.5)),downsampleByFactor(request.moving,factor,request.settings.smoothingSigmasVox?.[level]??(factor===1?0:factor*.5)),request.metric,request.scoring?.histogramRule==='pyramid'?Math.max(16,Math.round(request.bins/factor)):request.bins,request.fixedWindows,request.movingWindows,request.maxSamples??(request.settings.searchMode==='nearby'?60000:18000),undefined,undefined,request.fixedMask,request.movingMask,sampling,seed,foreground,request.normalization??'percentile',request.scoring);break}
        catch(error){if((!foreground&&!request.scoring)||factor<=1||!/Too few fixed-image samples|no included finite intensities|insufficient intensity variation/i.test(error instanceof Error?error.message:String(error)))throw error;factor=Math.max(1,Math.floor(factor/2));progress(`Sample support is sparse at this pyramid level; using ${factor}× samples with the same regions and exclusions…`)}
      }
      active=level
    }
    return scorer!(matrix)
  }
  const moments=request.settings.positionInitialization==='moments',fc=moments?intensityCenter(request.fixed,request.fixedWindows,request.fixedMask):center(request.fixed),mc=moments?intensityCenter(request.moving,request.movingWindows,request.movingMask):center(request.moving)
  const nearby=request.settings.searchMode==='nearby'
  const identity:Mat4=[[1,0,0,0],[0,1,0,0],[0,0,1,0],[0,0,0,1]]
  const physical=request.settings.positionInitialization==='physical'||request.settings.positionInitialization===undefined&&nearby
  const result=await optimizeAutomaticLinear({settings:request.settings,movingCenter:request.operation==='local'||physical?fc:mc,fixedCenter:fc,globalSearch:request.operation==='global',startMatrix:request.operation==='local'?request.start:physical?identity:undefined,score,progress,cancelled})
  if(!Number.isFinite(result.scoreAfter))throw new Error('No candidate has sufficient valid image overlap.');
  score(result.matrix,request.settings.shrinkFactors.length-1)
  return annotate(result,scorer!)
}

/** Optional controls are validated independently for worker and session boundaries. */
export function validateLinearScoring(options:LinearScoringOptions|undefined){
 if(options===undefined)return
 if(!options||typeof options!=='object'||Array.isArray(options))throw Error('Invalid linear scoring settings')
 if(options.strictSampleBudget!==undefined&&typeof options.strictSampleBudget!=='boolean')throw Error('Invalid sample budget policy')
 if(options.sampleFraction!==undefined&&(!Number.isFinite(options.sampleFraction)||options.sampleFraction<=0||options.sampleFraction>1))throw Error('Sample fraction must be greater than zero and at most one')
 if(options.histogramRule!==undefined&&!['fixed','cube-root','pyramid'].includes(options.histogramRule))throw Error('Unknown histogram rule')
 const p=options.rangePercentiles;if(p!==undefined&&(!Array.isArray(p)||p.length!==2||!p.every(Number.isFinite)||p[0]<0||p[1]>1||p[0]>=p[1]))throw Error('Invalid registration intensity percentiles')
}
/** Positive-intensity center of mass in physical RAS, restricted by current regions/masks. */
export function intensityCenter(image:RawNifti,windows:RegistrationWindow[]=[],mask?:IntensityMask):Vec3{
 let total=0;const sum=[0,0,0],inverse=mask?invertAffine(mask.affine):null
 for(let i=0;i<image.dims.reduce((a,b)=>a*b,1);i++){const v=image.values[i];if(!Number.isFinite(v)||v<=0)continue;const p=applyAffine(image.affine,[i%image.dims[0],Math.floor(i/image.dims[0])%image.dims[1],Math.floor(i/(image.dims[0]*image.dims[1]))]);if(!inside(p,windows))continue
 if(mask&&inverse){const q=applyAffine(inverse,p).map(Math.round);if(q.some((v,a)=>v<0||v>=mask.dims[a])||mask.values[q[0]+mask.dims[0]*(q[1]+mask.dims[1]*q[2])])continue}total+=v;for(let a=0;a<3;a++)sum[a]+=v*p[a]
 }
 if(!(total>0)||!Number.isFinite(total))throw Error('Intensity-center initialization needs positive included intensities. Choose image centers or existing coordinates.')
 return sum.map(v=>v/total) as Vec3
}
