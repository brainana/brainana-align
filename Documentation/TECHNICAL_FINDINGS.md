# 0.17.10 · flip inspection audit

The existing animation varies moving opacity between25% and75%; it never displays a pure image. Added a separate launch-local comparison selector and fixed/moving inspection state. Reserved values -1/2 in the existing per-view overlayAlphaShader select pure fixed/moving shader branches; ordinary opacity remains clamped0..1. The moving-only branch renders opaque black for empty moving samples rather than revealing fixed anatomy. Fixed/moving textures, camera/slice state, physical transform and source arrays remain untouched. The supported shader path only changes a uniform and redraws; the existing shader-rejection fallback changes volume opacities and uploads, then restores fixed opacity when returning to blend.

One RAF loop owns smooth or one-second flip playback; flips redraw only at interval boundaries. Switching modes stops playback, and pending normal-opacity RAF work rechecks the comparison mode to avoid replacing a flip frame. Display preference changes and rebuilt overlays reapply the current flip selection. Flip playback never changes the slider value, so saved smooth opacity is retained. Native GPU pixels and perceived interaction speed remain unverified in this environment.

# 0.17.9 · solid cyan pane investigation

The reported fill matches MRI crosshair color. A deterministic reproduction uses the pinned Niivue 0.69.0 wheel handler: it multiplies zoom for each wheel event regardless of magnitude and does not cap zoom. One hundred -0.01-pixel events yield 13,743×. Orthogonal crosshairs use 3D cylinders whose physical radius is magnified by that zoom; the original opaque geometry covers every one of 400 sampled screen locations. This reproduces a matching rendering failure, but the screenshot alone does not establish the user's exact wheel history.

The fix has two independent boundaries: distance-based bounded camera input, and constant pixel-width 2D crosshairs that cannot become a full-pane cylinder even with a previously corrupted zoom. Both are isolated from registration code. The adapter uses physical RAS coordinates and Niivue tile metadata; it clips line endpoints, preserves Retina scaling, honors color/visibility/thickness and restores GL state. Do not restore raw per-event vendor zoom or zoom-scaled 3D crosshairs for orthogonal panes. Run test:viewer-rendering, protected window/coordinate tests, full release checks and exact package verification. Native Chrome startup aborted here, so native GPU/pixel verification remains pending.

## Earlier release documentation

# 0.17.8 · defaults and scoring contracts

Profile facts, sources and mappings are reviewed together. AFNI uses a Hellinger dependence objective; never silently replace it with NMI. JIP requires a 50-bin option. Fractional sampling runs once over included fixed points, preserves physical window axes and exclusions, and uses a deterministic reservoir above the separate 250,000-point cap. Record requested fraction, included support and cap status. Percentile clipping and normalization affect scoring only. Geometric and positive-intensity centers are physical coordinates; existing-coordinate initialization remains independent of any accepted transform. Coarse sparse support may use finer samples without dropping constraints.

Preset application never runs a worker, accepts/rejects a proposal, changes images, resets regions, opens disclosures or touches local refinement. Changing the selected preview cannot relabel a previously applied profile. Preserve request-time provenance if a field changes while the worker runs. Golden comparisons protect the default five-model behavior.

All profiles populate the same editable Linear registration controls; no AFNI/FSL/ANTs/JIP/elastix linear executable or second engine is included. Profiles map supported settings to Align and display source-specific optimizer, masking, sampling and stopping differences. They do not claim identical results to those programs. Native nonlinear methods and the compact runtime remain unchanged.

## Earlier release documentation

# 0.17.7 linear consolidation and size correction

The 0.17.6 installed app measured 493,116,515 regular-file bytes: 251,507,345 core and 241,609,170 scientific runtime. The SimpleITK binding alone was 177,536,528 bytes; native JIP libraries totalled only 2,009,808. This was a packaging overhead problem. The replacement statically links only native ITK modules required by B-spline/N4 and exposes a small array-based ABI; NumPy/Python remain a deliberate compatibility layer. Release size is measured from the final app and recorded in Validation/REPORT.json.

The former ITK stage/model controls duplicated Linear registration and are removed. Main-process checks refuse affine/both requests before spawn and require a starting matrix for both nonlinear methods. ITK proposals must include a full field and retain the accepted affine exactly. No software defaults equivalence is claimed. The current broad and prepared-image linear math is unchanged.

## Earlier release documentation

# 0.17.6 · foreground-aware linear fitting for prepared images

In Image-based registration → Linear registration, click **Use prepared-image settings**, then choose the transform model and **Run linear registration**. The explicit preset selects nearby staged fitting, nonzero fixed-image foreground, mean-normalized mutual information and bounded rotation. Review and accept/reject the proposal normally. Rigid/similarity/anisotropic/affine/custom remain available. The unchanged broad orientation search, all-included-voxel matching and legacy percentile histogram ranges remain the launch defaults for compatibility.

Nearby fitting starts from the original images' physical coordinates. It does not use a previous accepted transform and is distinct from landmark refinement. Translations, rigid pose, axis scales and all enabled parameters are introduced in stages at finest registration resolution; Standard sampling uses a 60,000 sample target. Rotation limits are at most 30 degrees. Use broad search or establish an initial alignment when images are far apart. Switching back to broad restores its rotation range and pyramid controls.

Foreground uses nonzero ORIGINAL working voxels in the fixed image, intersected with existing physical regions and exclusions. It is intended for a skull-stripped/zero-background reference; valid zero-valued tissues can be excluded, so choose the policy deliberately. Sparse coarse foreground sampling uses finer samples without dropping any constraints. Mean-normalized common-range histograms apply only to MI/NMI. These are scoring operations, not N4 correction, image masking or intensity editing; source/working arrays, display and export intensities are unchanged. Session/export provenance records the selected policies; old sessions remain readable.

The provided anatomy/EPI pair is checked through real packaged loading, preparation and workers with 6- and 12-parameter models. Evidence distinguishes qualitative overlay improvement from ground-truth accuracy; the implementation is not claimed to reproduce native JIP numerics exactly. Optional JIP nonlinear and ITK routes and their private numerical runtime are unchanged (worker version 0.17.5). This candidate contains one self-contained Apple Silicon Align.app, complete source, consolidated documentation, validation, manifests and SHA-256 checksums. Native graphical/GPU hardware checks and notarization remain pending.

## Earlier release notes

# 0.17.5 integration correction

The earlier companion design and duplicated JIP linear workflow did not satisfy the requested integration. This release supersedes those deployment instructions: Align owns the source/runtime, ships one app and exposes JIP nonlinear fitting only after an accepted linear alignment. Main-process preflight rejects missing starts or JIP linear stages. Tests assert exact starting matrix, no duplicated controls, no action on disclosure, pending-proposal gating, cancellation and unchanged original buffers. Standalone JIP is preserved separately.

Frozen/source numerical comparison covers both directions and both engines; the scientific-library qualification also exercises its retained internal linear functionality, which is not exposed as an Align JIP action. Full-map transport, physical coordinate and sampling contracts remain protected. The runtime audit compares all frozen registration modules against source and checks native dependency closure, arm64 architecture and absence of Qt/standalone modules.

## Historical release notes (superseded where deployment differs)

# 0.17.4 method disclosure contract

Keep #linear-registration, #jip-registration and #itk-registration as independent peer details. Do not conflate the Transform model dropdown with a registration method. Shared optimization regions and acquisition metadata must remain outside method-specific settings. Native disclosure changes cannot run registration, change the selected model or expand a sibling. The guide must teach opening a nested method heading before its hidden controls. Retain all existing control IDs and physical-state/cancellation/export contracts.


## Previous release documentation

# 0.17.3 protected contracts

- Canonical nonlinear JSON must convert every field typed array to a real JSON array in browser and native export; never JSON.stringify a live dense field directly.
- Displacement NIfTI uses intent 1006, dimensions[x,y,z,1,3], planar RAS-mm components and explicit mapping direction. The tested SimpleITK reader converts both geometry and vectors automatically to LPS; never add an unconditional second flip.
- Counts6/6/8/8/20 gate landmark sensitivity only. Invalid/ill-conditioned omission subsets must be reported. Low refit error is not independent target accuracy; never enable heavy refits automatically at the fitting minimum.
- Physical windows still constrain only their two stored RAS axes. ITK fixed-window requests require dense native working-grid metric sampling (one pyramid level, fraction1). Moving physical windows and sheared voxel grids are refused explicitly, not silently approximated. Exclusion masks retain nearest-voxel semantics.
- The ITK residual transport shares dense-map storage with JIP, but settings.engine and execution.engine must confirm ITK. Do not fabricate JIP checkpoints/Continue, label the result JIP, or discard the full field on save/manual/export.
- EPI phase axes are native NIfTI i/j/k columns mapped through physical geometry; T1/T2 are not readout types. Only explicit user confirmation changes JIP axes or metric/init settings. Readout timing is provenance, not a correction field.
- Worker resampling must not transfer live source buffers or publish stale results. Keep latest-request cancellation, retained prior display, retry, and explicit busy/proposal/overlay phases. Test emitted production workers and packaged ASAR, not only pure helper copies.


## Historical documentation

## 0.17.2 — registration regions and interactive manual preview

Exact interpolation memoization also reuses common width1/2 Lanczos coefficients, keyed by the complete float coordinate and width. There is no quantization or approximate kernel table. Each context has a bounded256-entry cache per axis. Reuse is restricted to the serial CPU path on frozen displacement axes during local nonlinear probes; full-grid passes, changing axes and smoothing/large kernels retain the original calculation.

