# Unreleased · documentation

Reorganized the README around purpose, features, an application screenshot,
installation and first-launch help, and a practical quick start. Clarified Align's
role alongside the Brainana pipeline for routine T1/T2/EPI processing, distinguished
the registration tools, and added plain-language explanations of the JIP repairs
and what their reference tests establish. Added the supplied CT/MRI workspace
screenshot. These changes update documentation only.

# 0.18.0 · public source and native GitHub releases · 2026-10-27

Migrated the existing repository to the current standalone Align source organization and gated native GitHub builds for macOS Apple Silicon/Intel, Windows x64 and Linux x64. Retained the repository history and original BSD notice. Application/registration functionality matches the prepared0.17.10 baseline; version/bundle identity advances the older repository's separate0.17.26-workflow-ui.1 sequence. Complete source, licenses, tests, validation, manifests and checksums accompany successful releases. Native platforms are qualified by their actual CI jobs; configuration alone is not a platform pass.

Hosted-runner corrections: normalize test paths, wait for session/overlay/storage completion, launch SFTP fixtures through Node with POSIX remote paths, scope macOS launcher and POSIX permission checks correctly, and preserve golden fixture bytes on Windows. Disable obsolete FFTW Texinfo regeneration without changing its library. Windows native output uses exclusive temporary creation and atomic replacement in an isolated SDK adapter. Cross-platform float-reference tolerances are explicit; original-platform goldens and same-host rollback/restore remain exact. Manual native diagnostics retain synthetic failure evidence; all release gates remain required.

Runtime-size qualification strips Linux debug symbols and builds the same pinned Intel NumPy1.26.4 against macOS14 Accelerate, avoiding its69MB bundled OpenBLAS. Frozen-worker tests preserve uppercase Windows OS environment keys and stop the full process tree on timeout while saving synthetic failure evidence. Runtime-size reports identify the largest bundled files; the90MB worker limit remains enforced.

Added README credits and scientific references for incorporated software, with a linked JIP provenance/repair record separating original numerical corrections, the frozen repaired baseline, native-engine integration and platform packaging changes. Corrected the obsolete companion-JIP notice and documented validation scope. Package verification recognizes electron-builder's default Intel `mac/` output while still checking the executable's actual architecture.

# 0.17.10 · pure-image flip inspection · 2026-10-27

- Added Alignment display → Image comparison: Smooth opacity or Flip between images, with manual switching and optional one-second automatic flipping.
- Pure fixed/moving shader branches hide the other image completely, including empty moving background, without reloading/resampling pixels or uploading textures on supported shaders. Retained existing smooth opacity behavior and saved blend value.
- Pairing/direction-aware current/next image labels; shared slice/camera/grid state stays unchanged. Playback stops with tab/page/reset/overlay lifecycle and disabled comparison controls explain missing overlays.
- Added controller/shader/compiled-UI regressions for all three panes, timing, background isolation, returning to smooth, shader fallback and preserved image/camera state.

# 0.17.9 · bounded viewer zoom and screen-space crosshairs

Fixed a reproducible solid crosshair-color pane: the dependency's per-event wheel zoom could reach 13,743× from 100 tiny 0.01-pixel trackpad events. Align now consumes wheel input once, normalizes pixel/line/page distances, ignores zero/invalid deltas, and bounds slice zoom to 0.1–64×. Physical crosshair anchoring is retained. Orthogonal crosshairs are clipped screen-space lines with constant CSS-pixel thickness, rather than enlarged 3D cylinders. All six source and three review panes use the same instance-local adapter. Other 3D drawing remains with Niivue.

Added a regression that runs Niivue's actual wheel, slice, crosshair-geometry and line methods using a GL command recorder. It reproduces 100% sampled pane coverage before the fix, then checks nine panes, very high zoom, Retina scaling, mirrored/oblique/anisotropic/axis-swapped geometry, GL state restoration and immutable source data. Native GPU pixels remain an explicit verification limit. Registration math, defaults and the compact scientific runtime are unchanged.

## Earlier release documentation

# 0.17.8 · software settings profiles in Linear registration

Added the requested Defaults selector for AFNI 3dAllineate, FSL FLIRT, ANTs affine script, JIP 0.8.4 and the Elastix affine parameter map. Original Align and prepared-image settings are available alongside them. Added explicit preview/apply, keep-model, applied/modified status, references/differences and request-time provenance in sessions/exports. All defaults apply through shared controls. Added generic Hellinger scoring, histogram counts 16–256, adaptive/pyramid bin rules, bounded sample fractions, physical/geometric/intensity-center initialization, single-orientation initialization and editable smoothing/pass/candidate settings. Original settings and five-model worker results remain exact when new options are unused.

All profiles populate the same editable Linear registration controls; no AFNI/FSL/ANTs/JIP/elastix linear executable or second engine is included. Profiles map supported settings to Align and display source-specific optimizer, masking, sampling and stopping differences. They do not claim identical results to those programs. Native nonlinear methods and the compact runtime remain unchanged.

## Earlier release documentation

# 0.17.7 · one linear workflow and a compact native ITK backend

Removed ITK's independent linear stage, model selector and linear iteration settings. All rigid/similarity/axis-scale/affine/custom linear fitting stays in Linear registration, with shared orientation, foreground, histogram normalization, metric, sampling and search controls. Both JIP nonlinear and ITK nonlinear B-spline start only from an explicitly accepted linear base. The guide teaches that order. ITK preserves the exact accepted matrix; service/UI reject an affine-only result or changed matrix. Existing saved registrations remain readable.

Replaced the general-purpose SimpleITK Python distribution with a focused native ITK library for B-spline deformation and N4 correction. ITK is built from pinned vendored source, statically linked for macOS 14; the adapter has only system dynamic dependencies. Original JIP CPU/Metal/CUDA libraries and linear algorithms are unchanged. A thin Python/NumPy/nibabel adapter remains for safe reuse of the established JIP and NIfTI code; this release is not described as Python-free. ITK is not elastix.

Software-branded linear implementations or unverified AFNI/FSL/JIP/ANTs/elastix default presets were not added. Existing explicit prepared-image settings remain available. Similar metric names alone do not reproduce another package's full defaults or optimization behavior.

## Earlier release documentation

# 0.17.6 · foreground-aware linear fitting for prepared images

In Image-based registration → Linear registration, click **Use prepared-image settings**, then choose the transform model and **Run linear registration**. The explicit preset selects nearby staged fitting, nonzero fixed-image foreground, mean-normalized mutual information and bounded rotation. Review and accept/reject the proposal normally. Rigid/similarity/anisotropic/affine/custom remain available. The unchanged broad orientation search, all-included-voxel matching and legacy percentile histogram ranges remain the launch defaults for compatibility.

Nearby fitting starts from the original images' physical coordinates. It does not use a previous accepted transform and is distinct from landmark refinement. Translations, rigid pose, axis scales and all enabled parameters are introduced in stages at finest registration resolution; Standard sampling uses a 60,000 sample target. Rotation limits are at most 30 degrees. Use broad search or establish an initial alignment when images are far apart. Switching back to broad restores its rotation range and pyramid controls.

Foreground uses nonzero ORIGINAL working voxels in the fixed image, intersected with existing physical regions and exclusions. It is intended for a skull-stripped/zero-background reference; valid zero-valued tissues can be excluded, so choose the policy deliberately. Sparse coarse foreground sampling uses finer samples without dropping any constraints. Mean-normalized common-range histograms apply only to MI/NMI. These are scoring operations, not N4 correction, image masking or intensity editing; source/working arrays, display and export intensities are unchanged. Session/export provenance records the selected policies; old sessions remain readable.

