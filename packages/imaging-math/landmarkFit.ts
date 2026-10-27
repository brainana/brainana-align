import { applyMat4, fitRigid, type Mat4, type Vec3 } from './rigid'
import { determinant3, invertAffine } from './roiWarp'

export type LandmarkModel = 'rigid' | 'similarity' | 'anisotropic' | 'affine'
export const LANDMARK_MODELS = {
  rigid: { label: 'Rigid, 6 parameters', parameters: 6 },
  similarity: { label: 'Similarity, 7 parameters', parameters: 7 },
  anisotropic: { label: 'Anisotropic scale, 9 parameters', parameters: 9 },
  affine: { label: 'Affine, 12 parameters', parameters: 12 },
} as const
export type TransformDiagnostics = {
  determinant: number
  scales: Vec3
  shear: number
  reflection: boolean
  conditionNumber: number
  warnings: string[]
}
export function isLandmarkModel(value: unknown): value is LandmarkModel {
  return typeof value === 'string' && Object.hasOwn(LANDMARK_MODELS, value)
}
export function validateAffine(matrix: unknown): asserts matrix is Mat4 {
  if (!Array.isArray(matrix) || matrix.length !== 4 || !matrix.every(row => Array.isArray(row) && row.length === 4 && row.every(v => typeof v === 'number' && Number.isFinite(v))) || matrix[3].some((v: number, i: number) => Math.abs(v - (i === 3 ? 1 : 0)) > 1e-10)) throw new Error('Transform must be a finite 4 × 4 affine matrix')
  invertAffine(matrix)
}
const center = (points: Vec3[]): Vec3 => [0, 1, 2].map(axis => points.reduce((sum, p) => sum + p[axis], 0) / points.length) as Vec3
const dot = (a: number[], b: number[]) => a.reduce((sum, v, i) => sum + v * b[i], 0)
const subtract = (p: Vec3, c: Vec3): Vec3 => p.map((v, i) => v - c[i]) as Vec3

