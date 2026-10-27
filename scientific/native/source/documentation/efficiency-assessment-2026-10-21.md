# JIP efficiency and acceleration assessment

Assessment date: 2026-10-21. Scope: the repaired 64-bit JIP nonlinear engine, preserving its mathematical approach. Brainana Align has not been changed. The accompanying source contains one additional counter correction; the CPU pruning experiment and Metal experiment are separate benchmark code, not enabled application backends.

## Recommendation

Keep JIP's existing basis functions, coarse-to-fine schedule, sequential coefficient search, image metric, deformation penalty and topology checks. Modernize the computation beneath those components. Begin with CPU build optimization and resampling improvements, then introduce a small backend interface with an optional Metal implementation for Apple Silicon. Retain a complete CPU implementation and automatic fallback.

Replacing JIP's registration with another package would change the method. Borrowing architecture and carefully adapting individual numerical kernels is a better fit for the stated requirement.

## Measured results

| Grid / metric | O2 reference | O3 baseline | O3 + LTO | O3 + LTO + pruning | Speedup vs O3 |
|---|---:|---:|---:|---:|---:|
| 64³ / MI | 0.479 s | 0.448 s | 0.384 s | 0.326 s | 1.37× |
| 64³ / NC | 0.772 s | 0.704 s | 0.619 s | 0.527 s | 1.34× |
| 128³ / MI | 2.033 s | 1.828 s | 1.588 s | 1.348 s | 1.36× |
| 128³ / NC | 2.797 s | 2.484 s | 2.266 s | 1.911 s | 1.30× |


These are medians of five runs of the actual linked engine on finite synthetic images with anisotropic spacing. Each run performs **32 sequentially selected basis updates**, with MI or NC and the existing penalty/topology rules. The 64³ case has 405 basis functions; the 128³ case has 729. This is a bounded workload, **not a complete registration or hierarchy**, and excludes initialization, image I/O, final export and GUI painting. All five CPU variants produced **byte-identical final displacement fields** on each tested configuration. This establishes equivalence on these fixtures, not all inputs.

The reference is the direct `-O2` build from the preceding repair. The independent CMake Release build already uses `-O3` on this compiler; comparisons against that build should use the O3 column, not treat the entire O2 improvement as a new application gain. The optimized-build experiment uses `-O3 -flto`, without fast-math or a host-specific instruction-set requirement. The pruning experiment skips interpolation branches whose coefficients are exactly zero; it never changes the interpolation kernel or uses a near-zero cutoff.

The pruning experiment is restricted to **finite source data**. Skipping zero-weight reads can change propagation of NaN/Infinity (`0 * NaN` is NaN). Before adoption, it needs a data-validity guard and original-path fallback, plus interpolation/boundary regression coverage. It is not a released optimization.

Profiling attribution, using three runs with nested inclusive/self timers:

| Grid / metric | Resampling | Deformation penalty | Jacobian checks |
|---|---:|---:|---:|
| 64³ / MI | 64.9% | 21.9% | 7.9% |
| 64³ / NC | 69.1% | 17.8% | 9.3% |
| 128³ / MI | 58.9% | 26.7% | 8.5% |
| 128³ / NC | 64.5% | 22.6% | 9.1% |


The mutual-information entropy/log loop itself accounts for less than 0.2% here. Replacing logarithms with approximations or moving only the 50×50 histogram entropy calculation to a GPU would target the wrong bottleneck. The cost of resampling and finite-difference deformation evaluation is much larger. The bounded test does not time basis-order sorting or image-pyramid creation, so no performance conclusion about those stages is implied.

The raw records include both elapsed and process CPU time in the final comparison. Early exploratory runs had very large elapsed-time outliers consistent with host suspension or scheduling; those records are retained. Final medians are reported separately. Absolute timings depend on host load, compiler, hardware, resolution, interpolation widths and candidate acceptance. No speedup on a user's real dataset has been established.

## Specific CPU improvements, in order

