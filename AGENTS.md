# Optimization windows: protected behavior

These rules were added after repeated user-reported window regressions. Read the
window sections in Documentation/TECHNICAL_FINDINGS.md before changing capture,
projection, registration sampling, viewer coordinates or session serialization.

- Window rectangles constrain two **physical RAS axes**, identified by plane.
  Their depth is display context, never a zero-thickness 3D sampling constraint.
  Preserve the `axes` metadata in `registrationWindows` through the worker.
- All app slices use `setSliceMM(true)`. Use optimizationWindowGeometry for ROI
  input and rendering. Do not substitute raw voxel indices or override one
  fractional-volume axis for oblique images.
- Niivue canvasPos2frac can return finite `[-1,-1,-1]` outside an image. Finite
  checks alone do not validate it. frac2canvasPos may reject off-screen corners.
  Do not reintroduce either method into ROI capture/rendering without regression
  evidence covering these cases. Normal landmark projection is separate.
- A window is persistent physical state, not its temporary DOM rectangle. Slice
  navigation, zoom, pan, marker redraws, guide changes and opacity must not delete
  bounds. Project outside corners and let the display layer clip the rectangle.
- Rejecting a click/tiny/invalid replacement must preserve an existing window.
  Only explicit Clear windows / Reset workspace / deliberate dataset replacement
  or session restore may change persisted bounds independently of a valid drag.
- Size validation belongs in CSS pixels. Raw voxel-axis spacings do not describe
  the physical display axes of an oblique/axis-swapped image.
- Outside-panel pointerup and pointercancel must retain the last valid rectangle.
  Unrelated pointers and nonprimary buttons must not complete/cancel another drag.
  Dispose pending document listeners; do not multiply capture handlers.

Required verification for relevant changes:

    npm run build
    npm run test:windows
    npm run test:window-interaction
    npm run test:window-capture
    npm run test:automatic
    npm run test:automatic-ui
    npm run test:production-ui

The DOM capture regression covers all six MRI/CT planes, axial behavior, mirrored
viewports, CSS/backing scaling, oblique physical depth, outside releases, cancel,
invalid replacement preservation, zoom/pan persistence, sessions and worker axes.
Do not replace real event tests with tests that duplicate implementation or only
exercise bounds arithmetic. Run the full release validation and package verifier
for shipping. Native graphical verification remains explicitly pending when this
execution environment blocks browser/GPU startup; never claim it passed.

# Nonlinear transform contract

A nonlinear registration is not a 4×4 matrix. FitState.matrix/inverse are affine
components when nonlinear is present. Use createPointMapping for full forward and
inverse coordinates, including navigation, markers, residuals, resampling and
metrics. Compose local refinement after the spline; explicit manual image targets may
compose on either side using the saved proper inputAffine. Preserve its coefficients, and never export affine TXT as if it were the full warp. The inverse
is fixed-point inversion of the same bounded map, never an independently fitted
reverse spline. Validate the global gradient bound, proper base and landmark/control
consistency on restore. Run test:nonlinear, test:nonlinear-worker, test:automatic-ui,
test:production-ui and existing coordinate/session/window regressions after changes.
Maintain explicit RAS-mm vector conventions and field sampling metadata; LPS consumers must interpret both geometry and vectors correctly; ITK intent1006 readers may convert automatically, so never flip twice. Keep large-grid allocation guards and cancellation/rollback paths.

# Desktop runtime and size contract

Ship separate arm64/x64 desktop bundles. Do not embed standalone Node binaries
in workstation-support: the desktop supplies process.execPath, and
runtime-command.sh explicitly sets ELECTRON_RUN_AS_NODE=1 in the private launchd
wrapper. launchd does not inherit the parent environment. Preserve shell quoting,
per-connection ownership and cancellation. Browser packages alone carry standalone
Node for both architectures. Recreate generated payload directories to remove stale
runtimes. Run test:desktop-packaging and lifecycle/SSH tests; verify each packaged
architecture, signature, exact assets and the 350 MB installed-size budget. Distinguish
wrapper execution from actual launchd/SSH/native GUI validation.

# Guided-action synchronization

Observe direct tool actions after their application handlers complete. Internal
programmatic guide clicks/operation changes must not feed back into user-action
routing. Completing window editing follows the actual selected global/local
operation; merely editing parameters, navigating slices or drawing one rectangle
must not complete a multi-action task. Track proposal decisions for every route
(including automatic and spline), and refresh title/text/target when their content
changes. Hidden guides retain their route. Run test:guided's real DOM-event cases,
production UI, workspace reset and protected window regressions after changes.

Automatic guide placement must avoid the highlighted control and the active MRI/CT/review
panes. If no clear floating position fits, reserve guide space in the layout;
never choose a least-bad overlapping position. Explicit manual positioning may overlap controls or image panes and lasts only
until Follow highlighted control or the next step. Clamp it only to the viewport;
never route manual coordinates through automatic avoidance. Dragging or keyboard
movement can float out of automatic/preference docking without changing the preference. Keep docking/resize from mutating physical window state.
Pulse outlines only on new targets, three times; reduced-motion users get the
steady outline. Clean up timers/classes on target changes and close. Run the
real DOM/bounding-box and pulse cases in test:guided and all protected window tests.

# Export acceptance and save feedback

