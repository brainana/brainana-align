# 0.17.10 · flip inspection validation

Executed release checks: **82 passed, zero failed, two browser suites blocked**. Initial loopback-socket restrictions were resolved by granting network permission and rerunning the affected checks; the original restricted results are retained separately. Chrome exists but aborts during launch in this execution environment; no native GPU validation is claimed. The unchanged dependency lockfile currently reports13 npm audit findings(3 high,10 moderate); the raw audit is included. This display-only release does not claim dependency remediation.

The new flip-inspection regression exercises real DOM controls with a deterministic RAF clock: manual flips,1-second boundaries without intermediate redraws, three review panes, either pairing/direction label, retained blend setting, return to smooth, hidden/page/reset cleanup, empty-overlay gating, shader fallback, unchanged source buffers/cameras and zero texture rebuilds after shader configuration. Pure-image branches are evaluated from the actual shader source, including empty moving background; this is shader-command evidence, not GPU rasterization. Actual compiled UI checks exercise selector defaults, disabled no-overlay controls and smooth restoration. Full app, protected region/coordinate/registration/processing/guide suites and exact packaged proofs accompany the release. Native graphical verification remains pending unless separately executed and recorded.

# 0.17.9 · viewer regression qualification

`npm run test:viewer-rendering` executes the actual installed Niivue methods with a stateful WebGL command recorder. The original wheel handler reaches 13,743× zoom after 100 events of -0.01 pixels, and its projected opaque crosshair triangles cover all 400 sampled pane locations. The fixed route is exercised through real DOM wheel events for all nine source/review panes. Checks include event-distance equivalence, line/pixel normalization, upper/lower bounds and reversal, zero/invalid input, empty panes, constant 1–5 CSS-pixel thickness through 13,743×, correct physical projection for oblique/anisotropic/axis-swapped and mirrored volumes, and restored GL blending/depth state. No image/affine/crosshair coordinate or sibling camera mutation is allowed by the test.

This is renderer-command and geometry evidence, not native GPU rasterization. Chrome startup was attempted with the installed browser and aborted in this execution environment. Browser/GPU pixel inspection remains pending; DOM/worker tests are not described as GPU validation. Full suite results, exact packaged assets, isolated registration IPC, size, signature and archive checks are in Validation/REPORT.json.

## Earlier release documentation

# 0.17.8 · defaults qualification

The defaults DOM regression checks all profiles, preview/apply separation, keep-model, modified status, busy/missing-fixed-image gates, preserved fits/regions/local fields and immutable request-time metadata through sessions and exports. Compiled-frontend checks apply all profiles and confirm no workers, alignment changes or disclosure changes. The emitted-worker test compares five legacy models exactly with published 0.17.7 and exercises every software profile’s scoring, sampling, histogram, initialization and pyramid controls with a bounded rotation-frozen, one-pass smoke test. These smoke tests establish execution, not reproduction of another program or universal accuracy. Additional tests cover the Hellinger formula, fractions/caps, moments, invalid settings and session validation. Final suite counts, app size, package proofs and environment limitations are in Validation/REPORT.json.

All profiles populate the same editable Linear registration controls; no AFNI/FSL/ANTs/JIP/elastix linear executable or second engine is included. Profiles map supported settings to Align and display source-specific optimizer, masking, sampling and stopping differences. They do not claim identical results to those programs. Native nonlinear methods and the compact runtime remain unchanged.

## Earlier release documentation

# 0.17.7 validation scope

Actual native ITK tests cover both metrics, oblique anisotropic and axis-swapped images in both directions, accepted affine scale/shear preservation, supported fixed windows/exclusions, duplicate-linear/missing-base refusal, native iteration cancellation, N4 presets and immutable inputs. Source/frozen tests compare complete JIP/ITK fields and N4 outputs. The independent SimpleITK displacement reader remains a test dependency only.

Native replacement comparisons with the preceding SimpleITK implementation use the same synthetic physical data and parameters: four metric/direction B-spline fields are exactly equal; N4 mask is exact, maximum corrected difference 0.00000763 and bias difference 0.00000012. This is numerical regression evidence, not proof of registration accuracy on all datasets. Final suite counts, native tests, signature/dependency/size/ASAR proofs and environment limitations are in Validation/REPORT.json. Native graphical/GPU execution and notarization are not implied by headless tests.

## Earlier release documentation

# 0.17.6 · foreground-aware linear fitting for prepared images

In Image-based registration → Linear registration, click **Use prepared-image settings**, then choose the transform model and **Run linear registration**. The explicit preset selects nearby staged fitting, nonzero fixed-image foreground, mean-normalized mutual information and bounded rotation. Review and accept/reject the proposal normally. Rigid/similarity/anisotropic/affine/custom remain available. The unchanged broad orientation search, all-included-voxel matching and legacy percentile histogram ranges remain the launch defaults for compatibility.

Nearby fitting starts from the original images' physical coordinates. It does not use a previous accepted transform and is distinct from landmark refinement. Translations, rigid pose, axis scales and all enabled parameters are introduced in stages at finest registration resolution; Standard sampling uses a 60,000 sample target. Rotation limits are at most 30 degrees. Use broad search or establish an initial alignment when images are far apart. Switching back to broad restores its rotation range and pyramid controls.

Foreground uses nonzero ORIGINAL working voxels in the fixed image, intersected with existing physical regions and exclusions. It is intended for a skull-stripped/zero-background reference; valid zero-valued tissues can be excluded, so choose the policy deliberately. Sparse coarse foreground sampling uses finer samples without dropping any constraints. Mean-normalized common-range histograms apply only to MI/NMI. These are scoring operations, not N4 correction, image masking or intensity editing; source/working arrays, display and export intensities are unchanged. Session/export provenance records the selected policies; old sessions remain readable.

The provided anatomy/EPI pair is checked through real packaged loading, preparation and workers with 6- and 12-parameter models. Evidence distinguishes qualitative overlay improvement from ground-truth accuracy; the implementation is not claimed to reproduce native JIP numerics exactly. Optional JIP nonlinear and ITK routes and their private numerical runtime are unchanged (worker version 0.17.5). This candidate contains one self-contained Apple Silicon Align.app, complete source, consolidated documentation, validation, manifests and SHA-256 checksums. Native graphical/GPU hardware checks and notarization remain pending.

## Earlier release notes

# 0.17.5 validation scope

See release Validation/ for individual test logs, suite results, source/frozen numerical comparison, runtime module/native dependency audit, exact packaged ASAR/assets, isolated single-app execution and signature verification. JIP's product route is nonlinear only from an accepted Align linear matrix; scientific runtime qualification additionally exercises internal library capabilities for numerical parity. Tests cover missing-base rejection, removed duplicate linear controls, unchanged initial matrix, physical masks/windows and proposal/cancellation handling.

