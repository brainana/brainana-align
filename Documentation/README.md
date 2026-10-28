# Brainana Align 0.18.0

See the [project overview](../README.md), [software credits and scientific references](../docs/software-credits.md),
and [JIP provenance, repairs and integration changes](../docs/jip-changes.md).
The following feature guides remain applicable. Historical release notes retain
their original version labels; current qualification is recorded in each release's
validation reports and manifests.

# Brainana Align 0.17.10 · image flipping

In **Alignment display → Image comparison**, choose **Flip between images**. Use **Show MRI/CT** (or the selected image pairing) to switch manually, or **Auto flip · 1 s** to alternate each second. The label identifies the image currently shown and whether it is fixed or moving. Each view keeps its current slice/zoom and the same aligned grid. The other image is hidden, including through empty moving-image background. Display colors remain controlled by Display mode; choose Grayscale opacity for grayscale inspection.

Choose **Smooth opacity** to return to the existing slider and 25–75% animation at the retained blend setting. Flip state/playback is launch-local and does not overwrite the saved opacity or alter image data, transforms, landmarks, regions, registration or exports. Playback stops on manual flip, slider input, mode change, tab hiding, overlay clearing and workspace reset. Native graphical validation remains pending.

# Brainana Align 0.17.9

Viewer fix: trackpad/wheel zoom now follows scroll distance within safe bounds. Crosshairs retain a thin, consistent screen width even at high zoom. Use **Reset view** in the toolbar to restore every pane's zoom and field of view. Wheel zooms; Shift + wheel changes slices. Crosshair color, visibility and thickness remain in Settings.

The change is limited to viewer input and drawing. Existing registration options, linear software-default profiles, image processing, sessions and exports remain available. This Apple Silicon review release includes the self-contained app, complete source, documentation, validation, manifest and SHA-256 checksums. Native graphical verification remains pending in the agent environment.

## Earlier release documentation

# Brainana Align 0.17.8

Open **Image-based registration → Linear registration → Defaults**. Select AFNI, FSL, ANTs, JIP or Elastix, review the visible profile summary, and click **Apply defaults**. The checkbox **Keep my selected transform model** lets you use the scoring/search profile with your current rigid/similarity/axis-scale/affine/custom model. Previewing a selection does not apply it; applying it does not run registration. Review the fields and click **Run linear registration** separately.

All profiles populate the same editable Linear registration controls; no AFNI/FSL/ANTs/JIP/elastix linear executable or second engine is included. Profiles map supported settings to Align and display source-specific optimizer, masking, sampling and stopping differences. They do not claim identical results to those programs. Native nonlinear methods and the compact runtime remain unchanged.

## Earlier release documentation

# Brainana Align 0.17.7

Use Image-based registration → Linear registration for every parameterized image-based linear fit. Choose the transform model and numerical settings there; Run, inspect and explicitly accept the result. Landmark and manual initial alignment remain available. After accepting a proper linear alignment, choose JIP nonlinear registration or ITK nonlinear B-spline to add deformation. Neither nonlinear option reruns a linear fit.

The bundled native ITK adapter replaces the large SimpleITK distribution. The app remains self-contained; a thin private Python/NumPy adapter is included for the existing JIP and NIfTI bridge. There is no standalone JIP GUI or separate ITK linear workflow. See BUILD, ARCHITECTURE and VALIDATION for reproducibility and limits.

## Earlier release documentation

# 0.17.6 · foreground-aware linear fitting for prepared images

In Image-based registration → Linear registration, click **Use prepared-image settings**, then choose the transform model and **Run linear registration**. The explicit preset selects nearby staged fitting, nonzero fixed-image foreground, mean-normalized mutual information and bounded rotation. Review and accept/reject the proposal normally. Rigid/similarity/anisotropic/affine/custom remain available. The unchanged broad orientation search, all-included-voxel matching and legacy percentile histogram ranges remain the launch defaults for compatibility.

Nearby fitting starts from the original images' physical coordinates. It does not use a previous accepted transform and is distinct from landmark refinement. Translations, rigid pose, axis scales and all enabled parameters are introduced in stages at finest registration resolution; Standard sampling uses a 60,000 sample target. Rotation limits are at most 30 degrees. Use broad search or establish an initial alignment when images are far apart. Switching back to broad restores its rotation range and pyramid controls.

Foreground uses nonzero ORIGINAL working voxels in the fixed image, intersected with existing physical regions and exclusions. It is intended for a skull-stripped/zero-background reference; valid zero-valued tissues can be excluded, so choose the policy deliberately. Sparse coarse foreground sampling uses finer samples without dropping any constraints. Mean-normalized common-range histograms apply only to MI/NMI. These are scoring operations, not N4 correction, image masking or intensity editing; source/working arrays, display and export intensities are unchanged. Session/export provenance records the selected policies; old sessions remain readable.

The provided anatomy/EPI pair is checked through real packaged loading, preparation and workers with 6- and 12-parameter models. Evidence distinguishes qualitative overlay improvement from ground-truth accuracy; the implementation is not claimed to reproduce native JIP numerics exactly. Optional JIP nonlinear and ITK routes and their private numerical runtime are unchanged (worker version 0.17.5). This candidate contains one self-contained Apple Silicon Align.app, complete source, consolidated documentation, validation, manifests and SHA-256 checksums. Native graphical/GPU hardware checks and notarization remain pending.

## Earlier release notes

# Brainana Align 0.17.5 · incorporated nonlinear registration

**Install one application:** Brainana Align.app includes its private registration worker. No separate JIP application, app selector, Python installation or sibling app is required. Apple Silicon, macOS 14 or later. Standalone JIP remains an independent project and is not included in this release.

**Linear first, nonlinear second:** use Align's existing landmark alignment, manual adjustment, or image-based Linear registration and accept the result. Then open **JIP nonlinear registration** to add nonlinear deformation to that exact accepted alignment. There is no second JIP linear model or linear search. Review and accept/reject the nonlinear proposal. ITK registration remains a separately labelled CPU alternative using SimpleITK, not elastix. Physical windows, exclusion masks, full deformation sessions and native-resolution export retain their established contracts.

