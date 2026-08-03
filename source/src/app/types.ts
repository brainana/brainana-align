import type { Niivue, NVImage } from '@niivue/niivue'
import type { RawNifti } from '../domain/imaging/roiWarp'
import type { Mat4, Vec3 } from '../domain/registration/rigid'
import type { LinearDiagnostics, LinearParameter, TransformModel } from '../domain/registration/linear'
import type { LandmarkWarp } from '../domain/registration/nonlinear'
import type { DeformationGrid } from '../domain/registration/deformable'

export type Modality = 'mri' | 'ct'
export type Plane = 'sagittal' | 'coronal' | 'axial'
export type Landmark = { id: number; mri: Vec3 | null; ct: Vec3 | null; enabled: boolean; residual?: number }
export type Loaded = { name: string; raw: RawNifti; nvImage: NVImage; sourceFiles: string[] }
export type WindowBounds = { min: Vec3; max: Vec3 }
export type OptimizationWindows = Record<Modality, Partial<Record<Plane, WindowBounds>>>
export type View = { nv: Niivue; modality: Modality; plane: Plane; canvas: HTMLCanvasElement; overlay: SVGSVGElement; windowLayer: HTMLDivElement }
export type ReviewView = { nv: Niivue; plane: Plane; canvas: HTMLCanvasElement; overlay: SVGSVGElement }
export type Params6 = [number, number, number, number, number, number]
export type FitState = {
  direction: string
  transformModel: TransformModel
  enabledParameters: LinearParameter[]
  linearDiagnostics: LinearDiagnostics
  linearDeterminant: number
  reflectionConfirmed: boolean
  landmarkWarp: LandmarkWarp | null
  deformation: DeformationGrid | null
  landmarkMatrix: Mat4
  baseMatrix: Mat4
  matrix: Mat4
  inverse: Mat4
  rms: number
  manual: Params6
  proposal: { matrix: Mat4; scoreBefore: number; scoreAfter: number; params: number[]; initial?: boolean } | null
  deformationProposal: { previous: DeformationGrid | null; proposed: DeformationGrid; scoreBefore: number; scoreAfter: number } | null
  landmarkSnapshot: Landmark[]
  landmarksChanged: boolean
  fittedAt: string
  optimizationResolutionLevels?: number[]
  intensityRefined?: boolean
}
export type WindowConstraint = Partial<Record<Plane, WindowBounds>>
