# Brainana Align

A standalone desktop application for aligning brain images, with paired landmarks,
manual adjustment and image-based registration. It supports physical-coordinate
rigid, similarity, anisotropic and affine transforms, advanced landmark splines,
embedded JIP/ITK nonlinear registration, preparation tools and native-grid export.

## Download

Use this repository's **Releases** page and choose the installer for your computer:

| Computer | Release asset |
|---|---|
| macOS 14+, Apple Silicon | `brainana-align-VERSION-arm64.dmg` |
| macOS 14+, Intel | `brainana-align-VERSION-x64.dmg` |
| Windows x64 | `brainana-align-setup-VERSION-x64.exe` |
| Linux x64 (Ubuntu22.04+ or compatible glibc2.35+) | `.AppImage` or `.deb` |

Users do not need Node.js, Python, a compiler, or a separate registration application.
The scientific worker is built and included inside Align. Published installers are
release assets, never files committed to this source repository.

**Build status:** the workflow targets all four combinations above. Windows/Linux/Intel
qualification is established by their first successful native GitHub run, not by the
existence of these workflow files. Manual runs produce downloadable candidate artifacts
without publishing. Release uploads are gated on every platform's tests and package proof.
Headless checks do not certify native GPU appearance or clinical registration accuracy.

## Use

1. Choose the image pairing, select the two images and prepare working volumes.
2. Add paired landmarks, adjust manually or use independent image-based registration.
3. Inspect the overlay and diagnostics. Accept or reject registration proposals explicitly.
4. Use **Alignment display → Image comparison** for smooth opacity or fixed/moving flips.
5. Export results or save a session. Keep the original image files for later restore.

The default landmark transform remains **Rigid, 6 parameters**. Sessions and exports
record transform direction and physical coordinates. Existing user preferences and the
application identity are preserved; this publication setup does not replace Viewer or
copy its imaging functionality.

## Source organization

```
apps/align/          Desktop entry, server and readable TypeScript UI
packages/           Shared desktop, launcher, server, client and imaging mathematics
scientific/         Embedded numerical source, pinned vendor source and licenses
scripts/            Version, validation, packaging and CI orchestration
.github/            Source checks and native release matrix
tests/             Coordinate, region, scientific, worker, UI and lifecycle regressions
docs/               Contributor and publishing instructions
Documentation/      Consolidated existing user/technical/release documentation
```

- [Build and test](docs/building.md)
- [Publish a release](docs/publishing.md)
- [Architecture](docs/architecture.md)
- [User documentation](Documentation/README.md)
- [Changes](Documentation/CHANGELOG.md)
- [Maintenance contracts](AGENTS.md)

## Quick development start

Use Node24 and Python3.12; install a native C/C++ compiler, Make and Ninja/CMake as
explained in the build instructions.

```sh
npm ci
npm run build
npm run ci:test
```

These commands build and test the frontend/core. For the full embedded numerical
engine, run the native build procedure before `npm run dev:desktop`. On Windows,
use the documented MSYS2 MINGW64 toolchain; MSVC is not a supported engine toolchain.

## License and acknowledgements

Align is **AGPL-3.0-only**; see [LICENSE](LICENSE) and [NOTICE.txt](NOTICE.txt).
The scientific subtree retains its original JIP/FFTW/ITK and other third-party source,
checksums and license notices. Private subject datasets and credentials are excluded from the publication export. Synthetic reference fixtures are intentionally retained for regression tests.
The source/release layout follows the same approach as
[Brainana Viewer](https://github.com/brainana/brainana-viewer), while retaining Align's
separate registration functionality and embedded scientific runtime.