Includes JIP Registration 0.8.3 for Apple Silicon. Keep the two apps together. The JIP route now honors the shared optimization regions and registration exclusion masks; it requires a companion advertising sampling-v1 when either is active. Older companions remain usable for unconstrained runs. No constraints are silently cleared.

In **Image-based registration**, define the optional regions above the JIP controls. Each rectangle restricts only its two physical RAS axes, regardless of slice depth. The fixed window gates fixed sample centers. Moving windows follow the current continuous pull coordinates; moving exclusions use the nearest working voxel. Existing nonzero-fixed matching, interpolation taps, basis hierarchy, regularization and sequential optimization remain unchanged. Regions constrain the objective, not the spatial extent of the fitted deformation. They are not an output crop. At least 64 usable, varying fixed samples are required.

Manual nudges now move a cached display image immediately, coalesced once per display frame. After a 450 ms pause, the complete overlay is rebuilt from the original working image with the complete mapping. The temporary preview can show the cached field of view and interpolation; the label identifies it until full detail is ready. Only a compatible change to the displayed image uses this path; target/base changes fall back to a complete rebuild. Export never uses preview pixels. MRI/CT target selection, nonlinear direction, processing, native files and physical windows remain independent.

JIP caches exact centered coordinates for bounded diagonal internal grids, retaining the original conversion for general or oversized grids. This is an engineering optimization, not a new registration algorithm. Metal/CUDA options and CPU fallback remain. Native graphics and GPU speed still require checking in a normal desktop session.


## Registration efficiency (0.17.1)

Align 0.17.1 + JIP 0.8.2 preserve the current nonlinear optimizer, arithmetic, defaults and explicit proposal decisions. This is the first engineering group from the October23 audit; it does not add a replacement nonlinear engine.

Implemented: exact native field access without unused image/checkpoint/Jacobian exports; checksummed bounded binary IPC; shared immutable fields and validated decodes across live/older-session snapshots; explicit portable JSON encoding at persistence boundaries; reusable context-owned scratch; larger independent sample batches; common serial coefficient stack scratch; warm GPU performance trials; inclusive phase profiling; incremental progress reading. The desktop packaging command now refreshes its slim workstation payload after the frontend build, preventing stale secondary assets.

### Real-data measurements

Three interleaved measured repeats per route followed one warmup per route, using the saved N4-prepared baby4 EPI/anatomical inputs and accepted linear initialization. Each fit used a fresh process. The timing includes process startup, nonlinear registration, diagnostics and result files; it excludes preceding N4/linear estimation and Align rendering. No competing validation/build jobs were running during the final comparison. An earlier contended exploratory run was excluded.

| Route | Previous 0.8.1 | Current 0.8.2 | Wall-time reduction | Sampled peak worker RSS, previous → current |
|---|---:|---:|---:|---:|
| CPU | 10.848s | 10.571s | 2.56% | 161.98 → 142.28 MiB |
| 4 CPU threads | 11.071s | 11.021s | 0.45% | 156.42 → 142.89 MiB |

Peak worker RSS was approximately9–12% lower in the final run. CPU time improved about2.6%; threaded time was essentially unchanged (about0.4% lower), so no reliable threaded speedup is claimed. This is sampled worker memory, not whole-app/GPU peak memory or a guarantee for every input. Matrices, complete fields, field geometry and diagnostics matched exactly in all16 warmup/measured comparisons. Approximately25.13 MB of adapter result/intermediate files became4.82 MB, about81% less. The threaded measured tile-call count fell from50,275 to39,006. CPU remains the default and was still slightly faster than four threads on this small workload. Gains are modest and specific to this case; no GPU speedup was measured.

### Verification and limits

- Native CTest:15/15 passed. Allocation recovery is tested after each scratch-vector growth, followed by retry and kernel execution. Coverage also includes frozen search/resampler references, both arithmetic engines, checkpoint/CLI, fault rollback, lifecycle and backend/transaction checks.
- JIP frontend:56 tests passed; added tests compare six frozen0.8.1 bridge cases in both directions, direct/exported fields and diagnostics, and cancel/error behavior.
- Align:67 suites passed; two real-browser suites blocked by missing configured browser executables. After the final older-session decode reuse change, affected JIP/manual/session/nonlinear-worker/export tests passed again; the exact final packaged frontend/workers passed the complete package verifier.
- 100 complete isolated nonlinear jobs: every field exact and every child exited. Parent RSS after16 warmups grew 0.3906 MiB; original span/growth/slope limits passed unchanged.
- 18 frozen Python modules match source; both frozen native engine code/constant sections match current builds. All256 Mach-O dependencies passed the bundled-runtime/minimum-macOS14 audit.
- Packaged headless jobs, compatibility/reproducible continuation, CPU/threaded benchmark traces/exports and offscreen startup passed. The frozen real-data GUI/CLI workflow passed preparation, both N4 corrections, linear, nonlinear reject/retry/accept, native export and session restore; its source session was unchanged.
- Native Cocoa launch remains blocked by desktop services in this agent; Metal/CUDA devices remain unavailable here. Do not interpret offscreen tests as native interactive/GPU certification.
- This candidate packages Apple Silicon only. Core Align requires macOS13.5+, JIP14+. Signatures are local ad hoc, not Developer ID notarization or proof of installation on a second computer.

### Remaining audit candidates

The first release implements audit items1–5 in part/full: profiling, binary fields, result-only bridging, scratch reuse and batching. It also reduces snapshot work under item6. Larger improvements still require separate work: visible-slice/shader preview; further geometry/basis/histogram caching guided by the new timings; native JIP physical regions/exclusion masks; prepared-input/N4 cache and Align N4 recipe; optional block-matching/local-correlation/MIND/ITK comparators. Greedy/NiftyReg/ANTs/FireANTs engines, EPI acquisition-aware methods and learned registration remain optional research candidates. They are not silently introduced into this release.

Current JIP limits remain2 million working voxels/input,8+ per axis, proper starting geometry, and explicit refusal of optimization windows/exclusion masks. Existing Align searches continue to support them. Positive sampled Jacobians remain diagnostics, not continuous invertibility proof. Preserve the original nonlinear approach while evaluating future additions independently.


## Previous audit findings

## 0.17.0 JIP integration

