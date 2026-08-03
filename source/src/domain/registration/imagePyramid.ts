import type { Dims3, RawNifti } from '../imaging/roiWarp'

export type RegistrationResolutionPreset = 'fast' | 'balanced' | 'precise' | 'custom'
export type ShrinkFactorPreset = 'fast' | 'standard' | 'custom'

/** Input-grid-relative pyramid levels. One means the native grid. */
export function shrinkFactorLevels(preset: ShrinkFactorPreset, custom = ''): number[] {
  if (preset === 'fast') return [4, 2, 1]
  if (preset === 'standard') return [8, 4, 2, 1]
  const factors = custom.split(',').map(value => Number(value.trim())).filter(Number.isFinite)
  if (!factors.length || factors.some(value => value < 1 || value > 32 || !Number.isInteger(value))) {
    throw new Error('Custom shrink factors must be comma-separated integers from 1 through 32, for example 8,4,2,1.')
  }
  const unique = [...new Set(factors)]
  if (unique.some((value, index) => index > 0 && value >= unique[index - 1])) throw new Error('Shrink factors must run from coarse to fine, for example 8,4,2,1.')
  if (unique.at(-1) !== 1) throw new Error('The final shrink factor must be 1 so registration finishes on the native grid.')
  return unique
}

/** Characteristic physical extent used to make registration pyramids species-independent. */
export function characteristicExtentMm(image: RawNifti): number {
  const extents = image.dims.map((size, axis) => Math.max(image.pixDims[axis], (size - 1) * image.pixDims[axis]))
  return Math.cbrt(extents[0] * extents[1] * extents[2])
}

export function relativeResolutionLevels(image: RawNifti, preset: RegistrationResolutionPreset, custom = ''): number[] {
  if (preset === 'custom') return resolutionLevels(preset, custom)
  const extent = characteristicExtentMm(image)
  const native = Math.max(...image.pixDims)
  const samples = preset === 'fast' ? [32] : preset === 'balanced' ? [32, 64] : [32, 64, 128]
  const levels = samples.map(count => Math.max(native, extent / count))
  if (preset === 'precise') levels.push(0)
  return [...new Set(levels.map(value => value === 0 ? 0 : Number(value.toPrecision(6))))]
}

const frame = (image: RawNifti) => image.values.subarray(0, image.dims[0] * image.dims[1] * image.dims[2])
function sampleLinear(values: Float32Array, dims: Dims3, xyz: Dims3): number {
  const low=xyz.map(Math.floor) as Dims3, high=low.map((value,axis)=>Math.min(dims[axis]-1,value+1)) as Dims3, fraction=xyz.map((value,axis)=>value-low[axis]) as Dims3
  const at=(x:number,y:number,z:number)=>values[x+dims[0]*(y+dims[1]*z)]
  let total=0
  for(let dz=0;dz<2;dz++)for(let dy=0;dy<2;dy++)for(let dx=0;dx<2;dx++)total+=at(dx?high[0]:low[0],dy?high[1]:low[1],dz?high[2]:low[2])*(dx?fraction[0]:1-fraction[0])*(dy?fraction[1]:1-fraction[1])*(dz?fraction[2]:1-fraction[2])
  return total
}

export function resolutionLevels(preset: RegistrationResolutionPreset, custom = ''): number[] {
  if (preset === 'fast') return [4]
  if (preset === 'balanced') return [4, 2]
  if (preset === 'precise') return [4, 2, 0]
  const levels = custom.split(',').map(value => Number(value.trim())).filter(Number.isFinite)
  if (!levels.length || levels.some(value => value < 0 || value > 12)) throw new Error('Custom registration resolutions must be comma-separated values from 0 through 12 mm. Use 0 for native resolution.')
  return [...new Set(levels)].sort((a, b) => b - a)
}

function gaussianKernel(sigma: number): Float32Array {
  if (sigma < .25) return new Float32Array([1])
  const radius = Math.max(1, Math.ceil(sigma * 3)), kernel = new Float32Array(radius * 2 + 1)
  let total = 0
  for (let index = -radius; index <= radius; index++) { const value = Math.exp(-.5 * (index / sigma) ** 2); kernel[index + radius] = value; total += value }
  for (let index = 0; index < kernel.length; index++) kernel[index] /= total
  return kernel
}

