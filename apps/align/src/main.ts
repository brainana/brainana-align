import {installAcquisitionUI} from './acquisitionUI'
import {installSliceCrosshairs} from './sliceCrosshairs'
import {installItkRegistration} from './itkRegistrationUI'
import {manualPreviewDelta,transformedPreviewImage} from './manualPreview'
import {createOperationState} from './operationState'
import {createOverlayScheduler} from './overlayScheduler'
import {runOverlayTask} from './overlayTask'
import {replaceReviewVolumes} from './reviewVolumes'
import {installLandmarkSensitivityReview} from './landmarkQuality'
import {cloneFitState} from './fitSnapshot'
import {installJipRegistration} from './jipRegistrationUI'
import {installLandmarkList} from './landmarkList'
import {installWelcomeScreen} from './welcomeScreen'
import {imageLabel,imageKey,getRegistrationPairing,imagingText} from './registrationPairing'
import {resetInitialAlignment,rejectLegacyRefinement} from './resetInitialAlignment'
import {installOpacityCycle} from './opacityCycle'
import {installAlignedExportOptions,readAlignedExportOptions} from './alignedExportOptions'
import {installManualAdjustment} from './manualAdjustmentUI'
import {installManualSourcePreview,type SourcePreviewState} from './manualSourcePreview'
import {registrationModalities,registrationPoint} from './manualAffine'
import {installGeneratedImageExports} from './generatedImageExports'
import {finishWorkingImageCommit,sourceViewerNeedsReload} from './workingImageCommit'
import {installThresholdMasks} from './thresholdMaskUI'
import {installDisplayIntensity} from './displayIntensity'
import {processingViewPoint} from './imageProcessing'
import {installInitialProcessing} from './initialProcessingUI'
import {installImagePreparation} from './imagePreparation'
import {sharedViewerImage,nativeGeometry} from './volumeImages'
import {saveNativeAlignedImage} from './nativeExport'
import {hasLandmarkBasis,isLocalRegistration,installSharedRegionControls} from './registrationWorkflow'
import {displacementReferenceGrid} from '../../../packages/imaging-math/splineFields'
import {createPointMapping, type fitNonlinearSpline} from '../../../packages/imaging-math/nonlinearSpline'
import {runNonlinearTask} from './nonlinearTasks'
import {windowPointFromCanvas,windowRectangleOnCanvas} from './optimizationWindowGeometry'
import {installWorkspaceReset} from './workspaceReset'
import { applyDisplayPreferences } from './displayPreferences'
import { initializePreferences, savePreferences, getPreferences, type Preferences } from './preferences'
import { installSettings, rememberDataset } from './settingsUI'
import { installAutomaticRegistration } from './automaticRegistrationUI'
import { configureReviewAppearance, updateReviewFlip, updateReviewOpacity } from './reviewAppearance'
import { createAlignmentActivity, installWorkflowFocus, revealControl } from './workflowUI'
import { alignmentLayout } from './alignmentLayout'
import {installExportDialog} from './exportDialog'
import { installGuidedSetup } from './guidedSetup'
import { imageGeometryAlignment } from './geometryInitialization'
import { fitLandmarks, LANDMARK_MODELS,  } from './landmarkFit'
import { fitSummaryHtml, confirmReflectedTransform } from './fitReview'
import { makeResampledImage } from './outputImage'
import './style.css'
import { Niivue, NVImage, SLICE_TYPE } from '@niivue/niivue'
import type { RegistrationModel, FitState, Landmark, Loaded, Modality, OptimizationWindows, Plane, ReviewView, View, WindowConstraint } from './appTypes'
import { gzipSync } from 'fflate'
import { applyAffine, invertAffine } from './roiWarp'
import { installRuntimeIntegration } from './runtimeIntegration'
import { type Mat4, type Vec3 } from './rigid'
import { countOptimizationWindows, registrationWindows, optimizationConstraint as getOptimizationConstraint, sanitizeOptimizationWindows } from './optimizationWindows'
import { installOptimizationWindowCapture } from './optimizationWindowInteraction'
import { installHoverKeyboardPan, installProjectionRefresh, observeProjectionDraw, installWheelZoomAndSlice, isTypingTarget, panViewInScreenDirection, type HoveredView } from './viewInteraction'
import { canvasPointToImageOnCurrentSlice, coordinateLabel, isFiniteVec3, planeDepthAxis } from './coordinateProjection'
import { setOrthogonalCrosshairs, setReviewCrosshairs } from './crosshairController'
import { renderModalityMarkers, renderReviewLandmarks } from './landmarkRenderer'
import { VERSION as APP_VERSION } from './version'
import { createSessionPayload, parseSessionPayload, sessionGeometryMismatches } from './sessionPersistence'
import { createRegistrationArtifacts, saveArtifact, saveArtifacts } from './exportArtifacts'
import { detectBrowserCapabilities } from './browserCapabilities'
import { installBrowserCompatibilityBanner } from './browserCompatibility'

const planeTypes: Record<Plane, SLICE_TYPE> = {
  sagittal: SLICE_TYPE.SAGITTAL,
  coronal: SLICE_TYPE.CORONAL,
  axial: SLICE_TYPE.AXIAL,
}

const app = document.querySelector<HTMLDivElement>('#app')!
app.innerHTML = alignmentLayout(APP_VERSION)
installWorkflowFocus()
const operations=createOperationState(),reviewScheduler=createOverlayScheduler(operations)
const activity=createAlignmentActivity(operations,()=>reviewScheduler.invalidate())
document.querySelector('#alignment-cancel')!.addEventListener('click',()=>reviewScheduler.cancel())
window.addEventListener('pagehide',()=>reviewScheduler.invalidate(),{once:true})
let jipUI:ReturnType<typeof installJipRegistration>|undefined
let itkUI:ReturnType<typeof installItkRegistration>|undefined
let acquisitionUI:ReturnType<typeof installAcquisitionUI>|undefined
let exportUI:ReturnType<typeof installExportDialog>|undefined

const statusEl = document.querySelector<HTMLSpanElement>('#status')!
const fitSummary = document.querySelector<HTMLDivElement>('#fit-summary')!
const directionSelect = document.querySelector<HTMLSelectElement>('#direction')!
const linkAfter = document.querySelector<HTMLInputElement>('#link-after')!
const saveAligned = document.querySelector<HTMLButtonElement>('#save-aligned')!
const saveTransform = document.querySelector<HTMLButtonElement>('#save-transform')!
const reviewMode = document.querySelector<HTMLSelectElement>('#review-mode')!
const reviewOpacity = document.querySelector<HTMLInputElement>('#review-opacity')!
const opacityValue = document.querySelector<HTMLSpanElement>('#opacity-value')!
const resetManual = document.querySelector<HTMLButtonElement>('#reset-manual')!
const resetLandmark = document.querySelector<HTMLButtonElement>('#reset-landmark')!
const refineButton = document.querySelector<HTMLButtonElement>('#refine')!
const refineBounds = document.querySelector<HTMLSelectElement>('#refine-bounds')!
const refineTranslationLimit = document.querySelector<HTMLInputElement>('#refine-translation-limit')!
const refineRotationLimit = document.querySelector<HTMLInputElement>('#refine-rotation-limit')!
const refineTranslationStartStep = document.querySelector<HTMLInputElement>('#refine-translation-step')!
const refineRotationStartStep = document.querySelector<HTMLInputElement>('#refine-rotation-step')!
const refineSummary = document.querySelector<HTMLDivElement>('#refine-summary')!
const refineActions = document.querySelector<HTMLDivElement>('#refine-actions')!
const acceptRefine = document.querySelector<HTMLButtonElement>('#accept-refine')!
const rejectRefine = document.querySelector<HTMLButtonElement>('#reject-refine')!
const restoreAlignmentLandmarks = document.querySelector<HTMLButtonElement>('#restore-alignment-landmarks')!
const defineWindows = document.querySelector<HTMLButtonElement>('#define-windows')!
const clearWindows = document.querySelector<HTMLButtonElement>('#clear-windows')!
const windowTarget = document.querySelector<HTMLSelectElement>('#window-target')!
const windowSummary = document.querySelector<HTMLParagraphElement>('#window-summary')!

const views: Record<Modality, Record<Plane, View>> = { mri: {} as Record<Plane, View>, ct: {} as Record<Plane, View> }
const reviewViews = {} as Record<Plane, ReviewView>
const loaded: Record<Modality, Loaded | null> = { mri: null, ct: null }
const currentMm: Record<Modality, Vec3 | null> = { mri: null, ct: null }
let landmarks: Landmark[] = []
let selectedId: number | null = null
let nextId = 1
let fitResult: FitState | null = null
installLandmarkSensitivityReview(fitSummary,()=>fitResult)
let history: Landmark[][] = []
let suppressLinkedUpdate = false
let suppressReviewUpdate = false
const suppressCrosshairSync: Record<Modality, boolean> = { mri: false, ct: false }
type ReviewImages={fixed:NVImage;moving:NVImage;fixedModality:Modality;movingModality:Modality;fit:FitState}
let reviewImages:ReviewImages|null=null
let manualPreviewBase:ReviewImages|null=null
let manualPreviewFrame:number|null=null
let manualUI:ReturnType<typeof installManualAdjustment>|undefined
let manualReviewTarget:Modality|null=null
let sourcePreview:SourcePreviewState|null=null
const sourcePreviewController=installManualSourcePreview({loaded:m=>loaded[m],views:m=>Object.values(views[m]),current:m=>currentMm[m],gotoNative:gotoMm,state:next=>{sourcePreview=next},beforeShow:()=>{definingWindows=false;initialProcessing?.stop()},afterShow:()=>displayIntensity?.apply()})
function sourceDisplayPoint(modality:Modality,mm:Vec3):Vec3{return sourcePreview?.modality===modality?registrationPoint(sourcePreview.fit,modality,sourcePreview.reference,mm):mm}
function sourceNativePoint(modality:Modality,mm:Vec3):Vec3{return sourcePreview?.modality===modality?registrationPoint(sourcePreview.fit,sourcePreview.reference,modality,mm):mm}
function clearSourcePreview(){sourcePreviewController.clear()}
function updateSourcePreview(){if(!manualUI?.preview||!reviewImages||!fitResult){clearSourcePreview();return}sourcePreviewController.show(reviewImages.movingModality,reviewImages.fixedModality,reviewImages.moving,reviewImages.fit)}