The full suite has two browser execution checks blocked by the host environment; these are not passing graphical tests. Native graphical interaction, GPU hardware performance, Developer ID notarization and Windows/Intel macOS/Ubuntu packaging remain unqualified for this Apple Silicon candidate. CPU/source/frozen/actual packaged IPC results are reported separately.

## Historical release notes (superseded where deployment differs)

# Align 0.17.4 interface validation

Apple Silicon candidate, 24 October 2026. This update changes image-based method organization and guide disclosure teaching. Numerical registration methods, defaults, sessions and exports are preserved; the separately supplied JIP Registration 0.8.4 source/app remains unchanged from0.17.3.

The full registered Align run contains80 suites:78 passed,0 failed,2 blocked browser-runtime checks. The missing configured Playwright executables remain the reason for the two blocked suites. A guide regression found that the new Linear disclosure could remain open after a completed global proposal. The guide now teaches closing visible method headings before closing their parent; hidden peer state never triggers reopening an already-closed parent. That regression and affected guide/compiled-production checks passed after correction. An initial packaging/validation attempt hit exhausted disk space; disposable generated caches were removed and the checks were rerun. Those storage failures are retained separately and are not counted as application passes.

Method tests assert exact peer order and hierarchy, all three collapsed by default, shared acquisition information above choices, stable control IDs/model/default values, independent disclosures, preserved accepted fits/regions/settings, pending-proposal gates and zero registration calls from heading clicks. Guide tests use actual heading/control events for outer/nested/advanced fields and all three methods; Show/Follow does not execute the tool or change disclosure state. Compiled production tests execute the fresh emitted application and workers. Existing physical coordinates, windows, preprocessing, manual/nonlinear transforms, cancellation, export/session, network, desktop and lifecycle suites remain required.

Current packaged proofs check arm64/local signatures, exact frontend/ASAR/worker/runtime bytes and actual frozen JIP plus ITK trusted IPC with full-map session/export roundtrips. Companion0.8.4 is reused byte-for-byte, together with its unchanged71-test frontend,21-module frozen-source verification,16 native groups and prior offscreen qualification; no new companion implementation, GPU benchmark or anatomical-accuracy claim is made here. Archive extraction/signatures and delivered-source native rebuild are reported separately beside the final archives.

Native graphical/GPU behavior and second-computer installation remain unverified in this environment. Local ad-hoc signing is not Developer ID notarization. This candidate refreshes Apple Silicon only. All prior scientific limitations are retained in the preceding release report below.

## Previous release validation

# Align 0.17.3 / JIP Registration 0.8.4 validation

Apple Silicon candidate, 24 October 2026. Existing registration defaults, native JIP numerical source/libraries and optional Metal/CUDA path remain unchanged. New ITK is a separate SimpleITK 2.5.5/ITK 5.4 CPU route, not elastix. No native GPU performance claim is made.

## Completed source and worker checks

| Check | Result |
|---|---|
| Align registered suites |77 passed,0 failed,2 blocked browser suites |
| Final affected suites after review fixes |11 passed against fresh emitted build |
| Companion frontend |71 tests passed |
| Native JIP CTest |16 passed; engine sources and both libraries identical to prior release |
| ITK analytic suite within frontend total |9 passed, including oblique/aniso/axis-swapped physical mappings |
| Actual ITK Qt workflow |1 offscreen test passed, including preview, Accept/Reject, exports and restore |
| Packaged Align assets/workers |Exact source/ASAR hashes, actual emitted workers, arm64 and local signatures passed |
| Frozen companion trusted IPC |Original JIP and optional ITK full jobs passed; nonlinear physical maps/session/export retained |
| Frozen companion Python source |21 modules match source |
| Frozen native libraries |Code/constant sections match |
| Companion dependency closure |256 Mach-O files; no external non-system or mixed Qt dependencies |
| Frozen real-data existing JIP GUI/CLI workflow |Offscreen preparation/N4/linear/nonlinear/export/restore passed; outputs match exactly |
| Frozen constrained JIP bridge |Source/frozen field, matrix, geometry and sampling match in both directions |

Raw current evidence is retained under Validation. Browser suites test:browser and test:browser-matrix could not launch their configured executables. Local server tests initially hit sandbox loopback restrictions, then passed with loopback permission. Earlier failed logs and rerun results are not evidence of a passed first attempt. Native Cocoa launch was attempted and blocked by desktop/pasteboard services; automated offscreen GUI tests are reported separately. The tests do not prove that every possible UI ordering or anatomical case is correct.

Independent known-transform ITK tests achieved maximum held-out linear point error0.0084mm rigid,0.0290mm similarity and0.0196mm affine. A nonlinear analytic case with a nonidentity affine achieved mean point error0.1381mm and MSE0.78393→0.01059, including both physical map directions. These are synthetic regression measurements, not clinical accuracy, real-data superiority or a recommended parameter guarantee. Random samples are seeded and numerical bounds remain enforced.

Actual SimpleITK reading of emitted intent 1006 displacement NIfTI verified anisotropic/oblique geometry, known nonzero RAS translations, forward/inverse point mappings and image-resampling direction against Brainana. The reader automatically converts both geometry and vectors to LPS; an extra manual flip would be wrong. No Slicer GUI certification is claimed. Canonical JSON/session roundtrips retain full nonlinear fields and ITK/JIP engine provenance, rather than exporting the affine alone.

Landmark sensitivity tests cover model-specific gates6/6/8/8/20, omitted-pair refits, invalid retained geometry, bounded work, worker cancellation, source immutability and stale results. The report is deliberately labelled sensitivity. Training/omitted-point residuals do not independently establish target accuracy; clustered landmarks and extrapolated anatomy remain concerns.

Background overlay tests execute actual emitted workers across12 affine/spline/JIP direction/target/manual combinations, cancellation/failure/stale continuations, unchanged input arrays and fixed-volume reuse. A compiled production regression reaches the ITK preview worker and cancels it, retaining the previous display and accepted matrix. Acquisition tests cover signed physical axes, BIDS allowlisting, import ordering, manual Save/target invalidation, resolution-only retention, file/frame replacement, explicit settings/phase-axis actions, session13 and unsupported ITK region guards.

## Scope and limitations

- ITK offers linear6/7/12 models and optional B-spline fitting on CPU. Existing Align9-parameter and JIP routes remain available separately. Fixed physical windows require dense working-grid sampling; continuous moving windows and sheared grids are refused. Exclusion masks are supported at any pyramid. At least8 voxels/axis and at most 2 million voxels/input remain required.
- Positive sampled Jacobians do not prove continuous invertibility, inverse convergence or anatomical correspondence. ITK has no phase-encoding constraint. EPI axis metadata requires verified native voxel directions; timing is provenance only. No field-map estimation, TOPUP, BBR or automatic T1/T2 readout inference is added.
- Smooth MI/seeded jitter/full-orientation starts are optional; defaults are preserved. New choices are not proven better on every anatomy. Overlay work is moved off the UI thread, but visible-slice rendering and end-to-end native interaction benchmarks remain future work.
- Source and headless/offscreen tests cannot certify native GPU appearance, device speed, second-computer installation, Developer ID notarization or unbuilt Intel/Windows/Ubuntu binaries. This candidate refreshes Apple Silicon only.