function convolveAxis(input: Float32Array, dims: Dims3, axis: number, kernel: Float32Array): Float32Array {
  if (kernel.length === 1) return input
  const output = new Float32Array(input.length), radius = (kernel.length - 1) >> 1
  const index = (x: number, y: number, z: number) => x + dims[0] * (y + dims[1] * z)
  for (let z = 0; z < dims[2]; z++) for (let y = 0; y < dims[1]; y++) for (let x = 0; x < dims[0]; x++) {
    let sum = 0
    for (let offset = -radius; offset <= radius; offset++) {
      const coordinate = [x, y, z]
      coordinate[axis] = Math.max(0, Math.min(dims[axis] - 1, coordinate[axis] + offset))
      sum += input[index(coordinate[0], coordinate[1], coordinate[2])] * kernel[offset + radius]
    }
    output[index(x, y, z)] = sum
  }
  return output
}

/** Downsample each input axis by the same integer factor after Gaussian anti-alias filtering. */
export function downsampleByFactor(source: RawNifti, factor: number, smoothingSigmaNativeVoxels = Math.max(0, factor * .5)): RawNifti {
  if (!Number.isInteger(factor) || factor < 1) throw new Error('Registration shrink factors must be positive integers.')
  if (factor === 1 && smoothingSigmaNativeVoxels <= .01) return source
  const dims = source.dims.map(size => Math.max(2, Math.floor((size - 1) / factor) + 1)) as Dims3
  const scale = dims.map((size, axis) => (source.dims[axis] - 1) / Math.max(1, size - 1)) as Dims3
  let smoothed: Float32Array = new Float32Array(frame(source))
  for (let axis = 0; axis < 3; axis++) smoothed = convolveAxis(smoothed, source.dims, axis, gaussianKernel(smoothingSigmaNativeVoxels))
  const values = new Float32Array(dims[0] * dims[1] * dims[2])
  let outputIndex = 0
  for (let z = 0; z < dims[2]; z++) for (let y = 0; y < dims[1]; y++) for (let x = 0; x < dims[0]; x++, outputIndex++) values[outputIndex] = sampleLinear(smoothed, source.dims, [x * scale[0], y * scale[1], z * scale[2]])
  const affine = source.affine.map(row => [...row])
  for (let row = 0; row < 3; row++) for (let axis = 0; axis < 3; axis++) affine[row][axis] = source.affine[row][axis] * scale[axis]
  const pixDims = [0, 1, 2].map(axis => Math.hypot(affine[0][axis], affine[1][axis], affine[2][axis])) as Dims3
  return { ...source, dims, frameCount: 1, affine, pixDims, values, datatypeCode: 16, slope: 1, intercept: 0 }
}

/** Gaussian-prefilter and resample a volume while preserving its physical extent and orientation. */
export function downsampleForRegistration(source: RawNifti, spacingMm: number): RawNifti {
  if (!Number.isFinite(spacingMm) || spacingMm <= 0 || source.pixDims.every(value => value >= spacingMm * .95)) return source
  const requestedScale = source.pixDims.map(value => Math.max(1, spacingMm / value)) as Dims3
  const dims = source.dims.map((size, axis) => Math.max(2, Math.floor((size - 1) / requestedScale[axis]) + 1)) as Dims3
  const scale = dims.map((size, axis) => (source.dims[axis] - 1) / Math.max(1, size - 1)) as Dims3
  let smoothed: Float32Array = new Float32Array(frame(source))
  for (let axis = 0; axis < 3; axis++) smoothed = convolveAxis(smoothed, source.dims, axis, gaussianKernel(Math.max(0, .5 * requestedScale[axis])))
  const values = new Float32Array(dims[0] * dims[1] * dims[2])
  let outputIndex = 0
  for (let z = 0; z < dims[2]; z++) for (let y = 0; y < dims[1]; y++) for (let x = 0; x < dims[0]; x++, outputIndex++) values[outputIndex] = sampleLinear(smoothed, source.dims, [x * scale[0], y * scale[1], z * scale[2]])
  const affine = source.affine.map(row => [...row])
  for (let row = 0; row < 3; row++) for (let axis = 0; axis < 3; axis++) affine[row][axis] = source.affine[row][axis] * scale[axis]
  const pixDims = [0, 1, 2].map(axis => Math.hypot(affine[0][axis], affine[1][axis], affine[2][axis])) as Dims3
  // Explicitly verify that index zero remains at the same world origin.
  if (Math.hypot(...[0,1,2].map(axis=>source.affine[axis][3]-affine[axis][3])) > 1e-6) throw new Error('Registration pyramid changed the image world origin.')
  return { ...source, dims, frameCount: 1, affine, pixDims, values, datatypeCode: 16, slope: 1, intercept: 0 }
}