See [README.md#jip-registration-in-align-0170](README.md#jip-registration-in-align-0170) for method, physical coordinates, memory diagnosis and limits. Core desktop supplies an optional domain-action installer; executable selection, private NIfTI transport and worker lifecycle live in apps/align/desktop. The optional companion avoids duplicating its scientific runtime inside the core Align app.

## 0.16.84 — Deep audit findings and remaining priorities

Scope: current readable app and imaging packages; latest Viewer architecture/UI/runtime baseline; existing coordinate, registration, nonlinear, processing, export/session, guide/modal and desktop/network regressions; new adversarial sampling/worker/DOM cases; deterministic CPU timing. No clinical accuracy claim or proof of absence of bugs is possible from this audit.

### Findings fixed

| Priority | Finding | Fix and evidence |
|---|---|---|
| High | Registration pyramid interpolation read an unused neighbor and evaluated 0×NaN/Infinity, invalidating an exact sample. | Bypass zero-weight neighbor; positive-weight invalid values remain invalid. Tests cover each axis, nonfinite type and smoothing support. |
| High | Masked intensity-range sampling can miss every valid voxel through flat-index stride aliasing, before registration sampling even starts. | If masked range support is insufficient, scan included finite voxels into a bounded deterministic reservoir. A varying x=1 plane in a 128×32×32 grid now scores identity correctly; empty/constant masks still reject. |
| Medium | Invalid custom resolution/shrink entries were filtered out, so a typo could execute a different configuration without error. | Reject nonfinite/empty/out-of-range entries explicitly. Ordinary presets and valid custom ordering remain unchanged. |
| Medium | Every general redraw replaced every landmark row and all its listeners, losing focus and invalidating guide anchors. | Separate presentation controller skips unchanged state. Regression checks 1,000 unchanged refreshes, preserved focus, updated labels/residuals and restored-state actions. |
| Medium | Preparation/nonlinear worker progress exceptions or response decoding failures could leave a task pending with retained cancellation ownership. | Shared one-shot transport handles terminal, decoding, creation, clone, callback, abort and crash paths with one cleanup. Other workers retain their existing tested ownership; do not claim all worker paths use this helper. |
| Performance | Pyramid filtering allocated coordinate arrays and repeated clipping/index calculations inside its hottest loop. | Precompute small per-axis index tables; preserve summation order/Float32 boundaries. See timing below. Dead unused smoothing/interpolation helpers removed. |

### Measured performance

Node v24.4.0, macOS arm64, synthetic 144×128×112 Float32 input (2,064,384 voxels), two warmups then seven timed runs; median CPU time:

| Shrink | 0.16.83 baseline | 0.16.84 | Speedup |
|---|---:|---:|---:|
| 2× | 58.170 ms | 23.511 ms | 2.47× |
| 4× | 39.304 ms | 14.706 ms | 2.67× |
| 8× | 32.163 ms | 10.374 ms | 3.10× |

All three finite output SHA-256 hashes match the baseline exactly. Timings are a microbenchmark of pyramid generation, not end-to-end registration speed or native GPU responsiveness. Input arrays remain unchanged; output geometry preserves origin, orientation and endpoint extent. Index tables add O(axis length × kernel length) small scratch; no measured peak-RSS claim is made. Validation/pyramid-performance.json retains timing/output hashes and baseline source hash; scripts/benchmark-pyramid.mts reproduces the current test and accepts a baseline module path.

### Accuracy and physical-state assessment

Existing regression coverage includes 6/7/9/12-parameter all-pair fitting, large/near-180° rotations, degeneracy/reflection, inverse directions, anisotropic/oblique qform/sform data, bounded spline inversion/composition, physical ROI state, mask/crop/brush recipes, atomic preparation/restore, native and processed transformed exports. The audit also runs 320 seeded transform recoveries and the new independent finite-pyramid reference/physical endpoint tests. Synthetic recovery is a mathematical correctness check, not proof that intensity optimization finds the anatomical optimum on every MRI/CT/T1/T2/EPI dataset. Review remains required.

### Remaining work, in priority order

1. Linear overlay resampling still executes synchronously on the renderer and scans the entire fixed grid; repeated manual changes can pause large volumes. Move it to a cancellable preview task with latest-request generation checks and atomic installation; retain the current correct native export path. This release improves pyramid generation, not this separate preview bottleneck.
2. Registration still copies/transfers two working arrays into a worker; memory estimates remain planning ranges rather than measured peak allocations. Persistent workers/shared buffers might reduce repeated copies but increase retained memory and require explicit lifecycle/immutability design. Keep one active pyramid scorer and read-only viewer buffer sharing.
3. Several worker/export controllers retain separate lifecycle implementations. Extend common ownership only with their real cancellation/proposal/streaming tests. More generally, main.ts still combines session restore, fit installation and preview; extract controller boundaries incrementally.
4. Guide routing has broad bounded state-walk and real DOM coverage, but user sequences are unbounded and graphics-dependent positioning needs target-machine checks. Preserve the existing read-only teaching and modal-priority contracts.
5. Theme tokens/fonts and portions of core runtime concepts are duplicated between apps. A versioned shared UI/runtime contract is preferable to periodically copying Viewer's dashboard or replacing registration state with its source-manifest model.
6. Native GPU launch/layout/input latency, real dataset registration quality and peak RAM need profiling on supported target systems. This sandbox's browser launch blocker is recorded separately; passing DOM/worker tests must not be described as native visual verification.

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

Guide-only click handlers must not execute app button.click(), invoke proposal decisions, assign details.open, or call revealControl. Derive an instructional close/open target from live DOM instead; Follow only locates/focuses it. Keep explicit workflow shortcuts’ normal app behavior for actual user clicks. Do not auto-commit sparse voxel drafts when any guided-card button is clicked; ask users to finish via the real preprocessing control. Native window finish must route to current global/local operation without a queued reverse transition. Guide navigation and pending action stages must not pollute logical point-level Back history.

## 0.16.75 — In-app network browsing

Direct network loading (0.16.75) must preserve the workspace. Never route the Network chooser through the old native AppleScript launcher or desktop switchTo. Keep local picker activation synchronous, remote tokens in headers, session/Origin/method guards, connection caps/idle cleanup and streamed-download cancellation. Do not persist passwords/passphrases or bypass host-key checking. Only explicit image load may change an image; connection/browse/cancel must not run registration. The SSH config reader intentionally supports concrete Host/HostName/User/Port entries, not Include/Match/jump-host expansion.

## 0.16.74 — Local and network loading

Data-source invariant (0.16.74): selecting a source must not register, mutate volumes or advance the guide. Local selection must keep file-picker user activation (synchronous input.click with a scoped bypass), and cancellation keeps data/guide state. Remote selection uses existing file transfer/load handlers. New connection switching remains an explicit workspace replacement with the existing shell confirmation and rollback; do not imply it merges datasets across connections. Keep IPC fixed-action and main-frame-only; no arbitrary renderer SSH/path arguments.

# Export proposal and feedback audit — 2026-10-14

## Welcome, pairing and voxel inspection — 0.16.73

registrationPairing defines four validated label profiles while retaining stable internal image slots mri/ct and canonical transform directions. These slots are compatibility identifiers, not a claim that the second image is CT. No matrix, coordinate, default search setting or accuracy assumption changes with pairing. T1–T1 uses T1 ·1 / T1 ·2. The welcome screen appears before a workspace starts, supports guided setup and saved-session selection, and yields when files are selected through existing toolbar routes. Pairing cannot be changed through the welcome selector after inputs are selected; session restore supplies validated pairing metadata. Reset workspace reloads the neutral welcome screen. Future pairing-specific advice may use this identifier, but no specialized EPI distortion correction is implied.

Static chrome/instructions use profile labels, while original source filenames remain unchanged. The static-copy adapter captures only initial app-owned text, excludes welcome options and source-name fields, ignores detached text nodes, and never observes or rewrites later user strings. Runtime labels are produced from image-slot keys; settings and guide format static instructions only. Schema11 stores pairing, old sessions default to MRI–CT, and unknown profiles are rejected. Export metadata explicitly records pairing/internal slots/human spaces; source metadata and geometry remain authoritative.

Voxel inspection uses protected physical slice projection and nearest working-voxel lookup, not display calibration or normalized GPU samples. Only the active source panel displays a pointer-transparent lower-right readout in edit mode. It includes current intensity, selected binary mask value and separately labeled pending corrections. It clears on leaving/stopping, uses cached sparse lookup, and does no processing, registration, full-volume copies, texture uploads or crosshair movement.

Paint originally wrote its value before applied-mask fill, so a masked image could overwrite a newly painted value. Recipe6 final-paint markers now apply after prior fill while retaining legacy semantics for old recipes. Native processed export replays the same order. Explicit Apply generated mask removes final-paint markers before replay so later mask application can mask prior paint; Undo/Redo/session state preserves that ordering. Binary mask strokes never get intensity override markers, and crop boundaries remain protected. Image Erase is a literal0; mask Paint/Erase are1/0.

New test:welcome-hover covers all profiles, duplicate T1 labels, neutral startup, safe filenames, guide labels, schema11 legacy/unknown handling, transform metadata, oblique physical voxel sampling and panel readouts with no workers/data mutation. The emitted production bundle tests welcome choices, session pairing restoration, unchanged filenames and T1/T2 transform export labels. Expanded actual-worker brush tests verify literal image Paint on a masked image, matching native export, explicit later masking and Erase0. Full physical-window, scientific, mask/session and packaged frontend/worker regressions remain required. Native graphical appearance and real-data accuracy remain outside these adapter checks.

Current update (0.16.73): startup now presents a welcome screen explaining preprocessing, registration, review and export, with MRI–CT, T1–T2, T1–T1 and MRI–EPI choices. The choice labels both image slots across controls, directions, guide, preparation, review and export; duplicate T1 volumes are numbered. Pairing is saved in schema11 sessions and export metadata without renaming original files or changing coordinate mathematics/settings. Image Paint uses the entered intensity, Erase writes0, and mask Paint/Erase stay1/0. New recipe6 paint overrides honor the entered value even on a previously masked image; explicit later mask application can mask those painted values again. Editing shows the cursor's working-voxel intensity at the active panel's lower right, plus mask and pending values when relevant. Hover does not move the crosshair, mutate data, upload textures or start a worker. Release platforms: Apple Silicon and Intel macOS, Windows x64 and Ubuntu amd64, plus the separate macOS browser edition. Native graphical certification remains pending.

## Image and mask edit targets — 0.16.72

MRI and CT editing already existed through the crop-area image selector; it was poorly placed for editing. A dedicated four-choice selector now lives inside Voxel editing. Crop retains its own image selector, and modality/history context synchronize explicitly. A pending batch locks target switching until Apply/Discard. Image actions allow arbitrary finite replacement intensity or registration exclusion/inclusion; mask actions expose only0/1. The numeric radius accepts integers0–16 in working voxels and retains spherical-grid geometry; no pointer event starts a worker or changes a shared array.

BinaryMaskRule now optionally stores up to20,000 validated sparse original-working-grid overrides. Recipe5/schema10 prevent old applications silently dropping these edits. Thresholding and optional cavity filling run first; valid manual corrections are applied afterward so erasing inside a filled cavity remains erased. Nonfinite and outside-crop voxels remain protected. Mask-only creation/editing streams threshold selection in chunks and sends only selection bytes/eligible corrections to the worker, without cloning the full float image. The generated rule and already applied thresholdFill rule are independent snapshots: changing a generated mask leaves the working image untouched until explicitly applied again.

Generated previews, changed-threshold previews, direct binary exports, image mask application and native physical replay honor the same saved correction semantics. Create/update retains corrections; Clear removes them from both saved state and the threshold draft. Saved sessions reconstruct selections from the validated original grid. Existing schemas and recipes without corrections remain supported.

The brush regression now exercises both MRI and CT mask targets with actual emitted workers, zero processing during painting, numeric input ownership, binary paint/erase after3D filling, unchanged image buffers, independent already-applied snapshots, actual Mask Apply/Create/Clear UI, native replay, schema10 round trips/tamper rejection, and actual corrected binary NIfTI export. Compiled frontend checks verify the four target options inside Voxel editing and a numeric radius field. Native graphical certification remains pending.

Current update (0.16.72): Voxel editing has its own explicit Edit target selector: MRI image, CT image, MRI binary mask or CT binary mask. Generated-mask targets become available after creating that mask. Image actions and binary Paint1/Erase0 are separate. Mask edits update the generated selection only, never source intensities or an already applied mask snapshot; explicitly Apply generated mask to working image to use the corrected selection on image data. Radius is a numeric integer field from0–16 working voxels;0 selects a single voxel. Click/drag previews remain sparse and worker-free; Apply/Finish commits once. Manual mask corrections run after hole filling and survive regeneration, sessions and binary/native-processed exports. Clear mask removes corrections. Recipe5/schema10 retain compact original-working-grid overrides and reject malformed/nonbinary or out-of-grid entries; older schemas remain readable. Apple Silicon macOS only; native GPU interaction remains unverified here.

## Responsive voxel editing — 0.16.71

The previous pointerdown handler called the full preprocessing apply path for each click. Each edit cloned a full-volume worker payload, reconstructed crop/intensity masks/voxel values, returned a full array, committed a new NVImage and reloaded three source views. The shared activity strip made every click look like a separate processing job.

The new VoxelBrushDraft stores only changed indices and bounded stroke deltas. Pointer drags interpolate voxel centers to avoid gaps; spherical footprints clip to image/crop bounds in the original working grid. Duplicate visits merge, strokes support Undo/Redo, and capacity validation rejects a stamp before mutation. No original or loaded voxel buffer is changed by the draft. A cached bounded physical slice lookup supplies original voxel indices; frame-coalesced 2D orange recoloring uses those indices without GL uploads or worker starts. The same protected oblique/axis-swapped projection and post-draw refresh hooks remain authoritative.

Apply/Finish runs one existing cancellable processing transaction for the complete recipe. This preserves existing masks, hole filling, crop, session and export semantics instead of inventing a second incremental processing implementation. Commit can still pause for a large image; this update removes that cost from every click/stroke. Full-volume allocation at commit is unchanged. All three source views update after commit; any registration overlay is invalidated through the existing preprocessing contract, without starting registration.

Button navigation and saving apply a pending batch before replaying the requested button exactly once; failure/cancel prevents replay and retains edits. File-picker labels are not replayed asynchronously because browser user activation may expire; apply/discard before selecting files. Unapplied drafts are launch-local and are not part of automatic recovery snapshots. Applied edits retain schema9/recipe1–4 compatibility and are saved as ordinary sparse voxel edits.

New test:voxel-brush checks independent spherical footprints at centers/corners, crop clipping including an oblique affine, overlapping-stroke Undo/Redo, finite-value/size limits and atomic capacity failure. Real DOM pointer events check unrelated pointer releases, interpolated drag strokes, visible bounded preview, zero workers/commits while painting, one actual emitted worker per applied batch, original-array immutability, failure/cancel retention, discard and exactly-once navigation. Existing six-plane preprocessing tests now distinguish draft events from explicit application. Native GPU appearance and measured high-resolution latency remain pending workstation review.


Current update (0.16.71): voxel editing now collects sparse click/drag strokes with an immediate bounded orange slice preview, without starting a worker, scanning/copying a whole volume or reloading source textures per pointer event. Single voxel remains the default; 3D spherical brush radii of1/2/3/5/10 working voxels edit surrounding regions. Sphere dimensions follow the working voxel grid and may be physically anisotropic. Apply edits or Finish rebuilds the image once for the batch. Other button actions first apply the batch and replay once after success; input-file selection requires Apply/Discard first. Failed/cancelled application preserves the draft for retry. Undo/Redo operates on strokes while editing and on the applied batch afterward; Discard removes only unapplied edits. Native/base arrays, geometry, landmarks, fit and optimization windows remain unchanged during painting. Unapplied drafts are not retained after closing the app. The existing total20,000-voxel per-image recipe cap remains; use masks for larger regions. Saved/exported data use the committed recipe, never preview pixels. Apple Silicon macOS only. Native graphical responsiveness is not certified in this sandbox; committing still performs one full reconstruction and texture update, including mask/hole recomputation where applicable.

## Systematic code audit — 0.16.70

The audit combines source/error-path inspection, the complete regression suite, exact compiled DOM event tests, emitted-worker tests, and new independent fault injection. It does not establish exhaustive correctness over all possible datasets or operating-system states.

| Area | Evaluation and result |
| --- | --- |
| Landmark math | Existing large/90°/near-180°/oblique/reflection/degenerate fixtures plus 320 new seeded exact recoveries across all four linear models; all corresponding points fitted jointly. |
| Coordinates and image preparation | qform/sform, physical units, anisotropic/oblique grids, endpoints and native/working separation. Fixed fallback spacing to use selected affine column norms, matching the streaming path. |
| Resampling/export | Nearest/linear/cubic, both directions, spline inverse, native/processed/FoV options, output metadata. Fixed zero-weight NaN/Infinity contamination; genuinely nonfinite contributing samples remain nonfinite. |
| Manual/reset/session state | Twelve-parameter two-target composition, spline input composition, strict session replay and explicit proposals. Fixed reset retaining promoted affine model/stale diagnostics; retain optional original model for newly created fits and carry it through refinement. Older sessions without that field retain a compatible fallback. |
| Preprocessing and masking | Crop/edit isolation, hole topology, saved versus draft masks, shared array immutability, Undo and actual workers. Fixed physical crops that remove a crosshair without shrinking the rectangular grid, and invalid nonfinite voxel lookup. |
| Async failures | Removed the obsolete main-thread refinement implementation (129 coordinator lines); its hidden compatibility entry delegates to the current constrained worker. Legacy session proposals reconstruct their accepted/manual pose on rejection instead of retaining the candidate. Cancellation, stale-input rejection, transactional commits and worker errors. Fixed synchronous postMessage failures leaking workers/cancel listeners in preparation and exports. |
| File saving | Authorization, bounded streaming, retry queues, local/SFTP overwrite paths. Found backup deletion after failed rollback. Shared replacement helper now retains originals on rollback failure, names recovery path, and never rolls back a successfully installed file solely because backup cleanup failed. Fault injection covers install, rollback and cleanup errors. |
| Guide and UI | Existing actual event/compiled frontend tests cover four processing stages, point-level Back, disclosure independence, startup collapse, proposal decisions, docking/dragging and export gates. No new guide redesign. |
| Runtime and release | Desktop isolation/navigation/preferences, local server security/storage, SFTP protocol, lifecycle and launch-handshake suites; exact signed Apple Silicon ASAR/frontend/worker verification. Native graphics are not established by adapter tests. |

### Remaining limits and maintenance risks

- Two browser/GPU suites are blocked by the host's Chromium bootstrap sandbox. Native mouse/keyboard rendering, GPU texture output and macOS launch behavior require a graphical workstation run.
- Synthetic fixtures do not establish registration accuracy for the user's anatomy. Compare results with independent landmarks and visual review; metric improvement alone does not prove alignment.
- Memory planning remains an estimate, not measured peak residency. No stress-to-allocation-failure or multihour soak test was performed; finite native/export buffer limits remain documented.
- Current release includes arm64 macOS only; Windows, Linux and Intel packaging/runtime were not newly certified.
- Dense modules and the 883-line frontend coordinator make cross-feature state changes harder to review. Future refactoring should isolate state transitions with the existing compiled UI and physical-window regressions as gates; a broad refactor was deliberately avoided in this bug-fix audit.
- Older sessions cannot recover an original transform-model label that was never recorded. Their matrix remains authoritative; newly generated sessions retain the original model. Schema9 remains compatible through an optional validated field.

### Reproduced before correction

An sform with 2/3/4 mm columns and stale 1/1/1 pixdim produced 1/1/1 fallback spacing. Exact sampling of a finite voxel surrounded by NaNs returned NaN with linear and cubic interpolation. An unchanged-grid crop retained an outside crosshair. A throwing preparation worker rejected without calling terminate. Resetting a manually scaled rigid fit retained affine12 metadata. Overwrite code unconditionally removed its backup after a failed restore; this was established from the error path and reproduced with fault injection, without touching user files.


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

The reported session contained a pending automatic-registration proposal. Export guards correctly refused image/transform exports, but enabled-looking buttons and status messages behind the modal made this look unresponsive. The modal now gates buttons and shows the requirement with a route to existing proposal review. Session saving remains valid for proposals. Export actions are serialized and feedback is visible inside the modal.

Destination failures were also swallowed by the save queue, allowing callers to claim success and continue a multi-file export. Each attempt now rejects to its caller, while a separate recovered queue tail permits retry. Regressions exercise real DOM events, authorization/API adapters, batch stopping and recovery. Registration acceptance and reflection checks remain explicit; no preview is silently accepted or exported.

# Guide placement and attention audit — 2026-10-14

Floating positions are checked against the highlighted control plus the active MRI/CT or review panes with an outline-clearance margin. No overlapping candidate is selected. A crowded layout reserves dedicated guide space rather than accepting an overlapping placement; narrow viewports reserve a bottom tray. Step changes reset manual placement, while safe dragging remains available. Layout transitions dispatch resize and re-expose the target without changing physical optimization-window state.

A new target gets three 600 ms outline pulses, followed by the persistent highlight. Timers/classes are cleaned up when switching targets or hiding the guide. Reduced-motion media settings disable animation. DOM regressions supply actual element rectangles and dispatch guide/tool events to check unobstructed positions, modality changes, fallback docking, pulse lifetime and reduced motion. Native visual verification remains pending when browser/GPU bootstrap is blocked.

# Guided-action synchronization audit — 2026-10-14

Previously, the guide polled image/landmark state but did not follow direct tab/operation actions. It advanced from proposal decisions only on the refinement route and required a guide button after window editing. The active guide now observes delegated DOM clicks/changes, refreshes after application event handlers via a microtask, and follows the actual selected operation. It advances on completed window editing and all observed proposal decisions. Internal guide-generated control clicks are guarded to avoid feedback loops. Parameter edits do not complete multi-action tasks; manual work keeps explicit Continue. Hidden guides preserve their route. The refresh signature includes instruction text/title/target to avoid stale labels.

Regression tests execute real direct and guide DOM events, including window start/finish, global/local selection, parameter changes, same-step waiting, automatic/spline decisions, old-fit baseline and pause/resume. Keep these tests when changing guide state or event ordering. Optimization-window capture and physical geometry were not changed.

# Ubuntu packaging audit — 2026-10-13

The Linux x64 Electron runtime matches its official release SHA-256. The shared registration frontend/worker implementation is unchanged. ELF headers, exact ASAR source/assets and Linux menu adapters are checked; the Debian data archive must match every verified application file. Explicit dependency declarations cover the runtime-linked libraries, including t64 alternatives for newer Ubuntu. The installer uses standard per-app sandbox/AppArmor setup rather than a no-sandbox launcher, and removal unregisters the alternatives/profile. A synchronized desktopName fixes Linux window/launcher association. Native Ubuntu GUI, GPU, install/remove scripts and anatomical behavior remain unexecuted on this macOS host.

# Windows packaging audit — 0.16.43-desktop.1

The Windows desktop uses the same scientific frontend and workers as macOS, without the macOS shell/SSH payload. Its x64 PE executable and DLLs, product/version resources, ASAR header integrity, exact packaged source/assets, actual Windows menu adapters and linear/nonlinear workers are checked. The NSIS installer is extracted; every embedded application file must match the verified unpacked app. Neither these checks nor a cross-build establish native Windows GUI, GPU or install/uninstall behavior. The app/installer are unsigned. The Windows edition provides local image workflows; interactive workstation authentication remains macOS-only.

# Desktop packaging audit — 0.16.43-desktop.1

The prior universal desktop contained approximately 472 MB of Electron frameworks plus 243 MB of standalone Node binaries. Those binaries served the legacy browser/workstation launcher, even though Electron includes Node. The registration application archive was only 2.6 MB. This release separates arm64 and x64 desktop apps and supplies the remote launchd wrapper with the trusted Electron executable in explicit Node mode. A separate browser package retains both Node binaries. The full release includes both desktop variants, so its aggregate size should not be confused with one installed desktop app.

Maintain the embedded-runtime contract: process.execPath comes from the desktop main process; runtime-command.sh must write ELECTRON_RUN_AS_NODE=1 into the launchd wrapper rather than relying on inherited environment. Recreate generated payloads before building. Keep standalone Node out of workstation-support and verify each app's architecture, signature and size budget. Run test:desktop-packaging and lifecycle/SSH regressions after launcher changes. Actual authentication, launchd and native rendering require target-machine checks when sandbox restrictions prevent them here.

# Guarded 3D spline audit — 0.16.42-spline.1

Generalized the application point-mapping boundary rather than hiding a deformation behind the old affine matrix. The shared nonlinearSpline module fits a proper affine base plus normalized 3D -r kernel residual with an affine polynomial tail and nonnegative ridge. The sign gives a conditionally positive definite system; this is not the 2D r²log r basis. A bound on ||Du|| is the polynomial Frobenius norm plus the sum of radial weight norms, divided by normalization scale. Attenuating all residual coefficients to 0.65 makes inverse fixed-point iteration contractive and prevents a folding residual map globally. This is a conservative mathematical safeguard, not a validation of anatomy, and the kernel is not differentiable at control knots. Finite-difference Jacobian samples are explicitly estimates.

The full mapping is F(x)=D[G(Ax)], where A is the saved spline affine base and D is the current affine component times inverse(A). Navigation, landmarks, residuals, resampling, local intensity scores and fields use this same mapping/inverse. Manual and image-refinement adjustments act after the spline. The matrices saved in FitState are affine components only; nonlinear JSON sets forward_matrix/inverse_matrix null. Sessions validate controls against saved physical landmark pairs and recompute RMS. Reflections, missing/unsafe coefficients, mismatched controls, duplicates, poor rank and excessive allocation requests are rejected. Exact interpolation may be attenuated for safety; the UI reports this rather than silently forcing anatomy through suspect points.

Cancellable worker fit/resample/field operations preserve loaded buffers. Preview errors clear misleading overlays and revert/discard incomplete candidates. Field grids default to 2 mm, retain exact physical extents/directions, serialize little-endian vector data in dim5, and enforce a 256 MiB vector cap; they carry explicit RAS-mm conventions and sampling metadata. Continuous image export uses the actual inverse rather than interpolation of an inverse field.

New evidence covers exact nonlinear and affine reproduction, smoothing, inverse round trips beyond controls, explicit manual composition, reversed correspondence, degenerate/reflected rejection, safety-limited severe displacement, session/coefficient tampering, native oblique resampling against known physical intensities, NIfTI vector headers/data, field grid endpoints/allocation checks, local refinement retaining spline coefficients, and the actual emitted worker/controller including progress and cancellation. Native graphical appearance and the user's specific anatomy remain unverified here. Optimization-window modules are unchanged; their required regressions run for this release.

# Post-refinement guide audit — 0.16.42-spline.1

The guide formerly jumped directly to export when a refinement proposal disappeared after accepting or rejecting it. It now returns to alignment review, exposing manual adjustment, a new global registration, further local refinement, landmark retry and saving. Global registration remains a new search; refinement retains the existing starting matrix. Neither option is run automatically. Executable guide regressions exercise both proposal decisions and verify that choosing automatic registration selects the global operation. Existing window capture protections and release checks remain unchanged.

# Persistent optimization-window audit — 0.16.42-spline.1

Three code-level failure paths were reproduced/identified: canvasPos2frac returns finite [-1,-1,-1] outside the image (previous finite-only validation accepted it); frac2canvasPos rejects off-screen rectangle corners (so a stored window can disappear after zoom/pan or an outside release); rejecting a replacement deleted the previous valid bounds. Raw voxel-spacing thresholds also mixed voxel axes with physical display axes. The user reported an MRI axial window disappearing; without their volumes/native graphical reproduction we cannot attribute that specific occurrence conclusively to one path.

The isolated optimizationWindowGeometry module uses each Niivue physical slice tile, signed/mirrored viewport geometry, physical field of view and current world depth. Capture clamps outside endpoints at the visible tile boundary. Rendering projects the full rectangle and lets the layer clip it, without changing saved millimeter bounds. Invalid replacements leave state intact; CSS-pixel drag thresholds handle zoom independently of voxel geometry. Pointercancel uses the last valid point. The earlier worker-boundary two-axis correction remains intact.

## Maintenance contract

Root AGENTS.md now records the protected behavior and required verification. Source comments explain the vendor sentinel, off-screen projection, RAS/voxel distinction and clipping rules. Review these notes before changing window capture, renderer, crosshair coordinates, session persistence or registration worker sampling. Keep all six MRI/CT planes in real event tests; pure arithmetic tests previously failed to cover the event-to-viewer boundary. Native browser/GPU and the user's dataset remain pending workstation verification.

# Explicit workspace reset audit — 0.16.42-spline.1

Added an explicit Reset workspace action separate from Reset view, Restart guide and Restore all defaults. Reset confirms loss of unsaved work, pauses editing/autosaving, hides/inactivates the guide, clears guide/recovery/recent/layout storage through the same serialized native bridge, then reloads the document. Reload discards volumes, views, workers, pending reads, undo and registration state together. Appearance/settings, named presets, window preferences, original image files and saved exports remain untouched. Storage failures prevent reload and report a retryable error while keeping the loaded workspace. Late guide storage responses and guide persistence are suppressed during reset.

The method-choice guide target previously highlighted landmark fitting, which could look like automatic selection of landmarks. It now highlights the guide control; method-specific focus happens after selection.

# Guide session prerequisite audit — 0.16.42-spline.1

Guide route persistence was independent of session restoration. A remembered late route could show Resume guide on an empty launch, then skip the method choice after loading images. Saved route/history/paused status now reset to choose when there is no alignment, no landmarks and either image is absent. Position and guide preferences remain retained. Routes requiring an accepted alignment also return to choose when no fit exists. An already active guide is protected from late asynchronous storage reads. Same-session pause/resume regressions remain in place, with additional fresh-launch coverage for every persisted route.

# Optimization-window correction audit — 0.16.42-spline.1

Drawn rectangles correctly stored zero span on their slice-depth axis, but the worker integration discarded plane identity and checked all three axes. This turned each rectangle into a zero-thickness slab. Intersecting three slabs at unrelated crosshair depths could yield fewer than 64 fixed samples even when the visible windows were large.

Worker requests now carry each rectangle's two visible axis indices alongside physical min/max coordinates. Both fixed and moving sampling honor these axes, including pyramid levels and constrained local overlap. Existing full 3D numerical windows retain their original semantics. Sessions still store the original per-plane bounds; conversion happens only at the worker boundary. Tests explicitly reproduce the old error before verifying the corrected path. No fallback silently removes user constraints.

The screenshot also showed global Automatic registration during a refinement walkthrough. Guide route transitions now select the corresponding operation. Starting a new run clears stale status errors. Validation uses synthetic images and emitted worker code; the user's original MRI/CT volumes were not supplied, so dataset-specific performance and graphical workstation behavior require review.

# Settings audit — 0.16.42-spline.1

The native local server uses changing ports, so localStorage alone cannot provide reliable cross-launch desktop preferences. Added a context-isolated, sandbox-compatible preload with fixed-key persistence and native export-folder selection. Main-frame authorization, serialized atomic writes, 2 MB limits and file mode 0600 are covered by tests. Browser mode retains origin-bound localStorage.

Interface palette changes leave diagnostic image tints separate. Viewers apply tint/crosshair preferences; opacity continues to use the existing shader redraw path. Preference application distinguishes opacity/marker changes from colormap changes so persistence does not trigger texture rebuilds. Registration defaults remain separate from active controls and matrices. Docking reserves a guide column rather than covering anatomy. Recovery stores session metadata/state, retaining the previous launch snapshot without duplicating voxel buffers. Current session geometry/reflection validation remains in force.

Executed validation exercises source controllers, the actual built frontend, restricted IPC, and packaged UI/worker/backend paths. Native graphical appearance, GPU output and clinical registration behavior still require workstation review; no new native graphics certification is claimed. Custom shortcut remapping and one-click reopening of recent files are not implemented; recent names and existing shortcuts are available.

### Refinement correction audit

The global-registration release hid the former local bounds/steps and routed local refinement through the general optimizer. This release restores the bounded coordinate search in a worker, with explicit saved start-step settings and overlap support, while retaining broader model choices as explicit adjustments. Windows precede the operation controls. Real native graphics/clinical registration accuracy still requires workstation validation.

# Automatic registration audit — 0.16.42-spline.1

Adapted the automaticLinear and imagePyramid algorithms from the user-supplied v0.17.26 archive, retaining our existing desktop organization and interaction improvements. The attached regression checks used a matrix-distance score independent of pyramid level. Additional tests found the integration must refresh candidate scores at each level and compute final score comparisons on consistent samples. We also removed dependence of global fixed sampling on the starting overlap box, pass operation direction explicitly, and preserve held custom parameters during coarse optimization.

The solver is a bounded, heuristic broad-orientation initializer, not a guarantee of global optimality. Positive exponential scales prevent silent reflections; composed reflected starting transforms are rejected by the determinant check. The visible workflow provides global initialization and local refinement for four standard models and custom groups, with five metrics. Image tests validate 90/175-degree rotations, standard-model scale/shear recovery, both directions, oblique anisotropic geometry, and local preservation of an accepted nonrigid base. Exact production worker execution covers all model routes, session restore, and cancellation. These are synthetic/DOM/runtime tests, not certification on clinical MRI/CT datasets.

Preparation/optimization moved into a cancellable worker. Fused axis filtering/decimation avoids three native-grid smoothing copies. Input volumes are structured-cloned to the worker so the main app retains ownership. Large inputs still require additional memory and time; final overlay resampling remains synchronous. Native browser-worker rendering, interactive responsiveness, and shader timing require GUI review.

Release staging must be fresh: merging a rebuilt app into an older staging bundle can retain obsolete hashed assets and invalidate the resource signature. Added a staged signature gate to the release packager and verified the extracted final archive.

Sessions use schema 3 and preserve automatic provenance and reversible proposals, migrating older supported formats. Full affine resampling/export stays on the existing validated physical-coordinate path. Attached nonlinear/deformable features are outside this automatic-linear release.

---

# Workflow and opacity findings — 0.16.33-workflow.1

The guide inserted Apply above Add at the fitting threshold, moving an existing click target. Both now occupy fixed first/second slots, with Apply disabled before readiness. Workflow focus groups related tools and collapses other registration sections. Direction is independent of the selected panel.

The opacity handler previously reset both colormaps and rebuilt the entire volume texture on every input in all three panes. Niivue’s default overlayAlphaShader controls background weighting, so using it directly would give wrong opacity endpoints. A dedicated two-volume fragment shader, installed through the public custom-slice API, explicitly multiplies moving alpha instead. Keeping the single moving texture at unit opacity and applying the user value in that shader removes texture rebuilds from slider input; executable mock tests assert zero uploads across endpoints/intermediate values. Mode changes retain one texture rebuild per pane. Scalar statements extracted from the actual GLSL are tested at 0/1 and intermediate values, including empty moving samples and fixed-alpha preservation. Native timing and pixel/GPU review remain pending; no measured speedup ratio is claimed.

Automatic refinement previously updated only the bottom status and yielded after each pass. The new activity strip paints before major stages and between search axes, with guarded operation entry and temporary input lock. Synchronous overlap preparation and full-volume resampling remain possible UI pauses; a worker pipeline is still a future improvement.

Inspected the actual supplied v0.16.27 ZIP and its packaged frontend. Automatic refinement is present; Automatic registration is not a separate control. The method performs bounded local rigid NMI search, not global orientation recovery. Historical ZIP/frontend hashes and label counts are included in Validation/historical-automatic-ui.json. This finding does not establish the contents of earlier unavailable chats.

---

# Guide usability fixes — 0.16.32-guide.1

The guide was fixed to its automatically positioned target and closing it reset the route on reopen. Its landmark steps offered a skip action without an explicit add/apply action. Added user-positioned guidance, keyboard movement, viewport clamping, resumable dismissal and explicit restart. Adding pairs now leads to applying the selected model when its minimum is met. The guide delegates to existing application actions and does not silently fit, confirm reflection or accept refinement.

Executable guide regressions cover pointer movement/cancellation, keyboard movement, pause/resume with state advancement while hidden, fitting minimums/errors and primary actions. The exact production frontend executes the resume controls. New native visual certification remains pending in this restricted environment; the user reported that the previous .31 design looked good and identified these real-use guide issues.

---

# Viewer-style interface audit — 0.16.31-theme.1

Compared Viewer's pinned reference palette, typography, toolbar clusters, control rail and panel treatment with Align. Corrected the flat crowded toolbar, fixed 1120px body minimum (larger than the desktop window's 1000px minimum), blue styling across dialogs and the guide, and viewport-height arithmetic that ignored compatibility banners and wrapping controls. The registration rail now sits on the left, image columns can shrink, status occupies its own accessible strip, and all chrome uses centralized graphite/amber tokens. Diagnostic image colors remain stable.

