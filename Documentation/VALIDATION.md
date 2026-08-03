# Validation status

## 0.17.26-workflow-ui.1

Completed in the current macOS development environment:

- production TypeScript and Vite build
- workflow source regression tests for grouping, action ownership, unique IDs, tab/panel ARIA relationships, and keyboard navigation
- Electron preload and main-process integration checks
- production UI rendering at 1920 x 1080 in the Codex in-app browser; all three groups fit horizontally without document overflow, and New landmark/Undo were confined to the Landmarks panel
- complete scientific regression suite, because this release intentionally changes presentation only

The legacy external Playwright commands `test:browser` and `test:browser-matrix` were not executed because their configured `/usr/bin/chromium` executable is absent on this Mac. This is reported separately from the successful in-app production rendering check above.

Native packaged applications remain unsigned development artifacts. macOS signing/notarization and Windows signing require release credentials.

## 0.17.25-flirt-inspired.1

Completed in the current macOS development environment:

- strict TypeScript compilation and Vite production build
- input-relative `8,4,2,1`, `4,2,1`, and custom shrink-factor validation
- preservation of physical origin, orientation, and far-corner extent after factor-based pyramid construction
- synthetic translated 90-degree rigid recovery through multistart orientation search and bounded line optimization
- positive-determinant/reflection-impossible automatic-transform checks
- source checks for automatic/custom smoothing and sweep schedules and removal of sample-count-based controls
- complete pre-existing scientific, session/export, workflow, Electron, server-security, storage, SFTP, remote, and lifecycle regression gate

Not executed here: known-answer comparison against independent FLIRT outputs on curated MRI/CT volumes, native interactive timing on very large images, signed/notarized distribution testing, and real remote authentication. Automatic output remains a proposal requiring visual review and explicit acceptance.

## 0.17.24-auto-linear.1

Completed in the current macOS development environment:

- strict TypeScript compilation and Vite production build
- synthetic translated 90-degree rigid recovery through global orientation search
- positive-determinant and reflection-disabled automatic-transform checks
- species-relative pyramid tests for representative human, macaque, and tree-shrew geometries
- source checks for all models, metrics, angular searches, and reference-relative resolution controls
- existing scientific, landmark-linear, image-pyramid, session/export, workflow UI, Electron, remote-Electron, server security, storage, SFTP, and remote endpoint checks

Not executed here: known-answer registration against a curated real MRI/CT dataset, native interactive review of every metric, and signed/notarized distribution testing. Automatic output therefore remains a proposal requiring visual acceptance.

## 0.17.23-ui.1

Completed in the current macOS build environment:

- strict TypeScript compilation and Vite production build
- source checks for all six workflow tabs and contextual panels
- unique static DOM identifier verification after moving controls between stages
- independent rigid-intensity and deformable candidate controls
- persistent transform-stack and viewer-display controls
- keyboard Arrow Left/Right navigation between workflow stages
- linear, nonlinear-landmark, deformable, image-pyramid, opacity, status-overlay, user-feedback, Electron, and remote-Electron source regressions

The installed Chrome executable could not be launched headlessly under the current managed sandbox, so the production-browser interaction test remains to be executed outside that sandbox. Native Finder launch and interactive resizing remain real-machine checks for the packaged application.

## 0.17.22-multires.1

Completed in the current macOS build environment:

- Gaussian-prefiltered image reduction preserves the NIfTI physical origin, far-corner extent, orientation columns, and finite voxel data
- Fast, Balanced, Precise, and Custom resolution sequences are validated and executed coarse-to-fine for rigid and deformable intensity registration
- registration matrices and deformation grids remain millimeter/world-space objects, while review and export still resample onto the untouched native fixed-image grid
- optimization resolution metadata survives the existing session/export serialization path
- opacity dragging is animation-frame coalesced and does not reapply colormaps on every input event
- strict TypeScript compilation, production build, pyramid, opacity, scientific, registration, deformation, and Electron source checks

