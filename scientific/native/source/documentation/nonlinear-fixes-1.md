# JIP nonlinear registration — repair revision 1

> Historical first-revision notes. The current source is repair revision 2: see `64bit-repair-2.md` for current build instructions, widened interfaces, test counts and validation. The seven byte-identical-function observations below apply to revision 1, before the 64-bit type and accumulator changes.

This repairs the JIP source supplied in `jip-source 2.zip` (SHA-256 `8c5dc0613e2e12f7b764b228d860221549790d41af2c1207115853ae816b48be`). It is a separate source repair, not a Brainana Align integration or an upstream JIP release. Original copyright and license notices are retained.

## What stays the same

The original sampled control-grid basis, independent displacement components, image-driven MI / uncentered NC similarity choices, bending/stiffness penalty, coefficient step bounds, gain-weighted search direction, and coarse-to-fine strategy remain. The basis is recursively refined and piecewise linear; it is not a cubic B-spline or a cosine-squared basis. Seven core basis, penalty, step-bound and MI functions were compared byte for byte with the original; see `validation/preserved-method.json` in the evidence bundle.

## Repairs

1. Correct the two Z-displacement finite-difference stencil errors. Equivalent curved fields now have equal penalties regardless of component direction; linear fields have zero pure bending penalty.
2. Differentiate the actual affine-plus-displacement pull map for validity checks. Check all grid samples, including image-zero regions and outer planes, and reject nonfinite samples and singular affine baselines. Include a one-voxel neighborhood for local checks.
3. Clear each resampled output location before evaluating a local candidate, including locations that leave source support or map to an excluded zero source value.
4. Honor a valid NIfTI sform even without a qform; use column norms of the selected affine for voxel spacing. Reject singular/nonfinite selected spatial matrices. Retain full native grid metadata separately in saved parameters and default NIfTI outputs.
5. A flat or invalid objective no longer proposes movement. Choose the better of two improving probe directions. Bound parabolic estimates and verify the actual combined candidate with backtracking and rollback.
6. Use the same global NC objective for full-volume scores and local cached probes. Guard empty/nonfinite intensity samples and zero-mean normalization; accumulate deformation and correlation sums in double precision.
7. Reject nonfinite/overflowing coefficient edits before mutating any field samples. Apply one coefficient delta instead of separately subtracting and adding contributions.
8. Validate and stage all three displacement components before loading. Detect every read/write failure. NIfTI displacement output uses an exclusively created temporary file and atomic replacement; a failed component write leaves the previous destination intact. Validate grid geometry, not just dimensions.
9. Replace recursive stack-sized sorting buffers with stable iterative heap-based sorting.
10. Inverse image registration now solves the complete `A*x + u(x)` map using damped Newton iterations and trilinear field interpolation. Preflight sampled topology and finite data. Report unsolved / out-of-domain output samples and fill them with zero. Do not silently ignore a loaded field on the wrong grid.
11. The full-workflow action now requests the intended hierarchy and deselects affine controls after its affine stage. The usual first sequence executes grid settings 8, 4 and 2. Keep the final grid setting consistent with the actual basis; explicit grid setting 1 also executes an optimization level.
12. Correct the methodology documentation and describe the actual coordinate and Jacobian conventions. Modernize two unsafe command-parser operations discovered during native CLI testing, and return success for a normal CLI exit.

## Coordinates and validity

The registration remains in **centered target and source voxel axes, in millimeters**. Its pull map is `T(x) = A*x + u(x)`: a target location maps to the source sampling location. The inverse solves for the target location given a source location.

Native NIfTI qform/sform matrices are grid metadata. New `align.com` files record full row-major `target-voxel-affine` and `source-voxel-affine` matrices separately; the default NIfTI writer stores the corresponding full sform. They are not applied again during centered-axis registration. The displacement file's grid is the target grid, but its vector components retain JIP's centered source-axis meaning, not generic scanner-RAS vector meaning.

