import { NVImage } from '@niivue/niivue'
import type { Loaded } from './appTypes'
import type { Mat4 } from './rigid'
import { resampleRegistration } from './registrationResampling'

export function makeResampledImage(source: Loaded, target: Loaded, matrix: Mat4, values?: Float32Array): NVImage {
  const out = values ?? resampleRegistration(source.raw, target.raw, matrix)
  const copy = new NVImage(); Object.assign(copy,target.nvImage)
  if (!target.nvImage.hdr) throw new Error('Reference header unavailable')
  const outputHdr = JSON.parse(JSON.stringify(target.nvImage.hdr)) as NonNullable<NVImage['hdr']>
  outputHdr.datatypeCode=16; outputHdr.numBitsPerVoxel=32; outputHdr.scl_slope=1; outputHdr.scl_inter=0; outputHdr.dims[0]=3; outputHdr.dims[4]=1; outputHdr.dims[5]=1; outputHdr.dims[6]=1; outputHdr.dims[7]=1
  outputHdr.qform_code=0; outputHdr.sform_code=2; outputHdr.affine=target.raw.affine.map(row => [...row])
  outputHdr.xyzt_units=2
  copy.hdr = outputHdr
  copy.img = out
  copy.name = `aligned_${source.name}`
  return copy
}

