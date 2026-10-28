# JIP provenance, repairs and integration changes

Brainana Align incorporates a **modified JIP implementation**, derived from the
JIP toolkit developed by Joseph B. Mandeville at Massachusetts General Hospital /
Athinoula A. Martinos Center. This is a Brainana-maintained repair and integration,
not an upstream JIP release. Original copyright and software-license notices are
retained. See [software credits](software-credits.md), the
[original project](https://www.nmr.mgh.harvard.edu/~jbm/jip/) and the
[MGH license](../scientific/native/source/engine/JIP-LICENSE.txt).

The record separates intentional numerical corrections, later preservation of the
repaired method, application integration, and platform/build work. Saying that a
later optimization preserves a reference does **not** mean every repaired result
is identical to the original, unmodified JIP distribution.

Source-relative links below work in the repository or extracted source tree. The
full release bundle supplies that tree in its matching source ZIP.

## Provenance and reference baseline

| Stage | Record |
|---|---|
| Originally supplied JIP source | The historical [first repair notes](../scientific/native/source/documentation/nonlinear-fixes-1.md) identify `jip-source 2.zip` by SHA-256 `8c5dc0613e2e12f7b764b228d860221549790d41af2c1207115853ae816b48be`. That original archive is not presented as the current build input. |
| Nonlinear corrections, then 64-bit repair | [Nonlinear repair revision 1](../scientific/native/source/documentation/nonlinear-fixes-1.md) and [64-bit repair revision 2](../scientific/native/source/documentation/64bit-repair-2.md) document the successive changes. |
| Frozen reviewed baseline | [reviewed-source.zip](../scientific/native/reference/reviewed-source.zip), SHA-256 `49bb3dcf91ac911c0db985d6b9a89dfa8cb4e21c740ab200befae45f1c5ae1ae`, with a [file manifest](../scientific/native/reference/source-manifest.json). This baseline already contains the earlier repairs. |
| Current incorporated source | [scientific/native/source](../scientific/native/source/), including the guarded native engine and shared numerical bodies. [Maintenance contracts](../scientific/native/source/engine/MAINTENANCE.md) record the invariants for subsequent changes. |

Historical repair notes describe the standalone source package at the time they
were written. Statements there such as “not integrated into Align” or “Windows
not executed” are historical qualifications. Current platform qualification is
reported separately by the release's actual CI and validation artifacts. Internal
engine and worker version numbers are not upstream JIP version identifiers.

## Earlier numerical and geometry repairs

These corrections were intentional changes to defective calculations or unsafe
behavior, rather than claims of byte equivalence to all original results.

| Problem addressed | Change and intended behavior |
|---|---|
| Z-component bending stencil errors | Corrected two finite-difference stencils so equivalent curved fields receive consistent penalties across displacement components and linear fields have zero pure bending penalty. |
| Validity checks omitted parts of the transform/grid | Differentiate the full affine-plus-displacement map, reject nonfinite samples and singular affine baselines, and inspect boundary and image-zero locations as well as interior samples. Local checks include a neighboring voxel layer. |
| Stale resampled values during a local trial | Clear each touched output location before sampling, including points leaving source support or mapping to excluded zero source values. Preserve untouched locations. |
| Incomplete NIfTI geometry handling | Accept a valid sform without requiring qform, derive spacing from the selected affine's column norms, reject singular/nonfinite matrices and preserve native grid metadata separately from centered registration coordinates. |
| Unsafe or non-improving optimizer steps | Handle flat/invalid objectives, select an improving probe direction, bound parabolic estimates, and check combined candidates with backtracking and rollback. |
| Inconsistent local/global normalized-correlation scoring | Apply the same global NC objective to full scores and cached local probes; guard empty/nonfinite samples and zero-mean normalization. Use double accumulation for correlation/deformation sums. |
| Nonfinite or overflowing field edits | Validate coefficient changes before mutation and apply a single coefficient delta rather than subtracting and re-adding contributions. |
| Partial or mismatched field restore/write | Stage and validate all three components, check geometry and I/O failures, and commit individual NIfTI outputs through exclusive temporary files and replacement. |
| Stack-sized sorting workspaces | Replace recursive stack-sized buffers with stable iterative, checked heap storage. |
| Inverse deformation omitted nonlinear behavior | Solve the complete map with damped Newton iterations and trilinear field interpolation; reject inappropriate fields and report/fill unresolved or out-of-domain samples. |
| Hierarchy/stage state did not match the requested workflow | Run the intended hierarchy, retain the actual active basis/grid setting and keep stage controls consistent with the stage being executed. |
| 32-bit count, allocation and file-offset assumptions | Introduce wide signed indices, checked products and allocation sizes, wide loop/count interfaces and 64-bit file positioning. Preserve bounded external APIs and fixed-width on-disk formats such as 32-bit `.blong`. |

The repair also changed similarity-histogram counters and image-mean accumulation
to double precision to avoid saturation on large data. That can alter an optimizer's
path; it must not be described as universally byte-identical to the earlier code.
The historical notes contain the original test details and limits.

## Method and coordinate meaning

JIP's retained deformation uses its sampled, recursively refined **piecewise-linear
control-grid basis**, independent displacement components, its MI/uncentered-NC
choices, deformation penalties and coarse-to-fine search. It is not the cubic
B-spline model provided by Align's separate ITK nonlinear route.

Within JIP, the pull map is `T(x) = A*x + u(x)`: a fixed/target location maps to a
moving/source sampling location in centered image-axis millimeters. Native NIfTI
voxel-to-world matrices describe the image grids separately. They must not be
applied twice or confused with that centered transform.

The relative validity measure is `det(A + Du) / det(A)`. It measures deformation
relative to the affine baseline; it is not the determinant of the residual field
alone. Sampled Jacobian checks do not prove a globally unique inverse.

Align's [Python bridge](../scientific/brainana_registration/align_bridge.py) and
[geometry conversion](../scientific/brainana_registration/images.py) explicitly
convert the native map to the world/RAS dense-map convention used by Align's
sessions and exports. Fields remain on the fixed working grid. The independent
[SimpleITK reader test](../tests/displacement-simpleitk-regression.mts) checks the
exported geometry/vector convention and both transform directions.

## Native engine and Align integration

- **Guarded, independent contexts:** the C API owns its images and numerical state.
  Trials clone the accepted state and commit only after a successful operation.
  Recoverable errors, allocation faults and cancellation roll back a trial instead
  of terminating the host application or corrupting the accepted result.
- **Checkpoints and outputs:** checkpoints use explicit fixed-width little-endian
  fields and integrity checks, not raw pointers/native struct dumps. Configuration,
  stage history, masks and supported windows are serialized. Legacy checkpoint
  compatibility remains tested. Per-file atomic output does not make an entire
  multi-file registration export one indivisible transaction.
- **Measured optimizations:** resampling/index/bounds work, bounded caches and
  optional acceleration preserve the declared coefficient/tap order and accepted
  numerical behavior. Device failures recalculate the touched region on the CPU.
  Performance work is constrained by immutable references and rollback/resource
  tests; it is not a new deformation model or a general speed guarantee.
- **Regions:** fixed/moving selections restrict metric membership consistently;
  moving windows test continuous pulled physical points. They are not implemented
  by rewriting image intensities or silently dropping constraints.
- **Nonlinear only inside Align:** Align owns landmark and image-based linear
  registration. JIP receives the accepted linear matrix and performs nonlinear
  work from that exact base. The fuller legacy affine machinery retained in the
  shared source does not imply a second JIP linear workflow in Align.
- **Private bundled worker:** the application carries its own numerical worker;
  it does not discover or require a separate JIP GUI application, X11 frontend,
  Qt installation, external Python installation or user-selected registration app.

Implementation and current tests:
[engine API](../scientific/native/source/engine/jip_engine.h),
[transaction/recovery runtime](../scientific/native/source/engine/engine_runtime.c),
[checkpoint/I/O](../scientific/native/source/engine/engine_io.inc),
[context tests](../scientific/native/tests/context_driver.c),
[stage/interface tests](../scientific/native/tests/interface_driver.c),
[reliability/fault tests](../scientific/native/tests/reliability_driver.c),
[sampling tests](../scientific/native/tests/sampling_driver.c) and
[direct resampler comparison](../scientific/native/tests/test_resampler.py).

## Platform and packaging changes

These are separate from the earlier JIP numerical repairs:

- Build the vendored FFTW library without regenerating its obsolete Texinfo docs.
- Preserve golden fixture bytes across Windows checkout; line-ending conversion
  must not change a reference checksum.
- Use exclusive Windows temporary-file creation and atomic replacement through
  an isolated Windows SDK adapter. The selected MSVCRT/MinGW runtime does not support the
  exclusive-open/replacement assumptions used by the former `fopen("wbx")`/replacement path.
- Remove unused symbols/debug information where needed to meet the worker budget,
  then revalidate the actual frozen worker. Symbols are not a replacement for
  numerical validation.
- Build pinned Intel NumPy against system Accelerate rather than bundling its large
  OpenBLAS wheel. NumPy remains at the same pinned version.
- Select joined Windows **ITK** platform threads to avoid persistent-pool DLL
  shutdown hangs. This fixes the separate ITK worker route, not JIP's optimizer.
- Preserve required Windows OS environment variables, bound test processes and
  terminate timed-out process trees while retaining synthetic failure evidence.

See [build and platform policy](building.md) for the exact procedures. Root
[AGENTS.md](../AGENTS.md) and the native maintenance contracts are intended to stop
future refactors from reintroducing these failures.

## What validation establishes

The frozen reviewed baseline is immutable. On its original macOS-arm64 reference
target, full nonlinear search traces and legacy references remain byte-exact.
All platforms retain exact same-host restore/rollback comparisons and exact
source-versus-frozen worker field comparisons. Cross-platform comparisons against
stored legacy float references use the explicitly documented bounded tolerances;
see [native reference policy](building.md#native-reference-policy-across-platforms).
This does not regenerate golden values from a failing candidate.

Current evidence includes [full-search replay](../scientific/native/tests/verify_workflow.py),
[checkpoint compatibility](../scientific/native/tests/test_checkpoint_compat.py),
[known-truth geometry tests](../scientific/native/tests/accuracy_driver.c),
[actual source/frozen JIP/ITK/N4 comparisons](../scientific/tests/validate_runtime.py),
[release gates](../.github/workflows/release.yml) and the release's validation JSON,
manifests and SHA-256 checksums. Match those reports to the released source commit
and platform. Historical test counts, memory experiments or standalone X11 tests
must not be promoted into a claim that every current platform/GPU was exercised.

These are software-correctness and engineering checks. They do not establish
anatomical alignment accuracy for every dataset or replace inspection of the
resulting registration.
