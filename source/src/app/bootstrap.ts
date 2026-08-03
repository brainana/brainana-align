import '../ui/style.css'
import { Niivue, NVImage, SLICE_TYPE } from '@niivue/niivue'
import type { FitState, Landmark, Loaded, Modality, OptimizationWindows, Params6, Plane, ReviewView, View, WindowConstraint } from './types'
import { gzipSync } from 'fflate'
import { applyAffine, frame, invertAffine, sampleLinear, sampleNearest, type RawNifti } from '../domain/imaging/roiWarp'
import { installRuntimeIntegration } from '../infrastructure/runtime/integration'
import { applyMat4, multiplyMat4, rigidDelta, type Mat4, type Vec3 } from '../domain/registration/rigid'
import { diagnoseReflectionAlternative, fitLinearModel, invertAffineMatrix, linearDiagnostics, type LinearParameter, type TransformModel } from '../domain/registration/linear'
import { applyLandmarkWarpForward, applyLandmarkWarpReverse } from '../domain/registration/nonlinear'
import { applyForwardDeformation, applyInverseDeformation, optimizeDeformationGrid, type DeformableSettings, type DeformationGrid } from '../domain/registration/deformable'
import { downsampleByFactor, downsampleForRegistration, resolutionLevels, shrinkFactorLevels, type RegistrationResolutionPreset, type ShrinkFactorPreset } from '../domain/registration/imagePyramid'
import { optimizeAutomaticLinear, type AutomaticMetric, type RotationRange } from '../domain/registration/automaticLinear'
import { installNonlinearLandmarkControls, nonlinearLandmarkControlsHtml } from '../ui/nonlinearLandmarkControls'
import { linearRegistrationControlsHtml, selectedLinearParameters, transformModelLabel, updateTransformModelControls as updateLinearControls } from '../ui/linearRegistrationControls'
import { countOptimizationWindows, optimizationConstraint as getOptimizationConstraint, planeAxes, sanitizeOptimizationWindows, withinOptimizationWindows } from '../domain/optimization/windows'
import { installOptimizationWindowCapture } from '../viewer/optimizationWindowInteraction'
import { installHoverKeyboardPan, installProjectionRefresh, installWheelZoomAndSlice, isTypingTarget, panViewInScreenDirection, type HoveredView } from '../viewer/interaction'
import { canvasPointToImageOnCurrentSlice, clientPointToImageOnCurrentSlice, coordinateLabel, isFiniteVec3, planeDepthAxis } from '../viewer/coordinateProjection'
import { setOrthogonalCrosshairs, setReviewCrosshairs } from '../viewer/crosshairController'
import { renderModalityMarkers, renderReviewLandmarks } from '../viewer/landmarkRenderer'
import { VERSION as APP_VERSION } from '../version'
import { createSessionPayload, parseSessionPayload, sessionGeometryMismatches } from '../domain/sessions/persistence'
import { createRegistrationArtifacts, saveArtifact, saveArtifacts } from '../domain/export/artifacts'
import { detectBrowserCapabilities } from '../ui/browserCapabilities'
import { installBrowserCompatibilityBanner } from '../ui/browserCompatibility'
import { installWorkflowNavigation } from '../ui/workflowNavigation'

const planeTypes: Record<Plane, SLICE_TYPE> = {
  sagittal: SLICE_TYPE.SAGITTAL,
  coronal: SLICE_TYPE.CORONAL,
  axial: SLICE_TYPE.AXIAL,
}