Extracted shell markup from the frontend controller; release identity now flows into the displayed version rather than requiring a hard-coded HTML replacement. Fonts are offline and ship with their independent SIL OFL licenses. No Viewer functions or domain modules were copied.

The existing browser UI regression contained a duplicate const declaration and assumed WebGL failure without controlling that prerequisite. Fixed both and added window-boundary/guide checks. Browser startup remains sandbox-blocked, so those visual assertions are prepared but unexecuted here. Production DOM wiring, guide logic and all scientific checks execute separately.

The user confirmed that the previous standalone .30 build opens successfully on their Mac. That is useful real-world launch evidence for .30, not visual certification of this redesigned .31 build. Manual visual review of loaded data at desktop minimum size remains outstanding, as do the existing native/GPU/Intel/real-workstation checks.

---

# Desktop architecture review and audit

Reviewed Brainana Viewer at commit `aaa47a989329f9b42c97a592ad0801b7d896216e` on 2026-10-10. Reference: https://github.com/brainana/brainana-viewer/tree/aaa47a989329f9b42c97a592ad0801b7d896216e . Its app composition roots, npm workspaces, reusable desktop/server/client/math packages, hardened BrowserWindow and electron-builder packaging are the structural reference. Align's implementation is independently written; no Viewer application functions or domain modules are incorporated.