// Column-pivoted QR with reorthogonalization. Centering and column normalization
// avoid squaring the condition number or depending on the scanner's origin.
export function leastSquares(design: number[][], values: number[]): { solution: number[]; condition: number } {
  const n = design[0].length, m = design.length
  const scales = Array.from({ length: n }, (_, j) => Math.hypot(...design.map(row => row[j])))
  if (m < n || scales.some(v => !Number.isFinite(v) || v < 1e-12)) throw new Error('Landmarks are geometrically degenerate: insufficient independent directions')
  const columns = scales.map((s, j) => design.map(row => row[j] / s))
  const order = Array.from({ length: n }, (_, i) => i)
  const r = Array.from({ length: n }, () => Array(n).fill(0) as number[])
  const q: number[][] = []
  for (let j = 0; j < n; j++) {
    let pivot = j
    for (let k = j + 1; k < n; k++) if (dot(columns[k], columns[k]) > dot(columns[pivot], columns[pivot])) pivot = k
    ;[columns[j], columns[pivot]] = [columns[pivot], columns[j]]
    ;[order[j], order[pivot]] = [order[pivot], order[j]]
    for (let k = 0; k < j; k++) [r[k][j], r[k][pivot]] = [r[k][pivot], r[k][j]]
    r[j][j] = Math.hypot(...columns[j])
    if (r[j][j] < 1e-7) throw new Error('Landmarks are coplanar or nearly collinear; this model requires a stable three-dimensional configuration')
    q[j] = columns[j].map(v => v / r[j][j])
    for (let k = j + 1; k < n; k++) {
      for (let pass = 0; pass < 2; pass++) {
        const projection = dot(q[j], columns[k])
        r[j][k] += projection
        columns[k] = columns[k].map((v, i) => v - projection * q[j][i])
      }
    }
  }
  const b = q.map(col => dot(col, values))
  const x = Array(n).fill(0) as number[]
  for (let j = n - 1; j >= 0; j--) x[j] = (b[j] - r[j].reduce((sum, v, k) => sum + (k > j ? v * x[k] : 0), 0)) / r[j][j]
  const solution = Array(n).fill(0) as number[]
  x.forEach((v, j) => solution[order[j]] = v / scales[order[j]])
  return { solution, condition: (Math.max(...scales) / Math.min(...scales)) * Math.max(...r.map((row, j) => row[j])) / Math.min(...r.map((row, j) => row[j])) }
}
function matrixFromLinear(linear: number[][], cs: Vec3, ct: Vec3): Mat4 {
  return [...linear.map((row, i) => [...row, ct[i] - dot(row, cs)]), [0, 0, 0, 1]]
}
function affineLinear(x: Vec3[], y: Vec3[]) {
  const fits = [0, 1, 2].map(axis => leastSquares(x, y.map(p => p[axis])))
  return { linear: fits.map(f => f.solution), condition: Math.max(...fits.map(f => f.condition)) }
}
export function diagnoseTransform(matrix: Mat4): TransformDiagnostics {
  validateAffine(matrix)
  const columns = [0, 1, 2].map(j => [matrix[0][j], matrix[1][j], matrix[2][j]])
  const scales = columns.map(c => Math.hypot(...c)) as Vec3
  const determinant = determinant3(matrix)
  const shear = Math.max(...[[0, 1], [0, 2], [1, 2]].map(([i, j]) => Math.abs(dot(columns[i], columns[j]) / (scales[i] * scales[j]))))
  const inverse = invertAffine(matrix)
  const norm = (m: Mat4) => Math.sqrt(m.slice(0, 3).reduce((s, row) => s + row.slice(0, 3).reduce((t, v) => t + v * v, 0), 0))
  const conditionNumber = norm(matrix) * norm(inverse) / 3
  const reflection = determinant < 0
  const warnings: string[] = []
  if (scales.some(s => s < 0.25 || s > 4)) warnings.push('Extreme scale: an axis is outside 0.25–4. Check landmark pairing and image units.')
  if (shear > 0.15) warnings.push('Substantial affine shear: transformed axes are no longer orthogonal.')
  if (Math.abs(determinant) < 1e-6) warnings.push('Determinant is close to zero; the transform nearly collapses anatomy.')
  if (conditionNumber > 1000) warnings.push('The transform is numerically unstable; small landmark changes can cause large errors.')
  if (reflection) warnings.push('REFLECTION: this transform mirrors anatomy. Verify left–right conventions before accepting or exporting.')
  return { determinant, scales, shear, reflection, conditionNumber, warnings }
}
export function fitLandmarks(source: Vec3[], target: Vec3[], model: LandmarkModel = 'rigid') {
  if (!isLandmarkModel(model)) throw new Error('Unknown landmark transform model')
  if (source.length !== target.length || source.length < (model === 'affine' || model === 'anisotropic' ? 4 : 3)) throw new Error(`${LANDMARK_MODELS[model].label} requires at least ${model === 'affine' || model === 'anisotropic' ? 4 : 3} complete landmark pairs`)
  if (![...source, ...target].every(p => p.length === 3 && p.every(Number.isFinite))) throw new Error('Landmark coordinates must be finite physical millimeters')
  const rigid = fitRigid(source, target)
  const cs = center(source), ct = center(target)
  const x = source.map(p => subtract(p, cs)), y = target.map(p => subtract(p, ct))
  let matrix = rigid.matrix
  let geometryCondition = 1
  if (model === 'similarity') {
    const rotated = x.map(p => applyMat4([...rigid.matrix.slice(0, 3).map(row => [...row.slice(0, 3), 0]), [0, 0, 0, 1]], p))
    const scale = rotated.reduce((s, p, i) => s + dot(p, y[i]), 0) / x.reduce((s, p) => s + dot(p, p), 0)
    if (!(scale > 1e-8)) throw new Error('Similarity scale is zero or unstable')
    matrix = matrixFromLinear(rigid.matrix.slice(0, 3).map(row => row.slice(0, 3).map(v => v * scale)), cs, ct)
  } else if (model === 'affine' || model === 'anisotropic') {
    const targetGeometry = leastSquares(y, x.map(p => p[0]))
    const affine = affineLinear(x, y)
    geometryCondition = Math.max(affine.condition, targetGeometry.condition)
    if (model === 'affine') matrix = matrixFromLinear(affine.linear, cs, ct)
    else {
      // A = R diag(s): scales are along the moving physical RAS axes before
      // rotation. Optimize every pair jointly, with positive scales and det(R)=+1.
      const basis: Vec3[] = [[0, 0, 0], [1, 0, 0], [0, 1, 0], [0, 0, 1]]
      const lengths = [0, 1, 2].map(j => Math.hypot(...affine.linear.map(row => row[j])))
      const basisTarget = basis.map(p => [0, 1, 2].map(i => affine.linear[i].reduce((s, v, j) => s + v * p[j] / lengths[j], 0)) as Vec3)
      const seed = fitRigid(basis, basisTarget).matrix
      let bestError = Infinity
      let anyConverged = false
      for (const initial of [rigid.matrix, seed]) {
        let rotation = initial.slice(0, 3).map(row => row.slice(0, 3))
        let previousError = Infinity
        let scales: Vec3 = [1, 1, 1]
        let converged = false
        let seedError = Infinity, seedMatrix = matrix
        for (let iteration = 0; iteration < 3000; iteration++) {
          scales = [0, 1, 2].map(j => Math.max(1e-8, x.reduce((s, p, i) => s + p[j] * dot(rotation.map(row => row[j]), y[i]), 0) / x.reduce((s, p) => s + p[j] * p[j], 0))) as Vec3
          const scaled = x.map(p => p.map((v, j) => v * scales[j]) as Vec3)
          rotation = fitRigid(scaled, y).matrix.slice(0, 3).map(row => row.slice(0, 3))
          const candidate = matrixFromLinear(rotation.map(row => row.map((v, j) => v * scales[j])), cs, ct)
          const error = source.reduce((s, p, i) => s + dot(subtract(applyMat4(candidate, p), target[i]), subtract(applyMat4(candidate, p), target[i])), 0)
          seedError = error; seedMatrix = candidate
          if (Math.abs(previousError - error) < 1e-13 * Math.max(1, previousError === Infinity ? 1 : previousError)) { converged = true; break }
          previousError = error
        }
        if (converged || seedError < 1e-12) {
          anyConverged = true
          if (seedError < bestError) { bestError = seedError; matrix = seedMatrix }
        }
      }
      if (!anyConverged) throw new Error('Anisotropic fit did not converge; use more widely distributed landmarks or try another model')
    }
  }
  const diagnostics = diagnoseTransform(matrix)
  if (geometryCondition > 1000) diagnostics.warnings.push('Landmark geometry is numerically unstable; distribute points across all three dimensions.')
  const residuals = source.map((p, i) => Math.hypot(...subtract(applyMat4(matrix, p), target[i])))
  const rms = Math.sqrt(residuals.reduce((s, d) => s + d * d, 0) / residuals.length)
  // A reflected affine candidate can expose a convention mismatch even when
  // the selected proper transform is rigid, similarity, or anisotropic.
  if (model !== 'affine' && source.length >= 4) {
    try {
      const reflected = matrixFromLinear(affineLinear(x, y).linear, cs, ct)
      const reflectedRms = Math.sqrt(source.reduce((s, p, i) => s + dot(subtract(applyMat4(reflected, p), target[i]), subtract(applyMat4(reflected, p), target[i])), 0) / source.length)
      if (determinant3(reflected) < 0 && rms > 0.1 && reflectedRms < rms * 0.25) diagnostics.warnings.push('A mirrored affine candidate fits far better. Check left–right labels and coordinate conventions; it has not been applied.')
    } catch { /* Planar landmarks cannot identify an affine candidate. */ }
  }
  return { model, parameterCount: LANDMARK_MODELS[model].parameters, matrix, inverse: invertAffine(matrix), residuals, rms, diagnostics }
}