Exact-ASAR/worker verification, frozen JIP+ITK trusted IPC, archive extraction/signatures and a clean native rebuild from the delivered source are completed during assembly and recorded separately beside the release assets. Their reports must be marked passed before release; they are not implied by the source-suite table above. No new timed speedup or memory-reduction claim is made. Historical results below belong to earlier releases.


## Historical validation

# Align 0.17.2 / JIP 0.8.3 validation

Apple Silicon candidate, 23 October 2026. The original JIP optimizer, objective formulas, interpolation arithmetic, candidate ordering and CPU default are retained. Existing optional Metal/CUDA execution and CPU fallback remain available; no GPU speedup was measured in this environment.

## Changes verified

JIP now accepts independent fixed/moving exclusion masks and the shared physical optimization regions from Align. Fixed regions select working-grid sample centers. Moving regions select continuous physical RAS pull coordinates; moving exclusions select the nearest working voxel. Each rectangle restricts only its two visible physical axes; hidden slice depth never becomes a third constraint. Selected samples gate both objective accumulation and intensity normalization. Intensity buffers, interpolation taps, deformation support and output extent remain unchanged. Regions are neither a crop nor a deformation boundary. Fewer than 64 usable varying fixed samples produce a recoverable error without replacing the accepted fit.

The additive sampling-v1 ABI preserves existing configuration layouts. Sampled checkpoints use JIPCP004 with masks/windows under CRC; unmasked writes retain format003 and reads001–004 remain supported. Align requires the companion capability and confirms sampling in the result, refusing unsupported constrained requests. If an explicitly selected older companion remains active, select JIP0.8.3 in the JIP section. Older companions still support unconstrained jobs. Standalone JIP gains ABI/headless support; no new standalone region-drawing GUI is supplied.

Manual adjustment now reuses compatible cached review pixels for an immediate geometry preview, coalesced once per animation frame, then rebuilds the overlay from original working data 450ms after the last nudge. Both image targets and transform directions, affine/spline/JIP mappings, source-row preview, repeated nudges and stale asynchronous results are covered. The transient preview is labelled and may retain cached field of view/interpolation until the full rebuild. Real resampling, export, navigation and saved maps use the complete transform. Settled linear resampling still uses the existing main-thread path; this is not a visible-slice renderer or a complete worker refactor.

Bounded caches reuse exact centered coordinates and common width1/2 Lanczos coefficients. Coefficient memoization is limited to serial local nonlinear probes on frozen displacement axes; general geometry, large axes/kernels, changing-axis and accelerated coefficient generation retain original paths. Frozen references were not regenerated and numerical tolerances were not relaxed.

## Verification results

| Check | Result |
|---|---|
| Align registered suites | 69 passed, 0 failed, 2 blocked |
| JIP frontend tests | 61 passed |
| Native CTest groups | 16/16 passed |
| ASan/UBSan groups | Reliability, sampling, threaded resampler: 3/3 passed |
| Isolated complete nonlinear jobs | 100 exact fields; every worker exited; original memory limits passed |
| Frozen Python source | 19 modules match |
| Frozen native engines | Both libraries' code/constant sections match |
| Bundled JIP runtime | 256 Mach-O files; no external non-system or mixed Qt dependencies |
| Packaged Align assets | Exact source/ASAR/workers/slim runtime, arm64 and local signature checks passed |
| Companion discovery / trusted IPC | Full registration with the frozen sibling passed |
| Frozen region handling | Source/frozen matrices, fields, geometry, diagnostics and sampling match in both directions |
| GUI / CLI real-data workflow | Offscreen preparation, N4 for both inputs, linear, nonlinear reject/retry/accept, native export and restore passed |

The two blocked Align suites are test:browser and test:browser-matrix: configured browser executables are missing. Raw logs are retained. Other tests execute actual compiled UI modules, event handlers, workers and trusted IPC, but do not certify native graphical behavior. Native Cocoa launch was attempted and blocked by desktop services; offscreen execution is reported separately. Metal/CUDA devices were unavailable to this agent. ASan/UBSan passed with LeakSanitizer disabled on macOS; tracked native ownership checks independently returned zero retained resources. No current TSan result is claimed.

Native region tests independently calculate physical membership and objective references, cover all three plane-axis pairs with ignored depth, oblique/axis-swapped input geometry, copied inputs, CPU/threaded agreement, unchanged intensities, exact checkpoint continuation, invalid inputs, every creation allocation failure, cancellation/rollback and cache fallback. Protected Align window capture, interaction, processing, nonlinear, manual, session/export, guide and lifecycle suites also passed. Seventeen affected suites and exact emitted-preview checks were rerun after final code changes.

## Performance evidence

The final comparison ran one warmup and three interleaved measured repeats per route, using saved N4-prepared baby4 EPI/anatomical inputs and accepted linear initialization. Each fit used a fresh process. Timing includes startup, nonlinear fitting, diagnostics and transfer; it excludes preceding N4/linear estimation and Align rendering. No concurrent validation/build jobs ran during measured fits. All matrices, complete field bytes, field geometry and diagnostics match JIP0.8.2 exactly across the 16 warmup/measured jobs. The report records the native library SHA-256 used; it matches the final build.

| Route | JIP0.8.2 wall median | JIP0.8.3 wall median | Change | Sampled peak worker RSS, old → new |
|---|---:|---:|---:|---:|
| CPU | 10.232s | 10.041s | 1.87% faster | 144.05 → 141.67 MiB |
| Four CPU threads | 10.353s | 10.134s | 2.12% faster | 142.81 → 137.16 MiB |

This is a modest, approximately2% improvement on one case, not a general speed guarantee. CPU was still slightly faster than four threads. RSS differences are small and cannot establish a general memory reduction. Sampled worker RSS excludes whole-app/GPU memory. Over100 isolated fits, parent RSS after16 warmups grew393,216 bytes (0.375 MiB), span393,216 bytes, slope3466.90 bytes/job; original thresholds passed unchanged.

A separate synthetic CPU preparation benchmark on1,290,240 voxels measured506.945ms for full resampling versus0.155ms for cached display-geometry preparation (one warmup/five measured repeats). These operations deliver different intermediate results: a full rebuild still follows the transient preview. GPU uploads, drawing and native interaction latency were not measured; this is not an end-to-end speedup claim.

## Packaging and limits

Both self-contained Apple Silicon apps accompany complete shared source, component licenses, consolidated documentation, raw validation evidence, manifests and SHA-256 inventories. Keep Brainana Align.app and JIP Registration.app together, or explicitly choose the companion. The core Align app is251,332,224 bytes, below its350MB budget, without an extra standalone Node runtime. Core Align requires macOS13.5+; the companion requires macOS14+. Local ad-hoc signatures passed strict deep verification; Developer ID notarization and a second-computer installation test were not performed.