## Changes completed

- Replaced the packaged browser-opening shell as the default with a dedicated Electron window and embedded Chromium.
- Moved the domain to apps/align; extracted backend runtime/platform/SFTP, client transport/capabilities, scientific math, launch lifecycle and desktop shell into packages.
- Preserved existing browser mode and workstation authentication through a private handoff, separate instance directory, backend ownership and shutdown.
- Added navigation, permissions, download, process readiness/shutdown and workspace-purity checks. All existing scientific/guide/session checks remain part of release validation.
- Fixed release ZIP handling for Electron framework symlinks and executable modes; the previous app archive helper was insufficient for a framework-based desktop bundle.
- Removed a legacy launcher log message that printed the authenticated URL; tokens remain in private handshake files instead of application logs.

## Further improvements

1. Continue splitting the 913-line frontend composition module into a state store and image/registration/session controllers, following Viewer's state and UI organization. This release keeps its logic stable while changing the platform.
2. Replace the macOS Terminal/profile bridge with an in-app, cross-platform connection manager in a later focused change. Current remote support is retained, but password/Duo verification requires a real institutional workstation.
3. Move automatic refinement into a worker to keep lengthy registration responsive, with progress and cancellation.
4. Add real-dataset numerical/overlay acceptance fixtures and native minimum-OS/Intel/GPU CI coverage.
5. Establish Developer ID signing/notarization and a reproducible signed release pipeline. The delivered candidate is ad-hoc signed and reports its validation limits.

