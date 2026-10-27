import assert from 'node:assert/strict'
import { applyMat4, fitRigid, invertRigid, multiplyMat4, rigidDelta, type Mat4, type Vec3 } from '../apps/align/src/rigid.ts'
import { withinOptimizationWindows, type OptimizationWindows } from '../apps/align/src/optimizationWindows.ts'

const EPS = 1e-8
const source: Vec3[] = [[0,0,0],[10,0,0],[0,12,0],[0,0,8],[4,5,6],[9,3,2]]
const expected = rigidDelta([12,-7,4,8,-5,13], [3,4,2])
const target = source.map(point => applyMat4(expected, point))
const fit = fitRigid(source, target)
assert.ok(fit.rms < 1e-7, `Rigid fit RMS was ${fit.rms}`)
for (let r=0;r<4;r++) for (let c=0;c<4;c++) assert.ok(Math.abs(fit.matrix[r][c]-expected[r][c]) < 1e-6)



// Large rotations previously exposed the power-iteration eigensolver bug. The
// rigid fit must remain exact for quarter turns, near-180 degree rotations,
// and strongly oblique combinations.
for (const [name, params, center] of [
  ['quarter-turn', [35,-22,14,90,0,0], [4,-3,8]],
  ['axis-swapped-oblique', [-18,31,-9,87,-73,112], [-12,5,19]],
  ['near-180', [6,11,-20,179.5,-0.75,1.25], [23,-17,4]],
] as Array<[string, [number,number,number,number,number,number], Vec3]>) {
  const truth = rigidDelta(params, center)
  const transformed = source.map(point => applyMat4(truth, point))
  const recovered = fitRigid(source, transformed)
  assert.ok(recovered.rms < 1e-7, `${name} rigid fit RMS was ${recovered.rms}`)
  assertMatrix(recovered.matrix, truth, 1e-6, name)

  const reverse = fitRigid(transformed, source)
  assert.ok(reverse.rms < 1e-7, `${name} inverse rigid fit RMS was ${reverse.rms}`)
  assertMatrix(reverse.matrix, invertRigid(truth), 1e-6, `${name} inverse`)
}

// Landmark positions arrive at fitRigid in physical NIfTI world millimeters.
// Simulate strongly different voxel-to-world affines and verify that fitting
// the resulting physical points remains independent of voxel orientation.
const movingVoxelToWorld: Mat4 = [
  [0, -0.8, 0.15, 42],
  [1.1, 0.05, 0.2, -37],
  [-0.1, 0.25, 1.7, 18],
  [0, 0, 0, 1],
]
const worldSource = source.map(point => applyMat4(movingVoxelToWorld, point))
const worldTruth = rigidDelta([21,-13,7,102,-41,158], [8,-12,27])
const worldTarget = worldSource.map(point => applyMat4(worldTruth, point))
const worldFit = fitRigid(worldSource, worldTarget)
assert.ok(worldFit.rms < 1e-7, `oblique physical-coordinate fit RMS was ${worldFit.rms}`)
assertMatrix(worldFit.matrix, worldTruth, 1e-6, 'oblique physical-coordinate')

assert.throws(
  () => fitRigid([[0,0,0],[1,1,1],[2,2,2]], [[5,0,0],[6,1,1],[7,2,2]]),
  /collinear/,
  'Collinear landmarks should be rejected with a useful error',
)

const identity = multiplyMat4(fit.matrix, invertRigid(fit.matrix))
assertIdentity(identity, 1e-8)
for (const point of source) {
  const roundTrip = applyMat4(fit.inverse, applyMat4(fit.matrix, point))
  assertVec(roundTrip, point, 1e-7)
}

const windows: OptimizationWindows = {
  mri: {
    sagittal: { min: [0, 2, 3], max: [0, 8, 9] },
    axial: { min: [1, 4, 0], max: [7, 10, 0] },
  },
  ct: {},
}
assert.equal(withinOptimizationWindows([5,6,5], windows.mri), true)
assert.equal(withinOptimizationWindows([9,6,5], windows.mri), false)
assert.equal(withinOptimizationWindows([5,1,5], windows.mri), false)
assert.equal(withinOptimizationWindows([5,6,20], windows.mri), false)
assert.equal(withinOptimizationWindows([100,-100,50], windows.ct), true, 'No defined planes must remain unrestricted')

console.log('scientific regression checks passed')


function assertMatrix(actual: Mat4, expectedValue: Mat4, tolerance: number, label: string) {
  for (let r=0;r<4;r++) for (let c=0;c<4;c++) {
    assert.ok(Math.abs(actual[r][c]-expectedValue[r][c]) < tolerance, `${label} mismatch [${r},${c}]: ${actual[r][c]} vs ${expectedValue[r][c]}`)
  }
}

function assertIdentity(matrix: Mat4, tolerance: number) {
  for (let r=0;r<4;r++) for (let c=0;c<4;c++) {
    const expectedValue = r===c ? 1 : 0
    assert.ok(Math.abs(matrix[r][c]-expectedValue) < tolerance, `identity mismatch [${r},${c}]`)
  }
}
function assertVec(actual: Vec3, expectedValue: Vec3, tolerance: number) {
  for (let i=0;i<3;i++) assert.ok(Math.abs(actual[i]-expectedValue[i]) < tolerance)
}