Final archive extraction, signature/IPC checks and a clean native rebuild from the delivered source are recorded separately beside the release archives after assembly. Those reports are listed in the external release-assets manifest/checksums when complete; they are not evidence until marked passed.

This candidate refreshes Apple Silicon only. Previous Intel macOS, Windows and Ubuntu binaries remain retained; refreshed builds/native testing are pending. JIP working grids remain capped at2million voxels/input, at least8 per axis, proper initial geometry and sufficient selected samples. N4 remains a standalone JIP preprocessing option; an Align N4 recipe is not added. Positive sampled Jacobians do not prove continuous invertibility or anatomical accuracy. Private subject images, fields, screenshots and sessions are excluded from release/source archives.


## Previous release validation (historical)

## 0.17.0 / JIP 0.8.1 — memory and registration integration

Apple Silicon candidate, 23 October 2026. Complete source and original component licenses accompany the release.

- Align validation: **67 suites passed, 0 failed, 2 blocked**. The blocked Chromium/Firefox/WebKit matrix attempts could not start because the configured browser executables are absent. Their raw failed-launch logs are retained. Native graphics/Metal are not counted as passed.
- Standalone JIP: **53 frontend tests passed**, including fresh-process lifecycle/error/cancellation tests. All 18 frozen Python modules match current source; both frozen engine libraries match the unchanged native build code/constant sections. Native algorithm code remains unchanged from the previously qualified 0.8.0 engine.
- Memory: 100 complete isolated nonlinear jobs, identical fields, every child exited. After the original 16 warmups, parent RSS grew **229,376 bytes (0.219 MiB)**; original span/growth/slope limits were preserved. Four 400-repeat controls show RSS growth even without engine calls. This fixes accumulation across production worker/benchmark lifetimes without claiming all persistent Python embeddings are leak-free.
- Native coordinate fixtures: six real bridge jobs cover linear/nonlinear/combined stages, both directions, anisotropy, obliquity, shear and axis-swapped/reflected voxel orderings. Align versus native pull error was at most **3.675e-7 mm** on these synthetic fixtures. Independent 500-point checks on baby4 measured **1.62e-07 mm pull error** and **9.88e-06 mm forward error**.
- Actual emitted UI: JIP pending proposals are accepted/rejected through main and Manual controls; Reject restores the preceding fit or empty state. Manual edits after acceptance remain usable and survive save/restore. This caught and fixed both the missing JIP manual-state validator entry and stale disabled controls after asynchronous nonlinear overlay completion. Guide/region observers now stop on page exit and ignore detached views.
- Native export worker: a known 0.2 mm JIP residual shifts original native 0.2 mm voxels by exactly one X voxel. It does not substitute altered working intensities or export the affine alone. Sessions schema12 retains the dense field and both manual-composition directions; older schemas still load. Malformed fields, nonpositive sampled Jacobians, unsupported ROI/mask combinations, oversized grids, stale inputs and pending exports are rejected explicitly.
- Trusted IPC tests reject foreign windows/subframes and arbitrary actions. Real worker tests cover singular execution, private temporary-file cleanup, unchanged input arrays and cancellation/error rollback. Packaged source/ASAR hashes and strict local ad-hoc signatures are checked separately.
- Frozen baby4 headless bridge: **11.93s**, positive sampled relative Jacobian **0.689–1.650**. This verifies real input execution and coordinates; it is not a claim of anatomical accuracy.
- Frozen offscreen Qt workflow: preparation, N4 for both inputs, linear fitting, nonlinear reject/retry/accept, native export and session restore passed. Shared GUI/CLI aligned pixels, deformation, Jacobians and affines matched exactly. The platform was **offscreen**, not a native interactive window.
- Complete CPU/four-thread benchmark: traces, aligned images, fields, Jacobians and native exports matched exactly; accepted state was unchanged. Measured median fit times were **20.35s CPU** and **21.97s four threads** for one measured repeat after warmup. Threads were slightly slower on this case. Individual fit-worker peaks were approximately 163–171MiB. GPU execution/speedup were not demonstrated in this sandbox.

Current integration scope: full working images, at most 2 million voxels per input and 8+ per axis; no JIP optimization-window/exclusion-mask support yet. Existing methods continue to honor those. N4 remains in standalone JIP; an Align N4 preprocessing recipe is not included. The core Align app uses a separately supplied self-contained JIP runtime to avoid duplicating its scientific dependencies. Keep both apps side by side or select the companion in the JIP section once.

Native Cocoa launch was attempted and blocked by pasteboard/desktop-service errors; raw logs are retained. Windows, Ubuntu and Intel builds are not newly issued in this Apple Silicon candidate. They retain their previously documented 0.8.0 qualification status. Local ad-hoc verification is not Apple Developer ID notarization or a second-computer installation test.

## 0.16.84 — Deep audit validation

The production TypeScript build additionally enforces noUnusedLocals/noUnusedParameters. The initial audit found 23 unused declarations/imports; these were removed without altering action routing.

Full run: 64 passed, 0 failed, 2 blocked. scripts/run-validation.mjs executes the build and every registered test suite. The packaged Validation/results.json is authoritative for final counts and individual durations. Browser/GPU suites blocked by the host are reported as blocked, never passed; native GUI interaction and representative real-anatomical accuracy remain pending.

Fresh browser attempts are retained: the configured Playwright browser is missing, and the installed system Chrome exits during launch before a test page can run. The blocked report states this observed limitation rather than reusing an older specific MachPort error.

New tests:

- test:pyramid-audit: 80 independent finite separable-filter reference cases, oblique/anisotropic/singleton grids, physical endpoints, input immutability, zero/positive-weight nonfinite behavior, malformed custom levels/sigma, sparse-mask range aliasing and valid/empty/constant-mask scoring.
- test:landmark-list: 1,000 unchanged refreshes preserve actual DOM identity/focus; replaced model objects still receive actions; point/residual/selection/pairing changes refresh correctly.
- test:worker-task: success/error/crash/unreadable/invalid/unknown messages, progress callback/creation/clone failures and early/late cancellation release workers/listeners and settle once.
- Existing exact compiled UI and welcome tests require the two removed headings to be absent and the welcome section to retain its accessible name. Existing registration/window/session/processing/network suites run against the updated source/emitted workers.

Evidence also includes pyramid-performance.json (CPU timing and exact finite-output hashes), viewer-comparison.json (pinned source inventory), packaged arm64 verification and extracted ZIP manifest/signature checks. Source correspondence and finite-pyramid hash equality do not establish end-to-end native speed, clinical accuracy or zero remaining defects.

## 0.16.83 — Additional desktop platforms

The same 0.16.83 source is packaged for Intel macOS (x64), Windows (x64 installer and portable ZIP) and Ubuntu (amd64 Debian installer and portable tarball), in addition to the Apple Silicon release. Platform verification checks exact frontend/worker contents, dependency resolution, architecture and installer payload integrity. Native Windows/Ubuntu launch, GPU behavior and installation/removal remain pending on their target systems.