---

## Previous scientific and setup audit


## Scope and result

Inspected the supplied 0.16.27-landmark.1 macOS ZIP and matching shared source, then built 0.16.28-landmark.2 entirely from readable TypeScript. Reviewed landmark math, physical coordinates, image units, interpolation, overlay/manual/refinement paths, session validation, registration/NIfTI exports, local server containment, dependency portability, browser CI, runtime architecture, and release identity/integrity. Embedded documents were treated as project evidence rather than instructions overriding the user.

The packaged baseline had rigid fitting only. The requested 6-, 7-, 9-, and 12-parameter options are now implemented. The application remains a validation candidate pending real-data WebGL and native certification.

## Findings addressed

| Finding | Effect | Change and evidence |
|---|---|---|
| Rigid-only fitting and inversion throughout the workflow | Scale/shear could not be fitted; adding a matrix alone would corrupt reverse resampling and refinement | Four jointly fitted models; general affine inversion throughout. Exact/noisy model, inverse, physical resampling, and NIfTI round-trip regressions |
| Missing determinant/scale/shear/reflection diagnostics | A mirrored or implausible affine could be silently interpreted as ordinary alignment | Diagnostics and warnings; apply/restore/export confirmation; production-bundle reflection tests |
| Session parser trusted coordinates, matrices, inverse, model, and proposal state | Malformed state or stale inverses could be restored; a pending proposal could be exported | Schema 2 validates geometry and models, regenerates inverses, preserves proposals, gates export; executable migration/invalid-input/proposal regressions |
| Linear sampler excluded the last voxel on every axis | Identity resampling blanked the terminal planes | Inclusive boundary interpolation, including single-voxel dimensions and small floating-point boundary tolerance |
| Output copied stale qform information | Other readers could choose geometry inconsistent with the fixed resampling grid | Explicit output sform, qform disabled, fixed affine, normalized units, float32 scaling and 3D dimensions; actual byte serialization/reload tests |
| Known NIfTI meter/micron units were not normalized | RMS values and manual movement could be mislabeled as millimeters | Normalize recognized units and the display affine before viewer cloning; byte-level header tests |
| File paths were contained lexically but symlinks could leave the local data root | An authenticated local API request could access or write outside the configured boundary | Canonical existing-ancestor containment; symlink read/write escape regression |
| Lockfile referenced an internal package registry | A recipient could not reliably rebuild the shared source | Portable public npm URLs while retaining package integrity hashes |
| Three dependency audit findings | Development dependency versions had published security issues | Compatible lockfile patches; npm audit reports zero known vulnerabilities (not a complete security certification) |
| Truncated Intel Node binary in supplied ZIP | Intel executable load segments and signature offsets extended beyond the file; the old checksum faithfully matched the corrupt binary | Replaced with official Node 24.17.0 x64 archive after official manifest SHA-256 verification; both runtime signatures and authenticated server smoke tests pass; malformed Mach-O regressions added |
| Advertised macOS 11.0 below both bundled runtime deployment targets | The app claimed compatibility it could not provide | Declared minimum 13.5; release verification reads each Mach-O deployment target |
| CI installed browsers independently of locked Playwright and searched a fixed system path | Downloaded engines could be missed or mismatched | Locked CLI install, Playwright-discovered executable paths, full CI check gate, packaged option assertions |
| Release verification checked identity but not feature presence or binary architecture | A rigid-only or wrongly assembled package could pass | Packaged labels, source/runtime hashes, architecture, deployment-target verification, manifests, and SHA-256 inventories |