const app = document.querySelector<HTMLDivElement>('#app')!
app.innerHTML = `
<header class="topbar">
  <div class="brand">Brainana Align <span>v0.17.26-workflow-ui.1</span></div>
  <div class="workflow-group image-loads">
    <label class="load compact-load" title="Load an MRI volume"><input id="mri-file" type="file" multiple accept=".nii,.nii.gz,.hdr,.img,.img.gz,.head,.brik,.brik.gz,.mgh,.mgz,.nrrd,.nhdr,.mif,.mha,.mhd,.raw,.v,.v16,.vmr,.npy,.npz,.fib,.src,.gz,application/gzip,application/x-gzip,application/octet-stream"><strong id="mri-name">Load MRI</strong></label>
    <label class="load compact-load" title="Load a CT volume"><input id="ct-file" type="file" multiple accept=".nii,.nii.gz,.hdr,.img,.img.gz,.head,.brik,.brik.gz,.mgh,.mgz,.nrrd,.nhdr,.mif,.mha,.mhd,.raw,.v,.v16,.vmr,.npy,.npz,.fib,.src,.gz,application/gzip,application/x-gzip,application/octet-stream"><strong id="ct-name">Load CT</strong></label>
  </div>
  <div class="toolbar-divider"></div>
  <div class="workflow-group session-actions">
    <button id="save-session" title="Save landmarks and registration state">Save session</button>
    <label class="session-load" title="Restore a saved Brainana Align session"><input id="load-session" type="file" accept=".json,application/json"><span>Load session</span></label>
  </div>
  <div class="toolbar-divider"></div>
  <div class="workflow-group registration-actions">
    <button id="reset-view" title="Reset zoom and field of view">Reset view</button>
    <button id="export-open" disabled>Export</button>
  </div>
</header>
<nav class="workflow-tabs" role="tablist" aria-label="Alignment workflow">
  <div class="workflow-nav-group landmark-guided" role="presentation">
    <span class="workflow-group-label">Landmark guided</span><div class="workflow-group-tabs" role="presentation">
      <button id="tab-landmarks" role="tab" data-workflow-tab="landmarks" class="active" aria-selected="true" aria-controls="panel-landmarks"><strong>1 · Points</strong><small id="landmark-stage-status">Create pairs</small></button>
      <button id="tab-linear" role="tab" data-workflow-tab="linear" aria-selected="false" aria-controls="panel-linear"><strong>2 · Linear fit</strong><small id="linear-stage-status">Not fitted</small></button>
      <button id="tab-landmark-warp" role="tab" data-workflow-tab="landmark-warp" aria-selected="false" aria-controls="panel-landmark-warp"><strong>3 · Nonlinear warp</strong><small id="landmark-warp-stage-status">Optional</small></button>
    </div></div>
  <div class="workflow-nav-group image-driven" role="presentation">
    <span class="workflow-group-label">Image driven</span><div class="workflow-group-tabs" role="presentation">
      <button id="tab-intensity" role="tab" data-workflow-tab="intensity" aria-selected="false" aria-controls="panel-intensity"><strong>Linear intensity</strong><small id="intensity-stage-status">Global or local</small></button>
      <button id="tab-deformable" role="tab" data-workflow-tab="deformable" aria-selected="false" aria-controls="panel-deformable"><strong>Deformable</strong><small id="deformable-stage-status">After global fit</small></button>
    </div></div>
  <div class="workflow-nav-group output-group" role="presentation">
    <span class="workflow-group-label">Output</span><div class="workflow-group-tabs" role="presentation">
      <button id="tab-review" role="tab" data-workflow-tab="review" aria-selected="false" aria-controls="panel-review"><strong>Review</strong><small id="review-stage-status">Waiting for fit</small></button>
    </div></div>
</nav>
<main>
  <section class="viewer-region">
  <section class="workspace">
    <div class="column-head"></div><div class="column-head">Sagittal</div><div class="column-head">Coronal</div><div class="column-head">Axial</div>
    ${(['mri','ct'] as Modality[]).map(modality => `
      <div class="row-head"><strong>${modality.toUpperCase()}</strong><small id="${modality}-coords">x — y — z —</small></div>
      ${(['sagittal','coronal','axial'] as Plane[]).map(plane => `<div class="view-card" data-modality="${modality}" data-plane="${plane}"><canvas id="${modality}-${plane}"></canvas><svg class="marker-overlay" id="${modality}-${plane}-overlay"></svg><div class="optimization-window-layer" id="${modality}-${plane}-window-layer"></div><div class="image-placeholder" id="${modality}-${plane}-placeholder"></div></div>`).join('')}
    `).join('')}
    <div class="row-head review-head"><strong>OVERLAY</strong><small id="review-label">Fit a transform to review alignment</small></div>
    ${(['sagittal','coronal','axial'] as Plane[]).map(plane => `<div class="view-card review-card" data-plane="${plane}"><canvas id="review-${plane}"></canvas><svg class="marker-overlay" id="review-${plane}-overlay"></svg><div class="review-placeholder">Fit a landmark transform to display the aligned overlay</div></div>`).join('')}
  </section>
  <footer class="viewer-footer">
    <div class="transform-stack" aria-label="Accepted transform stack"><span class="footer-label">Accepted alignment</span><span id="stack-linear" class="transform-chip">No linear fit</span><span id="stack-landmark-warp" class="transform-chip inactive">Landmark warp</span><span id="stack-intensity" class="transform-chip inactive">Intensity</span><span id="stack-deformable" class="transform-chip inactive">Deformable</span></div>
    <div class="display-controls"><label>Display<select id="review-mode"><option value="falsecolor">Green–magenta</option><option value="opacity">Grayscale opacity</option></select></label><label>Opacity <span id="opacity-value">55%</span><input id="review-opacity" type="range" min="0" max="100" value="55"></label></div>
  </footer>
  </section>
  <aside class="sidebar" aria-label="Selected workflow stage">
    <section class="workflow-panel active" id="panel-landmarks" data-workflow-panel="landmarks" role="tabpanel" aria-labelledby="tab-landmarks">
      <div class="stage-header"><div><h2>Landmarks</h2><p>Create and review corresponding MRI and CT points.</p></div><div class="stage-header-actions"><span id="pair-count" class="stage-badge">0 pairs</span><div class="stage-action-buttons"><button id="new-pair" class="primary" title="Create a corresponding MRI and CT point">New landmark</button><button id="undo" title="Undo the last landmark edit" aria-label="Undo last landmark edit">Undo</button></div></div></div>
      <div class="panel-body">
        <p class="hint">Create a pair, navigate each image, then set its MRI and CT positions. Drag a visible marker to adjust it within the current slice.</p>
        <div id="selected-landmark-summary" class="selected-summary">No landmark selected</div>
        <div id="landmark-list" class="landmark-list"></div>
      </div>
    </section>
    <section class="workflow-panel" id="panel-linear" data-workflow-panel="linear" role="tabpanel" aria-labelledby="tab-linear">
      <div class="stage-header"><div><h2>Linear landmark fit</h2><p>Choose a transform model and fit all enabled landmark pairs.</p></div></div>
      <div class="panel-body">
        <label>Transform direction<select id="direction"><option value="ct-mri">CT → MRI</option><option value="mri-ct">MRI → CT</option></select></label>
        ${linearRegistrationControlsHtml}
        <button id="fit-side" class="primary">Fit rigid transform</button>
        <button id="reset-alignment" disabled>Reset alignment</button>
        <button id="restore-alignment-landmarks" disabled>Restore alignment landmarks</button>
        <div id="fit-summary" class="summary">At least three complete, enabled landmark pairs are required.</div>
        <label class="check"><input id="link-after" type="checkbox" checked> Link navigation after fitting</label>
      </div>
    </section>
    <details class="workflow-panel workflow-panel-secondary" data-workflow-panel="linear" id="manual-panel">
      <summary><span>Manual adjustment</span></summary>
      <div class="panel-body">
        <p class="hint">Nudges move the current moving image only. Translations use fixed-image world axes; rotations occur about the fixed-image center.</p>
        <div class="step-grid"><label>Translation step<select id="translation-step"><option value="0.1">0.1 mm</option><option value="0.5" selected>0.5 mm</option><option value="1">1 mm</option><option value="2">2 mm</option><option value="5">5 mm</option></select></label><label>Rotation step<select id="rotation-step"><option value="0.1">0.1°</option><option value="0.25">0.25°</option><option value="0.5" selected>0.5°</option><option value="1">1°</option><option value="2">2°</option></select></label></div>
        <div id="nudge-controls" class="nudge-controls">
          <div class="nudge-row"><span>X · L/R</span><button data-param="0" data-sign="-1">◀</button><output id="param-0">0.0 mm</output><button data-param="0" data-sign="1">▶</button></div>
          <div class="nudge-row"><span>Y · P/A</span><button data-param="1" data-sign="-1">◀</button><output id="param-1">0.0 mm</output><button data-param="1" data-sign="1">▶</button></div>
          <div class="nudge-row"><span>Z · I/S</span><button data-param="2" data-sign="-1">▼</button><output id="param-2">0.0 mm</output><button data-param="2" data-sign="1">▲</button></div>
          <div class="nudge-row"><span>Rotate X</span><button data-param="3" data-sign="-1">↶</button><output id="param-3">0.0°</output><button data-param="3" data-sign="1">↷</button></div>
          <div class="nudge-row"><span>Rotate Y</span><button data-param="4" data-sign="-1">↶</button><output id="param-4">0.0°</output><button data-param="4" data-sign="1">↷</button></div>
          <div class="nudge-row"><span>Rotate Z</span><button data-param="5" data-sign="-1">↶</button><output id="param-5">0.0°</output><button data-param="5" data-sign="1">↷</button></div>
        </div>
        <button id="reset-manual" disabled>Reset manual nudges</button><button id="reset-landmark" disabled>Reset to landmark fit</button>
      </div>
    </details>
    <section class="workflow-panel" id="panel-landmark-warp" data-workflow-panel="landmark-warp" role="tabpanel" aria-labelledby="tab-landmark-warp">
      <div class="stage-header"><div><h2>Landmark warp</h2><p>Optionally interpolate a smooth nonlinear deformation from the landmark residuals.</p></div></div>
      <div class="panel-body">${nonlinearLandmarkControlsHtml}</div>
    </section>
    <section class="workflow-panel" data-workflow-panel="intensity" id="panel-intensity" role="tabpanel" aria-labelledby="tab-intensity">
      <div class="stage-header"><div><h2>Automatic linear alignment</h2><p>Find a global intensity-based alignment, or locally refine an accepted transform.</p></div></div>
      <div class="panel-body">
        <label>Operation<select id="intensity-operation"><option value="global" selected>Global automatic alignment</option><option value="local">Local rigid refinement</option></select></label>
        <div id="global-linear-settings">
          <label>Transform model<select id="auto-linear-model"><option value="rigid">Rigid · 6 parameters</option><option value="similarity">Similarity · 7 parameters</option><option value="anisotropic">Anisotropic scale · 9 parameters</option><option value="affine">Affine · 12 parameters</option><option value="custom">Custom parameter groups</option></select></label>
          <div id="auto-custom" class="hidden"><label class="check"><input id="auto-scale-x" type="checkbox"> Scale X</label><label class="check"><input id="auto-scale-y" type="checkbox"> Scale Y</label><label class="check"><input id="auto-scale-z" type="checkbox"> Scale Z</label><label class="check"><input id="auto-shear" type="checkbox"> Shear XY, XZ, YZ</label></div>
          <label>Cost function<select id="auto-metric"><option value="nmi" selected>Normalized mutual information</option><option value="mi">Mutual information</option><option value="correlation-ratio">Correlation ratio</option><option value="normalized-correlation">Normalized correlation</option><option value="least-squares">Least squares</option></select></label>
          <label id="auto-bins-row">Histogram bins<select id="auto-bins"><option value="auto" selected>Auto</option><option value="32">32</option><option value="64">64</option><option value="128">128</option></select></label>
          <div class="step-grid"><label>Rotation X<select id="auto-range-x"><option value="0">Header only</option><option value="15">±15°</option><option value="30">±30°</option><option value="90" selected>±90°</option><option value="180">Full orientation</option></select></label><label>Rotation Y<select id="auto-range-y"><option value="0">Header only</option><option value="15">±15°</option><option value="30">±30°</option><option value="90" selected>±90°</option><option value="180">Full orientation</option></select></label><label>Rotation Z<select id="auto-range-z"><option value="0">Header only</option><option value="15">±15°</option><option value="30">±30°</option><option value="90" selected>±90°</option><option value="180">Full orientation</option></select></label></div>
          <details class="nested"><summary>Advanced global search</summary><div class="step-grid"><label>Translation limit<input id="auto-translation-limit" type="number" min="1" step="1" value="30"> mm</label><label>Scale limit<input id="auto-scale-limit" type="number" min="0" max="1" step="0.05" value="0.35"> fraction</label><label>Shear limit<input id="auto-shear-limit" type="number" min="0" max="1" step="0.05" value="0.3"></label></div></details>
          <button id="auto-linear" disabled class="primary">Run global automatic alignment</button>
        </div>
        <div id="local-linear-settings" class="hidden"><label>Search preset<select id="refine-bounds"><option value="tight">Tight</option><option value="standard" selected>Standard</option><option value="custom">Custom</option></select></label></div>
        <div class="global-intensity-setting">
          <label class="parameter-help" tabindex="0" data-help="Shrink factors are relative to each input image grid. 1 uses the native grid; 8,4,2,1 runs coarse to fine without assuming a species or physical brain size.">Registration pyramid<select id="registration-resolution"><option value="fast">Fast · 4× → 2× → 1×</option><option value="standard" selected>Standard · 8× → 4× → 2× → 1×</option><option value="custom">Custom shrink factors…</option></select></label>
          <label id="registration-resolution-custom-row" class="parameter-help hidden" tabindex="0" data-help="Comma-separated integer input-grid shrink factors, coarse to fine. End in 1 for native resolution; for example 8,4,2,1.">Custom shrink factors<input id="registration-resolution-custom" value="8,4,2,1" inputmode="numeric"></label>
          <label>Smoothing<select id="auto-smoothing-mode"><option value="auto" selected>Auto for each level</option><option value="custom">Custom native-voxel σ…</option></select></label>
          <label id="auto-smoothing-custom-row" class="parameter-help hidden" tabindex="0" data-help="Gaussian sigma in native input voxels for each pyramid level. Supply one value per shrink factor.">Custom smoothing σ<input id="auto-smoothing-custom" value="4,2,1,0" inputmode="decimal"></label>
          <label>Optimization sweeps<select id="auto-sweeps-mode"><option value="auto" selected>Auto schedule</option><option value="custom">Custom per level…</option></select></label>
          <label id="auto-sweeps-custom-row" class="parameter-help hidden" tabindex="0" data-help="Maximum complete parameter sweeps at each level. Supply one positive integer per shrink factor.">Custom sweeps<input id="auto-sweeps-custom" value="2,4,4,2" inputmode="numeric"></label>
        </div>
        <label class="local-intensity-setting hidden">Local refinement resolution<select id="local-registration-resolution"><option value="fast">Fast · 4 mm</option><option value="balanced" selected>Balanced · 4 → 2 mm</option><option value="precise">Precise · 4 → 2 → native</option></select></label>
        <div class="step-grid refine-grid local-intensity-setting hidden"><label>Translation limit<input id="refine-translation-limit" type="number" min="0.1" step="0.1" value="5"> mm</label><label>Rotation limit<input id="refine-rotation-limit" type="number" min="0.1" step="0.1" value="10"> °</label></div>
        <details class="nested local-intensity-setting hidden"><summary>Advanced local settings</summary><div class="step-grid refine-grid"><label>Translation start step<input id="refine-translation-step" type="number" min="0.05" step="0.05" value="2"> mm</label><label>Rotation start step<input id="refine-rotation-step" type="number" min="0.05" step="0.05" value="1"> °</label></div></details>
        <div class="optimization-window-controls">
          <label>Optimization windows<select id="window-target"><option value="both">MRI and CT</option><option value="mri">MRI only</option><option value="ct">CT only</option></select></label>
          <button id="define-windows">Define windows</button>
          <button id="clear-windows">Clear windows</button>
          <p id="window-summary" class="hint">No optimization windows defined. The full geometric overlap will be used.</p>
        </div>
        <button id="refine" class="hidden" disabled>Refine current alignment (rigid)</button>
        <div id="refine-summary" class="summary">Fits locally around the current transform using normalized mutual information and geometrically overlapping voxels only.</div>
        <div id="refine-actions" class="refine-actions hidden"><button id="accept-refine" class="primary">Accept refinement</button><button id="reject-refine">Reject</button></div>
      </div>
    </section>
    <section class="workflow-panel" id="panel-deformable" data-workflow-panel="deformable" role="tabpanel" aria-labelledby="tab-deformable">
      <div class="stage-header"><div><h2>Deformable refinement</h2><p>Generate and review a smooth intensity-driven deformation proposal.</p></div></div>
      <div class="panel-body">
        <p class="hint">Fits a smooth target-grid deformation with NMI after the current linear and optional landmark warp. The current optimization windows are respected.</p>
        <label class="parameter-help" tabindex="0" data-help="Controls the overall deformation strength. Conservative favors smooth small changes; Flexible allows finer, larger local changes; Custom exposes every setting.">Deformable preset<select id="deform-preset"><option value="conservative">Conservative</option><option value="balanced" selected>Balanced</option><option value="flexible">Flexible</option><option value="custom">Custom</option></select></label>
        <label class="parameter-help" tabindex="0" data-help="Controls only the resolution used to calculate deformable registration. The accepted deformation remains in physical coordinates and can be applied during native-resolution export.">Registration resolution<select id="deform-registration-resolution"><option value="fast">Fast · 4 mm</option><option value="balanced" selected>Balanced · 4 → 2 mm</option><option value="precise">Precise · 4 → 2 → native</option></select></label>
        <div id="deform-custom" class="refine-grid hidden"><label class="parameter-help" tabindex="0" data-help="Comma-separated control-grid spacings from coarse to fine, in millimeters. Smaller final spacing models more localized anatomy but increases runtime and overfitting risk.">Grid levels (mm)<input id="deform-levels" value="40,20"></label><label class="parameter-help" tabindex="0" data-help="Penalizes differences between neighboring control-point displacements. Larger values produce smoother, more conservative warps; zero removes this smoothness penalty.">Regularization<input id="deform-regularization" type="number" min="0" step="0.01" value="0.05"></label><label class="parameter-help" tabindex="0" data-help="Maximum displacement allowed at any control point, in millimeters. Larger values permit stronger warps but increase anatomical-distortion risk.">Maximum displacement<input id="deform-max" type="number" min="0.1" step="0.5" value="5"> mm</label><label class="parameter-help" tabindex="0" data-help="Number of coordinate-search passes at each grid level. More iterations can improve convergence but increase runtime and may overfit.">Iterations per level<input id="deform-iterations" type="number" min="1" max="4" step="1" value="1"></label><label class="parameter-help" tabindex="0" data-help="Lowest permitted local Jacobian determinant. Values near zero indicate severe compression; nonpositive values indicate folding. Raising this limit is more conservative.">Minimum Jacobian<input id="deform-jac-min" type="number" min="0.05" step="0.05" value="0.5"></label><label class="parameter-help" tabindex="0" data-help="Highest permitted local Jacobian determinant. Large values indicate strong local expansion. Lowering this limit is more conservative.">Maximum Jacobian<input id="deform-jac-max" type="number" min="1" step="0.1" value="2"></label></div>
        <button id="deform-refine" disabled>Run deformable refinement</button><button id="cancel-deform" class="hidden">Cancel deformable refinement</button><button id="reset-deform" disabled>Reset deformable refinement</button>
        <div id="deform-summary" class="summary">No deformable refinement has been applied.</div>
        <div id="deform-actions" class="refine-actions hidden"><button id="accept-deform" class="primary">Accept proposal</button><button id="reject-deform">Reject</button></div>
      </div>
    </section>
    <section class="workflow-panel" id="panel-review" data-workflow-panel="review" role="tabpanel" aria-labelledby="tab-review">
      <div class="stage-header"><div><h2>Review and export</h2><p>Inspect the accepted transform stack and save the final registration.</p></div></div>
      <div class="panel-body">
        <div id="review-diagnostics" class="summary">Fit a landmark transform to enable final review and export.</div>
        <button id="review-export" disabled class="primary">Export accepted alignment</button>
        <p class="hint interaction-hint"><strong>Zoom:</strong> wheel or two-finger scroll. <strong>Change slices:</strong> Shift + scroll. <strong>Pan:</strong> arrow keys or drag empty image space.</p>
      </div>
    </section>
  </aside>
</main>
<span id="status" class="visually-hidden" role="status" aria-live="polite">Ready.</span>
<div id="activity-overlay" class="activity-overlay hidden" role="status" aria-live="polite" aria-atomic="true">
  <div class="activity-card"><div class="activity-spinner" aria-hidden="true"></div><div id="activity-text">Working…</div></div>
</div>
<div id="status-toast" class="status-toast hidden" role="alert" aria-live="assertive"><span id="status-toast-text"></span><button id="status-toast-close" aria-label="Dismiss status">×</button></div>
<div id="parameter-tooltip" class="parameter-tooltip hidden" role="tooltip"></div>
<div id="export-modal" class="modal hidden" role="dialog" aria-modal="true" aria-labelledby="export-title">
  <div class="modal-card">
    <div class="modal-head"><h2 id="export-title">Export registration</h2><button id="export-close" aria-label="Close">×</button></div>
    <label>Output interpolation<select id="output-interpolation"><option value="linear" selected>Linear · MRI/CT intensities</option><option value="nearest">Nearest neighbor · labels/masks</option></select></label>
    <p class="hint">Optimization always uses trilinear interpolation for a smoother objective. This setting controls only display/export resampling on the selected reference grid.</p>
    <button id="save-aligned" disabled>Save aligned image</button>
    <button id="save-transform" disabled>Save transforms and landmarks</button>
    <button id="save-session-export">Save reloadable session</button>
  </div>
</div>
<div id="handedness-modal" class="modal hidden" role="dialog" aria-modal="true" aria-labelledby="handedness-title">
  <div class="modal-card handedness-card">
    <div class="modal-head"><h2 id="handedness-title">Landmark handedness conflict</h2><button id="handedness-close" aria-label="Close">×</button></div>
    <div id="handedness-detail" class="handedness-detail"></div>
    <p class="hint">The diagnostic can identify a likely crossed pair, but it cannot determine automatically whether the MRI or CT assignments are reversed. Choose the image whose two points should be exchanged, or review them manually.</p>
    <div class="handedness-actions">
      <button id="handedness-swap-ct" class="primary">Swap the two CT points</button>
      <button id="handedness-swap-mri">Swap the two MRI points</button>
      <button id="handedness-review">Review landmarks manually</button>
      <button id="handedness-reflection" class="danger-action">Enable reflection instead</button>
    </div>
  </div>
</div>
`

const workflowNavigation = installWorkflowNavigation()
const statusEl = document.querySelector<HTMLSpanElement>('#status')!
const landmarkList = document.querySelector<HTMLDivElement>('#landmark-list')!
const fitSummary = document.querySelector<HTMLDivElement>('#fit-summary')!
const directionSelect = document.querySelector<HTMLSelectElement>('#direction')!
const transformModelSelect = document.querySelector<HTMLSelectElement>('#transform-model')!
const customParameters = document.querySelector<HTMLDivElement>('#custom-parameters')!
const allowReflection = document.querySelector<HTMLInputElement>('#allow-reflection')!
const fitButton = document.querySelector<HTMLButtonElement>('#fit-side')!
const resetAlignment = document.querySelector<HTMLButtonElement>('#reset-alignment')!
const linkAfter = document.querySelector<HTMLInputElement>('#link-after')!
const saveAligned = document.querySelector<HTMLButtonElement>('#save-aligned')!
const saveTransform = document.querySelector<HTMLButtonElement>('#save-transform')!
const reviewMode = document.querySelector<HTMLSelectElement>('#review-mode')!
const reviewOpacity = document.querySelector<HTMLInputElement>('#review-opacity')!
const opacityValue = document.querySelector<HTMLSpanElement>('#opacity-value')!
const translationStep = document.querySelector<HTMLSelectElement>('#translation-step')!
const rotationStep = document.querySelector<HTMLSelectElement>('#rotation-step')!
const resetManual = document.querySelector<HTMLButtonElement>('#reset-manual')!
const resetLandmark = document.querySelector<HTMLButtonElement>('#reset-landmark')!
const refineButton = document.querySelector<HTMLButtonElement>('#refine')!
const automaticLinearButton = document.querySelector<HTMLButtonElement>('#auto-linear')!
const intensityOperation = document.querySelector<HTMLSelectElement>('#intensity-operation')!
const globalLinearSettings = document.querySelector<HTMLDivElement>('#global-linear-settings')!
const localLinearSettings = document.querySelector<HTMLDivElement>('#local-linear-settings')!
const automaticModel = document.querySelector<HTMLSelectElement>('#auto-linear-model')!
const automaticMetric = document.querySelector<HTMLSelectElement>('#auto-metric')!
const automaticBins = document.querySelector<HTMLSelectElement>('#auto-bins')!
const deformButton = document.querySelector<HTMLButtonElement>('#deform-refine')!
const cancelDeformButton = document.querySelector<HTMLButtonElement>('#cancel-deform')!
const resetDeformButton = document.querySelector<HTMLButtonElement>('#reset-deform')!
const deformPreset = document.querySelector<HTMLSelectElement>('#deform-preset')!
const deformRegistrationResolution = document.querySelector<HTMLSelectElement>('#deform-registration-resolution')!
const deformCustom = document.querySelector<HTMLDivElement>('#deform-custom')!
const refineBounds = document.querySelector<HTMLSelectElement>('#refine-bounds')!
const registrationResolution = document.querySelector<HTMLSelectElement>('#registration-resolution')!
const registrationResolutionCustomRow = document.querySelector<HTMLLabelElement>('#registration-resolution-custom-row')!
const registrationResolutionCustom = document.querySelector<HTMLInputElement>('#registration-resolution-custom')!
const localRegistrationResolution = document.querySelector<HTMLSelectElement>('#local-registration-resolution')!
const automaticSmoothingMode = document.querySelector<HTMLSelectElement>('#auto-smoothing-mode')!
const automaticSmoothingCustomRow = document.querySelector<HTMLLabelElement>('#auto-smoothing-custom-row')!
const automaticSmoothingCustom = document.querySelector<HTMLInputElement>('#auto-smoothing-custom')!
const automaticSweepsMode = document.querySelector<HTMLSelectElement>('#auto-sweeps-mode')!
const automaticSweepsCustomRow = document.querySelector<HTMLLabelElement>('#auto-sweeps-custom-row')!
const automaticSweepsCustom = document.querySelector<HTMLInputElement>('#auto-sweeps-custom')!
const refineTranslationLimit = document.querySelector<HTMLInputElement>('#refine-translation-limit')!
const refineRotationLimit = document.querySelector<HTMLInputElement>('#refine-rotation-limit')!
const refineTranslationStartStep = document.querySelector<HTMLInputElement>('#refine-translation-step')!
const refineRotationStartStep = document.querySelector<HTMLInputElement>('#refine-rotation-step')!
const refineSummary = document.querySelector<HTMLDivElement>('#refine-summary')!
const refineActions = document.querySelector<HTMLDivElement>('#refine-actions')!
const acceptRefine = document.querySelector<HTMLButtonElement>('#accept-refine')!
const rejectRefine = document.querySelector<HTMLButtonElement>('#reject-refine')!
const deformSummary = document.querySelector<HTMLDivElement>('#deform-summary')!
const deformActions = document.querySelector<HTMLDivElement>('#deform-actions')!
const acceptDeform = document.querySelector<HTMLButtonElement>('#accept-deform')!
const rejectDeform = document.querySelector<HTMLButtonElement>('#reject-deform')!
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
let reviewBuildTimer: number | null = null
let history: Landmark[][] = []
let suppressLinkedUpdate = false
let suppressReviewUpdate = false
const suppressCrosshairSync: Record<Modality, boolean> = { mri: false, ct: false }
let reviewImages: { fixed: NVImage; moving: NVImage; fixedModality: Modality; movingModality: Modality } | null = null
let optimizationWindows: OptimizationWindows = { mri: {}, ct: {} }
let definingWindows = false
const activityOverlay = document.querySelector<HTMLDivElement>('#activity-overlay')!
const activityText = document.querySelector<HTMLDivElement>('#activity-text')!
const statusToast = document.querySelector<HTMLDivElement>('#status-toast')!
const statusToastText = document.querySelector<HTMLSpanElement>('#status-toast-text')!
let processingActive = false
let processingError = false
let processingAutomatic = false
let toastTimer: number | null = null