A pending proposal may be saved in a session, but cannot be exported as an
accepted image/transform. Keep these buttons gated and explain why inside the
Export modal, with a route to existing review controls. Never silently accept a
proposal. Propagate each destination failure to the caller; keep a recovered queue
tail only for subsequent retries. Stop multi-file saves on failure and never report
success after cancellation/write errors. Run test:export-dialog plus production
UI, server-local-export, reflection, session and resampling regressions.

# Registration workflow boundaries

Landmark alignment owns linear fitting and advanced nonlinear paired-landmark spline fitting. Landmark refinement owns constrained rigid image-based refinement. Independent Image-based registration owns global 6/7/9/12/custom models.
Require a landmark-derived accepted base for follow-up (including old sessions and
manual adjustments of landmark fits); global runs must not inherit landmark lineage.
Landmark refinement and Image-based registration each own a visible optimization-region editor, mirrored onto one canonical physical state and capture controller. Initial processing has no optimization-region editor.
Do not duplicate stored bounds/capture handlers or clear bounds on workflow switches.
The operation selector is internal; tab/guide navigation selects the route explicitly.
Same-operation selection/resume must not reset custom bins; each permanent form retains its own fields. Run buttons own their fixed local/global route; the internal operation selector tracks guide context only.
Run test:registration-workflow, test:guided, test:automatic-ui, production UI and all
protected window/nonlinear tests. Verify packaged labels for both workflows.

# Working images and native export contract

Scalar single-file NIfTI selection reads bounded header data, not a full voxel array.
Prepare workspace streams only the selected 3D frame through normalized triangular
low-pass/decimation in a worker. Keep native File references + native metadata separate
from working RawNifti/NVImage. Never use a working image as the native export source.
Origin, affine axis directions and voxel-center endpoint extents must stay invariant;
never upsample a coarse native axis by default. Landmarks/windows remain RAS mm.
Resolution-only changes preserve fit/manual/landmark/window state; dataset/frame
changes clear the fit and the changed modality's windows. Commit both inputs only
after success; cancellation/failure must preserve the previous workspace.
Viewer metadata/IDs are independent but img arrays are read-only shared buffers.
Do not reintroduce NVImage.clone's pixel copies or edit shared image values in place.
Keep only one active pyramid scorer, reuse histogram scratch, and preserve sample
phase/world-axis semantics when bounding iteration by ROI. Never remove window axes.
Native image export reloads moving voxels, uses native fixed geometry only, and
resamples/compresses slabs. Destination Blob/upload buffers remain budgeted; do not
claim end-to-end constant-memory export. Native scalar array has a 1.5 GiB limit.
Header-only/streaming fallback support must be labelled honestly for other formats.
Memory readings are trusted main-frame, read-only IPC; estimates are conservative
ranges, reserve headroom and do not claim registration accuracy or guaranteed speed.
Run test:working-volumes, test:preparation-ui, test:native-export-worker, all protected
window/coordinate/nonlinear/session tests, full release validation and packaged worker
proofs. Native GPU rendering remains pending unless actually executed.

Resolution UI: keep MRI/CT/Resolution in Volumes and separate Session/Landmarks. Toolbar controls must not wrap. Native/2×/4×/8× presets are ordered native-relative spacings; joint recommendation is separate. Show X/Y/Z in one row and volume selection only for multivolume inputs. Never infer pressure by comparing an entire planning envelope to transient free RAM: allocations already resident and reclaimable cache make that comparison misleading. Require capacity/budget tests covering unchanged geometry with falling free memory.

Landmark guide targets are created on a later animation frame and rebuilt frequently. Never substitute/cached-highlight the guide launcher when a point target is absent. Re-resolve selector identity before fast-path refresh; observe landmark-list renders. Protect the whole landmark row in guide placement. Back history must include point-level displayed steps, preserve coordinates and support review/Replace plus continuing current progress. Run delayed-target/rebuilt-row and multi-pair Back DOM regressions before shipping.

Sidebar disclosure headings must change only their own open state. Never delegate native summary toggles to workflow shortcut clicks. Closing must not select an operation or expand a sibling. Keep section order Initial processing, Landmarks, Landmark alignment, Landmark refinement, Manual adjustment, Image-based registration, Alignment display. Guide refresh/row rebuilds must respect manual collapse; new steps and explicit Follow must teach native heading clicks instead of opening panels. Registration execution must remain singular; local and global forms are permanent, independently identified DOM controls. Never move the form nodes between sections. Run real summary clicks in both directions for all seven panels and guide-collapse regressions.

Every sidebar section, including nested advanced sections, starts collapsed on every launch. Ignore legacy saved expansion and workflow fields; never restore expansion from preferences, imports, presets or sessions. Panel expansion is launch-local. Do not reintroduce a Remember panel expansion setting. Test the compiled startup with all seven saved panels open and legacy rememberLayout=true, including delayed initialization.

Landmark alignment starts collapsed. The Landmarks workflow shortcut opens the point list, without pre-expanding Landmark alignment; open that section explicitly or follow the guide’s prompt to click its heading when fitting is the current step.

Guide completion is one action: Finish guide calls completion directly. Do not reintroduce a terminal confirmation screen or second End guide click. Legacy done routes may remain for compatibility only. Finishing preserves the workspace and clears Resume/highlights; × remains pause/hide.

