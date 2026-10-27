import {validateLinearDefaults} from './linearDefaults'
import {validateLinearScoring} from '../../../packages/imaging-math/intensityRegistration'
import {validateAcquisition,type AcquisitionMetadata} from './acquisitionMetadata'
import {cloneFitState} from './fitSnapshot'
import {validPairing,getRegistrationPairing,type RegistrationPairing} from './registrationPairing'
import {multiplyMat4} from './rigid'
import {manualComposition,validateManualAffine} from './manualAffine'
import {validateProcessingRecipe,geometryMatches,type ProcessingRecipe} from './imageProcessing'
import {validateSpline,createPointMapping} from '../../../packages/imaging-math/nonlinearSpline'
import { diagnoseTransform, isLandmarkModel, LANDMARK_MODELS, validateAffine } from './landmarkFit'
import { invertAffine } from './roiWarp'
import type { RegistrationModel, FitState, Landmark, Loaded, Modality, OptimizationWindows } from './appTypes'

export const SESSION_SCHEMA_VERSION = 13

export type ImageSignature = {
  acquisition?:AcquisitionMetadata
  name: string
  sourceFiles: string[]
  dims: [number, number, number]
  pixDims: [number, number, number]
  affine: number[][]
  workingDims?:[number,number,number]
  workingSpacing?:[number,number,number]
  selectedFrame?:number
}

export type SessionPayload = {
  application: 'Brainana Align'
  registrationPairing:RegistrationPairing
  coordinateSystem?: 'NIfTI RAS physical millimeters'
  movingSpace?: Modality
  fixedSpace?: Modality
  schemaVersion: number
  appVersion: string
  savedAt: string
  images: Record<Modality, ImageSignature | null>
  landmarks: Landmark[]
  selectedId: number | null
  nextId: number
  direction: string
  landmarkModel?: RegistrationModel
  splineSmoothing?: number
  fit: FitState | null
  optimizationWindows: OptimizationWindows
  processing?: Partial<Record<Modality,ProcessingRecipe>>
}

type LegacySessionPayload = Partial<SessionPayload> & {
  application?: string
  version?: string
  saved_at?: string
  images?: Partial<Record<Modality, ImageSignature | null>>
  landmarks?: Landmark[]
  selectedId?: number | null
  nextId?: number
  direction?: string
  fit?: FitState | null
  optimizationWindows?: OptimizationWindows
}

export function imageSignature(item: Loaded | null): ImageSignature | null {
  if (!item) return null
  const source=item.processing?.base??item.raw
  return {
    acquisition:item.acquisition?validateAcquisition(item.acquisition):undefined,
    name: item.name,
    sourceFiles: [...item.sourceFiles],
    workingDims:[...item.raw.dims] as [number,number,number],
    workingSpacing:[...item.raw.pixDims] as [number,number,number],
    selectedFrame:item.native?.frame??0,
    dims: [...(item.native?.info.dims??source.dims)] as [number, number, number],
    pixDims: [...(item.native?.info.spacing??source.pixDims)] as [number, number, number],
    affine: (item.native?.info.affine??source.affine).map(row => [...row]),
  }
}

export function geometryDifference(a: ImageSignature, b: ImageSignature): string[] {
  const issues: string[] = []
  if((a.selectedFrame??0)!==(b.selectedFrame??0))issues.push('selected 3D frame differs')
  if (a.dims.some((value, index) => value !== b.dims[index])) {
    issues.push(`dimensions ${a.dims.join('×')} vs ${b.dims.join('×')}`)
  }
  if (a.pixDims.some((value, index) => Math.abs(value - b.pixDims[index]) > 1e-3)) {
    issues.push('voxel sizes differ')
  }
  let maxAffine = 0
  for (let row = 0; row < 4; row++) {
    for (let column = 0; column < 4; column++) {
      maxAffine = Math.max(maxAffine, Math.abs(a.affine[row][column] - b.affine[row][column]))
    }
  }
  if (maxAffine > 1e-3) issues.push(`affines differ (max |Δ| ${maxAffine.toFixed(4)})`)
  return issues
}