## 0.16.83 — Readable guide paragraphs

61 checks passed, zero failed; two browser checks remain environment-blocked. Teaching checks verify formatted option paragraphs and literal HTML handling; actual compiled frontend checks verify formatted welcome copy. Packaged frontend/worker hashes, slim runtime, dependencies, Apple Silicon architecture and ad-hoc signature passed verification. Native GUI certification remains pending.

Separated the starting options into distinct paragraphs with bold labels for optional Initial processing, landmarks, manual adjustment and independent image-based registration. Applied the same approach to dense welcome, resolution, masking, voxel editing, landmark-count, spline, automatic registration, manual adjustment, refinement, region, proposal-review and export instructions. Short prompts remain compact. Guide routing and data actions are unchanged.

Guide text uses a small text-node renderer for paragraph breaks and bold emphasis, never HTML interpretation. Existing teaching checks verify the option labels, paragraph structure and literal HTML handling; actual compiled frontend checks verify formatted welcome instructions. Apple Silicon review release; native graphical certification remains pending.

## 0.16.82 — Restart shows starting choices immediately

61 checks passed, zero failed; two browser checks remain environment-blocked. Restart regression coverage includes all seven Close-panel prompts and all three loading/preparation prerequisites with an open sidebar. Packaged frontend/worker hashes, slim runtime, dependencies, Apple Silicon architecture and ad-hoc signature passed verification. Native GUI certification remains pending.

Restart guide cleared history but sidebar navigation immediately replaced the starting choices with the same Close Initial processing prompt, making it appear ineffective. Starting-choice screens and loading prerequisites now display immediately without first requiring sidebar changes. After a user chooses a concrete task, ordinary close/open/control teaching resumes. Restart keeps loaded images, transforms, landmarks and disclosure states, clears instruction history and pending continuations, and retains prerequisite/proposal/busy safeguards.

Added direct visible-step regressions from all seven Close-panel prompts, including the reported Initial processing case, without the auto-follow helper that previously hid this issue. Tests require immediate starting choices, stable refresh, empty Back history, preserved data/panels and resumed panel teaching after a new task choice. Registration choices after initial processing similarly precede panel navigation. Apple Silicon review release; native graphical certification remains pending.

## 0.16.81 — Remove guide Reset shortcut

61 checks passed, zero failed; two browser checks remain environment-blocked. Existing teaching and compiled frontend checks confirm the shortcut is absent and the main toolbar retains the reset confirmation. Packaged frontend/worker hashes, slim runtime, dependencies, Apple Silicon architecture, absence of the reset shortcut and ad-hoc signature passed verification. Native GUI certification remains pending.

Removed Show Reset workspace from the guide and its unused click handler. The main-toolbar Reset workspace button retains its confirmation and behavior. Restart guide continues to restart instructions while keeping loaded images and registration work. Updated existing guide and compiled frontend checks to require absence of the removed shortcut; package verification rejects bundles containing it. Apple Silicon review release.

## 0.16.80 — Dialogs take priority over the guide

61 checks passed, zero failed; two real-browser checks remain environment-blocked. Dialog regressions verify computed visibility/pointer handling and stacking in JSDOM, actual loading Escape handlers, and compiled frontend CSS/event behavior. Exact packaged frontend/workers, slim runtime, dependencies, Apple Silicon architecture and ad-hoc signature also passed verification. Native GPU/visual certification remains pending.

Replaced the Resolution-only guide visibility exception with a shared rule for all open application and native HTML dialogs. Loading either image, network browsing, the workstation browser and export-folder picker, Resolution, Settings and Export cannot be covered by the guide or its Resume button. All standard modal backdrops also stack above the guide. Instructions and manual placement are preserved; closing the last dialog refreshes the guide from current application state. Closing a dialog never resumes an intentionally paused guide.

Suspend guide projection, automatic positioning and focus/scroll changes while any dialog is open. Escape ownership is captured before a dialog handler closes it, preventing the same Escape from pausing the guide. Added real MRI/CT source-chooser, seven-dialog CSS/stacking, nested/replaced/removed/hidden/native dialog, focus/scroll and pause/resume regressions. Exact compiled frontend tests apply its actual stylesheet to reproduce the reported overlap for both image selectors and check the other dialog types. Native GPU/visual certification remains pending. Apple Silicon review release.

## 0.16.79 — Guide begins on the welcome screen

60 checks passed, zero failed; two browser checks remain environment-blocked. Startup integration and compiled frontend tests passed. Exact packaged frontend/workers, slim runtime, dependencies, architecture and ad-hoc signature passed verification. Native GPU/visual certification remains pending.

Starting the guide from either entry point keeps the welcome screen visible and highlights all four pairing choices. Only the user choosing a pairing opens the workspace. Pair selection never loads files. Welcome, missing images and pending preparation take precedence over remembered routes and Back; completed entry prerequisites are excluded from instruction history. Repeated welcome Guide keeps an active guide open. Docking reserves space on the welcome screen.

Integrated startup tests install welcome, guide, workflow and source chooser together, with both installation orders, both launchers, all four pairings and either header-selection order (32 sequences). They exercise cancel/retry, pause/resume, Restart, preparation failure/completion, plus four saved-session paths. Actual compiled frontend tests now start the guide before selecting a pairing and verify the welcome screen and highlighted choices remain visible. Prior bounded sidebar tests did not cover this integration boundary and were insufficient to detect the shortcut. Native GPU/visual verification remains pending. Apple Silicon review release.

## 0.16.78 — Guide ordering audit

59 checks passed, zero failed; two real-browser checks remain environment-blocked. Ordering coverage includes 1,536 disclosure/target combinations, 1,600 deterministic mixed actions, cancelled processing continuations, pause/resume preservation and proposal-arrival priority. Exact packaged frontend/workers, slim runtime, dependency resolution, Apple Silicon architecture and ad-hoc signature passed verification. Native GPU behavior remains pending.

Cancelled pending processing continuations on Back, Restart, guide completion and reset, while retaining pause/resume. Newly delivered registration proposals supersede older control-location and Back instructions; explicit Show Accept/Reject requests remain available until a decision. Rejecting a first proposal also clears an earlier Show Accept instruction. Added bounded ordering regressions across all 128 sidebar disclosure combinations, 12 control destinations and 1,600 seeded mixed actions. This is bounded orchestration testing, not proof of every possible action sequence or native GPU behavior. Apple Silicon review release.

## 0.16.77 — Visible guide heading frames

58 checks passed, zero failed; two browser checks remain environment-blocked. Exact packaged source/frontend/worker, architecture and signature checks passed. Heading tests cover clipped-panel CSS, initial three-cycle cue, steady-frame retention, explicit replay, reduced motion and no panel-toggle side effects.