## 0.17.21-feedback.1

Completed in the current macOS build environment:

- landmark fitting yields across two animation frames so the processing overlay is visibly painted before synchronous computation begins
- accepting, rejecting, applying, resetting, saving, and restoring operations that rebuild images enter and leave the blocking processing state through `finally`
- both modality and review NiiVue instances have an empty built-in loading label, and unloaded MRI/CT panels contain no placeholder text
- parameter explanations use a single controlled fixed tooltip and explicitly dismiss on pointer leave, focus loss, scroll, resize, outside pointer input, and Escape
- strict TypeScript compilation, production build, feedback source checks, scientific registration tests, and Electron integration tests

## 0.17.20-status-ui.1

Completed in the current macOS build environment:

- the rendered topbar contains no status element or initial loading instruction
- toolbar width is reclaimed rather than reserved for clipped text
- every idle status call uses the full dismissible notification, while active processing updates the centered overlay
- a hidden polite live region mirrors the complete status for screen readers
- MRI/CT empty-state messages remain visible in their respective image panels
- strict TypeScript compilation, production build, status UI checks, deformable/registration/scientific regressions, and Electron integration checks

## 0.17.19-deformable.1

Completed in the current macOS build environment:

- a second deformable optimization begins with the accepted first-pass displacement field after resampling it onto the requested control-grid levels
- the first objective evaluation of the second-pass regression fixture observes the accepted nonzero deformation rather than identity
- pending-proposal state blocks ambiguous reruns until Accept or Reject is chosen
- Accept retains the proposal as the next pass's starting deformation; Reject restores the preceding deformation
- custom-setting validation reports each invalid field independently
- every deformable parameter has hover and keyboard-focus help, with panel overflow handling so explanations are not clipped
- strict TypeScript compilation, production build, deformable/scientific regressions, status overlay checks, and Electron integration checks

## 0.17.18-status.1

Completed in the current macOS build environment:

- processing-overlay DOM, wrapped status text, dimmed interaction surface, spinner, dismissible completion/error notification, and accessibility attributes
- image loading, landmark fitting, rigid NMI refinement, deformable refinement, transform saving, and aligned-image resampling all enter processing state and clear it through `finally`
- progress callbacks update the same overlay instead of creating stacked notifications
- error paths retain the complete message in the notification and toolbar tooltip
- remote authentication and transfer messages ending in an active-progress ellipsis automatically use the overlay and close it on the terminal status
- strict TypeScript compilation and production build

## 0.17.17-correspondence.1

Completed in the current macOS build environment:

- the reported six-landmark fixture still uniquely identifies correspondence indices 5 and 6
- the repair dialog maps diagnostic indices back to the actual landmark IDs, including when disabled or incomplete landmarks are present
- separate CT and MRI swap actions exchange only the selected modality coordinates
- swaps enter undo history, invalidate stale residuals, retain reflection-disabled state, and require a new explicit fit
- cancel, close, and Escape preserve all coordinates and select the suspected landmark for manual review
- the reflection alternative only enables the checkbox and still does not fit or apply a transform
- strict TypeScript compilation, production build, source UI regression checks, and linear scientific regressions

## 0.17.16-handedness.1

Completed in the current macOS build environment:

- the reflection diagnostic is not run unless a reflection-disabled affine has first failed plausibility QC
- requires six or more correspondences and at least 80% weighted tetrahedral handedness reversal
- rejects reflected diagnostic candidates with implausible scale, shear, condition, or determinant
- the reported coordinate fixture produces a plausible reflected RMS below 3 mm, greater than 90% weighted handedness reversal, and uniquely identifies landmarks 5 and 6 as the suspected crossed pair
- correcting that pair suppresses the reflection diagnostic and produces a stable proper affine
- accepting the popup only enables the reflection checkbox; it does not fit or apply a transform
- strict TypeScript compilation, production build, and all existing linear/nonlinear scientific regressions