let optimizationWindows: OptimizationWindows = { mri: {}, ct: {} }
let initialProcessing:ReturnType<typeof installInitialProcessing>|undefined
let definingWindows = false

function setStatus(text: string, error = false) { statusEl.textContent = text; statusEl.classList.toggle('error', error);exportUI?.report(text,error) }

function setImagePlaceholder(modality: Modality, mode: 'empty' | 'loading' | 'hidden', detail?: string) {
  const label = imageLabel(modality)
  for (const plane of ['sagittal','coronal','axial'] as Plane[]) {
    const placeholder = document.querySelector<HTMLDivElement>(`#${modality}-${plane}-placeholder`)!
    placeholder.classList.toggle('hidden', mode === 'hidden')
    placeholder.classList.toggle('loading', mode === 'loading')
    if (mode === 'empty') placeholder.textContent = `No ${label} selected`
    else if (mode === 'loading') placeholder.textContent = detail ? `Loading ${label}: ${detail}` : `Loading ${label}…`
  }
}
function snapshot() { history.push(structuredClone(landmarks)); if (history.length > 40) history.shift() }

let hoveredView: HoveredView | null = null

document.addEventListener('keydown', event => {
  if (!hoveredView || isTypingTarget(event.target)) return
  const directions: Record<string, [number, number]> = {
    ArrowLeft: [-1, 0], ArrowRight: [1, 0], ArrowUp: [0, -1], ArrowDown: [0, 1]
  }
  const direction = directions[event.key]
  if (!direction) return
  event.preventDefault()
  const step = event.shiftKey ? 36 : 12
  panViewInScreenDirection(hoveredView, direction[0] * step, direction[1] * step)
})

function setupViews() {
  for (const modality of ['mri','ct'] as Modality[]) {
    for (const plane of ['sagittal','coronal','axial'] as Plane[]) {
      const canvas = document.querySelector<HTMLCanvasElement>(`#${modality}-${plane}`)!
      const overlay = document.querySelector<SVGSVGElement>(`#${modality}-${plane}-overlay`)!
      const windowLayer = document.querySelector<HTMLDivElement>(`#${modality}-${plane}-window-layer`)!
      const nv = new Niivue({ show3Dcrosshair: false, isColorbar: false })
      installSliceCrosshairs(nv)
      views[modality][plane] = { nv, modality, plane, canvas, overlay, windowLayer }
      nv.attachToCanvas(canvas)
      nv.setSliceMM(true)
      nv.setDragMode('pan')
      nv.setSliceType(planeTypes[plane])
      nv.setCrosshairColor(modality === 'mri' ? [0.25,0.85,1,1] : [1,0.7,0.15,1])
      nv.onLocationChange = (loc: any) => handleLocation(modality, loc?.mm, plane)
      installWheelZoomAndSlice(canvas, nv, () => sourcePreview?.modality===modality?loaded[sourcePreview.reference]?.raw.dims??null:loaded[modality]?.raw.dims ?? null, plane, mm => gotoMm(modality, sourceNativePoint(modality,mm)))
      installHoverKeyboardPan(canvas, nv, () => renderMarkers(modality), view => { hoveredView = view })
      installProjectionRefresh(canvas, () => renderMarkers(modality))
      observeProjectionDraw(nv, () => renderMarkers(modality))
      installWindowCapture(views[modality][plane]);initialProcessing?.attach(views[modality][plane]);thresholdMasks?.attach(views[modality][plane])
      window.addEventListener('resize', () => renderMarkers(modality))
    }
  }
}

function setupReviewViews() {
  for (const plane of ['sagittal','coronal','axial'] as Plane[]) {
    const canvas = document.querySelector<HTMLCanvasElement>(`#review-${plane}`)!
    const overlay = document.querySelector<SVGSVGElement>(`#review-${plane}-overlay`)!
    const nv = new Niivue({ show3Dcrosshair: false, isColorbar: false })
    installSliceCrosshairs(nv)
    nv.addColormap('brainanaMagenta', { R: [0, 128, 255], G: [0, 0, 0], B: [0, 128, 255], A: [0, 64, 128], I: [0, 128, 255] })
    reviewViews[plane] = { nv, plane, canvas, overlay }
    nv.attachToCanvas(canvas)
    nv.setSliceMM(true)
    nv.setDragMode('pan')
    nv.setSliceType(planeTypes[plane])
    nv.setCrosshairColor([1,1,1,0.85])
    nv.onLocationChange = (loc: any) => {
      if (suppressReviewUpdate || !reviewImages || !loc?.mm) return
      const mm: Vec3 = [loc.mm[0], loc.mm[1], loc.mm[2]]
      // Overlay coordinates are in the fixed-image world. Update the fixed image,
      // map the same location back into the original moving image, and then redraw
      // every overlay plane. This keeps MRI, CT, and overlay navigation coherent.
      suppressReviewUpdate = true
      suppressLinkedUpdate = true
      gotoMm(reviewImages.fixedModality, mm)
      if (fitResult) {
        const movingMm = registrationPoint(reviewImages.fit,reviewImages.fixedModality,reviewImages.movingModality,mm)
        gotoMm(reviewImages.movingModality, movingMm)
      }
      for (const reviewView of Object.values(reviewViews)) {
        if (!reviewView.nv.volumes.length) continue
        reviewView.nv.scene.crosshairPos = reviewView.nv.mm2frac(mm)
        reviewView.nv.drawScene()
      }
      renderReviewMarkers()
      requestAnimationFrame(() => {
        suppressLinkedUpdate = false
        suppressReviewUpdate = false
      })
    }
    installWheelZoomAndSlice(canvas, nv, () => {
      if (!reviewImages) return null
      const fixed = loaded[reviewImages.fixedModality]
      return fixed?.raw.dims ?? null
    }, plane, mm => {
      if (!reviewImages) return
      gotoMm(reviewImages.fixedModality, mm)
    })
    installHoverKeyboardPan(canvas, nv, renderReviewMarkers, view => { hoveredView = view })
    installProjectionRefresh(canvas, renderReviewMarkers)
    observeProjectionDraw(nv, renderReviewMarkers)
    window.addEventListener('resize', renderReviewMarkers)
  }
}

const supportedExtensions = [
  '.nii', '.nii.gz', '.hdr', '.img', '.img.gz', '.head', '.brik', '.brik.gz',
  '.mgh', '.mgz', '.nrrd', '.nhdr', '.mif', '.mha', '.mhd', '.raw',
  '.v', '.v16', '.vmr', '.npy', '.npz', '.fib', '.src',
]

function matchesSupportedExtension(name: string): boolean {
  const lower = name.toLowerCase()
  return supportedExtensions.some(ext => lower.endsWith(ext))
}


let pairingUI:ReturnType<typeof installWelcomeScreen>|undefined
let preparation:ReturnType<typeof installImagePreparation>
async function loadFiles(modality:Modality,files:File[],fromDisk=false){if(!files.length)return;if(!files.some(file=>matchesSupportedExtension(file.name)))throw Error('Unsupported image format');await preparation.select(modality,files,fromDisk)}
async function commitPrepared(items:Record<Modality,Loaded>,changed:Modality[],reason:'preparation'|'processing'='preparation'){
 if(sourcePreview)manualUI?.stopPreview()
 const old={...loaded},oldFit=fitResult,oldWindows=structuredClone(optimizationWindows),oldMm=structuredClone(currentMm),oldLandmarks=structuredClone(landmarks)
 try{
  for(const modality of ['mri','ct'] as Modality[]){if(items[modality]===old[modality])continue;loaded[modality]=items[modality];if(sourceViewerNeedsReload(old[modality],items[modality]))for(const view of Object.values(views[modality])){while(view.nv.volumes.length)view.nv.removeVolumeByIndex(0);view.nv.addVolume(sharedViewerImage(items[modality].nvImage))}if(changed.includes(modality)){optimizationWindows[modality]={};currentMm[modality]=applyAffine(items[modality].raw.affine,items[modality].raw.dims.map(n=>(n-1)/2) as Vec3)}if(old[modality]&&currentMm[modality]&&(old[modality]!.processing||items[modality].processing))currentMm[modality]=processingViewPoint(old[modality]!.raw,items[modality].raw,currentMm[modality]!,items[modality].processing?.recipe.crop??null,items[modality].processing?.focus??applyAffine(items[modality].raw.affine,items[modality].raw.dims.map(n=>(n-1)/2) as Vec3));gotoMm(modality,currentMm[modality]!);setImagePlaceholder(modality,'hidden');document.querySelector(`#${modality}-name`)!.textContent=items[modality].name;document.querySelector<HTMLElement>(`#${modality}-name`)!.closest('label')!.title=`Current ${imageLabel(modality)}: ${items[modality].name}`;void rememberDataset(modality,items[modality].name).catch(()=>{})}
  await finishWorkingImageCommit(reason,changed,Boolean(fitResult)&&(['mri','ct'] as Modality[]).some(m=>old[m]?.raw!==items[m].raw),{clearFit,refreshOverlay:()=>buildReview(true),invalidateOverlay:invalidateProcessedOverlay});updateWindowControls();renderAll()
 }catch(error){loaded.mri=old.mri;loaded.ct=old.ct;fitResult=oldFit;landmarks=oldLandmarks;if(oldFit)fitSummary.innerHTML=fitSummaryHtml(oldFit);optimizationWindows=oldWindows;currentMm.mri=oldMm.mri;currentMm.ct=oldMm.ct;for(const m of ['mri','ct'] as Modality[])for(const view of Object.values(views[m])){while(view.nv.volumes.length)view.nv.removeVolumeByIndex(0);if(old[m])view.nv.addVolume(sharedViewerImage(old[m]!.nvImage))}for(const m of ['mri','ct'] as Modality[]){document.querySelector(`#${m}-name`)!.textContent=old[m]?.name??`Select ${imageLabel(m)}`;document.querySelector<HTMLElement>(`#${m}-name`)!.closest('label')!.title=old[m]?`Current ${imageLabel(m)}: ${old[m]!.name}`:`Select ${imageLabel(m)}`;setImagePlaceholder(m,old[m]?'hidden':'empty');if(old[m]&&oldMm[m])gotoMm(m,oldMm[m]!)}updateWindowControls();if(oldFit){if(reason==='processing')invalidateProcessedOverlay();else await buildReview(true).catch(()=>clearReview())}renderAll();throw error}
}