Workflow buttons start with aria-pressed=false. Never replay saved workflow clicks while restoring panel layout. Keep independent disclosure states during the current launch only; never restore them at startup. Keep the compiled startup regression seeded with a legacy Manual workflow; startup must dispatch no workflow clicks or select any workflow.

Landmark alignment contains the linear fit first, then advanced nonlinear landmark spline with its own result and Accept/Reject controls. Landmark refinement contains its own mirrored optimization-region controls and contains the permanent constrained image-based refinement form only. Guide and export-review links for spline proposals must target the spline controls in Landmark alignment. Global image-based registration has its own permanent form. Neither intensity workflow moves/hides the other form or borrows its parameter values. Keep one shared worker/controller and preserve physical ROI state. Disabled run buttons must have adjacent live explanations, including the spline’s 12-pair minimum and accepted-landmark prerequisite. Run real DOM ownership and spline review-delegation tests before shipping.

Explicit export navigation may reveal details ancestors. Guide navigation must instead teach users to click each required ancestor heading without opening it programmatically. Same-step refreshes must respect manual collapse. Closing the last section in a workflow clears its shortcut highlight without activating a sibling or changing registration state. Run the real DOM guide nesting and disclosure tests.

Resolution uses two columns on desktop: MRI/CT native and working spacing left, presets and memory planning right. Header and action/footer stay outside the scrollable body. Keep X/Y/Z in one row, native-relative presets ordered 1×/2×/4×/8× and one-column fallback for narrow windows or scroll safety for short screens/large text. Never clip controls or estimates to force a fit. Layout changes must not change memory calculations, preparation/export geometry or guide synchronization.

# Initial processing contract

Original working arrays and native File references are immutable. Processing creates a new array in a cancellable worker; Loaded.processing retains the original working base plus a validated sparse recipe and separate exclusion mask. Crop bounds are physical RAS mm, never zero-depth slice slabs. Reuse protected physical drag capture with exclusive crop/voxel/region modes. Crop changes affine translation by the selected voxel origin and retains orientation/scales; oblique outside-box corners are zero-padded and masked. Voxel edits refer to original working-grid indices; refuse different-grid session replay and resolution changes while processing is active. No silent index reinterpretation.

Masks gate physical sample centers and intensity-range samples at original working-mask resolution for global/local scorers, including pyramid levels and both directions. Pyramid filtering/interpolation remains unchanged; masks are not segmentation or mask-aware low-pass filters. Never encode masks by changing displayed intensities or by treating zero as exclusion. Keep native-aligned export on original native files (or original working base for decoder fallback); processed-working export is explicitly selected. Save crop/edits/mask recipe in schema-6 sessions (legacy schemas remain readable) and registration metadata. Legacy sessions load with empty recipes. Restore both images atomically; failures/cancellation preserve current images, coordinates, windows and transform. Processing Undo/Redo is bounded to 32 launch-local states; active recipes persist in sessions. Individual edits are capped at 20,000 per image. Never auto-accept pending registrations to process images.

Run test:initial-processing with actual emitted processing/export workers, test:native-export-worker (native versus explicit processed, including fallback), schema/session/nonlinear tests, all protected window tests, guide and compiled UI checks. Verify seven collapsed sections and two mirrored region editors in packaged UI. Native graphics cannot be claimed without a graphical run.

Display intensity controls are dataset-local viewer metadata only. Never mutate voxel buffers, registration intensities, exclusion masks or export values. Apply min/max to all three source views and the correct fixed/moving review volume in either direction; newly rebuilt overlays must inherit the range. Coalesce GPU uploads and never resample to change contrast. Run test:display-intensity and compiled UI tests.

Intensity mask drafts use independent per-image thresholds, never display contrast. Preview only recolors cached bounded 2D physical slices (nearest original working voxel + sparse edits, before mask fill); no full-volume copy, processing commit, registration or GPU volume upload per slider event. Preview uses a pointer-transparent canvas below markers, persists correctly on navigation/mirroring/resize and releases old volume references when hidden/reset. Registration masks and intensity fills are separate snapshots in recipe version2/schema6; original arrays/files stay immutable. Threshold masks combine with crop/manual exclusions; zero intensity alone never excludes. Both operations evaluate pre-fill intensities so repeated application/session replay is stable. Clear/Undo/Redo and cancellation preserve transactional history. Apply changes select the matching processing image so Undo targets the right modality. Exports distinguish selection mask (1 selected) from registration exclusion mask (1 excluded); native-aligned export remains original unless processed export is explicitly selected. Run test:threshold-mask with actual emitted workers in all platform package verifiers, plus all initial-processing, physical window, registration, session/native export and compiled startup regressions.

Preprocessing must never initiate registration OR automatic overlay resampling. Processing commits preserve the full accepted transform/proposal/landmarks, update the selected working image, and invalidate any existing overlay. Refresh overlay is an explicit display-only action using the unchanged transform. Later registration reads the current processed raw arrays. Shared progress has typed activity kinds: preprocessing uses processing labels in the banner, cancel button and guide, never Alignment in progress/Cancel registration. Mask application and mask sampling actions never click registration controls or move the guide into registration. Test the CT-only real worker path with an accepted nonlinear fit, unchanged MRI/transform, zero overlay refreshes, no registration workers, and processing activity kinds in test:threshold-mask/test:workflow/test:guided.