This release provided diagnostic guidance only and never treated lower reflected RMS alone as sufficient evidence that anatomy should be mirrored. Version 0.17.17 adds an explicit, user-selected correspondence repair.

## 0.17.15-affine-qc.1

Completed in the current macOS build environment:

- exact replay of the six MRI and CT landmark coordinates reported by the user
- rejection of the reported orientation-preserving affine because it collapses one axis and produces extreme shear
- recovery after swapping only CT landmark assignments 5 and 6: RMS below 3 mm, positive determinant, and shear below 0.1
- rejection thresholds for axis scale below 0.2 or above 5, normalized shear above 0.75, or determinant below 0.01
- all rigid, similarity, anisotropic-scale, proper affine, reflected affine, physical-coordinate, inversion, degeneracy, session, and export fixtures
- strict TypeScript compilation and production frontend build

The application does not automatically exchange landmarks because doing so could conceal a genuine labeling error. Users remain responsible for confirming that every numbered MRI point represents the same anatomical location as the correspondingly numbered CT point.

## 0.17.14-affine.1

Completed in the current macOS build environment:

- reflection-disabled affine fitting traverses only a positive-determinant parameterization
- a synthetic reflected correspondence set returns a proper best-effort fit with positive determinant rather than being rejected or mirrored
- the same fixture returns the exact negative-determinant transform only through the explicit reflection-enabled path
- exact recovery of a 137-degree oblique proper affine containing scale and shear
- exact recovery of the previous affine, similarity, anisotropic-scale, rigid, physical-coordinate, and inverse fixtures
- rejection of insufficient and geometrically rank-deficient affine landmarks
- strict TypeScript compilation, production bundling, session/export regression, and source verification

Real MRI/CT landmark placement and visual overlay review remain target-machine validation items. Reflection remains an advanced opt-in capability and must not be selected solely to reduce landmark RMS.

## 0.17.13-landmark-errors.1

Completed in the current macOS build environment:

- reflected affine candidates remain rejected when reflection is not enabled
- the rejection produces a complete inline explanation and explicit safety dialog
- the reflection control is highlighted and brought into view without being enabled automatically
- non-reflection fit failures remain visible in the Landmark alignment panel
- toolbar status exposes the complete message through its title even when visual space is limited
- all 0.17.12 scientific landmark-model, physical-coordinate, session, export, server, remote, and lifecycle regressions

The exact dataset shown by the user was not available for automated landmark replay. The observed failure text matches the intentional reflected-affine rejection path; this release corrects the missing explanation and discoverability, not the reflection safety policy.

## 0.17.12-landmarks.1

Completed in the current macOS build environment:

- exact rigid recovery at 4, 90, 137, and 179.9 degrees around an oblique axis
- exact seven-parameter similarity recovery
- exact nine-parameter anisotropic axis-scale recovery without shear
- exact twelve-parameter affine recovery and forward/inverse round trips
- CT-to-MRI and MRI-to-CT fitting with distinct oblique, anisotropic physical NIfTI affines
- rejection of insufficient, collinear, rank-deficient, singular, and unapproved reflected fits
- explicit reflected-fit opt-in, application confirmation, restore confirmation, and export confirmation source gates
- determinant, scale, shear, reflection, condition, and warning serialization
- migration of older sessions to schema 5
- strict TypeScript compilation and the existing scientific, nonlinear-landmark, deformable, coordinate, window, session, runtime, Electron, remote, security, and packaging gates

The application stores landmark locations in the physical millimeter coordinates exposed by NiiVue from the active NIfTI affine and uses the raw NIfTI affine for voxel/world resampling. Real oblique clinical datasets, interactive WebGL review, and native execution of every packaged platform remain target-machine validation items.

## 0.17.11-deformable.1

Completed in the current macOS build environment:

- identity inverse-grid and forward fixed-point mapping
- uniform deformation forward/inverse round trips
- identity and uniform-field Jacobian determinants equal to one
- preservation of a uniform field across grid refinement
- zero first-derivative penalty for uniform translation
- deterministic coordinate-search improvement on a synthetic objective
- cancellation and Jacobian/folding safety paths
- strict TypeScript compilation, Vite production build, session migration, export serialization, and existing scientific regressions
- common production frontend assembly for Apple Silicon, Intel macOS, Ubuntu x64, and Windows x64

Target-machine validation remains required for real MRI/CT convergence, runtime and memory performance on full volumes, interactive cancellation, WebGL review, and scientific selection of preset defaults. This is a conservative first deformable milestone, not a claim that every MRI/CT dataset can be registered automatically.

## 0.17.10-nonlinear.2

Completed in the current macOS build environment:

- all nonlinear landmark scientific fixtures from 0.17.9
- preset resolution independent of the custom field
- exact preservation of custom zero and fractional values
- rejection of empty, negative, NaN, and infinite custom values
- source checks for Custom selector visibility, numeric input constraints, conditional display, and fitter integration
- strict TypeScript compilation, production build, existing scientific regressions, and cross-platform package assembly

Real-data behavior, WebGL interaction, and full-volume nonlinear resampling performance remain target-machine validation items.

## 0.17.9-nonlinear.1

Completed in the current macOS build environment:

- strict TypeScript compilation and Vite production build
- exact interpolation of a synthetic landmark deformation with zero regularization
- identity-warp numerical Jacobian determinants equal to one
- regularized forward/reverse landmark round trips within the fixture tolerance
- rejection of insufficient landmark sets and sampled nonpositive Jacobians
- nonlinear transform use in navigation, review, volume resampling, sessions, and exports
- migration of older session schemas with no nonlinear warp
- existing linear, rigid, scientific, coordinate, window, session, runtime, Electron, remote, and platform source regressions

Environment-limited checks:

- Interactive WebGL behavior and resampling performance on full clinical volumes require target-machine validation.
- The independently fitted reverse warp is validated at landmarks and by sampled consistency checks; it is an approximation away from the landmark support.
- Automatic image-driven deformable registration is not included in this milestone.
- Loopback listener and browser automation limitations remain as documented for earlier releases.

## 0.17.8-linear.1

Completed in the current macOS build environment:

- strict TypeScript compilation and Vite production build
- exact recovery of a known seven-parameter similarity transform
- exact recovery of a known twelve-parameter affine transform
- forward/inverse affine round-trip checks
- custom shear-only fit with every unselected parameter held at identity
- insufficient-landmark and rank-deficiency rejection
- reset-alignment checks confirming landmarks and optimization windows are retained
- session schema migration and transform-model export checks
- existing rigid, scientific, coordinate, crosshair, optimization-window, session, runtime, Electron, SSH source, and platform regressions

The Codex sandbox still prevents loopback-listener integration tests and does not provide Chromium UI automation. Visible interaction and real-data similarity/affine fitting require target-Mac confirmation.

## 0.17.7-structure.1

Completed in the current macOS build environment:

- strict TypeScript compilation and Vite production build
- release identity and consolidated-documentation checks
- layered source-architecture and dependency-path checks
- coordinate, crosshair, landmark, optimization-window, session, export, runtime, and scientific regressions
- browser-compatibility, platform-core, Electron isolation, and Electron remote-integration source checks
- Apple Silicon and Intel Electron assembly from the same production frontend
- strict deep ad-hoc signature verification for both macOS packages
- Ubuntu x64 and Windows x64 package assembly from cached official Electron runtimes

Environment-limited checks:

- Loopback server, storage, SFTP, and detached-lifecycle integration tests could not execute because the Codex sandbox rejected `listen(127.0.0.1)` with `EPERM`; this is an execution restriction, not a test assertion failure.
- Chromium UI automation could not execute because no Chromium executable is installed in this environment.
- Real Finder launch, institutional SSH authentication, and native Ubuntu/Windows execution remain target-machine validation steps.