export function createSessionPayload(options: {
  appVersion: string
  registrationPairing?:RegistrationPairing
  loaded: Record<Modality, Loaded | null>
  landmarks: Landmark[]
  selectedId: number | null
  nextId: number
  direction: string
  landmarkModel?: RegistrationModel
  splineSmoothing?: number
  fit: FitState | null
  optimizationWindows: OptimizationWindows
  now?: Date
}): SessionPayload {
  return {
    application: 'Brainana Align',
    registrationPairing:options.registrationPairing??getRegistrationPairing(),
    coordinateSystem: 'NIfTI RAS physical millimeters',
    movingSpace: options.direction === 'ct-mri' ? 'ct' : 'mri',
    fixedSpace: options.direction === 'ct-mri' ? 'mri' : 'ct',
    schemaVersion: SESSION_SCHEMA_VERSION,
    appVersion: options.appVersion,
    savedAt: (options.now ?? new Date()).toISOString(),
    images: {
      mri: imageSignature(options.loaded.mri),
      ct: imageSignature(options.loaded.ct),
    },
    landmarks: structuredClone(options.landmarks),
    selectedId: options.selectedId,
    nextId: options.nextId,
    direction: options.direction,
    splineSmoothing:options.splineSmoothing,
    landmarkModel: options.landmarkModel ?? (options.fit?.model==='jip'?'rigid':options.fit?.model) ?? 'rigid',
    fit: options.fit ? cloneFitState(options.fit,true) : null,
    optimizationWindows: structuredClone(options.optimizationWindows),
    processing:Object.fromEntries(Object.entries(options.loaded).flatMap(([m,item])=>item?.processing?[[m,structuredClone(item.processing.recipe)]]:[])),
  }
}

export function parseSessionPayload(value: unknown): SessionPayload {
  if (!value || typeof value !== 'object') throw new Error('This is not a valid Brainana Align session file.')
  const payload = value as LegacySessionPayload
  if (payload.application !== 'Brainana Align' || !Array.isArray(payload.landmarks)) {
    throw new Error('This is not a valid Brainana Align session file.')
  }
  if(payload.registrationPairing!==undefined&&!validPairing(payload.registrationPairing))throw Error('Unknown registration pairing in session')
  const schemaVersion = Number(payload.schemaVersion ?? 0)
  if (schemaVersion > SESSION_SCHEMA_VERSION) {
    throw new Error(`This session uses newer schema version ${schemaVersion}. Update Brainana Align before loading it.`)
  }
  if (!Number.isInteger(schemaVersion) || schemaVersion < 0) throw new Error('Invalid session schema version')
  if (payload.direction !== undefined && !['ct-mri', 'mri-ct'].includes(payload.direction)) throw new Error('Invalid transform direction')
  if (payload.coordinateSystem !== undefined && payload.coordinateSystem !== 'NIfTI RAS physical millimeters') throw new Error('Unsupported session coordinate system')
  validateLandmarks(payload.landmarks)
  if (payload.nextId !== undefined && (!Number.isInteger(payload.nextId) || payload.nextId < 1)) throw new Error('Invalid next landmark identifier')
  if (payload.selectedId !== undefined && payload.selectedId !== null && !Number.isInteger(payload.selectedId)) throw new Error('Invalid selected landmark identifier')
  if (payload.landmarkModel !== undefined && !(isLandmarkModel(payload.landmarkModel)||payload.landmarkModel==='spline')) throw new Error('Unknown landmark model in session')
  if(payload.splineSmoothing!==undefined&&(!Number.isFinite(payload.splineSmoothing)||payload.splineSmoothing<0||payload.splineSmoothing>10))throw Error('Invalid spline smoothing')
  const fit = payload.fit ? normalizeFit(payload.fit, payload.landmarks) : null
  for (const signature of [payload.images?.mri, payload.images?.ct]) {
    if (!signature) continue
    if(signature.acquisition!==undefined)signature.acquisition=validateAcquisition(signature.acquisition)
    validateAffine(signature.affine)
    if (!Array.isArray(signature.dims) || signature.dims.length !== 3 || !signature.dims.every(v => Number.isInteger(v) && v > 0) || !Array.isArray(signature.pixDims) || signature.pixDims.length !== 3 || !signature.pixDims.every(v => Number.isFinite(v) && v > 0)) throw new Error('Invalid image geometry in session')
  }
  const processing:Partial<Record<Modality,ProcessingRecipe>>={}
  for(const m of ['mri','ct'] as Modality[])if(payload.processing?.[m])processing[m]=validateProcessingRecipe(payload.processing[m])
  const savedAt = payload.savedAt ?? payload.saved_at ?? new Date(0).toISOString()
  return {
    application: 'Brainana Align',
    registrationPairing:payload.registrationPairing??'mri-ct',
    coordinateSystem: 'NIfTI RAS physical millimeters',
    movingSpace: (payload.direction ?? fit?.direction ?? 'ct-mri') === 'ct-mri' ? 'ct' : 'mri',
    fixedSpace: (payload.direction ?? fit?.direction ?? 'ct-mri') === 'ct-mri' ? 'mri' : 'ct',
    schemaVersion: SESSION_SCHEMA_VERSION,
    appVersion: payload.appVersion ?? payload.version ?? 'unknown',
    savedAt,
    images: {
      mri: payload.images?.mri ?? null,
      ct: payload.images?.ct ?? null,
    },
    landmarks: structuredClone(payload.landmarks),
    selectedId: payload.selectedId ?? null,
    nextId: payload.nextId ?? 1,
    direction: payload.direction ?? fit?.direction ?? 'ct-mri',
    splineSmoothing:payload.splineSmoothing,
    landmarkModel: payload.landmarkModel ?? (fit?.model==='jip'?'rigid':fit?.model) ?? 'rigid',
    fit,
    processing,
    optimizationWindows: payload.optimizationWindows
      ? structuredClone(payload.optimizationWindows)
      : { mri: {}, ct: {} },
  }
}

