# JIP cross-platform implementation and verification plan

Version: 1.0 · Prepared: 2026-10-21 · Status: proposed implementation plan; execution has not begun.

## 1. Objective and definition of completion

Deliver one maintained 64-bit JIP numerical codebase that runs natively on Apple Silicon macOS, Intel macOS, Ubuntu x86_64 and Windows x86_64. Preserve the repaired nonlinear registration approach. Every platform must operate without GPU hardware or a GPU development toolkit. Optional GPU backends may be enabled only on OS/device combinations with passing validation.

Completion is demonstrated separately for: (a) portable CPU registration, (b) CPU optimizations, (c) each GPU backend/platform, and (d) distributable packages. A successful compilation, cross-compilation or isolated kernel benchmark does not establish that a complete registration works. A skipped GPU test does not establish GPU support.

The first portable product is the shared registration engine plus a native command-line runner and compatible existing CLI tools. Existing X11 frontends remain supported as explicitly labeled compatibility components where built. They are not represented as new native Windows graphical applications. Integration into Brainana Align and self-contained Align application releases are separate application changes and must pass the same platform matrix if undertaken. This plan does not silently replace either interface or registration method.

## 2. Baseline and non-negotiable constraints

Use the reviewed JIP source from `work/jip-performance-2026-10-21/source`, including the additional wide sample-counter correction. Create an immutable baseline revision before implementing further changes. Retain the original supplied archive and both repair histories.

Existing evidence includes Apple Silicon builds of the six active tools, synthetic nonlinear/CLI/engine cases, source audits, large allocation/index/file checks, a follow-up counter regression and bounded CPU performance experiments. The later counter correction has its own targeted and engine checks; that is not a new six-platform release. Intel macOS, Ubuntu and Windows execution evidence is still absent. The Metal fixture found no device in the available environment and did not execute its shader.

Preserve:

- Basis functions, hierarchy, coefficient order and finite-difference definitions.
- Line probes, combined-direction search, acceptance/rejection and topology checks.
- JIP's centered-millimeter pull-map convention, `A*x + u(x)`, and separate native-image metadata.
- Lanczos/Gaussian interpolation, normalization, wrapping, boundary and zero/nonfinite rules.
- Existing 50-bin MI normalization, uncentered global NC, bending/stiffness penalty and Jacobian conventions.
- Fixed on-disk sample widths, compatibility with prior saved fields/parameters and explicit transform direction.
- Checked 64-bit allocation lengths, products, flat indices and file offsets; no dependence on Windows `long` being 64-bit.

FireANTs is an architecture reference for fused operations, backend separation, buffers and index dispatch. Its optimizer, deformation model and default losses/interpolation are not replacements for this contract. No new learned model, optimizer, coarse-level omission, sampling approximation or altered stopping rule enters as an “optimization.”

## 3. Platform and hardware matrix

| Target ID | Native CPU target | Proposed toolchain | Automated CPU lane | Additional release evidence |
|---|---|---|---|---|
| MAC-ARM | macOS arm64 | Apple Clang, C11 core; C++17/Objective-C++ adapters as needed | Explicit arm64 macOS image | Physical Apple Silicon Mac; clean package launch; Metal device when backend enabled |
| MAC-INTEL | macOS x86_64 | Apple Clang, matching x86_64 dependencies | Explicit Intel macOS image | Physical Intel Mac; no Rosetta substitution; Metal tested separately if enabled |
| LINUX | Ubuntu x86_64 | GCC primary, Clang diagnostic lane | Ubuntu 24.04 primary; 22.04 compatibility lane | Native Ubuntu execution; NVIDIA GPU lane for CUDA |
| WINDOWS | Windows x86_64 | MSVC-compatible toolchain preferred; evaluate MSVC/clang-cl during portability milestone | Windows x64 runner | Windows 11 native execution; NVIDIA GPU lane for CUDA |

Initial CI labels can use explicit `macos-15` arm64, `macos-15-intel`, `ubuntu-24.04`, `ubuntu-22.04` and `windows-2022`, subject to repository availability. Verify actual process architecture in each job. Runner labels identify build environments, not minimum supported desktop OS versions. GitHub's current [runner reference](https://docs.github.com/en/actions/reference/runners/github-hosted-runners) lists the architectures; labels and images must be rechecked when implementation starts.

