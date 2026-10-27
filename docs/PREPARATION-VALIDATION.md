# Local source-publication preparation validation

Prepared 2026-10-27 from the working Align 0.17.10 source in a separate directory.
The original working application and its published artifacts were not modified.

Executed locally on Apple Silicon/macOS:

- Fresh `npm ci` and TypeScript/Vite build from the clean export.
- 79 existing source/core/DOM/emitted-worker suites passed. Two native-dependent
  suites run explicitly after the engine build; browser tests have their own CI job.
- Both native-dependent suites passed: real JIP domain execution/cancel cleanup,
  and independent SimpleITK displacement/physical-space/export interpretation.
- New publication tests passed: all four platform manifests required, source commit
  and installer hashes checked, missing/incorrect platform/commit/hash rejected,
  exact-commit source archive, ignored private fixture exclusion, manual no-publication
  policy, tag-version checks and final publication dependency.
- Pinned vendored FFTW/zlib, original JIP and static ITK compiled from clean source.
  Eleven native CTest groups passed. An additional legacy workflow reference CTest
  matched all complete reference traces byte-for-byte; compressed fixtures decompress
  to the original independent baseline, not candidate-generated replacements.
- Source and frozen full JIP/ITK fields and N4 outputs matched exactly in the existing
  numerical qualification. No separate JIP app or host Python is needed by end users.
- Native Apple Silicon Electron app built and passed ad-hoc signature, exact ASAR
  frontend, embedded CPU identity, compiled UI/emitted-worker and real packaged
  JIP/ITK IPC verification. App source and numerical C/C++ implementation are unchanged.
- Electron-builder configuration resolution checked for exact dmg/nsis+zip/AppImage+deb
  target lists and inherited embedded-runtime payloads.
- GitHub workflows passed actionlint; JavaScript/Python syntax and full-dist browser
  fixture HTTP/module/token/isolation smoke checks passed.

## Limits that remain explicit

This host's sandbox prevents disk-image creation (`hdiutil: Device not configured`)
and Chrome startup. It therefore did not qualify the DMG container or native GPU
pixels. It also cannot execute native Intel/Windows/Linux runners. Those targets and
real browser suites MUST pass the initial manual GitHub workflow run. Their success
is not implied by configuration syntax or the local Apple Silicon app test.

The local worker build used Python 3.11.13; the hosted workflows request Python 3.12
(the application's existing production worker has also used Python 3.12). This is
not a claim of bit-identical installers between toolchain versions. Build reports
record the actual compiler, CMake, platform, architecture and native-test coverage.

The upload source excludes dependencies, compiled runtimes/apps, caches, generated
validation output, credentials and private subject datasets. Immutable synthetic
reference data, pinned vendor source archives and licenses are retained. This folder
is an upload-ready repository, not an assertion that all platform builds have
already run or that anything has been uploaded/published to GitHub.
