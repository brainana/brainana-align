# JIP repair revision 2 — 64-bit memory and indexing

This revision builds on the nonlinear-registration repairs. It converts the **six active JIP applications and their shared code** to a consistent 64-bit memory model. The control-grid registration basis, deformation equations, coordinate conventions, metrics, and hierarchy remain the same. Brainana Align has not been modified or integrated with this source.

## What changed

- `JIP_INDEX` is a signed 64-bit pointer-distance type. Image totals, flat voxel indices, nonlinear basis indices and neighbor tables, overlay counts, allocation lengths, and byte offsets use it. Signed indices retain existing negative sentinels.
- Allocation byte counts use `size_t`. The utility vector/matrix allocators no longer cast requests to 32-bit `unsigned`. Every allocator checks negative lengths and multiplication overflow before allocating. Zero-length requests have a defined non-null allocation, and actual allocation failures report the requested byte count.
- Image totals are constructed with checked products. Index arithmetic widens **before** multiplication; widening the assignment alone would leave an overflowing 32-bit intermediate expression.
- Loop counters follow the range of the data they traverse. Sorting uses 64-bit index arrays; nonlinear basis tables have matching 64-bit entries. Freeing a zero-row matrix still uses a signed counter, avoiding unsigned underflow.
- File seeking uses a 64-bit interface: POSIX `fseeko`/`ftello` with large-file definitions, or Windows `_fseeki64`/`_ftelli64`. File offsets no longer depend on `long`, which remains 32-bit in Windows' 64-bit ABI. Loaded dimensions and offsets are validated before buffer sizes are constructed.
- `.blong` data remains **32-bit on disk**. GLM, SRTM and REM readers now read `int32_t`, rather than `sizeof(long)`, which previously changed the read width on 64-bit Unix systems.
- Text formats match the widened fields, including portable `%td` count/offset output. Narrow display coordinates and bounded external API arguments are explicitly range-checked. Pixel datatypes, file-format fields, flags, and per-axis coordinates are not indiscriminately changed to 64-bit.
- Similarity histograms use double-precision counters, and image means accumulate in double precision. This prevents float increment saturation on large images. Small numerical differences can change an optimizer's path; these changes preserve the equations rather than guaranteeing identical fitted coefficients.
- New CMake builds reject a 32-bit toolchain. The shared headers also require 64-bit pointers, object sizes, index types and file offsets. Legacy makefiles no longer select i386/PPC builds.
- Launchers use rebuilt tools, preserve quoted arguments and exit status, support `.exe` names, and never silently fall back to archived platform executables. `JIP_HOME` defaults to the source directory for documentation; `JIP_BUILD_DIR` selects a build directory explicitly.

All modules must be **cleanly rebuilt** because shared structure layouts and function signatures changed. Do not reuse old object files, libraries, executables or a previous application installation's launcher.

## What “64-bit” does and does not mean

The former fixed process/address-size limits and truncated allocation requests are removed from the supported code paths. This does not create additional physical RAM. Large images, multiple working copies and dense deformation fields can still exhaust available memory. Oversized or invalid requests fail explicitly instead of wrapping to a smaller buffer.

File formats retain their own limits. For example, NIfTI-1 dimension fields remain signed 16-bit fields, and image samples can still be float32 or int32. A 64-bit process does not require 64-bit voxels. FFT and display APIs with bounded integer dimensions retain those bounds; values crossing them are checked. None of these per-axis/file-format constraints imposes a 2 GiB total-memory ceiling.

The archived `jip-align/jip-align-old` implementation is retained for historical comparison and is excluded from every supported build. Bundled third-party sources are retained with their original APIs and licenses and are compiled for the selected 64-bit target. Active JIP NIfTI dependency calls are the quaternion/matrix conversion routines; image loading uses the repaired JIP I/O path.

## Build

The preferred build is CMake, with a C11-capable 64-bit Clang/GCC toolchain, FFTW3 and zlib. The two graphical tools additionally require X11/XQuartz.

