# Build and test from source

## Prerequisites

- Node24/npm (the committed package-lock.json is authoritative).
- Python3.12 with a private virtual environment.
- A 64-bit native compiler and enough free disk/memory for a static ITK build.
- macOS: Xcode Command Line Tools, `make`, native Apple Silicon or Intel Python.
- Linux: GCC/G++, Make and standard development headers (`build-essential`).
- Windows: MSYS2 MINGW64 GCC, Make/diffutils/patch. System CPython is separate from
  MSYS2's Python. Use the same native x64 architecture for Python, GCC and Electron.

The workflows select fixed OS runner labels (`macos-15`, `macos-15-intel`, `windows-2022`,
`ubuntu-22.04`), Node24, Python3.12 and Python/build dependency versions. macOS14 is
the deployment minimum; release runners use macOS15 rather than a deprecated runner.
Hosted image/compiler patch versions can evolve; build reports record the actual toolchain. Bit-identical installer reproduction is not claimed.
FFTW3.2.2 and the SHA-verified ITK5.4.6 source are already included. No Homebrew ITK,
external JIP application, GPU SDK, or standalone Python is required by end users.

## Frontend/core

```sh
npm ci
npm run build
npm run ci:test
```

`ci:test` separates browser jobs, OS-specific launcher checks and native-worker
checks. Required tests fail CI; absent native payloads do not count as passing them.
The native job runs the required worker checks after compilation. Original `npm run check`
remains available for the full existing development suite on a prepared workstation.

## Embedded scientific worker (macOS/Linux)

```sh
python3.12 -m venv .ci-venv
. .ci-venv/bin/activate
python -m pip install -r scientific/requirements.txt -r scripts/ci/requirements.txt
python scripts/ci/build-scientific.py --jobs 4
export BRAINANA_SIMPLEITK_PYTHON="$(command -v python)"
npm run ci:test -- --native-only
```

The independent SimpleITK reader is a **test dependency** only. It is excluded from
the frozen worker. Production ITK/N4 uses the compact native adapter built from source.
The driver builds vendored FFTW/zlib, JIP, static ITK, its C API and a private PyInstaller
worker. It runs native CTest checks and compares complete source/frozen JIP/ITK fields
and N4 outputs. No numerical implementation or default registration settings are replaced.

Use `--plan` to inspect the native target without building. `--skip-freeze` is for
native-library development only and must never be used for release packaging.

## Windows

The release workflow installs `msys2/setup-msys2` with MINGW64 GCC, Make, diffutils
and patch. It supplies the actual MSYS2 location to the driver through:

```
ALIGN_MSYS_ROOT=C:/path/to/msys64
ALIGN_MINGW_ROOT=C:/path/to/msys64/mingw64
```

Create/activate a normal Windows CPython virtual environment and install the same
requirements. Invoke `python scripts/ci/build-scientific.py --jobs 4` from the repo.
The driver runs FFTW's configure/Make under MSYS2 Bash, and native CMake/Ninja with
MinGW GCC for the libraries. PyInstaller uses the system Python, not MSYS2 Python.
It packages matching `.dll`/`.exe` files and checks the complete frozen worker.
On hosted Windows runners it uses a short RUNNER_TEMP build directory to avoid
long-path failures in upstream ITK. Compiler runtime DLL imports and license
notices are copied beside the private libraries before freezing.

The POSIX `fcntl(F_GETFD)` descriptor-injection C test is not compiled on Windows.
Shared API, contexts, sampling, resampler, source/frozen, domain-service and package
checks still run. No Windows test success is implied before that runner completes.

## Package this computer's app

```sh
npm run desktop:prepare
npm run ci:config
npx electron-builder --config generated/github-builder.yml --publish never
npm run ci:verify-package
```

For CI the native flags are explicit (`--mac --arm64`, `--mac --x64`, `--win --x64`,
`--linux --x64`). On Linux the workflow installs GTK/NSS/GBM/ALSA/FUSE dependencies
for Electron/AppImage packaging. Each package includes its own matching scientific
worker. The package verifier checks exact ASAR frontend bytes, model labels,
embedded architecture, worker capabilities and actual packaged JIP/ITK execution.

## Reference data and reproducibility

Private subject data and credentials must stay outside this repository. Native
validation outputs are generated under ignored directories. Immutable synthetic
reference traces are gzip-compressed for storage; decompression preserves the exact
legacy bytes and checksums. The original reviewed source archive remains intact.
Independent reference traces can be architecture-specific; do not regenerate a
reference from the candidate to make a failing numerical check pass.

Native GUI/GPU pixels, platform installation behavior and code-signing identities
require their own target-system review beyond headless worker/DOM checks.

### Hosted-runner compatibility notes

FFTW 3.2.2 is built with `MAKEINFO=true` for both compilation and installation.
Its legacy Texinfo regeneration rules fail with modern Linux Texinfo and absolute
out-of-tree paths; the bundled documentation remains available. This does not
change the FFTW library or registration code. Keep the override on Windows too.

Cross-platform path fixtures normalize expected paths using the host path module.
The compiled UI proposal test waits for the actual manual controls to unlock,
with a bounded timeout, rather than assuming an Intel runner finishes its overlay
within 180 milliseconds. Keep the transform and proposal assertions after this wait.