The provided anatomy/EPI pair is checked through real packaged loading, preparation and workers with 6- and 12-parameter models. Evidence distinguishes qualitative overlay improvement from ground-truth accuracy; the implementation is not claimed to reproduce native JIP numerics exactly. Optional JIP nonlinear and ITK routes and their private numerical runtime are unchanged (worker version 0.17.5). This candidate contains one self-contained Apple Silicon Align.app, complete source, consolidated documentation, validation, manifests and SHA-256 checksums. Native graphical/GPU hardware checks and notarization remain pending.

## Earlier release notes

# 0.17.5

- Incorporated the registration source and private runtime into Align.app; removed standalone JIP app delivery, runtime selection and external app discovery.
- Corrected Align's JIP route to nonlinear only after an accepted linear alignment. Removed duplicate JIP linear stages, model and metric controls; preserved standalone JIP independently.
- Preserved original JIP CPU/Metal/CUDA numerical code and physical sampling/full-field mapping. Removed Qt, standalone GUI and unused SciPy from the private worker.
- Added source/frozen comparisons, dependency/module audits, isolated single-app IPC checks and explicit size breakdown. Incorporated scientific dependencies require macOS 14.

## Historical release notes (superseded where deployment differs)

# 0.17.4 — image-based method organization

- Place existing global registration settings inside an independently expandable Linear registration section, alongside peer JIP registration and ITK registration sections.
- Keep Transform model as a parameter label inside Linear registration; rename its execution button Run linear registration.
- Place shared acquisition information ahead of method selection, retain shared optimization regions, and explain global versus local deformation choices.
- Teach nested headings and actual method controls through the guide without proxy execution or automatic expansion.
- Preserve all scientific methods, defaults, IDs, sessions, full-map export and optional GPU behavior. Companion remains0.8.4.


## Previous release documentation

# 0.17.3 — registration review, responsiveness and optional ITK

- Add on-demand worker-based leave-one-out landmark refits above model-specific minimum counts, with unstable-subset reporting, bounded work, cancellation and explicit sensitivity/accuracy distinction.
- Serialize full JIP/ITK deformation arrays canonically in both export routes; write intent 1006 RAS displacement fields with physical grid, direction and conversion metadata. Verify with an independent actual SimpleITK reader.
- Move settled affine overlays to the shared cancellable worker path, coalesce updates and reuse unchanged review volumes; guard stale results, retain previous display on failure and expose refresh retry.
- Add an explicit shared activity/proposal/overlay state projection for guide decisions while preserving teaching through main controls.
- Add optional Parzen MI, reproducible jittered sampling, sample/overlap diagnostics and geodesically separated full-orientation quaternion initialization. Existing metrics/Euler defaults are retained.
- Add validated BIDS/manual acquisition provenance, explicit contrast-aware suggestions, and verified native-axis EPI constraints for JIP. No automatic field-map inference or T1/T2 readout assumptions.
- Add optional isolated SimpleITK CPU registration: rigid6/similarity7/affine12, Mattes MI/local correlation and B-spline proposals; separate standalone GUI dialog, session/export support, cancellation and truthful backend labels. Explicitly reject unsupported region/grid combinations.
- Write schema 13 sessions with acquisition metadata; retain older-session reading. Original native JIP numerical source and optional GPU path remain unchanged.
- Ship Apple Silicon first with full shared source, consolidated documentation, validation, manifests and SHA-256 checksums. Other platform binaries are not refreshed in this candidate.


## Historical documentation

## 0.17.2

- Support physical optimization regions and independent exclusion masks in the Align JIP route, in both directions and all JIP stages; require and confirm the companion capability.
- Preserve continuous moving-window boundaries, two-axis plane constraints, oblique/axis-swapped geometry and original intensity arrays.
- Persist native sampled masks/windows in checksummed JIPCP004 checkpoints; retain unmasked JIPCP001–003 reads and unchanged unmasked JIPCP003 writes.
- Add bounded exact centered-coordinate caching, with general/large-grid fallback and original interpolation arithmetic.
- Preview compatible manual adjustments through display geometry once per frame; resample original data after adjustment pauses. Share source preview pixels/field snapshots and update geometry without rebuilding those arrays.
- Report inadequate sampled regions without replacing the accepted map; cover masks, physical membership, backend parity, checkpoint continuation, allocation failure/cancellation and manual mapping equivalence.
- Apple Silicon first; other operating-system refreshes follow candidate qualification. CPU remains default; no new registration algorithm is enabled.


## 0.17.1

- Read exact JIP fields through the native result API; remove unused adapter image/checkpoint/Jacobian exports and reloads. Standalone exports remain available.
- Transfer bounded, checksummed binary fields through private IPC; retain older companion/base64 results and schema12 sessions.
- Share immutable field buffers across manual, review and proposal snapshots; serialize explicitly at session/export boundaries.
- Recover safely from partial scratch allocation failures; test every growth stage, retry and subsequent kernel execution.
- Reuse bounded native acceleration scratch; use larger backend-specific batches and stack scratch for common serial interpolation kernels.
- Separate GPU correctness checks from warm performance selection; retain CPU fallback and unchanged candidate ordering.
- Add native/bridge phase timings and workspace counters; read progress incrementally, preserving partial events.
- Refresh the slim workstation payload automatically after each desktop build so its assets cannot lag the packaged frontend.
- Add compatibility, malformed binary asset, snapshot ownership and warm fallback regression coverage. Apple Silicon build first; optional new registration algorithms are not included in this release.

## 0.17.0

- Optional standalone JIP 0.8.1 runtime integrated into image-based registration, with CPU/GPU choices, nonlinear preview, explicit decisions, full-map manual adjustment, native exports and schema12 sessions.
- Fresh scientific workers bound repeated-fit lifetime; streamed benchmark fingerprints avoid full-image hashing copies.
- Fixed JIP manual-state restore and controls left disabled after nonlinear overlay completion; guide/region observers clean up on page exit.
- Explicit working-grid and unsupported-region/mask guards preserve existing workflow state.
- See README.md#jip-registration-in-align-0170 for setup, validation evidence and current limits.

## 0.16.84

- Remove unused imports/helpers and a write-only guide flag; enforce TypeScript noUnusedLocals/noUnusedParameters during every build.
- Remove “BRAINANA / IMAGE REGISTRATION” and “Bring your brain images into alignment.” from welcome; retain an accessible section label and all pairing/load choices.
- Fix zero-weight NaN/Infinity propagation in registration pyramid interpolation; preserve nonfinite values that actually contribute.
- Precompute per-axis interpolation indices and remove per-voxel coordinate-array allocation and dead pyramid helpers. Synthetic finite outputs match 0.16.83 bit-for-bit; measured CPU microbenchmark improves 2.47–3.10× at tested shrink factors.
- Reject invalid/empty custom resolution tokens instead of silently deleting them; validate smoothing sigma.
- Fix stride aliasing that incorrectly reports a thin included mask as having no finite intensities. Use a bounded deterministic reservoir fallback for insufficient masked range samples; empty/constant masks still reject.
- Extract landmark-list presentation from application orchestration; unchanged refreshes preserve DOM/focus and callbacks resolve current model objects after session/Undo replacement.
- Share one-shot worker lifecycle for preparation/nonlinear tasks; handle unreadable/unknown messages, progress-handler exceptions, creation/cloning errors and cancellation, with settle-once cleanup.
- Assess latest Viewer source at pinned commit aaa47a989329f9b42c97a592ad0801b7d896216e, document intentional differences and remaining performance/architecture work. Add targeted regressions and a reproducible CPU benchmark.
- Package Apple Silicon first. Native graphical and real-anatomical validation remain pending.

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