function handleLocation(modality: Modality, mm: number[] | undefined, sourcePlane?: Plane) {
  if (!isFiniteVec3(mm)) return
  const nextMm: Vec3 = sourceNativePoint(modality,[mm[0], mm[1], mm[2]])
  currentMm[modality] = nextMm

  if (!suppressCrosshairSync[modality]) {
    suppressCrosshairSync[modality] = true
    setOrthogonalCrosshairs(modality, views[modality], sourceDisplayPoint(modality,nextMm), sourcePlane)
    requestAnimationFrame(() => { suppressCrosshairSync[modality] = false })
  } else {
    document.querySelector(`#${modality}-coords`)!.textContent = coordinateLabel(sourceDisplayPoint(modality,nextMm))
  }

  renderMarkers(modality)
  if (reviewImages && modality === reviewImages.fixedModality && !suppressReviewUpdate) gotoReviewMm(nextMm)
  if (fitResult && linkAfter.checked && !suppressLinkedUpdate) {
    const other: Modality = modality === 'mri' ? 'ct' : 'mri'
    if (!loaded[other]) return
    const mapper=createPointMapping(fitResult.matrix,fitResult.nonlinear),isMoving=(fitResult.direction==='ct-mri'?modality==='ct':modality==='mri')
    const mapped=isMoving?mapper.forward(nextMm):mapper.inverse(nextMm)
    suppressLinkedUpdate = true
    gotoMm(other, mapped)
    requestAnimationFrame(() => { suppressLinkedUpdate = false })
  }
}

function gotoMm(modality: Modality, mm: Vec3) {
  currentMm[modality] = mm
  setOrthogonalCrosshairs(modality, views[modality], sourceDisplayPoint(modality,mm))
  renderMarkers(modality)
  if (reviewImages && modality === reviewImages.fixedModality && !suppressReviewUpdate) gotoReviewMm(mm)
}

function gotoReviewMm(mm: Vec3) {
  suppressReviewUpdate = true
  setReviewCrosshairs(reviewViews, mm)
  renderReviewMarkers()
  requestAnimationFrame(() => { suppressReviewUpdate = false })
}

function renderMarkers(modality: Modality) {
  renderModalityMarkers({
    views: views[modality],
    loaded: sourcePreview?.modality===modality?loaded[sourcePreview.reference]:loaded[modality],
    currentMm: currentMm[modality]?sourceDisplayPoint(modality,currentMm[modality]!):null,
    landmarks: sourcePreview?.modality===modality?landmarks.map(l=>({...l,[modality]:l[modality]?sourceDisplayPoint(modality,l[modality]!):null})):landmarks,
    selectedId,
    fitResult,
    renderOptimizationWindow,
    onMarkerPointerDown: startMarkerDrag,
  })
}

function reviewPointFor(lm: Landmark, modality: Modality): Vec3 | null {
  if (!fitResult) return null
  const point = lm[modality]
  if (!point) return null
  return reviewImages?registrationPoint(reviewImages.fit,modality,reviewImages.fixedModality,point):point
}

function renderReviewMarkers() {
  renderReviewLandmarks({
    views: reviewViews,
    fixedLoaded: reviewImages ? loaded[reviewImages.fixedModality] : null,
    fixedCurrentMm: reviewImages ? currentMm[reviewImages.fixedModality] : null,
    fitResult,
    pointFor: reviewPointFor,
    fixedModality: reviewImages?.fixedModality ?? null,
    movingModality: reviewImages?.movingModality ?? null,
  })
}

function startMarkerDrag(ev: PointerEvent, view: View, id: number) {
  if(sourcePreview?.modality===view.modality){ev.preventDefault();return setStatus('Turn off transformed source-row preview to drag native landmarks.',true)}
  ev.preventDefault(); ev.stopPropagation()
  selectedId = id; snapshot(); renderAll()
  const move = (e: PointerEvent) => {
    const rect = view.canvas.getBoundingClientRect()
    const x = (e.clientX - rect.left) * (view.canvas.width / rect.width)
    const y = (e.clientY - rect.top) * (view.canvas.height / rect.height)
    const mm = canvasPointToImageOnCurrentSlice(view, x, y)
    if (!mm) return
    const lm = landmarks.find(v => v.id === id); if (!lm) return
    const old = lm[view.modality]; if (!old) return
    const axis = planeDepthAxis(view.plane)
    mm[axis] = old[axis]
    lm[view.modality] = mm
    markLandmarksChanged(); renderAll()
  }
  const up = () => { window.removeEventListener('pointermove',move); window.removeEventListener('pointerup',up); renderLandmarks() }
  window.addEventListener('pointermove',move)
  window.addEventListener('pointerup',up)
}

let generatedExports:ReturnType<typeof installGeneratedImageExports>|undefined
let thresholdMasks:ReturnType<typeof installThresholdMasks>|undefined
let displayIntensity:ReturnType<typeof installDisplayIntensity>|undefined
let automaticUI:ReturnType<typeof installAutomaticRegistration>|undefined
let renderAllFrame: number | null = null
function renderAllNow() {
  renderAllFrame = null
  pairingUI?.refresh()
  automaticUI?.refresh();jipUI?.refresh();itkUI?.refresh();acquisitionUI?.refresh()
  manualUI?.refresh()
  initialProcessing?.refresh()
  displayIntensity?.refresh()
  thresholdMasks?.refresh()
  generatedExports?.refresh()
  document.querySelector<HTMLButtonElement>('#export-open')!.disabled=!loaded.mri&&!loaded.ct
  renderMarkers('mri')
  renderMarkers('ct')
  renderReviewMarkers()
  renderLandmarks()
  exportUI?.refresh()
}
function renderAll() {
  operations.dispatch({type:'proposal/project',pending:Boolean(fitResult?.proposal)})
  if (renderAllFrame !== null) return
  renderAllFrame = requestAnimationFrame(renderAllNow)
}

const renderLandmarks=installLandmarkList({
 getLandmarks:()=>landmarks,getSelected:()=>selectedId,
 select:id=>{const lm=landmarks.find(l=>l.id===id);if(!lm)return;selectedId=id;if(lm.mri)gotoMm('mri',lm.mri);if(lm.ct)gotoMm('ct',lm.ct);renderAll()},
 toggle:(id,enabled)=>{const lm=landmarks.find(l=>l.id===id);if(!lm)return;snapshot();lm.enabled=enabled;markLandmarksChanged();renderAll()},
 remove:id=>{snapshot();landmarks=landmarks.filter(l=>l.id!==id);if(selectedId===id)selectedId=null;markLandmarksChanged();renderAll()},
 setPoint:(id,modality)=>{const lm=landmarks.find(l=>l.id===id);if(!lm)return;if(!loaded[modality]||!currentMm[modality])return setStatus(`Load ${imageLabel(modality)} first.`,true);snapshot();lm[modality]=[...currentMm[modality]!] as Vec3;selectedId=id;markLandmarksChanged();renderAll()},
 go:(id,modality)=>{const lm=landmarks.find(l=>l.id===id);if(!lm)return;if(lm[modality])gotoMm(modality,lm[modality]!);selectedId=id;renderAll()},
})

function markLandmarksChanged() {
  for (const lm of landmarks) delete lm.residual
  if (!fitResult) return
  fitResult.landmarksChanged = true
  fitSummary.innerHTML = fitSummaryHtml(fitResult)
  restoreAlignmentLandmarks.disabled = false
  setStatus('Landmarks changed. The accepted alignment is still active and exportable.')
}

function clearFit() {
  operations.dispatch({type:'proposal/clear'})
  manualUI?.stopPreview();clearSourcePreview();manualReviewTarget=null
  fitResult = null
  for (const lm of landmarks) delete lm.residual
  fitSummary.textContent = 'At least three complete, enabled landmark pairs are required.'
  saveAligned.disabled = true; saveTransform.disabled = true; document.querySelector<HTMLButtonElement>('#export-open')!.disabled = !loaded.mri&&!loaded.ct; 
  resetManual.disabled = true; resetLandmark.disabled = true; refineButton.disabled = true; restoreAlignmentLandmarks.disabled = true
  refineSummary.textContent = 'Load both images to run automatic registration without landmarks, or refine an accepted starting alignment.'
  refineActions.classList.add('hidden')
  clearReview(); updateNudgeOutputs();automaticUI?.refresh();jipUI?.refresh();itkUI?.refresh();acquisitionUI?.refresh()
}