CPU compatibility, parallel CPU and optional Metal/CUDA requests remain available for JIP; actual availability/fallback is reported. This release does not claim native graphical/GPU validation in this agent environment.

## Historical release notes (superseded where deployment differs)

# Brainana Align 0.17.4 / JIP Registration 0.8.4

Image-based registration now groups its three methods into peer expandable sections: **Linear registration**, **JIP registration**, and **ITK registration**. Shared optimization regions and acquisition details appear before method selection. Each section starts closed and can be opened or closed independently.

Use **Linear registration** to estimate one global transform from image intensities. Its **Transform model** dropdown still selects rigid6, similarity7, anisotropic9, affine12 or custom groups. The run button now says **Run linear registration**. JIP and ITK each offer their own linear/nonlinear stages; ITK is SimpleITK, not elastix. Their methods, defaults, constraints and proposal acceptance are unchanged.

The guide teaches opening the relevant method heading before pointing to a field. Show Linear/JIP/ITK controls locates the real main-program controls; these guide actions never execute registration. Opening a method runs nothing and does not change parameters, data, regions or accepted alignment. All prior0.17.3 scientific features remain available below.


## Previous release documentation

# Brainana Align 0.17.3 / JIP Registration 0.8.4

This Apple Silicon candidate adds optional landmark refit sensitivity, portable displacement exports, background overlay resampling, reproducible image metrics/initialization, acquisition metadata, and an independent ITK registration option. Existing registration defaults and JIP's original approach remain unchanged.

**Landmark checks.** After accepting a landmark fit, expand Landmark refit sensitivity in its diagnostics and request the calculation. Minimum complete enabled pairs are6 for rigid/similarity,8 for anisotropic/affine, and20 for spline. These are conservative application gates, not statistical guarantees; every retained subset must also fit stably. The report refits the original landmark model after omitting each pair. It measures refit sensitivity, not target accuracy. Manual/image-based refinement is excluded.

**Image-based registration.** Existing hard-binned metrics and Euler initialization remain defaults. Additional settings offer Smooth MI (Parzen), deterministic jittered sampling and full-orientation quaternion starts. Distributed starts require a full±180° rotation range. Reports record usable samples and overlap; these counts do not establish anatomical correctness.

**Acquisition information.** Import a BIDS MRI JSON sidecar or explicitly enter the readout and phase-encoding axis under Image-based registration. T1/T2 describe contrast, not acquisition geometry. Similar contrasts can favor correlation; mixed contrasts can favor mutual information. EPI-specific displacement constraints require verified EPI readout and a native voxel phase axis. Use suggested settings and Restrict JIP displacement are separate explicit actions, never automatic runs. Readout time is recorded only; this does not estimate a field map or implement TOPUP/BBR. ITK B-splines have no phase-axis constraint.

**Optional ITK.** Keep JIP Registration 0.8.4 beside Align, or select it with Choose registration companion. Expand ITK image-based registration. This is SimpleITK/ITK, not elastix. Choose rigid6, similarity7 or affine12; linear only, linear plus nonlinear B-spline, or nonlinear from an accepted linear alignment. Choose Mattes MI for mixed contrasts or local neighborhood correlation for similar contrasts. The isolated CPU worker produces a proposal for inspection and explicit Accept/Reject. JIP remains available separately, with its existing optional Metal/CUDA path.

ITK working images must have8+ voxels per axis and at most 2 million voxels each. Orthogonal oblique/anisotropic grids are supported; sheared grids and continuous moving-image windows are refused. Fixed optimization windows require Physical pyramid: Working grid and Metric sample fraction:1, preserving exact physical boundaries. Exclusion masks can be used at any pyramid. Positive sampled Jacobians are checked but do not prove continuous topology or anatomical validity. Start a different backend only from a linear alignment.

**Review and export.** Settled affine/nonlinear overlays run in cancellable background workers; unchanged fixed textures are reused. Cached manual previews remain transient until the original-source rebuild. Export uses the complete accepted mapping, preserves ITK/JIP provenance and acquisition metadata, and writes canonical field arrays. Displacement NIfTI uses intent 1006, RAS-mm components and explicit grid/direction metadata. Tested ITK readers automatically convert geometry and vectors to LPS; do not flip them again. Schema13 sessions preserve acquisition details and older sessions remain readable.

See VALIDATION.md for evidence and limitations; BUILD.md for reproducible source builds. Historical release sections below describe earlier candidates.


## Historical documentation

## 0.17.2 — registration regions and interactive manual preview

Exact interpolation memoization also reuses common width1/2 Lanczos coefficients, keyed by the complete float coordinate and width. There is no quantization or approximate kernel table. Each context has a bounded256-entry cache per axis. Reuse is restricted to the serial CPU path on frozen displacement axes during local nonlinear probes; full-grid passes, changing axes and smoothing/large kernels retain the original calculation.

Includes JIP Registration 0.8.3 for Apple Silicon. Keep the two apps together. The JIP route now honors the shared optimization regions and registration exclusion masks; it requires a companion advertising sampling-v1 when either is active. Older companions remain usable for unconstrained runs. No constraints are silently cleared.

In **Image-based registration**, define the optional regions above the JIP controls. Each rectangle restricts only its two physical RAS axes, regardless of slice depth. The fixed window gates fixed sample centers. Moving windows follow the current continuous pull coordinates; moving exclusions use the nearest working voxel. Existing nonzero-fixed matching, interpolation taps, basis hierarchy, regularization and sequential optimization remain unchanged. Regions constrain the objective, not the spatial extent of the fitted deformation. They are not an output crop. At least 64 usable, varying fixed samples are required.