0.16.76 — 2026-10-17. Reworked guided setup to teach native main-interface controls. Added explicit close/open/nested-panel prompts and Show actions; removed guide-triggered point placement, fit, proposal decisions, save/reset, window finish and automatic panel changes. Require main Finish processing when leaving an active tool; prevent guide-only navigation from applying voxel drafts. Retained ordinary direct-app actions, pause/resume, neutral landmark counts, placement/pulsing and per-point Back behavior.

## 0.16.75 — In-app network browsing

Replaced the extra native connection popup in the loading path with an in-app SSH/SFTP connection/file browser adapted from current Brainana Viewer. Preserve workspace while connecting/browsing; remember the connection during use; include SSH-config recall, credential clearing and strict known-host verification. Network image selection uses the same header-first resolution preparation as local files. Added real loopback SSH/SFTP and UI/backend regressions.

## 0.16.74 — Local and network loading

0.16.74 — volume source chooser for local files/mounted shares or SSH/SFTP workstations; pairing-aware titles, connected workstation browsing, clear connection-switch implications and fixed-action trusted desktop IPC. Apple Silicon review release.

## 0.16.73 — 2026-10-16

Welcome and four pairing choices; pairing-aware labels/session/export metadata; active-panel edit intensity readout; clear Paint/Erase semantics with literal post-mask intensity and explicit later mask ordering. Apple Silicon only.

Current update (0.16.73): startup now presents a welcome screen explaining preprocessing, registration, review and export, with MRI–CT, T1–T2, T1–T1 and MRI–EPI choices. The choice labels both image slots across controls, directions, guide, preparation, review and export; duplicate T1 volumes are numbered. Pairing is saved in schema11 sessions and export metadata without renaming original files or changing coordinate mathematics/settings. Image Paint uses the entered intensity, Erase writes0, and mask Paint/Erase stay1/0. New recipe6 paint overrides honor the entered value even on a previously masked image; explicit later mask application can mask those painted values again. Editing shows the cursor's working-voxel intensity at the active panel's lower right, plus mask and pending values when relevant. Hover does not move the crosshair, mutate data, upload textures or start a worker. Release platforms: Apple Silicon and Intel macOS, Windows x64 and Ubuntu amd64, plus the separate macOS browser edition. Native graphical certification remains pending.

## 0.16.72 — 2026-10-16

Explicit MRI/CT image and binary-mask edit targets, numeric radius0–16, separate binary Paint/Erase, post-hole-fill sparse corrections, and schema10/session/export replay. Apple Silicon only.

Current update (0.16.72): Voxel editing has its own explicit Edit target selector: MRI image, CT image, MRI binary mask or CT binary mask. Generated-mask targets become available after creating that mask. Image actions and binary Paint1/Erase0 are separate. Mask edits update the generated selection only, never source intensities or an already applied mask snapshot; explicitly Apply generated mask to working image to use the corrected selection on image data. Radius is a numeric integer field from0–16 working voxels;0 selects a single voxel. Click/drag previews remain sparse and worker-free; Apply/Finish commits once. Manual mask corrections run after hole filling and survive regeneration, sessions and binary/native-processed exports. Clear mask removes corrections. Recipe5/schema10 retain compact original-working-grid overrides and reject malformed/nonbinary or out-of-grid entries; older schemas remain readable. Apple Silicon macOS only; native GPU interaction remains unverified here.

## 0.16.71 — 2026-10-16

Responsive sparse voxel brush, single/3D neighborhood sizes, drag painting, pending preview, stroke Undo/Redo, explicit batch Apply/Discard, and safe application before tool/save navigation. Apple Silicon only.

Current update (0.16.71): voxel editing now collects sparse click/drag strokes with an immediate bounded orange slice preview, without starting a worker, scanning/copying a whole volume or reloading source textures per pointer event. Single voxel remains the default; 3D spherical brush radii of1/2/3/5/10 working voxels edit surrounding regions. Sphere dimensions follow the working voxel grid and may be physically anisotropic. Apply edits or Finish rebuilds the image once for the batch. Other button actions first apply the batch and replay once after success; input-file selection requires Apply/Discard first. Failed/cancelled application preserves the draft for retry. Undo/Redo operates on strokes while editing and on the applied batch afterward; Discard removes only unapplied edits. Native/base arrays, geometry, landmarks, fit and optimization windows remain unchanged during painting. Unapplied drafts are not retained after closing the app. The existing total20,000-voxel per-image recipe cap remains; use masks for larger regions. Saved/exported data use the committed recipe, never preview pixels. Apple Silicon macOS only. Native graphical responsiveness is not certified in this sandbox; committing still performs one full reconstruction and texture update, including mask/hole recomputation where applicable.

## 0.16.70 — 2026-10-16

Systematic audit and fixes for physical fallback spacing, nonfinite interpolation, crop navigation, worker failure cleanup, original-fit reset/metadata, and recoverable file overwrite. Added seeded scientific and fault-injection regressions. Apple Silicon only; native GUI validation remains pending.

Current audit release (0.16.70): fixed selected-affine/header spacing disagreement in decoder fallback; exact linear/cubic samples no longer inherit NaN/Infinity from zero-weight neighbors; unchanged-grid physical crops relocate excluded crosshairs; nonfinite voxel clicks are rejected; preparation/generated/native/processed export workers clean up synchronous message-cloning failures; resetting the initial landmark alignment restores its model, parameter count and diagnostics as well as its matrix; transform metadata reports original-landmark diagnostics separately from current affine diagnostics. Failed overwrite/rollback retains the recoverable original, and backup-cleanup failure cannot trigger destructive rollback. The obsolete hidden refinement entry now delegates to the current cancellable worker, and legacy pending-proposal rejection restores the accepted pose. Added independent adversarial regressions and 320 seeded all-pair transform recoveries. Apple Silicon macOS only. Native GUI/GPU, real anatomical accuracy and large-volume memory/performance remain unverified; test success is not proof of absence of all bugs.

Rename the manual initializer to Begin manual alignment, with plain-language help: create a starting overlay so MRI or CT can be moved, rotated, scaled or sheared; initial overlap may be poor. Show this button/help only when no alignment exists, including after rejecting an initial proposal with no previous alignment. Hide and disable it for every existing alignment, including pending proposals; use the manual controls or explicit proposal decisions directly. The underlying legacy initializer handler also refuses to replace an existing fit, so old/programmatic routes cannot reset registration. Update guide copy and packaged-label requirements while preserving internal image-geometry provenance, physical coordinates, registration, session and export formats. Apple Silicon macOS only; numeric version0.16.69.

## 0.16.68 — 2026-10-15