1. **Production build settings.** Offer optimized Release builds with optional link-time optimization. Preserve finite/nonfinite checks and normal floating-point semantics; do not enable `-ffast-math`. Measure against the actual distributed binary. The current CMake Release build is already O3, while the earlier direct build is O2.
2. **Resampling inner loops.** In `transform_coordinates`, hoist row/plane strides and immutable options, separate interior voxels from boundary handling, and skip provably zero interpolation contributions on finite data. Keep the same Lanczos/Gaussian formulas, normalization, wrap rules, nearest-source-zero gate and centered physical-coordinate map. Hardware trilinear interpolation is not an equivalent replacement.
3. **Avoid unnecessary intermediate warps.** `Set_AlignNL_Value` resamples after every single-axis coefficient update and restoration. A combined trial often sets X, Y and Z in succession before scoring. Add an internal transaction that validates and updates the same independent component fields in the same order, then resamples once before any metric or external consumer reads the mimic. Preserve the two-pass validation that prevents partial mutation on invalid input. Single-axis probes still need their corresponding warped image. This change needs full objective/cache tests before adoption.
4. **Reuse small working buffers.** Each resampling call allocates/frees three coefficient arrays. In the measured 64³ workload that means 492 MI or 954 NC resamplings for only 32 selected coefficients. Keep per-context/per-worker scratch space sized for the largest required interpolation width. This is a concrete avoidable overhead, though its individual speed benefit has not been measured.
5. **Simplify regularization/Jacobian stencils.** Use fixed row/plane offsets and an interior kernel with a boundary kernel. Preserve sample ordering for an exact CPU mode. Any incremental penalty cache must account for every affected finite-difference stencil, including a one-voxel halo, and be checked against a full recomputation after updates, rejection and restoration.
6. **Cache invariant target work.** Target inclusion flags, geometry and target-bin mappings can be reused during a scoring epoch. Bin mappings must be invalidated when the globally normalized means/range are rebuilt; the moving image and its normalization are not invariant. Data edits, masks, regions, pyramid level and geometry also invalidate appropriate caches.
7. **Parallelize independent voxels.** Once inputs/state are explicit, parallelize resampling output tiles and per-voxel stencil evaluation within each sequential coefficient trial. Use a thread pool or OpenMP with a workload threshold. Use private histograms and fixed-order reductions; shared histogram increments and the current static metric caches cannot simply be executed concurrently.

The outer coefficient loop should remain sequential. Overlapping basis supports, neighbor constraints, incremental fields, cumulative metric state and data-dependent step acceptance make a simple parallel-for over coefficients a method change and a race hazard. Changing search order, evaluating overlapping coefficients as if independent, replacing the optimizer or switching to a different metric would require separate scientific validation.

## Code organization that enables safe acceleration

The engine is currently coupled to global image stacks, `AlignNL`, `AlignAid`, X11 globals and function-static metric/penalty caches. `transform_coordinates` temporarily substitutes coordinate matrices in image stacks. These dependencies complicate concurrency and invalidate assumptions made by generic thread/GPU libraries.

Introduce a `RegistrationContext` owning immutable input views, physical transforms, basis metadata, accepted fields, proposal scratch, metric caches and their data-version keys. Extract computation behind a small C interface: resample a region, apply a coefficient delta, evaluate local penalty/Jacobian, build/reduce metric statistics. Keep the existing optimizer as the caller and retain a serial reference backend. X11 should consume snapshots and progress callbacks without owning the numerical state.

Preallocate scratch once, make cancellation/proposal commit explicit, and test cache invalidation centrally. Backend selection should be `Auto / CPU / GPU`, with the selected device and fallback reason recorded. A driver/allocation failure must leave accepted coefficients/fields intact; CPU fallback must rerun the affected computation from valid state rather than continuing from partial GPU output.

## GPU feasibility

**Apple Silicon: Metal is the first backend to investigate.** Apple's [Metal compute API](https://developer.apple.com/metal/) and [Metal-cpp interface](https://developer.apple.com/metal/cpp/) allow a small native backend around the existing C engine. My recommendation is an architecture inference from the measured workload, not an existing drop-in JIP accelerator.

Prioritize full-volume/final resampling and sufficiently large local resampling regions, then stencil calculations. Retain the source, target and accepted working fields on the device between related operations. Reuse pipelines and buffers; combine related dispatches into command buffers. Keep small patches on the CPU when launch/synchronization costs exceed computation. Thousands of individually submitted, synchronously waited tiny kernels are unlikely to make the sequential search efficient.

