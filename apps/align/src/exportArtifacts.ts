import {serializeNonlinear} from '../../../packages/imaging-math/nonlinearSerialization'
import {DISPLACEMENT_FIELD_PROFILE} from '../../../packages/imaging-math/splineFields'
import {getRegistrationPairing,imageLabel} from './registrationPairing'
import {validateSpline,splineDiagnostics} from '../../../packages/imaging-math/nonlinearSpline'
import { diagnoseTransform, LANDMARK_MODELS, validateAffine } from './landmarkFit'
import { invertAffine } from './roiWarp'
import type { FitState, Landmark } from './appTypes'
import type { Mat4 } from './rigid'

export type ExportArtifact = { filename: string; blob: Blob }

export function matrixText(matrix: Mat4): string {
  return matrix.map(row => row.map(value => value.toFixed(10)).join(' ')).join('\n') + '\n'
}

export function createRegistrationArtifacts(options: { alignedExportOptions?:import('./alignedExportOptions').AlignedExportOptions;
  appVersion: string
  imageExportMode?:'native'|'processed'
  fit: FitState
  landmarks: Landmark[]
  images?:Record<import('./appTypes').Modality,import('./appTypes').Loaded|null>
  fieldExport?:{requestedSpacingMm:number;forwardGrid:{dims:number[];affine:number[][]};inverseGrid:{dims:number[];affine:number[][]}}
}): ExportArtifact[] {
  const { fit, landmarks, appVersion } = options
  if (fit.proposal) throw new Error('Accept or reject the refinement proposal before exporting')
  validateAffine(fit.matrix)
  const model = fit.model ?? 'rigid'
  if((['spline','jip'].includes(model))!==Boolean(fit.nonlinear))throw Error('Nonlinear model and coefficients do not match')
  const diagnostics = diagnoseTransform(fit.matrix)
  if (diagnostics.reflection && !fit.reflectionConfirmed) throw new Error('Explicit reflection confirmation is required before export')
  const inverse = invertAffine(fit.matrix)
  const moving=imageLabel(fit.direction==='ct-mri'?'ct':'mri'),fixed=imageLabel(fit.direction==='ct-mri'?'mri':'ct')
  const payload = {
    image_preparation:options.images?Object.fromEntries(Object.entries(options.images).map(([modality,item])=>[modality,item?{label:imageLabel(modality==='ct'?'ct':'mri'),acquisition:item.acquisition??null,native_dimensions:item.native?.info.dims??item.processing?.base.dims??item.raw.dims,native_spacing_mm:item.native?.info.spacing??item.processing?.base.pixDims??item.raw.pixDims,native_affine:item.native?.info.affine??item.processing?.base.affine??item.raw.affine,working_dimensions:item.raw.dims,working_spacing_mm:item.raw.pixDims,selected_frame_zero_based:item.native?.frame??0,processing:item.processing?.recipe??null,mask_convention:'1 excluded, 0 included; registration sample centers',export_sampling:options.alignedExportOptions?'See aligned_image_export_options; exact output geometry accompanies the saved image':options.imageExportMode==='processed'?'processed working moving input onto processed working fixed grid':'original moving input onto native fixed grid'}:null])):null,
    application: 'Brainana Align',
    registration_pairing:getRegistrationPairing(),
    internal_image_slots:{first:'mri',second:'ct'},
    aligned_image_export_options:options.alignedExportOptions??null,
    schemaVersion: fit.nonlinear?4:3,
    transform_model: fit.jipRegistration?.settings.engine==='itk'&&model==='jip'?'itk-bspline':model,
    registration_engine:fit.jipRegistration?.settings.engine==='itk'?'ITK':fit.jipRegistration?'JIP':'Align',
    parameter_count: model==='jip'?null:fit.nonlinear&&fit.nonlinear.kernel!=='jip-dense-pull'?fit.nonlinear.controls.length*3:LANDMARK_MODELS[model==='spline'?'affine':model].parameters,
    coordinate_system: 'NIfTI RAS physical millimeters',
    moving_space: moving,
    fixed_space: fixed,
    matrix_direction: 'moving_to_fixed',
    determinant: diagnostics.determinant,
    scale_estimates_xyz: diagnostics.scales,
    shear_estimate: diagnostics.shear,
    shear_definition: 'maximum absolute cosine between transformed basis axes',
    reflection: diagnostics.reflection,
    reflection_confirmed: Boolean(fit.reflectionConfirmed),
    warnings: [...new Set([...(fit.diagnostics?.warnings ?? []), ...diagnostics.warnings])],
    landmark_fit_diagnostics: diagnoseTransform(fit.landmarkMatrix),
    landmark_transform_model: fit.landmarkModel ?? null,
    appVersion,
    direction: fit.direction,
    rms_mm: fit.landmarkSnapshot.some(l=>l.enabled&&l.mri&&l.ct)?fit.rms:null,
    automatic_registration: fit.automaticRegistration ?? null,
    jip_registration:fit.jipRegistration??null,
    initialization_source: fit.initializationSource ?? 'landmarks',
    landmark_matrix: fit.landmarkMatrix,
    accepted_base_matrix: fit.baseMatrix,
    manual_adjustment: {
      translation_mm: fit.manual.slice(0, 3),
      rotation_deg: fit.manual.slice(3),
      affine: fit.manualAffine ?? null,
    },
    forward_matrix: fit.nonlinear?null:fit.matrix,
    inverse_matrix: fit.nonlinear?null:inverse,
    nonlinear:serializeNonlinear(fit.nonlinear),
    nonlinear_diagnostics:fit.nonlinear&&fit.nonlinear.kernel!=='jip-dense-pull'?splineDiagnostics(fit.nonlinear,fit.matrix):null,
    affine_component_forward_matrix:fit.nonlinear?fit.matrix:undefined,
    nonlinear_mapping:fit.nonlinear?.kernel==='jip-dense-pull'?'F(x)=D K^-1(A P x), K(y)=y+stored JIP pull residual; fixed RAS-mm field, zero residual outside fixed working field':fit.nonlinear?'F(x)=D(G(A P x)); A=nonlinear.baseAffine; P=nonlinear.inputAffine or identity; D=affine_component_forward_matrix * inverse(A P); G(q)=q+regularized 3D spline displacement':undefined,
    inverse_method:fit.nonlinear?.kernel==='jip-dense-pull'?'Direct JIP pull for fixed-to-moving; damped Newton inverse for moving-to-fixed, checked convergence':fit.nonlinear?'fixed-point inverse of G, then inverse(A P); convergence tolerance 1e-7 mm; not a separately fitted reverse spline':undefined,
    displacement_field_sampling:options.fieldExport??null,
    displacement_field_convention:fit.nonlinear?'RAS millimeters; F(x)-x on moving grid, F^-1(y)-y on fixed grid; NIfTI displacement intent 1006, R/A/S components in dim5; LPS tools require explicit coordinate conversion':undefined,
    displacement_field_profile:fit.nonlinear?DISPLACEMENT_FIELD_PROFILE:undefined,
    alignment_landmarks: fit.landmarkSnapshot,
    current_landmarks: landmarks,
    landmarks_changed_since_alignment: fit.landmarksChanged,
    fitted_at: fit.fittedAt,
  }
  if(fit.nonlinear){validateSpline(fit.nonlinear);if(diagnostics.reflection)throw Error('Nonlinear export cannot reflect anatomy');return [{filename:'brainana-align_registration.json',blob:new Blob([JSON.stringify(payload,null,2)],{type:'application/json'})}]}
  return [
    {
      filename: 'brainana-align_registration.json',
      blob: new Blob([JSON.stringify(payload, null, 2)], { type: 'application/json' }),
    },
    {
      filename: `from-${moving}_to-${fixed}_${model}.txt`,
      blob: new Blob([matrixText(fit.matrix)], { type: 'text/plain' }),
    },
    {
      filename: `from-${fixed}_to-${moving}_${model}.txt`,
      blob: new Blob([matrixText(inverse)], { type: 'text/plain' }),
    },
  ]
}

export function triggerBrowserDownload(blob: Blob, filename: string): void {
  const anchor = document.createElement('a')
  const url = URL.createObjectURL(blob)
  anchor.href = url
  anchor.download = filename
  anchor.click()
  setTimeout(() => URL.revokeObjectURL(url), 1000)
}

export function saveArtifact(blob: Blob, filename: string): Promise<void> | void {
  if (window.brainanaAlignSaveBlob) return window.brainanaAlignSaveBlob(blob, filename)
  triggerBrowserDownload(blob, filename)
}

export async function saveArtifacts(artifacts: ExportArtifact[]): Promise<void> {
  for (const artifact of artifacts) await saveArtifact(artifact.blob, artifact.filename)
}
