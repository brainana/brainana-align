# 0.17.9 · viewer reliability

Source and overlay views share normalized bounded wheel zoom and fixed pixel-width crosshairs. MRI, CT/T1/T2/EPI labels and registration methods remain independent of that viewer implementation. Six source panes and three review panes have renderer-command coverage for high zoom and physical geometry. Native GPU visual verification is still required on the review build.

## Earlier release documentation

# 0.17.8 · profile scope

AFNI/FSL/ANTs/JIP/Elastix settings are selectable within Defaults at the top of Linear registration. All five transform model choices remain independently editable. Region and preprocessing state are preserved. Source-dependent limitations are visible in each profile and in saved provenance. No external linear workflow is introduced. Both nonlinear methods still require an accepted proper linear alignment. Native runtime/GPU behavior is unchanged.

All profiles populate the same editable Linear registration controls; no AFNI/FSL/ANTs/JIP/elastix linear executable or second engine is included. Profiles map supported settings to Align and display source-specific optimizer, masking, sampling and stopping differences. They do not claim identical results to those programs. Native nonlinear methods and the compact runtime remain unchanged.

## Earlier release documentation

# 0.17.7 workflow and deployment parity

Linear registration retains rigid 6, similarity 7, independent scale 9, affine 12 and custom groups, broad/nearby orientation, matching region, histogram normalization, metrics and sampling/search settings. ITK no longer duplicates those linear controls. JIP and ITK nonlinear methods require an accepted linear base; existing pending-proposal Accept/Reject, full-map sessions/export, direction and physical masks remain.

Native ITK retains Mattes mutual information and neighborhood correlation, dense/seeded sampling, multiresolution B-spline fitting, exact supported fixed windows, exclusions, diagnostics and N4 settings. Unsupported moving windows and sheared working grids are refused explicitly. It is not elastix. Brand-named defaults are not inferred from similar settings. Standalone JIP remains separate and unchanged. This review release builds Apple Silicon first.

## Earlier release documentation

# 0.17.6 · foreground-aware linear fitting for prepared images

In Image-based registration → Linear registration, click **Use prepared-image settings**, then choose the transform model and **Run linear registration**. The explicit preset selects nearby staged fitting, nonzero fixed-image foreground, mean-normalized mutual information and bounded rotation. Review and accept/reject the proposal normally. Rigid/similarity/anisotropic/affine/custom remain available. The unchanged broad orientation search, all-included-voxel matching and legacy percentile histogram ranges remain the launch defaults for compatibility.

Nearby fitting starts from the original images' physical coordinates. It does not use a previous accepted transform and is distinct from landmark refinement. Translations, rigid pose, axis scales and all enabled parameters are introduced in stages at finest registration resolution; Standard sampling uses a 60,000 sample target. Rotation limits are at most 30 degrees. Use broad search or establish an initial alignment when images are far apart. Switching back to broad restores its rotation range and pyramid controls.

Foreground uses nonzero ORIGINAL working voxels in the fixed image, intersected with existing physical regions and exclusions. It is intended for a skull-stripped/zero-background reference; valid zero-valued tissues can be excluded, so choose the policy deliberately. Sparse coarse foreground sampling uses finer samples without dropping any constraints. Mean-normalized common-range histograms apply only to MI/NMI. These are scoring operations, not N4 correction, image masking or intensity editing; source/working arrays, display and export intensities are unchanged. Session/export provenance records the selected policies; old sessions remain readable.

The provided anatomy/EPI pair is checked through real packaged loading, preparation and workers with 6- and 12-parameter models. Evidence distinguishes qualitative overlay improvement from ground-truth accuracy; the implementation is not claimed to reproduce native JIP numerics exactly. Optional JIP nonlinear and ITK routes and their private numerical runtime are unchanged (worker version 0.17.5). This candidate contains one self-contained Apple Silicon Align.app, complete source, consolidated documentation, validation, manifests and SHA-256 checksums. Native graphical/GPU hardware checks and notarization remain pending.

## Earlier release notes

# 0.17.5 feature organization

One self-contained Align app incorporates nonlinear registration; standalone JIP is not shipped with it. Align's existing linear models remain in Linear registration and landmark alignment. JIP nonlinear registration follows an accepted linear fit and does not run another linear search. Optional CPU/GPU paths, physical optimization windows/exclusion masks, proposal review, complete nonlinear sessions and native export are preserved. ITK remains a distinct CPU method with explicit provenance. The guide teaches the actual JIP nonlinear controls without changing parameters or executing tools.

## Historical release notes (superseded where deployment differs)

# 0.17.4 method presentation

The image-based panel presents Linear, JIP and ITK registration as peer expandable choices. Built-in linear6/7/9/12/custom models and companion stages/metrics remain unchanged. The guide locates real controls instead of performing their actions. No elastix implementation is claimed.


## Previous release documentation

# 0.17.3 additive capabilities

All prior Align rigid6/similarity7/anisotropic9/affine12 landmark models, landmark spline, manual adjustment, constrained refinement and independent image-based routes remain available. New ITK is a separate option and offers6/7/12 linear models; it does not replace the9-parameter Align model or original JIP. Standalone JIP also exposes the optional ITK dialog with normal proposal decisions. Existing optional Metal/CUDA belongs to JIP; ITK uses CPU.