## 0.17.4-export-browser.3

- Added a regression check rejecting nested `main.ba-export-main`, which inherited the application's full-height main layout.
- Added checks for the top-right Close action and fixed Select this folder footer action.
- Re-ran production build, export, SFTP, Electron, and scientific validation.

## 0.17.4-export-browser.2

- Added layout checks requiring a non-shrinking export action footer and internally constrained scroll regions.
- Verified the packaged frontend contains the visible Select this folder action.
- Re-ran build, export, remote endpoint, storage, Electron, and scientific regression tests.

## 0.17.4-export-browser.1

- Verified compilation and production bundling of the Finder-style export-folder chooser.
- Added source checks for history, breadcrumbs/direct paths, search, sorting, hidden folders, root navigation, remembered locations, and folder creation.
- Re-ran remote endpoint, SFTP, storage, security, export, Electron, optimization-window, and scientific regression suites.
- Visual interaction and a real workstation write remain real-Mac confirmation steps.

## 0.17.3-remote-export.1

- Verified that the Export destination selector appears when an Electron remote endpoint becomes active and returns to Local when disconnected.
- Verified remote folder listing, folder creation, overwrite confirmation, and rollback-capable SFTP save routing.
- Re-ran remote endpoint authentication, SFTP transport, storage, local export, Electron isolation, and scientific regression tests.
- A real workstation export remains the final institutional SSH/Duo smoke test.

## 0.17.2-remote-browser.1

- Verified TypeScript compilation and production rendering of the Finder-style browser.
- Added release checks for history navigation, breadcrumbs/direct paths, search, sorting metadata, hidden files, root navigation, and remembered locations.
- Verified local and SFTP directory metadata handling, remote endpoint authentication, server path security, and packaged Electron preload isolation.
- Re-ran scientific, coordinate, session, export, and all optimization-window regressions.
- Interactive visual confirmation and institutional SSH browsing remain real-Mac tests.

## 0.17.1-remote.3

- Verified that Electron starts its remote SFTP endpoint at `/` while returning the saved profile directory as the browser's initial location.
- Verified that the remote browser enables upward navigation until the remote filesystem root is reached.
- Verified that subsequent MRI/CT browsing retains the last visited remote directory during the active connection.
- Re-ran TypeScript, production-build, Electron remote integration, remote endpoint, SFTP transport, server security, scientific, coordinate, and optimization-window tests.
- Real remote permissions remain enforced by the SSH server and require confirmation against the user's workstation account.

## 0.17.1-remote.2

- Confirmed the prior package contained an ES-module preload incompatible with the sandboxed renderer configuration.
- Replaced it with a CommonJS preload using only Electron's permitted sandbox APIs.
- Verified preload syntax, packaging path, context isolation, absence of filesystem/process imports, TypeScript build, and Electron source checks.
- The Codex execution environment could not launch a visible Electron process through macOS services; the two dropdown arrows remain a real-Mac confirmation.

## 0.17.1-remote.1

Completed locally:

- TypeScript compilation and production build
- Electron preload isolation and IPC surface checks
- profile-schema and non-secret storage source checks
- separate remote endpoint session-token checks
- cross-origin preflight and origin-policy integration test
- SFTP protocol transport tests
- local server, security, storage, and export regression tests
- Electron app assembly and strict deep ad-hoc signature verification

Interactive password/Duo authentication and browsing against a real workstation require confirmation on the user's Mac. Remote export selection remains a later Electron integration step; this release adds remote MRI/CT loading.

## 0.17.0-electron.4

- Executed a deterministic MRI axial regression fixture in which NiiVue reports both drag endpoints at the same coordinate; the shared rendered-plane projection correctly produced a 50 × 50 mm window.
- Re-ran exhaustive missing-plane tests for sagittal, coronal, and axial constraints.
- Re-ran window completion, invalid-drag preservation, TypeScript compilation, production build, scientific, session, Electron source, package-signature, and archive-integrity checks.
- Visible MRI axial drawing remains a final real-Mac interaction check.