Pending refinement is a proposal, so manual editing and new registration remain locked until an explicit decision. The guide previously said Accept/Reject but offered only Inspect, and users switching to Manual could not reach the hidden decision controls. The guide now delegates Accept, Reject, Accept-and-adjust-manually and Reject-and-adjust-manually to the existing canonical handlers for local, global and spline proposals, plus Save session for later. Manual adjustment has visible inline decision buttons above the locked controls. Decisions never silently occur when selecting a workflow or pressing a disabled nudge. Accept/reject returns the next-step hub; direct manual choices reveal enabled manual controls, or the image-geometry initializer when rejecting a first proposal without a prior alignment. Guide refresh keys track proposal presence and kind so choices cannot go stale. Landmark collection stays on one neutral Add-pair step after the minimum, with no primary Apply promotion or automatic expansion of Landmark alignment; users choose when to fit. Added a practical five-pair rigid starting set (anterior/posterior/superior midline and left/right lateral mid A–P), with identifiable paired anatomy, distribution and overlay review emphasized rather than a guarantee from count. Introductory text explains display versus masking/cropping/voxel editing, and the post-processing action says Start with landmarks. Opacity runs40% faster (an8-second cycle becomes about5.71seconds), retaining the same range, shader-only updates and stop paths. Apple Silicon macOS only; numeric version0.16.68.

## 0.16.67 — 2026-10-15

The guide now walks through Image display, Binary mask, Cropping and Voxel editing as four optional stages, with explicit Continue/skip and one-step Back. Creating/applying a mask or changing a slider never completes an entire stage. Completed landmark alignment and accepted/rejected registration proposals return to a next-step hub with all sidebar sections collapsed; new landmark placement and manual adjustment wait for the user. Inspect alignment is a prominent guide action that reveals Alignment display. Opacity animation cycles smoothly from25% to75% and back over8seconds, using the existing shader opacity path without resampling; the button, manual slider input, hidden document, reset and page close stop it. The export dialog now centers on the transformed moving image, with independent data source (native, native with applied preprocessing, or current processed working image), FoV, native/working/custom spacing and nearest/linear/cubic interpolation. 2×/4×/8× presets derive from moving-native spacing. The summary shows actual endpoint-preserving grid spacing, voxel count and uncompressed size. Full native reference FoV can be retained independently of processing; linear transforms additionally support full native or cropped moving FoV. Nonlinear splines use reference FoVs; moving FoVs are explicitly unavailable. Native preprocessing projects the saved working-grid mask and edited voxel footprints into native coordinates with nearest-cell selection, preserves other native intensities, pads outside crops with0, and does not apply registration exclusion masks to intensity data. Working-resolution masks cannot recover native segmentation detail. Generated masks and unregistered working images have a separate disclosure. Redundant original-input copy choices are removed; cropped or edited working images are offered even without a mask. Aligned image sidecars and transform JSON record selected options/geometry; display window/contrast is never baked into exported data. Cubic interpolation can overshoot; downsampling here uses the chosen interpolator without a separate anti-alias filter. Version numbers are numeric. This candidate ships Apple Silicon macOS only; other platform builds wait for approval of stability. Native graphics remain pending in this sandbox.

## 0.16.66-manual-affine.1 — 2026-10-15

Manual adjustment now offers independent Adjust MRI / Adjust CT targets without changing the saved export direction. Twelve controls provide reference-RAS translation (mm), rotation (degrees), positive axis scales (%) and XY/XZ/YZ shear; rotation/scale/shear pivot around the other image’s center. Positive scale bounds are 5–2000%; shear is bounded to ±3. Reset all manual adjustments restores the accepted base. Optional transformed source-row preview shares the already resampled overlay array, labels its reference grid, and restores native views before landmark, region or preprocessing edits. Original voxels remain unchanged. Start from image geometry uses NIfTI physical coordinates without estimating a registration. Manual affine controls compose on both sides of the canonical mapping; spline coefficients/control snapshots remain unchanged, with a proper input affine and the same-map inverse retained. Schema9 saves both targets’ controls and validates recomposed matrices; older sessions remain supported. The free guide-window dragging fix is included. Native graphical appearance and interactive rendering remain pending a graphical run.

- Let explicit guide dragging/keyboard movement override automatic image avoidance, so the guide can cross the sidebar boundary and float out of either docking mode.
- Restore automatic placement on Follow/new steps, retain viewport clamping and clean up overrides on close. Add real and compiled DOM drag regressions; rebuild all platform apps, source, full bundles, docs, validation and SHA-256 manifests.

## 0.16.64-mask-holes.1 — 2026-10-15

- Continue from preprocessing to an acknowledged registration-choice step, preserving one-step Back and current-alignment review.
- Add explicit 3D cavity / voxel-Z slice hole filling to generated binary masks, with bounded worker flood fill, exact generated previews, saved-mask application, Undo, schema8 sessions and metadata.
- Write working-grid image/mask exports directly to avoid interpolating 0/1 masks on oblique grids. Add independent reference, topology, actual worker, preview/export/session/cancellation regressions. Rebuild all platform releases, shared source, full bundles, docs and SHA-256 manifests.

## 0.16.63-mask-preview.1 — 2026-10-15

- Refresh mask previews after completed source-renderer draws, including deferred layout/resize events. Preserve viewer texture, zoom and pan on metadata-only mask creation.
- Add six-view real-renderer projection/aspect and asynchronous redraw regressions, retaining voxel/export isolation checks. Rebuild all platforms, shared source, full bundles, documentation, validation and SHA-256 manifests.

## 0.16.58-processing.1 — 2026-10-15

## 0.16.62-binary-mask.1 — 2026-10-15

- Separate binary masking from view-only display adjustments. Remove display-range copy and sampling-mask assignment controls. Add explicit mask creation before optional application to working data.
- Export original inputs, generated binary masks and masked images independently, available before alignment. Preserve registration export gates and never bake display adjustments into voxels.
- Preserve mask snapshots/session compatibility with recipe3/schema7, add real worker/data export/isolation checks, rebuild all platform releases, source, docs and checksum manifests.

## 0.16.61-preprocessing.1 — 2026-10-15

- Keep preprocessing source-only: applying masks/crop/edits updates working images without fitting or resampling an overlay. Preserve transform/landmarks and offer explicit Refresh overlay using the unchanged transform.
- Replace misleading shared registration progress/cancel/guide wording with task-specific kinds. Label registration sampling-mask assignment explicitly.
- Add real-worker CT-only and accepted-transform boundary regressions; rebuild all apps, source, full bundles, documentation and SHA-256 manifests.

## 0.16.60-mask.1 — 2026-10-15

- Add independent MRI/CT threshold mask controls and live green previews in all source views, with inversion and explicit display-range copy. Cache bounded physical slices so dragging recolors visible pixels without full-volume processing.
- Add separate reversible registration-exclusion and outside-fill actions, clear, binary selection-mask export and independent saved thresholds. Preserve native originals and existing crop/voxel edits.
- Introduce processing recipe version2 and session schema6 with legacy support and real-worker/preview/session/export regression tests. Rebuild macOS, Windows and Ubuntu apps, source, full releases, docs and SHA-256 manifests.

## 0.16.59-display.1 — 2026-10-15

- Return optimization-region controls to Landmark refinement and Image-based registration with shared bounds and workflow-specific guide routing. Initial processing no longer jumps into registration after region editing.
- Add separate MRI/CT display contrast and minimum/maximum sliders, numeric thresholds and reset. Synchronize all source/overlay views without modifying voxels, registration or exports.
- Add regression coverage and rebuild all platform releases, source, documentation, validation and checksum manifests.