function clearReview() {
  reviewScheduler.invalidate()
  manualPreviewBase=null
  if(manualPreviewFrame!==null){cancelAnimationFrame(manualPreviewFrame);manualPreviewFrame=null}
  if(typeof opacityCycle!=='undefined')opacityCycle.stop()
  clearSourcePreview()
  reviewImages = null
  if(typeof opacityCycle!=='undefined')opacityCycle.refresh()
  document.querySelector('#refresh-processed-overlay')!.classList.add('hidden')
  document.querySelectorAll('.review-placeholder').forEach(el=>el.textContent='Fit a transform to display the aligned overlay')
  document.querySelector('#review-label')!.textContent = 'Fit a transform to review alignment'
  document.querySelectorAll('.review-placeholder').forEach(el => el.classList.remove('hidden'))
  for (const view of Object.values(reviewViews)) {
    while (view.nv.volumes.length) view.nv.removeVolumeByIndex(0)
    view.overlay.innerHTML = ''
  }
}

function invalidateProcessedOverlay(){
 clearReview()
 document.querySelector('#review-label')!.textContent='Working images updated; existing transform unchanged'
 document.querySelector('#refresh-processed-overlay')!.classList.remove('hidden')
 document.querySelectorAll('.review-placeholder').forEach(el=>el.textContent='Working images updated. Refresh the existing overlay when ready; no registration has run.')
}


function applyReviewAppearance() {
  opacityValue.textContent = `${reviewOpacity.value}%`
  if(reviewImages)configureReviewAppearance(Object.values(reviewViews).map(v=>v.nv),reviewMode.value,Number(reviewOpacity.value)/100)
  if(typeof opacityCycle!=='undefined')opacityCycle.refresh()
}
let opacityFrame:number|null=null
function scheduleReviewOpacity() {
  opacityValue.textContent=`${reviewOpacity.value}%`
  if(opacityFrame!==null)return
  opacityFrame=requestAnimationFrame(()=>{opacityFrame=null;if(document.querySelector<HTMLSelectElement>('#review-comparison')!.value==='flip'){opacityCycle.refresh();return}updateReviewOpacity(Object.values(reviewViews).map(v=>v.nv),Number(reviewOpacity.value)/100)})
}

async function buildReview(allowBusy=false,interactive=false) {
  if(activity.busy&&!allowBusy)return
  if (!fitResult) return
  const movingModality: Modality = manualReviewTarget??registrationModalities(fitResult.direction).moving
  const fixedModality: Modality = movingModality==='ct'?'mri':'ct'
  const reverse=movingModality!==registrationModalities(fitResult.direction).moving
  const moving = loaded[movingModality]
  const fixed = loaded[fixedModality]
  if (!moving || !fixed) return
  const originalFit=fitResult,candidate=cloneFitState(fitResult),ownActivity=!activity.busy&&!interactive
  if(ownActivity&&!await activity.begin('Preparing transformed overlay…','display'))return
  const activityId=activity.id,blocking=activity.busy,cancel=document.querySelector('#alignment-cancel')!,cancelWasHidden=cancel.classList.contains('hidden')
  if(blocking)cancel.classList.remove('hidden')
  let aligned:NVImage,outcome:'completed'|'cancelled'|'failed'='completed'
  try{
    if(blocking&&activity.state.activity.status==='cancelling'){outcome='cancelled';throw Error('Overlay refresh cancelled. Existing alignment and display retained.')}
    const task=await reviewScheduler.run(signal=>runOverlayTask({source:moving.raw,target:fixed.raw,fit:candidate,reverse},signal,text=>{if(blocking)void activity.phase(text,undefined,activityId)}))
    if(task.status==='stale')return
    if(task.status==='cancelled'){outcome='cancelled';throw Error('Overlay refresh cancelled. Existing alignment and display retained.')}
    if(task.status==='failed'){outcome='failed';throw task.error}
    if(blocking&&(activity.id!==activityId||activity.state.activity.status==='cancelling')){outcome='cancelled';throw Error('Overlay refresh cancelled. Existing alignment and display retained.')}
    if(!reviewScheduler.isCurrent(task.generation)||fitResult!==originalFit||loaded[movingModality]!==moving||loaded[fixedModality]!==fixed)return
    aligned=makeResampledImage(moving,fixed,reverse?invertAffine(candidate.matrix):candidate.matrix,task.result)
  }catch(error){if(outcome==='completed')outcome='failed';document.querySelector('#refresh-processed-overlay')!.classList.remove('hidden');document.querySelector('#review-label')!.textContent=`Overlay refresh ${outcome==='cancelled'?'cancelled':'failed'}${reviewImages?'; showing the previous display':''}`;setStatus(error instanceof Error?error.message:String(error),true);if(allowBusy)throw error;return}finally{if(blocking&&cancelWasHidden&&activity.id===activityId)cancel.classList.add('hidden');if(ownActivity){activity.end(outcome,activityId);manualUI?.refresh();automaticUI?.refresh();jipUI?.refresh();itkUI?.refresh();acquisitionUI?.refresh()}}
  document.querySelector('#refresh-processed-overlay')!.classList.add('hidden')
  const previousFixed=reviewImages?.fixed??null
  reviewImages = { fixed: fixed.nvImage, moving: aligned, fixedModality, movingModality,fit:candidate }
  for (const view of Object.values(reviewViews)) {
    replaceReviewVolumes(view.nv,fixed.nvImage,aligned,previousFixed)
  }
  manualPreviewBase=reviewImages
  document.querySelector('#review-label')!.textContent = `${imageLabel(movingModality)} aligned to ${imageLabel(fixedModality)}${reverse?' · export '+(fitResult.direction==='ct-mri'?imageLabel('ct')+' → '+imageLabel('mri'):imageLabel('mri')+' → '+imageLabel('ct')):''}${fitResult.landmarksChanged ? ' · using saved landmark snapshot' : ''}`
  document.querySelectorAll('.review-placeholder').forEach(el => el.classList.add('hidden'))
  displayIntensity?.apply()
  applyReviewAppearance()
  updateSourcePreview()
  gotoReviewMm(currentMm[fixedModality] ?? applyAffine(fixed.raw.affine,[(fixed.raw.dims[0]-1)/2,(fixed.raw.dims[1]-1)/2,(fixed.raw.dims[2]-1)/2]))
}

async function runFit() {
  if(activity.busy)return
  if(fitResult?.proposal)return setStatus('Accept or reject the current registration proposal first.',true)
  const complete = landmarks.filter(l=>l.enabled&&l.mri&&l.ct)
  const model = document.querySelector<HTMLSelectElement>('#landmark-transform')!.value as RegistrationModel
  const minimum=model==='spline'?12:['affine','anisotropic'].includes(model)?4:3
  if(complete.length<minimum)return setStatus(`At least ${minimum} complete, enabled landmark pairs are required for this model.`,true)
  if(model==='jip')return setStatus('JIP belongs to Image-based registration.',true)
  if(model==='spline'){await runSplineFit();return}
  const direction = directionSelect.value
  const source = complete.map(l => direction === 'ct-mri' ? l.ct! : l.mri!)
  const target = complete.map(l => direction === 'ct-mri' ? l.mri! : l.ct!)
  const previous=fitResult?cloneFitState(fitResult):null,previousTarget=manualReviewTarget,initialImages={...loaded},residuals=new Map(landmarks.map(l=>[l.id,l.residual]))
  let fitted:FitState|null=null,outcome:'completed'|'cancelled'|'failed'='completed'
  if(!await activity.begin('Fitting landmark transformation…'))return
  const fitActivityId=activity.id
  try {
    const result = fitLandmarks(source,target,model)
    const candidate = { matrix: result.matrix } as FitState
    if (!confirmReflectedTransform(candidate, 'apply')) return
    complete.forEach((l,i)=>l.residual=result.residuals[i])
    manualReviewTarget=null;manualUI?.stopPreview();clearSourcePreview()
    fitResult={direction,model,landmarkModel:model,parameterCount:result.parameterCount,diagnostics:result.diagnostics,reflectionConfirmed:candidate.reflectionConfirmed ?? false,landmarkMatrix:result.matrix,baseMatrix:result.matrix,matrix:result.matrix,inverse:result.inverse,rms:result.rms,manual:[0,0,0,0,0,0],proposal:null,landmarkSnapshot:structuredClone(landmarks),landmarksChanged:false,fittedAt:new Date().toISOString()}
    fitted=fitResult
    fitSummary.innerHTML=fitSummaryHtml(fitResult)
    saveAligned.disabled=false;saveTransform.disabled=false;resetManual.disabled=false;resetLandmark.disabled=false;refineButton.disabled=false;restoreAlignmentLandmarks.disabled=false;document.querySelector<HTMLButtonElement>('#export-open')!.disabled=false
    await activity.phase('Preparing transformed overlay…')
    updateNudgeOutputs(); await buildReview(true);if(fitResult!==fitted)return;renderAll();setStatus(`${LANDMARK_MODELS[model].label} fitted with RMS error ${result.rms.toFixed(3)} mm.`)
  } catch(err) {
    outcome=activity.state.activity.status==='cancelling'?'cancelled':'failed'
    if(fitted&&fitResult===fitted&&loaded.mri===initialImages.mri&&loaded.ct===initialImages.ct){
      if(previous){fitResult=previous;manualReviewTarget=previousTarget;fitSummary.innerHTML=fitSummaryHtml(previous);refineActions.classList.toggle('hidden',!previous.proposal);updateNudgeOutputs()}
      else clearFit()
      for(const landmark of landmarks){const residual=residuals.get(landmark.id);if(residual===undefined)delete landmark.residual;else landmark.residual=residual}
      renderAll()
    }
    setStatus(err instanceof Error?err.message:String(err),true)
  }finally{activity.end(outcome,fitActivityId);automaticUI?.refresh();jipUI?.refresh();itkUI?.refresh();acquisitionUI?.refresh()}
}