## 0.17.0-electron.3

Validated on Apple Silicon macOS:

- exhaustive missing-plane optimization-window semantics
- shared six-view window interaction source path
- invalid-drag preservation of existing windows
- coordinate-projection fallback checks
- TypeScript compilation and production Vite build
- Electron security/source checks

The automated browser UI runner could not execute in the Codex environment because its configured standalone Chromium executable is not installed. Visible MRI/CT window drawing in the packaged Electron application remains a real-Mac interaction check.

## 0.17.0-electron.1

Validated on Apple Silicon macOS:

- TypeScript compilation and production Vite build
- Electron security/source checks
- scientific transform regression tests
- optimization-window tests
- coordinate, crosshair, landmark, session, and export tests
- Apple Silicon Electron bundle assembly
- strict deep ad-hoc code-signature verification
- packaged Electron runtime execution in Node mode
- packaged server handshake and authenticated health response

The Codex execution sandbox cannot communicate with macOS LaunchServices, so the final visible-window/Finder launch must be tested outside that sandbox. Remote mode is not claimed for this Electron milestone.

## Current release: 0.16.26-docs.1

This release changes documentation organization and release validation only. Application source and runtime behavior are unchanged from 0.16.25-local-start.1.

Completed checks:

- TypeScript and Vite production build
- release identity consistency
- documentation-layout enforcement
- source architecture checks
- coordinate, crosshair, landmark, and optimization-window tests
- scientific transform regression tests
- session and export tests
- server smoke, security, storage, and local-export tests
- platform path and SFTP transport tests
- browser UI and compatibility tests
- Chromium production-bundle engine test in the available environment
- launch-handshake and detached-lifecycle stress tests
- macOS runtime-selection, launchd, SSH cancellation, Terminal-helper, and Local-mode startup tests
- packaged frontend hash comparison
- bundled runtime and launcher verification
- ZIP integrity, release manifest, documentation inventory, and checksum validation

## Browser certification boundary

The repository requires Chromium, Firefox, and WebKit automation in CI. In the current build environment, only Chromium was available for execution. Firefox, WebKit, native Safari, and native Edge are not claimed as certified until their required tests run successfully. Native Safari remains necessary because WebKit automation does not fully test Safari-specific WebGL, graphics-driver, privacy, and file-dialog behavior.

## Native system boundary

Finder launch, Dock behavior, Terminal LaunchServices behavior, interactive institutional SSH authentication, sleep/wake, and real network interruption require validation on actual macOS hardware. The current remote authentication flow has been confirmed by the user on macOS after the SSH correction. Linux and Windows remain architectural foundations rather than deployable packages.

## Historical validation policy

Major changes to validation are summarized in `CHANGELOG.md`. Detailed historical snapshots remain available in prior release archives and version-control history rather than duplicated inside every new source package.
# 0.17.5-remote-ui.1

Validated the shared import/export modal layout, explicit import close and action controls, authentication TTY capture, and success-only Terminal-tab cleanup. Real password authentication and Terminal automation still require confirmation on a user Mac.
# 0.17.5-remote-ui.2

Added structural layout assertions covering every flex/grid ancestor between the import dialog and its footer. The Terminal close script is stored separately, compiled with AppleScript tooling, packaged explicitly, and invoked only after a zero authentication exit status. A close-attempt log is retained in the authentication diagnostics directory.
# 0.17.6-platforms.1

All four platform payloads are assembled from the same generated frontend and Electron main process. Binary formats, required resources, version identity, executable permissions, macOS signatures, and archive integrity are checked locally. Apple Silicon macOS is the native build host. Intel macOS, Ubuntu x64, and Windows x64 packages are cross-assembled and require real-machine launch validation. Ubuntu remote authentication is implemented but also requires a real Linux desktop and SSH target. Windows remote access is intentionally disabled pending a separately validated authentication transport.