The reported validity measure is **relative** to the affine: `det(A + Du) / det(A)`. The default limit 2 accepts the interval [0.5, 2]. This retains the original notion of deformation relative to the affine while using the correct derivative. A baseline reflection is not automatically removed; a new local orientation reversal relative to it is rejected. A lower invalid-sample count can be accepted when repairing an already invalid field. These are sampled checks, not a mathematical guarantee of global invertibility between samples.

The similarity score is maximized after subtracting `(rigidity² / 10) * penalty`. MI bin normalization is held fixed within a local-probe pass, then refreshed at full initialization. NC remains the original uncentered normalized correlation, not Pearson correlation. Corrected penalties and acceptance checks can change fitted results compared with the buggy original.

`align.mat` is the inverse **centered-axis affine only**. It neither contains the nonlinear field nor, by itself, constitutes a scanner-RAS or ITK/ANTs transform. Older parameter files without native-affine entries retain their legacy geometry fallback. Previously saved oblique fields whose metadata was already lost cannot be reconstructed automatically; mismatching fields are rejected.

## Build and validation

The repair package includes scripts in `tests/`:

1. `python3 tests/run_regression.py` — 14 isolated production-function regression groups, compiled with AddressSanitizer and UndefinedBehaviorSanitizer. The 2,146,689-element sort also runs with an explicit 8 MiB stack limit.
2. `python3 tests/build_native.py` — build both registration executables on macOS Apple Silicon using Clang, FFTW and XQuartz. This script expects FFTW under `/opt/homebrew` and XQuartz under `/opt/X11`; adjust those paths for another installation.
3. `python3 tests/test_native_cli.py` — 10 actual CLI cases, covering forward/inverse fields, spatially varying displacement, original affine-only behavior, output geometry, fold/nonfinite rejection, wrong-grid rejection, and explicit quaternion-only output.
4. `python3 tests/test_engine.py` — run the actual GUI registration engine without opening X11, with both MI and NC, including fitted-field save/restore.

Perform a **clean rebuild** after applying the patch: the shared Alignment structure has new geometry fields, and the legacy makefile dependency lists do not cover every included header. Never reuse old object files or executables. The supplied native build script compiles every object afresh.

Run these from the repair package root containing `source/` and `tests/`. The isolated suite requires Python 3 and Clang with sanitizer support. Native test scripts expect the preceding native build to have completed.

The actual-engine synthetic tests improved MI from 1.35246086 to 2.51465821 and NC from 0.942355633 to 0.999723136. Maximum field change after save/restore was below 0.000001 mm. The inverse helper recovered 343 off-grid synthetic points within 0.0001 mm with an oblique affine and anisotropic spacing. These are regression results, not anatomical accuracy estimates.

## Practical limits

- The interactive X11 interface has not been visually exercised in this repair. The native GUI needs XQuartz; it is not a self-contained macOS app.
- Whole-brain / subject-level anatomical validation, Windows/Ubuntu builds, and clinical validation have not been performed. The prior bundled executables are not rebuilt and are excluded from the new source ZIP to avoid accidentally running stale code.
- Dense fields and very fine grids can still consume substantial RAM. Removing stack overflow does not remove their heap-memory requirements. The finite-update preflight and full inverse solver add work; no throughput benchmark is claimed.
- A sampled positive Jacobian does not establish a unique inverse everywhere. Failed inverse locations are reported rather than silently treated as affine-only.
- Individual NIfTI field saves are atomic. Legacy raw `.bfloat` header/data pairs and the multi-file `align.com` / `align.mat` / displacement session are not a single atomic transaction. Preserve backups when replacing a valuable registration.
- Legacy raw headers and explicit quaternion-only output cannot preserve arbitrary shear. Use default sform-based NIfTI output for full grid geometry.
- This source revision has not been integrated into Brainana Align. Any integration requires a deliberate adapter between Align's physical-coordinate transforms and JIP's centered-axis model.
