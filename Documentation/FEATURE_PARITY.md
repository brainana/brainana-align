# Feature parity status

Global automatic linear initialization is now an additive workflow beyond the v0.15.4 reference. It supports 6-, 7-, 9-, 12-, and custom proper transforms, broad orientation search, multiple intensity metrics, and fixed-image-relative pyramids without requiring landmarks. Existing landmark fitting and constrained local refinement remain available and share the same physical NIfTI coordinate, review, session, and export paths.

## Electron desktop prototype

The Electron target uses the same compiled frontend as the browser release, including MRI/CT loading, visualization, landmarks, selectable linear fitting, an optional regularized nonlinear landmark warp, optimization windows, refinement, ROI operations, sessions, and exports. Landmark alignment defaults to a six-parameter rigid model and also supports seven-parameter similarity, nine-parameter independent axis scaling without shear, full 12-parameter affine, and a custom selectable subset of the 12 linear parameters. Reflected affine fitting is off by default and requires explicit opt-in and confirmation. Resetting the alignment preserves landmarks and optimization windows; the nonlinear stage can also be reset independently. Local loading is the default. Remote MRI/CT loading and remote export, saved profiles, interactive system-SSH authentication, and structured SFTP browsing are available on demand without a startup mode prompt. A profile's starting directory is only the initial location; users can navigate throughout the remote filesystem wherever their SSH account has permission.

This release reconstructs the retained v0.15.4 frontend behavior in readable TypeScript. The recovered minified frontend and injected export patch are not part of the production runtime.

| Feature | v0.15.4 reference | current release | Source status |
|---|---:|---:|---|
| MRI and CT loading | Yes | Yes | Readable TypeScript |
| Landmark editing | Yes | Yes | Readable TypeScript |
| Rigid fit and manual adjustment | Yes | Yes | Readable TypeScript |
| Rigid, similarity, anisotropic-scale, affine, and custom linear landmark fits | No | Yes | Readable TypeScript |
| Linear diagnostics and opt-in reflection safeguards | No | Yes | Readable TypeScript |
| Regularized nonlinear landmark warp with presets and custom smoothing | No | Yes | Readable TypeScript |
| Nonlinear Jacobian, folding, and displacement QC | No | Yes | Readable TypeScript |
| Reset alignment while retaining landmarks/windows | No | Yes | Readable TypeScript |
| Automatic NMI refinement | Yes | Yes | Readable TypeScript |
| Automatic deformable NMI refinement | No | Yes | Readable TypeScript |
| Deformable cancellation, Jacobian bounds, and fold rejection | No | Yes | Readable TypeScript |
| Optimization refinement windows | Yes | Yes | Readable TypeScript |
| Review overlay | Yes | Yes | Readable TypeScript |
| Transform and aligned-volume export | Yes | Yes | Readable TypeScript |
| Session save/load, including windows | Yes | Yes | Readable TypeScript |
| Workstation file browser | Yes | Yes | Readable TypeScript |
| Workstation export folder browser | Yes | Yes | Readable TypeScript |
| Contextual six-stage registration workflow and visible transform stack | No | Yes | Readable TypeScript |
| Bundled dual-architecture Node | No | Yes | Packaged runtime |
| Remote Node/server upload | Yes | No | Replaced with local SSH architecture |
| Compiled recovered frontend required | Yes | No | Absent from production runtime |

## Validation boundary

Build, static parity, server, packaging, and synthetic algorithm checks were completed in the current environment. Finder launch, interactive WebGL behavior, and institutional SSH/Duo remain real-machine smoke tests.


## Overlay crosshair synchronization
Validated in source and production build: changing the crosshair in any overlay plane updates sagittal, coronal, and axial overlay views through the shared 3D coordinate state.