Binary mask workflow: display/window/contrast is independent. Never copy display ranges into mask thresholds or expose registration-sampling controls in the Binary mask section. Create/update binary mask saves a compact binary selection rule in recipe3/schema7 without changing source raw arrays or invalidating the overlay. Apply uses the GENERATED mask, never uncommitted preview thresholds, and fills outside that mask in the working image (default0); no registration/resampling. Creating a new mask does not retroactively change an already masked image. Export source/native inputs, saved binary masks and masked working images independently, available without alignment. Masks are1selected/0excluded. Export workers never read NVImage calibration/display pixels. Legacy schemas0–6/recipes1–2 remain readable; old thresholdFill can supply the generated binary mask and old sampling exclusions remain preserved without adding new ones. Run test:threshold-mask real workers, export-dialog gates, schema7, display and preprocessing isolation plus all release checks.

Mask preview projection lifecycle: DOM raster overlays must be refreshed AFTER source drawScene completes, including deferred Niivue parent ResizeObserver draws caused by progress strips, resizing and layout changes. Input/window resize listeners alone are insufficient: they run before the new screenSlices projection. Keep observeProjectionDraw installed on each source viewer; coalesce redraw notifications and preserve draw return/this semantics. Metadata-only binaryMask commits must not reupload/recreate unchanged raw/NVImage source viewers. Test real Niivue draw2DMain shader projections with wide/tall backing sizes in all six views and asynchronous draws without additional input; retain bounded cached slice recoloring and real-worker mask/export tests. Do not alter protected optimization-window math to fix raster lifecycle issues.

After processing, the guide must acknowledge completion and offer registration/current-alignment choices; never return to the initial How would you like to begin screen. Back must return to processing without resetting work or launching registration. Hole filling is an explicit generated-mask rule in recipe4/schema8, off by default: 6-connected 3D cavities or4-connected native voxel-Z slices. Never label voxel slices as RAS axial on oblique/axis-swapped data. No dilation/closing, no silent filling during slider drags. Threshold drafts stay separate from the saved mask; generated previews, Apply, sessions and exports must use the same filled selection. Protect nonfinite/outside-crop voxels. Keep the bounded32MiB span stack, cancellation during chunked mask-only thresholding and worker termination/atomic commits. Do not copy a full float volume into a worker merely to generate a binary mask. Serialize working-grid masks/images directly; identity interpolation can corrupt exact0/1 masks on oblique grids. Run test:threshold-mask (independent reference/topology, real workers, generated preview, original immutability, saved-vs-draft, failure/cancel, replay/export), test:guided, session/native-export and full packaged release checks.

Guide drag regression contract: guide-manual-floating overrides every dock CSS rule, including narrow layouts. Preserve this until Follow/new steps; clean up on close/reset. Use the visible dock rectangle when beginning a drag or keyboard move, never stale inline positions. Preserve pointer identity, capture cancellation and the existing Back/close/preference listeners. Test crossing the sidebar/image boundary, refresh/scroll/resize stability, both dock modes, keyboard escape, Follow and new-step automatic restoration in test:guided plus exact compiled UI. Keep physical ROI/image/fit state unchanged.

Manual affine regression contract (schema9): use separate MRI/CT controls independent from export direction. Canonical Fnew = Dmoving * Fbase * inverse(Dfixed). For spline F(x)=D G(A P x), retain original A/control snapshots and coefficients; P is a proper input affine, updated by right composition. Never flatten a spline or discard precomposition when reversing preview/export. Validate serialized controls against matrix and inputAffine. 12 controls are translations/rotations/log positive scales/upper-triangular shears with reference-center pivots. Target changes alone never alter F. Transformed source rows share overlay arrays, have explicit reference-grid labels, map navigation/markers using the displayed fit snapshot, and restore native cameras/images before all source editing. Preview must not mutate raw arrays, affines, landmarks, masks, ROI bounds or export direction. Keep modules separate; never replace a broad main.ts range spanning unrelated viewer/window setup. Run test:manual-affine, exact compiled UI and all package verifiers plus full regression checks.

Guide/export contract for0.16.67: processing stages require explicit Continue; individual edits do not imply completion. Registration completion prompts users to close the current panel before returning to the next-step hub; Inspect teaches users to open Alignment display themselves. Parameter changes must not reopen Landmark alignment after completion. New landmarks/manual controls remain user-completed. Opacity animation must use shader opacity only and stop on manual input/hide/reset/close. Export data source, FoV and resolution are independent; never imply native processed masks are regenerated at native detail. Project saved mask/edit footprints in physical space, preserve original arrays, exclude registration sampling masks from intensity export, and reject moving-FoV spline export. Exact output geometry/options accompany aligned images and transform metadata. Keep native default behavior for legacy API calls. Run test:guide-export-inspection against actual packaged export workers plus all protected window/guide/export/coordinate/processing tests. Build only Apple Silicon and use numeric version names until the user requests all platforms.

Pending proposal recovery contract: a guide that instructs Accept/Reject must point to working canonical decision controls in the main interface, then offer manual adjustment after an explicit user decision; it must not execute that decision itself. The Manual panel must reveal the same decision actions above its disabled controls. Delegate through actual local/global/spline review handlers exactly once; never treat workflow selection or nudges as implicit acceptance. Reject without a prior alignment must lead to an enabled image-geometry initializer. Guide refresh keys must include proposal state. Landmark collection uses the same Add-pair target before and after its mathematical minimum; no primary Apply promotion or threshold-triggered opening of the alignment panel. Five-point advice is a practical rigid starting set, never a fit guarantee. Run test:guided-decisions and the compiled session-restore/pending-decision/manual-nudge regression, plus existing guide, registration, manual and packaged checks.

