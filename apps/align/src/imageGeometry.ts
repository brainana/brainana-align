import type { NVImage } from '@niivue/niivue'
import type { RawNifti } from './roiWarp'
import { validateAffine } from './landmarkFit'

export function rawFromNVImage(image: NVImage): RawNifti {
  if (!image.img) throw new Error('The selected image contains no voxel data')
  const dimsSource = image.hdr?.dims ?? image.dims
  if (!dimsSource || dimsSource.length < 4) throw new Error('Unable to determine image dimensions')
  const dims: [number, number, number] = [Number(dimsSource[1]), Number(dimsSource[2]), Number(dimsSource[3])]
  if (!dims.every(v => Number.isFinite(v) && v > 0)) throw new Error('Invalid image dimensions')
  const frameCount = Array.from(dimsSource).slice(4, Math.max(4, Number(dimsSource[0]) + 1)).reduce((n, v) => n * Math.max(1, Number(v) || 1), 1)
  normalizeSpatialUnits(image)
  const affine = image.getAffine().map(row => row.map(Number))
  validateAffine(affine)
  // The selected sform/qform defines physical geometry, even when pixdim is stale.
  const pixDims: [number, number, number] = [0, 1, 2].map(axis =>
    Math.hypot(affine[0][axis], affine[1][axis], affine[2][axis])) as [number, number, number]
  const slope = Number(image.hdr?.scl_slope) || 1
  const intercept = Number(image.hdr?.scl_slope) ? (Number(image.hdr?.scl_inter) || 0) : 0
  const source = image.img as ArrayLike<number>
  const expected = dims[0] * dims[1] * dims[2] * frameCount
  if (source.length < expected) throw new Error(`Image payload is truncated: expected ${expected} voxels, found ${source.length}`)
  const values = new Float32Array(dims[0]*dims[1]*dims[2])
  for (let i = 0; i < values.length; i += 1) values[i] = Number(source[i]) * slope + intercept
  return {
    dims,
    frameCount:1,
    affine,
    pixDims,
    values,
    datatypeCode: Number(image.hdr?.datatypeCode) || 16,
    littleEndian: Boolean(image.hdr?.littleEndian ?? true),
    slope,
    intercept,
  }
}


// Niivue's selected qform/sform is the common affine for rendering and fitting.
// Normalize known spatial units before cloning volumes into any viewer.
export function normalizeSpatialUnits(image: NVImage) {
  if (!image.hdr) throw new Error('Image header unavailable')
  const units = Number(image.hdr.xyzt_units) & 7
  if (![0, 1, 2, 3].includes(units)) throw new Error('Unsupported NIfTI spatial units')
  const factor = units === 1 ? 1000 : units === 3 ? 0.001 : 1
  if (factor === 1) return
  const affine = image.getAffine().map((row, i) => row.map(v => i < 3 ? v * factor : v))
  validateAffine(affine)
  for (let j = 1; j <= 3; j++) image.hdr.pixDims[j] *= factor
  image.hdr.qform_code = 0
  image.hdr.sform_code = 2
  image.hdr.xyzt_units = (Number(image.hdr.xyzt_units) & ~7) | 2
  image.setAffine(affine)
}