function showStatusToast(text: string, error = false): void {
  statusToastText.textContent = text
  statusToast.classList.toggle('error', error)
  statusToast.classList.remove('hidden')
  if (toastTimer !== null) window.clearTimeout(toastTimer)
  toastTimer = window.setTimeout(() => statusToast.classList.add('hidden'), error ? 12000 : 5000)
}

function beginProcessing(text: string): void {
  processingActive = true
  processingError = false
  processingAutomatic = false
  activityText.textContent = text
  activityOverlay.classList.remove('hidden')
  document.querySelector('main')?.setAttribute('aria-busy', 'true')
  setStatus(text)
}

/** Allow the browser to paint a newly displayed progress overlay before synchronous work begins. */
async function yieldForPaint(): Promise<void> {
  await new Promise<void>(resolve => requestAnimationFrame(() => requestAnimationFrame(() => resolve())))
}

function endProcessing(): void {
  if (!processingActive) return
  const finalText = statusEl.textContent || 'Processing finished.'
  processingActive = false
  processingAutomatic = false
  activityOverlay.classList.add('hidden')
  document.querySelector('main')?.removeAttribute('aria-busy')
  showStatusToast(finalText, processingError)
}

function setStatus(text: string, error = false) {
  const processingCue = !error && /(?:…|\.\.\.)$/.test(text.trim())
  if (!processingActive && processingCue) {
    processingActive = true
    processingAutomatic = true
    processingError = false
    activityOverlay.classList.remove('hidden')
    document.querySelector('main')?.setAttribute('aria-busy', 'true')
  }
  statusEl.textContent = text
  statusEl.title = text
  statusEl.classList.toggle('error', error)
  if (processingActive) {
    activityText.textContent = text
    processingError ||= error
    if (processingAutomatic && (!processingCue || error)) window.setTimeout(endProcessing, 0)
  } else showStatusToast(text, error)
}

document.querySelector('#status-toast-close')!.addEventListener('click', () => statusToast.classList.add('hidden'))

function installParameterHelpTooltips(): void {
  const tooltip = document.querySelector<HTMLDivElement>('#parameter-tooltip')!
  let active: HTMLElement | null = null
  const hide = () => { active = null; tooltip.classList.add('hidden'); tooltip.textContent = '' }
  const show = (target: HTMLElement) => {
    const help = target.dataset.help
    if (!help) return
    active = target
    tooltip.textContent = help
    tooltip.classList.remove('hidden')
    const targetRect = target.getBoundingClientRect()
    const tooltipRect = tooltip.getBoundingClientRect()
    const left = Math.max(12, Math.min(window.innerWidth - tooltipRect.width - 12, targetRect.left))
    const above = targetRect.top - tooltipRect.height - 8
    const top = above >= 12 ? above : Math.min(window.innerHeight - tooltipRect.height - 12, targetRect.bottom + 8)
    tooltip.style.left = `${left}px`
    tooltip.style.top = `${top}px`
  }
  document.querySelectorAll<HTMLElement>('.parameter-help[data-help]').forEach(target => {
    target.addEventListener('pointerenter', () => show(target))
    target.addEventListener('pointerleave', hide)
    target.addEventListener('focusin', () => show(target))
    target.addEventListener('focusout', hide)
  })
  document.addEventListener('pointerdown', event => { if (active && !active.contains(event.target as Node)) hide() })
  document.addEventListener('keydown', event => { if (event.key === 'Escape') hide() })
  window.addEventListener('resize', hide)
  window.addEventListener('scroll', hide, true)
}

installParameterHelpTooltips()

function setImagePlaceholder(modality: Modality, mode: 'empty' | 'loading' | 'hidden', detail?: string) {
  const label = modality.toUpperCase()
  for (const plane of ['sagittal','coronal','axial'] as Plane[]) {
    const placeholder = document.querySelector<HTMLDivElement>(`#${modality}-${plane}-placeholder`)!
    placeholder.classList.toggle('hidden', mode === 'hidden')
    placeholder.classList.toggle('loading', mode === 'loading')
    if (mode === 'empty') placeholder.textContent = ''
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
      const nv = new Niivue({ show3Dcrosshair: false, isColorbar: false, loadingText: '' })
      views[modality][plane] = { nv, modality, plane, canvas, overlay, windowLayer }
      nv.attachToCanvas(canvas)
      nv.setSliceMM(true)
      nv.setDragMode('pan')
      nv.setSliceType(planeTypes[plane])
      nv.setCrosshairColor(modality === 'mri' ? [0.25,0.85,1,1] : [1,0.7,0.15,1])
      nv.onLocationChange = (loc: any) => handleLocation(modality, loc?.mm, plane)
      installWheelZoomAndSlice(canvas, nv, () => loaded[modality]?.raw.dims ?? null, plane, mm => gotoMm(modality, mm))
      installHoverKeyboardPan(canvas, nv, () => renderMarkers(modality), view => { hoveredView = view })
      installProjectionRefresh(canvas, () => renderMarkers(modality))
      const card = canvas.closest<HTMLElement>('.view-card')!
      installWindowCapture(views[modality][plane])
      window.addEventListener('resize', () => renderMarkers(modality))
    }
  }
}

