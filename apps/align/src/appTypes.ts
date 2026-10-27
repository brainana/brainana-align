import type { LandmarkModel, TransformDiagnostics } from './landmarkFit'
import type { Niivue, NVImage } from '@niivue/niivue'
import type { RawNifti } from './roiWarp'
import type { Mat4, Vec3 } from './rigid'

export type RegistrationModel = LandmarkModel | 'spline' | 'jip'
export type Modality = 'mri' | 'ct'
export type Plane = 'sagittal' | 'coronal' | 'axial'
export type Landmark = { id: number; mri: Vec3 | null; ct: Vec3 | null; enabled: boolean; residual?: number }
export type Loaded = { acquisition?:import('./acquisitionMetadata').AcquisitionMetadata;processing?:{binarySelection?:Uint8Array;focus?:[number,number,number];base:RawNifti;recipe:import('./imageProcessing').ProcessingRecipe;excluded:Uint8Array}; name: string; raw: RawNifti; nvImage: NVImage; sourceFiles: string[]; native?:{info:import('../../../packages/imaging-math/workingVolume').VolumeInfo;files:File[];frame:number;requested:[number,number,number];streamable:boolean} }
export type WindowBounds = { min: Vec3; max: Vec3 }
export type OptimizationWindows = Record<Modality, Partial<Record<Plane, WindowBounds>>>
export type View = { nv: Niivue; modality: Modality; plane: Plane; canvas: HTMLCanvasElement; overlay: SVGSVGElement; windowLayer: HTMLDivElement }
export type ReviewView = { nv: Niivue; plane: Plane; canvas: HTMLCanvasElement; overlay: SVGSVGElement }
export type Params6 = [number, number, number, number, number, number]
export type FitState = {
  jipRegistration?:{settings:Record<string,unknown>;execution:Record<string,unknown>;diagnostics:Record<string,unknown>;stages:string}

  automaticRegistration?: {defaults?:import('./linearDefaults').LinearDefaultsProvenance;scoring?:import('../../../packages/imaging-math/intensityRegistration').LinearScoringOptions;foreground?:'all'|'fixed-nonzero';normalization?:'percentile'|'mean';sampling?:'grid'|'jittered';sampleSeed?:number;samplingDiagnostics?:NonNullable<import('../../../packages/imaging-math/automaticLinear').AutomaticLinearResult['samplingDiagnostics']>;workingResolution?:{fixed:number[];moving:number[];finestSpacingMm:number;maxSamples:number};operation:'global'|'local';model:import('../../../packages/imaging-math/automaticLinear').TransformModel;parameterCount:number;metric:string;bins:number;settings:import('../../../packages/imaging-math/automaticLinear').AutomaticLinearSettings;scoreBefore:number|null;scoreAfter:number;candidatesEvaluated:number;warnings:string[]}
  direction: string
  initializationSource?: 'landmarks' | 'image-geometry' | 'automatic'
  model?: RegistrationModel
  nonlinear?: import('../../../packages/imaging-math/nonlinearSpline').NonlinearSpline
  parameterCount?: number
  diagnostics?: TransformDiagnostics
  reflectionConfirmed?: boolean
  landmarkModel?: RegistrationModel
  landmarkMatrix: Mat4
  baseMatrix: Mat4
  matrix: Mat4
  inverse: Mat4
  rms: number
  manualAffine?: import('./manualAffine').ManualAffineState
  manual: Params6
  proposal: { matrix: Mat4; scoreBefore: number; scoreAfter: number; params: number[]; kind?: 'automatic' | 'spline' | 'jip'; previous?: FitState | null } | null
  landmarkSnapshot: Landmark[]
  landmarksChanged: boolean
  fittedAt: string
}
export type WindowConstraint = Partial<Record<Plane, WindowBounds>>