function residualRms(matrix: Mat4): number {
  if (!fitResult) return NaN
  const complete = fitResult.landmarkSnapshot.filter(l=>l.enabled&&l.mri&&l.ct)
  if (!complete.length) return 0
  let sum = 0
  for (const lm of complete) {
    const source = fitResult.direction === 'ct-mri' ? lm.ct! : lm.mri!
    const target = fitResult.direction === 'ct-mri' ? lm.mri! : lm.ct!
    const q = createPointMapping(matrix,fitResult.nonlinear).forward(source)
    const d = Math.hypot(q[0]-target[0],q[1]-target[1],q[2]-target[2])
    lm.residual = d; sum += d*d
  }
  return Math.sqrt(sum/complete.length)
}

function updateNudgeOutputs() {
 if(manualUI){manualUI.refresh();return}
 const p=fitResult?.manual??[0,0,0,0,0,0];p.forEach((v,i)=>{const field=document.querySelector<HTMLInputElement>(`#param-${i}`);if(field)field.value=String(v)})
}
function updateCurrentTransform(rebuild=true){
 if(!fitResult)return
 fitResult.inverse=invertAffine(fitResult.matrix);fitResult.rms=residualRms(fitResult.matrix);fitResult.proposal=null;refineActions.classList.add('hidden');fitSummary.innerHTML=fitSummaryHtml(fitResult);updateNudgeOutputs();renderAll();if(rebuild)scheduleReviewBuild()
}

function scheduleReviewBuild() {
  document.querySelector('#refresh-processed-overlay')!.classList.remove('hidden')
  reviewScheduler.schedule(()=>{void buildReview(false,true)},450)
  // Coalesce repeated nudges into one GPU display update per animation frame.
  // The original source is resampled only after the user pauses adjustment.
  if(manualPreviewFrame===null)manualPreviewFrame=requestAnimationFrame(()=>{
    manualPreviewFrame=null
    const base=manualPreviewBase,candidate=fitResult,target=manualReviewTarget
    if(!base||!candidate||!target||base.movingModality!==target||activity.busy)return
    const delta=manualPreviewDelta(base.fit,candidate,target);if(!delta)return
    const image=transformedPreviewImage(base.moving,delta)
    for(const view of Object.values(reviewViews)){
      if(view.nv.volumes.length!==2)continue
      view.nv.setVolumeAffine(1,image.hdr!.affine)
    }
    reviewImages={...base,moving:image,fit:cloneFitState(candidate)}
    document.querySelector('#review-label')!.textContent=`${imageLabel(target)} · interactive preview; full detail updates after adjustment pauses`
    updateSourcePreview();renderReviewMarkers()
  })
}


function updateWindowControls() {
  defineWindows.setAttribute('aria-pressed', String(definingWindows))
  const mriCount = countOptimizationWindows(optimizationWindows, 'mri'), ctCount = countOptimizationWindows(optimizationWindows, 'ct')
  windowSummary.textContent = mriCount || ctCount
    ? `Active constraints: ${imageLabel('mri')} ${mriCount} plane${mriCount === 1 ? '' : 's'}, ${imageLabel('ct')} ${ctCount} plane${ctCount === 1 ? '' : 's'}. Each defined plane restricts only its two visible axes; any plane without a window is unrestricted.`
    : 'No optimization windows defined. The full geometric overlap will be used.'
  defineWindows.textContent = definingWindows ? 'Finish defining windows' : 'Define windows'
  for (const modality of ['mri','ct'] as Modality[]) for (const view of Object.values(views[modality])) {
    const active = definingWindows && (windowTarget.value === 'both' || windowTarget.value === modality)
    view.windowLayer.classList.toggle('active', active||Boolean(initialProcessing?.enabled(view)))
    renderOptimizationWindow(view)
  }
}

function renderOptimizationWindow(view: View) {
  thresholdMasks?.render(view)
  view.windowLayer.replaceChildren()
  const bounds = optimizationWindows[view.modality][view.plane]
  if (!bounds || !loaded[view.modality]) {initialProcessing?.render(view);return}
  const projected=windowRectangleOnCanvas(view,bounds)
  if(!projected){initialProcessing?.render(view);return}
  const canvasRect=view.canvas.getBoundingClientRect()
  const scaleX=canvasRect.width/Math.max(1,view.canvas.width),scaleY=canvasRect.height/Math.max(1,view.canvas.height)
  const rect=document.createElement('div');rect.className='optimization-window-box'
  rect.style.left=`${projected.left*scaleX}px`;rect.style.top=`${projected.top*scaleY}px`
  rect.style.width=`${projected.width*scaleX}px`;rect.style.height=`${projected.height*scaleY}px`
  view.windowLayer.appendChild(rect)
  initialProcessing?.render(view)
}

function eventMm(view: View, event: PointerEvent): Vec3 | null {
  const rect = view.canvas.getBoundingClientRect()
  if (rect.width <= 0 || rect.height <= 0) return null
  const pos: [number,number] = [
    (event.clientX-rect.left)*(view.canvas.width/rect.width),
    (event.clientY-rect.top)*(view.canvas.height/rect.height),
  ]
  return windowPointFromCanvas(view,pos[0],pos[1],true)
}

function installWindowCapture(view: View) {
  installOptimizationWindowCapture({
    view,
    isEnabled: () => Boolean(
      loaded[view.modality] &&
      definingWindows &&
      (windowTarget.value === 'both' || windowTarget.value === view.modality)
    ),
    getPixDims: () => loaded[view.modality]?.raw.pixDims ?? null,
    eventToMm: event => eventMm(view, event),
    onCommit: bounds => {
      optimizationWindows[view.modality][view.plane] = bounds
      optimizationWindows = sanitizeOptimizationWindows(optimizationWindows)
      updateWindowControls()
      renderMarkers(view.modality)
      setStatus(`${imageLabel(view.modality)} ${view.plane} optimization window set.`)
    },
    onReject: () => {
      // Invalid replacement drags must never erase an existing valid window.
      setStatus('Window was not changed: drag a rectangle at least 4 pixels wide and high.', true)
      updateWindowControls()
    },
  })
}

function optimizationConstraint(modality: Modality): WindowConstraint | null {
  return getOptimizationConstraint(optimizationWindows, modality)
}

// Compatibility entry point: never run a second, main-thread refinement engine.
function runRefinement(){
 document.querySelector<HTMLButtonElement>('#local-auto-linear')!.click()
}

function acceptRefinement(){
  if(!fitResult?.proposal)return
  operations.dispatch({type:'proposal/project',pending:true});operations.dispatch({type:'proposal/decision',decision:'accepted'})
  fitResult.baseMatrix=fitResult.proposal.matrix;fitResult.matrix=fitResult.proposal.matrix;fitResult.inverse=invertAffine(fitResult.matrix);fitResult.manual=[0,0,0,0,0,0];fitResult.proposal=null
  fitResult.rms=residualRms(fitResult.matrix);fitSummary.innerHTML=fitSummaryHtml(fitResult);refineActions.classList.add('hidden');updateNudgeOutputs();buildReview();renderAll();setStatus('Registration accepted.')
}
function rejectRefinement(){
  if(!fitResult?.proposal)return
  operations.dispatch({type:'proposal/project',pending:true});operations.dispatch({type:'proposal/decision',decision:'rejected'})
  if(fitResult.proposal.kind==='automatic'||fitResult.proposal.kind==='spline'||fitResult.proposal.kind==='jip'){const previous=fitResult.proposal.previous;if(previous){fitResult=cloneFitState(previous);fitSummary.innerHTML=fitSummaryHtml(fitResult);updateNudgeOutputs();buildReview();renderAll()}else clearFit();refineActions.classList.add('hidden');setStatus('Registration rejected. Previous alignment restored.');return}
  const fixed=loaded[registrationModalities(fitResult.direction).fixed];if(!fixed)return;const center=applyAffine(fixed.raw.affine,fixed.raw.dims.map(n=>(n-1)/2) as Vec3);fitResult=rejectLegacyRefinement(fitResult,center);updateCurrentTransform(true);refineSummary.textContent='Refinement rejected. The previous accepted transform has been restored.';setStatus('Automatic refinement rejected.')
}

function currentSession() {
  return createSessionPayload({
    appVersion: APP_VERSION,
    registrationPairing:getRegistrationPairing(),
    loaded,
    landmarks,
    selectedId,
    nextId,
    direction: directionSelect.value,
    splineSmoothing:sessionSplineSmoothing(),
    landmarkModel: document.querySelector<HTMLSelectElement>('#landmark-transform')!.value as RegistrationModel,
    fit: fitResult,
    optimizationWindows,
  })
}

async function saveSessionFile() {
  await saveArtifact(
    new Blob([JSON.stringify(currentSession(), null, 2)], { type: 'application/json' }),
    'brainana-align_session.json',
  )
  setStatus('Session saved.')
}