function setupReviewViews() {
  for (const plane of ['sagittal','coronal','axial'] as Plane[]) {
    const canvas = document.querySelector<HTMLCanvasElement>(`#review-${plane}`)!
    const overlay = document.querySelector<SVGSVGElement>(`#review-${plane}-overlay`)!
    const nv = new Niivue({ show3Dcrosshair: false, isColorbar: false, loadingText: '' })
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
        const movingMm = mapFixedToMoving(mm)
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

function rawFromNVImage(image: NVImage): RawNifti {
  if (!image.img) throw new Error('The selected image contains no voxel data')
  const dimsSource = image.hdr?.dims ?? image.dims
  if (!dimsSource || dimsSource.length < 4) throw new Error('Unable to determine image dimensions')
  const dims: [number, number, number] = [Number(dimsSource[1]), Number(dimsSource[2]), Number(dimsSource[3])]
  if (!dims.every(v => Number.isFinite(v) && v > 0)) throw new Error('Invalid image dimensions')
  const frameCount = Math.max(1, Number(dimsSource[4]) || 1)
  const affine = image.getAffine().map(row => row.map(Number))
  const pix = image.hdr?.pixDims ?? image.pixDims
  const pixDims: [number, number, number] = [
    Math.abs(Number(pix?.[1] ?? 1)),
    Math.abs(Number(pix?.[2] ?? 1)),
    Math.abs(Number(pix?.[3] ?? 1)),
  ]
  const slope = Number(image.hdr?.scl_slope) || 1
  const intercept = Number(image.hdr?.scl_inter) || 0
  const source = image.img as ArrayLike<number>
  const expected = dims[0] * dims[1] * dims[2] * frameCount
  if (source.length < expected) throw new Error(`Image payload is truncated: expected ${expected} voxels, found ${source.length}`)
  const values = new Float32Array(expected)
  for (let i = 0; i < expected; i += 1) values[i] = Number(source[i]) * slope + intercept
  return {
    dims,
    frameCount,
    affine,
    pixDims,
    values,
    datatypeCode: Number(image.hdr?.datatypeCode) || 16,
    littleEndian: Boolean(image.hdr?.littleEndian ?? true),
    slope,
    intercept,
  }
}

async function loadFiles(modality: Modality, files: File[]) {
  if (!files.length) return
  if (fitResult) clearFit()
  if (!files.some(file => matchesSupportedExtension(file.name))) {
    throw new Error('Unsupported image format. Select a standard volumetric neuroimaging file.')
  }
  const displayName = files.length === 1 ? files[0].name : `${files[0].name} + ${files.length - 1} paired file${files.length === 2 ? '' : 's'}`
  const hadExistingImage = Boolean(loaded[modality])
  setImagePlaceholder(modality, 'loading', displayName)
  beginProcessing(`Loading ${displayName}…`)
  try {
    const nvImage = await NVImage.loadFromFile({ file: files.length === 1 ? files[0] : files, name: files[0].name })
    const raw = rawFromNVImage(nvImage)
    loaded[modality] = { name: displayName, raw, nvImage, sourceFiles: files.map(file => file.name) }
    document.querySelector<HTMLElement>(`#${modality}-name`)!.textContent = displayName
    for (const view of Object.values(views[modality])) {
      while (view.nv.volumes.length) view.nv.removeVolumeByIndex(0)
      view.nv.addVolume(nvImage.clone())
    }
    currentMm[modality] = applyAffine(raw.affine, [(raw.dims[0]-1)/2,(raw.dims[1]-1)/2,(raw.dims[2]-1)/2])
    gotoMm(modality, currentMm[modality]!)
    renderMarkers(modality)
    setImagePlaceholder(modality, 'hidden')
    automaticLinearButton.disabled = !(loaded.mri && loaded.ct)
    setStatus(`${modality.toUpperCase()} loaded: ${raw.dims.join(' × ')} voxels from ${files.length} file${files.length === 1 ? '' : 's'}.`)
  } catch (error) {
    setImagePlaceholder(modality, hadExistingImage ? 'hidden' : 'empty')
    setStatus(error instanceof Error ? error.message : String(error), true)
    throw error
  } finally {
    endProcessing()
  }
}

function handleLocation(modality: Modality, mm: number[] | undefined, sourcePlane?: Plane) {
  if (!isFiniteVec3(mm)) return
  const nextMm: Vec3 = [mm[0], mm[1], mm[2]]
  currentMm[modality] = nextMm

  if (!suppressCrosshairSync[modality]) {
    suppressCrosshairSync[modality] = true
    setOrthogonalCrosshairs(modality, views[modality], nextMm, sourcePlane)
    requestAnimationFrame(() => { suppressCrosshairSync[modality] = false })
  } else {
    document.querySelector(`#${modality}-coords`)!.textContent = coordinateLabel(nextMm)
  }

  renderMarkers(modality)
  if (reviewImages && modality === reviewImages.fixedModality && !suppressReviewUpdate) gotoReviewMm(nextMm)
  if (fitResult && linkAfter.checked && !suppressLinkedUpdate) {
    const other: Modality = modality === 'mri' ? 'ct' : 'mri'
    if (!loaded[other]) return
    const movingModality: Modality = fitResult.direction === 'ct-mri' ? 'ct' : 'mri'
    const mapped = modality === movingModality ? mapMovingToFixed(nextMm) : mapFixedToMoving(nextMm)
    suppressLinkedUpdate = true
    gotoMm(other, mapped)
    requestAnimationFrame(() => { suppressLinkedUpdate = false })
  }
}

function gotoMm(modality: Modality, mm: Vec3) {
  currentMm[modality] = mm
  setOrthogonalCrosshairs(modality, views[modality], mm)
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
    loaded: loaded[modality],
    currentMm: currentMm[modality],
    landmarks,
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
  const sourceMod: Modality = fitResult.direction === 'ct-mri' ? 'ct' : 'mri'
  return modality === sourceMod ? mapMovingToFixed(point) : point
}

function mapMovingToFixed(point: Vec3): Vec3 {
  if (!fitResult) return point
  return applyForwardDeformation(fitResult.deformation,applyLandmarkWarpForward(fitResult.matrix, fitResult.landmarkWarp, point))
}

function mapFixedToMoving(point: Vec3): Vec3 {
  if (!fitResult) return point
  return applyLandmarkWarpReverse(fitResult.inverse, fitResult.landmarkWarp,applyInverseDeformation(fitResult.deformation,point))
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

let renderAllFrame: number | null = null
function renderAllNow() {
  renderAllFrame = null
  renderMarkers('mri')
  renderMarkers('ct')
  renderReviewMarkers()
  renderLandmarks()
  updateWorkflowState()
}
function renderAll() {
  if (renderAllFrame !== null) return
  renderAllFrame = requestAnimationFrame(renderAllNow)
}

function renderLandmarks() {
  document.querySelector('#pair-count')!.textContent = `${landmarks.length} pair${landmarks.length === 1 ? '' : 's'}`
  const selected = landmarks.find(lm => lm.id === selectedId)
  const selectedSummary = document.querySelector<HTMLDivElement>('#selected-landmark-summary')!
  selectedSummary.textContent = selected
    ? `Landmark ${selected.id} · MRI ${selected.mri ? 'set' : 'not set'} · CT ${selected.ct ? 'set' : 'not set'} · ${selected.enabled ? 'included' : 'excluded'}${selected.residual !== undefined ? ` · residual ${selected.residual.toFixed(2)} mm` : ''}`
    : 'No landmark selected'
  landmarkList.innerHTML = landmarks.length ? '' : '<div class="empty">No landmarks yet.</div>'
  for (const lm of landmarks) {
    const row = document.createElement('div')
    row.className = `landmark-row ${selectedId === lm.id ? 'selected' : ''}`
    const coord = (p: Vec3 | null) => p ? p.map(v=>v.toFixed(1)).join(', ') : 'not set'
    row.innerHTML = `
      <div class="landmark-head"><button class="select">Landmark ${lm.id}</button><label><input class="enabled" type="checkbox" ${lm.enabled?'checked':''}> use</label><button class="delete" title="Delete">×</button></div>
      <div class="point-row"><span>MRI</span><code>${coord(lm.mri)}</code><button class="set-mri">${lm.mri?'Replace':'Set'}</button><button class="go-mri" ${lm.mri?'':'disabled'}>Go</button></div>
      <div class="point-row"><span>CT</span><code>${coord(lm.ct)}</code><button class="set-ct">${lm.ct?'Replace':'Set'}</button><button class="go-ct" ${lm.ct?'':'disabled'}>Go</button></div>
      ${lm.residual !== undefined ? `<div class="residual">Residual: ${lm.residual.toFixed(2)} mm</div>` : ''}
    `
    row.querySelector('.select')!.addEventListener('click',()=>{selectedId=lm.id; if(lm.mri)gotoMm('mri',lm.mri); if(lm.ct)gotoMm('ct',lm.ct); renderAll()})
    row.querySelector('.enabled')!.addEventListener('change',(e)=>{snapshot();lm.enabled=(e.target as HTMLInputElement).checked;markLandmarksChanged();renderAll()})
    row.querySelector('.delete')!.addEventListener('click',()=>{snapshot();landmarks=landmarks.filter(v=>v.id!==lm.id);if(selectedId===lm.id)selectedId=null;markLandmarksChanged();renderAll()})
    for (const modality of ['mri','ct'] as Modality[]) {
      row.querySelector(`.set-${modality}`)!.addEventListener('click',()=>{
        if(!loaded[modality]||!currentMm[modality]) return setStatus(`Load ${modality.toUpperCase()} first.`,true)
        snapshot(); lm[modality]=[...currentMm[modality]!] as Vec3; selectedId=lm.id; markLandmarksChanged(); renderAll()
      })
      row.querySelector(`.go-${modality}`)!.addEventListener('click',()=>{if(lm[modality])gotoMm(modality,lm[modality]!);selectedId=lm.id;renderAll()})
    }
    landmarkList.appendChild(row)
  }
}

function updateWorkflowState() {
  const complete = landmarks.filter(landmark => landmark.enabled && landmark.mri && landmark.ct).length
  document.querySelector('#landmark-stage-status')!.textContent = complete ? `${complete} complete` : 'Create pairs'
  document.querySelector('#linear-stage-status')!.textContent = fitResult ? `${transformModelLabel(fitResult.transformModel)} accepted` : 'Not fitted'
  document.querySelector('#landmark-warp-stage-status')!.textContent = fitResult?.landmarkWarp ? 'TPS accepted' : 'Not applied'
  document.querySelector('#intensity-stage-status')!.textContent = fitResult?.proposal ? 'Candidate ready' : fitResult?.intensityRefined ? 'Applied' : 'Not applied'
  document.querySelector('#deformable-stage-status')!.textContent = fitResult?.deformationProposal ? 'Candidate ready' : fitResult?.deformation ? 'Applied' : 'Not applied'
  document.querySelector('#review-stage-status')!.textContent = fitResult ? 'Ready' : 'Waiting for fit'

  const linear = document.querySelector('#stack-linear')!
  linear.textContent = fitResult ? transformModelLabel(fitResult.transformModel) : 'No linear fit'
  linear.classList.toggle('inactive', !fitResult)
  document.querySelector('#stack-landmark-warp')!.classList.toggle('inactive', !fitResult?.landmarkWarp)
  document.querySelector('#stack-intensity')!.classList.toggle('inactive', !fitResult?.intensityRefined)
  document.querySelector('#stack-deformable')!.classList.toggle('inactive', !fitResult?.deformation)
  const reviewDiagnostics = document.querySelector<HTMLDivElement>('#review-diagnostics')!
  const reviewExport = document.querySelector<HTMLButtonElement>('#review-export')!
  reviewExport.disabled = !fitResult
  reviewDiagnostics.textContent = fitResult
    ? `${transformModelLabel(fitResult.transformModel)} · landmark RMS ${fitResult.rms.toFixed(3)} mm · determinant ${fitResult.linearDeterminant.toFixed(5)} · reflection ${fitResult.linearDiagnostics.reflected ? 'YES' : 'no'}`
    : 'Fit a landmark transform to enable final review and export.'
}

function markLandmarksChanged() {
  for (const lm of landmarks) delete lm.residual
  if (!fitResult) return
  fitResult.landmarksChanged = true
  fitSummary.innerHTML = `<strong>Accepted alignment preserved</strong><br>Current landmarks differ from the snapshot used for this alignment. Export, manual nudges, and accepted refinement remain available until you run a new landmark fit.`
  restoreAlignmentLandmarks.disabled = false
  setStatus('Landmarks changed. The accepted alignment is still active and exportable.')
}

function clearFit() {
  fitResult = null
  for (const lm of landmarks) delete lm.residual
  fitSummary.textContent = 'At least three complete, enabled landmark pairs are required.'
  saveAligned.disabled = true; saveTransform.disabled = true; document.querySelector<HTMLButtonElement>('#export-open')!.disabled = true; 
  resetManual.disabled = true; resetLandmark.disabled = true; refineButton.disabled = true; deformButton.disabled=true;resetDeformButton.disabled=true;restoreAlignmentLandmarks.disabled = true
  resetAlignment.disabled = true
  nonlinearControls.sync()
  refineSummary.textContent = 'Fits locally around the current transform using normalized mutual information and only geometrically overlapping voxels.'
  refineActions.classList.add('hidden')
  deformSummary.textContent = 'No deformable refinement has been applied.'
  deformActions.classList.add('hidden')
  clearReview(); updateNudgeOutputs()
  updateWorkflowState()
}

function clearReview() {
  reviewImages = null
  document.querySelector('#review-label')!.textContent = 'Fit a transform to review alignment'
  document.querySelectorAll('.review-placeholder').forEach(el => el.classList.remove('hidden'))
  for (const view of Object.values(reviewViews)) {
    while (view.nv.volumes.length) view.nv.removeVolumeByIndex(0)
    view.overlay.innerHTML = ''
  }
}

function makeResampledImage(source: Loaded, target: Loaded): NVImage {
  const out = new Float32Array(target.raw.dims[0] * target.raw.dims[1] * target.raw.dims[2])
  const invSourceAffine = invertAffine(source.raw.affine)
  const vals = frame(source.raw, 0)
  let idx = 0
  for (let k=0;k<target.raw.dims[2];k++) for (let j=0;j<target.raw.dims[1];j++) for (let i=0;i<target.raw.dims[0];i++,idx++) {
    const targetWorld = applyAffine(target.raw.affine,[i,j,k])
    const sourceWorld = mapFixedToMoving(targetWorld)
    const sourceVox = applyAffine(invSourceAffine,sourceWorld)
    const interpolation=document.querySelector<HTMLSelectElement>('#output-interpolation')?.value??'linear'
    out[idx] = interpolation==='nearest'?sampleNearest(vals,source.raw.dims,sourceVox):sampleLinear(vals,source.raw.dims,sourceVox)
  }
  const copy = new NVImage(); Object.assign(copy,target.nvImage)
  if (!target.nvImage.hdr) throw new Error('Reference header unavailable')
  const outputHdr = JSON.parse(JSON.stringify(target.nvImage.hdr)) as NonNullable<NVImage['hdr']>
  outputHdr.datatypeCode=16; outputHdr.numBitsPerVoxel=32; outputHdr.scl_slope=1; outputHdr.scl_inter=0; outputHdr.dims[0]=3; outputHdr.dims[4]=1
  copy.hdr = outputHdr
  copy.img = out
  copy.name = `aligned_${source.name}`
  return copy
}

let opacityFrame: number | null = null
function applyReviewOpacityNow() {
  opacityFrame = null
  if (!reviewImages) return
  const opacity = Number(reviewOpacity.value) / 100
  opacityValue.textContent = `${reviewOpacity.value}%`
  for (const view of Object.values(reviewViews)) {
    if (view.nv.volumes.length < 2) continue
    const moving = view.nv.volumes[1]
    moving.opacity = opacity
    view.nv.updateGLVolume()
    view.nv.drawScene()
  }
}

function scheduleReviewOpacity(): void {
  opacityValue.textContent = `${reviewOpacity.value}%`
  if (opacityFrame === null) opacityFrame = requestAnimationFrame(applyReviewOpacityNow)
}

function applyReviewAppearance() {
  if (!reviewImages) return
  for (const view of Object.values(reviewViews)) {
    if (view.nv.volumes.length < 2) continue
    const fixed = view.nv.volumes[0], moving = view.nv.volumes[1]
    view.nv.setColormap(fixed.id, reviewMode.value === 'falsecolor' ? 'green' : 'gray')
    view.nv.setColormap(moving.id, reviewMode.value === 'falsecolor' ? 'brainanaMagenta' : 'gray')
  }
  applyReviewOpacityNow()
}

function buildReview() {
  if (!fitResult) return
  const movingModality: Modality = fitResult.direction === 'ct-mri' ? 'ct' : 'mri'
  const fixedModality: Modality = fitResult.direction === 'ct-mri' ? 'mri' : 'ct'
  const moving = loaded[movingModality]
  const fixed = loaded[fixedModality]
  if (!moving || !fixed) return
  const aligned = makeResampledImage(moving, fixed)
  reviewImages = { fixed: fixed.nvImage, moving: aligned, fixedModality, movingModality }
  for (const view of Object.values(reviewViews)) {
    while (view.nv.volumes.length) view.nv.removeVolumeByIndex(0)
    view.nv.addVolume(fixed.nvImage.clone())
    view.nv.addVolume(aligned.clone())
  }
  document.querySelector('#review-label')!.textContent = `${movingModality.toUpperCase()} aligned to ${fixedModality.toUpperCase()}${fitResult.landmarkWarp ? ' · nonlinear landmark warp' : ''}${fitResult.deformation ? ' · deformable NMI' : ''}${fitResult.landmarksChanged ? ' · using saved landmark snapshot' : ''}`
  document.querySelectorAll('.review-placeholder').forEach(el => el.classList.add('hidden'))
  applyReviewAppearance()
  gotoReviewMm(currentMm[fixedModality] ?? applyAffine(fixed.raw.affine,[(fixed.raw.dims[0]-1)/2,(fixed.raw.dims[1]-1)/2,(fixed.raw.dims[2]-1)/2]))
}

function updateTransformModelControls(): void {
  updateLinearControls(transformModelSelect, customParameters, fitButton)
}

type HandednessAction = 'swap-ct' | 'swap-mri' | 'review' | 'reflection'

function requestHandednessAction(detail: string, first: Landmark, second: Landmark): Promise<HandednessAction> {
  const modal = document.querySelector<HTMLDivElement>('#handedness-modal')!
  const detailEl = document.querySelector<HTMLDivElement>('#handedness-detail')!
  const swapCt = document.querySelector<HTMLButtonElement>('#handedness-swap-ct')!
  const swapMri = document.querySelector<HTMLButtonElement>('#handedness-swap-mri')!
  detailEl.innerHTML = `<strong>Likely crossed correspondence: landmarks ${first.id} and ${second.id}</strong><br>${detail}<br><br>No transform has been applied.`
  swapCt.textContent = `Swap CT points for landmarks ${first.id} and ${second.id}`
  swapMri.textContent = `Swap MRI points for landmarks ${first.id} and ${second.id}`
  modal.classList.remove('hidden')
  swapCt.focus()
  return new Promise(resolve => {
    const finish = (action: HandednessAction) => {
      modal.classList.add('hidden')
      for (const [selector, handler] of handlers) document.querySelector(selector)!.removeEventListener('click', handler)
      window.removeEventListener('keydown', onKey)
      resolve(action)
    }
    const onKey = (event: KeyboardEvent) => { if (event.key === 'Escape') finish('review') }
    const handlers: Array<[string, EventListener]> = [
      ['#handedness-swap-ct', () => finish('swap-ct')],
      ['#handedness-swap-mri', () => finish('swap-mri')],
      ['#handedness-review', () => finish('review')],
      ['#handedness-reflection', () => finish('reflection')],
      ['#handedness-close', () => finish('review')],
    ]
    for (const [selector, handler] of handlers) document.querySelector(selector)!.addEventListener('click', handler)
    window.addEventListener('keydown', onKey)
  })
}

function applyHandednessAction(action: HandednessAction, first: Landmark, second: Landmark): void {
  if (action === 'swap-ct' || action === 'swap-mri') {
    const modality: Modality = action === 'swap-ct' ? 'ct' : 'mri'
    snapshot()
    ;[first[modality], second[modality]] = [second[modality], first[modality]]
    selectedId = first.id
    markLandmarksChanged()
    renderAll()
    setStatus(`${modality.toUpperCase()} points for landmarks ${first.id} and ${second.id} were swapped. Click Fit affine transform to test the corrected non-reflected fit.`)
    return
  }
  if (action === 'reflection') {
    allowReflection.checked = true
    updateTransformModelControls()
    document.querySelector('#reflection-option')?.classList.add('safety-attention')
    document.querySelector('#reflection-option')?.scrollIntoView({block:'center',behavior:'smooth'})
    setStatus('Reflection enabled for the next fit; review the warning and click Fit affine transform again.',true)
    return
  }
  selectedId = first.id
  renderAll()
  document.querySelector('.landmark-list')?.scrollIntoView({block:'center',behavior:'smooth'})
  setStatus(`Review landmarks ${first.id} and ${second.id}. No transform or automatic point swap was applied.`,true)
}

async function runFit() {
  const complete = landmarks.filter(l=>l.enabled&&l.mri&&l.ct)
  const model = transformModelSelect.value as TransformModel
  fitSummary.classList.remove('error-summary')
  const minimum = model === 'affine' || model==='anisotropic' || (model === 'custom' && selectedLinearParameters().some(parameter => parameter.startsWith('s'))) ? 4 : 3
  if (complete.length < minimum) return setStatus(`At least ${minimum} complete, enabled landmark pairs are required for this transform model.`,true)
  const direction = directionSelect.value
  const source = complete.map(l => direction === 'ct-mri' ? l.ct! : l.mri!)
  const target = complete.map(l => direction === 'ct-mri' ? l.mri! : l.ct!)
  beginProcessing(`Fitting ${transformModelLabel(model).toLowerCase()} from ${complete.length} landmark pairs…`)
  await yieldForPaint()
  try {
    const enabledParameters = model === 'custom' ? selectedLinearParameters() : []
    const result = fitLinearModel(source,target,model,enabledParameters,{allowReflection:allowReflection.checked})
    if(result.diagnostics.reflected&&!window.confirm('WARNING: This affine transform reflects the anatomy and may swap left and right. Apply this mirrored transform anyway?'))return setStatus('Reflected affine fit was not applied.',true)
    complete.forEach((l,i)=>l.residual=result.residuals[i])
    fitResult={direction,transformModel:result.model,enabledParameters:result.enabledParameters,linearDiagnostics:result.diagnostics,linearDeterminant:result.determinant,reflectionConfirmed:result.diagnostics.reflected,landmarkWarp:null,deformation:null,landmarkMatrix:result.matrix,baseMatrix:result.matrix,matrix:result.matrix,inverse:result.inverse,rms:result.rms,manual:[0,0,0,0,0,0],proposal:null,deformationProposal:null,landmarkSnapshot:structuredClone(landmarks),landmarksChanged:false,fittedAt:new Date().toISOString(),intensityRefined:false}
    const diagnostics=result.diagnostics,warnings=diagnostics.warnings.map(warning=>`<br><span class="warning">${warning}</span>`).join('')
    fitSummary.innerHTML=`<strong>${transformModelLabel(result.model)} RMS error: ${result.rms.toFixed(3)} mm</strong><br>${complete.length} landmark pairs; ${diagnostics.parameterCount} parameters.<br>Determinant ${result.determinant.toFixed(5)} · scales ${diagnostics.scales.map(value=>value.toFixed(4)).join(', ')} · shear ${diagnostics.shearMagnitude.toFixed(4)} · reflection ${diagnostics.reflected?'YES':'no'}.${warnings}`
    saveAligned.disabled=false;saveTransform.disabled=false;resetManual.disabled=false;resetLandmark.disabled=false;resetAlignment.disabled=false;refineButton.disabled=false;deformButton.disabled=false;restoreAlignmentLandmarks.disabled=false;nonlinearControls.sync();document.querySelector<HTMLButtonElement>('#export-open')!.disabled=false
    updateNudgeOutputs(); buildReview(); renderAll();setStatus(`${transformModelLabel(result.model)} fitted with RMS error ${result.rms.toFixed(3)} mm.`)
  } catch(err) {
    const message = err instanceof Error ? err.message : String(err)
    const reflected = model === 'affine' && /reflection|reflects|mirrors/i.test(message)
    fitSummary.classList.add('error-summary')
    if (reflected) {
      fitSummary.innerHTML = '<strong>Affine fit was not applied</strong><br>The best affine solution mirrors the anatomy and could swap left and right.<br><br>First verify that every MRI landmark corresponds to the same anatomical point in CT. If reflection is genuinely intended, enable <strong>Allow reflected affine fit</strong> and run the fit again.'
      document.querySelector('#reflection-option')?.classList.add('safety-attention')
      setStatus('Affine fit not applied—reflection safeguard. See Landmark alignment.', true)
      window.alert('Affine fit not applied\n\nThe best affine solution contains a reflection, which would mirror the brain and may swap left and right.\n\nCheck the landmark correspondences first. If the reflection is genuinely intended, enable “Allow reflected affine fit” and run the fit again.')
      document.querySelector('#reflection-option')?.scrollIntoView({ block: 'center', behavior: 'smooth' })
    } else {
      const reflectionDiagnostic=model==='affine'&&!allowReflection.checked?diagnoseReflectionAlternative(source,target):null
      if(reflectionDiagnostic){
        const pair=reflectionDiagnostic.suspectedSwappedPair
        const pairMessage=pair?` A single-pair check strongly suggests landmarks ${pair[0]} and ${pair[1]} may be crossed in one image.`:''
        const detail=`A stable proper affine could not be found, while a reflected diagnostic candidate has RMS ${reflectionDiagnostic.reflectedRms.toFixed(3)} mm, scales ${reflectionDiagnostic.scales.map(value=>value.toFixed(3)).join(', ')}, and shear ${reflectionDiagnostic.shearMagnitude.toFixed(3)}. ${(100*reflectionDiagnostic.handednessReversalFraction).toFixed(0)}% of the weighted landmark geometry reverses handedness.${pairMessage}`
        fitSummary.innerHTML=`<strong>Landmark handedness conflict</strong><br>${detail}<br><br>Check corresponding points—especially left/right—before considering reflection.`
        if(pair){
          const first=complete[pair[0]-1],second=complete[pair[1]-1]
          void requestHandednessAction(detail,first,second).then(action=>applyHandednessAction(action,first,second))
        }else{
          setStatus('Affine fit not applied—review the landmark handedness warning before enabling reflection.',true)
          window.alert(`Landmark handedness conflict\n\n${detail}\n\nNo uniquely identifiable crossed pair was found. Review all landmark correspondences before considering reflection.`)
        }
      }else{
        fitSummary.innerHTML = `<strong>${transformModelLabel(model)} was not applied</strong><br>${message}`
        setStatus(`${transformModelLabel(model)} not applied—see Landmark alignment.`, true)
      }
    }
  } finally { endProcessing() }
}


function fixedCenter(): Vec3 {
  if (!fitResult) return [0,0,0]
  const fixedModality: Modality = fitResult.direction === 'ct-mri' ? 'mri' : 'ct'
  const fixed = loaded[fixedModality]!
  return applyAffine(fixed.raw.affine, [(fixed.raw.dims[0]-1)/2,(fixed.raw.dims[1]-1)/2,(fixed.raw.dims[2]-1)/2])
}

function residualRms(matrix: Mat4): number {
  if (!fitResult) return NaN
  const complete = fitResult.landmarkSnapshot.filter(l=>l.enabled&&l.mri&&l.ct)
  if (!complete.length) return NaN
  let sum = 0
  for (const lm of complete) {
    const source = fitResult.direction === 'ct-mri' ? lm.ct! : lm.mri!
    const target = fitResult.direction === 'ct-mri' ? lm.mri! : lm.ct!
    const q = applyForwardDeformation(fitResult.deformation,fitResult.landmarkWarp ? applyLandmarkWarpForward(matrix, fitResult.landmarkWarp, source) : applyMat4(matrix, source))
    const d = Math.hypot(q[0]-target[0],q[1]-target[1],q[2]-target[2])
    lm.residual = d; sum += d*d
  }
  return Math.sqrt(sum/complete.length)
}

function updateNudgeOutputs() {
  const params = fitResult?.manual ?? [0,0,0,0,0,0]
  params.forEach((v,i)=>{ const el=document.querySelector<HTMLOutputElement>(`#param-${i}`); if(el) el.value=`${v.toFixed(i<3?1:2)}${i<3?' mm':'°'}` })
}

function updateCurrentTransform(rebuild = true) {
  if (!fitResult) return
  const delta = rigidDelta(fitResult.manual, fixedCenter())
  fitResult.matrix = multiplyMat4(delta, fitResult.baseMatrix)
  fitResult.inverse = invertAffineMatrix(fitResult.matrix)
  fitResult.rms = residualRms(fitResult.matrix)
  fitResult.proposal = null
  refineActions.classList.add('hidden')
  fitSummary.innerHTML = fitResult.landmarksChanged ? `<strong>Accepted alignment preserved</strong><br>Manual correction is relative to the accepted base transform. Current landmarks differ from the alignment snapshot.` : `<strong>Alignment-snapshot RMS: ${fitResult.rms.toFixed(3)} mm</strong><br>Manual correction is relative to the accepted base transform.`
  updateNudgeOutputs(); renderAll()
  if (rebuild) scheduleReviewBuild()
}

function scheduleReviewBuild() {
  if (reviewBuildTimer !== null) window.clearTimeout(reviewBuildTimer)
  reviewBuildTimer = window.setTimeout(()=>{ reviewBuildTimer=null; buildReview() }, 120)
}


function updateWindowControls() {
  const mriCount = countOptimizationWindows(optimizationWindows, 'mri'), ctCount = countOptimizationWindows(optimizationWindows, 'ct')
  windowSummary.textContent = mriCount || ctCount
    ? `Active constraints: MRI ${mriCount} plane${mriCount === 1 ? '' : 's'}, CT ${ctCount} plane${ctCount === 1 ? '' : 's'}. Each defined plane restricts only its two visible axes; any plane without a window is unrestricted.`
    : 'No optimization windows defined. The full geometric overlap will be used.'
  defineWindows.textContent = definingWindows ? 'Finish defining windows' : 'Define windows'
  for (const modality of ['mri','ct'] as Modality[]) for (const view of Object.values(views[modality])) {
    const active = definingWindows && (windowTarget.value === 'both' || windowTarget.value === modality)
    view.windowLayer.classList.toggle('active', active)
    renderOptimizationWindow(view)
  }
}

function canvasToCss(view: View, pos: number[]): [number, number] {
  const rect = view.canvas.getBoundingClientRect()
  return [pos[0] * rect.width / Math.max(1, view.canvas.width), pos[1] * rect.height / Math.max(1, view.canvas.height)]
}

function renderOptimizationWindow(view: View) {
  view.windowLayer.replaceChildren()
  const bounds = optimizationWindows[view.modality][view.plane]
  if (!bounds || !loaded[view.modality]) return
  const [a, b, depth] = planeAxes[view.plane]
  const center = currentMm[view.modality] ?? [0,0,0]
  const first = [...center] as Vec3, second = [...center] as Vec3
  first[a] = bounds.min[a]; first[b] = bounds.min[b]; first[depth] = center[depth]
  second[a] = bounds.max[a]; second[b] = bounds.max[b]; second[depth] = center[depth]
  const raw0 = view.nv.frac2canvasPos(view.nv.mm2frac(first)), raw1 = view.nv.frac2canvasPos(view.nv.mm2frac(second))
  if (!raw0 || !raw1) return
  const p0 = canvasToCss(view, raw0), p1 = canvasToCss(view, raw1)
  const rect = document.createElement('div')
  rect.className = 'optimization-window-box'
  rect.style.left = `${Math.min(p0[0],p1[0])}px`
  rect.style.top = `${Math.min(p0[1],p1[1])}px`
  rect.style.width = `${Math.abs(p1[0]-p0[0])}px`
  rect.style.height = `${Math.abs(p1[1]-p0[1])}px`
  view.windowLayer.appendChild(rect)
}

function eventMm(view: View, event: PointerEvent): Vec3 | null {
  return clientPointToImageOnCurrentSlice(view, event.clientX, event.clientY)
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
      setStatus(`${view.modality.toUpperCase()} ${view.plane} optimization window set.`)
    },
    onReject: () => {
      // A missed coordinate or tiny replacement drag must never erase an existing
      // valid constraint. Clear windows is the only destructive UI action.
      const hasExisting = Boolean(optimizationWindows[view.modality][view.plane])
      setStatus(hasExisting
        ? `${view.modality.toUpperCase()} ${view.plane} window was not changed; the existing window remains active.`
        : `${view.modality.toUpperCase()} ${view.plane} window was not set. This plane remains fully unrestricted.`)
      updateWindowControls()
    },
  })
}

