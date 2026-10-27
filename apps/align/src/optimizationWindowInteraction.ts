import type { View, WindowBounds } from './appTypes'
import type { Vec3 } from './rigid'
import { createWindowBounds, isUsableWindowBounds } from './optimizationWindows'

export function windowBoundsForCompletedDrag(start: Vec3, lastValid: Vec3, current: Vec3 | null, plane: View['plane'], _pixDims: number[]): WindowBounds | null {
  const bounds = createWindowBounds(start, current ?? lastValid, plane)
  // Size is checked in CSS pixels by capture, not raw voxel spacings: voxel
  // axes can be oblique/swapped and zoom changes what a useful drag looks like.
  return isUsableWindowBounds(bounds, plane, 1e-6) ? bounds : null
}

export type OptimizationWindowInteractionOptions = {
  view: View
  isEnabled: () => boolean
  getPixDims: () => number[] | null
  eventToMm: (event: PointerEvent) => Vec3 | null
  onCommit: (bounds: WindowBounds) => void
  onReject: () => void
}

export function installOptimizationWindowCapture(options: OptimizationWindowInteractionOptions): () => void {
  const { view } = options
  let cancelCurrent:(()=>void)|undefined
  const onPointerDown = (event: PointerEvent) => {if(event.button!==0||event.isPrimary===false||!options.isEnabled())return;cancelCurrent?.();cancelCurrent=beginOptimizationWindowDrag(event, options)}
  view.windowLayer.addEventListener('pointerdown', onPointerDown)
  return () => {cancelCurrent?.();view.windowLayer.removeEventListener('pointerdown', onPointerDown)}
}

function beginOptimizationWindowDrag(event: PointerEvent, options: OptimizationWindowInteractionOptions) {
  const { view } = options
  if (!options.isEnabled()) return
  const start = options.eventToMm(event)
  if (!start) return
  event.preventDefault()
  event.stopPropagation()

  const preview = document.createElement('div')
  preview.className = 'optimization-window-box preview'
  view.windowLayer.replaceChildren(preview)

  let lastMm = start, lastX=event.clientX, lastY=event.clientY
  const pointerId = event.pointerId
  const layerRect = view.windowLayer.getBoundingClientRect()
  const sx = event.clientX - layerRect.left
  const sy = event.clientY - layerRect.top
  const controller = new AbortController()
  const listenerOptions = { capture: true, signal: controller.signal } as AddEventListenerOptions

  const update = (currentEvent: PointerEvent) => {
    if (currentEvent.pointerId !== pointerId) return
    currentEvent.preventDefault()
    const ex = currentEvent.clientX - layerRect.left
    const ey = currentEvent.clientY - layerRect.top
    preview.style.left = `${Math.min(sx, ex)}px`
    preview.style.top = `${Math.min(sy, ey)}px`
    preview.style.width = `${Math.abs(ex - sx)}px`
    preview.style.height = `${Math.abs(ey - sy)}px`
    const mm = options.eventToMm(currentEvent)
    if (mm) {lastMm = mm;lastX=currentEvent.clientX;lastY=currentEvent.clientY}
  }

  const complete = (currentEvent: PointerEvent) => {
    if (currentEvent.pointerId !== pointerId) return
    currentEvent.preventDefault()
    currentEvent.stopPropagation()
    controller.abort()
    const current=currentEvent.type==='pointercancel'?null:options.eventToMm(currentEvent)
    const ex=current?currentEvent.clientX:lastX,ey=current?currentEvent.clientY:lastY
    if(Math.abs(ex-event.clientX)<4||Math.abs(ey-event.clientY)<4){options.onReject();return}
    const bounds = windowBoundsForCompletedDrag(
      start,
      lastMm,
      current,
      view.plane,
      options.getPixDims() ?? [1, 1, 1],
    )
    if (!bounds) {
      options.onReject()
      return
    }
    options.onCommit(bounds)
  }

  // Document-level listeners avoid browser and WebGL-canvas differences in
  // pointer capture. A pointercancel commits the last valid image coordinate,
  // matching pointerup rather than silently discarding a visible drag preview.
  document.addEventListener('pointermove', update, listenerOptions)
  document.addEventListener('pointerup', complete, listenerOptions)
  document.addEventListener('pointercancel', complete, listenerOptions)
  update(event)
  return ()=>controller.abort()
}