async function loadSessionFile(file: File) {
  const payload = parseSessionPayload(JSON.parse(await file.text()))
  const mismatch = sessionGeometryMismatches(payload.images, loaded,payload.processing)
  let restoreFit = true
  if (mismatch.length) {
    restoreFit = false
    const ok = window.confirm(`Image geometry does not match the saved session:\n\n${mismatch.join('\n')}\n\nLoad landmarks only?`)
    if (!ok) return false
  }
  if (restoreFit && payload.fit && !confirmReflectedTransform(payload.fit, 'restore and apply')) return false
  if(restoreFit){if(!await activity.begin('Restoring image processing…','processing'))return false;document.querySelector('#alignment-cancel')!.classList.remove('hidden');try{await initialProcessing!.restore(payload.processing??{})}finally{document.querySelector('#alignment-cancel')!.classList.add('hidden');activity.end()}}
  if(restoreFit)for(const m of ['mri','ct'] as Modality[])if(loaded[m])loaded[m]!.acquisition=payload.images[m]?.acquisition
  acquisitionUI?.refresh()
  if(restoreFit||(!loaded.mri&&!loaded.ct))pairingUI?.restore(payload.registrationPairing)
  snapshot()
  landmarks = structuredClone(payload.landmarks)
  selectedId = payload.selectedId
  nextId = Math.max(payload.nextId, ...landmarks.map(landmark => landmark.id + 1), 1)
  directionSelect.value = payload.direction
  document.querySelector<HTMLSelectElement>('#landmark-transform')!.value = (payload.landmarkModel ?? payload.fit?.model)==='spline'?'rigid':payload.landmarkModel ?? payload.fit?.model ?? 'rigid'
  document.querySelector<HTMLInputElement>('#spline-smoothing')!.value=String(payload.splineSmoothing??(payload.fit?.nonlinear?.kernel==='3d-polyharmonic-r'?payload.fit.nonlinear.smoothing:undefined)??.01)
  updateSplineControls()
  optimizationWindows = sanitizeOptimizationWindows(structuredClone(payload.optimizationWindows))
  updateWindowControls()
  if (restoreFit && payload.fit && loaded.mri && loaded.ct) {
    clearSourcePreview();manualReviewTarget=payload.fit?.manualAffine?.selected??null
    fitResult = cloneFitState(payload.fit)
    if (!fitResult.landmarkSnapshot) fitResult.landmarkSnapshot = structuredClone(payload.landmarks)
    if (fitResult.landmarksChanged === undefined) fitResult.landmarksChanged = false
    if (!fitResult.fittedAt) fitResult.fittedAt = payload.savedAt
    saveAligned.disabled = false
    saveTransform.disabled = false
    resetManual.disabled = false
    resetLandmark.disabled = false
    refineButton.disabled = false
    restoreAlignmentLandmarks.disabled = false
    document.querySelector<HTMLButtonElement>('#export-open')!.disabled = false
    updateNudgeOutputs()
    const restored=fitResult
    await buildReview(true)
    if(fitResult!==restored)return false
    fitSummary.innerHTML = fitSummaryHtml(fitResult)
    refineActions.classList.toggle('hidden', !fitResult.proposal)
    if (fitResult.proposal) refineSummary.textContent = 'Saved refinement proposal restored. Review, then accept or reject before exporting.'
  } else {
    clearFit()
  }
  renderAll()
  setStatus(restoreFit && payload.fit ? 'Full session restored.' : 'Landmarks restored; fit was not restored.')
  return true
}

async function saveTransformFiles() {
  if (!fitResult) throw new Error('Create or accept an alignment before exporting.')
  if (!confirmReflectedTransform(fitResult, 'export')) return setStatus('Export cancelled.',true)
  await saveArtifacts(createRegistrationArtifacts({ alignedExportOptions:readAlignedExportOptions(), appVersion: APP_VERSION, fit: fitResult, landmarks,images:loaded,imageExportMode:document.querySelector<HTMLSelectElement>('#aligned-image-source')!.value==='processed'?'processed':'native' }))
  setStatus('Transform files saved.')
}

async function saveAlignedImage() {
  if (!fitResult) return
  if (fitResult.proposal) throw new Error('Accept or reject the refinement proposal before exporting')
  if (!confirmReflectedTransform(fitResult, 'resample and export')) return setStatus('Export cancelled.',true)
  const sourceMod: Modality = fitResult.direction === 'ct-mri' ? 'ct' : 'mri'
  const targetMod: Modality = fitResult.direction === 'ct-mri' ? 'mri' : 'ct'
  const source = loaded[sourceMod]
  const target = loaded[targetMod]
  if (!source || !target) return setStatus('Both images must be loaded.', true)
  const saved=await saveNativeAlignedImage(source,target,fitResult,sourceMod,targetMod,activity,setStatus,readAlignedExportOptions());if(!saved)return
  setStatus('Aligned image saved.')
}

displayIntensity=installDisplayIntensity({loaded:()=>loaded,viewers:m=>[...Object.values(views[m]).map(v=>({nv:v.nv,index:0})),...Object.values(reviewViews).flatMap(v=>reviewImages?[{nv:v.nv,index:reviewImages.fixedModality===m?0:1}]:[])]})
initialProcessing=installInitialProcessing({getLoaded:()=>loaded,pending:()=>Boolean(fitResult?.proposal),activity,commit:items=>commitPrepared(items,[],'processing'),stopRegion:()=>{definingWindows=false},redraw:()=>{updateWindowControls();renderAll()},status:setStatus})
thresholdMasks=installThresholdMasks({getLoaded:()=>loaded,views:m=>Object.values(views[m]),busy:()=>activity.busy,pending:()=>Boolean(fitResult?.proposal),update:(m,fn)=>initialProcessing!.update(m,fn),status:setStatus,activity})
preparation=installImagePreparation({getLoaded:()=>loaded,busy:()=>activity.busy,begin:text=>activity.begin(text,'preparation'),phase:text=>activity.phase(text),end:()=>activity.end(),commit:commitPrepared,status:setStatus,preparationBlocked:()=>Object.values(loaded).some(item=>item?.processing)?'Restore original working images in Initial processing before changing resolution or replacing inputs. Save a session to preserve processing first.':null})
pairingUI=installWelcomeScreen(()=>Boolean(loaded.mri||loaded.ct||preparation.state().mri||preparation.state().ct))
document.querySelector<HTMLInputElement>('#mri-file')!.addEventListener('change',e=>{const files=Array.from((e.target as HTMLInputElement).files ?? []);if(files.length)loadFiles('mri',files,true).catch(err=>setStatus(err.message,true))})
document.querySelector<HTMLInputElement>('#ct-file')!.addEventListener('change',e=>{const files=Array.from((e.target as HTMLInputElement).files ?? []);if(files.length)loadFiles('ct',files,true).catch(err=>setStatus(err.message,true))})
document.querySelector('#new-pair')!.addEventListener('click',()=>{snapshot();const lm={id:nextId++,mri:null,ct:null,enabled:true};landmarks.push(lm);selectedId=lm.id;markLandmarksChanged();renderAll()})
document.querySelector('#undo')!.addEventListener('click',()=>{const prev=history.pop();if(prev){landmarks=prev;markLandmarksChanged();renderAll()}})
directionSelect.addEventListener('change',clearFit)
saveTransform.addEventListener('click',()=>exportUI!.run(saveTransformFiles).catch(err=>setStatus(err instanceof Error?err.message:String(err),true)))
saveAligned.addEventListener('click',()=>exportUI!.run(saveAlignedImage).catch(err=>setStatus(err instanceof Error?err.message:String(err),true)))
reviewMode.addEventListener('change', applyReviewAppearance)
reviewOpacity.addEventListener('input', scheduleReviewOpacity)
const opacityCycle=installOpacityCycle(reviewOpacity,document.querySelector<HTMLButtonElement>('#opacity-cycle')!,scheduleReviewOpacity,{mode:document.querySelector<HTMLSelectElement>('#review-comparison')!,flip:document.querySelector<HTMLButtonElement>('#review-flip')!,label:document.querySelector<HTMLElement>('#review-flip-label')!,show:image=>updateReviewFlip(Object.values(reviewViews).map(v=>v.nv),image),names:()=>({fixed:reviewImages?imageLabel(reviewImages.fixedModality):'Fixed image',moving:reviewImages?imageLabel(reviewImages.movingModality):'Moving image'}),available:()=>Boolean(reviewImages)})
const alignedOptions=installAlignedExportOptions(()=>{const moving=fitResult?.direction==='mri-ct'?'mri':'ct';return {fit:fitResult,source:loaded[moving],target:loaded[moving==='ct'?'mri':'ct']}})
document.querySelector('#export-open')!.addEventListener('click',()=>alignedOptions.refresh())
manualUI=installManualAdjustment({getFit:()=>fitResult,setFit:fit=>{fitResult=fit},getLoaded:()=>loaded,busy:()=>activity.busy,changed:()=>{manualReviewTarget=manualUI!.target;updateCurrentTransform(true)},targetChanged:()=>{clearSourcePreview();manualReviewTarget=manualUI!.target;if(fitResult)void buildReview(false,true)},previewChanged:()=>{if(!manualUI?.preview){clearSourcePreview();renderAll()}else{manualReviewTarget=manualUI.target;void buildReview(false,true)}},status:setStatus})
document.addEventListener('click',event=>{const button=(event.target as HTMLElement).closest('button');if(!sourcePreview||!button)return;if(button.matches('[data-workflow=landmarks],[data-workflow=followup],[data-workflow=automatic],.set-mri,.set-ct,.go-mri,.go-ct')||button.closest('#initial-processing-panel')||['new-pair','define-windows','define-windows-followup','working-resolution','load-session','run-landmark-spline'].includes(button.id))manualUI?.stopPreview()},true)
document.addEventListener('input',event=>{if(sourcePreview&&(event.target as HTMLElement).closest('#initial-processing-panel'))manualUI?.stopPreview()},true)
resetLandmark.addEventListener('click',()=>{if(!fitResult||fitResult.proposal||activity.busy)return;fitResult=resetInitialAlignment(fitResult);updateCurrentTransform(true);setStatus('Restored the original landmark fit.')})
refineButton.addEventListener('click',()=>runRefinement())
defineWindows.addEventListener('click',()=>{initialProcessing?.stop();definingWindows=!definingWindows;updateWindowControls();renderMarkers('mri');renderMarkers('ct');setStatus(definingWindows?imagingText('Drag a rectangle in any selected MRI or CT panel. Draw in multiple planes to define a 3D block.'):'Optimization-window definition finished.')})
clearWindows.addEventListener('click',()=>{optimizationWindows={mri:{},ct:{}};updateWindowControls();renderMarkers('mri');renderMarkers('ct');setStatus('Optimization windows cleared.')})
windowTarget.addEventListener('change',updateWindowControls)
installSharedRegionControls()
document.querySelector('#run-landmark-spline')!.addEventListener('click',()=>{if(!hasLandmarkBasis(fitResult)||fitResult?.proposal)return setStatus('Create and accept a landmark registration first.',true);void runSplineFit()})
updateWindowControls()
acceptRefine.addEventListener('click',acceptRefinement)
rejectRefine.addEventListener('click',rejectRefinement)
document.querySelector('#save-session')!.addEventListener('click',()=>saveSessionFile().catch(err=>setStatus(err instanceof Error?err.message:String(err),true)))
document.querySelector<HTMLInputElement>('#load-session')!.addEventListener('change',e=>{const file=(e.target as HTMLInputElement).files?.[0];if(file)loadSessionFile(file).catch(err=>setStatus(err instanceof Error?err.message:String(err),true));(e.target as HTMLInputElement).value=''})
refineBounds.addEventListener('change',()=>{
  if(refineBounds.value==='tight'){refineTranslationLimit.value='2';refineRotationLimit.value='1';refineTranslationStartStep.value='1';refineRotationStartStep.value='0.5'}
  else if(refineBounds.value==='standard'){refineTranslationLimit.value='5';refineRotationLimit.value='10';refineTranslationStartStep.value='2';refineRotationStartStep.value='1'}
})
for(const input of [refineTranslationLimit,refineRotationLimit,refineTranslationStartStep,refineRotationStartStep])input.addEventListener('input',()=>{refineBounds.value='custom'})