From the repair package root containing `source/`:

```sh
cmake -S source -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel 3
```

On Apple Silicon, installed dependencies may require `-DCMAKE_PREFIX_PATH="/opt/homebrew;/opt/X11"`. Explicit FFTW and X11 path overrides are also supported by standard CMake discovery variables. Intel macOS and 64-bit Linux use their corresponding 64-bit dependencies. For a build without graphical tools, use `-DJIP_BUILD_X11=OFF`.

For Windows, the full X11 suite requires a suitable 64-bit environment such as Cygwin64. The source includes the Windows 64-bit seek and portable formatting paths. **Windows and Linux builds were not executed in this repair**; only Apple Silicon macOS builds are validated here. This is not an MSVC conversion of the legacy application.

Outputs are `jip`, `jip-align`, `jip-display`, `jip-glm`, `jip-rem`, and `jip-srtm` (platform suffixes as appropriate). Set `JIP_BUILD_DIR` to the output directory when using `source/bin/` launchers.

## Validation and reproduction

The packaged report and evidence distinguish actual memory use from virtual reservation:

- **2.125 GiB allocated and touched:** measured peak resident memory exceeded 2 GiB.
- **8 GiB + 72 bytes in one allocation:** four locations, including locations beyond 2 GiB, 4 GiB and the last element, were written and read successfully. This is a virtual allocation test, not an 8 GiB resident-memory claim.
- **24 real allocation interfaces:** instrumented allocation requests retain lengths above `INT_MAX`, including vector and matrix row requests. Large requests are recorded rather than physically allocated by this fixture.
- **Five actual indexing functions on two grids:** flat indices beyond 32 bits and a per-slice count above 4 billion are computed correctly under sanitizers.
- **Large header/count arithmetic:** 17,179,869,184 samples and a 64 GiB float payload are represented correctly without allocating or loading that payload.
- **Large file offsets:** sparse-file seek/readback beyond 5 GiB, native CLI loading with a 3 GiB payload offset, and all three analysis readers at 3 GiB offsets.
- Negative lengths, arithmetic overflow, and values exceeding bounded external APIs are rejected before unsafe allocation or truncation.
- The original nonlinear regressions, both actual registration-engine metrics, fitted-field session restore, native CLI image cases, and launchers pass.
- All 42 active first-party translation units pass type/format/interface checks. A Clang AST audit finds no remaining narrow image-size multiplications or counters compared with wide counts. CMake treats narrowing, format, incompatible pointer, implicit declaration and implicit-int failures as build errors in first-party targets.

The detailed executable test scripts currently target this Apple Silicon build environment. Run:

```sh
python3 tests/build_native.py
python3 tests/audit_compile.py
python3 tests/audit_integer_paths.py
python3 tests/run_regression.py
python3 tests/test_native_cli.py
python3 tests/test_engine.py
python3 tests/test_64bit.py
python3 tests/test_wide_interfaces.py
python3 tests/test_analysis_readers.py
python3 tests/test_launchers.py
```

The resident-memory test only runs when its host-memory check finds a substantial margin; it reports a skip otherwise. Large-object and sparse-file tests deliberately avoid consuming their full virtual/disk size. The `--apply` / `--fix-formats` options on audit helpers are maintenance aids; normal validation runs those audits without mutation.

## Remaining validation boundaries

Interactive X11 behavior, full GLM/SRTM/REM statistical workflows, whole-brain accuracy and throughput benchmarks have not been exhaustively validated. Existing unrelated statistical/UI logic warnings are recorded in the compiler evidence. These repairs address size/address correctness and retain the research-tool status of the supplied source; they do not establish clinical accuracy or guarantee success for every dataset.

The nonlinear map remains JIP's centered-axis millimeter map. The earlier inverse, Jacobian and geometry repairs remain in place. See `nonlinear-fixes-1.md` for those conventions; its first-revision test counts and byte-identical-function observations are historical, not claims about the widened function text in this revision.