- Add Initial processing before Landmarks, collapsed on launch. Move shared optimization regions into this single editor; registration sections link to it.
- Add reversible physical crop boxes, individual voxel intensity editing (default 0), separate registration exclusion/inclusion masks, bounded Undo/Redo and Restore original.
- Add cancellable processing/export workers, processed image/mask/recipe exports and explicit native versus processed aligned export. Preserve original inputs and arrays.
- Persist validated recipes in schema-5 sessions; require the same working grid, support older sessions and refuse silent resolution changes of edited voxels.
- Extend full and packaged-worker regression validation; rebuild all platform releases with shared source, documentation and checksums.

## 0.16.57-resolution.1 — 2026-10-15

- Reorganize Resolution into desktop columns: image spacing left, presets and memory planning right.
- Keep title and Prepare/Cancel actions visible; stack columns on narrow windows and retain overflow safety for short screens/large text.
- Preserve preparation, memory planning and native export behavior. Rebuild platform bundles, shared source, documentation, validation and checksums.

## 0.16.56-sidebar.1 — 2026-10-15

- Start every sidebar section collapsed; ignore legacy saved expansion and remove the conflicting setting.
- Clear workflow highlights when their last section closes. Preserve independent disclosure behavior and section order.
- Reveal all ancestors for explicit guide/export targets, including nested spline controls.
- Audit startup, preferences/presets, guide, reset, export review and workflow routing; add compiled and DOM regressions.
- Rebuild all platform bundles, shared source, documentation, validation and SHA-256 manifests.

## 0.16.55-spline-ui.1 — 2026-10-15

- Move advanced nonlinear landmark spline, diagnostics and preview decisions into Landmark alignment after the linear fit.
- Keep Landmark refinement focused on constrained image-based refinement.
- Update guide and export-review routing; preserve landmark prerequisites, fitting math and physical regions.
- Rebuild all platform releases, source, documentation, validation, manifests and SHA-256 checksums.

## 0.16.54-refinement.1 — 2026-10-15

- Put constrained image-based refinement and its optimization regions first within Landmark refinement; place advanced nonlinear landmark spline below.
- Replace the moving shared form with permanent, independently identified local/global forms and run buttons. Keep one registration execution controller/worker; each run reads only its own metric, sample budget, pyramid and histogram settings. Local fitting retains its constrained rigid adjustment and preserves accepted scale/shear/spline.
- Show adjacent live eligibility messages for refinement and spline. Disable spline until both images, an accepted landmark-derived base and 12 complete enabled pairs are available; recommend 20+ distributed pairs.
- Derive each result display from its own current/restored FitState metadata, so rejected proposals cannot leave another workflow’s score text behind.
- Keep local/spline proposal review and accept/reject controls in Landmark refinement. Guide/export review targets follow the proposal’s workflow. Shared physical regions and independent sidebar disclosures remain intact.
- Add emitted-worker request checks for independent local settings, permanent DOM/order tests, disabled-button explanations and local proposal actions. Native graphical verification remains pending.

## 0.16.53-startup.1 — 2026-10-15

- Start with no highlighted/selected registration workflow. Remove the initial Landmarks selection and stop replaying the previous workflow button when restoring layout (the cause of a saved Manual highlight on launch).
- Remember panel expansion independently across all six sections. Ignore legacy saved workflow fields, store only panel states, and clarify the Settings label. Restoring layout must not click workflow controls or select registration operations.
- Verify neutral fresh markup and the actual compiled startup with a saved Manual workflow; retain panel expansion without simulated clicks or selection. Explicit user/guide workflow navigation still selects its workflow normally.

## 0.16.52-guide.1 — 2026-10-15

- Finish guide now closes/completes the guide immediately. Remove the extra terminal screen and second End guide click from the normal flow. Legacy saved terminal routes also offer the single Finish guide action.
- Preserve loaded images, landmarks and alignment; clear guide highlighting and the Resume affordance when finished. Closing with × still pauses the guide.
- Update the existing guide regression to check one-click completion and unchanged workspace state.

## 0.16.51-sidebar.1 — 2026-10-15

- Make native sidebar disclosure headings independent: expanding/collapsing one section preserves every sibling section. Opening a registration section selects its own operation without clicking a workflow shortcut; closing changes no operation.
- Order the sidebar Landmarks, Landmark alignment, Landmark refinement, Manual adjustment, Image-based registration, Alignment display. Rename Landmark follow-up to Landmark refinement.
- Keep explicit workflow shortcuts as focused navigation. If both registration sections are independently open, show a named action to bring the single shared registration controller into that section; bounds, per-operation settings and active capture remain shared/preserved.
- Guide refresh/rebuilt landmark rows respect a manually collapsed section. New guide steps, explicit navigation and Follow highlighted control can reveal the section needed for that action.
- Add real disclosure-click tests covering all six sections in both directions, sibling-state preservation, operation changes only on opening, shared controls with both sections open and guide repaint/collapse behavior.

## 0.16.50-guide.1 — 2026-10-14

- Fix a landmark guide timing race: the instruction advanced before the next animation frame created its Set button, fell back to highlighting Guided setup, then cached that fallback indefinitely. Resolve the real target on every refresh and observe completed landmark-list renders; a missing target never highlights an unrelated launch button.
- Track individual displayed guide steps, including MRI/CT placement, rather than only broad workflow routes. Back revisits one instruction at a time without deleting/reverting landmarks; completed points offer review/Replace and Continue with current progress.
- During point placement show an explicit Set MRI/CT point action linked to the actual app control, instead of disabled Add/Apply actions. Keep Add and Apply on the completed-pair screen.
- Protect the whole active landmark row as well as the required image panes from floating guide placement. Rebuilt DOM nodes reacquire the outline without repeatedly restarting its attention pulse.
- Add real DOM-event tests for delayed/rebuilt target rows, two full landmark pairs, adding a third pair, per-point Back, preserved coordinates, returning to current progress and pause/resume. Native graphical verification remains pending.

## 0.16.49-resolution.2 — 2026-10-14

- Keep the toolbar on one control row; MRI, CT and Resolution belong to Volumes, with separate Session and Landmarks groups. Narrow windows scroll horizontally instead of wrapping.
- Rename the preparation dialog Resolution. Show X/Y/Z spacing in one row; show Volume / time point only for multi-volume inputs and explain its purpose.
- Replace ambiguous detail/memory presets with Native, 2×, 4× and 8× native spacing in increasing downsampling order. Keep the joint recommendation separate. Presets never upsample.
- Correct capacity guidance: compare the conservative lower planning envelope against installed RAM minus headroom or an explicit budget. Free RAM is contextual only; loading images must not trigger a false pressure alarm. Upper-envelope/workload guidance remains advisory. Export confirmations use installed capacity rather than transient free RAM.
- Add regressions for toolbar ownership, actual ordered preset clicks, conditional volume selection and capacity readings before/after loading. Native rendering and measured peak-memory calibration remain pending.

## 0.16.46-export.1 — 2026-10-14

- Disable image/transform exports for pending proposals and show the acceptance requirement inside Export, with a direct Review pending alignment action. Reloadable session saving remains available.
- Show export progress/errors inside the modal and prevent overlapping export clicks.
- Propagate destination write/cancellation failures instead of reporting false success; stop multi-file exports on failure while allowing subsequent retries.
- Added DOM/API-adapter regressions for proposal gating, sessions, review navigation, failure visibility, batch failure and queue recovery.

## 0.16.48-resolution.1 — 2026-10-14