Manual initializer contract: Begin manual alignment is only for an empty alignment state. Hide its wrapper and disable its button whenever a fit/proposal exists. The legacy use-image-geometry handler must refuse to replace an existing fit; workflow selection must never reset it. Existing manual transforms are edited directly. After rejecting a first proposal without a previous fit, reveal the initializer again. Keep image-geometry as internal provenance for backward compatibility. Verify initial visibility, post-initialization hiding, legacy-handler nonreplacement, and pending accept/reject recovery in compiled/manual/guide regressions.

Audit regression contract: raw fallback pixDims must be selected-affine column norms. Zero interpolation weights must not propagate nonfinite neighbors; preserve nonfinite values when their weight contributes. Physical crop membership must be checked even with unchanged grid dimensions. Every worker postMessage failure must terminate and detach cancellation listeners. Reset initial alignment restores matrix AND original model/count/diagnostics; carry landmarkModel through local refinement and session validation. Never delete a backup after a failed restore or roll back an installed file because backup cleanup failed; report its recoverable path. Run test:audit plus all affected scientific/session/processing/storage and full packaged checks.

Voxel brush contract: pointer events stage sparse edits only. Never invoke full-volume workers, GL volume uploads, registration or raw-array mutation per click/stamp. Use bounded physical-slice index previews, voxel-grid spherical radius and crop clipping; preserve pointer identity and dispose drag listeners. Apply/Finish performs one existing preprocessing transaction, retaining failure/cancel drafts. Button navigation/save replays exactly once after success, never after failure; file selectors require explicit Apply/Discard. Undo/Redo uses stroke deltas while draft exists, batch history after commit. Cap is total20,000 edited voxels per image, not per brush batch. Preserve original arrays, fit/ROI state, schema compatibility and mask semantics. Run test:voxel-brush with emitted workers in each package verifier, existing initial-processing/threshold/session/guide/production and full release validation. Do not claim native graphical responsiveness without a graphical run.

Mask editing contract (recipe5/schema10): explicit MRI/CT image vs MRI/CT binary-mask targets live in Voxel editing; radius is a whole-number input0–16. Never encode binary corrections as source intensity edits or registration exclusions. Generated BinaryMaskRule.edits and applied thresholdFill.edits are independent snapshots. Validate0/1, unique original-grid indices and20,000 cap; apply eligible corrections AFTER filling, protecting nonfinite/crop voxels. Metadata-only edits must not reload source viewers or invalidate overlays. Saved preview, changed-threshold draft preview, Apply, native replay and exact binary exports share corrections. Create/update retains them; Clear removes saved and draft corrections. Newer-schema rejection prevents silent loss in older apps. Run expanded test:voxel-brush, threshold/initial-processing/session/nonlinear/guide/production and protected window suites, full release validation and package worker proofs.

Welcome/pairing/hover contract: keep internal mri/ct slots and matrix directions stable; use validated registrationPairing labels and schema11 metadata for human modality identity. Never globally rewrite filenames, dynamic user strings or data arrays. Startup welcome and profile selection must not expand tool panels, start registration or change search settings; selected inputs lock welcome choices. Hover uses protected physical projection and actual working values, shows only the active panel in edit mode, distinguishes mask/pending values, and never starts workers/uploads/crosshair changes. Recipe6 final image paint overrides prior mask fill; explicit later Mask Apply demotes those markers before replay; binary strokes never receive them. Preserve legacy recipe semantics, crop protection and native export order. Run test:welcome-hover, actual compiled pairing/session/export tests and brush/native/threshold/session/nonlinear/window/guide regressions plus full packaged verification.

Data-source selection: preserve synchronous file-picker activation and never route source choice into registration. Fixed-action desktop IPC must validate sender and main frame. Connection replacement must retain explicit save-state confirmation, cancellation and rollback; mounted shares remain local file selections. Run test:data-sources, actual compiled chooser/guide tests, desktop/preload/SSH/runtime/lifecycle tests and packaged source proof before shipping.

Direct Network loading must use networkFileBrowser + token-guarded networkBrowser routes, not desktop switchTo/AppleScript. Preserve workspace on connect/browse/cancel. Verify real SSH host-key rejection and exact streamed transfer, dependency completeness in ASAR/slim payload, no saved credentials, header-only connection tokens, bounded sessions and cancellation before packaging. Keep Viewer attribution and AGPL source/license notices.

Teaching guide (0.16.76): guide controls change instructions/highlights only. Never call main controls, proposal decisions, fit/reset/save/point actions, or detail.open from the guide. First prompt users to close other sidebar panels, then open each required outer/nested heading, then point to the actual field/control. Native toggles/actions alone advance these stages. Busy progress takes priority. Require explicit main Finish processing tool before leaving an active crop/brush; guide navigation must not trigger draft auto-commit. Keep point placement/minimum neutral, same-pair targets stable and transient navigation out of point-level Back history. Run test:guide-teaching, guided, guided-decisions, guide-export-inspection, registration-workflow, voxel-brush, actual compiled UI and protected window regressions; full validation and packaged proof remain required.