function optimizationConstraint(modality: Modality): WindowConstraint | null {
  return getOptimizationConstraint(optimizationWindows, modality)
}

function robustRange(values: Float32Array): [number, number] {
  const stride = Math.max(1, Math.floor(values.length/50000))
  const sample: number[]=[]
  for(let i=0;i<values.length;i+=stride){ const v=values[i]; if(Number.isFinite(v)) sample.push(v) }
  sample.sort((a,b)=>a-b)
  if(!sample.length) return [0,1]
  const at=(p:number)=>sample[Math.min(sample.length-1,Math.max(0,Math.floor(p*(sample.length-1))))]
  let lo=at(.01), hi=at(.99); if(!(hi>lo)){lo=sample[0];hi=sample[sample.length-1]||lo+1} if(!(hi>lo))hi=lo+1
  return [lo,hi]
}

type MetricVolume = { raw: RawNifti }
type MetricContext = { fixed: MetricVolume; moving: MetricVolume; fixedModality: Modality; movingModality: Modality; movingWindow: WindowConstraint | null; points: Array<[Vec3,number]>; movingInv: number[][]; fixedRange:[number,number]; movingRange:[number,number]; overlapBox:{min:Vec3;max:Vec3}; candidateCount:number; overlapFractionFixed:number; resolutionMm:number }
function imageCornersWorld(image: MetricVolume): Vec3[] {
  const [nx,ny,nz]=image.raw.dims
  const corners:Vec3[]=[]
  for(const i of [0,nx-1])for(const j of [0,ny-1])for(const k of [0,nz-1]) corners.push(applyAffine(image.raw.affine,[i,j,k]))
  return corners
}

const pyramidCache = new WeakMap<RawNifti, Map<number, RawNifti>>()
function registrationRaw(raw: RawNifti, spacingMm: number): RawNifti {
  if (spacingMm <= 0) return raw
  let cache = pyramidCache.get(raw)
  if (!cache) { cache = new Map(); pyramidCache.set(raw, cache) }
  let result = cache.get(spacingMm)
  if (!result) { result = downsampleForRegistration(raw, spacingMm); cache.set(spacingMm, result) }
  return result
}
const factorPyramidCache = new WeakMap<RawNifti, Map<string, RawNifti>>()
function registrationRawByFactor(raw:RawNifti,factor:number,sigma:number):RawNifti{
  let cache=factorPyramidCache.get(raw);if(!cache){cache=new Map();factorPyramidCache.set(raw,cache)}
  const key=`${factor}:${sigma}`,found=cache.get(key);if(found)return found
  const result=downsampleByFactor(raw,factor,sigma);cache.set(key,result);return result
}
function selectedResolutionLevels(stage: 'intensity' | 'deformable' = 'intensity'): number[] {
  const preset = stage === 'deformable' ? deformRegistrationResolution.value : localRegistrationResolution.value
  return resolutionLevels(preset as RegistrationResolutionPreset)
}