- Stage MRI/CT selection and inspect scalar NIfTI headers before preparing voxel data. Joint preparation shows native geometry, frame count, file size, decoded-frame size, editable per-axis working spacing and derived dimensions. Recommended/higher-detail/lower-memory/native presets and optional memory budget are user controlled.
- Read current system RAM/available-memory estimates through restricted native IPC. Show live conservative processing/export ranges, workload guidance, headroom and a high-pressure confirmation. Browser capacity can be unavailable; decoder fallback estimates are explicitly unknown.
- Stream compressed/uncompressed NIfTI-1/2 selected frames through normalized triangular antialias filtering and decimation without full-native allocation. Preserve physical affine origin, directions and endpoints; never fabricate finer native sampling. Other formats retain a labelled native decoder fallback.
- Atomically commit both prepared images. Cancellation/decoding failure preserves the old workspace; resolution-only changes preserve physical landmarks, windows and transforms. Dataset/frame changes invalidate the fit and changed-image regions.
- Share immutable viewer voxel arrays with independent metadata/IDs; convert only one decoded frame in fallback; keep one pyramid scorer at a time; reuse metric histograms; bound ROI sampling iteration without changing sample phase or physical axes. Registration exposes finest working spacing and sample budgets.
- Reload native moving inputs for export and use native fixed geometry without its pixel payload. Worker slab resampling/incremental gzip handles linear and full spline maps; compressed Blob/destination buffers remain included in memory guidance. Source-array allocation cap is 1.5 GiB.
- Save native geometry, working spacing/dimensions, selected frame and registration-resolution provenance in sessions/registration metadata. Added streaming, UI, emitted-worker, native-export, memory and cancellation regressions.


## 0.16.47-workflows.1 — 2026-10-14

- Separate Landmark follow-up from independent Image-based registration. Follow-up contains advanced paired-landmark spline fitting and constrained rigid image-based refinement; global registration retains all 6/7/9/12-parameter and custom options.
- Put region editors in both workflows, delegating to one shared physical MRI/CT window state. Switching tabs preserves regions, active editing and local search limits.
- Require a landmark-derived base for refinement; preserve old landmark/local sessions and retain landmark lineage through subsequent refinement/manual adjustments. Global registration no longer inherits an unrelated landmark snapshot.
- Keep custom histogram bins when reopening a tab/resuming the guide and remember bins separately for the two operations.
- Guide routes reflect the separate workflows, shared optional regions, nonlinear landmark follow-up, and pending proposal review. Added DOM regression coverage.


## 0.16.45-guide.1 — 2026-10-14

- Place the guide clear of its highlighted control and the image row required for landmark placement, window selection or overlay review. Choose a new safe position as steps change; manual placement is retained only while clear.
- Reserve dedicated guide space when no clear floating position fits; narrow windows use a bottom tray. Preserve image/window physical state during layout resize.
- Pulse the new target outline three times over 1.8 seconds, then retain the steady outline. Respect prefers-reduced-motion and clean up pulses on target changes/close.
- Added DOM-event/bounding-box regressions for MRI/CT step changes, unobstructed active areas, crowded-layout docking, pulse cleanup and reduced motion.

## 0.16.44-guide.1 — 2026-10-14

- Synchronize active guide routes with direct workflow-tab, registration-operation and tool actions after application handlers complete.
- Automatically continue after finishing optimization-window editing, preserving the selected global/local operation. Accept/reject decisions for automatic, refinement and spline proposals all return to review.
- Changing the landmark model returns to the appropriate pair requirement/apply step instead of showing an old model’s fit as complete.
- Keep parameter changes and multi-point/window/manual tasks on their appropriate step until complete; manual review retains an explicit Continue. Prevent guide-proxied actions from feeding back into duplicate transitions.
- Added real DOM-event regression coverage for direct versus guide actions, operation preservation, parameter waiting, all proposal decisions, and pause/resume. Scientific fitting and optimization-window geometry are unchanged.

## 0.16.43-desktop.1 — 2026-10-12

Ubuntu packaging added 2026-10-13: x64 `.deb` and portable tar.gz, explicit linked-library dependencies with Ubuntu t64 alternatives, synchronized desktop entry/window identity, normal Chromium sandbox/AppArmor install/remove scripts, and verified Electron upstream checksum. Scientific code and version are unchanged; native Ubuntu validation remains pending.

- Added Windows x64 installer and portable desktop ZIP; excluded macOS launcher payloads, retained all registration features, checked PE branding/ASAR integrity and exact installer contents. Native Windows execution remains pending.

- Split macOS desktop releases into Apple Silicon and Intel packages; removed both standalone Node runtimes from desktop apps.
- Retained workstation authentication and launchd ownership using the app's embedded Electron Node. Explicit Node-mode environment survives launchd; executable paths and arguments remain quoted.
- Delivered browser compatibility as a separate self-contained universal browser package. All registration, spline, session and export features remain in each desktop build.
- Added real runtime-wrapper/workstation-adapter regressions, stale-payload removal, architecture checks and a 350 MB desktop size budget. Updated shared-source/full-release packaging, manifests and checksums for multiple architectures.

## 0.16.42-spline.1 — 2026-10-12

- Added an expert Nonlinear spline option to landmark alignment. Rigid remains default; automatic search models retain their existing linear options. Requires 12–128 complete, spatially distributed pairs; 20+ recommended, with explicit caution and smoothing controls.
- Fit a normalized regularized 3D polyharmonic/thin-plate spline after a proper affine base. A conservative global residual-gradient bound of 0.65 limits deformation rather than permit folding. Show attenuation, RMS, per-pair residuals, sampled Jacobian estimates, invertibility bound and extrapolation warnings.
- Fit, resample and export in cancellable workers; review/accept/reject proposals. Same-map inverse serves linked navigation, overlay markers, native-grid image resampling and displacement fields. Manual adjustment composes after the spline; local intensity refinement adjusts the whole existing warp while preserving coefficients. Global registration remains a new linear search.
- Schema-4 sessions retain the full spline and normalize/validate coefficients, controls, direction and RMS; older linear sessions remain supported. Nonlinear registration JSON marks matrix-only fields null and records the affine component and inverse recipe.
- Added forward/inverse RAS-mm vector NIfTI exports with configurable grid spacing (2 mm default), extent-preserving oblique grids, allocation limits and transform JSON metadata. Aligned images use the continuous spline inverse; sampled fields are approximate when interpolated by other software.
- Added numerical/nonlinear image, session, field-byte, local-refinement, emitted-worker, cancellation and production-control regressions. Existing optimization-window regression contract remains in force.

## 0.16.41-guide.1 — 2026-10-11

- After accepting or rejecting a refinement proposal, the guide returns to alignment review rather than jumping to export. Review offers manual adjustment, a new automatic registration, additional refinement, landmark retry and saving.
- Clarify that automatic registration proposes a new starting alignment while refinement improves the existing transform. The automatic guide route highlights the run control for accepted alignments too.

## 0.16.40-windows.1 — 2026-10-11

- ROI input/rendering now uses the physical slice tile rather than Niivue projection methods that return an invalid finite sentinel outside the image or reject off-screen corners. Drags clamp at panel edges; saved windows remain visible through zoom/pan/slice redraws where they intersect the view.
- Invalid or tiny replacement drags preserve existing windows. CSS-pixel size checks replace raw voxel-spacing thresholds; oblique/axis-swapped images retain physical RAS plane semantics.
- Pointercancel retains the last valid drag; primary-pointer guards and pending-listener disposal prevent stale capture handlers.
- Added real DOM capture regressions in every MRI/CT plane, including axial, mirrored viewports, scaling, cancellation, outside release, zoom/pan, session retention and worker axis metadata.
- Added root AGENTS.md protection rules and implementation comments documenting the recurring failure mechanisms and required checks.