M0 establishes a support ledger with exact tested OS, compiler, SDK and dependency versions. Proposed minimum desktop/OS support must be validated, not inferred from a CI image. A Windows Server CI runner does not replace Windows 11 package testing. Ubuntu compatibility includes libc/runtime baselines. GPU toolkit/host-compiler combinations must follow the chosen toolkit's published [Windows](https://docs.nvidia.com/cuda/cuda-installation-guide-microsoft-windows/) and [Linux](https://docs.nvidia.com/cuda/cuda-installation-guide-linux/) compatibility tables.

Required hardware access: one physical Apple Silicon Mac, one physical Intel Mac, Windows 11 and Ubuntu native test environments, and NVIDIA hardware accessible on both Windows and Ubuntu before claiming CUDA support there. Intel Mac GPU support is optional until that actual device is validated. Standard hosted runners must not be assumed to expose usable GPU devices or enough memory for heavy tests.

Ubuntu ARM64, Windows ARM64 and additional Windows/Linux AMD/Intel GPU backends are later extensions. CPU coverage still applies to x86_64 machines with AMD/Intel graphics.

## 4. Proposed component boundaries

Keep a C11 numerical implementation behind a small C ABI. Use opaque contexts and neutral data descriptions so C++ adapters do not include legacy headers containing identifiers such as `template`. Avoid converting every C file to C++.

| Component | Responsibility | Dependency rule |
|---|---|---|
| Core/context | Images/views, geometry, basis/state, lifecycle and errors | No X11, UI framework, CUDA or Metal requirement |
| Reference CPU backend | Canonical serial resampling, penalties, metrics and constraints | Defines the comparison behavior |
| Optimized CPU backend | Same calculations with guarded loop/buffer improvements; later voxel threading | Complete fallback to reference CPU |
| Metal backend | Validated operations on supported Mac devices | macOS-only adapter and shader sources |
| CUDA backend | Validated operations on supported NVIDIA devices | Optional module; CPU launch works without CUDA libraries/drivers |
| I/O/session layer | Files, transforms, metadata, restore/export | Explicit fixed/moving conventions and checked byte offsets |
| Frontends | Native runner, existing CLI tools, optional X11 UI, future Align adapter | Call core API; do not own numerical globals |

Each registration context owns its accepted state, proposal buffers, metric caches, scratch and backend choice. Specify image ownership, buffer sizes/strides, aliasing, replace versus accumulate semantics, geometry versions, cancellation and callbacks. Results must not depend on mutable process-wide backend flags. Preserve scalar execution order during extraction; changing to threads or GPU work is a later milestone.

The present CMake `JIP_BUILD_X11=OFF` omits the nonlinear aligner; it does not extract a standalone nonlinear engine. That is a known extraction task, not an already available headless product.

## 5. Implementation milestones and exit gates

### M0 — Freeze the reference and establish verification infrastructure

Deliver an immutable baseline source revision, dependency inventory, support ledger, fixture manifest/checksums, numerical-contract document and reproducible reference outputs. Record CPU/GPU/memory/OS/toolchain details with every result. Preserve the prior CPU experiments as experiments; do not enable finite-only interpolation pruning without its input guard.