function commaNumbers(value:string,label:string):number[]{const result=value.split(',').map(part=>Number(part.trim())).filter(Number.isFinite);if(!result.length)throw new Error(`${label} must be a comma-separated list.`);return result}
function automaticSchedule(){
  const factors=shrinkFactorLevels(registrationResolution.value as ShrinkFactorPreset,registrationResolutionCustom.value)
  const smoothing=automaticSmoothingMode.value==='auto'?factors.map(factor=>factor===1?0:factor*.5):commaNumbers(automaticSmoothingCustom.value,'Custom smoothing')
  const sweeps=automaticSweepsMode.value==='auto'?factors.map((_,index)=>index===0?2:index===factors.length-1?2:4):commaNumbers(automaticSweepsCustom.value,'Custom sweeps')
  if(smoothing.length!==factors.length)throw new Error('Custom smoothing must contain one value per shrink factor.')
  if(smoothing.some(value=>value<0||value>16))throw new Error('Custom smoothing values must be from 0 through 16 native voxels.')
  if(sweeps.length!==factors.length||sweeps.some(value=>!Number.isInteger(value)||value<1||value>12))throw new Error('Custom sweeps must contain one integer from 1 through 12 per shrink factor.')
  return {factors,smoothing,sweeps}
}

function makeMetricContext(maxSamples=25000, resolutionMm=0, overlapMatrix?:Mat4, factorLevel?:{factor:number;sigma:number}): MetricContext {
  const direction=fitResult?.direction??directionSelect.value
  const movingModality: Modality = direction === 'ct-mri' ? 'ct' : 'mri'
  const fixedModality: Modality = direction === 'ct-mri' ? 'mri' : 'ct'
  const moving={raw:factorLevel?registrationRawByFactor(loaded[movingModality]!.raw,factorLevel.factor,factorLevel.sigma):registrationRaw(loaded[movingModality]!.raw,resolutionMm)},fixed={raw:factorLevel?registrationRawByFactor(loaded[fixedModality]!.raw,factorLevel.factor,factorLevel.sigma):registrationRaw(loaded[fixedModality]!.raw,resolutionMm)}
  const fixedWindow=optimizationConstraint(fixedModality), movingWindow=optimizationConstraint(movingModality)
  const fv=frame(fixed.raw,0), fixedInv=invertAffine(fixed.raw.affine)
  const matrix=overlapMatrix??fitResult?.matrix
  if(!matrix)throw new Error('Automatic alignment needs an initial center-alignment matrix.')
  const transformedCorners=imageCornersWorld(moving).map(p=>applyMat4(matrix,p)).map(p=>applyAffine(fixedInv,p))
  const mins:[number,number,number]=[0,1,2].map(a=>Math.max(0,Math.floor(Math.min(...transformedCorners.map(p=>p[a]))))) as Vec3
  const maxs:[number,number,number]=[
    Math.min(fixed.raw.dims[0]-1,Math.ceil(Math.max(...transformedCorners.map(p=>p[0])))),
    Math.min(fixed.raw.dims[1]-1,Math.ceil(Math.max(...transformedCorners.map(p=>p[1])))),
    Math.min(fixed.raw.dims[2]-1,Math.ceil(Math.max(...transformedCorners.map(p=>p[2])))),
  ]
  if(maxs.some((v,a)=>v<mins[a])) throw new Error('The images have no geometric overlap at the current transform.')
  const candidateCount=(maxs[0]-mins[0]+1)*(maxs[1]-mins[1]+1)*(maxs[2]-mins[2]+1)
  let stride=Math.max(1,Math.ceil(Math.cbrt(candidateCount/maxSamples)))
  let points:Array<[Vec3,number]>=[]
  const collect = (step:number) => {
    const result:Array<[Vec3,number]>=[]
    for(let k=mins[2];k<=maxs[2];k+=step)for(let j=mins[1];j<=maxs[1];j+=step)for(let i=mins[0];i<=maxs[0];i+=step){
      const idx=i+fixed.raw.dims[0]*(j+fixed.raw.dims[1]*k); const v=fv[idx]
      const world=applyAffine(fixed.raw.affine,[i,j,k])
      if(Number.isFinite(v)&&withinOptimizationWindows(world,fixedWindow)) result.push([world,v])
    }
    return result
  }
  points=collect(stride)
  // Constraints can occupy a small part of a large overlap. Densify sampling
  // before deciding that the region is unusable. Missing planes remain fully unrestricted.
  while(points.length<1000 && stride>1){ stride=Math.max(1,Math.floor(stride/2)); points=collect(stride) }
  const fixedTotal=fixed.raw.dims[0]*fixed.raw.dims[1]*fixed.raw.dims[2]
  if (points.length < 64) {
    if (fixedWindow) {
      const planes = Object.keys(fixedWindow).join(', ')
      throw new Error(`The defined ${fixedModality.toUpperCase()} optimization window${Object.keys(fixedWindow).length === 1 ? '' : 's'} (${planes}) contain too few valid fixed-image voxels. Clear or enlarge a defined window. Any view without a window remains unrestricted.`)
    }
    throw new Error('The current transform leaves too few valid overlapping fixed-image voxels for automatic refinement. Adjust the alignment or verify the image geometry.')
  }
  return {fixed,moving,fixedModality,movingModality,movingWindow,points,movingInv:invertAffine(moving.raw.affine),fixedRange:robustRange(fv),movingRange:robustRange(frame(moving.raw,0)),overlapBox:{min:mins,max:maxs},candidateCount,overlapFractionFixed:candidateCount/fixedTotal,resolutionMm}
}

function metricScore(matrix: Mat4, ctx: MetricContext, metric:AutomaticMetric='nmi',requestedBins=40): number {
  const bins=requestedBins, joint=new Uint32Array(bins*bins), fh=new Uint32Array(bins), mh=new Uint32Array(bins)
  const mv=frame(ctx.moving.raw,0), invT=invertAffineMatrix(matrix)
  const [flo,fhi]=ctx.fixedRange,[mlo,mhi]=ctx.movingRange
  let n=0,sumF=0,sumM=0,sumFF=0,sumMM=0,sumFM=0,sumSquared=0
  for(const [world,fv] of ctx.points){
    const sw=applyMat4(invT,world); if(!withinOptimizationWindows(sw,ctx.movingWindow))continue; const vox=applyAffine(ctx.movingInv,sw)
    if(vox[0]<0||vox[1]<0||vox[2]<0||vox[0]>=ctx.moving.raw.dims[0]-1||vox[1]>=ctx.moving.raw.dims[1]-1||vox[2]>=ctx.moving.raw.dims[2]-1)continue
    const mvv=sampleLinear(mv,ctx.moving.raw.dims,vox); if(!Number.isFinite(mvv))continue
    if(fv<=flo && mvv<=mlo) continue
    const fb=Math.max(0,Math.min(bins-1,Math.floor((fv-flo)/(fhi-flo)*bins)))
    const mb=Math.max(0,Math.min(bins-1,Math.floor((mvv-mlo)/(mhi-mlo)*bins)))
    fh[fb]++;mh[mb]++;joint[fb*bins+mb]++;n++;sumF+=fv;sumM+=mvv;sumFF+=fv*fv;sumMM+=mvv*mvv;sumFM+=fv*mvv
    const fn=(fv-flo)/(fhi-flo),mn=(mvv-mlo)/(mhi-mlo);sumSquared+=(fn-mn)**2
  }
  if(n<Math.min(500, Math.max(64, Math.floor(ctx.points.length*0.25))))return -Infinity
  if(metric==='least-squares')return -sumSquared/n
  if(metric==='normalized-correlation'){
    const numerator=n*sumFM-sumF*sumM,denominator=Math.sqrt(Math.max(0,(n*sumFF-sumF*sumF)*(n*sumMM-sumM*sumM)))
    return denominator>0?numerator/denominator:-Infinity
  }
  const entropy=(hist:Uint32Array)=>{let h=0;for(const c of hist)if(c){const p=c/n;h-=p*Math.log(p)}return h}
  const hf=entropy(fh),hm=entropy(mh),hj=entropy(joint)
  if(metric==='mi')return hf+hm-hj
  if(metric==='correlation-ratio'){
    const means=new Float64Array(bins),counts=new Uint32Array(bins)
    for(let fb=0;fb<bins;fb++)for(let mb=0;mb<bins;mb++){const count=joint[fb*bins+mb];if(count){means[fb]+=count*(mb+.5)/bins;counts[fb]+=count}}
    let overall=0;for(let mb=0;mb<bins;mb++)overall+=mh[mb]*(mb+.5)/bins;overall/=n
    let between=0,total=0;for(let fb=0;fb<bins;fb++)if(counts[fb])between+=counts[fb]*(means[fb]/counts[fb]-overall)**2
    for(let mb=0;mb<bins;mb++)total+=mh[mb]*((mb+.5)/bins-overall)**2
    return total>0?between/total:-Infinity
  }
  return hj>0?(hf+hm)/hj:-Infinity
}

const nmiScore=(matrix:Mat4,ctx:MetricContext)=>metricScore(matrix,ctx,'nmi',40)

function nmiDeformationScore(grid:DeformationGrid|null,ctx:MetricContext):number{
  if(!fitResult)return -Infinity
  const bins=40,joint=new Uint32Array(bins*bins),fh=new Uint32Array(bins),mh=new Uint32Array(bins)
  const mv=frame(ctx.moving.raw,0),[flo,fhi]=ctx.fixedRange,[mlo,mhi]=ctx.movingRange
  let n=0
  for(const [world,fv] of ctx.points){
    const predeformed=applyInverseDeformation(grid,world)
    const sw=applyLandmarkWarpReverse(fitResult.inverse,fitResult.landmarkWarp,predeformed)
    if(!withinOptimizationWindows(sw,ctx.movingWindow))continue
    const vox=applyAffine(ctx.movingInv,sw)
    if(vox[0]<0||vox[1]<0||vox[2]<0||vox[0]>=ctx.moving.raw.dims[0]-1||vox[1]>=ctx.moving.raw.dims[1]-1||vox[2]>=ctx.moving.raw.dims[2]-1)continue
    const mvv=sampleLinear(mv,ctx.moving.raw.dims,vox);if(!Number.isFinite(mvv)||(fv<=flo&&mvv<=mlo))continue
    const fb=Math.max(0,Math.min(bins-1,Math.floor((fv-flo)/(fhi-flo)*bins))),mb=Math.max(0,Math.min(bins-1,Math.floor((mvv-mlo)/(mhi-mlo)*bins)))
    fh[fb]++;mh[mb]++;joint[fb*bins+mb]++;n++
  }
  if(n<Math.min(500,Math.max(64,Math.floor(ctx.points.length*.25))))return -Infinity
  const entropy=(hist:Uint32Array)=>{let h=0;for(const count of hist)if(count){const p=count/n;h-=p*Math.log(p)}return h}
  const jointEntropy=entropy(joint);return jointEntropy>0?(entropy(fh)+entropy(mh))/jointEntropy:-Infinity
}

function deformableSettings():DeformableSettings{
  const direction=fitResult?.direction??directionSelect.value,fixedModality:Modality=direction==='ct-mri'?'mri':'ct',raw=loaded[fixedModality]!.raw
  const extent=Math.cbrt(raw.dims.reduce((product,size,axis)=>product*Math.max(raw.pixDims[axis],(size-1)*raw.pixDims[axis]),1))
  const presets:Record<string,DeformableSettings>={
    conservative:{levelsMm:[extent/3.5,extent/6],regularization:.12,maximumDisplacement:extent/50,iterationsPerLevel:1,jacobianMinimum:.65,jacobianMaximum:1.6},
    balanced:{levelsMm:[extent/4,extent/8],regularization:.05,maximumDisplacement:extent/32,iterationsPerLevel:1,jacobianMinimum:.5,jacobianMaximum:2},
    flexible:{levelsMm:[extent/5,extent/10],regularization:.02,maximumDisplacement:extent/20,iterationsPerLevel:2,jacobianMinimum:.35,jacobianMaximum:2.5},
  }
  if(deformPreset.value!=='custom')return structuredClone(presets[deformPreset.value])
  const levels=(document.querySelector<HTMLInputElement>('#deform-levels')!.value.split(',').map(Number).filter(Number.isFinite))
  const settings:DeformableSettings={levelsMm:levels,regularization:Number(document.querySelector<HTMLInputElement>('#deform-regularization')!.value),maximumDisplacement:Number(document.querySelector<HTMLInputElement>('#deform-max')!.value),iterationsPerLevel:Number(document.querySelector<HTMLInputElement>('#deform-iterations')!.value),jacobianMinimum:Number(document.querySelector<HTMLInputElement>('#deform-jac-min')!.value),jacobianMaximum:Number(document.querySelector<HTMLInputElement>('#deform-jac-max')!.value)}
  const problems:string[]=[]
  if(!levels.length)problems.push('Grid levels must contain at least one comma-separated number.')
  else if(levels.some(value=>value<8||value>100))problems.push('Every grid level must be between 8 and 100 mm.')
  if(!Number.isFinite(settings.regularization)||settings.regularization<0)problems.push('Regularization must be zero or greater.')
  if(!Number.isFinite(settings.maximumDisplacement)||settings.maximumDisplacement<=0)problems.push('Maximum displacement must be greater than zero.')
  if(!Number.isInteger(settings.iterationsPerLevel)||settings.iterationsPerLevel<1||settings.iterationsPerLevel>4)problems.push('Iterations per level must be an integer from 1 through 4.')
  if(!Number.isFinite(settings.jacobianMinimum)||settings.jacobianMinimum<=0)problems.push('Minimum Jacobian must be greater than zero.')
  if(!Number.isFinite(settings.jacobianMaximum)||settings.jacobianMaximum<=settings.jacobianMinimum)problems.push('Maximum Jacobian must be greater than the minimum Jacobian.')
  if(problems.length)throw new Error(`Custom deformable settings need attention:\n• ${problems.join('\n• ')}`)
  return settings
}

