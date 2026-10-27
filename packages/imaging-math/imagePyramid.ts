import type { Dims3, RawNifti } from './roiWarp'

export type RegistrationResolutionPreset = 'fast' | 'balanced' | 'precise' | 'custom'
export type ShrinkFactorPreset = 'fast' | 'standard' | 'custom'

/** Input-grid-relative pyramid levels. One means the native grid. */
export function shrinkFactorLevels(preset: ShrinkFactorPreset, custom = ''): number[] {
  if (preset === 'fast') return [4, 2, 1]
  if (preset === 'standard') return [8, 4, 2, 1]
  const factors = custom.split(',').map(value => value.trim() === '' ? NaN : Number(value))
  if (!factors.length || factors.some(value => !Number.isFinite(value) || value < 1 || value > 32 || !Number.isInteger(value))) {
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
export function resolutionLevels(preset: RegistrationResolutionPreset, custom = ''): number[] {
  if (preset === 'fast') return [4]
  if (preset === 'balanced') return [4, 2]
  if (preset === 'precise') return [4, 2, 0]
  const levels = custom.split(',').map(value => value.trim() === '' ? NaN : Number(value))
  if (!levels.length || levels.some(value => !Number.isFinite(value) || value < 0 || value > 12)) throw new Error('Custom registration resolutions must be comma-separated values from 0 through 12 mm. Use 0 for native resolution.')
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

/** Downsample each input axis by the same integer factor after Gaussian anti-alias filtering. */
export function downsampleByFactor(source: RawNifti, factor: number, smoothingSigmaNativeVoxels = Math.max(0, factor * .5)): RawNifti {
  if (!Number.isInteger(factor) || factor < 1) throw new Error('Registration shrink factors must be positive integers.')
  if (!Number.isFinite(smoothingSigmaNativeVoxels) || smoothingSigmaNativeVoxels < 0) throw new Error('Registration smoothing sigma must be finite and nonnegative.')
  if (factor === 1 && smoothingSigmaNativeVoxels <= .01) return source
  const dims = source.dims.map(size => size===1?1:Math.max(2, Math.floor((size - 1) / factor) + 1)) as Dims3
  const scale = dims.map((size, axis) => size===1?1:(source.dims[axis] - 1) / (size - 1)) as Dims3
  return downsampleGrid(source,dims,scale,[smoothingSigmaNativeVoxels,smoothingSigmaNativeVoxels,smoothingSigmaNativeVoxels])
}
function downsampleGrid(source:RawNifti,dims:Dims3,scale:Dims3,sigmas:Dims3):RawNifti{
  // Fuse each separable filtering pass with decimation. Later passes operate on
  // already smaller grids instead of three full-size smoothed copies.
  let values:Float32Array=frame(source),currentDims=[...source.dims] as Dims3
  for(let axis=0;axis<3;axis++){
    const kernel=gaussianKernel(sigmas[axis]),radius=(kernel.length-1)>>1
    if(dims[axis]===currentDims[axis]&&kernel.length===1)continue
    const nextDims=[...currentDims] as Dims3;nextDims[axis]=dims[axis]
    const output=new Float32Array(nextDims[0]*nextDims[1]*nextDims[2])
    // Precompute source indices once per axis position, not once per output
    // voxel/tap. Keep the original summation order and Float32 pass boundaries.
    const stride = axis === 0 ? 1 : axis === 1 ? currentDims[0] : currentDims[0] * currentDims[1]
    const firstIndices = new Int32Array(dims[axis] * kernel.length)
    const secondIndices = new Int32Array(firstIndices.length)
    const fractions = new Float64Array(dims[axis])
    for (let position = 0; position < dims[axis]; position++) {
      const coordinate = position * scale[axis], low = Math.floor(coordinate)
      fractions[position] = coordinate - low
      for (let tap = 0; tap < kernel.length; tap++) {
        const index = position * kernel.length + tap, offset = tap - radius
        firstIndices[index] = Math.max(0, Math.min(currentDims[axis] - 1, low + offset)) * stride
        secondIndices[index] = Math.max(0, Math.min(currentDims[axis] - 1, low + 1 + offset)) * stride
      }
    }
    let index = 0
    for (let z = 0; z < nextDims[2]; z++) for (let y = 0; y < nextDims[1]; y++) for (let x = 0; x < nextDims[0]; x++) {
      const position = axis === 0 ? x : axis === 1 ? y : z
      const base = x + currentDims[0] * (y + currentDims[1] * z) - position * stride
      const fraction = fractions[position], start = position * kernel.length
      let value = 0
      for (let tap = 0; tap < kernel.length; tap++) {
        const first = values[base + firstIndices[start + tap]]
        // IEEE 0 * NaN/Infinity is NaN: an unused neighbor must not corrupt a
        // voxel-center sample. Nonfinite values with positive support stay invalid.
        const interpolated = fraction === 0 ? first : first * (1 - fraction) + values[base + secondIndices[start + tap]] * fraction
        value += kernel[tap] * interpolated
      }
      output[index++] = value
    }
    values=output;currentDims=nextDims
  }
  const affine = source.affine.map(row => [...row])
  for (let row = 0; row < 3; row++) for (let axis = 0; axis < 3; axis++) affine[row][axis] = source.affine[row][axis] * scale[axis]
  const pixDims = [0, 1, 2].map(axis => Math.hypot(affine[0][axis], affine[1][axis], affine[2][axis])) as Dims3
  return { ...source, dims, frameCount: 1, affine, pixDims, values, datatypeCode: 16, slope: 1, intercept: 0 }
}

/** Gaussian-prefilter and resample a volume while preserving its physical extent and orientation. */
export function downsampleForRegistration(source: RawNifti, spacingMm: number): RawNifti {
 if(!Number.isFinite(spacingMm)||spacingMm<=0||source.pixDims.every(v=>v>=spacingMm))return source
 const requested=source.pixDims.map(v=>Math.max(1,spacingMm/v)) as Dims3
 const dims=source.dims.map((n,a)=>n===1?1:Math.max(2,Math.floor((n-1)/requested[a])+1)) as Dims3
 const scale=dims.map((n,a)=>n===1?1:(source.dims[a]-1)/(n-1)) as Dims3
 return downsampleGrid(source,dims,scale,requested.map(v=>v===1?0:v*.5) as Dims3)
}
