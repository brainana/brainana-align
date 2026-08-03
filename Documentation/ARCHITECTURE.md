# Brainana Align architecture

The global automatic-linear engine is isolated in `src/domain/registration/automaticLinear.ts`. It owns orientation candidates, proper-transform parameterization, enabled degrees of freedom, staged optimization, and diagnostics. The application supplies physical-coordinate metric samples and presents results for acceptance. Registration pyramids use explicit shrink factors relative to each input grid rather than assumptions about species, field of view, occupied brain voxels, or absolute millimeter scale.

The automatic strategy is an original implementation of established multiresolution registration ideas: broad orientation candidates, bounded derivative-free line searches, retained distinct solutions, perturb-and-retry, and progressive release of scale and shear parameters. FSL source is not linked, bundled, translated, or copied. Brainana Align does not claim numerical identity with FLIRT.

## User-interface composition

The imaging workspace remains a persistent three-by-three MRI, CT, and aligned-overlay grid. Registration operations are organized as a widescreen horizontal workflow with three explicit groups: Landmark guided (Points, Linear fit, and optional Nonlinear warp), Image driven (Linear intensity and Deformable), and Output (Review). Selecting a stage displays only its contextual controls in the right inspector; display mode and overlay opacity remain attached to the viewer rather than a registration stage.

Actions are owned by their stage rather than by the global application toolbar. Landmark creation and undo therefore live in the Landmarks panel, while the top toolbar is reserved for volume, session, view, and export operations. Tab-to-panel ARIA relationships and Arrow/Home/End keyboard navigation preserve accessibility without changing scientific state.

The accepted transform state is summarized independently of the selected inspector. This prevents navigation from changing scientific state and makes the active sequence of linear, landmark-warp, intensity, and deformable operations continuously visible. Workflow navigation is implemented in `src/ui/workflowNavigation.ts`; scientific handlers continue to address stable control IDs and remain independent of stage visibility.

## Landmark linear-registration models

Landmark pairs are stored and fitted in physical NIfTI millimeter coordinates. The selected transform direction explicitly determines the moving and fixed landmark arrays. The active NIfTI affine is retained for voxel/world conversion during display, resampling, and export, so landmark matrices are not applied in voxel space or composed with image affines twice.

The shared linear-registration module provides rigid (6), similarity (7), independent axis scale without shear (9), affine (12), and custom selectable-parameter models. Every result is represented as a full 4-by-4 moving-to-fixed matrix with a general inverse. Diagnostics record parameter count, determinant, axis-scale estimates, normalized shear magnitude, reflection, condition estimate, and warnings.

Affine reflection is a safety-controlled capability. It is disabled by default, must be explicitly enabled for a fit, and requires confirmation before the result is applied, restored, or exported. This prevents a numerically better mirrored brain from being accepted silently.

## Renderer source layers

The readable TypeScript renderer is organized by responsibility rather than as a flat collection of files:

- `src/app` composes the application and owns shared application types.
- `src/domain` contains registration, imaging/ROI, optimization, session, and export rules.
- `src/viewer` contains coordinate projection and interactive view rendering.
- `src/infrastructure` contains runtime, Electron bridge, remote endpoint, and filesystem adapters.
- `src/ui` contains browser-facing dialogs, compatibility presentation, and styles.

Domain modules must not depend on DOM, Electron, HTTP, or SFTP implementations. Viewer and UI modules may depend on domain types and operations. Infrastructure implements external boundaries and is composed by the application layer. Architecture tests enforce the physical layout and reject reintroduction of the previous flat entry modules.

## Linear registration models

The registration domain exposes a common linear-fit result containing the forward matrix, general affine inverse, residuals, RMS error, determinant, model identifier, and enabled parameters. Rigid registration uses the established closed-form solver. Similarity adds a positive uniform scale. Affine uses a rank-validated least-squares solution. Custom parameter subsets use a damped constrained optimizer centered on the source-landmark centroid; disabled parameters retain their identity values.

Manual adjustment and automatic refinement currently apply rigid increments around any accepted linear base transform. They do not silently add scale or shear. Sessions and exports record the landmark model and enabled parameter set.

## Nonlinear landmark transform

The optional nonlinear stage is an explicit transform after the accepted linear landmark fit. It uses a regularized three-dimensional order-2 polyharmonic radial-basis model with an affine polynomial tail. The forward model fits residual landmark displacement in fixed-image space after the linear transform. A separately fitted reverse model supports reverse resampling and linked navigation without hiding an iterative inverse inside rendering code.

