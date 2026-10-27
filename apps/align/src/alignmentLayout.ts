import {welcomeMarkup} from './welcomeScreen'
import { automaticControls,refinementControls } from './automaticRegistrationUI'
import { LANDMARK_MODELS } from './landmarkFit'
import type { Modality, Plane } from './appTypes'

/** Registration-specific shell; keeps control IDs stable for actions and guided setup. */
export function alignmentLayout(version: string): string {
  return `
<header class="topbar">
  <div class="brand">Brainana Align <span>v${version}</span></div>
  <div class="toolbar-cluster"><span class="cluster-label">Volumes</span><div class="workflow-group image-loads">
    <label class="load compact-load" title="Load an MRI volume"><input id="mri-file" type="file" multiple accept=".nii,.nii.gz,.hdr,.img,.img.gz,.head,.brik,.brik.gz,.mgh,.mgz,.nrrd,.nhdr,.mif,.mha,.mhd,.raw,.v,.v16,.vmr,.npy,.npz,.fib,.src,.gz,application/gzip,application/x-gzip,application/octet-stream"><strong id="mri-name">Select MRI</strong></label>
    <label class="load compact-load" title="Load a CT volume"><input id="ct-file" type="file" multiple accept=".nii,.nii.gz,.hdr,.img,.img.gz,.head,.brik,.brik.gz,.mgh,.mgz,.nrrd,.nhdr,.mif,.mha,.mhd,.raw,.v,.v16,.vmr,.npy,.npz,.fib,.src,.gz,application/gzip,application/x-gzip,application/octet-stream"><strong id="ct-name">Select CT</strong></label>
    <button id="working-resolution" disabled>Resolution</button>
  </div>
  </div><div class="toolbar-divider" aria-hidden="true"></div>
  <div class="toolbar-cluster"><span class="cluster-label">Session</span><div class="workflow-group session-actions">
    <button id="save-session" title="Save landmarks and registration state">Save session</button>
    <label class="session-load" title="Restore a saved Brainana Align session"><input id="load-session" type="file" accept=".json,application/json"><span>Load session</span></label>
  </div></div><div class="toolbar-divider" aria-hidden="true"></div>
  <div class="toolbar-cluster"><span class="cluster-label">Landmarks</span><div class="workflow-group landmark-actions">
    <button id="new-pair" class="primary-action">New landmark</button>
  </div>
  </div><div class="toolbar-divider" aria-hidden="true"></div>
  <div class="toolbar-cluster"><span class="cluster-label">Workspace</span><div class="workflow-group workspace-actions">
    <button id="guided-setup" title="Step-by-step help for loading, alignment, and saving">Guided setup</button>
    <button id="settings-open" title="Appearance, display, interaction and saved defaults">Settings</button>
    <button id="reset-workspace" title="Unload images and clear all registration work, guide progress and recovery snapshots">Reset workspace</button>
  </div></div><div class="toolbar-divider" aria-hidden="true"></div>
  <div class="toolbar-cluster"><span class="cluster-label">Review & export</span><div class="workflow-group registration-actions">
    <button id="reset-view" title="Reset zoom and field of view">Reset view</button>
    <button id="export-open" disabled>Export</button>
  </div>
  </div><button id="undo" class="icon-action" title="Undo last landmark edit">Undo</button>
</header>
<section id="alignment-progress" class="alignment-progress hidden" role="status" aria-live="polite"><span class="busy-spinner" aria-hidden="true"></span><strong id="alignment-progress-label"></strong><progress max="1" aria-label="Alignment progress"></progress><button id="alignment-cancel" class="hidden">Cancel registration</button></section>
${welcomeMarkup()}
<main>
  <section class="workspace" aria-label="MRI, CT and aligned overlay in three anatomical planes">
    <div class="column-head"></div><div class="column-head">Sagittal</div><div class="column-head">Coronal</div><div class="column-head">Axial</div>
    ${(['mri','ct'] as Modality[]).map(modality => `
      <div class="row-head"><strong data-image-label="${modality}">${modality.toUpperCase()}</strong><small id="${modality}-coords">x — y — z —</small></div>
      ${(['sagittal','coronal','axial'] as Plane[]).map(plane => `<div class="view-card" data-modality="${modality}" data-plane="${plane}"><canvas id="${modality}-${plane}"></canvas><canvas id="${modality}-${plane}-mask-preview" class="threshold-mask-preview" aria-hidden="true" hidden></canvas><svg class="marker-overlay" id="${modality}-${plane}-overlay"></svg><div class="optimization-window-layer" id="${modality}-${plane}-window-layer"></div><div class="image-placeholder" id="${modality}-${plane}-placeholder">No ${modality.toUpperCase()} selected</div></div>`).join('')}
    `).join('')}
    <div class="row-head review-head"><strong>OVERLAY</strong><small id="review-label">Fit a transform to review alignment</small><button id="refresh-processed-overlay" class="hidden" title="Redraw using the existing transform; does not fit a registration">Refresh overlay</button></div>
    ${(['sagittal','coronal','axial'] as Plane[]).map(plane => `<div class="view-card review-card" data-plane="${plane}"><canvas id="review-${plane}"></canvas><svg class="marker-overlay" id="review-${plane}-overlay"></svg><div class="review-placeholder">Fit a transform to display the aligned overlay</div></div>`).join('')}
  </section>
  <aside class="sidebar" aria-label="Registration controls">
    <div class="rail-title">Registration workflow</div><div class="workflow-tabs" aria-label="Focus registration tools"><button data-workflow="landmarks" aria-pressed="false">Landmarks</button><button data-workflow="followup" aria-pressed="false">Landmark refinement</button><button data-workflow="manual" aria-pressed="false">Manual</button><button data-workflow="automatic" aria-pressed="false">Image-based</button></div>
    <label class="shared-direction">Transform direction<select id="direction"><option value="ct-mri">CT → MRI</option><option value="mri-ct">MRI → CT</option></select></label>
<details class="panel" id="initial-processing-panel"><summary><span>Initial processing</span></summary><div class="panel-body"><p class="hint">Optional, reversible changes to working images. Original files are never overwritten.</p><div id="processing-display"><h3>Image display</h3><p class="hint">Display only: contrast changes the intensity window width. Minimum and maximum set its endpoints. These controls do not change voxels, registration or exports.</p>${(['mri','ct'] as const).map(m=>`<fieldset class="intensity-controls"><legend><span data-image-label="${m}">${m.toUpperCase()}</span></legend><label>Contrast <output id="${m}-contrast-value">1×</output><input id="${m}-contrast" type="range" min="0.25" max="4" step="0.01" value="1"></label>${(['min','max'] as const).map(k=>`<label>${k==='min'?'Minimum':'Maximum'} intensity<input id="${m}-display-${k}" type="range" step="any"><input id="${m}-display-${k}-number" type="number" step="any" aria-label="${m.toUpperCase()} ${k} intensity"></label>`).join('')}<button id="${m}-display-reset">Reset display range</button></fieldset>`).join('')}</div><div id="processing-masking"><h3>Binary mask</h3><p class="hint">Independent thresholds select voxels from underlying working data: 1 selected, 0 excluded. Green previews the selection while dragging. Create a binary mask, then optionally apply it to the working image. Display contrast/windowing never changes mask values.</p>${(['mri','ct'] as const).map(m=>`<fieldset class="threshold-controls" data-mask-modality="${m}"><legend><span data-image-label="${m}">${m.toUpperCase()}</span> binary mask</legend><label class="check"><input id="${m}-mask-preview" type="checkbox">Preview binary mask</label>${(['min','max'] as const).map(k=>`<label>${k==='min'?'Minimum':'Maximum'} threshold<input id="${m}-mask-${k}" type="range" step="any"><input id="${m}-mask-${k}-number" type="number" step="any" aria-label="${m.toUpperCase()} mask ${k}"></label>`).join('')}<label class="check"><input id="${m}-mask-invert" type="checkbox">Invert mask</label><label>Fill enclosed holes<select id="${m}-mask-holes"><option value="">Off</option><option value="3d">3D enclosed cavities</option><option value="slices">Within each voxel-Z slice</option></select></label><p class="hint">Fills enclosed background without dilation. Slice filling can include interiors open in 3D; slices follow the working voxel-Z axis, which may be oblique. This can include real anatomical cavities. Create/update to preview the filled result before applying.</p><button id="${m}-mask-create">Create / update binary mask</button><label>Outside fill value<input id="${m}-mask-fill" type="number" step="any" value="0"></label><button id="${m}-mask-apply">Apply generated mask to working image</button><button id="${m}-mask-clear">Clear mask / undo mask application</button><p id="${m}-mask-status" class="hint" role="status">Prepare images first.</p></fieldset>`).join('')}</div><hr><div id="processing-cropping"><label>Image to process<select id="processing-target"><option value="mri">MRI</option><option value="ct">CT</option></select></label><h3>Crop image</h3><button id="processing-crop-draw">Draw crop box</button><p class="hint">Drag in two orthogonal views to bound all three RAS axes, or edit the bounds below. Review before applying. Oblique crops retain the image grid orientation; outside-box corners are zero-padded and excluded from registration.</p><div class="processing-bounds"><label>X min<input id="crop-min-0" type="number" step="any"></label><label>X max<input id="crop-max-0" type="number" step="any"></label><label>Y min<input id="crop-min-1" type="number" step="any"></label><label>Y max<input id="crop-max-1" type="number" step="any"></label><label>Z min<input id="crop-min-2" type="number" step="any"></label><label>Z max<input id="crop-max-2" type="number" step="any"></label></div><button id="processing-crop-apply">Apply crop</button><button id="processing-crop-reset">Reset crop selection</button></div><div id="processing-editing"><h3>Voxel editing</h3><label>Edit target<select id="processing-edit-target"><option value="mri">MRI image</option><option value="ct">CT image</option><option value="mri-mask">MRI binary mask</option><option value="ct-mask">CT binary mask</option></select></label><p id="processing-edit-target-status" class="hint" role="status"></p><label id="processing-mask-value-label" class="hidden">Mask value<select id="processing-mask-value"><option value="0">Erase · 0</option><option value="1">Paint · 1</option></select></label><label id="processing-image-action-label">Action<select id="processing-action"><option value="replace">Paint specified intensity</option><option value="erase">Erase · 0</option><option value="exclude">Exclude from registration</option><option value="include">Include in registration</option></select></label><label id="processing-image-value-label">Paint intensity<input id="processing-value" type="number" step="any" value="0"></label><label>3D brush radius (working voxels)<input id="processing-brush-radius" type="number" min="0" max="16" step="1" value="0"></label><button id="processing-voxel">Edit voxels</button><button id="processing-edit-apply" disabled>Apply edits</button><button id="processing-edit-discard" disabled>Discard unapplied edits</button><p id="processing-edit-status" class="hint" role="status"></p><button id="processing-stop">Finish processing tool</button><p class="hint">Enter radius 0 for a single voxel, or an integer up to 16 for a larger 3D brush. Click or drag to paint in the selected image’s views. Orange previews unapplied edits; Apply edits or Finish applies the batch once. Brush radius uses a 3D sphere in working-grid voxels, so its physical shape depends on voxel spacing. 0 replaces intensity; exclusion preserves intensity and skips registration sample centers. Up to 20,000 edited voxels per image; use masking for larger regions.</p></div><div class="processing-history"><button id="processing-undo">Undo processing</button><button id="processing-redo">Redo processing</button></div><button id="processing-restore">Restore original working image</button><button id="processing-export" class="hidden">Export processed image + mask</button><p id="processing-summary" class="hint" role="status"></p><p class="hint">Undo history is kept during this launch; sessions save the active crop/edits/mask. Restore original before changing working resolution. Exported processed images use working resolution; native aligned-image export uses originals.</p></div></details>
    <details class="panel" id="landmarks-panel">
      <summary><span>Landmarks</span><span id="pair-count">0 pairs</span></summary>
      <div class="panel-body">
        <p class="hint">Create a pair, navigate each image, then set its MRI and CT positions. Drag a visible marker to adjust it within the current slice.</p>
        <div id="selected-landmark-summary" class="selected-summary">No landmark selected</div>
        <p class="hint">Rigid starting set: five well-distributed pairs are often sufficient for a useful initial alignment—anterior midline, posterior midline, superior midline, left lateral mid A–P, and right lateral mid A–P. Choose anatomy identifiable in both images, add more pairs when useful, and verify the overlay. Three pairs is only the minimum.</p>
      <div id="landmark-list" class="landmark-list"></div>
      </div>
    </details>
    <details class="panel" id="landmark-alignment-panel">
      <summary><span>Landmark alignment</span></summary>
      <div class="panel-body">
        <label>Landmark transform<select id="landmark-transform">${Object.entries(LANDMARK_MODELS).map(([value, info]) => `<option value="${value}">${info.label}</option>`).join('')}</select></label>

        <button id="fit-side" class="primary">Run landmark alignment</button>
        <button id="restore-alignment-landmarks" disabled>Restore alignment landmarks</button>
        <div id="fit-summary" class="summary">At least three complete, enabled landmark pairs are required.</div>
        <label class="check"><input id="link-after" type="checkbox" checked> Link navigation after fitting</label>
      <details id="spline-alignment" class="nested"><summary>Nonlinear landmark spline (advanced)</summary>
        <div id="spline-settings" ><p class="warning">Advanced: at least 12 distributed pairs; 20+ recommended. Incorrect or sparse landmarks can distort anatomy. The safety limit may leave residual errors rather than force a fold. Review and accept every proposal.</p><label>Spline smoothing<input id="spline-smoothing" type="number" min="0" max="10" step="0.01" value="0.01"></label><p class="hint">0 requests interpolation; positive values smooth landmark errors. Nonlinear fitting and resampling are cancellable. Rigid remains the default.</p></div>
      <p id="spline-availability" class="hint" role="status">Create and accept a landmark alignment first; spline fitting also needs at least 12 complete pairs.</p><button id="run-landmark-spline" disabled aria-describedby="spline-availability">Fit landmark spline</button><p id="spline-result-summary" class="summary"></p><div id="spline-actions" class="refine-actions hidden"><button id="accept-spline" class="primary">Accept spline</button><button id="reject-spline">Reject</button></div></details>
      </div>
    </details>
    <details class="panel" id="followup-panel">
      <summary><span>Landmark refinement</span></summary><div class="panel-body">
      <p class="hint">Start with an accepted landmark registration. Improve the landmark-derived alignment with constrained image-based refinement.</p>
      <h3>Image-based refinement</h3>
<h3>Optimization region</h3>        <div class="optimization-window-controls">
          <label>Define optimization region<select id="window-target-followup"><option value="both">MRI and CT</option><option value="mri">MRI only</option><option value="ct">CT only</option></select></label>
          <button id="define-windows-followup">Define windows</button>
          <button id="clear-windows-followup">Clear windows</button>
          <p id="window-summary-followup" class="hint">No optimization windows defined. The full geometric overlap will be used.</p>
        </div>

      <div id="followup-registration-host">${refinementControls()}</div>

</div>
    </details>
    <details class="panel" id="manual-panel">
      <summary><span>Manual adjustment</span></summary>
      <div class="panel-body">
        <div id="manual-start-section"><button id="manual-start-geometry">Begin manual alignment</button><p class="hint">Create a starting overlay so you can move, rotate, scale, or shear MRI or CT manually. The images may initially overlap poorly.</p></div>
        <div class="manual-targets" role="group" aria-label="Image to adjust"><button data-manual-target="mri" aria-pressed="false">Adjust MRI</button><button data-manual-target="ct" aria-pressed="true">Adjust CT</button></div>
        <label class="check"><input type="checkbox" id="manual-source-preview">Preview adjustment in selected source row</label><p class="hint">The preview row uses the other image’s grid and is labelled as transformed. Original voxels stay unchanged. Native views return when editing landmarks, processing images or defining regions.</p><p id="manual-status" class="hint" role="status"></p><div id="manual-proposal-actions" class="hidden"><p class="hint">Resolve the preview here to unlock manual controls. Accept keeps it; Reject restores the previous alignment, or clears the preview if there was none.</p><button id="manual-accept-proposal">Accept preview and enable manual adjustment</button><button id="manual-reject-proposal">Reject preview and enable manual adjustment</button></div>
        <div class="step-grid"><label>Translation step<select id="translation-step"><option value="0.1">0.1 mm</option><option value="0.5" selected>0.5 mm</option><option value="1">1 mm</option><option value="2">2 mm</option><option value="5">5 mm</option></select></label><label>Rotation step<select id="rotation-step"><option value="0.1">0.1°</option><option value="0.25">0.25°</option><option value="0.5" selected>0.5°</option><option value="1">1°</option><option value="2">2°</option></select></label><label>Scale step<select id="scale-step"><option value="0.1">0.1%</option><option value="0.5">0.5%</option><option value="1" selected>1%</option><option value="5">5%</option></select></label><label>Shear step<select id="shear-step"><option value="0.001">0.001</option><option value="0.01" selected>0.01</option><option value="0.05">0.05</option></select></label></div>
        <div id="nudge-controls" class="nudge-controls">
          ${['Translation X · L/R','Translation Y · P/A','Translation Z · I/S','Rotation X','Rotation Y','Rotation Z','Scale X (%)','Scale Y (%)','Scale Z (%)','Shear XY','Shear XZ','Shear YZ'].map((label,i)=>`<div class="nudge-row"><label for="param-${i}">${label}</label><button data-param="${i}" data-sign="-1" aria-label="Decrease ${label}">−</button><input id="param-${i}" type="number" step="any" value="${i>=6&&i<9?100:0}" aria-label="${label}"><button data-param="${i}" data-sign="1" aria-label="Increase ${label}">+</button></div>`).join('')}
        </div>
        <button id="reset-manual" disabled>Reset all manual adjustments</button><button id="reset-landmark" disabled>Reset to landmark fit</button>
      </div>
    </details>
    <details class="panel" id="refine-panel">
      <summary><span>Image-based registration</span></summary><div class="panel-body">
      <p class="hint">Align images from their intensities without requiring landmarks. Establish a global linear alignment or use a nonlinear method to adjust local shape. Every result is a proposal to inspect and accept or reject.</p>
<h3>Optimization region</h3>        <div class="optimization-window-controls">
          <label>Define optimization region<select id="window-target"><option value="both">MRI and CT</option><option value="mri">MRI only</option><option value="ct">CT only</option></select></label>
          <button id="define-windows">Define windows</button>
          <button id="clear-windows">Clear windows</button>
          <p id="window-summary" class="hint">No optimization windows defined. The full geometric overlap will be used.</p>
        </div>

      <div id="image-registration-host"><div id="registration-controls">
        ${automaticControls()}
        <button id="refine" class="hidden" disabled>Run legacy rigid refinement</button>
        <div id="refine-summary" class="summary">Review every proposal before accepting.</div>
        <div id="refine-actions" class="refine-actions hidden"><button id="accept-refine" class="primary">Accept registration</button><button id="reject-refine">Reject</button></div>
      </div></div>
      <button id="use-image-geometry" class="hidden">Begin manual alignment</button>
      </div>
    </details>
    <details class="panel" id="alignment-display-panel">
      <summary><span>Alignment display</span></summary>
      <div class="panel-body">
        <label>Display mode<select id="review-mode"><option value="falsecolor">Green–magenta false color</option><option value="opacity">Grayscale opacity</option></select></label>
        <label>Image comparison<select id="review-comparison"><option value="smooth">Smooth opacity</option><option value="flip">Flip between images</option></select></label><button id="review-flip" class="hidden">Flip image</button><p id="review-flip-label" class="hint hidden"></p>
        <button id="opacity-cycle" aria-pressed="false">Animate opacity · 25–75%</button><label>Moving image opacity <span id="opacity-value">55%</span><input id="review-opacity" type="range" min="0" max="100" value="55"></label>
        <p class="hint">The fixed image defines the review grid. Paired landmark positions are shown as circles and diamonds after fitting.</p>
        <p class="hint interaction-hint"><strong>Zoom:</strong> mouse wheel or two-finger trackpad scroll. <strong>Change slices:</strong> Shift + wheel or Shift + two-finger scroll. <strong>Pan the field of view:</strong> hover a panel and use the arrow keys, or drag empty image space. Dragging a landmark still moves only that landmark.</p>
      </div>
    </details>
  </aside>
</main>
<footer class="statusbar"><span class="status-label">Status</span><span id="status" role="status" aria-live="polite">Load MRI and CT to begin.</span><span class="status-units" title="Landmark positions and errors use physical coordinates">Physical space · mm</span></footer>
<div id="export-modal" class="modal hidden" role="dialog" aria-modal="true" aria-labelledby="export-title">
  <div class="modal-card">
    <div class="modal-head"><h2 id="export-title">Export images and registration</h2><button id="export-close" aria-label="Close">×</button></div>
    
    <p id="export-notice" class="transform-warning hidden" role="status"></p><button id="export-review-proposal" class="hidden">Review pending alignment</button>
    <p id="export-feedback" role="status" aria-live="polite"></p>
    <h3>Transformed moving image</h3><p class="hint">Save aligned image applies the accepted transform in the selected CT → MRI or MRI → CT direction. These options control that image; transforms and sessions retain their full registration state.</p>
    <div class="aligned-export-grid"><label>Image data<select id="aligned-image-source"><option value="native">Native intensities · no preprocessing</option><option value="native-processed">Native intensities + applied mask, crop and voxel edits</option><option value="processed">Current working image · includes preprocessing</option></select></label>
    <label>Output field of view<select id="export-fov"><option value="reference-native">Full native reference FoV</option><option value="reference-working">Current reference FoV · includes reference crop</option><option value="moving-native">Full moving-image native FoV · transformed grid (linear only)</option><option value="moving-working">Moving working/cropped FoV · transformed grid (linear only)</option></select></label>
    <label>Output resolution<select id="export-resolution"><option value="moving-native">Moving image native spacing</option><option value="reference-native" selected>Reference image native spacing</option><option value="working">Reference working spacing · downsampled if configured</option><option value="custom">Custom spacing (mm)</option></select></label><div class="step-grid export-spacing">${['X','Y','Z'].map((a,i)=>`<label>${a}<input id="export-spacing-${i}" type="number" min="0.01" max="100" step="0.1" value="1" disabled></label>`).join('')}</div>
    <label>Image interpolation<select id="export-interpolation"><option value="nearest">Nearest neighbor</option><option value="linear" selected>Linear</option><option value="cubic">Cubic</option></select></label></div><div class="export-presets"><span>Downsample moving native spacing:</span>${[2,4,8].map(f=>`<button data-export-factor="${f}">${f}×</button>`).join('')}</div><p id="aligned-export-summary" class="hint" role="status"></p>
    <button id="save-aligned" disabled>Save aligned image</button>
    <div id="spline-field-export" class="hidden"><label>Displacement field spacing (mm)<input id="spline-field-spacing" type="number" min="0.2" max="20" step="0.2" value="2"></label><p class="hint">Fields sample the accepted nonlinear mapping on a physical grid; 2 mm limits export size. JSON retains the full mapping definition. Interpolation by other software may differ.</p><button id="save-spline-fields">Save nonlinear displacement fields (RAS mm)</button></div>
    <button id="save-transform" disabled>Save transforms and landmarks</button>
    <button id="save-session-export">Save reloadable session</button>
    <details id="generated-exports"><summary>Standalone masks and working images · no registration applied</summary><label>Generated image<select id="generated-image-choice"></select></label><button id="save-generated-image" disabled>Save selected generated image</button><p id="generated-image-status" class="hint">Binary masks are saved as 0/1 on their working grid. Masked working images retain current crop and edits. These files have no registration applied. Display adjustments are never exported.</p></details>
  </div>
</div>
`
}