## 0.16.39-reset.1 — 2026-10-11

- Added Reset workspace in the toolbar and guide. Confirmation explains that images, landmarks, alignment/proposals, windows, undo history, guide progress, recovery snapshots and recent dataset names are cleared. Original files, saved exports and personal settings remain intact.
- Reset blocks concurrent editing/autosaving, clears stored workspace metadata before reloading, and fully discards viewer resources, pending reads and in-memory state. Failures preserve the loaded workspace and report a retryable error.
- Guide method choice now highlights the guide control rather than scrolling to landmark fitting before a method is chosen. Restart guide explicitly describes retaining loaded work.
- Added cancellation, busy/double-click, persistence-ordering, failure and preserved-preference regressions.

## 0.16.38-guide.1 — 2026-10-11

- Reset saved walkthrough routes when starting with an empty workspace. Guide preferences and floating position remain remembered, but an un-restored session no longer shows Resume guide or skips the initial method choice after loading MRI/CT.
- Return review/window/refinement routes to the method choice when their required starting alignment is absent. Same-session hide/resume with valid data remains unchanged.
- Ignore asynchronous saved-route restoration after a user has already opened the guide.
- Added executable fresh-launch regressions for every saved route, checking labels, MRI/CT loading and the landmark/manual/automatic choice.

## 0.16.37-windows.1 — 2026-10-11

- Fixed drawn optimization windows losing plane identity at the registration-worker boundary. Each rectangle restricts only its two visible physical axes; its slice depth remains unrestricted. Applies to fixed/moving images, global pyramids and constrained refinement in either direction. Existing session window format remains compatible.
- Guided automatic registration selects the global operation; guided window/refinement steps select constrained local refinement. User-selected transform models and local bounds remain available.
- Clear stale errors when a new search begins and distinguish preparation of constrained refinement from global registration.
- Added regressions reproducing the previous zero-thickness failure, then exercising real drawn rectangles through the emitted worker, all planes, both directions and oblique/anisotropic physical geometry. Truly empty windows remain rejected.

## 0.16.36-settings.1 — 2026-10-11

- Added Settings via toolbar and native application menu (⌘/Ctrl + ,), with keyboard focus management, six sections and automatic preference saving.
- Graphite + Amber, Neutral dark, Light and High contrast interface palettes, custom color editing/live preview, contrast checks, text sizing and larger targets. Imaging overlay colors remain independent of interface colors.
- Added fixed/moving overlay tints, default/remembered overlay mode and opacity, crosshair colors/thickness/visibility, landmark marker size/labels, linked navigation and slice-scroll direction/sensitivity.
- Added guide docking in a reserved column, optional launch opening, persisted guide route and floating position, and remembered workflow/panel layout.
- Saved transform/refinement defaults do not change existing transforms or active controls until explicitly applied. Original rigid defaults remain intact.
- Named configuration presets, JSON import/export, per-section reset and full default restore.
- Restricted, context-isolated Electron preference IPC with fixed storage keys, atomic serialized writes, size bounds and main-frame authorization; desktop settings survive changing loopback ports and workstation connections.
- Recovery autosaves session metadata/state, retaining the previous launch snapshot; reload original image volumes before restore. Added recent dataset names, native default export-folder selection, and remembered desktop window size.
- Added settings and desktop-storage regressions, and exercised Settings in the actual emitted and packaged frontend.

# v0.16.35-refinement.1

- Restored constrained local refinement: Tight (2 mm / 1°), Standard (5 mm / 10°), and Custom bounds, with configurable translation/rotation start steps and four halved-step levels. Runs in a cancellable worker and uses initial physical overlap support with a original valid-sample threshold (25%, minimum 64, capped at 500). Original 40-bin NMI is the local default; other image metrics and transform models remain available.
- Refinement bounds and steps are recorded in session/export automatic registration settings.
- Optimization windows are now first in the Automatic tab, above operation/model/run controls. Global registration remains a distinct broad orientation/pyramid search.
- Added bounded refinement and exact built-worker UI regression coverage.

# Brainana Align changelog

## 0.16.34-automatic.1 — 2026-10-11

- Add automatic image registration without landmarks and refinement of an accepted transform, with rigid, similarity, anisotropic, affine, and custom parameter groups; rigid remains default.
- Add five intensity metrics, broad/full-orientation search, bounded translation/scale/shear, fixed/moving optimization windows, and Gaussian input-relative pyramids.
- Correct resolution-level score comparisons, fixed-space global sampling, explicit direction handling, and custom held-parameter constraints from the supplied source.
- Run preparation/search in a cancellable worker; fuse filtering/downsampling to reduce full-volume passes; retain the prior alignment until a proposal is accepted.
- Preserve previous transforms on reject and pending proposals in schema-3 sessions; extend exported provenance with models, settings, metrics, scores, and search warnings.
- Add image phantom and exact built-worker tests; package full universal macOS/source release with documentation, manifests, and checksums. Native GUI certification remains pending in this environment.

## 0.16.33-workflow.1 — 2026-10-11

- Stable landmark-guide action order: Add landmark pair first, Apply landmark transformation second, alternative method third. Apply stays disabled until ready; adding or completing points does not shift the action positions.
- Landmarks/Manual/Automatic workflow focus collapses unrelated panels while retaining data. Shared direction remains visible; image-geometry initialization is available directly in automatic/manual workflows.
- Visible alignment activity strip, staged search progress, guarded concurrent runs and temporary editing lock; refinement yields between search axes so progress can paint.
- Moving-opacity slider uses a dedicated two-volume review shader through Niivue’s custom-slice API with animation-frame coalescing. On the successful shader path, slider gestures perform no volume texture rebuilds, colormap resets or resampling. A compatibility fallback preserves normal rendering if shader compilation fails.
- Inspected supplied .27 packaged frontend: Automatic refinement was present; no separate global automatic-registration control was found. Current descriptions explicitly state local-search limitations.
- New workflow/opacity/activity regressions and expanded actual production-bundle checks; scientific models and confirmation/export/session behavior retained.

## 0.16.32-guide.1 — 2026-10-10

- Guided setup can be dragged by its header or moved with arrow keys; its chosen position persists across steps and is constrained to the window. Follow highlighted control returns to automatic positioning.
- ×, Escape and the toolbar toggle hide the guide while preserving its route/history. A Resume guide control appears in the toolbar and status strip; resuming reflects work completed while hidden. Restart guide is explicit.
- Landmark prompts now offer Add landmark pair and, at the selected model’s minimum, a primary Apply landmark transformation action. Registration alternatives no longer displace the fitting action. Normal fit validation and reflection confirmations remain in control.
- Expanded state/DOM and exact production-bundle regressions for pause/resume, hidden-state advancement, movement/clamping/cancellation, model-dependent minimums, add/apply actions, fitting errors and explicit restart.

## 0.16.31-theme.1 — 2026-10-10

- Viewer-style warm graphite palette, amber primary actions, Source Sans 3 typography and IBM Plex Mono coordinates; fonts and license notices bundled offline.
- Left registration rail, compact grouped toolbar, separate accessible status strip and a flexible nine-pane grid that fits the native minimum window size.
- Consistent guided-setup, export, remote browser and folder-picker styling; diagnostic annotation/overlay colors retain their existing meanings.
- Extracted registration shell markup into alignmentLayout.ts and theme tokens into theme.css; generated release identity supplies the displayed version.
- Added exact packaged font/notice serving checks and expanded production UI checks for shell wiring; repaired the browser regression's duplicate declaration and added minimum-window/guide-placement checks for environments that permit browser execution.