Move portable regression fixtures toward normal test targets instead of relying entirely on extracted function bodies and stubs. Keep isolated sanitizer probes for failure injection, but add tests that call the production API directly. Introduce CMake presets, CTest labels and machine-readable results; [CTest](https://cmake.org/cmake/help/latest/manual/ctest.1.html) provides a common runner without forcing a new application runtime.

**Exit gate:** the baseline reproduces its recorded Apple Silicon results; every retained test has a stable ID and scope; four CPU build jobs and hardware prerequisites are specified; proposed tolerance rules and reference fixtures are frozen before optimization.

### M1 — Extract the headless numerical engine

Inventory functions/state currently shared with `align.h`, `align-Xwindows.c`, display globals and callbacks. Move engine initialization, images, `AlignNL`/`AlignAid`, static objective/penalty caches and lifecycle into an explicit context. Replace progress/display calls with callbacks. Define globals once or remove them; stop relying on common-symbol linker behavior.

Add a minimal native runner that loads images, configures the same method, executes registration and saves results. Adapt the existing X11 aligner to use the same engine; preserve its behavior. Extraction should not introduce algorithmic cleanup, compiler fast-math, concurrency or GPU changes.

**Exit gate:** the library and runner build/run without X11; complete reference registrations and intermediate objective/coefficient traces match baseline on the same toolchain; independent/repeated contexts do not share caches; X11 compatibility still builds on its supported configurations.

### M2 — Establish native CPU execution on all four targets

Replace or isolate platform-specific file seeking, paths, environment, timing, threading and process helpers. Review POSIX-only includes/functions and legacy compiler extensions. Retain 32-bit raw pixel formats while widening sizes independently. Use a single dependency manifest and build suitable 64-bit FFTW/zlib dependencies for each target.

Evaluate native MSVC-compatible compilation first because the optional Windows CUDA lane must use a compatible host toolchain. A MinGW/Cygwin build can remain a clearly labeled compatibility path; it does not satisfy the native Windows engine gate if it requires an X server or POSIX runtime. Record the selected Windows toolchain and library ABI before expanding the CUDA work.

Port test infrastructure too: remove absolute Homebrew/XQuartz assumptions from portable tests; replace macOS-only RSS/`vm_stat` checks, POSIX stack controls and sparse-file assumptions with platform adapters. Test filenames with spaces and non-ASCII characters, long paths, read-only inputs, permission failures, compressed/uncompressed NIfTI and Windows drive/UNC paths without assuming a live network share.

**Exit gate:** all required CPU tests run natively on MAC-ARM, MAC-INTEL, LINUX and WINDOWS; saved results can be exchanged across platforms; no required result is a skip; missing-GPU startup succeeds. This is the first portable CPU release candidate.

### M3 — Improve CPU efficiency without changing the method

Measure the actual M2 Release binaries first. Introduce changes separately: optional LTO, guarded exact-zero interpolation pruning, row/plane stride hoisting, reusable per-context scratch and removal of unnecessary intermediate resampling between component updates. Preserve all validation/rollback behavior. Then evaluate parallel voxel tiles, private histogram accumulators and fixed-order statistics reduction; leave overlapping coefficient updates sequential.

For each change, run reference comparisons and profile complete registrations in addition to kernels. Keep a switch to the reference implementation. Measure memory as well as elapsed time; do not replace compact histograms/basis factors with large dense tensor intermediates.

**Exit gate:** correctness/behavior criteria pass on all four CPUs; no unexplained objective/acceptance differences; performance and peak-memory evidence are published. Slower cases can retain the reference path. A faster kernel alone is insufficient.

### M4 — Implement and qualify the Metal backend

Implement the region-aware fused coordinate/displacement/resampling operation first, retaining JIP interpolation and physical conventions. Reuse device buffers/pipelines and preserve wide/tiled offsets. Add robust capability checks, memory budgets and accepted/proposal rollback. Handle full-volume operations and small regions separately.

Verify on physical Apple Silicon hardware; use CPU fallback where the workload or capability is unsuitable. Qualify Intel Mac Metal independently on actual supported hardware before enabling it there. Only then evaluate the original penalty/Jacobian/statistics operations. Do not substitute trilinear sampling, low-precision statistics or different losses.

**Exit gate:** GPU numerical and failure tests pass on every enabled Mac/device combination; full registrations meet the numerical contract; CPU-only launching/fallback pass; end-to-end workload timings justify automatic GPU selection. Unverified devices use CPU.

### M5 — Implement and qualify CUDA on Ubuntu and Windows

Use the same operation/context contract and fixtures as Metal/CPU. Package CUDA as optional acceleration; importing the main engine or starting the CPU runner must not require a CUDA installation. Validate host-compiler compatibility, runtime loading and per-device limits. Preserve equivalent buffers, coordinates, statistics and rollback semantics.

**Exit gate:** tests pass on actual NVIDIA hardware under both Ubuntu and Windows. Success under one OS does not certify the other. No-driver, unsupported-device, allocation-failure and forced-CPU cases work. Publish driver/toolkit/GPU details for the tested support set.

### M6 — Validate complete workflows, interoperability and performance

Run complete multilevel registrations, including order selection and convergence, on synthetic truth cases and representative paired datasets. Include varied size, contrast, voxel spacing, orientation and overlap. Use shareable non-sensitive datasets or fixtures whose use is authorized. Record CPU/GPU results, objective histories, accepted states, Jacobian ranges, resampled images and session/transform restore.

Test load → configure → run → cancel/retry → save → reopen → export sequences and error recovery. Test simultaneous independent jobs once contexts/threading exist. Verify numerical-core behavior separately from frontend/GUI interactions.

**Exit gate:** reference comparisons, platform comparisons, soak/failure tests and final benchmarks pass; no unresolved correctness/size/address failures; supported configurations have real execution evidence. All deviations have an explanation and disposition, not a blanket “within tolerance.”

### M7 — Package, test delivery and publish the validation record

Produce separate macOS arm64 and x86_64, Ubuntu x86_64 and Windows x86_64 packages from the same source revision. Include required runtime dependencies and dependency/license/version records; avoid developer-machine library paths. Label optional legacy X11 tools and their dependencies explicitly.

For a self-contained macOS application distribution, use signing/notarization when credentials are available and test a genuinely downloaded/quarantined copy on a second Mac. An ad-hoc local launch is not proof that another user's Mac will accept the download. Windows packages must be tested on a clean system without development tools; verify DLL loading and optional CUDA absence. Ubuntu packages must be tested against the stated runtime baseline, including a clean container/VM and native execution where appropriate. Signing credentials are an external release input, not a reason to leave code/tests unfinished.

Deliver complete shared source ZIP, exact revision/patch history, consolidated documentation, changelog, per-platform validation report, fixtures/manifests, dependency records and SHA-256 checksums. A combined release bundle contains every published artifact. Use numeric release versions. No platform or GPU combination is listed as supported solely because its binary was generated.

**Exit gate:** clean-install/run/output/restore/export checks pass for all four packages; source archive rebuilds; all checksums verify; the support table accurately reflects executed tests. If only an engine/CLI release is built, do not label it a self-contained Brainana Align application release.

## 6. Verification protocol and numerical acceptance

The companion CSV is the required scenario/platform matrix. It applies the same fixture definitions across platforms; additional backend cases apply only where that backend is advertised. Results distinguish PASS, FAIL, RESOURCE_SKIP, UNSUPPORTED and NOT_RUN. None of the latter three satisfies a required release test.

Use three comparisons: analytic synthetic truth, the immutable serial reference, and full-workflow consistency after local updates/restore. Final RMS or a visually plausible overlay alone cannot detect an altered objective, direction error or stale cache.

### Proposed tolerance policy, to freeze in M0

These are engineering starting points for bounded fixtures, not established accuracy claims. Establish input ranges and operation-specific error bounds before implementing fast paths. Do not loosen thresholds merely to obtain a passing speed result.

| Quantity | Initial criterion | Additional condition |
|---|---|---|
| Counts, shapes, indices, inclusion/invalid flags, fixed file fields | Exact | Values beyond 2/4 GiB included |
| Unmodified serial operations on the same target/toolchain | Bitwise match wherever operation order is unchanged | Any mismatch is investigated |
| Geometry mapping on fixtures with FOV ≤512 mm | Maximum error ≤1e-4 mm, with explicit float-ULP analysis | For other ranges use a justified scale-aware bound |
| GPU/kernel intensity samples | Maximum error ≤1e-5 × max(1, maximum absolute reference intensity) | Exact expected zero/nonfinite classification; inspect boundaries separately |
| MI/NC and normalized objective components | Absolute difference ≤1e-6 on bounded fixtures | This does not authorize different search decisions |
| Full-workflow displacement on bounded reference fixtures | Proposed maximum deviation ≤0.01 of the smallest working voxel spacing | Validate per-case objective, direction, topology and mapped-point error; stricter analytic cases retain stricter bounds |

Acceptance/rejection and Jacobian decisions may have tighter margins than these aggregate tolerances. In strict mode, reference-evaluate any potentially ambiguous branch, including would-reject comparisons; if a reliable error bound cannot establish equivalence, keep that decision/computation on the CPU. Rechecking only accepted proposals is not enough to preserve rejected-candidate behavior. No test may permit new folds or changed validity classification merely because average image error is small. A GPU/backend failing its contract remains disabled.

## 7. Memory, failure and performance evidence

Run cheap size/offset/overflow tests in all CPU lanes. Run actual >2 GiB resident-memory and >4 GiB-offset/large-object tests on adequately provisioned release machines for every target. Resource checks must precede heavy allocation; allocation admission can differ between virtual-memory models. A safely skipped heavy test on a small hosted runner is recorded, then rerun on suitable hardware before platform sign-off.

Distinguish reserved virtual bytes, committed/private memory and peak resident memory. Use OS-specific measurements with normalized byte units. Sparse files must actually use a supported sparse mechanism; do not accidentally create huge physical files on a platform where seeking is not enough. Preserve safe failure on negative/overflow lengths and bounds exceeding external APIs.

Failure injection covers allocation failure, incomplete I/O, malformed headers/fields, unavailable GPU, device/dispatch errors, cancellation and incompatible restores. Accepted state must remain intact; retry/fallback starts from valid state and produces a clearly reported outcome.

For benchmarks, record warmup, initialization/I/O/compile time, complete registration time, per-stage time, iterations, accepted coefficients, peak memory, device/driver versions and input/output hashes. Use repeated interleaved comparisons on the same host and report median/range plus outliers; distinguish elapsed from process CPU time. Include a small, medium and large workload and small/local versus full-volume operations.

Proposed promotion rule: Auto selects a GPU/optimized route only when correctness passes and a representative end-to-end operation or candidate workload shows at least a 1.2× median benefit with a stable margin; whole-workflow results must also show no material regression. Investigate median regressions above 5% against that machine's baseline. These are implementation selection criteria, not promised performance. Validate memory budgets and repeated-run stability; no unexplained steady growth across a 50-job soak.

## 8. CI and execution cadence

- **Every numerical/API change:** four native CPU build/test lanes; small geometry/I/O/integration cases; boundary arithmetic; forced-CPU startup. Add GCC/Clang diagnostics and platform-appropriate sanitizers. Unsupported sanitizer combinations receive another concrete diagnostic lane rather than a false pass.
- **Extraction/cache/thread changes:** reference traces, repeated/independent contexts, local-versus-full objective tests, rejection/restore and race checks. Run thread diagnostics on supported CPU toolchains; ASan and thread sanitizer run as separate builds.
- **Nightly or before backend promotion:** complete registrations, broader fixtures, long-running cases, heavy memory tests where resources allow and available dedicated GPU hardware jobs.
- **Every release candidate:** native packages on all four targets, actual GPU tests on each advertised combination, cross-platform file exchange, clean install/launch, representative data, soak/failure recovery, source rebuild and checksum verification.

Archive executable/library hashes, compiler flags, test inputs, output hashes and per-test outcomes with each result. CI configuration is itself reviewed and must fail when required tests are missing. No cloud workflow, runner purchase, credentials configuration or release publication is performed by this planning task.

## 9. Release sign-off and sequencing

Roles: implementation owner develops each milestone; verification owner checks evidence/traceability; hardware/test owner supplies native/GPU runs; release owner verifies package/source/checksum consistency. These roles can be performed by the same person but remain separate checks in the report. Sign-off is evidence-based, not an assumption of a separate reviewer or test machine that has not been secured.

Sequence: M0 → M1 → M2 → M3. M4 and M5 depend on M3's stable backend contract; after that their implementation can proceed independently. M6 follows the selected backend milestones; M7 packages only configurations that passed M6. A portable CPU-only release can complete M6/M7 after M3 while GPU work continues as a later release. That CPU release must not advertise GPU acceleration. No calendar commitment is made until M0 resolves hardware access, dependency/toolchain versions, representative fixtures and release inputs.

Blocking failures: incorrect geometry/direction, unsafe size arithmetic, memory corruption, objective/cache inconsistency, changed unsupported numerical behavior, lost accepted state, incompatible file output, native launch failure, or missing required execution evidence. Performance goals never override correctness gates. Unsupported optional acceleration stays disabled; missing evidence for a required CPU target prevents claiming four-platform support.

The plan is complete when implementation scope, test IDs, gates and artifacts can be traced to the companion JSON/CSV. The implementation is complete only when the required executed evidence and distributable packages satisfy those gates.