Open/close-panel guide prompts now draw their accent frame inside the summary heading, so the panel’s overflow clipping cannot hide it. Each new heading flashes three times, then keeps its frame. Show heading and Follow replay the attention cue without opening, closing or executing any control. Reduced motion keeps the steady frame only. Apple Silicon review build; native graphical certification remains pending.

## 0.16.76 — Teaching walkthrough

Final validation: 58 passed, zero failed, two environment-blocked browser checks. Local-server, desktop lifecycle and real loopback SSH/SFTP tests passed after localhost access was granted. Apple Silicon package verification covers exact source/frontend/worker contents, architecture, dependencies and ad-hoc signature. Native graphical verification remains pending.

Added teaching-specific real DOM-event coverage for no guide proxy actions or disclosure mutation; close/open/field order; nested spline access; neutral landmark minimum; real proposal decisions; alignment inspection; reset location and explicit Finish processing. Pending brush tests prove guide navigation starts no worker and commits no data. Existing route/worker tests simulate human panel/control clicks using a test-only helper; dedicated teaching tests assert the intermediate prompts without that helper. Actual compiled UI verifies guide reset does not call the application reset handler. Native GPU verification remains pending when blocked by the execution environment.

## 0.16.75 — In-app network browsing

All-platform packaging update: macOS arm64/x64, Windows x64 and Ubuntu amd64, plus the separate macOS browser edition. Native Windows/Ubuntu GUI, GPU and installation/removal remain untested on this macOS host. Platform reports and installer-integrity reports are supplied in Validation.

Final release validation: 57 passed, zero failed, two environment-blocked browser checks. Exact packaged source/frontend, dependency resolution, architecture and signature checks also pass. Native workstation testing remains pending.

New regression coverage includes in-app profiles/connect/browse/load/reuse, slot selection, credential clearing, failed authentication, header-only browse tokens, backend token isolation, fixed methods, supported-file filtering, disconnect cleanup and SSH-profile parsing. A real loopback ssh2/SFTP server verifies known-host acceptance, unknown/changed/revoked/missing-key rejection and exact streamed image bytes. Native GPU and the user’s actual workstation authentication remain unverified.

## 0.16.74 — Local and network loading

0.16.74: data-source regression covers local-picker activation, pairing labels, connected-workstation selection, connection retry/error, unsupported-platform guidance, Escape and trusted fixed-action IPC. Full release checks and exact packaged source/frontend/worker verification are retained in Validation. Native GUI and real SSH authentication are not certified by DOM/adapter tests.

# Validation report — 0.16.73

## Welcome and hover validation — 0.16.73

The full suite records55 passed,0 failed,2 blocked browser/GPU checks. New welcome/hover tests exercise four pairings, duplicate T1 labels, safe filenames, guide/session/export labels, unknown/legacy profile handling, oblique nearest-voxel sampling and active-panel readouts without workers or voxel mutation. Production tests execute the exact frontend's welcome and profile restore/export paths. Actual processing/native export regressions test entered paint values after a mask, explicit later masking, Erase0 and binary-only mask painting. Existing scientific, preprocessing, physical-window, guide and session regressions remain required.

Pairing changes labels/metadata only; defaults and registration mathematics are unchanged. EPI-specific distortion correction and pairing-specific guided recommendations are not added. Native GPU appearance/hover latency and real anatomical accuracy are not certified here. Recipe6/schema11 retain legacy session reading while older app versions reject the newer schema rather than silently dropping paint order/profile metadata.

Current update (0.16.73): startup now presents a welcome screen explaining preprocessing, registration, review and export, with MRI–CT, T1–T2, T1–T1 and MRI–EPI choices. The choice labels both image slots across controls, directions, guide, preparation, review and export; duplicate T1 volumes are numbered. Pairing is saved in schema11 sessions and export metadata without renaming original files or changing coordinate mathematics/settings. Image Paint uses the entered intensity, Erase writes0, and mask Paint/Erase stay1/0. New recipe6 paint overrides honor the entered value even on a previously masked image; explicit later mask application can mask those painted values again. Editing shows the cursor's working-voxel intensity at the active panel's lower right, plus mask and pending values when relevant. Hover does not move the crosshair, mutate data, upload textures or start a worker. Release platforms: Apple Silicon and Intel macOS, Windows x64 and Ubuntu amd64, plus the separate macOS browser edition. Native graphical certification remains pending.

## Edit-target validation — 0.16.72

The full suite records54 passed,0 failed,2 blocked browser/GPU checks. Extended test:voxel-brush covers MRI and CT masks independently using actual processing/export workers, numeric radius input, original/image immutability during mask edits, post-hole-fill corrections, explicit mask application snapshots, native export replay, schema10 validation and corrected binary NIfTI values. Actual threshold UI regeneration preserves corrections and Clear removes them. Existing mask/topology, session/nonlinear, physical-window, guide and compiled frontend tests remain required. Packaged checks run the expanded brush test against emitted app workers and inspect four target choices plus numeric input in the exact frontend.

No native GPU or real anatomical accuracy certification is added. Mask corrections use the original working grid and do not gain native resolution detail during export. The total20,000-edits-per-mask cap and radius0–16 guard remain explicit. A mask must exist before selecting its edit target; editing does not automatically apply it to intensity data.

Current update (0.16.72): Voxel editing has its own explicit Edit target selector: MRI image, CT image, MRI binary mask or CT binary mask. Generated-mask targets become available after creating that mask. Image actions and binary Paint1/Erase0 are separate. Mask edits update the generated selection only, never source intensities or an already applied mask snapshot; explicitly Apply generated mask to working image to use the corrected selection on image data. Radius is a numeric integer field from0–16 working voxels;0 selects a single voxel. Click/drag previews remain sparse and worker-free; Apply/Finish commits once. Manual mask corrections run after hole filling and survive regeneration, sessions and binary/native-processed exports. Clear mask removes corrections. Recipe5/schema10 retain compact original-working-grid overrides and reject malformed/nonbinary or out-of-grid entries; older schemas remain readable. Apple Silicon macOS only; native GPU interaction remains unverified here.

## Voxel brush validation — 0.16.71

Release validation: **54 passed, 0 failed, 2 blocked**. The browser/GPU checks remain blocked by the host sandbox; no native visual latency or clinical accuracy claim is made.

The added real-controller/real-worker test verifies zero worker starts or commits during clicking and interpolated dragging, a visible bounded 2D preview, one processing worker for a completed batch, identical original arrays, 3D spherical neighborhoods at image boundaries, crop clipping on oblique grids, sparse stroke Undo/Redo, atomic 20,000-voxel capacity rejection, unrelated pointer releases, cancellation/failure retention, discard and exactly-once apply-before-navigation. Existing six-plane edit tests now apply each staged batch explicitly. The complete session, mask/hole-filling, physical-window, registration, guide and compiled frontend suites remain required.