The application treats the transform chain as `moving -> accepted linear -> nonlinear landmark warp -> fixed`. Anatomical coordinates remain the source of truth; display coordinates are derived. Serialized sessions and registration exports contain the full spline control points, weights, affine terms, regularization value, fit residuals, and Jacobian summary. Sampled nonpositive Jacobians cause rejection rather than silently accepting a fold.

Rigid image-intensity refinement remains available as a separate operation. The deformable image-intensity stage composes after the optional landmark warp; it does not reinterpret or refit the landmark model.

## Automatic deformable transform

The image-driven deformable stage stores an inverse displacement field on a coarse target-space control grid. Trilinear interpolation maps each final fixed-space sample into the pre-deformation fixed space, after which the optional reverse landmark warp and inverse linear transform map it into the moving image. A fixed-point inverse supplies the corresponding forward map for linked navigation and annotations.

Optimization proceeds coarse-to-fine with deterministic coordinate searches over control-point displacements and normalized mutual information. The objective includes first-derivative smoothness regularization. Every candidate is bounded in displacement and screened against configured sampled Jacobian limits; folds are never accepted. The result is a proposal until explicitly accepted and can be cancelled, rejected, or reset independently.

## Electron desktop target

The Electron main process starts the existing secured local server using Electron's embedded Node runtime, waits for its private atomic handshake, and loads the tokenized localhost URL into a hardened `BrowserWindow`. The renderer has no Node integration, uses context isolation and sandboxing, and cannot navigate away from its assigned localhost origin. Closing the application terminates the owned server.

This transitional design deliberately retains the server boundary so the scientific frontend and browser build remain identical. A later desktop phase may replace local HTTP APIs with narrow Electron IPC after parity is established.

## Shared application core

The frontend, scientific alignment logic, session model, export construction, local server, SFTP transport, and automated tests are shared across platforms. Platform-specific code is restricted to launchers, folder selection, browser opening, process lifecycle, runtime selection, and package assembly.

The intended repository model is one shared source tree with platform directories for macOS, Linux, and Windows. Linux and Windows packaging are not yet complete releases.

## macOS runtime

The macOS application bundles Apple Silicon and Intel Node executables. The launcher selects the runtime from physical hardware architecture, starts a loopback-only server, waits for a secure launch handshake, and opens the authenticated browser URL.

Local mode opens without a startup data-folder chooser. MRI and CT files are selected with the dedicated browser controls. The user's home directory is supplied internally as the server-side filesystem containment boundary. Export destination selection occurs when an export is requested.

Remote mode uses a saved workstation profile containing a starting directory. In the Electron target this path is a convenience location, not a security boundary: users may navigate upward to `/` and access any path allowed by their authenticated SSH account. The app establishes an interactive system-SSH control connection in Terminal, verifies it with `ssh -O check`, then performs structured remote filesystem operations through SFTP. No server or Node runtime is installed on the workstation. Browser-mode remote roots remain contained for compatibility with that launch model.

The Electron remote browser is rendered locally and presents Finder-style navigation over SFTP. History, breadcrumbs, search, sorting, remembered locations, and hidden-file visibility are interface state only; they do not invoke or require a graphical desktop environment on the remote workstation.

## Storage and security

The server binds only to loopback. Each launch uses an unpredictable session credential and a fresh port handshake. Local and browser-mode filesystem operations remain contained beneath their configured roots. Electron remote operations normalize paths but deliberately use `/` as their navigation root, leaving authorization to the SSH account and remote operating system. Temporary-file and replacement procedures are intended to avoid partial output. Remote filesystem requests use structured SFTP messages rather than shell command construction.

## Browser compatibility

Safari, Firefox, Chrome, and Edge are first-class targets. Core behavior must not require Chromium-only APIs. Folder APIs are optional enhancements; ordinary file inputs, browser downloads, and server-side export remain supported fallbacks.

Automated CI is designed to run the production bundle through Playwright Chromium, Firefox, and WebKit. Native certification additionally requires real Safari on macOS, Firefox on macOS and Linux, Chrome across supported systems, and Edge on Windows because WebGL drivers, file dialogs, privacy prompts, and graphics acceleration cannot be fully represented by engine automation.

## Documentation and release model

Documentation is maintained as a small current-state set. `CHANGELOG.md` is the single running historical record. New releases update the existing architecture and validation documents instead of creating version-stamped copies. Git history and release archives provide deeper historical snapshots.