Guide heading visibility: sidebar panels clip overflow, so summary.guided-target must use an inward outline/inset frame. Use a dedicated three-cycle heading animation that stays within this box, and matching-specificity reduced-motion override. Show/Follow may replay the cue but must never toggle a panel or trigger data actions. Test actual heading classes, CSS computed inset/offset, timer cleanup, explicit replay, reduced-motion and retained steady frame.

Guide ordering: queued processing continuations must be cancelled by Back, Restart, Finish and both reset events, but retained on pause/resume. A newly arriving proposal clears stale requested controls and point-history instructions before routing to review; subsequent Show Accept/Reject instructions remain until the decision. Run test:guide-orderings (all sidebar combinations and seeded state walks), test:guide-teaching and guide decision regressions. Keep guide-only actions read-only for data and disclosure state. Do not claim bounded simulation exhausts all possible user sequences.

Process proposal completion before the no-fit fallback: Reject may restore null. Otherwise an earlier Show Accept request can survive because the fallback cleared hadProposal too early. Cover Show Accept followed by a real main Reject on a first-ever proposal.

Startup guide contract: neither the welcome Guide nor toolbar Guide may choose the default pairing, dismiss welcome or load an image. Explicit pairing selection alone opens the empty workspace; successful session/input selection may also proceed. GuideState must receive the real welcome lifecycle state, ahead of viewer readiness/loading/history. Test the welcome+guide+source chooser together from a fresh launch and in the actual compiled frontend, not just individual module fixtures. Cover both entry points, four pairings, cancelled pickers/session restore and pending preparation; keep prerequisite stages out of Back history. Run test:guide-startup plus production-ui, welcome-hover and all guide tests. Docked guidance must reserve welcome space too.

Guide/modal ownership: every .modal not hidden and every open HTML dialog must obscure neither its controls nor content with guide/Resume. Keep the generic CSS visibility/pointer-events guard and modal stacking above the guide; do not return to per-dialog exceptions. Suspend guide projection/position/focus/scroll while dialogs are open and refresh after the last closes. Capture Escape ownership before bubbling handlers hide a modal, or the same key will also pause the guide. Do not auto-resume a guide the user hid. Run test:guide-modals, source/startup tests, actual compiled frontend with its stylesheet, and all existing guide/placement regressions. Include nested/replaced/removed dialogs, both image slots and paused Resume controls.

The guide has no Show Reset workspace shortcut (removed at user request in 0.16.81). Keep Reset workspace in the main toolbar only, with its confirmation. Do not restore a guide proxy/location button for reset. Restart guide preserves workspace data.

Starting-choice hubs and loading prerequisites are not sidebar actions. Never gate choose/registration or welcome/loading/resolution prerequisites behind Close-panel teaching. Restart must immediately present starting choices (or an unmet prerequisite/pending decision), preserve data/panels, and clear history/pending continuation. Only after selecting a task should normal sidebar teaching apply. Test visible steps directly from Close Initial processing and all other Close-panel prompts without an auto-follow helper.

Guide copy supports blank-line paragraph breaks and **bold** emphasis through guideText.ts. Render text nodes and strong elements only; do not interpret guide copy, labels, filenames or errors as HTML. Keep dense options separated without changing stage keys, targets, decisions or registration behavior. Preserve pairing-aware labels and literal HTML text.

Pyramid/range audit contract (0.16.84): preserve zero-weight nonfinite semantics through separable decimation, not only exported interpolation. Precompute index tables without changing Float32 pass boundaries, physical endpoint extents or positive-support invalidity. Never silently discard malformed custom pyramid tokens. Strided percentile samples can miss thin masked structures: use the bounded included-voxel fallback rather than declaring a nonempty mask empty. Run test:pyramid-audit, scientific/automatic/processing and full packaged validation; report CPU microbenchmarks separately from native GUI performance.

Landmark list presentation lives in landmarkList.ts; unchanged state refreshes must retain DOM/focus/guide identity. Event callbacks must resolve current model by ID after restore/Undo, even if identical state reuses DOM. Run test:landmark-list, guide rebuilt/delayed-target cases and exact compiled UI. One-shot preparation/nonlinear transport lives in core-client/workerTask.ts and must settle once, terminate and detach listeners on result, abort, constructor/clone/callback/protocol/message-decode errors. Run test:worker-task and affected emitted-worker tests.


JIP integration (0.17.0): never treat the JIP pull field as a landmark spline or an affine alone. Use createPointMapping for F=D K^-1(A P x), with K(y)=y+v(y) in fixed RAS mm. Sessions validate finite field size and positive voxel-stencil Jacobians; these do not prove continuous topology. Keep the existing landmark spline bound unchanged. Preserve native zero-residual extension and report failed forward inversion. JIP 0.8.3 supports physical windows/exclusion masks with sampling-v1. Require and confirm this capability for constrained requests; older companions must refuse constrained fits. Never silently clear constraints or encode them as zero intensities. Domain IPC runs one fresh private worker, accepts no renderer paths/commands and cleans on cancel/error/close. Optional companion is discovered beside Align or explicitly chosen, not embedded into core desktop. Run test:jip, test:jip-service and existing nonlinear/manual/session/export/guide/worker regressions.

