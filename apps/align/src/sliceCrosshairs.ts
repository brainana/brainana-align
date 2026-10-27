import type { Niivue } from '@niivue/niivue'

/** Align uses orthogonal physical slices. Keep their crosshairs in screen pixels,
 * separate from Niivue's zoom-scaled 3D cylinders. At high zoom those cylinders
 * can cover the entire image. This adapter is instance-local; 3D drawing stays
 * with Niivue. Do not change image affines, textures or navigation to draw UI. */
export function installSliceCrosshairs(nv: Niivue): void {
  const original = nv.drawCrosshairs3D
  nv.drawCrosshairs3D = function (depthTest = true, alpha = 1, matrix = null, is2D = false, isSliceMM = true) {
    if (!is2D || !isSliceMM) return original.call(this, depthTest, alpha, matrix, is2D, isSliceMM)
    // Niivue calls a second, translucent pass for 3D cylinders behind the slice.
    // A screen-space crosshair needs exactly one pass.
    if (!depthTest || this.opts.crosshairWidth <= 0 || !this.volumes.length) return
    const tile = this.screenSlices.at(-1)
    if (!tile || tile.axCorSag < 0 || tile.axCorSag > 2) return
    const axes = [[0, 1], [0, 2], [1, 2]][tile.axCorSag]
    const mm = this.frac2mm(this.scene.crosshairPos, 0, true)
    const [left, top, width, height] = tile.leftTopWidthHeight
    const [fx, fy] = tile.fovMM
    if (![left, top, width, height, fx, fy, ...mm].every(Number.isFinite) || !width || height <= 0 || !fx || !fy) return
    const project = (x: number, y: number) => [left + (x - tile.leftTopMM[0]) / fx * width, top + height - (y - tile.leftTopMM[1]) / fy * height]
    const point = project(mm[axes[0]], mm[axes[1]])
    const [min, max] = this.sceneExtentsMinMax(true)
    const lo = project(min[axes[0]], min[axes[1]]), hi = project(max[axes[0]], max[axes[1]])
    const x0 = Math.max(0, Math.min(left, left + width), Math.min(lo[0], hi[0]))
    const x1 = Math.min(this.gl.canvas.width, Math.max(left, left + width), Math.max(lo[0], hi[0]))
    const y0 = Math.max(0, top, Math.min(lo[1], hi[1]))
    const y1 = Math.min(this.gl.canvas.height, top + height, Math.max(lo[1], hi[1]))
    if (![...point, x0, x1, y0, y1].every(Number.isFinite) || x1 <= x0 || y1 <= y0) return
    const rect = (this.gl.canvas as HTMLCanvasElement).getBoundingClientRect()
    const pixelRatio = rect.width > 0 ? this.gl.canvas.width / rect.width : 1
    const thickness = Math.min(5, Math.max(1, Number(this.opts.crosshairWidth) || 1)) * pixelRatio
    const color = [...this.opts.crosshairColor]
    color[3] *= alpha
    const gl = this.gl, depth = gl.isEnabled(gl.DEPTH_TEST), blend = gl.isEnabled(gl.BLEND)
    const mask = gl.getParameter(gl.DEPTH_WRITEMASK)
    const factors = [gl.BLEND_SRC_RGB, gl.BLEND_DST_RGB, gl.BLEND_SRC_ALPHA, gl.BLEND_DST_ALPHA].map(p => gl.getParameter(p))
    gl.disable(gl.DEPTH_TEST); gl.depthMask(false); gl.enable(gl.BLEND); gl.blendFunc(gl.SRC_ALPHA, gl.ONE_MINUS_SRC_ALPHA)
    try {
      if (point[1] >= y0 && point[1] <= y1) this.drawLine([x0, point[1], x1, point[1]], thickness, color)
      if (point[0] >= x0 && point[0] <= x1) this.drawLine([point[0], y0, point[0], y1], thickness, color)
    } finally {
      if (depth) gl.enable(gl.DEPTH_TEST); else gl.disable(gl.DEPTH_TEST)
      if (blend) gl.enable(gl.BLEND); else gl.disable(gl.BLEND)
      gl.depthMask(mask); gl.blendFuncSeparate(factors[0], factors[1], factors[2], factors[3])
    }
  }
}