Manual nudges now move a cached display image immediately, coalesced once per display frame. After a 450 ms pause, the complete overlay is rebuilt from the original working image with the complete mapping. The temporary preview can show the cached field of view and interpolation; the label identifies it until full detail is ready. Only a compatible change to the displayed image uses this path; target/base changes fall back to a complete rebuild. Export never uses preview pixels. MRI/CT target selection, nonlinear direction, processing, native files and physical windows remain independent.

JIP caches exact centered coordinates for bounded diagonal internal grids, retaining the original conversion for general or oversized grids. This is an engineering optimization, not a new registration algorithm. Metal/CUDA options and CPU fallback remain. Native graphics and GPU speed still require checking in a normal desktop session.


## 0.17.1 — registration efficiency

This Apple Silicon update includes JIP Registration 0.8.2. Keep the two apps together. The registration model, default CPU route and explicit proposal decisions are preserved. Align receives the exact native field without generating an unused aligned image, checkpoint or Jacobian image. Fields stay binary in memory and are shared across manual/review snapshots; saved sessions and transform metadata retain the portable base64 representation and schema12 compatibility.

JIP reuses bounded acceleration scratch, batches more independent samples per dispatch, avoids heap allocation for common serial interpolation kernels, and tests automatic GPU performance on warm batches. Execution metadata now includes phase timings and bounded workspace counters. Numerical compatibility and hardware speed are separate checks; source-level Metal/CUDA checks do not demonstrate GPU performance on a user's device.

See TECHNICAL_FINDINGS.md#registration-efficiency-0171 for scope, measurements and the remaining candidate work. Native GUI/GPU and other-platform qualification retain their explicit limits.

## 0.17.0 — JIP image-based registration and bounded fitting memory

The Apple Silicon candidate includes the optional self-contained JIP Registration 0.8.1 companion. Keep both apps together, then open **Image-based registration → JIP image-based registration (advanced)**. Linear and nonlinear JIP stages use the established method, CPU/GPU execution settings and explicit proposal decisions. Sessions, manual adjustments and native exports retain the full deformation.