document.querySelector('#fit-side')!.addEventListener('click',runFit)
const exportModal=document.querySelector<HTMLDivElement>('#export-modal')!
exportUI=installExportDialog(()=>({fit:fitResult,images:Boolean(loaded.mri&&loaded.ct),busy:activity.busy}),()=>{exportModal.classList.add('hidden');const spline=fitResult?.proposal?.kind==='spline',local=isLocalRegistration(fitResult);document.querySelector<HTMLButtonElement>(spline?'[data-workflow=landmarks]':local?'[data-workflow=followup]':'[data-workflow=automatic]')?.click();const actions=document.querySelector<HTMLElement>(spline?'#spline-actions':local?'#refine-actions-followup':'#refine-actions')!;actions.classList.remove('hidden');revealControl(actions);actions.scrollIntoView?.({block:'nearest'});document.querySelector<HTMLElement>(spline?'#accept-spline':local?'#accept-refine-followup':'#accept-refine')!.focus();setStatus('Review the preview, then accept or reject the pending alignment.')})
generatedExports=installGeneratedImageExports({getLoaded:()=>loaded,activity,run:action=>exportUI!.run(action),report:(s,e)=>exportUI!.report(s,e)})
const openExport=()=>{generatedExports?.refresh();document.querySelector('#spline-field-export')!.classList.toggle('hidden',!fitResult?.nonlinear);exportUI!.open()}
const closeExport=()=>exportModal.classList.add('hidden')
document.querySelector('#export-open')!.addEventListener('click',openExport)
document.querySelector('#export-close')!.addEventListener('click',closeExport)
exportModal.addEventListener('click',e=>{if(e.target===exportModal)closeExport()})
document.addEventListener('keydown',e=>{if(e.key==='Escape')closeExport()})
document.querySelector('#save-session-export')!.addEventListener('click',()=>exportUI!.run(saveSessionFile).catch(err=>setStatus(err instanceof Error?err.message:String(err),true)))

restoreAlignmentLandmarks.addEventListener('click',()=>{
  if(!fitResult)return
  snapshot()
  landmarks=structuredClone(fitResult.landmarkSnapshot)
  selectedId=landmarks.find(l=>l.id===selectedId)?.id ?? landmarks[0]?.id ?? null
  fitResult.landmarksChanged=false
  residualRms(fitResult.matrix)
  fitSummary.innerHTML=fitSummaryHtml(fitResult)
  renderAll();setStatus('Restored the landmark snapshot used for the accepted alignment.')
})


function resetAllViews() {
  const all = [
    ...Object.values(views.mri),
    ...Object.values(views.ct),
    ...Object.values(reviewViews),
  ]
  for (const view of all) {
    view.nv.scene.pan2Dxyzmm = [0, 0, 0, 1] as any
    view.nv.drawScene()
  }
  renderMarkers('mri')
  renderMarkers('ct')
  renderReviewMarkers()
  setStatus('Zoom and field of view reset for all panels.')
}

document.querySelector('#reset-view')!.addEventListener('click', resetAllViews)
document.querySelector('#refresh-processed-overlay')!.addEventListener('click',async()=>{
 if(!fitResult||activity.busy)return
 if(!await activity.begin('Refreshing overlay with the unchanged transform…','display'))return
 try{await buildReview(true);setStatus('Overlay refreshed using the existing transform. No registration was run.')}catch(error){setStatus(error instanceof Error?error.message:String(error),true)}finally{activity.end()}
})


const browserCapabilities = detectBrowserCapabilities()
const browserReady = installBrowserCompatibilityBanner(browserCapabilities)
if (browserReady) {
  setupViews()
  setupReviewViews()
} else {
  setStatus('This browser cannot initialize the required WebGL2 viewer.', true)
}
renderLandmarks()
installRuntimeIntegration((modality,files)=>loadFiles(modality,files,true), setStatus)


document.querySelector('#use-image-geometry')!.addEventListener('click',async()=>{
  if(activity.busy)return
  if(fitResult?.proposal)return setStatus('Accept or reject the current registration proposal first.',true)
  if(!loaded.mri||!loaded.ct)return setStatus(imagingText('Load MRI and CT before beginning manual alignment.'),true)
  if(fitResult)return setStatus('An alignment already exists. Use the manual controls to adjust it.',true)
  if(!await activity.begin('Preparing manual alignment overlay…','display'))return
  try{
  manualReviewTarget=null;manualUI?.stopPreview();clearSourcePreview()
  fitResult=imageGeometryAlignment(directionSelect.value)
  for(const lm of landmarks)delete lm.residual
  saveAligned.disabled=false;saveTransform.disabled=false;resetManual.disabled=false;resetLandmark.disabled=false;refineButton.disabled=false;document.querySelector<HTMLButtonElement>('#export-open')!.disabled=false
  restoreAlignmentLandmarks.disabled=true;refineActions.classList.add('hidden');fitSummary.innerHTML=fitSummaryHtml(fitResult)
  updateNudgeOutputs();await buildReview(true);renderAll();setStatus('Manual alignment is ready. Use the manual controls and inspect the overlay in all three planes.')
  }catch(err){setStatus(err instanceof Error?err.message:String(err),true)}finally{activity.end();automaticUI?.refresh();jipUI?.refresh();itkUI?.refresh();acquisitionUI?.refresh()}
})
installGuidedSetup(()=>({welcome:pairingUI?.isWelcome(),preparation:preparation.state(),mri:Boolean(loaded.mri),ct:Boolean(loaded.ct),landmarks,selectedId,landmarkModel:document.querySelector<HTMLSelectElement>('#landmark-transform')!.value,minimumPairs:document.querySelector<HTMLSelectElement>('#landmark-transform')!.value==='spline'?12:['affine','anisotropic'].includes(document.querySelector<HTMLSelectElement>('#landmark-transform')!.value)?4:3,fit:fitResult,busy:activity.busy,busyKind:activity.kind,error:statusEl.classList.contains('error')?statusEl.textContent:null,viewerReady:browserReady}))

document.querySelector('#manual-start-geometry')!.addEventListener('click',()=>document.querySelector<HTMLButtonElement>('#use-image-geometry')!.click())

async function acceptRegistrationCandidate(fit:FitState){const prior=fitResult,priorImages={...loaded},priorReview=reviewImages,priorTarget=manualReviewTarget;manualReviewTarget=null;manualUI?.stopPreview();clearSourcePreview();try{fitResult=fit;fitResult.rms=residualRms(fit.matrix);saveAligned.disabled=false;saveTransform.disabled=false;resetManual.disabled=false;resetLandmark.disabled=false;refineButton.disabled=false;document.querySelector<HTMLButtonElement>('#export-open')!.disabled=false;fitSummary.innerHTML=fitSummaryHtml(fit);refineSummary.textContent=fit.automaticRegistration?`${fit.automaticRegistration!.metric.toUpperCase()} score ${fit.automaticRegistration!.scoreBefore?.toFixed(5)??'unavailable'} → ${fit.automaticRegistration!.scoreAfter.toFixed(5)}; ${fit.automaticRegistration!.candidatesEvaluated} evaluations. Review before accepting.`:`${fit.jipRegistration?.settings.engine==='itk'?'ITK':'JIP'} proposal ready. Inspect alignment, then accept or reject.`;acceptRefine.textContent='Accept registration';refineActions.classList.remove('hidden');updateNudgeOutputs();await buildReview(true);renderAll()}catch(error){if(loaded.mri===priorImages.mri&&loaded.ct===priorImages.ct&&prior){fitResult=prior;manualReviewTarget=priorTarget;fitSummary.innerHTML=fitSummaryHtml(prior);refineActions.classList.toggle('hidden',!prior.proposal);updateNudgeOutputs();if(reviewImages!==priorReview)await buildReview(true).catch(()=>clearReview());renderAll()}else clearFit();throw error}}
automaticUI=installAutomaticRegistration({getLoaded:()=>loaded,getFit:()=>fitResult,getLandmarks:()=>landmarks,getDirection:()=>directionSelect.value,getWindows:m=>registrationWindows(optimizationConstraint(m)),activity,status:setStatus,onResult:acceptRegistrationCandidate})
jipUI=installJipRegistration({getLoaded:()=>loaded,getFit:()=>fitResult,getDirection:()=>directionSelect.value,getWindows:m=>registrationWindows(optimizationConstraint(m)),activity,status:setStatus,onResult:acceptRegistrationCandidate})
itkUI=installItkRegistration({getLoaded:()=>loaded,getFit:()=>fitResult,getDirection:()=>directionSelect.value,getWindows:m=>registrationWindows(optimizationConstraint(m)),activity,status:setStatus,onResult:acceptRegistrationCandidate})
acquisitionUI=installAcquisitionUI({getLoaded:()=>loaded,busy:()=>activity.busy,status:setStatus})
document.addEventListener('click',()=>queueMicrotask(()=>{jipUI?.refresh();itkUI?.refresh();acquisitionUI?.refresh()}));document.addEventListener('change',()=>queueMicrotask(()=>{jipUI?.refresh();itkUI?.refresh();acquisitionUI?.refresh()}))