## Numerical choices and limits

Rigid fits retain the corrected Horn quaternion solver with symmetric Jacobi diagonalization; matrix normalization improves its numerical scale tolerance. Similarity uses the same proper rotation and one least-squares positive scale. Affine uses centered, column-normalized, pivoted QR with reorthogonalization rather than normal equations. Both moving and fixed geometries must span three dimensions for affine and anisotropic fitting.

Anisotropic fitting alternates exact proper-rotation and positive-axis-scale updates over all points, starting from the rigid fit and an affine-derived orientation. Iteration is bounded and failure to converge is actionable. It is a nonlinear local method; two starts and exact/noisy regression cases do not prove a global optimum for every noisy configuration. Strong correlation or poor landmark distribution should be handled by more distributed landmarks, another model, or further validation.

Physical scale estimates are column lengths. Shear is a dimensionless maximum axis cosine, not a Euler decomposition or a shear angle. The reported transform conditioning value is a normalized Frobenius inverse-norm estimate; landmark geometry warnings additionally account for unequal coordinate spread and QR independence. Warning thresholds are transparent heuristics, not dataset-specific plausibility certification.

The inverse of a moving-axis anisotropic model generally does not belong to the same constrained parameterization. Forward/inverse exports are exact; a reverse constrained fit optimizes a new model in the other moving space. Affine handles the unrestricted inverse.

The chosen qform/sform remains Niivue's common display affine. Conflicting valid forms follow that library's code/validity policy; no new form selector is introduced. Known spatial units are normalized; unknown units keep the previous millimeter assumption.

## Recommended next improvements

1. **Real MRI/CT and browser certification:** validate the user's differently oriented datasets in native Safari, Chrome, Firefox, and Edge; verify landmark placement, reflected confirmation, overlay, manual corrections, export, and restored sessions. Chromium launch is blocked here by a macOS sandbox Mach IPC restriction. DOM checks exercise the production code but do not certify WebGL or file dialogs.
2. **Signed macOS release:** add an authorized Developer ID signing/notarization workflow and verify Finder, Dock, Gatekeeper, launchd lifetime, sleep/wake, and institutional SSH/Duo on target hardware. The current app bundle is not newly signed/notarized. Both current official Node runtime signatures verify; the valid Apple Silicon executable was retained and the Intel executable replaced.
3. **Responsive large-volume handling:** move landmark optimization, full-volume resampling, and refinement into workers, with progress/cancellation and bounded memory. The current review path allocates float32 buffers and multiple viewer copies; large datasets can block the UI and stress GPU/memory.
4. **Anatomical-frame initializer:** construct orthogonalized A–P/I–S/L–R axes with explicit handedness before optional model refinement. Those landmarks already work as ordinary point correspondences.
5. **Data-aware diagnostics:** expose the selected form/units, conflicting qform/sform warnings, point-layout visualization, leave-one-out residuals, conditioning estimates, and uncertainty. Add robust fitting/outlier handling only with clear user controls.
6. **Broader scientific validation:** independent numeric reference comparisons, noisy/adversarial optimizer cases, large coordinate offsets, and realistic end-to-end fixtures. Several inherited checks inspect source patterns rather than execute the behavior they name; the added tests execute fitting, restore/export, NIfTI bytes, and the built UI.
7. **Packaging/runtime coverage:** the corrected x86_64 runtime executes successfully through translation here, and both architectures serve the authenticated packaged frontend. Test native Intel app interaction on an Intel Mac as well. Review bundled runtime advisories separately from npm dependencies.

## Sources consulted

The official NIfTI field documentation defines spatial units, qform/sform, affine rows, and scaling fields: https://nifti.nimh.nih.gov/nifti-1/documentation/nifti1fields.html

PostCSS's maintainer security advisories corroborate the dependency audit findings: https://github.com/postcss/postcss/security/advisories

The release decisions above are based on inspected application/dependency code, actual binary headers, and executed regression evidence. The replacement Intel runtime archive and SHA-256 manifest came from https://nodejs.org/dist/v24.17.0/. External source links support format/security context; they do not certify the application.

## Guided setup extension — 2026-10-10

0.16.29-guided.1 adds state-aware nonmodal guidance. Skipping landmarks now leads to an explicitly selected physical-coordinate starting alignment, with overlap review before local refinement. Guidance waits on successful loads/fits and actual proposal decisions. Existing data and errors remain visible. No-landmark initialization is recorded and has unavailable/null landmark RMS. The guide is opt-in and does not perform registration decisions automatically. State and DOM tests cover branching, load/error waiting, corresponding points, model minimums, proposal prerequisites, dismissal/restart, and session/export provenance. Native tooltip positioning, file dialogs, and WebGL certification remain pending.

---

## Historical reconstruction findings

The following record describes earlier source reconstruction and is retained as history. Current behavior and certification are defined by the sections above, README, and VALIDATION.

# Brainana Align Technical Findings

## Executive finding

The available v0.11.1 package is a genuine readable TypeScript/Vite project, but it predates most launcher, workstation, remote browsing, and remote export functionality. The packaged v0.15.4 application is the appropriate behavioral baseline, although it is not a maintainable source release and does not satisfy the desired self-contained architecture.

A reconstruction is feasible because the important materials are split across three forms:

1. readable v0.11.1 frontend and algorithm source;
2. readable v0.15.4 launcher, server, remote-launch helper, and export patch;
3. inspectable v0.15.4 compiled frontend containing newer interface and workflow behavior.

## Inspected inputs

### v0.11.1 source package

Readable project files include:

- `src/main.ts`
- `src/rigid.ts`
- `src/roiWarp.ts`
- `src/style.css`
- `package.json`
- `package-lock.json`
- TypeScript and Vite configuration
- compiled `dist` output

The main TypeScript file is 61,087 bytes. No automated tests or release-building scripts were found.

### v0.15.4 packaged application

Readable files include:

- macOS launcher shell script
- remote launch shell script
- local/remote HTTP server module
- remote/local export integration script
- `Info.plist`
- minimal README

The main frontend is a 934,891-byte compiled JavaScript bundle. No readable current frontend source, package manifest, lockfile, build configuration, source map, tests, or complete release documentation were found.

## Version inconsistencies

The package contains at least four identities:

- `Info.plist`: 0.15.4
- launcher deployment argument: `app-v0.15.3`
- compiled frontend session and transform metadata: 0.15.3
- README heading: 0.15.2

This prevents reliable diagnosis and makes it impossible to prove which frontend, server, and launcher revisions belong together. The rebuild should generate all identities from one version file at build time.

## Current local architecture

For local mode, the launcher selects a local directory, resolves a port, searches the user's machine for Node, starts `server.mjs`, waits for `/api/config`, and opens the default browser.

The launcher searches an interactive shell, Homebrew locations, Conda locations, and NVM directories. The package therefore is not self-contained and may behave differently based on the user's shell setup.