export function sessionGeometryMismatches(
  saved: SessionPayload['images'],
  current: Record<Modality, Loaded | null>,
  processing:Partial<Record<Modality,ProcessingRecipe>>={},
): string[] {
  const mismatch: string[] = []
  for (const modality of ['mri', 'ct'] as Modality[]) {
    const savedSignature = saved[modality]
    const currentSignature = imageSignature(current[modality])
    const item=current[modality],r=processing[modality];if(item&&r&&(!geometryMatches(r.base,item.processing?.base??item.raw)||savedSignature?.name!==item.name))mismatch.push(`${modality.toUpperCase()}: processing requires the same dataset and original working grid (${r.base.dims.join(' × ')} voxels at ${r.base.pixDims.join(' × ')} mm). Prepare this resolution first.`)
    if (!savedSignature || !currentSignature) {
      mismatch.push(`${modality.toUpperCase()}: saved or loaded image geometry is missing`)
    } else {
      mismatch.push(...geometryDifference(savedSignature, currentSignature).map(issue => `${modality.toUpperCase()}: ${issue}`))
    }
  }
  return mismatch
}

function validateLandmarks(landmarks: Landmark[]) {
  const ids = new Set<number>()
  for (const lm of landmarks) {
    if (!lm || !Number.isInteger(lm.id) || lm.id < 1 || ids.has(lm.id) || typeof lm.enabled !== 'boolean') throw new Error('Invalid or duplicate landmark in session')
    ids.add(lm.id)
    for (const p of [lm.mri, lm.ct]) if (p !== null && (!Array.isArray(p) || p.length !== 3 || !p.every(v => typeof v === 'number' && Number.isFinite(v)))) throw new Error('Invalid physical landmark coordinates in session')
  }
}
function normalizeFit(input: FitState, landmarks: Landmark[]): FitState {
  const fit = cloneFitState(input)
  if(fit.jipRegistration&&(!["both","affine","nonlinear"].includes(fit.jipRegistration.stages)||!fit.jipRegistration.settings||!fit.jipRegistration.execution||!fit.jipRegistration.diagnostics))throw Error("Invalid JIP registration provenance")
  if (!['ct-mri', 'mri-ct'].includes(fit.direction)) throw new Error('Invalid fitted transform direction')
  for (const matrix of [fit.matrix, fit.landmarkMatrix, fit.baseMatrix]) validateAffine(matrix)
  fit.inverse = invertAffine(fit.matrix)
  if (!Array.isArray(fit.manual) || fit.manual.length !== 6 || !fit.manual.every(Number.isFinite)) throw new Error('Invalid manual transform in session')
  if(fit.manualAffine){validateManualAffine(fit.manualAffine);if(fit.manualAffine.direction!==fit.direction)throw Error('Manual affine direction mismatch');const composition=manualComposition(fit.manualAffine),composed=composition.matrix;if(fit.nonlinear){const expected=fit.manualAffine.priorSplineInput?multiplyMat4(fit.manualAffine.priorSplineInput,composition.pre):composition.pre,actual=fit.nonlinear.inputAffine;if(!actual||expected.some((r,i)=>r.some((v,j)=>Math.abs(v-actual[i][j])>1e-6)))throw Error('Nonlinear input composition does not match manual state');}if(composed.some((r,i)=>r.some((v,j)=>Math.abs(v-fit.matrix[i][j])>1e-6*Math.max(1,Math.abs(v)))))throw Error('Manual affine state does not match saved transformation')}
  if (!Number.isFinite(fit.rms) || fit.rms < 0) throw new Error('Invalid RMS error in session')
  if (fit.initializationSource !== undefined && !['landmarks','image-geometry','automatic'].includes(fit.initializationSource)) throw new Error('Invalid alignment initialization source')
  if(fit.landmarkModel!==undefined&&!(isLandmarkModel(fit.landmarkModel)||fit.landmarkModel==='spline'||fit.landmarkModel==='jip'&&fit.nonlinear?.kernel==='jip-dense-pull'))throw Error('Invalid original alignment model')
  if(fit.landmarkModel==='spline'&&!fit.nonlinear)throw Error('Original alignment model and spline coefficients mismatch')
  fit.model = fit.model ?? 'rigid'
  if (!(isLandmarkModel(fit.model)||['spline','jip'].includes(fit.model??''))) throw new Error('Unknown fitted transform model')
  if(['spline','jip'].includes(fit.model??'')){if(!fit.nonlinear)throw Error('Nonlinear session is missing spline coefficients');validateSpline(fit.nonlinear);if((fit.model==='jip')!==(fit.nonlinear.kernel==='jip-dense-pull'))throw Error('Nonlinear model and field kind mismatch');fit.parameterCount=fit.nonlinear.kernel==='jip-dense-pull'?3*fit.nonlinear.dims.reduce((a,b)=>a*b,1):fit.nonlinear.controls.length*3}else{if(fit.nonlinear)throw Error('Linear session unexpectedly contains nonlinear coefficients');fit.parameterCount=LANDMARK_MODELS[fit.model as keyof typeof LANDMARK_MODELS].parameters}
  const diagnostics = diagnoseTransform(fit.matrix)
  if (!['affine','spline','jip'].includes(fit.model) && diagnostics.shear > 1e-4) throw new Error('Saved transform has shear inconsistent with its landmark model')
  if (fit.model === 'rigid' && diagnostics.scales.some(v => Math.abs(v - 1) > 1e-4)) throw new Error('Saved rigid transform contains scaling')
  if (fit.model === 'similarity' && Math.max(...diagnostics.scales) - Math.min(...diagnostics.scales) > 1e-4) throw new Error('Saved similarity transform has nonuniform scaling')
  if (!['affine','spline','jip'].includes(fit.model) && diagnostics.reflection) throw new Error('A proper landmark model cannot contain a reflection')
  if(['spline','jip'].includes(fit.model??'')&&diagnostics.reflection)throw Error('Nonlinear session cannot mirror anatomy')
  const oldWarnings = fit.diagnostics?.warnings?.filter(w => typeof w === 'string') ?? []
  fit.diagnostics = { ...diagnostics, warnings: [...new Set([...oldWarnings, ...diagnostics.warnings])] }
  fit.landmarkSnapshot = fit.landmarkSnapshot ?? structuredClone(landmarks)
  validateLandmarks(fit.landmarkSnapshot)
  if(fit.nonlinear&&fit.nonlinear.kernel!=='jip-dense-pull'){const s=fit.nonlinear,pairs=fit.landmarkSnapshot.filter(l=>l.enabled&&l.mri&&l.ct);if(pairs.length!==s.controls.length)throw Error('Spline controls do not match saved landmark snapshot');const map=createPointMapping(fit.matrix,s);let sum=0;for(let i=0;i<pairs.length;i++){const source=fit.direction==='ct-mri'?pairs[i].ct!:pairs[i].mri!,target=fit.direction==='ct-mri'?pairs[i].mri!:pairs[i].ct!,q=createPointMapping(s.baseAffine).forward(source);if(q.some((v,a)=>Math.abs((v-s.center[a])/s.scale-s.controls[i][a])>1e-6))throw Error('Spline control geometry does not match its saved physical landmarks');const mapped=map.forward(source),residual=Math.hypot(...mapped.map((v,a)=>v-target[a]));sum+=residual*residual}fit.rms=Math.sqrt(sum/pairs.length)}
  fit.landmarksChanged = Boolean(fit.landmarksChanged)
  fit.reflectionConfirmed = Boolean(fit.reflectionConfirmed && diagnostics.reflection)
  if (fit.proposal) {
    if(fit.proposal.kind!==undefined && !['automatic','spline','jip'].includes(fit.proposal.kind))throw new Error('Invalid proposal kind')
    if(fit.proposal.previous){if(fit.proposal.previous.proposal)throw new Error('Nested pending proposals are not supported');fit.proposal.previous=normalizeFit(fit.proposal.previous,landmarks)}
    validateAffine(fit.proposal.matrix)
    if (!Number.isFinite(fit.proposal.scoreBefore) || !Number.isFinite(fit.proposal.scoreAfter) || !Array.isArray(fit.proposal.params) || fit.proposal.params.length !== (fit.proposal.kind==='automatic'?12:fit.proposal.kind==='spline'||fit.proposal.kind==='jip'?0:6) || !fit.proposal.params.every(Number.isFinite)) throw new Error('Invalid refinement proposal in session')
  }
  if(fit.automaticRegistration){const a=fit.automaticRegistration;if(a.defaults)validateLinearDefaults(a.defaults);validateLinearScoring(a.scoring);if(a.settings?.positionInitialization!==undefined&&!['physical','geometry','moments'].includes(a.settings.positionInitialization)||a.settings?.smoothingSigmasVox!==undefined&&(!Array.isArray(a.settings.smoothingSigmasVox)||a.settings.smoothingSigmasVox.length!==a.settings.shrinkFactors.length||a.settings.smoothingSigmasVox.some(v=>!Number.isFinite(v)||v<0||v>16)))throw Error('Invalid linear starting position or smoothing');if(a.foreground!==undefined&&!['all','fixed-nonzero'].includes(a.foreground)||a.normalization!==undefined&&!['percentile','mean'].includes(a.normalization)||a.settings?.searchMode!==undefined&&!['broad','nearby'].includes(a.settings.searchMode))throw Error('Invalid linear matching settings');if(!['global','local'].includes(a.operation)||!['rigid','similarity','anisotropic','affine','custom'].includes(a.model)||!Number.isFinite(a.scoreAfter)||!(a.scoreBefore===null||Number.isFinite(a.scoreBefore))||!Number.isInteger(a.parameterCount)||a.parameterCount<6||a.parameterCount>12||!Number.isInteger(a.candidatesEvaluated)||a.candidatesEvaluated<1)throw new Error('Invalid automatic registration metadata')}
  return fit
}