let deformCancelled=false
async function runDeformableRefinement(){
  if(!fitResult||!loaded.mri||!loaded.ct)return
  if(fitResult.deformationProposal)return setStatus('A deformable proposal is already awaiting review. Accept or reject it before running another deformable refinement.',true)
  deformCancelled=false;deformButton.disabled=true;cancelDeformButton.classList.remove('hidden');beginProcessing('Preparing deformable NMI refinement…')
  await new Promise(resolve=>setTimeout(resolve,20))
  try{
    const resolutions=selectedResolutionLevels('deformable'),contexts=resolutions.map(level=>makeMetricContext(10000,level)),ctx=contexts[contexts.length-1],settings=deformableSettings(),previous=fitResult.deformation
    fitResult.optimizationResolutionLevels=[...resolutions]
    const points=ctx.points.map(entry=>entry[0]),bounds={min:[0,1,2].map(axis=>Math.min(...points.map(point=>point[axis]))) as Vec3,max:[0,1,2].map(axis=>Math.max(...points.map(point=>point[axis]))) as Vec3}
    const before=nmiDeformationScore(previous,ctx)
    const contextForGrid=(grid:DeformationGrid)=>{
      const gridSpacing=Math.max(...grid.spacing),levelIndex=settings.levelsMm.reduce((best,value,index)=>Math.abs(value-gridSpacing)<Math.abs(settings.levelsMm[best]-gridSpacing)?index:best,0)
      const contextIndex=settings.levelsMm.length===1?contexts.length-1:Math.round(levelIndex*(contexts.length-1)/(settings.levelsMm.length-1))
      return contexts[Math.max(0,Math.min(contexts.length-1,contextIndex))]
    }
    const proposed=await optimizeDeformationGrid({bounds,settings,initialGrid:previous,score:grid=>nmiDeformationScore(grid,contextForGrid(grid)),cancelled:()=>deformCancelled,progress:message=>setStatus(message)})
    const after=nmiDeformationScore(proposed,ctx);proposed.scoreBefore=before;proposed.scoreAfter=after
    if(!Number.isFinite(after)||after<before-1e-5)throw new Error(`The deformable proposal worsened NMI (${before.toFixed(5)} → ${after.toFixed(5)}) and was rejected.`)
    fitResult.deformationProposal={previous:previous?structuredClone(previous):null,proposed,scoreBefore:before,scoreAfter:after};fitResult.deformation=proposed;fitResult.rms=residualRms(fitResult.matrix)
    buildReview();renderAll();resetDeformButton.disabled=false
    deformSummary.innerHTML=`<strong>Deformable NMI ${before.toFixed(5)} → ${after.toFixed(5)}</strong><br>Image resolution ${resolutions.map(value=>value?`${value} mm`:'native').join(' → ')}; grid ${proposed.dims.join('×')}; spacing ${proposed.spacing.map(value=>value.toFixed(1)).join(', ')} mm; displacement ≤ ${proposed.jacobian.maximumDisplacement.toFixed(2)} mm.<br>Sampled Jacobian ${proposed.jacobian.minimum.toFixed(3)}–${proposed.jacobian.maximum.toFixed(3)}.`
    deformActions.classList.remove('hidden');updateWorkflowState();setStatus('Deformable refinement proposal ready. Review it, then accept or reject.')
  }catch(error){setStatus(error instanceof Error?error.message:String(error),true)}finally{deformButton.disabled=Boolean(fitResult?.deformationProposal);cancelDeformButton.classList.add('hidden');endProcessing()}
}

async function runRefinement() {
  if(!fitResult||!loaded.mri||!loaded.ct)return
  if(fitResult.deformation)return setStatus('Reset the accepted deformable refinement before running rigid intensity refinement.',true)
  refineButton.disabled=true; beginProcessing('Preparing constrained mutual-information refinement…')
  await new Promise(r=>setTimeout(r,20))
  try{
    const resolutions=selectedResolutionLevels(),contexts=resolutions.map(level=>makeMetricContext(30000,level)),start=fitResult.matrix.map(r=>[...r])
    fitResult.optimizationResolutionLevels=[...resolutions]
    const tLimit=Number(refineTranslationLimit.value), rLimit=Number(refineRotationLimit.value)
    const tStep=Number(refineTranslationStartStep.value), rStep=Number(refineRotationStartStep.value)
    if(![tLimit,rLimit,tStep,rStep].every(v=>Number.isFinite(v)&&v>0)) throw new Error('Refinement limits and steps must be positive numbers.')
    const limits:Params6=[tLimit,tLimit,tLimit,rLimit,rLimit,rLimit]
    let params:Params6=[0,0,0,0,0,0]
    let steps:Params6=[Math.min(tStep,tLimit),Math.min(tStep,tLimit),Math.min(tStep,tLimit),Math.min(rStep,rLimit),Math.min(rStep,rLimit),Math.min(rStep,rLimit)]
    const center=fixedCenter(), matrixFor=(p:Params6)=>multiplyMat4(rigidDelta(p,center),start)
    const levelsPerResolution=contexts.length===1?4:2
    let best=-Infinity
    for(let resolutionIndex=0;resolutionIndex<contexts.length;resolutionIndex++){
      const ctx=contexts[resolutionIndex]
      best=nmiScore(matrixFor(params),ctx)
      setStatus(`Refining at ${ctx.resolutionMm>0?`${ctx.resolutionMm} mm`:'native resolution'}…`)
      for(let level=0;level<levelsPerResolution;level++){
      let improved=true,passes=0
      while(improved&&passes<2){ improved=false;passes++
        for(let axis=0;axis<6;axis++){
          let bestAxis=best,bestValue=params[axis]
          for(const sign of [-1,1]){
            const candidate=[...params] as Params6; candidate[axis]=Math.max(-limits[axis],Math.min(limits[axis],candidate[axis]+sign*steps[axis]))
            if(candidate[axis]===params[axis])continue
            const score=nmiScore(matrixFor(candidate),ctx)
            if(score>bestAxis+1e-6){bestAxis=score;bestValue=candidate[axis]}
          }
          if(bestValue!==params[axis]){params[axis]=bestValue;best=bestAxis;improved=true}
        }
        await new Promise(r=>setTimeout(r,0))
      }
      steps=steps.map(v=>v/2) as Params6
      }
    }
    const ctx=contexts[contexts.length-1],before=nmiScore(start,ctx)
    const proposal=matrixFor(params)
    fitResult.proposal={matrix:proposal,scoreBefore:before,scoreAfter:best,params}
    fitResult.matrix=proposal;fitResult.inverse=invertAffineMatrix(proposal);fitResult.rms=residualRms(proposal)
    buildReview();renderAll()
    const gain=best-before
    const hit=params.some((v,i)=>Math.abs(v)>=limits[i]-1e-6)
    refineSummary.innerHTML=`<strong>NMI ${before.toFixed(5)} → ${best.toFixed(5)}</strong><br>Resolution ${resolutions.map(value=>value?`${value} mm`:'native').join(' → ')}; gain ${gain.toFixed(5)}; proposed Δ ${params.slice(0,3).map(v=>v.toFixed(2)).join(', ')} mm and ${params.slice(3).map(v=>v.toFixed(2)).join(', ')}°<br>Overlap box ${ctx.candidateCount.toLocaleString()} fixed-grid voxels (${(100*ctx.overlapFractionFixed).toFixed(1)}% of fixed image); ${ctx.points.length.toLocaleString()} samples evaluated.${hit?'<br><span class="warning">Warning: a search bound was reached.</span>':''}`
    refineActions.classList.remove('hidden');setStatus('Automatic refinement proposal ready. Review it, then accept or reject.')
  }catch(err){setStatus(err instanceof Error?err.message:String(err),true)}finally{refineButton.disabled=false;endProcessing()}
}

function volumeCenter(raw:RawNifti):Vec3{return applyAffine(raw.affine,[(raw.dims[0]-1)/2,(raw.dims[1]-1)/2,(raw.dims[2]-1)/2])}
function automaticEnabledParameters(model:TransformModel):LinearParameter[]{
  const pose:LinearParameter[]=['tx','ty','tz','rx','ry','rz']
  if(model==='rigid')return pose
  if(model==='similarity')return [...pose,'sx']
  if(model==='anisotropic')return [...pose,'sx','sy','sz']
  if(model==='affine')return [...pose,'sx','sy','sz','shxy','shxz','shyz']
  const result=[...pose]
  if(document.querySelector<HTMLInputElement>('#auto-scale-x')!.checked)result.push('sx')
  if(document.querySelector<HTMLInputElement>('#auto-scale-y')!.checked)result.push('sy')
  if(document.querySelector<HTMLInputElement>('#auto-scale-z')!.checked)result.push('sz')
  if(document.querySelector<HTMLInputElement>('#auto-shear')!.checked)result.push('shxy','shxz','shyz')
  return result
}

async function runAutomaticLinear(){
  if(!loaded.mri||!loaded.ct)return
  if(fitResult?.deformationProposal||fitResult?.proposal)return setStatus('Accept or reject the current registration proposal before starting another alignment.',true)
  automaticLinearButton.disabled=true;beginProcessing('Preparing global automatic linear alignment…');await yieldForPaint()
  try{
    const direction=directionSelect.value,movingModality:Modality=direction==='ct-mri'?'ct':'mri',fixedModality:Modality=direction==='ct-mri'?'mri':'ct'
    const movingCenter=volumeCenter(loaded[movingModality]!.raw),fixedCenter=volumeCenter(loaded[fixedModality]!.raw)
    const centerMatrix:Mat4=[[1,0,0,fixedCenter[0]-movingCenter[0]],[0,1,0,fixedCenter[1]-movingCenter[1]],[0,0,1,fixedCenter[2]-movingCenter[2]],[0,0,0,1]]
    const schedule=automaticSchedule(),contexts=schedule.factors.map((factor,index)=>makeMetricContext(18000,0,centerMatrix,{factor,sigma:schedule.smoothing[index]}))
    const metric=automaticMetric.value as AutomaticMetric
    const bins=automaticBins.value==='auto'?Math.max(32,Math.min(128,2**Math.round(Math.log2(Math.sqrt(contexts[0].points.length)/2)))):Number(automaticBins.value)
    const model=automaticModel.value as TransformModel,enabled=automaticEnabledParameters(model)
    const result=await optimizeAutomaticLinear({
      settings:{model,enabledParameters:enabled,rotationRanges:['x','y','z'].map(axis=>Number(document.querySelector<HTMLSelectElement>(`#auto-range-${axis}`)!.value) as RotationRange) as [RotationRange,RotationRange,RotationRange],translationLimitMm:Number(document.querySelector<HTMLInputElement>('#auto-translation-limit')!.value),scaleLimit:Number(document.querySelector<HTMLInputElement>('#auto-scale-limit')!.value),shearLimit:Number(document.querySelector<HTMLInputElement>('#auto-shear-limit')!.value),candidatesToRefine:4,shrinkFactors:schedule.factors,sweepsPerLevel:schedule.sweeps},
      movingCenter,fixedCenter,score:(matrix,level)=>metricScore(matrix,contexts[level],metric,bins),progress:setStatus,
    })
    const diagnostics=linearDiagnostics(result.matrix,enabled.length)
    fitResult={direction,transformModel:model,enabledParameters:enabled,linearDiagnostics:diagnostics,linearDeterminant:result.determinant,reflectionConfirmed:false,landmarkWarp:null,deformation:null,landmarkMatrix:centerMatrix,baseMatrix:centerMatrix,matrix:result.matrix,inverse:invertAffineMatrix(result.matrix),rms:NaN,manual:[0,0,0,0,0,0],proposal:{matrix:result.matrix,scoreBefore:result.scoreBefore,scoreAfter:result.scoreAfter,params:result.parameters,initial:true},deformationProposal:null,landmarkSnapshot:structuredClone(landmarks),landmarksChanged:false,fittedAt:new Date().toISOString(),intensityRefined:false,optimizationResolutionLevels:[...schedule.factors]}
    buildReview();renderAll();refineActions.classList.remove('hidden');acceptRefine.textContent='Accept automatic alignment';rejectRefine.textContent='Reject'
    refineSummary.innerHTML=`<strong>${metric.toUpperCase()} ${result.scoreBefore.toFixed(5)} → ${result.scoreAfter.toFixed(5)}</strong><br>${model}; ${enabled.length} parameters; ${result.candidatesEvaluated.toLocaleString()} metric evaluations.<br>Input-grid shrink factors ${schedule.factors.map(value=>`${value}×`).join(' → ')}; smoothing σ ${schedule.smoothing.join(' → ')} native voxels; sweeps ${schedule.sweeps.join(' → ')}; histogram bins ${bins}.${result.warnings.length?`<br><span class="warning">${result.warnings.join('<br>')}</span>`:''}`
    updateWorkflowState();setStatus('Global automatic linear proposal ready. Review the overlay, then accept or reject it.')
  }catch(error){setStatus(error instanceof Error?error.message:String(error),true)}finally{automaticLinearButton.disabled=!(loaded.mri&&loaded.ct);endProcessing()}
}

async function acceptRefinement(){
  if(!fitResult?.deformationProposal&&!fitResult?.proposal)return
  beginProcessing(fitResult.deformationProposal?'Applying deformable refinement…':'Applying automatic refinement…')
  await yieldForPaint()
  try{
    if(fitResult.deformationProposal){fitResult.deformation=fitResult.deformationProposal.proposed;fitResult.deformationProposal=null;deformButton.disabled=false;deformActions.classList.add('hidden');buildReview();renderAll();setStatus('Deformable refinement accepted. You may run another pass initialized from this accepted deformation.');return}
    if(!fitResult.proposal)return
    fitResult.baseMatrix=fitResult.proposal.matrix;fitResult.matrix=fitResult.proposal.matrix;fitResult.inverse=invertAffineMatrix(fitResult.matrix);fitResult.manual=[0,0,0,0,0,0];fitResult.proposal=null;fitResult.intensityRefined=true
    fitResult.rms=residualRms(fitResult.matrix);refineActions.classList.add('hidden');acceptRefine.textContent='Accept refinement';rejectRefine.textContent='Reject';saveAligned.disabled=false;saveTransform.disabled=false;resetAlignment.disabled=false;resetManual.disabled=false;refineButton.disabled=false;deformButton.disabled=false;document.querySelector<HTMLButtonElement>('#export-open')!.disabled=false;updateNudgeOutputs();buildReview();renderAll();setStatus('Automatic linear alignment accepted.')
  }finally{endProcessing()}
}
async function rejectRefinement(){
  if(!fitResult?.deformationProposal&&!fitResult?.proposal)return
  beginProcessing(fitResult.deformationProposal?'Restoring the previous deformation…':'Restoring the previous alignment…')
  await yieldForPaint()
  try{
    if(fitResult.deformationProposal){fitResult.deformation=fitResult.deformationProposal.previous;fitResult.deformationProposal=null;deformButton.disabled=false;deformActions.classList.add('hidden');buildReview();renderAll();deformSummary.textContent='Deformable refinement rejected. The previous accepted deformation was restored.';setStatus('Deformable refinement rejected. You may run another pass from the restored deformation.');return}
    if(!fitResult.proposal)return
    if(fitResult.proposal.initial){clearFit();refineActions.classList.add('hidden');acceptRefine.textContent='Accept refinement';rejectRefine.textContent='Reject';refineSummary.textContent='Automatic alignment rejected. No transform was applied.';setStatus('Automatic alignment rejected.');return}
    fitResult.proposal=null;updateCurrentTransform(true);refineSummary.textContent='Refinement rejected. The previous accepted transform has been restored.';setStatus('Automatic refinement rejected.')
  }finally{endProcessing()}
}