## Current workstation architecture

The workstation path currently:

1. establishes an SSH control connection;
2. archives and uploads the packaged runtime into the user's home directory;
3. searches for Node through the remote login shell;
4. starts the application server on the workstation;
5. adds an SSH local port forward;
6. serves the browser through that forwarded remote HTTP server.

This design explains why the app depends on workstation Node availability and why remote deployment state can survive between versions. It should not be carried into the rebuilt application.

## Server findings

The readable server provides useful behavior that can be retained behind adapters:

- directory listing with directory-first sorting;
- neuroimaging-file extension filtering;
- file streaming;
- export-directory listing;
- directory creation;
- binary request streaming to disk;
- temporary-file writes followed by rename;
- explicit overwrite protection;
- path containment beneath the configured root.

However, it directly binds all operations to one local filesystem root. In workstation mode it only works because the entire server is running remotely. The rebuilt server should call a `FilesystemAdapter` interface instead.

The existing overwrite implementation removes the destination before renaming the temporary file. This protects against partial writes but can briefly leave no destination file. A stronger replacement strategy should be selected and tested for each supported filesystem.

## Export integration findings

Workstation and local-folder export were added through `remote-export-v0153.js`, which injects controls into the compiled export dialog and overrides saving through `window.brainanaAlignSaveBlob`.

Useful behavior to preserve includes:

- local download fallback;
- optional local folder selection;
- explicit local versus workstation destination;
- workstation folder browsing;
- workstation directory creation;
- overwrite confirmation;
- serialized export operations;
- rechecking runtime mode at export time.

The injection mechanism should be removed. These controls and save calls should be native TypeScript application code.

## Profile findings

The launcher stores profiles in a tab-separated file under Application Support and sanitizes tab and newline characters. Editing and deletion are implemented.

Problems include:

- hard-coded example profiles containing institutional usernames, hosts, IP addresses, and data paths;
- no structured schema version;
- no migration framework;
- connection port stored as part of the profile even though v0.15.4 now generates a fresh random candidate each launch;
- UI implemented entirely through sequential AppleScript dialogs.

The rebuild should use a versioned structured profile format and ship with no personal or institution-specific default profiles.

## Source-recovery assessment

The v0.15.4 compiled bundle still retains enough literal interface markup and readable application-specific symbols to recover behavior. Inspection confirms the newer bundle includes the same major registration workflow as v0.11.1 plus server browsing and export integration.

The safe approach is not deminification into a purported original source tree. It is behavioral reconstruction:

- identify every current control and state transition;
- define session and export schemas;
- compare algorithms with v0.11.1;
- write regression fixtures;
- implement missing behavior in readable modules.

## Risks

### Numerical behavior drift

Rigid fitting, interpolation, refinement sampling, image-affine handling, and NIfTI writing must be compared using fixed datasets. Interface parity alone is not sufficient.

### Session compatibility

Current package output labels itself 0.15.3. Existing user sessions may rely on fields added after v0.11.1. A migration layer should accept known historical schemas and preserve unknown metadata where practical.

### SSH authentication compatibility

Penn and Harvard environments may use keys, passphrases, Duo, jump hosts, and login-shell differences. The local SSH/SFTP design should prefer the system OpenSSH client and persistent control sockets. Real institutional tests remain necessary.

### Browser folder APIs

The File System Access API is not uniformly available. Local direct-folder export must remain optional, with normal downloads as a fallback.

### Large remote files

Remote neuroimaging files should be streamed or cached without loading entire files into server memory. Cache keys must include connection identity, path, size, and modification time.

## Recommended reconstruction boundary

Use the old source for application logic that can be verified. Use the packaged app only as a behavioral and schema reference. Recover readable launcher and server concepts, but replace remote runtime deployment, remote Node discovery, remote HTTP serving, and SSH tunneling.

## Validation completed in this audit

- ZIP extraction and content inventory
- identification of readable versus compiled files
- version-identity comparison
- static launcher flow inspection
- static server endpoint and path-safety inspection
- static export integration inspection
- static comparison of major v0.11.1 and v0.15.4 user-interface strings

## Validation not yet completed

- clean dependency installation and source build
- numerical registration comparison
- browser rendering and interaction
- macOS launcher execution
- Apple Silicon and Intel runtime validation
- local export to a selected folder
- institutional SSH, Duo, or jump-host testing
- remote large-file streaming
- binary output validation in AFNI, FSL, Freeview, or NiiVue


## Recovery release note

This release intentionally packages the verified v0.15.4 production bundle unchanged for immediate behavioral recovery. It does not claim that newer frontend code has already been reconstructed into readable TypeScript.


## Workflow separation — 0.16.47-workflows.1

Workflow audit — 2026-10-14: v0.16.45 and v0.16.46 shared identical optimizer, guide, worker and window modules. A saved user session recorded global registration, not local refinement; this does not prove the cause of the reported anatomical misalignment. Existing guide resume/operation dispatch could reset histogram bins. Explicit separate hosts and same-operation preservation remove that ambiguity. Refinement now has a landmark-only eligibility check and rigid adjustment; independent image-based registration retains broad models. Region proxies must never create separate bounds or capture handlers.

## Working-resolution audit — 0.16.48-resolution.1

The previous loader allocated all-frame Float32 conversion plus NVImage clones per source/review pane, ordinary worker message copies and all-level scorer closures. New NIfTI preparation bypasses full-native conversion, scans only the requested frame, transfers the prepared buffer and shares read-only CPU voxel data. Only one pyramid scorer survives at a time; scratch histograms are reused. ROI bounds reduce fixed-grid iteration while retaining its original stride phase and window axis semantics. Native output avoids full destination float/NIfTI buffers by slab compression, but keeps the moving native frame and compressed Blob/destination copies; estimates and hard limits explicitly include these. No elapsed-time prediction or clinical accuracy guarantee is inferred. Non-NIfTI streaming, GPU-context sharing, native detail-on-demand and persistent worker caching remain unimplemented.

## Resolution layout and capacity correction — 0.16.49-resolution.2

Single-row toolbar groups MRI/CT/Resolution under Volumes and separates Session from Landmarks. Resolution has a three-column spacing row and ordered native-relative 1×/2×/4×/8× presets; recommendation is a separate action. Multi-volume selection is conditional and labelled Volume / time point. Current free memory is contextual: it excludes already-resident workspace allocations and may omit reclaimable caches. Do not compare the full planning envelope to free memory or call it measured pressure. Compare lower/upper estimates with installed RAM minus headroom (or explicit budget) for strong/advisory guidance respectively. Buffer/GPU peak estimates remain uncalibrated conservative ranges; real GUI/peak-memory measurement is pending. Protect native/working coordinate and export contracts.

## Landmark guide synchronization — 0.16.50-guide.1

Landmark Set targets may not exist until the scheduled render completes. Never cache Guided setup as their fallback. Resolve the actual selector before the refresh fast path, observe landmark-list child changes and protect the whole active landmark row plus its required image panes from guide overlap. Recreated nodes reacquire the outline without restarting the same-step pulse. Track displayed MRI/CT steps in guide history; Back revisits instructions and preserves coordinates. Completed historical points offer Replace/review and Continue with current progress. Pending points offer Set MRI/CT through the existing app action; Add/Apply remain on completed-pair screens. DOM regression reproduces delayed and rebuilt targets plus two completed pairs, adding a third, Back to previous CT/MRI, preserved data and resume. Native GUI verification remains pending.

## Independent sidebar sections — 0.16.51-sidebar.1

The six top-level sections are ordered Landmarks, Landmark alignment, Landmark refinement, Manual adjustment, Image-based registration and Alignment display. Native disclosure heading clicks affect that section alone. Opening a section activates its controls; closing a section does not select another operation or change sibling open states. Explicit workflow shortcuts retain focused navigation. Both registration sections can be independently open; a named Show controls action moves the single shared controller without changing disclosure states. Guide refresh/recreated rows do not reopen a manually collapsed section; a new step or explicit Follow highlighted control may reveal its required section. DOM regressions exercise every heading in both directions, operation ownership, shared controls, guide repaint and preserved region editing/settings. Native GUI validation remains pending.

Landmark alignment starts collapsed. The Landmarks workflow shortcut opens the point list, without pre-expanding Landmark alignment; open that section explicitly or let the guide reveal it when fitting is the current step.

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

Hole-fill references: [SimpleITK BinaryFillhole](https://simpleitk.org/doxygen/latest/html/classitk_1_1simple_1_1BinaryFillholeImageFilter.html), [SciPy binary_fill_holes](https://docs.scipy.org/doc/scipy/reference/generated/scipy.ndimage.binary_fill_holes.html). Our TypeScript implementation uses exterior span flood filling; these are algorithm references, not bundled dependencies or clinical certification. Closing (dilation followed by erosion) is a different operation for small open gaps and is not silently applied.

Viewer source provenance: https://github.com/brainana/brainana-viewer main ZIP retrieved 2026-10-16. networkSftpClient.mjs, knownHosts.mjs, SSH-config parser and host-verification fixtures/tests are adapted from that source. Copyright (c) the Brainana Developers; GNU AGPL v3. LICENSE and NOTICE.txt are included in source and app ASAR, with complete corresponding source in each full release. Dependencies retain their own licenses.

Exact Viewer reference: commit aaa47a989329f9b42c97a592ad0801b7d896216e; fetched main ZIP SHA-256 81a6fcddb157e67aa71af6bc53a7fc7028e6669ba7fd9bddb73212a786ac813d.