The packaged verifier runs the same brush regression against the app's emitted processing worker, checks new control labels and exact frontend/runtime hashes, and verifies arm64 architecture/signature. Release archives retain the full source, seven consolidated docs, raw validation logs/results, manifests and SHA-256 checksums. Commit still incurs a full reconstruction/texture update once per batch; memory allocations at commit are unchanged. Unapplied previews are not exported or automatically recovered; Apply/Finish or tool/save button navigation commits them first. Apply or discard before closing; unapplied drafts are not retained.

Current update (0.16.71): voxel editing now collects sparse click/drag strokes with an immediate bounded orange slice preview, without starting a worker, scanning/copying a whole volume or reloading source textures per pointer event. Single voxel remains the default; 3D spherical brush radii of1/2/3/5/10 working voxels edit surrounding regions. Sphere dimensions follow the working voxel grid and may be physically anisotropic. Apply edits or Finish rebuilds the image once for the batch. Other button actions first apply the batch and replay once after success; input-file selection requires Apply/Discard first. Failed/cancelled application preserves the draft for retry. Undo/Redo operates on strokes while editing and on the applied batch afterward; Discard removes only unapplied edits. Native/base arrays, geometry, landmarks, fit and optimization windows remain unchanged during painting. Unapplied drafts are not retained after closing the app. The existing total20,000-voxel per-image recipe cap remains; use masks for larger regions. Saved/exported data use the committed recipe, never preview pixels. Apple Silicon macOS only. Native graphical responsiveness is not certified in this sandbox; committing still performs one full reconstruction and texture update, including mask/hole recomputation where applicable.

## Audit evidence for 0.16.70

The completed suite records **53 passed, 0 failed, 2 blocked** in Validation/results.json. The two blocked checks require Chromium/native graphics and are retained as blocked, not passed. Tests include real compiled DOM actions and emitted workers; they do not certify GPU rendering.

New test:audit reproduces selected-affine spacing disagreement, finite samples beside NaN/Infinity, unchanged-grid crop navigation and invalid voxel input; executes all four real worker controllers with a throwing postMessage and verifies cleanup/busy recovery; checks original-model reset and distinct original/current export diagnostics; injects install/rollback/cleanup failures into the same overwrite helper used by local and SFTP saves; and performs 320 deterministically seeded exact rigid/similarity/anisotropic/affine recoveries. The compiled production test additionally rejects a legacy pending proposal back to its accepted manual pose and resets a promoted affine fit to its original rigid metadata.

The optional original-model field is validated while allowing a spline to retain an earlier linear landmark-origin label. A stricter initial check rejected that compatible lineage in two synthetic restore regressions; it was corrected, and all affected session/nonlinear/manual/automatic/compiled tests were rerun successfully. The obsolete hidden refinement path now delegates to the current constrained worker. No additional intensity-search algorithm is retained in main.ts.

Packaged verification checks exact ASAR and slim workstation sources, emitted worker/frontend checks, arm64 architecture, ad-hoc signature and installed size. It also imports the actual slim server runtime through embedded Electron Node to catch missing helper dependencies. Archive checksums, fresh extraction, internal file/symlink inventories and signature verification are required before delivery. Native macOS graphical launch, real MRI/CT accuracy, long-duration/large-allocation stress and other OS releases remain outside this certification.

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

Date: 2026-10-15. Build ID: `manual-alignment-start-1`.

**52 build/check commands passed; zero assertion failures; two native-browser checks blocked by the execution environment.** This is a desktop validation candidate, not complete native graphical or anatomical certification. Machine-readable results and logs are included under `Validation/` in the app, source and full release bundles.

The current Ubuntu release run has authorized network access, allowing the shared loopback backend/integration tests to execute successfully. Two native-browser checks remain blocked by the macOS bootstrap/MachPort restriction previously observed here. The collector records them as blocked, not passed. Native Linux execution is unavailable on this macOS host; Docker is installed but its daemon is not running. No native Ubuntu launch or install/remove pass is inferred from shared tests.

Current change checks: workflow-local mirrored region editors preserve canonical physical bounds and local/global guide routing. MRI/CT display controls cover negative and constant intensities, invalid endpoints, independent modalities, contrast/reset, newly rebuilt viewers and immutable voxel buffers. Packaged compiled UI checks require the sliders in Initial processing and region controls in both registration panels. GPU appearance and interactive slider latency remain pending a native graphical run. Display ranges are launch/dataset-local, not serialized session fields.

## Executed evidence

| Area | Evidence |
|---|---|
| Build and identity | TypeScript/Vite build; generated release identities, documentation and source architecture |
| Four linear models | Exact rigid/similarity/anisotropic/affine recovery, inversion, noisy fits, degeneracy and reflection decisions; small, 90°, oblique, near/exact 180° cases |
| Physical coordinates | qform/sform, oblique/anisotropic geometry, negative qfac, units normalization, forward/reverse directions, resampling and actual NIfTI bytes |
| Optimization windows | All six MRI/CT planes, physical slice input, mirrored viewports, CSS/backing scaling, outside release, cancellation, invalid replacement retention, zoom/pan persistence, sessions and worker axes |
| Nonlinear fitting | Exact nonlinear and affine reproduction, smoothing, forward/reversed correspondence, duplicate/coplanar/reflected rejection, safety attenuation of severe deformation, same-map inverse round trips beyond controls |
| Full nonlinear mapping | Explicit post-spline manual composition; known physical-space intensity image improves under nonlinear resampling; native oblique reference-grid pull mapping applies each image affine once |
| Nonlinear persistence | Schema-4 coefficients, controls, smoothing and proposals; coefficient/control tamper rejection; recalculated RMS; older linear migration; JSON export refuses to imply a matrix represents the warp |
| Vector fields | Actual little-endian forward/inverse NIfTI vector bytes, intent 1007, components in dim5, RAS-mm values, sform/units, oblique grid endpoint preservation and allocation limits |
| Emitted workers | Actual built linear and nonlinear JavaScript workers executed in Node worker threads; fit/resample/field operations, progress, error propagation, transfer-safe input buffers, cancellation and termination; local intensity refinement preserves spline coefficients |
| Production controls | Actual built frontend executes in DOM/VM tests: four linear labels and advanced spline option, rigid default, smoothing/caution visibility, original exact fits/export/reflection behavior, settings, reset and guide controls |
| Sessions/settings | Schema compatibility, matrix checks, native preference authorization/atomic persistence adapters, recovery controls, palette separation, defaults separation and reset/cancel guards |
| Guide | Load/pair placement, model minima, proposal review, skips, route/operation selection, restart/resume state and post-refinement choices |
| Runtime/source | Browser capability, structured SFTP/platform, Mach-O, launcher/lifecycle source and non-socket adapter checks |

The spline uses a normalized regularized 3D polyharmonic distance kernel with an affine polynomial tail. A proper affine base and conservative residual-gradient bound of 0.65 make the residual inverse contractive. Unsafe unconstrained deformations are attenuated and reported; exact interpolation is not forced at the expense of folding. This is an analytical property of the continuous map, not a guarantee of anatomical accuracy. The distance kernel is not differentiable at control knots; displayed local Jacobians are finite-difference estimates and the analytical Jacobian lower bound applies where differentiable.