The objective uses double-precision statistics and several double intermediates. A float GPU port is not automatically numerically equivalent, even if the stored images are float32. Define tolerances, retain stable higher-precision reductions where supported or on the CPU, and recheck ambiguous acceptance/topology decisions with the reference path. A reference mode must remain available. Moving only statistics back to the CPU can introduce frequent synchronization; measure the whole candidate evaluation, not just GPU kernel time.

The GPU must preserve `A*x + u(x)` pull-map direction, anisotropic spacing, boundary/wrap behavior and the exact interpolation family. Texture sampling with a different interpolation rule changes the registration objective. Preserve integer-address correctness with 64-bit host sizes and checked/tiled device offsets. Per-buffer limits and 3D texture limits are distinct from available system RAM. [Apple's feature tables](https://developer.apple.com/metal/Metal-Feature-Set-Tables.pdf) show device-family differences; capability checks must be runtime checks rather than an assumption that all Macs expose the same operations.

**Windows/Linux:** keep the same CPU backend and consider an optional CUDA backend for NVIDIA hardware. CUDA is NVIDIA's GPU programming platform, not an Apple Silicon backend. See the [current CUDA programming guide](https://docs.nvidia.com/cuda/cuda-programming-guide/). AMD/Intel GPU support would need another backend and its own validation. A broad portability layer may become worthwhile after one backend is correct and demonstrably useful; it adds toolchain and testing costs now.

### What was actually tested for Metal

An isolated Metal penalty experiment is included. It extracts the actual CPU `bending_penalty` function, supplies the same CPU helper macro, and prepares a float Metal stencil port with fast-math disabled. Its intended measurements include command encoding, submission and waiting, warmups, repeated trials and a CPU comparison; the buffers are prefilled and reused. It is a feasibility fixture, not an application backend or a double-precision-equivalent implementation.

The Objective-C host program compiled. `MTLCreateSystemDefaultDevice()` returned **nil** in this execution environment. Therefore the shader was **not compiled by Metal or executed**, and GPU numerical accuracy and speed are **unverified**. The fixture reports this explicitly and exits safely. I have not added a working GPU acceleration option to JIP or claimed any GPU speedup.

## Modern codebases and components

| Candidate | Useful role | Compatibility with preserving JIP |
|---|---|---|
| [NiftyReg](https://github.com/KCL-BMEIS/niftyreg) | Reference for separating image operations from CPU/CUDA/OpenCL backends, optional GPU packaging and CPU fallback | Its documented nonlinear method uses cubic B-splines, NMI/bending energy and a conjugate-gradient optimizer. Replacing JIP with it changes the method. Its [current build configuration](https://raw.githubusercontent.com/KCL-BMEIS/niftyreg/master/CMakeLists.txt) offers OpenMP and optional CUDA/OpenCL/plugin paths; no guarantee that every backend covers every nonlinear feature is inferred. |
| [ITK/elastix](https://github.com/SuperElastix/elastix) | Reference for modular metrics, transforms, resampling, pyramids and threading; potential infrastructure for a custom JIP adapter | ITK's standard [BSplineTransform](https://docs.itk.org/projects/doxygen/en/stable/classitk_1_1BSplineTransform.html) is a particular transform definition. A default elastix registration is not the JIP method. The [GPUImage API](https://docs.itk.org/projects/doxygen/en/stable/classitk_1_1GPUImage.html) manages CPU/GPU buffers but does not automatically accelerate arbitrary JIP functions. |
| OpenMP / [oneTBB](https://oneapi-spec.uxlfoundation.org/specifications/oneapi/latest/elements/onetbb/source/algorithms/functions/parallel_for_func) | Mature CPU task/loop execution beneath independent voxel kernels | Preserve the optimizer; define private scratch, deterministic reductions and nonoverlapping writes first. OpenMP integrates with existing C more directly; oneTBB introduces a C++ boundary. No measured multithread speedup is claimed here. |
| [FFTW](https://www.fftw.org/) | FFT/pyramid preprocessing | The native build already links FFTW **3.3.11**, not the older bundled source. Replacing the library version alone is not the resampling optimization. Cache plans/wisdom and investigate batched transforms for repeated sizes. |
| Metal / Metal-cpp | Apple GPU kernel backend | Adapt the original kernels and numerical contract, keep CPU fallback. It is an API, not a registration-method replacement. |

The existing `ft_1d` creates one FFTW plan per call, reuses it for all lines, then destroys it. It does **not** create a plan per line. Reusing compatible plans across calls or [saved wisdom](https://www.fftw.org/fftw3_doc/Words-of-Wisdom_002dSaving-Plans.html) could reduce repeated setup. [FFTW's threading documentation](https://www.fftw.org/fftw3_doc/Multi_002dthreaded-FFTW.html) cautions that synchronization can outweigh gains for small transforms. Retain centered-frequency swapping and normalization when considering strided/batched FFTs. FFT work was outside the measured coefficient loop.

No external numerical code was copied into JIP in this assessment. Dependency adoption should preserve original licenses and record versions.

## Memory efficiency

The fixture owns eight float32 volume buffers: approximately `32 * voxel_count` bytes before basis tables, display, I/O and other scratch. At 512³ this component alone is **4 GiB**. One additional float volume costs 512 MiB; three displacement components cost 1.5 GiB. Duplicating every CPU volume on a GPU can increase memory substantially. Unified memory does not make duplicate allocations free.

Retain the current separable one-dimensional basis representation; do not cache a full 3D basis image for every coefficient. Use reusable region-sized scratch and blockwise reductions instead of a full-volume temporary for every score. Release obsolete pyramid/scratch buffers at explicit lifetime boundaries. Avoid complete field copies for every trial; a proposal region with the required stencil halo can support rollback, subject to full acceptance/rejection checks. Stream final output in tiles when the same interpolation/source-access rules can be preserved.

GPU device budgets, maximum allocation lengths, driver errors and per-tile offsets must be checked without reintroducing the old 2/4 GiB truncations. An automatic selector should base its decision on a warmed-up, end-to-end kernel/candidate timing and memory budget, not just GPU presence.

## Additional correctness finding

`calculate_volume_averages_nonlinear` retained an `int n` sample counter. This was missed by the preceding wide-index audit because the counter is incremented rather than compared with a wide bound. Above `INT_MAX` it can overflow during normalization. It is now `JIP_INDEX` in the accompanying source; the registration equations are unchanged.

A targeted sanitizer fixture seeds the actual counter at `INT_MAX` and processes one sample: the old declaration produces signed-overflow diagnostics and the correction passes. This deliberately avoids claiming to process billions of samples. A separate allocated-array fixture validates ordinary means/range. All 16 existing sanitizer regression groups pass, and the actual engine's two 64³ fields remain byte-identical after the correction. The incremental patch targets the prior 64-bit repair. This is the only application-source change in this assessment.

## Suggested implementation sequence and acceptance checks

1. **CPU revision:** guarded finite-data interpolation pruning, reusable scratch, optional LTO, and elimination of unnecessary intermediate warps. Benchmark complete registrations on representative datasets against the actual release binary. Require identical CPU outputs where operation order is preserved; investigate any differences.
2. **Context/parallel revision:** separate engine state and add voxel/tile parallelism with fixed-order statistics. Preserve coefficient order and accept/reject logic; verify full objective after local updates and restores. Run thread/race checks and serial-versus-parallel comparisons.
3. **Metal revision:** implement the matching resampling kernel, first for large full-volume operations; then benchmark local regions and add automatic CPU thresholds. Validate driver loss/allocation failure and fallback. Only extend to penalty/metric work after precision tests pass.
4. **Other GPU platforms:** add CUDA or another backend against the same kernel contract once there is a demonstrated need and hardware to test.

The validation set should include the prior nonlinear/geometry/inverse tests, full-level convergence and objective tracking, local cache versus full recomputation with halo cases, MI/NC, stiffness endpoints, anisotropic and oblique geometry, wrap modes, zero/nonfinite input behavior, coarse Gaussian versus fine Lanczos interpolation, near-fold/near-acceptance cases, repeated runs, sessions/export, cancellation, memory ceilings and device fallback. Speed is accepted only alongside acceptable field, Jacobian, objective and output-image agreement. No whole-anatomy or clinical accuracy claim is made by these synthetic experiments.