Reproducible starts/metrics, acquisition provenance, count-gated landmark sensitivity and full-field export are included. Dedicated independent anatomical validation annotations, TOPUP/BBR/field-map estimation, automatic phase inference, elastix, GPU ITK, visible-slice-only rendering and refreshed non-Apple-Silicon binaries are outside this candidate.


## Historical documentation

## 0.17.2 update

Align 0.17.2 uses the optional JIP 0.8.3 companion. The domain IPC validates bounded mask bytes and physical windows, requires sampling-v1 and rejects unconfirmed sampling. The native additive jip_sampling ABI leaves existing configuration structs unchanged; fixed selection is copied, moving membership follows the pull, sample scratch is copied/released with each transactional trial. JIPCP004 explicitly serializes sampled state; older unmasked formats remain supported.

ManualPreview.ts separates compatible display-only affine deltas from the full mapping. The display snapshot shares immutable image pixels; exact resampling resumes after a pause. Both image targets/directions and nonlinear composition are tested. The default algorithm and CPU route are unchanged. See README.md, TECHNICAL_FINDINGS.md and VALIDATION.md for scope and evidence. The existing build commands apply; refresh desktop:prepare before packaging.


## 0.16.84 — Current Viewer assessment

Pinned comparison: [Viewer aaa47a989329f9b42c97a592ad0801b7d896216e](https://github.com/brainana/brainana-viewer/tree/aaa47a989329f9b42c97a592ad0801b7d896216e), 2026-10-20. This is a source comparison, not certification of either app's native rendering.

| Area | Assessment and decision |
|---|---|
| Dedicated desktop | Both use Electron, an app-specific workspace, shared desktop/launcher/server/client packages and a loopback runtime. Retain Align's hardened window and lifecycle. |
| Local/network loading | Both distinguish local/mounted paths from in-app SSH/SFTP browsing, remember nonsecret connection identity and verify host keys. Align opens scalar image files in its existing workspace; Viewer registers dataset sources. Keep those distinct workflows. |
| Visual language | Both use Source Sans 3, IBM Plex Mono and graphite/amber design tokens. Align also supports user-selected palettes. Preserve preferences and larger guided controls rather than forcing Viewer's denser 20px controls into registration. Removed the requested welcome slogans. |
| Layout | Align retains one-row toolbar groups, ordered independent collapsed sections, three source/review planes and modal/guide ownership. Viewer organizes surfaces, atlases and reports in its dashboard; those functions do not belong in Align. |
| State | Both favor typed state/component ownership. Align's fit/proposal/native/working/processing transaction must remain explicit; Viewer's small Store is a keyed holder, not a reactive state engine to adopt for its own sake. |
| Domain math | Registration, constrained refinement, spline inversion, physical windows, masks, sessions and transformed export remain Align responsibilities. Viewer's surface/ROI projection math is not a replacement for registration fitting. |
| Worker architecture | Viewer uses persistent projection jobs. Align uses one-shot cancellable workers for volume/registration/export operations. Shared lifecycle now covers preparation/nonlinear tasks; long-lived worker pooling is a separate design decision because retained images increase memory. |
| Packaging | Both separate frontend build from desktop payload and supply corresponding source. Align preserves exact packaged worker/source, architecture, signature, dependency and manifest checks. |
| Remaining drift | Theme tokens and some runtime concepts are duplicated rather than one versioned shared upstream dependency. Future extraction should preserve palette overrides and validated network/export contracts; no untested wholesale source sync was performed. |

The comparison inventory counts byte-identical versus adapted files and one-app-only files. It intentionally does not treat byte differences as bugs or infer API compatibility from matching filenames.

## 0.16.83 — Additional desktop platforms

The same 0.16.83 source is packaged for Intel macOS (x64), Windows (x64 installer and portable ZIP) and Ubuntu (amd64 Debian installer and portable tarball), in addition to the Apple Silicon release. Platform verification checks exact frontend/worker contents, dependency resolution, architecture and installer payload integrity. Native Windows/Ubuntu launch, GPU behavior and installation/removal remain pending on their target systems.

## 0.16.83 — Readable guide paragraphs

Separated the starting options into distinct paragraphs with bold labels for optional Initial processing, landmarks, manual adjustment and independent image-based registration. Applied the same approach to dense welcome, resolution, masking, voxel editing, landmark-count, spline, automatic registration, manual adjustment, refinement, region, proposal-review and export instructions. Short prompts remain compact. Guide routing and data actions are unchanged.

Guide text uses a small text-node renderer for paragraph breaks and bold emphasis, never HTML interpretation. Existing teaching checks verify the option labels, paragraph structure and literal HTML handling; actual compiled frontend checks verify formatted welcome instructions. Apple Silicon review release; native graphical certification remains pending.

## 0.16.82 — Restart shows starting choices immediately

Restart guide cleared history but sidebar navigation immediately replaced the starting choices with the same Close Initial processing prompt, making it appear ineffective. Starting-choice screens and loading prerequisites now display immediately without first requiring sidebar changes. After a user chooses a concrete task, ordinary close/open/control teaching resumes. Restart keeps loaded images, transforms, landmarks and disclosure states, clears instruction history and pending continuations, and retains prerequisite/proposal/busy safeguards.

Added direct visible-step regressions from all seven Close-panel prompts, including the reported Initial processing case, without the auto-follow helper that previously hid this issue. Tests require immediate starting choices, stable refresh, empty Back history, preserved data/panels and resumed panel teaching after a new task choice. Registration choices after initial processing similarly precede panel navigation. Apple Silicon review release; native graphical certification remains pending.

## 0.16.81 — Remove guide Reset shortcut

Removed Show Reset workspace from the guide and its unused click handler. The main-toolbar Reset workspace button retains its confirmation and behavior. Restart guide continues to restart instructions while keeping loaded images and registration work. Updated existing guide and compiled frontend checks to require absence of the removed shortcut; package verification rejects bundles containing it. Apple Silicon review release.

## 0.16.80 — Dialogs take priority over the guide

Replaced the Resolution-only guide visibility exception with a shared rule for all open application and native HTML dialogs. Loading either image, network browsing, the workstation browser and export-folder picker, Resolution, Settings and Export cannot be covered by the guide or its Resume button. All standard modal backdrops also stack above the guide. Instructions and manual placement are preserved; closing the last dialog refreshes the guide from current application state. Closing a dialog never resumes an intentionally paused guide.

Suspend guide projection, automatic positioning and focus/scroll changes while any dialog is open. Escape ownership is captured before a dialog handler closes it, preventing the same Escape from pausing the guide. Added real MRI/CT source-chooser, seven-dialog CSS/stacking, nested/replaced/removed/hidden/native dialog, focus/scroll and pause/resume regressions. Exact compiled frontend tests apply its actual stylesheet to reproduce the reported overlap for both image selectors and check the other dialog types. Native GPU/visual certification remains pending. Apple Silicon review release.

## 0.16.79 — Guide begins on the welcome screen

Starting the guide from either entry point keeps the welcome screen visible and highlights all four pairing choices. Only the user choosing a pairing opens the workspace. Pair selection never loads files. Welcome, missing images and pending preparation take precedence over remembered routes and Back; completed entry prerequisites are excluded from instruction history. Repeated welcome Guide keeps an active guide open. Docking reserves space on the welcome screen.

Integrated startup tests install welcome, guide, workflow and source chooser together, with both installation orders, both launchers, all four pairings and either header-selection order (32 sequences). They exercise cancel/retry, pause/resume, Restart, preparation failure/completion, plus four saved-session paths. Actual compiled frontend tests now start the guide before selecting a pairing and verify the welcome screen and highlighted choices remain visible. Prior bounded sidebar tests did not cover this integration boundary and were insufficient to detect the shortcut. Native GPU/visual verification remains pending. Apple Silicon review release.

## 0.16.78 — Guide ordering audit

Cancelled pending processing continuations on Back, Restart, guide completion and reset, while retaining pause/resume. Newly delivered registration proposals supersede older control-location and Back instructions; explicit Show Accept/Reject requests remain available until a decision. Rejecting a first proposal also clears an earlier Show Accept instruction. Added bounded ordering regressions across all 128 sidebar disclosure combinations, 12 control destinations and 1,600 seeded mixed actions. This is bounded orchestration testing, not proof of every possible action sequence or native GPU behavior. Apple Silicon review release.

## 0.16.77 — Visible guide heading frames

Open/close-panel guide prompts now draw their accent frame inside the summary heading, so the panel’s overflow clipping cannot hide it. Each new heading flashes three times, then keeps its frame. Show heading and Follow replay the attention cue without opening, closing or executing any control. Reduced motion keeps the steady frame only. Apple Silicon review build; native graphical certification remains pending.

## 0.16.76 — Teaching walkthrough

Teaching walkthrough: close current panel, open required outer/nested headings, then use the real field/control. Guide Show actions cannot set landmarks, fit transforms, accept/reject proposals, save/reset or apply pending brush edits. Native actions advance guide progress, including point pairs and completed window editing. Users may still pause, restart, go Back, choose a route and finish the guide. Registration algorithms, image geometry and export/session formats are unchanged.

## 0.16.75 — In-app network browsing

Network loading now follows the current Viewer in-app approach: SSH-config host recall, password/agent/private-key authentication, known-host verification, folder navigation and image selection. Connecting does not clear current work. Multiple local/network images can be prepared together. The local option still supports system-mounted shares. Binaries are published for Apple Silicon and Intel macOS, Windows x64 and Ubuntu amd64; the separate macOS browser edition includes both Mac architectures.

## 0.16.74 — Local and network loading

0.16.74 adds a Viewer-inspired local/network chooser to each volume input. Labels follow MRI–CT/T1–T2/T1–T1/MRI–EPI profiles. Local includes mounted network drives. Connected workstations expose the existing SFTP browser. New SSH setup remains packaged-macOS-only; other systems can use mounted shares. Workspace replacement on connection changes is explicitly disclosed and confirmed.

# Feature parity status

Current update (0.16.73): startup now presents a welcome screen explaining preprocessing, registration, review and export, with MRI–CT, T1–T2, T1–T1 and MRI–EPI choices. The choice labels both image slots across controls, directions, guide, preparation, review and export; duplicate T1 volumes are numbered. Pairing is saved in schema11 sessions and export metadata without renaming original files or changing coordinate mathematics/settings. Image Paint uses the entered intensity, Erase writes0, and mask Paint/Erase stay1/0. New recipe6 paint overrides honor the entered value even on a previously masked image; explicit later mask application can mask those painted values again. Editing shows the cursor's working-voxel intensity at the active panel's lower right, plus mask and pending values when relevant. Hover does not move the crosshair, mutate data, upload textures or start a worker. Release platforms: Apple Silicon and Intel macOS, Windows x64 and Ubuntu amd64, plus the separate macOS browser edition. Native graphical certification remains pending.

Current update (0.16.72): Voxel editing has its own explicit Edit target selector: MRI image, CT image, MRI binary mask or CT binary mask. Generated-mask targets become available after creating that mask. Image actions and binary Paint1/Erase0 are separate. Mask edits update the generated selection only, never source intensities or an already applied mask snapshot; explicitly Apply generated mask to working image to use the corrected selection on image data. Radius is a numeric integer field from0–16 working voxels;0 selects a single voxel. Click/drag previews remain sparse and worker-free; Apply/Finish commits once. Manual mask corrections run after hole filling and survive regeneration, sessions and binary/native-processed exports. Clear mask removes corrections. Recipe5/schema10 retain compact original-working-grid overrides and reject malformed/nonbinary or out-of-grid entries; older schemas remain readable. Apple Silicon macOS only; native GPU interaction remains unverified here.

Current update (0.16.71): voxel editing now collects sparse click/drag strokes with an immediate bounded orange slice preview, without starting a worker, scanning/copying a whole volume or reloading source textures per pointer event. Single voxel remains the default; 3D spherical brush radii of1/2/3/5/10 working voxels edit surrounding regions. Sphere dimensions follow the working voxel grid and may be physically anisotropic. Apply edits or Finish rebuilds the image once for the batch. Other button actions first apply the batch and replay once after success; input-file selection requires Apply/Discard first. Failed/cancelled application preserves the draft for retry. Undo/Redo operates on strokes while editing and on the applied batch afterward; Discard removes only unapplied edits. Native/base arrays, geometry, landmarks, fit and optimization windows remain unchanged during painting. Unapplied drafts are not retained after closing the app. The existing total20,000-voxel per-image recipe cap remains; use masks for larger regions. Saved/exported data use the committed recipe, never preview pixels. Apple Silicon macOS only. Native graphical responsiveness is not certified in this sandbox; committing still performs one full reconstruction and texture update, including mask/hole recomputation where applicable.

Current audit release (0.16.70): fixed selected-affine/header spacing disagreement in decoder fallback; exact linear/cubic samples no longer inherit NaN/Infinity from zero-weight neighbors; unchanged-grid physical crops relocate excluded crosshairs; nonfinite voxel clicks are rejected; preparation/generated/native/processed export workers clean up synchronous message-cloning failures; resetting the initial landmark alignment restores its model, parameter count and diagnostics as well as its matrix; transform metadata reports original-landmark diagnostics separately from current affine diagnostics. Failed overwrite/rollback retains the recoverable original, and backup-cleanup failure cannot trigger destructive rollback. The obsolete hidden refinement entry now delegates to the current cancellable worker, and legacy pending-proposal rejection restores the accepted pose. Added independent adversarial regressions and 320 seeded all-pair transform recoveries. Apple Silicon macOS only. Native GUI/GPU, real anatomical accuracy and large-volume memory/performance remain unverified; test success is not proof of absence of all bugs.

Current update: Rename the manual initializer to Begin manual alignment, with plain-language help: create a starting overlay so MRI or CT can be moved, rotated, scaled or sheared; initial overlap may be poor. Show this button/help only when no alignment exists, including after rejecting an initial proposal with no previous alignment. Hide and disable it for every existing alignment, including pending proposals; use the manual controls or explicit proposal decisions directly. The underlying legacy initializer handler also refuses to replace an existing fit, so old/programmatic routes cannot reset registration. Update guide copy and packaged-label requirements while preserving internal image-geometry provenance, physical coordinates, registration, session and export formats. Apple Silicon macOS only; numeric version0.16.69.

Current update: Pending refinement is a proposal, so manual editing and new registration remain locked until an explicit decision. The guide previously said Accept/Reject but offered only Inspect, and users switching to Manual could not reach the hidden decision controls. The guide now delegates Accept, Reject, Accept-and-adjust-manually and Reject-and-adjust-manually to the existing canonical handlers for local, global and spline proposals, plus Save session for later. Manual adjustment has visible inline decision buttons above the locked controls. Decisions never silently occur when selecting a workflow or pressing a disabled nudge. Accept/reject returns the next-step hub; direct manual choices reveal enabled manual controls, or the image-geometry initializer when rejecting a first proposal without a prior alignment. Guide refresh keys track proposal presence and kind so choices cannot go stale. Landmark collection stays on one neutral Add-pair step after the minimum, with no primary Apply promotion or automatic expansion of Landmark alignment; users choose when to fit. Added a practical five-pair rigid starting set (anterior/posterior/superior midline and left/right lateral mid A–P), with identifiable paired anatomy, distribution and overlay review emphasized rather than a guarantee from count. Introductory text explains display versus masking/cropping/voxel editing, and the post-processing action says Start with landmarks. Opacity runs40% faster (an8-second cycle becomes about5.71seconds), retaining the same range, shader-only updates and stop paths. Apple Silicon macOS only; numeric version0.16.69.

Current update: The guide now walks through Image display, Binary mask, Cropping and Voxel editing as four optional stages, with explicit Continue/skip and one-step Back. Creating/applying a mask or changing a slider never completes an entire stage. Completed landmark alignment and accepted/rejected registration proposals return to a next-step hub with all sidebar sections collapsed; new landmark placement and manual adjustment wait for the user. Inspect alignment is a prominent guide action that reveals Alignment display. Opacity animation cycles smoothly from25% to75% and back over approximately5.71seconds, using the existing shader opacity path without resampling; the button, manual slider input, hidden document, reset and page close stop it. The export dialog now centers on the transformed moving image, with independent data source (native, native with applied preprocessing, or current processed working image), FoV, native/working/custom spacing and nearest/linear/cubic interpolation. 2×/4×/8× presets derive from moving-native spacing. The summary shows actual endpoint-preserving grid spacing, voxel count and uncompressed size. Full native reference FoV can be retained independently of processing; linear transforms additionally support full native or cropped moving FoV. Nonlinear splines use reference FoVs; moving FoVs are explicitly unavailable. Native preprocessing projects the saved working-grid mask and edited voxel footprints into native coordinates with nearest-cell selection, preserves other native intensities, pads outside crops with0, and does not apply registration exclusion masks to intensity data. Working-resolution masks cannot recover native segmentation detail. Generated masks and unregistered working images have a separate disclosure. Redundant original-input copy choices are removed; cropped or edited working images are offered even without a mask. Aligned image sidecars and transform JSON record selected options/geometry; display window/contrast is never baked into exported data. Cubic interpolation can overshoot; downsampling here uses the chosen interpolator without a separate anti-alias filter. Version numbers are numeric. This candidate ships Apple Silicon macOS only; other platform builds wait for approval of stability. Native graphics remain pending in this sandbox.

Current update: Manual adjustment now offers independent Adjust MRI / Adjust CT targets without changing the saved export direction. Twelve controls provide reference-RAS translation (mm), rotation (degrees), positive axis scales (%) and XY/XZ/YZ shear; rotation/scale/shear pivot around the other image’s center. Positive scale bounds are 5–2000%; shear is bounded to ±3. Reset all manual adjustments restores the accepted base. Optional transformed source-row preview shares the already resampled overlay array, labels its reference grid, and restores native views before landmark, region or preprocessing edits. Original voxels remain unchanged. Start from image geometry uses NIfTI physical coordinates without estimating a registration. Manual affine controls compose on both sides of the canonical mapping; spline coefficients/control snapshots remain unchanged, with a proper input affine and the same-map inverse retained. Schema9 saves both targets’ controls and validates recomposed matrices; older sessions remain supported. The free guide-window dragging fix is included. Native graphical appearance and interactive rendering remain pending a graphical run.

Current fix (0.16.69): manual guide placement is clamped only to the viewport. Previously, manual drags were passed through automatic image/control avoidance, rejecting positions over source panes and snapping the guide back to the sidebar. Pointer and keyboard header movement now float the guide anywhere, including out of automatic or preference-selected docking. Dock CSS yields to an explicit manual-floating override across desktop/narrow layouts. The dock preference itself is preserved. Same-step refreshes, scrolling and resize retain the manual position; Follow highlighted control and new steps restore automatic avoidance/docking. Close/reset clean up the override. Unrelated pointer releases cannot interrupt a drag. Registration, physical optimization windows, images and mask algorithms are unchanged. Real DOM regressions cover crossing into protected panes, dock escape, keyboard movement, viewport clamps, refresh/scroll/resize, Follow, new steps and state preservation; compiled UI tests exercise horizontal dragging and override cleanup.

Current update (0.16.64-mask-holes.1): after Initial processing, Continue acknowledges completion and offers registration methods (or the current alignment); Back returns one step to processing. Binary mask adds optional 3D enclosed-cavity filling (6-connected background) or independent voxel-Z slice filling (4-connected background), off by default. Exterior flood filling preserves the original foreground and does not dilate/smooth the outer boundary. Slice planes follow the working voxel grid, including oblique acquisitions. Open cavities stay unfilled in 3D; slice filling may include interiors open in 3D and real anatomical cavities. Select a mode, Create/update, review the generated green preview, then explicitly Apply if desired. Slider changes show threshold drafts until Create; Apply uses the saved generated rule. Filling does not alter source data or run registration. Cancellation, Undo/Redo and errors preserve transactional state. Recipe4/schema8 save the compact rule and reconstruct masks; schemas0–7 remain readable. Generated filled masks use one resident Uint8 per original working voxel. Mask-only creation thresholds in cancellable chunks and transfers bytes to a worker, avoiding a full float-volume clone; span flood-fill scratch is capped at32MiB with explicit failure. Applying/editing/exporting still uses the existing full-volume processing allocations. Working-image and binary-mask exports serialize voxel values directly, without identity interpolation, preserving exact0/1 masks and underlying intensities. Native/registration resampling is unchanged.

Current fix (0.16.63-mask-preview.1): synchronize mask previews with completed source-renderer draws, including deferred parent resizing and progress-strip layout changes. Previously, input/window listeners could paint before the renderer updated its projection, leaving a cached preview stretched over the resized CT. Binary-mask creation now preserves the source viewer/texture when voxel data and NVImage are unchanged. Threshold, mask application, exported geometry and registration math are unchanged. Regression checks use the installed Niivue draw path and captured shader matrices across all six source views and wide/tall/high-DPI backing sizes; asynchronous draws must update previews without another user action. Real-worker mask application/export tests remain in place. Native graphical confirmation is still pending in this sandbox.

Current update (0.16.62-binary-mask.1): display intensity/windowing remains view-only, independent from Binary mask. Removed display-range copying and registration-sampling assignment from the mask tool. Threshold sliders preview voxel selection; Create/update binary mask saves it, and Apply generated mask optionally fills outside with0 in the working image. Changing preview thresholds does not change the saved mask or an already masked image until explicitly updated/applied. Mask creation preserves source voxel arrays and the existing overlay. Export offers actual available original/native or original-working images, binary masks (1selected/0excluded) and masked working images, without requiring alignment. Aligned-image/transform exports remain a separate gated section. No display brightness/contrast is baked into any mask or data export. Recipe3/schema7 store generated masks; schemas0–6 remain supported, including old applied masks and sampling exclusions. Original inputs stay immutable.

Current fix (0.16.61-preprocessing.1): applying a mask modifies only the selected working image. Preprocessing neither runs registration nor automatically resamples an existing overlay. The saved transform and landmarks stay unchanged; later registration uses the updated working data. Existing overlays are invalidated with an explicit Refresh overlay action that uses the unchanged transform. The shared progress/cancel/guide now distinguish preprocessing, preparation, export and display from registration. Previously every busy task was called Alignment in progress/Cancel registration, and processing also rebuilt existing overlays automatically. The mask handler did not call the optimizer, but those behaviors made preprocessing appear to run registration. Regression checks cover CT-only updates, unchanged MRI and accepted nonlinear transform, no registration workers/automatic overlay refresh, and correct task labels.

Current update (0.16.60-mask.1): Initial processing adds independent MRI/CT intensity-mask sliders with a live green selected-voxel overlay in all three source views. Preview is independent of contrast and applied masks. Use current display range copies endpoints explicitly. Use mask for registration preserves intensities; Apply mask to working image fills outside selection (default 0). Their saved ranges are independent. Invert, Clear applied masks, Undo/Redo, export selection mask and schema-6 session replay are supported; schemas 0–5 remain readable. Threshold rules evaluate original working intensities plus voxel edits before mask fill, keeping repeat application stable. Crop and individual registration exclusions remain active. Filled voxels are not automatically excluded: use the registration mask explicitly. Preview caches at most 512×512 pixels per view and uses nearest-voxel physical sampling; slider changes do not resample/copy a full volume. Native input files remain unchanged, and aligned export defaults to native originals.

Current update (0.16.59-display.1): optimization-region controls are available directly in both Landmark refinement and Image-based registration, sharing the same physical windows. Initial processing instead provides independent MRI/CT display contrast and minimum/maximum intensity sliders, numeric endpoints and Reset display range. Contrast changes window width about its center. Display changes affect source and overlay views only; registration, voxel data and exports remain unchanged. Display ranges are dataset-local and reset when inputs are replaced; they are not saved as global intensity presets or session fields.

This release reconstructs the retained v0.15.4 frontend behavior in readable TypeScript. The recovered minified frontend and injected export patch are not part of the production runtime.

| Feature | v0.15.4 reference | current release | Source status |
|---|---:|---:|---|
| MRI and CT loading | Yes | Yes | Readable TypeScript |
| Landmark editing | Yes | Yes | Readable TypeScript |
| Rigid fit and manual adjustment | Yes | Yes | Readable TypeScript |
| Automatic NMI refinement | Yes | Yes | Readable TypeScript |
| Optimization refinement windows | Yes | Yes | Readable TypeScript |
| Review overlay | Yes | Yes | Readable TypeScript |
| Transform and aligned-volume export | Yes | Yes | Readable TypeScript |
| Session save/load, including windows | Yes | Yes | Readable TypeScript |
| Workstation file browser | Yes | Yes | Readable TypeScript |
| Workstation export folder browser | Yes | Yes | Readable TypeScript |
| Bundled dual-architecture Node | No | Yes | Packaged runtime |
| Remote Node/server upload | Yes | No | Replaced with local SSH architecture |
| Compiled recovered frontend required | Yes | No | Absent from production runtime |

## Validation boundary

Build, static parity, server, packaging, and synthetic algorithm checks were completed in the current environment. Finder launch, interactive WebGL behavior, and institutional SSH/Duo remain real-machine smoke tests.


## Overlay crosshair synchronization
Validated in source and production build: changing the crosshair in any overlay plane updates sagittal, coronal, and axial overlay views through the shared 3D coordinate state.


## Large-orientation landmark alignment

The six-parameter rigid landmark workflow now supports arbitrarily large initial orientation differences in physical NIfTI world coordinates. Existing transform direction, review overlay, residual reporting, manual nudging, refinement, session, and export behavior are retained.

## Four-model landmark extension

Rigid, similarity, positive anisotropic no-shear, and affine models are implemented in shared readable TypeScript and packaged into the app. Diagnostics, reflection confirmation, model-specific export, schema-2 restore, general affine resampling, and rigid-delta refinement are integrated. The exact packaged production bundle is exercised by a DOM test for all four options and reflection apply/export behavior. Full WebGL and real-data interaction still require browser/native certification.

A dedicated A–P/I–S/L–R anatomical-frame initializer is deferred. The six anatomical points already work as ordinary correspondences.

## Guided setup extension

An opt-in adaptive guide covers loading, corresponding landmark placement, transform fitting, existing-geometry initialization, manual adjustments, local automatic refinement, proposal review, optional windows, and export. Existing registration behavior is available without starting the guide. Executable state/DOM and production-bundle tests cover the new entry point; native positioning and WebGL interaction still need real-browser validation.

## Standalone desktop extension

The native default bundles Chromium, provides a dedicated window and standard menus, redirects downloads to native save dialogs and stops its backend on quit. Workstation access remains available through the existing macOS authentication adapter. Shared packages now hold platform and math implementations; application features remain in apps/align. Browser mode and existing 6/7/9/12-parameter models, guided setup, sessions and exports are retained. Native release checks and real institutional SSH coverage are reported in VALIDATION.md.

## Workflow focus, progress and opacity

Stable Add/Apply guide actions, method-focused panels and alignment activity are included. Automatic refinement remains the inherited local NMI search; no new global registration solver is introduced. Opacity changes use a shader-only draw path and do not change registration/export calculations.

## Automatic registration addition

The supplied v0.17.26 source contains broad automatic linear registration. This release adapts its automaticLinear/imagePyramid algorithms into the current shared workspace, retaining the current desktop shell and workflow improvements. Four standard models, custom parameter groups, five metrics, global initialization/local refinement, windows, cancel/progress, proposal review, sessions, and exports are implemented. Nonlinear landmark warps and deformable image registration from that separate source are outside this linear-registration update.

Settings adds independent saved themes/display/guide/interaction defaults, configuration presets and session recovery; it does not alter registration matrix conventions. All 6/7/9/12-parameter fitting and bounded refinement remain available.

Nonlinear landmark spline: 12–128 controls, normalized 3D kernel and smoothing, conservative invertibility bound, preview acceptance, complete JSON/schema-4 persistence, exact same-map inverse, cancellable resampling and sampled forward/inverse RAS field exports. Image-based refinement composes a linear correction after the spline; spline coefficients are not intensity-optimized.

## Workflow separation — 0.16.47-workflows.1

The interface separates landmark fitting, landmark follow-up (nonlinear paired-point spline and constrained rigid image-based refinement), manual adjustment, and independent global image-based registration. Both intensity workflows edit the same physical optimization regions. Global registration keeps rigid/similarity/anisotropic/affine/custom options. Legacy landmark and local sessions remain usable; image-geometry/global starts do not enable landmark-only refinement.

## Working-resolution audit — 0.16.48-resolution.1

Added jointly planned, user-controlled working images and native-grid image export. Scalar single-file NIfTI-1/2 support bounded compressed/uncompressed header reads, streaming selected-frame preparation, normalized antialias filtering and physical endpoint preservation. Legacy decoders remain available with clearly labelled unknown peak/native fallback. Current-memory guidance is available in standalone platforms; browser system capacity can be unavailable. Shared CPU image storage and active-level pyramids reduce duplication; separate GPU contexts remain.

## Resolution layout and capacity correction — 0.16.49-resolution.2

Single-row toolbar groups MRI/CT/Resolution under Volumes and separates Session from Landmarks. Resolution has a three-column spacing row and ordered native-relative 1×/2×/4×/8× presets; recommendation is a separate action. Multi-volume selection is conditional and labelled Volume / time point. Current free memory is contextual: it excludes already-resident workspace allocations and may omit reclaimable caches. Do not compare the full planning envelope to free memory or call it measured pressure. Compare lower/upper estimates with installed RAM minus headroom (or explicit budget) for strong/advisory guidance respectively. Buffer/GPU peak estimates remain uncalibrated conservative ranges; real GUI/peak-memory measurement is pending. Protect native/working coordinate and export contracts.

## Landmark guide synchronization — 0.16.50-guide.1

Landmark Set targets may not exist until the scheduled render completes. Never cache Guided setup as their fallback. Resolve the actual selector before the refresh fast path, observe landmark-list child changes and protect the whole active landmark row plus its required image panes from guide overlap. Recreated nodes reacquire the outline without restarting the same-step pulse. Track displayed MRI/CT steps in guide history; Back revisits instructions and preserves coordinates. Completed historical points offer Replace/review and Continue with current progress. Pending points offer Set MRI/CT through the existing app action; Add/Apply remain on completed-pair screens. DOM regression reproduces delayed and rebuilt targets plus two completed pairs, adding a third, Back to previous CT/MRI, preserved data and resume. Native GUI verification remains pending.

## Independent sidebar sections — 0.16.51-sidebar.1

The six top-level sections are ordered Landmarks, Landmark alignment, Landmark refinement, Manual adjustment, Image-based registration and Alignment display. Native disclosure heading clicks affect that section alone. Opening a section activates its controls; closing a section does not select another operation or change sibling open states. Explicit workflow shortcuts retain focused navigation. Both registration sections can be independently open; a named Show controls action moves the single shared controller without changing disclosure states. Guide refresh/recreated rows do not reopen a manually collapsed section; a new step or explicit Follow highlighted control may reveal its required section. DOM regressions exercise every heading in both directions, operation ownership, shared controls, guide repaint and preserved region editing/settings. Native GUI validation remains pending.

## Single-action guide completion — 0.16.52-guide.1

Finish guide immediately completes and closes the guide without a second End guide screen. Workspace images, landmarks and registration remain unchanged. Completion removes highlighting and Resume; × still hides/pauses. The old done route is accepted for saved-progress compatibility but uses the same single Finish guide action. The existing real DOM regression verifies one click, closed state, cleared highlighting, no Resume and unchanged workspace state. Native graphical validation remains pending.

## Neutral startup workflow — 0.16.53-startup.1

All workflow buttons start unselected. The static Landmarks default and saved-layout workflow click replay have been removed. Remember panel expansion restores the six independent disclosure states, ignores legacy workflow fields and saves panels only; it must not select a tool or registration operation. Explicit user/guide navigation may highlight the chosen workflow. Real compiled startup tests seed an old Manual selection and assert no startup workflow click, no highlighted button and independently restored Manual/refinement panels. Fresh markup is also checked. Native graphical validation remains pending.

## Permanent registration forms and spline order — 0.16.54-refinement.1

Landmark refinement presents optimization regions and constrained image-based refinement first, then the optional advanced nonlinear landmark spline. Each local/global form has permanent independent controls; workflow changes never move form DOM nodes or borrow parameter values. A shared execution controller enforces a single active worker. Local runs remain rigid adjustments of an accepted landmark-derived base, preserving its affine scale/shear or full spline mapping. Regions still delegate to one physical state. Run eligibility is explained beside each button; spline needs accepted landmark alignment and at least 12 complete enabled pairs (20+ recommended). Local/spline proposal review lives in the refinement section; guide and export-review navigation target it. Tests verify fixed form ownership/order, independent posted worker settings, eligibility messages and accept/reject delegation. Native GPU/graphical validation remains pending.


## Spline belongs to Landmark alignment — 0.16.55-spline-ui.1

Landmark alignment contains the four linear models followed by the advanced nonlinear landmark spline. First accept a linear landmark fit, then use at least 12 well-distributed complete pairs (20+ recommended) to fit a spline. Smoothing, distortion cautions, readiness messages, preview results and Accept spline/Reject controls live together there. Landmark refinement contains only constrained image-based refinement of an accepted landmark-derived alignment. Guide and export-review navigation return spline proposals to Landmark alignment. The mathematical spline, physical coordinates, worker/controller and shared optimization regions are unchanged. Regression tests cover permanent panel ownership and spline proposal decisions. Native graphical validation remains pending in this environment.


## Collapsed startup and organization audit — 0.16.56-sidebar.1

Every sidebar section starts collapsed on every launch, including with legacy saved panels, workflow choices and rememberLayout=true. Removed the Remember panel expansion setting and its asynchronous storage restore/save handlers; the legacy settings field is normalized to false for compatibility. Sections remain independently expandable during the current launch. Closing the last section for a workflow clears its shortcut highlight without selecting another tool. Explicit guide/export navigation reveals all collapsed ancestors of the requested control, fixing nested spline guidance; refreshes still respect manual collapse. The section order remains Landmarks → Landmark alignment (linear models and advanced spline) → Landmark refinement (constrained image-based) → Manual adjustment → Image-based registration → Alignment display. Global/local forms, proposal gating, shared physical optimization regions, coordinates, fitting and resampling are unchanged. Compiled startup tests seed all six panels open plus legacy preferences and wait for late initialization. DOM tests exercise independent closing, neutral highlights and nested spline guide visibility. Full suite and packaged source/worker checks run for all releases; native graphics remain pending.


## Two-column Resolution — 0.16.57-resolution.1

Resolution now uses a wider two-column desktop layout: MRI and CT native/working resolution fields on the left; native-relative presets, joint recommendation, memory budget and live estimates on the right. X/Y/Z stay in one row. The title and Prepare workspace/Cancel footer remain outside the scrollable content, so actions stay visible. At widths of 760 CSS pixels or less the columns stack; short screens or enlarged text retain safe scrolling instead of clipping controls. Native/working geometry, presets, memory calculations, preparation cancellation/rollback and native-grid export are unchanged. DOM and emitted-worker tests verify both column ownership and footer placement alongside existing preparation behavior. Native graphical certification remains pending. Release assembly also avoids duplicating immutable temporary Mac payloads while constructing the full bundle; all archive file/symlink inventories remain verified.


## Initial processing — 0.16.58-processing.1

Initial processing is the first collapsed sidebar section. Display controls live here; optimization-region editors live directly in Landmark refinement and Image-based registration and share one physical state. Crop image draws a physical RAS box; drag in multiple orthogonal views or enter six bounds, then Apply crop. The output retains original grid orientation and voxel spacing, adjusts its origin, and zero-pads/excludes oblique corners outside the box. Original files and original working arrays remain unchanged. Individual voxel replacement defaults to 0; exclusion/inclusion are separate actions that preserve intensity. Masks gate metric sample centers and intensity-range sampling in physical coordinates at all pyramid levels; they do not change pyramid filtering/interpolation and are not segmentation masks. Undo/Redo stores up to 32 launch-local states; Restore original removes processing reversibly. Up to 20,000 individual edited/masked voxels per image are supported. Processing jobs and exports run in cancellable workers with rollback; pending registration proposals must be resolved first. Working-resolution changes require restoring originals first. Schema-5 sessions store active recipes and validate dataset, frame and original working grid before replay; older sessions remain readable. Export processed image + mask writes working-grid NIfTI images (mask 1=excluded, 0=included) and JSON recipe. Aligned-image export defaults to original inputs on native reference geometry; processed working inputs/grid require explicit selection. Registration JSON records processing provenance. Full regression and packaged-worker checks include oblique/axis-swapped crops, all six view clicks, original immutability, masks, undo/redo, sessions, worker errors/cancellation and native/processed export separation. Native graphical validation remains pending.