The UI requires 12–128 distributed pairs and recommends 20+. These are application safeguards, not established universal clinical thresholds. Incorrect landmarks, sparse coverage and extrapolation remain risks. Use independent validation points and inspect all slices. No patient/animal dataset accuracy benchmark, native GPU output or measured workstation responsiveness is claimed.

Local image refinement adjusts the existing spline as a whole with the selected linear model; it does not optimize spline coefficients from intensities. Global registration proposes a new linear alternative. Native-grid image export evaluates the actual spline inverse, while exported vector fields are sampled approximations (2 mm spacing by default). External field interpolation does not automatically inherit the continuous map's safety guarantee. RAS/LPS coordinate conventions require explicit handling by downstream software.

## Export feedback and proposal gates

The reported pending automatic proposal was confirmed from the saved session without modifying user data. New DOM/API-adapter tests check disabled image/transform buttons, visible proposal requirement/review action, allowed session saving, busy/repeated-click guards, visible resampling/write errors, authorization, stopping a multi-file batch after failure and successful retry after queue recovery. The exact production-bundle transform-save click handler also shows an injected destination error inside the modal and successfully retries. Existing export/reflection/session and image-resampling regressions remain required.

## Guide placement and attention

The guide regression checks actual DOM interactions and element rectangles: clear highlighted control and active MRI/CT panes, new position on modality change, reserved docking when crowded, outline pulse removal after 1.8 seconds with persistent highlight, and reduced-motion suppression. The existing synchronization/reset/drag/window tests remain required. These are DOM/geometry checks; native graphical verification is separately pending.

## Guided-action fix

The real DOM-event guide regression covers direct workflow tabs, global/local operation changes, finishing window editing without Next, preservation of global operation, parameter editing without premature progression, guide-generated proxy clicks without feedback, successful landmark fitting, automatic and spline accept/reject, same-step title refresh, landmark-model changes, and hidden-guide route preservation. Existing loading/error, model minima, restart/reset, movable guide and proposal tests also pass. Scientific fitting, optimization-window capture/geometry and image sampling were not changed.

## Packaged platform verification

The release includes separate macOS arm64/x64 apps, Windows x64 installer/portable package, Ubuntu amd64 .deb/portable package, and optional macOS browser edition. Platform verifiers check actual packaged source/assets and emitted workers against the current build, all original 6/7/9/12 plus nonlinear options, preferences and relevant menu adapters. macOS signatures/architectures and size budgets, Windows PE branding/ASAR integrity, and Linux ELF/desktop identity are checked. Installer verifiers compare every embedded application file against the verified unpacked app; Ubuntu also checks dependencies, root ownership and install/remove script paths/syntax. No default sandbox bypass is introduced.

The complete shared source, consolidated docs/changelog, per-platform full bundles, validation evidence, manifests and SHA-256 checksums are shipped. Archive/internal/external hashes and extracted executable permissions are verified. Windows executables are unsigned; macOS apps are ad-hoc signed, without Developer ID/notarization. No new dependency or scientific algorithm was added.

## Target-machine checks still needed

Native Windows and Ubuntu launch/install/remove, macOS graphical launch, GPU/WebGL2 and real MRI/CT guided interactions remain pending. This host is macOS, and native browser bootstrap is restricted. Shared loopback backend assertions pass with the session's authorized network access; they do not establish native graphical behavior on another OS. Anatomical accuracy, large-volume performance and actual workstation authentication remain separate target-machine checks.

## Workflow separation — 0.16.48-resolution.1

The workflow separation is covered by real DOM actions: global/local host selection, legacy landmark eligibility, geometry/global rejection, shared region proxy actions and summaries, active-edit preservation across tabs, separately remembered histogram bins, global region guide/pause/resume, and landmark-only spline/refinement routes. Worker regressions verify the local request limits, paired-window axes and preserved nonlinear coefficients. Guide Back/Resume restore matching workflow hosts; independent results are not labeled as completed landmark fits. Native GUI/anatomical validation remains pending; higher NMI is not proof of anatomical correctness.

## Working/native separation validation

New regressions cover compressed/uncompressed NIfTI-1/2, big/little endian scalar decoding, slope/intercept, selected frames, independent Niivue qform/micron interpretation, oblique endpoint/axis preservation, normalized filtering of constant data, shared read-only buffers with independent IDs/headers, malformed/truncated data, memory recommendation monotonicity, and native affine/full-spline output bytes. Actual emitted preparation/export workers run in Node worker threads. UI DOM tests cover header-only selection, joint live planning, guided preparation waiting, atomic commits, resolution/source differentiation, failed decoding and cancellation. Native-export worker tests deliberately corrupt working voxels and verify native-file fidelity and native reference dimensions, no fixed pixel payload, progress and no destination write after cancellation. Restricted resource IPC rejects forged senders.

This does not certify GPU texture sharing, real native UI appearance, all compression/format variants, anatomical accuracy or execution times. Viewer/GPU contexts remain separate; non-NIfTI streaming, native slice-on-demand, persistent worker caches and fully streamed destinations are not claimed.

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

Manual regression evidence: independent noncommuting affine composition in both directions; exact centered 12-parameter values; spline pre/post composition and inverse; invalid bounds/atomic edits; schema9 save/reload/tamper rejection; export metadata; explicit-target UI/reset; shared-array source preview and camera restoration; emitted forward/reverse resampling worker. Packaged verifiers rerun these tests against extracted workers.

Current release scope: Apple Silicon macOS only. Regression tests exercise real guide DOM transitions, four processing targets/Back, completed-stage collapse and inspection, manual/point exceptions, deterministic opacity timestamps/stops, output presets/FoV/geometry, interpolation ramp values, actual native export worker with projected masks/crop/edits, original immutability and exact packaged assets. Native graphical execution is blocked; no visual certification is claimed.

Current regression evidence includes local/global/spline decisions via guide and inline manual controls, exactly-once canonical delegation, no silent changes before decision, manual unlock and first-proposal rejection, stable minimum landmark target and neutral Apply, updated animation timestamps, and actual emitted app session restore → pending refinement → accept/reject → working manual nudge. Packaged Apple Silicon verification repeats these checks. Native graphical confirmation remains pending.

## 0.16.73 all-platform packaging

The same validated 0.16.73 application is packaged for macOS arm64/x64, Windows x64 and Ubuntu amd64. The separate macOS browser edition includes both Node architectures. Package reports verify architecture, exact frontend/worker/source contents, menus and installer payload integrity. The 55 passed checks and two environment-blocked checks remain applicable. Native Windows/Ubuntu GUI, GPU, installation and removal have not been executed on this macOS host; macOS GUI certification remains pending. See Validation/packaging-*.json, installer-integrity.json and deb-integrity.json in the release bundles.