let appliedPreferences:Preferences|null=null
function applySettings(p:Preferences){
  applyDisplayPreferences({mri:Object.values(views.mri).map(v=>v.nv),ct:Object.values(views.ct).map(v=>v.nv),review:Object.values(reviewViews).map(v=>v.nv),mode:reviewMode,opacity:reviewOpacity,opacityLabel:opacityValue,appearance:applyReviewAppearance},p.display,appliedPreferences?.display)
  opacityCycle.refresh()
  if(!appliedPreferences||p.interaction.linked!==appliedPreferences.interaction.linked)linkAfter.checked=p.interaction.linked
  appliedPreferences=structuredClone(p);renderMarkers('mri');renderMarkers('ct');renderReviewMarkers()
}
function applyRegistrationDefaults(){const p=getPreferences().registration;document.querySelector<HTMLSelectElement>('#landmark-transform')!.value=p.model;document.querySelector<HTMLSelectElement>('#auto-linear-model')!.value=p.model;document.querySelector('#auto-linear-model')!.dispatchEvent(new Event('change'));refineBounds.value=p.preset;refineTranslationLimit.value=String(p.translationLimit);refineRotationLimit.value=String(p.rotationLimit);refineTranslationStartStep.value=String(p.translationStep);refineRotationStartStep.value=String(p.rotationStep);updateSplineControls();renderAll()}
window.addEventListener('align-preferences-storage-error',(event)=>setStatus('Saved settings could not be loaded; using defaults. '+(event as CustomEvent).detail,true))
window.addEventListener('align-preferences-changed',()=>applySettings(getPreferences()))
window.addEventListener('align-registration-defaults',()=>{if(!activity.busy)applyRegistrationDefaults()})
void initializePreferences().then(p=>{applySettings(p);if(!fitResult&&!landmarks.length)applyRegistrationDefaults();installSettings({status:setStatus,session:currentSession,hasWork:()=>Boolean(loaded.mri||loaded.ct||landmarks.length),busy:()=>activity.busy,recover:async snapshot=>{if(activity.busy)throw new Error('Wait for alignment to finish.');if(!loaded.mri||!loaded.ct)throw new Error(imagingText('Load the original MRI and CT before restoring recovery.'));if(!await loadSessionFile(new File([JSON.stringify(snapshot)],'brainana-align-recovery.json',{type:'application/json'})))throw new Error('Recovery was not applied.')}});window.dispatchEvent(new Event('align-preferences-ready'))}).catch(error=>setStatus('Settings could not be loaded: '+error.message,true))

let preferenceSaveTimer:number|undefined
const rememberDisplayControls=()=>{window.clearTimeout(preferenceSaveTimer);preferenceSaveTimer=window.setTimeout(()=>{const p=structuredClone(getPreferences());p.display.mode=reviewMode.value as Preferences['display']['mode'];p.display.opacity=Number(reviewOpacity.value);p.interaction.linked=linkAfter.checked;void savePreferences(p).catch(error=>setStatus('Display preferences could not be saved: '+error.message,true))},250)}
reviewOpacity.addEventListener('input',rememberDisplayControls);reviewMode.addEventListener('change',rememberDisplayControls);linkAfter.addEventListener('change',rememberDisplayControls)

installWorkspaceReset({activity,status:setStatus})

function updateSplineControls(){document.querySelector('#spline-settings')!.classList.remove('hidden')}
document.querySelector('#landmark-transform')!.addEventListener('change',updateSplineControls)
async function runSplineFit(){
  if(!loaded.mri||!loaded.ct)return setStatus(imagingText('Load both MRI and CT to preview a nonlinear proposal.'),true)
  const pairs=landmarks.filter(l=>l.enabled&&l.mri&&l.ct),direction=directionSelect.value,initialImages={...loaded},landmarkState=JSON.stringify(landmarks),previous=fitResult?cloneFitState(fitResult):null
  if(pairs.length<12)return setStatus('Nonlinear spline needs at least 12 complete, well-distributed pairs; 20+ is recommended.',true)
  if(!await activity.begin('Preparing nonlinear landmark spline…'))return
  try{
    const result=await runNonlinearTask<ReturnType<typeof fitNonlinearSpline>>({type:'fit',source:pairs.map(l=>direction==='ct-mri'?l.ct!:l.mri!),target:pairs.map(l=>direction==='ct-mri'?l.mri!:l.ct!),smoothing:Number(document.querySelector<HTMLInputElement>('#spline-smoothing')!.value)},text=>{void activity.phase(text)})
    if(loaded.mri!==initialImages.mri||loaded.ct!==initialImages.ct||directionSelect.value!==direction||JSON.stringify(landmarks)!==landmarkState)throw Error('Images or landmarks changed during spline fitting; candidate discarded')
    pairs.forEach((l,i)=>l.residual=result.residuals[i])
    manualReviewTarget=null;manualUI?.stopPreview();clearSourcePreview()
    fitResult={direction,model:'spline',landmarkModel:'spline',initializationSource:'landmarks',nonlinear:result.spline,parameterCount:result.parameterCount,diagnostics:result.diagnostics,reflectionConfirmed:false,landmarkMatrix:result.matrix,baseMatrix:result.matrix,matrix:result.matrix,inverse:result.inverse,rms:result.rms,manual:[0,0,0,0,0,0],proposal:{matrix:result.matrix,scoreBefore:result.affineRms,scoreAfter:result.rms,params:[],kind:'spline',previous},landmarkSnapshot:structuredClone(landmarks),landmarksChanged:false,fittedAt:new Date().toISOString()}
    fitSummary.innerHTML=fitSummaryHtml(fitResult);saveAligned.disabled=false;saveTransform.disabled=false;resetManual.disabled=false;resetLandmark.disabled=false;refineButton.disabled=false;document.querySelector<HTMLButtonElement>('#export-open')!.disabled=false
    refineSummary.textContent=`Spline proposal: affine RMS ${result.affineRms.toFixed(3)} → spline RMS ${result.rms.toFixed(3)} mm. Inspect all slices and distortion diagnostics before accepting.`;acceptRefine.textContent='Accept spline';refineActions.classList.remove('hidden');updateNudgeOutputs();await buildReview(true);renderAll();setStatus('Nonlinear spline proposal ready. Review, then accept or reject.')
  }catch(error){if(previous&&loaded.mri===initialImages.mri&&loaded.ct===initialImages.ct){fitResult=previous;fitSummary.innerHTML=fitSummaryHtml(previous);refineActions.classList.add('hidden');await buildReview(true).catch(()=>clearReview());renderAll()}else clearFit();setStatus(error instanceof Error?error.message:String(error),true)}finally{activity.end();automaticUI?.refresh();jipUI?.refresh();itkUI?.refresh();acquisitionUI?.refresh()}
}
document.querySelector('#save-spline-fields')!.addEventListener('click',()=>{void (async()=>{
 if(!fitResult?.nonlinear)return setStatus('Fit and accept a nonlinear spline first.',true)
 if(fitResult.proposal)return setStatus('Accept or reject the proposal before exporting.',true)
 const fit=fitResult,moving:Modality=fit.direction==='ct-mri'?'ct':'mri',fixed:Modality=moving==='ct'?'mri':'ct'
 if(!loaded[moving]||!loaded[fixed])return setStatus('Load both images to export displacement fields.',true)
 const spacing=Number(document.querySelector<HTMLInputElement>('#spline-field-spacing')!.value),grids={forward:displacementReferenceGrid(nativeGeometry(loaded[moving]!),spacing),inverse:displacementReferenceGrid(nativeGeometry(loaded[fixed]!),spacing)}
 if(!await activity.begin('Exporting nonlinear displacement fields…','export'))return
 try{for(const direction of ['forward','inverse'] as const){const bytes=await runNonlinearTask<Uint8Array>({type:'field',reference:grids[direction],matrix:fit.matrix,spline:fit.nonlinear,direction},text=>{void activity.phase(text)});await saveArtifact(new Blob([gzipSync(bytes)],{type:'application/gzip'}),`from-${imageKey(direction==='forward'?moving:fixed)}_to-${imageKey(direction==='forward'?fixed:moving)}_${fit.nonlinear!.kernel==='jip-dense-pull'?(fit.jipRegistration?.settings.engine==='itk'?'itk':'jip'):'spline'}_displacement_RAS-mm.nii.gz`)}await saveArtifacts(createRegistrationArtifacts({appVersion:APP_VERSION,fit,landmarks,images:loaded,fieldExport:{requestedSpacingMm:spacing,forwardGrid:{dims:grids.forward.dims,affine:grids.forward.affine},inverseGrid:{dims:grids.inverse.dims,affine:grids.inverse.affine}}}));setStatus('Forward and inverse displacement fields saved in RAS millimeters. Use the JSON transform metadata for conventions.')}catch(error){setStatus(error instanceof Error?error.message:String(error),true)}finally{activity.end();automaticUI?.refresh();jipUI?.refresh();itkUI?.refresh();acquisitionUI?.refresh()}
 })().catch(error=>setStatus(error instanceof Error?error.message:String(error),true))})

function sessionSplineSmoothing(){const value=Number(document.querySelector<HTMLInputElement>('#spline-smoothing')!.value);return Number.isFinite(value)&&value>=0&&value<=10?value:(fitResult?.nonlinear?.kernel==='3d-polyharmonic-r'?fitResult.nonlinear.smoothing:undefined)??.01}