## 0.16.30-desktop.1 — 2026-10-10

- Dedicated Electron application window with embedded Chromium, native menus/download dialogs, single-instance focus and owned server shutdown.
- npm workspace structure parallel to Brainana Viewer: apps/align composition roots and independent core-desktop, core-launcher, core-server, core-client and imaging-math packages. Viewer functionality is not imported.
- Existing workstation profile/interactive SSH flow hands its authenticated server to the native window; private desktop instance state, connection-change reminder and backend/SSH cleanup.
- Existing browser mode, four physical-space landmark models, guided setup, session compatibility and registration/export behavior retained.
- Executable desktop security/lifecycle regression tests, package content checks, universal macOS packaging and symlink-preserving release archives.


## 0.16.29-guided.1 — 2026-10-10

- Added an opt-in Guided setup toolbar button with anchored, highlighted, nonmodal prompts, keyboard dismissal, Back, restart, and cleanup.
- Guided setup recognizes existing data, waits for successful loading, retains loading/fit errors, guides each MRI/CT landmark pair, and adapts minimum counts to the selected transform model.
- Added choices and skips for landmarks, manual adjustment, automatic refinement, optional windows, proposal acceptance/rejection, and saving/export. Pending proposals take precedence over other registration guidance.
- Added explicit Start from image geometry initialization for users who skip landmarks. It uses existing physical coordinates as a starting alignment, warns that automatic refinement is local, and requires visual overlap review. Existing alignments require confirmation before replacement.
- Preserved geometry initialization in sessions and exported metadata. No-landmark RMS is displayed as unavailable and exported as null; manual/refinement updates remain serializable.
- Added executable guided-state and DOM interaction tests plus production-bundle checks for the guide entry point and viewer-error behavior. All four landmark models and packaged-runtime verification remain covered.


## 0.16.28-landmark.2 — 2026-10-09

- Added the default rigid 6-parameter, similarity 7-parameter, constrained anisotropic-scale 9-parameter, and affine 12-parameter landmark models.
- Added centered, normalized, pivoted-QR affine least squares and joint positive no-shear anisotropic fitting with two rotation starts; retained large-rotation-safe Horn/Jacobi rigid fitting.
- Replaced rigid-only inversion in overlay resampling, manual adjustments, image refinement, acceptance, and export with general affine inversion.
- Added RMS, determinant, physical-axis scale, shear, reflection, and geometry/conditioning diagnostics. Reflections require explicit apply, restore, and export confirmation; a superior reflected candidate can trigger a convention warning.
- Added validated session schema 2, old rigid-session migration, correct inverse regeneration, explicit coordinate/space metadata, model-specific export filenames, and pending-refinement restoration. Pending proposals cannot be exported without acceptance or rejection.
- Fixed linear interpolation at the last voxel plane, spatial-unit normalization, fixed-grid NIfTI output sform, and multi-volume output dimensionality. Resampling applies each image affine once.
- Closed local symbolic-link escapes outside the configured data root.
- Replaced internal-registry lockfile URLs with portable public npm URLs; applied compatible dependency security patches. Dependency audit reports zero known vulnerabilities.
- Corrected minimum macOS from 11.0 to 13.5 to match both actual bundled runtime binaries. Apple Silicon runtime executes as Node 24.17.0; Replaced the supplied truncated Intel runtime with the official SHA-256-verified Node 24.17.0 x64 binary. Both runtime signatures and authenticated packaged server smoke tests pass, including x64 execution through translation.
- Strengthened release checks for all four packaged dropdown labels, source/runtime hashes, Mach-O architecture, load-command/segment/signature bounds, and actual deployment targets. Added malformed-binary regressions. Corrected CI to install the locked Playwright engines and use their discovered paths; the full check gate runs in CI.
- Added executable scientific, NIfTI serialization/reload, session/reflection, and production-bundle DOM tests. Validation evidence distinguishes passed checks from sandbox-blocked browser certification.


## 0.16.27-landmark.1

- Replaced the rigid landmark solver's power-iteration quaternion eigensolver with a symmetric Jacobi eigendecomposition that selects the largest algebraic eigenvalue.
- Corrected failures for scans separated by large rotations, including quarter turns, strongly oblique orientations, axis-swapped relationships, and near-180-degree rotations.
- Confirmed that landmark fitting operates in NIfTI physical world millimeters and preserves explicit CT-to-MRI or MRI-to-CT transform direction.
- Added clear rejection of coincident or nearly collinear landmark configurations that cannot constrain a stable 3D rigid transform.
- Added regression coverage for forward and inverse fits, large rotations, oblique voxel-to-world geometry, and degeneracy handling.
- No SSH, launch lifecycle, local loading, remote loading, optimization-window, export-format, or browser behavior changed.

## 0.16.26-docs.1

- Replaced per-release architecture, changelog, and validation Markdown files with one maintained document of each type.
- Removed the duplicated `Documentation-release/` tree.
- Consolidated browser-support policy into the current architecture and validation documents.
- Added an automated documentation-layout test that fails on version-stamped release documents, duplicate documentation trees, or missing maintained documents.
- Updated package manifests and release verification requirements for the consolidated documentation model.
- No application, scientific, SSH, local-mode, remote-mode, or export behavior changed.

## 0.16.25-local-start.1

- Local mode now opens directly without a startup MRI/CT folder chooser.

# Brainana Align 0.16.24-ssh.1

## SSH reliability correction

- Removed the explicit OpenSSH `-f` option from the interactive persistent-master command.
- The Terminal helper now uses the command form verified successfully on the affected macOS/OpenSSH 10.2 system: `ssh -M -S <socket> -o ControlPersist=600 -o ExitOnForwardFailure=yes -NT <target>`.
- OpenSSH may background the control master through `ControlPersist` only after authentication succeeds.
- The launcher continues to require a successful `ssh -O check` before starting the application server.
- Failed and timed-out SSH helper directories are retained and their paths are reported for diagnosis.
- Added a regression test that rejects any generated helper containing `-fNT`.
- Browser-matrix functionality from 0.16.23-browser-matrix.1 is retained unchanged.

# Brainana Align 0.16.21-lifecycle.6

## Fixed
- Replaced the unreliable AppleScript `do script` SSH helper launch with an executable `.command` helper opened by macOS LaunchServices in Terminal.
- Added a positive startup acknowledgement so the launcher distinguishes “Terminal accepted the open request” from “the helper actually ran.”
- Added explicit user-facing errors when Terminal does not launch or execute the helper.
- Corrected the successful-SSH race by waiting for the persistent control socket to become ready after SSH returns success.
- Preserved clean cancellation behavior for Control-C, Terminal-tab closure, HUP, INT, and TERM.

## Packaging
- Restored the complete application documentation set, release manifest, build metadata, and checksums to the macOS package.
- Added package-content validation so required release files cannot be silently omitted.

## 0.16.22-lifecycle.7
- Fixed nounset-safe generation of the Terminal SSH authentication helper.

Landmark alignment starts collapsed. The Landmarks workflow shortcut opens the point list, without pre-expanding Landmark alignment; open that section explicitly or let the guide reveal it when fitting is the current step.