function currentSession() {
  return createSessionPayload({
    appVersion: APP_VERSION,
    loaded,
    landmarks,
    selectedId,
    nextId,
    direction: directionSelect.value,
    fit: fitResult,
    optimizationWindows,
  })
}

async function saveSessionFile() {
  beginProcessing('Preparing session file…')
  await yieldForPaint()
  try {
    await saveArtifact(new Blob([JSON.stringify(currentSession(), null, 2)], { type: 'application/json' }), 'brainana-align_session.json')
    setStatus('Session saved.')
  } finally { endProcessing() }
}

async function loadSessionFile(file: File) {
  const payload = parseSessionPayload(JSON.parse(await file.text()))
  const mismatch = sessionGeometryMismatches(payload.images, loaded)
  let restoreFit = true
  if (mismatch.length) {
    restoreFit = false
    const ok = window.confirm(`Image geometry does not match the saved session:\n\n${mismatch.join('\n')}\n\nLoad landmarks only?`)
    if (!ok) return
  }
  if (restoreFit && payload.fit?.linearDiagnostics.reflected) {
    const ok = window.confirm('WARNING: This session contains a reflected landmark transform that mirrors anatomy. Restore and apply the reflected transform?')
    if (!ok) restoreFit = false
    else payload.fit.reflectionConfirmed = true
  }
  beginProcessing('Restoring session…')
  await yieldForPaint()
  try {
  snapshot()
  landmarks = structuredClone(payload.landmarks)
  selectedId = payload.selectedId
  nextId = Math.max(payload.nextId, ...landmarks.map(landmark => landmark.id + 1), 1)
  directionSelect.value = payload.direction
  optimizationWindows = sanitizeOptimizationWindows(structuredClone(payload.optimizationWindows))
  updateWindowControls()
  if (restoreFit && payload.fit && loaded.mri && loaded.ct) {
    fitResult = structuredClone(payload.fit)
    transformModelSelect.value = fitResult.transformModel
    updateTransformModelControls()
    if (!fitResult.landmarkSnapshot) fitResult.landmarkSnapshot = structuredClone(payload.landmarks)
    if (fitResult.landmarksChanged === undefined) fitResult.landmarksChanged = false
    if (!fitResult.fittedAt) fitResult.fittedAt = payload.savedAt
    saveAligned.disabled = false
    saveTransform.disabled = false
    resetManual.disabled = false
    resetLandmark.disabled = false
    resetAlignment.disabled = false
    refineButton.disabled = false
    deformButton.disabled = false
    resetDeformButton.disabled = !fitResult.deformation
    nonlinearControls.sync()
    restoreAlignmentLandmarks.disabled = false
    document.querySelector<HTMLButtonElement>('#export-open')!.disabled = false
    updateNudgeOutputs()
    buildReview()
    fitSummary.innerHTML = fitResult.landmarksChanged
      ? '<strong>Accepted alignment restored</strong><br>Current landmarks differ from the saved alignment snapshot.'
      : '<strong>Accepted alignment restored</strong><br>Landmarks match the saved alignment snapshot.'
  } else {
    clearFit()
  }
  renderAll()
  setStatus(restoreFit && payload.fit ? 'Full session restored.' : 'Landmarks restored; fit was not restored.')
  } finally { endProcessing() }
}

async function saveTransformFiles() {
  if (!fitResult) return
  if(fitResult.linearDiagnostics.reflected&&!window.confirm('WARNING: The accepted transform reflects anatomy. Export this mirrored transform and its metadata?'))return setStatus('Reflected transform export cancelled.',true)
  beginProcessing('Saving transform files…')
  try {
    await saveArtifacts(createRegistrationArtifacts({ appVersion: APP_VERSION, fit: fitResult, landmarks }))
    setStatus('Transform files saved.')
  } catch (error) {
    setStatus(error instanceof Error ? error.message : String(error), true)
    throw error
  } finally { endProcessing() }
}

async function saveAlignedImage() {
  if (!fitResult) return
  if(fitResult.linearDiagnostics.reflected&&!window.confirm('WARNING: The accepted transform reflects anatomy. Export the mirrored aligned image?'))return setStatus('Reflected aligned-image export cancelled.',true)
  const sourceMod: Modality = fitResult.direction === 'ct-mri' ? 'ct' : 'mri'
  const targetMod: Modality = fitResult.direction === 'ct-mri' ? 'mri' : 'ct'
  const source = loaded[sourceMod]
  const target = loaded[targetMod]
  if (!source || !target) return setStatus('Both images must be loaded.', true)
  beginProcessing('Resampling aligned image…')
  try {
    await new Promise(resolve => setTimeout(resolve, 20))
    const copy = makeResampledImage(source, target)
    setStatus('Compressing and saving aligned image…')
    await saveArtifact(
      new Blob([gzipSync(copy.toUint8Array())], { type: 'application/gzip' }),
      `${sourceMod}_space-${targetMod.toUpperCase()}_${fitResult.transformModel}${fitResult.landmarkWarp ? '-tps' : ''}${fitResult.deformation ? '-deformable' : ''}.nii.gz`,
    )
    setStatus('Aligned image saved.')
  } catch (error) {
    setStatus(error instanceof Error ? error.message : String(error), true)
    throw error
  } finally { endProcessing() }
}

const nonlinearControls=installNonlinearLandmarkControls({
  getFit:()=>fitResult,
  residualRms:()=>fitResult?residualRms(fitResult.matrix):NaN,
  rebuild:buildReview,
  render:renderAll,
  setStatus,
  setRigidRefinementEnabled:enabled=>{refineButton.disabled=!enabled},
  beginProcessing,
  endProcessing,
  yieldForPaint,
})

document.querySelector<HTMLInputElement>('#mri-file')!.addEventListener('change',e=>{const files=Array.from((e.target as HTMLInputElement).files ?? []);if(files.length)loadFiles('mri',files).catch(err=>setStatus(err.message,true))})
document.querySelector<HTMLInputElement>('#ct-file')!.addEventListener('change',e=>{const files=Array.from((e.target as HTMLInputElement).files ?? []);if(files.length)loadFiles('ct',files).catch(err=>setStatus(err.message,true))})
document.querySelector('#new-pair')!.addEventListener('click',()=>{workflowNavigation.select('landmarks');snapshot();const lm={id:nextId++,mri:null,ct:null,enabled:true};landmarks.push(lm);selectedId=lm.id;markLandmarksChanged();renderAll()})
document.querySelector('#undo')!.addEventListener('click',()=>{const prev=history.pop();if(prev){landmarks=prev;markLandmarksChanged();renderAll()}})
directionSelect.addEventListener('change',clearFit)
transformModelSelect.addEventListener('change',()=>{if(transformModelSelect.value!=='affine')allowReflection.checked=false;updateTransformModelControls()})
allowReflection.addEventListener('change',()=>document.querySelector('#reflection-option')?.classList.remove('safety-attention'))
resetAlignment.addEventListener('click',()=>{
  if (!fitResult) return
  clearFit()
  renderAll()
  setStatus('Alignment reset. Landmarks and optimization windows were retained.')
})
saveTransform.addEventListener('click',()=>saveTransformFiles().catch(err=>setStatus(err instanceof Error?err.message:String(err),true)))
saveAligned.addEventListener('click',()=>saveAlignedImage().catch(err=>setStatus(err.message,true)))
reviewMode.addEventListener('change', applyReviewAppearance)
reviewOpacity.addEventListener('input', scheduleReviewOpacity)
document.querySelectorAll<HTMLButtonElement>('#nudge-controls button').forEach(button=>button.addEventListener('click',()=>{
  if(!fitResult)return
  const index=Number(button.dataset.param),sign=Number(button.dataset.sign)
  const step=index<3?Number(translationStep.value):Number(rotationStep.value)
  fitResult.manual[index]+=sign*step;updateCurrentTransform(true)
}))
resetManual.addEventListener('click',()=>{if(!fitResult)return;fitResult.manual=[0,0,0,0,0,0];updateCurrentTransform(true);setStatus('Manual nudges reset.')})
resetLandmark.addEventListener('click',()=>{if(!fitResult)return;fitResult.baseMatrix=fitResult.landmarkMatrix;fitResult.landmarkWarp=null;fitResult.deformation=null;fitResult.deformationProposal=null;fitResult.intensityRefined=false;resetDeformButton.disabled=true;fitResult.manual=[0,0,0,0,0,0];updateCurrentTransform(true);nonlinearControls.sync();setStatus('Restored the original linear landmark fit. Nonlinear stages were cleared.')})
refineButton.addEventListener('click',()=>runRefinement())
automaticLinearButton.addEventListener('click',()=>runAutomaticLinear())
intensityOperation.addEventListener('change',()=>{
  const global=intensityOperation.value==='global'
  globalLinearSettings.classList.toggle('hidden',!global);localLinearSettings.classList.toggle('hidden',global)
  document.querySelectorAll('.global-intensity-setting').forEach(element=>element.classList.toggle('hidden',!global))
  document.querySelectorAll('.local-intensity-setting').forEach(element=>element.classList.toggle('hidden',global))
  automaticLinearButton.classList.toggle('hidden',!global);refineButton.classList.toggle('hidden',global)
  refineSummary.textContent=global?'Searches globally from image centers using multistart orientation search, bracketed line optimization, and an input-relative coarse-to-fine pyramid.':'Fits locally around the accepted transform using a constrained six-parameter NMI search.'
})
automaticModel.addEventListener('change',()=>document.querySelector('#auto-custom')!.classList.toggle('hidden',automaticModel.value!=='custom'))
automaticMetric.addEventListener('change',()=>document.querySelector('#auto-bins-row')!.classList.toggle('hidden',!['nmi','mi','correlation-ratio'].includes(automaticMetric.value)))
deformButton.addEventListener('click',()=>runDeformableRefinement())
cancelDeformButton.addEventListener('click',()=>{deformCancelled=true;setStatus('Cancelling deformable refinement…')})
resetDeformButton.addEventListener('click',()=>void (async()=>{if(!fitResult)return;beginProcessing('Restoring alignment without deformable refinement…');await yieldForPaint();try{fitResult.deformation=null;fitResult.deformationProposal=null;resetDeformButton.disabled=true;fitResult.rms=residualRms(fitResult.matrix);buildReview();renderAll();setStatus('Deformable refinement reset. Linear and landmark alignment were retained.')}finally{endProcessing()}})())
deformPreset.addEventListener('change',()=>deformCustom.classList.toggle('hidden',deformPreset.value!=='custom'))
registrationResolution.addEventListener('change',()=>{const custom=registrationResolution.value==='custom';registrationResolutionCustomRow.classList.toggle('hidden',!custom);if(custom){registrationResolutionCustom.focus();registrationResolutionCustom.select()}})
automaticSmoothingMode.addEventListener('change',()=>automaticSmoothingCustomRow.classList.toggle('hidden',automaticSmoothingMode.value!=='custom'))
automaticSweepsMode.addEventListener('change',()=>automaticSweepsCustomRow.classList.toggle('hidden',automaticSweepsMode.value!=='custom'))
defineWindows.addEventListener('click',()=>{definingWindows=!definingWindows;updateWindowControls();renderMarkers('mri');renderMarkers('ct');setStatus(definingWindows?'Drag a rectangle in any selected MRI or CT panel. Every plane you leave without a window remains fully unrestricted.':'Optimization-window definition finished. Planes without windows will use all voxels.')})
clearWindows.addEventListener('click',()=>{optimizationWindows={mri:{},ct:{}};updateWindowControls();renderMarkers('mri');renderMarkers('ct');setStatus('Optimization windows cleared.')})
windowTarget.addEventListener('change',updateWindowControls)
updateWindowControls()
acceptRefine.addEventListener('click',()=>void acceptRefinement())
rejectRefine.addEventListener('click',()=>void rejectRefinement())
acceptDeform.addEventListener('click',()=>void acceptRefinement())
rejectDeform.addEventListener('click',()=>void rejectRefinement())
document.querySelector('#save-session')!.addEventListener('click',()=>saveSessionFile().catch(err=>setStatus(err instanceof Error?err.message:String(err),true)))
document.querySelector<HTMLInputElement>('#load-session')!.addEventListener('change',e=>{const file=(e.target as HTMLInputElement).files?.[0];if(file)loadSessionFile(file).catch(err=>setStatus(err instanceof Error?err.message:String(err),true));(e.target as HTMLInputElement).value=''})
refineBounds.addEventListener('change',()=>{
  if(refineBounds.value==='tight'){refineTranslationLimit.value='2';refineRotationLimit.value='1';refineTranslationStartStep.value='1';refineRotationStartStep.value='0.5'}
  else if(refineBounds.value==='standard'){refineTranslationLimit.value='5';refineRotationLimit.value='10';refineTranslationStartStep.value='2';refineRotationStartStep.value='1'}
})
for(const input of [refineTranslationLimit,refineRotationLimit,refineTranslationStartStep,refineRotationStartStep])input.addEventListener('input',()=>{refineBounds.value='custom'})

fitButton.addEventListener('click',()=>{void runFit()})
const exportModal=document.querySelector<HTMLDivElement>('#export-modal')!
const openExport=()=>exportModal.classList.remove('hidden')
const closeExport=()=>exportModal.classList.add('hidden')
document.querySelector('#export-open')!.addEventListener('click',openExport)
document.querySelector('#review-export')!.addEventListener('click',openExport)
document.querySelector('#export-close')!.addEventListener('click',closeExport)
exportModal.addEventListener('click',e=>{if(e.target===exportModal)closeExport()})
document.addEventListener('keydown',e=>{if(e.key==='Escape')closeExport()})
document.querySelector('#save-session-export')!.addEventListener('click',()=>saveSessionFile().catch(err=>setStatus(err instanceof Error?err.message:String(err),true)))

restoreAlignmentLandmarks.addEventListener('click',()=>{
  if(!fitResult)return
  snapshot()
  landmarks=structuredClone(fitResult.landmarkSnapshot)
  selectedId=landmarks.find(l=>l.id===selectedId)?.id ?? landmarks[0]?.id ?? null
  fitResult.landmarksChanged=false
  residualRms(fitResult.matrix)
  fitSummary.innerHTML=`<strong>Alignment landmarks restored</strong><br>Current landmarks again match the snapshot used by the accepted alignment.`
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

const browserCapabilities = detectBrowserCapabilities()
const browserReady = installBrowserCompatibilityBanner(browserCapabilities)
if (browserReady) {
  setupViews()
  setupReviewViews()
} else {
  setStatus('This browser cannot initialize the required WebGL2 viewer.', true)
}
renderLandmarks()
updateTransformModelControls()
installRuntimeIntegration(loadFiles, setStatus)
