import type { FitState, Landmark } from '../../app/types'
import type { Mat4 } from '../registration/rigid'

export type ExportArtifact = { filename: string; blob: Blob }

export function matrixText(matrix: Mat4): string {
  return matrix.map(row => row.map(value => value.toFixed(10)).join(' ')).join('\n') + '\n'
}

export function createRegistrationArtifacts(options: {
  appVersion: string
  fit: FitState
  landmarks: Landmark[]
}): ExportArtifact[] {
  const { fit, landmarks, appVersion } = options
  const [moving, fixed] = fit.direction.split('-').map(value => value.toUpperCase())
  const payload = {
    application: 'Brainana Align',
    schemaVersion: 5,
    appVersion,
    direction: fit.direction,
    transform_model: fit.transformModel,
    enabled_parameters: fit.enabledParameters,
    parameter_count: fit.linearDiagnostics.parameterCount,
    determinant: fit.linearDeterminant,
    scale_estimates: fit.linearDiagnostics.scales,
    shear_magnitude: fit.linearDiagnostics.shearMagnitude,
    reflection: fit.linearDiagnostics.reflected,
    reflection_explicitly_confirmed: fit.reflectionConfirmed,
    linear_condition_estimate: fit.linearDiagnostics.conditionEstimate,
    linear_warnings: fit.linearDiagnostics.warnings,
    nonlinear_landmark_warp: fit.landmarkWarp,
    automatic_deformation_grid: fit.deformation,
    rms_mm: fit.rms,
    landmark_matrix: fit.landmarkMatrix,
    accepted_base_matrix: fit.baseMatrix,
    manual_adjustment: {
      translation_mm: fit.manual.slice(0, 3),
      rotation_deg: fit.manual.slice(3),
    },
    forward_matrix: fit.matrix,
    inverse_matrix: fit.inverse,
    alignment_landmarks: fit.landmarkSnapshot,
    current_landmarks: landmarks,
    landmarks_changed_since_alignment: fit.landmarksChanged,
    fitted_at: fit.fittedAt,
  }
  return [
    {
      filename: 'brainana-align_registration.json',
      blob: new Blob([JSON.stringify(payload, null, 2)], { type: 'application/json' }),
    },
    {
      filename: `from-${moving}_to-${fixed}_${fit.transformModel}.txt`,
      blob: new Blob([matrixText(fit.matrix)], { type: 'text/plain' }),
    },
    {
      filename: `from-${fixed}_to-${moving}_${fit.transformModel}.txt`,
      blob: new Blob([matrixText(fit.inverse)], { type: 'text/plain' }),
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
