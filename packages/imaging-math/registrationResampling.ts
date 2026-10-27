import { applyMat4, type Mat4 } from './rigid'
import { applyAffine, frame, invertAffine, sampleLinear, type RawNifti } from './roiWarp'
import { validateAffine } from './landmarkFit'

// Pull destination voxels back through fixed world -> moving world -> moving
// voxel coordinates. Each image affine is applied exactly once.
export function resampleRegistration(source: RawNifti, target: RawNifti, movingToFixed: Mat4): Float32Array {
  validateAffine(movingToFixed)
  const output = new Float32Array(target.dims[0] * target.dims[1] * target.dims[2])
  const fixedToMoving = invertAffine(movingToFixed)
  const worldToVoxel = invertAffine(source.affine)
  const values = frame(source, 0)
  let index = 0
  for (let k = 0; k < target.dims[2]; k++) for (let j = 0; j < target.dims[1]; j++) for (let i = 0; i < target.dims[0]; i++, index++) {
    const fixedWorld = applyAffine(target.affine, [i, j, k])
    const movingWorld = applyMat4(fixedToMoving, fixedWorld)
    output[index] = sampleLinear(values, source.dims, applyAffine(worldToVoxel, movingWorld))
  }
  return output
}