JIP efficiency contract (0.17.1): direct native result access must match the standalone field export exactly and must not write unused aligned/checkpoint/Jacobian files for Align. Binary IPC accepts only residual.f32 inside the private job directory with exact dimensions, encoding, byte count and SHA-256; return values only, never worker paths. Older base64 bridge results and schema-12 sessions remain readable. Displacement arrays are immutable and shared across fit snapshots; clone metadata and proposal history separately. Session/export boundaries encode base64 explicitly; never JSON-stringify a live typed field as numeric object properties. New scratch must remain bounded and context-owned, not leak trial arena blocks. Preserve complete candidate ordering/arithmetic, epoch invalidation, warm GPU correctness/fallback, cancellation and tracked ownership. Profile counters are separate from checkpoint/numerical ABI and report inclusive timing scope. Run test:jip-efficiency, test:jip, service/IPC and full scientific/session/manual/export/worker tests; compare six frozen native bridge fields and both exported native libraries.

Native scratch failure recovery: all three vector lengths must be rechecked after a partial allocation failure. Do not use job-vector length as a proxy for output/index storage. Keep failure injection at every growth step and verify successful kernel execution after retry; no oversized/null buffer may cross the C++/C boundary.

JIP regions/preview contract (0.17.2): rectangles retain their axes; moving windows gate continuous physical RAS pull points, independently of nearest original-working-grid exclusion masks. Fixed windows gate working sample centers. Never use a slice depth as a third-axis bound or convert excluded voxels to intensity zero. IPC requires sampling-v1 and a confirmed result; preserve older unconstrained companions. Native masks are immutable borrowed inputs to trials; mutable membership scratch must update clone/release/ownership together. Sampled checkpoints use JIPCP004; no-mask writes retain003. At least64 usable varying fixed samples are required. Run test:jip-sampling, native engine-sampling, all physical window/processing/session/nonlinear and packaged tests.

Manual display deltas may update only compatible cached pixels; exports and settled overlays must resample ORIGINAL data with the full map. Never change raw/native/header arrays or persist transient previews. Preserve both targets/directions and reject a changed opposite-image adjustment/base warp. Keep one display update per frame, full rebuild after450ms pause, source-preview camera state and old-result guards. Run test:manual-preview, test:manual-affine and exact compiled UI; do not claim native GPU responsiveness from DOM tests.

#0.17.3 regression boundaries

Preserve the protected contracts at the start of Documentation/TECHNICAL_FINDINGS.md. Canonical full-field JSON and intent1006 RAS vectors must pass independent SimpleITK reading. Landmark leave-one-out is on-demand, count-gated and labelled sensitivity, never target accuracy. ITK is a distinct CPU route with explicit engine provenance and no JIP checkpoint; reject unsupported moving windows/shear and require dense working-grid sampling for fixed windows. Acquisition metadata does not infer an EPI axis from T1/T2 labels or silently change settings. Keep original JIP native math/defaults and optional GPU path. Worker overlays must cancel superseded tasks and reuse unchanged fixed inputs without mutating/transferring originals. Run full release checks and exact packaged worker/ASAR tests after changes.

# Embedded registration contract (0.17.5)

The user explicitly requires nonlinear registration incorporated into Align. Ship one Align.app with scientific-runtime inside its own Resources. NEVER require, discover, select, download, or launch a standalone JIP Registration.app. Never restore jip-runtime.json or add companion-selection UI. Main-process IPC resolves only the app-owned worker and requires its private runtime identity. Keep Python/engine overrides out of worker environments. Scientific source, native CPU/Metal/CUDA, vendor code, tests and licenses are under scientific/. No Qt, PyQt or standalone JIP GUI modules belong in the worker. Preserve original numerical code and full-map contracts. The existing350MB budget now applies to core Electron/resources; the explicitly incorporated scientific runtime has a separate300MB budget, total650MB, measured at release. macOS14 is required by the incorporated scientific libraries. Run the embedded source/frozen comparisons, real packaged JIP/ITK IPC in an isolated directory without sibling apps, full app suite, exact-ASAR/runtime/signature checks and complete release checksums. Standalone JIP remains an independent project, never an Align deliverable.

Align JIP workflow correction (0.17.5): JIP is NONLINEAR ONLY in Align. Require an accepted linear base, preserve its direction and pass that exact matrix with stages=nonlinear. Never expose standalone JIP linear/model/affine-metric controls or repeat its linear search. Align owns linear registration. Standalone JIP retains its independent full workflow. Enforce this at main-process service and test no-base rejection plus exact starting matrix.

Prepared-image linear registration (0.17.6): keep the existing broad/legacy path numerically unchanged by default. Nearby mode starts from original physical image coordinates, independently of any accepted registration, with bounded rotations <=30 degrees; do not confuse this global independent route with landmark refinement. Prepared settings are an explicit main-program action that sets nearby, nonzero fixed foreground and mean-normalized MI; it never runs or accepts a fit. Stage translation, rigid, axis scale and all selected parameters using full finest-registration geometry and denser default sampling. Foreground gates ORIGINAL working-grid sample centers even when pyramid smoothing makes zero background nonzero; AND it with existing physical windows and exclusion masks. Never drop constraints when adapting a sparse coarse level. Mean/common-range normalization affects histogram scoring only and accepts MI/NMI, never working/source/display/export intensities. Record foreground, normalization, search mode and sampling in sessions/exports. Legacy sessions omit these fields. No new standalone JIP app, duplicated JIP linear workflow or changed native scientific library. Run foreground-nearby synthetic recovery, exact emitted worker/profile/guide tests, real pair both models, all protected coordinate/region/processing/session/export regressions and the full packaged release checks. Private subject data stay outside source/release ZIPs.