See [setup and current limits](#jip-registration-in-align-0170), [validation](VALIDATION.md) and [changes](CHANGELOG.md). Existing optimization-window and exclusion-mask registration routes remain available; JIP 0.8.1–0.8.2 require full working images; 0.8.3 supports the shared regions and exclusions. Other operating-system updates follow this Apple Silicon candidate.

## 0.16.84 — Accuracy, performance and architecture audit

Removed the welcome eyebrow and large slogan as requested. Registration preparation now rejects malformed custom pyramid settings and avoids zero-weight nonfinite contamination. Sparse included masks no longer disappear from intensity-range estimation solely because of sample stride. Landmark controls retain their DOM identity during unrelated redraws. Preparation and nonlinear one-shot workers share explicit cleanup/error ownership.

Apple Silicon review release. Read [the assessment and remaining priorities](TECHNICAL_FINDINGS.md), [Viewer comparison](FEATURE_PARITY.md), [architecture changes](ARCHITECTURE.md) and [executed validation/limits](VALIDATION.md). Full source, release manifest and SHA-256 checksums accompany the app.

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

The guided setup now teaches the main interface. Method choices select instructions, not application actions. Follow prompts to close the current sidebar panel, open the indicated panel (including nested advanced sections), then click the actual highlighted field or button in the app. Show controls locate actions without performing them. Set points, run alignment, accept/reject proposals, save and reset using their main controls. Closing/opening panels preserves data. The guide advances when those real actions complete.

## 0.16.75 — In-app network browsing

Choose Network when loading an image to open the in-app SSH/SFTP browser. Enter a host/user (or choose a concrete SSH-config profile), authenticate, browse folders and load an image or paired-format files. The other image, landmarks and registration remain in the same workspace. Reopening Network reuses its connection; Change connection returns to the connection form. Local loading includes mounted network shares. Passwords/passphrases are cleared after each connection attempt and are not saved.

## 0.16.74 — Local and network loading

Select either volume to choose Local computer / mounted network drive or Network workstation (SSH/SFTP). Mounted shares use the normal file picker. In a connected workstation window, Network browses that workstation and Local still chooses files from your computer. New SSH/SFTP connections are available in the packaged macOS app. Changing connections opens a fresh workspace; save a session first and use the explicit confirmation. Cancelled or failed connections keep the current workspace.

# Brainana Align desktop

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

The default packaged application is now a standalone Electron program. It bundles Chromium and opens Align in its own window, with native file dialogs, export save dialogs, a Dock identity, standard editing shortcuts, File → Save session, and Help → Guided setup. Registration, display and session formats use the same scientific implementation as the previous browser release.

**Workstation access:** File → Connect to workstation… preserves saved profiles and the existing interactive system-SSH flow. Authentication may open Terminal; the connected workspace opens inside Align. File → Return to local files… changes back. Save a reloadable session before changing connections. The application owns its local server and adopted remote connection and shuts them down when it quits.

Separate **arm64** (Apple Silicon) and **x64** (Intel) desktop builds support macOS 13.5 or later. The optional **browser-universal** package retains system-browser support independently of the desktop app (`npm run start:web` also works from source). Windows x64 is available as an installer or portable ZIP. Extract the complete portable ZIP and open **Brainana Align.exe**. The Windows edition supports local MRI/CT workflows; the macOS interactive SSH/workstation launcher is unavailable there. Ubuntu x64 is available as a `.deb` installer or portable archive. Prefer the `.deb` for desktop integration and sandbox setup. Linux uses local MRI/CT workflows; the macOS interactive SSH/workstation adapter is not included. Landmark refinement and independent image-based registration have separate workflow tabs.


Brainana Align aligns MRI and CT volumes using physical landmarks, overlay review, manual adjustments, and automatic image registration and local refinement. The macOS application is self-contained for Apple Silicon and Intel Macs running macOS 13.5 or later. Node, npm, Python, Homebrew, Conda, and a remote Node server are not required to run it.

## Viewer-style layout

The interface follows Viewer's Graphite + Amber palette and typography. The left rail orders Initial processing, Landmarks, Landmark alignment, Landmark refinement, Manual adjustment, Image-based registration and Alignment display. Each section opens independently. The toolbar separates Volumes, Session, Landmarks, Workspace and Review & export. A bottom status strip keeps messages visible without crowding the toolbar. All nine image panes remain arranged as MRI, CT and overlay rows across sagittal, coronal and axial columns.

The imaging background remains black. Landmark, optimization-window and green–magenta overlay colors keep their existing diagnostic meaning. Guided setup highlights the relocated controls and uses the same warm theme. Source Sans 3 and IBM Plex Mono fonts ship locally with their SIL Open Font License and copyright notices; no font service is required.

## Guided setup

Select **Guided setup** in the toolbar for interactive, nonmodal help anchored to the relevant controls. The guide follows actual application state and keeps the tool usable while a prompt is open.

- Select MRI, then CT. Successful header selections advance to joint preparation; a canceled picker or loading error does not advance. Already-loaded images are recognized.
- Choose landmarks, automatic registration, manual adjustment, or an existing alignment. You can skip landmarks and move to the other options.
- The landmark branch guides MRI and CT placement for each pair, highlights the corresponding Set button, counts complete enabled pairs, and adapts the minimum to the selected model. The guide waits for a successful fit and preserves fitting errors for review.
- Landmark refinement requires an accepted landmark-derived alignment. **Start from image geometry** in Manual explicitly initializes the identity transform in the images’ physical coordinates. It does not estimate a global registration. Review overlap, use manual adjustments or landmarks when necessary, then optionally define windows and refine a landmark-derived result.
- Pending refinement proposals must be accepted or rejected. The guide then returns to review, with the applicable follow-up, manual, independent registration and saving choices. Image-geometry initialization is recorded in sessions/metadata; landmark RMS is unavailable until there are fitted landmark correspondences.
- Drag the guide header to move the popup; a safe manual position stays for the current step, and the guide repositions for the next target. Focus the header and use arrow keys (Shift for larger steps) for keyboard movement. Follow highlighted control restores automatic positioning.
- ×, Escape, or the toolbar button hides the guide while preserving its route and Back history. Resume guide in the toolbar or status strip continues using current application state, including work done while hidden. Restart guide explicitly restarts the walkthrough. Finish guide ends the walkthrough in one action. The guide opens automatically only if you explicitly enable Open guided setup on launch in Settings.
- Add landmark pair creates the next correspondence. Add landmark pair always stays first and Apply landmark transformation always stays second; Apply is disabled until the selected model has enough complete, enabled pairs and the current pair is finished. It becomes the primary action when ready. You can add more pairs first; errors or declined reflection confirmation keep you on the fitting step. Other registration methods remain a secondary alternative in the same third position.

The guide offers advice and explicit actions; it does not select landmarks, run registration, accept proposals, or export data on the user's behalf. Browser/WebGL errors take precedence over image-loading instructions.

## Landmark workflow

1. Select MRI and CT, review joint working resolution/memory estimates, then Prepare workspace.
2. Add corresponding landmarks. Spread them across the anatomy and all three dimensions.
3. Choose the transform direction and **Landmark transform** model.
4. Select **Run landmark alignment** and review the overlay, RMS error, determinant, scales, shear, reflection status, and warnings.
5. Adjust manually or retry with another model. Image refinement adds rigid nudges to the current transform while preserving its scale/shear.
6. Accept or reject any refinement proposal before exporting. Save the session to preserve landmarks and registration state.

| Landmark transform | Parameters | Minimum geometry | Behavior |
|---|---:|---|---|
| Rigid (default) | 6 | 3 noncollinear pairs | Translation and proper rotation |
| Similarity | 7 | 3 noncollinear pairs | Rigid plus one positive uniform scale |
| Anisotropic scale | 9 | 4 pairs spanning 3D | Proper rotation plus positive independent scales; no shear |
| Affine | 12 | 4 pairs spanning 3D | Full linear matrix and translation; permits shear and reflection |

Every enabled complete pair contributes to the fit. Minimum counts alone do not guarantee stable geometry. Coincident, nearly collinear, and insufficiently three-dimensional configurations are rejected where the selected model cannot be identified. More than the minimum is useful for assessing residual error, particularly for affine fits where four pairs can fit exactly.

For the nine-parameter model, `A = R diag(sx, sy, sz)`: scales act along the **moving image's physical RAS X/Y/Z axes before rotation**, not along voxel indices. Reversing a general anisotropic matrix can mix those axes; its exact inverse is exported, but fitting the reverse direction with the same constrained model is a different optimization. Use affine when you need unrestricted geometry in either direction.

A negative determinant means mirrored anatomy. Affine reflection requires explicit confirmation before application, session restoration, and each registration/image export. Check left–right labels and coordinate conventions. A warning may identify a much better mirrored affine candidate without applying it.

Scale diagnostics are lengths of the transformed physical unit axes. Shear magnitude is the maximum absolute cosine between the transformed axes: zero means orthogonal. These diagnostics do not replace visual anatomical review. Scale outside 0.25–4, shear above 0.15, determinant magnitude below 1e-6, and poor geometry/conditioning generate warnings.

## Coordinates, sessions, and exports

Landmarks and transforms use NIfTI RAS physical millimeters. Rendering and fitting share Niivue's selected qform/sform affine. Known meter and micron units are normalized to millimeters before display. Unknown NIfTI spatial units retain the established millimeter assumption; verify those datasets independently. Niivue selects the form using valid geometry and transform-code priority; this release does not introduce a separate qform/sform selector.

Forward matrices map moving world coordinates to fixed world coordinates; inverse files map fixed to moving. Text matrices are **RAS world transforms**, not FSL FLIRT scaled-voxel or ITK LPS matrices. Exports preserve the fixed voxel grid through an explicit sform and include model-specific names. Image export currently resamples the first volume of a multi-volume input using linear interpolation.

Session schema 6 includes intensity-mask processing recipes plus model, parameter count, full matrices, image geometry, landmarks, residual, diagnostics, reflection confirmation, warnings, manual adjustments, windows, and pending refinement proposals. Older rigid sessions remain supported. Invalid matrices/coordinates are rejected; mismatched or missing image geometry permits landmark-only restoration after confirmation. Registration JSON records fixed/moving spaces and coordinate conventions explicitly.

## Documentation and release files

- `BUILD.md`: reproducible development, validation, and assembly instructions.
- `CHANGELOG.md`: running release history.
- `ARCHITECTURE.md`: modules and platform boundaries.
- `FEATURE_PARITY.md`: supported features.
- `VALIDATION.md`: executed checks and uncompleted native/browser certification.
- `TECHNICAL_FINDINGS.md`: general audit, changes, and future improvements.

`VERSION.json`, `RELEASE_CONTENTS.txt`, and `SHA256SUMS.txt` identify each package. The full bundle includes the macOS package, complete shared-source package, validation evidence, and checksums. Read the validation limits before treating this as a fully certified release.

## Workflow focus and responsiveness

Use Landmarks, Landmark refinement, Manual or Image-based above the control rail to focus that workflow; the other registration panels collapse without changing images, landmarks or transforms. Direction stays visible above the panels. Image-based exposes independent global registration. Landmark refinement exposes nonlinear paired-landmark spline fitting and constrained rigid refinement. Manual exposes the existing-coordinate initializer beside nudges. Image geometry does not enable landmark follow-up refinement.

A progress strip shows fitting, overlap preparation, search levels and transformed-overlay preparation. Controls pause during computation to keep the input state stable. Image-registration preparation and optimization execute in a cancellable worker. Progress names stages and candidate/pass counts rather than estimating wall-clock completion. Final overlay resampling remains synchronous.

Moving-image opacity updates the single overlay’s shader alpha and redraws once per animation frame. It does not reset colormaps, rebuild textures or resample images during slider gestures. Changing display mode or rebuilding an aligned overlay still requires texture updates.

The fast opacity path automatically falls back to normal texture updates if the review shader cannot be compiled. This preserves display behavior on unsupported graphics configurations; native latency has not been benchmarked in this environment.

## Automatic registration and refinement

Load MRI and CT, select Image-based, and check direction. Independent registration needs no landmarks and searches for a new starting alignment; it does not refine the current landmark result. Rigid (6 parameters) is the default. Similarity (7), independent-axis scale (9), affine (12), and custom scale/shear groups are available. NMI is the default for MRI/CT; MI, correlation ratio, normalized correlation, and least squares are alternatives. Correlation and least squares are primarily useful for images with comparable intensity relationships.

The search settings expose angular range, translation/scale/shear bounds, histogram bins, and input-relative pyramid levels. Full orientation uses broad seeds and bounded multi-candidate optimization; it does not guarantee a global optimum. Optimization windows constrain fixed and moving physical spaces. Progress names the current stage; Cancel registration terminates the worker and preserves the prior alignment. Preparation and optimization use a worker; final overlay resampling still occurs on the UI thread.

Review the preview, diagnostics, and score before accepting. Reject restores the previous accepted transform, or returns to an unfitted state when none existed. Manual adjustments and exports require a decision first. Sessions preserve pending proposals and their prior alignment. Export metadata records operation, model/group count, metric, search settings, resolution schedule, scores on the same final-resolution samples, and warnings. Landmark RMS is unavailable when no correspondence pairs exist. Local adjustments compose with the accepted matrix, so an earlier affine transform remains affine even when its adjustment is rigid.

The older bounded rigid NMI implementation remains in source for compatibility, while the visible operation selector uses the worker solver. Existing-coordinate initialization remains an optional alternative.

## Constrained refinement

After accepting a landmark fit, open Landmark refinement. Define the optimization region there first (optional), then run constrained rigid image-based refinement of that landmark-derived alignment. Tight uses ±2 mm and ±1°, with 1 mm / 0.5° start steps; Standard uses ±5 mm and ±10°, with 2 mm / 1° start steps. Edit limits or Advanced start steps for Custom. The worker searches four halved-step levels, up to two passes each, using initial geometric overlap and using the original valid-sample threshold (25%, minimum 64, capped at 500). The refinement adjustment is rigid and preserves the base transform’s existing scale, shear or nonlinear warp. Additional search settings expose metric histogram bins; original 40-bin NMI is selected for local refinement. Review and accept/reject the proposed overlay. Automatic registration uses the separate broader orientation and resolution pyramid settings.

## Settings and personal configurations

Open **Brainana Align → Settings…**, press **⌘,** (Ctrl+, outside macOS), or select **Settings** in the toolbar. Changes save automatically. The desktop stores settings in Electron's user-data directory independently of the local-server port; workstation connections use the same preferences. Browser mode uses browser local storage for its current origin.

- **Appearance:** choose Graphite + Amber, Neutral dark, Light, or High contrast; edit individual colors for Custom. Contrast diagnostics cover panel/secondary text and accent-button text. Adjust text size or enable larger targets. Interface palettes leave image-overlay colors unchanged. Save up to 20 named complete configurations; load or delete them later.
- **Display:** customize fixed/moving overlay tints, default opacity/mode, MRI/CT crosshairs, crosshair thickness/visibility, marker size and labels. The normal display-mode/opacity/navigation controls also remember their choices. Loaded image backgrounds remain black. Fixed/moving colors follow the registration direction.
- **Interaction:** linked navigation and reverse/sensitivity controls for Shift+scroll slice stepping. Ordinary scroll still zooms. Existing keyboard shortcuts are listed; custom key remapping is not included.
- **Guide:** dock instructions in a dedicated column so they do not cover anatomy or point controls, open the guide on launch, or remember progress/position. Hiding it retains Resume guide. Docking reduces the space available to image panes; floating mode remains available.
- **Registration:** choose future model/refinement defaults. These apply on the next launch or through **Apply defaults to visible controls**. Preferences do not change accepted matrices, loaded sessions, pending proposals or a running search. Actual runs use the visible controls and retain their settings in session/export provenance.
- **Saving:** choose recovery autosave interval (10–300 seconds), remember workflow/panels, inspect recent dataset names, choose the default native export folder, or restore/export/clear recovery. Desktop window dimensions and the most recently used native save folder are remembered. Browser downloads follow browser settings; a server-selected export folder can still be chosen in Export.

Recovery contains session state (landmarks, physical image geometry, windows, accepted transform and pending proposals), **not image voxel arrays**. Reload the same MRI and CT before restoring through Settings → Saving; existing geometry/reflection checks still apply. The previous launch snapshot is retained when current-work autosaves begin. Recent entries are names rather than automatic file reopen links. Recovery is not a substitute for saved sessions and source datasets.

**Export settings / Import settings** transfer configurations as versioned JSON. **Reset section** resets that section's preference values. **Restore all defaults** resets preferences, saved guide/layout and export destination, with default window size on next launch; named configurations and recovery snapshots remain available. A failed storage write is reported and preserves the previously saved preferences.

Optimization windows constrain only the two visible physical axes of each drawn rectangle, not its current slice depth. Multiple windows intersect these constraints; missing planes leave their axes unrestricted except for other drawn windows. Both tabs show region editors that operate on the same stored MRI/CT windows. The guide follows independent registration or landmark follow-up explicitly. Small or disjoint intersections can still legitimately produce too few samples.

On a fresh empty launch, Guided setup starts with loading MRI and CT, then offers landmarks, automatic registration or manual setup. Remembered guide progress does not restore image/session data. Hiding and resuming within the current session preserves the current step.

Reset workspace (toolbar or guide) unloads MRI/CT and clears landmarks, alignment, windows, undo, guide progress, recovery snapshots and recent dataset names after confirmation. Save your session first if needed. Original files, exported sessions/results and personal settings remain. Restart guide retains loaded work; Reset view only resets viewing position.

Optimization-window drawing now keeps the last valid rectangle on cancellation or an invalid replacement. Drag at least 4 screen pixels in each direction. Leaving a panel clamps to its edge; zoom/pan clips the displayed rectangle without deleting its physical bounds. Clear windows explicitly removes saved constraints.

After refinement, the guide offers manual adjustment, a new automatic registration search, further refinement, landmark retry or saving. Review each proposed alignment before accepting.

## Nonlinear landmark spline (advanced)

Choose **Nonlinear spline (advanced)** under Landmark transform. Add 12–128 complete enabled pairs spread through the volume; 20+ is recommended. These are conservative application choices, not a literature guarantee of accuracy. Rigid remains the default. A dense cluster or a single plane is unsuitable, and incorrect point matches can reshape unsampled anatomy. Use independent validation points and inspect all slices; lower training RMS alone is not evidence of correct alignment.

Default smoothing is 0.01 in normalized coordinates; 0 requests interpolation. A proper affine base handles broad pose/scale, then a 3D polyharmonic/thin-plate residual models deformation. The implementation uses the 3D distance kernel, not the 2D r² log r formula. It normalizes coordinates and solves all corresponding pairs with pivoted, reorthogonalized QR. Duplicate, nearly clustered, degenerate, unstable and reflected configurations are rejected. Extreme affine geometry produces warnings.

A conservative global residual-gradient bound of 0.65 keeps G(q)=q+u(q) contractive for inverse iteration. If the unconstrained fit exceeds this bound, all residual coefficients are attenuated and a warning reports the fraction retained. This may leave significant landmark residuals; improve distribution, matches or smoothing rather than force an exact result. Diagnostics include pair residuals, RMS, sampled finite-difference Jacobian estimates and a positive analytical lower bound away from control knots. The r kernel has nondifferentiable control knots; the global Lipschitz condition still makes the residual mapping one-to-one. This mathematical property does not establish anatomical accuracy.

The proposal must be accepted or rejected. Manual nudges and constrained image refinement compose an outer linear adjustment after the full spline. Image refinement keeps spline coefficients fixed; nonlinear intensity optimization is not implemented. A new global registration search proposes a linear alternative and can be rejected to restore the previous spline.

Sessions use schema 4 and retain affine base, normalized control locations, weights, polynomial tail, smoothing, safety bound, attenuation, direction and landmarks. Restore validates coefficient safety and correspondence with saved physical controls, then recomputes RMS. Older linear sessions remain supported; older apps cannot load schema-4 sessions. Export nonlinear transforms as JSON, never as misleading 4×4 TXT matrices. The affine component alone is not the registration.

Aligned NIfTI images use native reference-grid pull resampling through the same fitted inverse (frame 0, matching the existing export workflow). Forward/inverse vector fields use RAS millimeters, NIfTI vector intent 1007 and components in dim5. Forward fields sample F(x)-x on the moving grid; inverse fields sample F^-1(y)-y on the fixed grid. Field spacing defaults to 2 mm; extent-preserving oblique grids and a 256 MiB vector limit avoid oversized fine-CT exports. Headers and JSON record sampling. Fields interpolated by other software approximate the continuous warp and do not inherit its analytical guarantee automatically. LPS-based tools require explicit coordinate/vector conversion. The JSON retains the full continuous model.

Fitting, image resampling and field exports run in cancellable workers. Nonlinear operations can take substantially longer than linear ones, particularly with many controls or large native grids. Loaded voxel buffers are not detached. Failures/cancellation discard incomplete proposals or previews; accepted source state remains recoverable.

Research basis: [ITK 3D ThinPlateSplineKernelTransform](https://docs.itk.org/projects/doxygen/en/stable/classitk_1_1ThinPlateSplineKernelTransform.html), [SciPy RBFInterpolator regularization and rank requirements](https://docs.scipy.org/doc/scipy/reference/generated/scipy.interpolate.RBFInterpolator.html), [ITK registration guide: landmark kernel transforms](https://itk.org/ITKSoftwareGuide/html/Book2/ITKSoftwareGuide-Book2ch3.html). B-spline lattices are a useful alternative for dense image-based optimization; a landmark spline fits scattered points directly without selecting a large control lattice. Compact-support RBFs offer greater locality, as discussed by [Fornefett et al.](https://www.sciencedirect.com/science/article/pii/S0262885600000573). No method is universally best; this release chooses a bounded spline for the current scattered-landmark workflow.

The active guide follows direct workflow tabs and operation changes. Successful loading/point placement and proposal decisions update the instructions; finishing window editing continues to the selected registration operation. Parameter edits remain part of setup, and manual adjustments require Continue when you are ready. Hiding the guide preserves its route instead of following unrelated clicks; resume checks the current data.

The guide automatically avoids its highlighted control and the image panes needed for the current step. When crowded it reserves a docked area; narrow windows use a bottom tray. Each new target receives three brief outline pulses, followed by a steady outline. Reduced-motion settings disable pulses. You can drag a floating guide while keeping the active area clear; the next step chooses a fresh position.

Image and transform exports require an accepted alignment. If a proposal is pending, Export explains the requirement and offers Review pending alignment; session saving can preserve the preview. Export progress and failures appear inside the dialog. A failed multi-file export stops rather than claiming success; retry after correcting the destination problem.

## Landmark refinement and shared regions

Use Landmarks for the initial 6/7/9/12-parameter paired-point fit. Landmark refinement offers an optional nonlinear landmark spline (12 distributed pairs minimum, 20+ recommended) and constrained rigid image-based refinement. The spline uses paired landmarks, not intensities or optimization regions. Review its warnings and accept/reject its proposal before refining. Manual adjustments remain available across workflows. Older spline sessions retain their full nonlinear transform; the initial landmark dropdown offers only linear models.

Each registration workflow provides Define/edit and Clear region controls above its intensity settings, with the same active-constraint summary and modality selector. Changes in either place immediately update the other. Regions are shared; they do not change the displayed image data or the landmark fit. No regions means the available image support/overlap is used. Switching workflows and resuming the guide do not erase regions or customized bins. Each operation remembers its own bins during the current workspace session.

## Working resolution and memory guidance

Select both input files before allocating workspace images. For scalar single-file NIfTI-1/2 (.nii/.nii.gz), bounded metadata inspection reports native X/Y/Z spacing in mm, dimensions, frames, file size and decoded frame size. Recommended considers both physical extents, native spacing and memory capacity; Native/2×/4×/8× spacing presets or custom X/Y/Z fields on one row remain available. Requested spacing finer than native keeps the native sampling; actual spacing may be slightly coarser to preserve endpoint extent. Choose Volume / time point when the file contains multiple 3D volumes; ordinary 3D files hide this control.

The desktop reads installed RAM and a conservative OS available-memory estimate via a restricted bridge, refreshes during preparation planning and checks again before Prepare/export. The optional GiB budget limits recommendations. Ratings are Comfortable/Potentially slow/Above planning allowance/Exceeds supported working size; unavailable capacity is identified rather than assumed safe. Estimates account for CPU storage, worker copies, current separate GPU contexts, active pyramid/preview buffers, headroom, and resident downloaded blobs. Timing and anatomical accuracy are not guaranteed by spacing or these ratings. Recommendations retain roughly 96+ samples across the largest physical extent rather than silently reducing anatomy to a few voxels to satisfy a very low budget; a comfortable setting may be unavailable.

Preparation streams the selected frame through a normalized separable triangular low-pass filter and decimation. It does not allocate the full native voxel volume first. Origin, axis directions and voxel-center endpoint extents are preserved in physical coordinates. Other supported formats use the existing decoder only after Prepare, at native resolution; they are labelled as fallback because header-only planning, per-axis streaming resampling and selected-frame selection are not yet implemented for those decoders. Their true peak may exceed file size substantially.

Use Resolution to change sampling later. Resolution-only changes keep landmarks, optimization regions, accepted matrices and manual adjustments in physical millimeters. New datasets or a different frame invalidate the fit and changed-image regions. Both working images commit together; cancellation/failure leaves the old workspace active. The guide waits for preparation and is temporarily visually hidden while the preparation dialog supplies its own instructions.

Both intensity workflows have Finest registration spacing (0 = current working images) and a metric sample budget. These can reduce registration work further without changing the displayed workspace. The global pyramid ends at the working/selected registration grid, not necessarily native. Viewer arrays are shared read-only across independent pane metadata, and only one pyramid scorer is retained. GPU contexts/textures remain separate.

Aligned-image export reads the native moving frame and resamples it onto the native fixed grid in slabs with incremental gzip, using the full affine or spline inverse mapping. Fixed native voxels are not loaded. Export never enlarges an already filtered image as a substitute for native data. Compressed output remains a Blob and destination adapters can allocate additional buffers; this is not constant-memory end-to-end export. A native moving Float32 array above 1.5 GiB requires exporting the transform for external resampling. Working arrays also have a 1.5 GiB allocation limit and conservative maximum axis length of 2048 for viewer preparation. Original input files remain unchanged.

Sessions record native geometry and selected frame separately from working spacing/dimensions. Reload the same native files/frame before restoring the fit; working resolution can differ because transforms and regions are physical. Registration JSON also records preparation geometry and intensity-resolution/sample settings. Native slice-on-demand inspection, shared GPU rendering, persistent worker caches and fully streamed file-destination writing remain future improvements; Native preset is currently the way to inspect all native detail in the workspace.

## Landmark guide synchronization — 0.16.50-guide.1

Landmark Set targets may not exist until the scheduled render completes. Never cache Guided setup as their fallback. Resolve the actual selector before the refresh fast path, observe landmark-list child changes and protect the whole active landmark row plus its required image panes from guide overlap. Recreated nodes reacquire the outline without restarting the same-step pulse. Track displayed MRI/CT steps in guide history; Back revisits instructions and preserves coordinates. Completed historical points offer Replace/review and Continue with current progress. Pending points offer Set MRI/CT through the existing app action; Add/Apply remain on completed-pair screens. DOM regression reproduces delayed and rebuilt targets plus two completed pairs, adding a third, Back to previous CT/MRI, preserved data and resume. Native GUI verification remains pending.

## Independent sidebar sections — 0.16.51-sidebar.1

The six top-level sections are ordered Landmarks, Landmark alignment, Landmark refinement, Manual adjustment, Image-based registration and Alignment display. Native disclosure heading clicks affect that section alone. Opening a section activates its controls; closing a section does not select another operation or change sibling open states. Explicit workflow shortcuts retain focused navigation. Both registration sections can be independently open; a named Show controls action moves the single shared controller without changing disclosure states. Guide refresh/recreated rows do not reopen a manually collapsed section; a new step or explicit Follow highlighted control may reveal its required section. DOM regressions exercise every heading in both directions, operation ownership, shared controls, guide repaint and preserved region editing/settings. Native GUI validation remains pending.

Landmark alignment starts collapsed. The Landmarks workflow shortcut opens the point list, without pre-expanding Landmark alignment; open that section explicitly or let the guide reveal it when fitting is the current step.

## Single-action guide completion — 0.16.52-guide.1

Finish guide immediately completes and closes the guide without a second End guide screen. Workspace images, landmarks and registration remain unchanged. Completion removes highlighting and Resume; × still hides/pauses. The old done route is accepted for saved-progress compatibility but uses the same single Finish guide action. The existing real DOM regression verifies one click, closed state, cleared highlighting, no Resume and unchanged workspace state. Native graphical validation remains pending.

## Neutral startup workflow — 0.16.53-startup.1

All workflow buttons start unselected and every tool section starts collapsed. Legacy saved expansion and workflow fields are ignored. Explicit user/guide navigation may open the requested section during this launch. Compiled startup tests cover legacy all-open panels and late initialization; no panel or workflow is restored.

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

Applying a crop keeps the crosshair if it remains visible; otherwise navigation moves to a retained voxel. Physical landmark/window coordinates and the accepted transform are preserved.


## JIP registration in Align 0.17.0

This Apple Silicon candidate adds the established JIP image-based registration method to **Image-based registration → JIP image-based registration (advanced)**. Standard landmark fitting, landmark refinement, automatic linear searches and paired-landmark splines keep their existing roles.

### Setup and use

Keep **Brainana Align.app** and **JIP Registration.app (0.8.1)** together in the same folder. Align discovers the companion automatically; otherwise select **Choose JIP Registration app…** once. Both applications are self-contained. The JIP companion requires macOS 14 or later; core Align requires 13.5 or later. Python, Qt and scientific libraries are supplied by the optional companion, rather than duplicated inside Align. Local and network-loaded working images use the same bridge. The browser edition cannot execute this local desktop bridge.

Prepare both images with Resolution. This initial integration limits each working image to 2 million voxels, with at least 8 voxels on each axis. Native input files remain available for the usual native-resolution exports. JIP is most relevant to anatomical/EPI images; choose the original registration methods for MRI/CT unless you have validated a suitable intensity relationship.

Use a good accepted linear starting alignment when images differ greatly in orientation. Choose linear then nonlinear, linear only, or nonlinear from the accepted linear alignment. Linear models are 6/9/12 parameters. Nonlinear MI/correlation, basis step, regularization, stiffness, relative Jacobian bound, displacement axes and execution settings are editable. Axis restrictions refer to the moving working voxel grid; orientation preparation remaps these axes explicitly. Existing nonlinear fits cannot be silently used as an affine starting point.

Review the resulting proposal, inspect Alignment display, and explicitly Accept or Reject. Reject restores the prior alignment, including a prior empty state. Manual adjustment and native image export use the full deformation. Sessions use schema12 and include the sampled field, RAS geometry, settings and execution provenance; older sessions still load. Nonlinear export contains the full mapping and displacement fields, never a misleading affine-only transform.

## Current scope and safeguards

With JIP 0.8.3, the JIP route honors active physical optimization windows and registration exclusion masks. Older companions refuse constrained runs. It never silently discards those constraints. The original searches continue to support both. Working-image intensity edits are supplied as displayed; JIP retains its established nonzero-fixed foreground convention.

The field is the native JIP sampled pull residual converted once to fixed RAS millimetres. Pull resampling preserves the native convention of zero residual outside the fixed working field. Forward point coordinates use damped Newton inversion with explicit failure, rather than a separately fitted reverse warp. Native JIP and restored sessions reject nonpositive voxel-stencil Jacobians. These are sampled diagnostics, not a continuous-domain topology proof.

GPU is optional. The result records the actual active backend and fallback reason; selecting Metal/CUDA does not prove GPU execution. Native graphical interaction and Metal require a normal desktop session; they cannot be certified by this agent environment. N4 remains available in standalone JIP; this first Align integration adds registration rather than a new N4 preprocessing recipe.

## Memory result

The original repeated-process RSS alarm was modest (about 10–23 MB over 400 synthetic tests) and also occurred when the fixtures ran without the JIP engine. Engine-owned allocations were released. This does not establish that every long-lived external embedding is leak-free, and it is separate from the historic 2 GB allocation problem.

Each production fit/export now runs in a fresh process. A 100-complete-nonlinear-job soak retained the original 16-run warmup and RSS limits: parent growth 229,376 bytes (0.219 MiB), span 229,376 bytes, slope 2,957 bytes/run. All fields matched exactly and every worker exited. Repeated benchmarks now measure each child’s sampled peak RSS rather than reporting a cumulative high-water mark across all fits. NIfTI benchmark hashing streams bounded blocks rather than materializing another full image. Align owns one job at a time and reclaims temporary files after success, failure or cancellation.

This resolves accumulation across the supported application lifecycle. No change to the underlying nonlinear objective, basis search or update method was required. Persistent library consumers should still close engines and monitor their own runtime/allocator behavior.
