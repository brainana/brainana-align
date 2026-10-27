import type { Niivue } from '@niivue/niivue'

export const SLICE_ZOOM_LIMITS = { min: 0.1, max: 64 } as const

/** Pixel-distance zoom, rather than a fixed multiplier for each trackpad event.
 * Keep the current physical crosshair anchored and never mutate its position. */
export function zoomSliceView(nv: Niivue, deltaPixels: number): boolean {
  if (!Number.isFinite(deltaPixels) || deltaPixels === 0 || !nv.volumes.length) return false
  const pan = nv.scene.pan2Dxyzmm
  if (!Array.from(pan).every(Number.isFinite) || pan[3] <= 0) {
    nv.scene.pan2Dxyzmm = [0, 0, 0, 1] as any
    nv.drawScene()
    return true
  }
  const oldZoom = pan[3]
  const nextZoom = Math.min(SLICE_ZOOM_LIMITS.max, Math.max(SLICE_ZOOM_LIMITS.min, oldZoom * Math.exp(-Math.max(-240, Math.min(240, deltaPixels)) * 0.0015)))
  const mm = nv.frac2mm(nv.scene.crosshairPos)
  if (!Array.from(mm).every(Number.isFinite) || nextZoom === oldZoom) return false
  for (let axis = 0; axis < 3; axis++) pan[axis] += (oldZoom - nextZoom) * mm[axis]
  pan[3] = nextZoom
  if (nv.opts.yoke3Dto2DZoom) nv.scene.volScaleMultiplier = nextZoom
  nv.drawScene()
  return true
}