# Consolidated linear / compact runtime contract (0.17.7)

All parameterized image-based linear registration belongs to Linear registration. Never add backend-specific linear/model/stage selectors to a nonlinear panel. Both JIP and ITK in Align are nonlinear only and require an accepted proper linear base; enforce before IPC dispatch. ITK must return that exact accepted matrix/baseAffine with a nonempty dense field; refuse matrix changes or affine-only responses. Old saved ITK fits remain readable. Keep software brands out of linear model selection; verified settings presets may set shared controls explicitly, never promise another program's exact optimizer from similar settings. Preserve broad and prepared-image linear algorithms/defaults.

Use scientific/native-itk for native B-spline and N4 only. Ship neither the SimpleITK wheel nor an external ITK installation. The pinned upstream ITK source and nlohmann headers/licenses are included for offline reconstruction. Build ITK statically for the declared OS minimum, not against Homebrew dylibs requiring newer macOS. Keep only system dynamic dependencies in the native adapter, immutable arrays, bounded masks/fields and real cancellation. The thin Python/NumPy NIfTI/JIP adapter remains deliberately; do not describe the entire runtime as Python-free. Scientific runtime <=90MB; total arm64 app <=350MB. These tighter limits supersede the 0.17.5 size allowance. Run actual native ITK/N4 tests, source/frozen fields and N4, original JIP native tests, all app suites and isolated packaged IPC/ASAR/signature/size checks.


# Linear defaults contract (0.17.8)

The user explicitly requires AFNI/FSL/ANTs/JIP/Elastix defaults in the ONE shared Linear registration workflow. Do not omit profiles or introduce separate linear engines. Each profile names a concrete command/API, reviewed source and mapping revision. Source-default differences must remain visible; do not substitute NMI for AFNI Hellinger or omit JIP50/FSL256 bins. Selection previews, Apply writes shared settings only, optional Keep model preserves custom groups. Never run/accept/reset or change regions/data/local refinement/disclosures. Fields stay editable and show modified state. Snapshot profile provenance before worker execution and validate/serialize it in sessions/exports without auto-applying imported metadata. Bound fractions/reservoirs and preserve physical axes/exclusions. Preserve original launch defaults and exact five-model golden worker results when new options are unused. Test actual compiled frontend and emitted workers, guide/region/session/export boundaries, full suite and release artifacts.


# Slice renderer contract (0.17.9)

Never delegate raw wheel zoom to Niivue's uncapped per-event multiplier. viewInteraction consumes the event once and viewZoom normalizes distance, keeps the physical crosshair anchored and bounds the current camera. All six source and three review panes install sliceCrosshairs: 2D lines are clipped and constant CSS-pixel width regardless of camera zoom or voxel spacing. Keep the adapter instance-local, preserve GL state, and delegate other 3D drawing. No image/affine/registration mutation belongs in either viewer module. Run test:viewer-rendering (actual dependency draw/geometry methods, not a fake Niivue), protected coordinate/window/processing/guide tests, the full suite and exact packaged verification. Command recording is not native GPU certification.

# Image comparison contract (0.17.10)

Flip inspection is launch-local and separate from saved opacity/color mode. Pure-image branches use reserved overlayAlphaShader endpoints(-1 fixed/2 moving); smooth opacity stays clamped0..1. Moving-only empty background must be black, never fixed anatomy. No viewer arrays/physical mappings/cameras/landmarks/regions or export values may change. Keep one RAF loop; redraw flips only at1-second boundaries, stop on manual/mode/hidden/page/reset/clear, and reapply flip after colormap/preference/overlay rebuilds. Pending smooth-opacity RAF callbacks must recheck the comparison mode. Supported shaders must not rebuild textures per flip; retain honest fallback behavior and restore fixed opacity when returning to blend. Pair labels follow actual fixed/moving modalities, including reverse/manual-target overlays. Run test:flip-inspection, workflow, guide-export-inspection, settings, viewer-rendering, compiled UI, all protected region tests and full packaged checks. Shader-command tests are not native GPU proof.

# Public source / CI contract

This export adds source publication without changing UI or numerical implementations.
Keep generated native runtimes, compiler caches, node_modules, installers and subject
data out of git. Retain pinned vendor sources/licenses and immutable reference
fixtures. The compressed reference trace bytes must decompress exactly; never
regenerate them from the candidate to pass. Source tests use fileURLToPath for
Windows/space-containing paths and real HTTP complete-dist browser fixtures.
The required native-dependent test group runs after compilation, never counts
missing workers as a pass. Only the final publish job has contents:write and it
depends on all four native packages, browser checks and hash/commit/platform
aggregation. Manual runs never publish. Keep architecture-matched private runtime
inside each app; never ship a renderer-only fallback or external JIP companion.
The source zip is git archive of the exact commit. Preserve the one-source VERSION
identity and tag equality. Record native platform/GPU/installer limits honestly.
Run test:publication, actionlint, CI core/native tests and packaged proof after
changing this build/release setup. Do not retrofit these changes into the original
working-app folder unless the user explicitly requests that migration.
