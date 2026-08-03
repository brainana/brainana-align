# Brainana Align changelog

## 0.17.26-workflow-ui.1

- Reorganized the widescreen workflow into three explicit groups: Landmark guided, Image driven, and Output.
- Moved New landmark and Undo out of the permanent top toolbar and into the Landmarks stage that owns them.
- Made the landmark sequence explicit as Points, Linear fit, and optional Nonlinear warp.
- Clarified that Linear intensity can be global or local and that Deformable follows a reasonable global alignment.
- Added tab-to-panel ARIA relationships, active-panel visibility state, and Home/End keyboard navigation.
- Added release tests that prevent contextual landmark actions from returning to the global toolbar and verify the grouped horizontal layout.
- Kept registration, resampling, deformation, session, remote-access, and export calculations unchanged.

## 0.17.25-flirt-inspired.1

- Replaced the field-of-view/sample-count pyramid with explicit input-grid shrink factors. Standard uses `8,4,2,1`, Fast uses `4,2,1`, and Custom accepts a descending sequence ending in `1`.
- Added automatic or per-level custom Gaussian smoothing and optimization-sweep schedules.
- Replaced fixed-step global coordinate descent with an independently written bounded derivative-free optimizer using local bracketing and golden-section line refinement.
- Added coarse orientation multi-start search, center-based translation initialization, retained distinct candidates, translation refinement, rotation perturb-and-retry, and coarse-to-fine candidate reduction.
- Added staged model freedom: proper pose/uniform scale first, then independent scale, then shear when the selected model permits them.
- Kept reflection mathematically unavailable in automatic registration through strictly positive scale parameterization.
- Added regression checks for shrink schedules, invalid schedules, factor-based pyramid geometry, large rotation/translation recovery, staged optimization, and updated UI controls.
- The strategy is an original implementation informed by published multiresolution registration principles; it contains no copied FSL source and is not represented as numerically identical to FLIRT.

## 0.17.24-auto-linear.1

- Added global automatic linear initialization without requiring landmarks.
- Added rigid (6), similarity (7), anisotropic-scale (9), affine (12), and custom models; automatic reflection is never allowed.
- Added independent X/Y/Z searches through full orientation, multiple retained candidates, coarse-to-fine refinement, and boundary warnings.
- Added NMI, MI, correlation ratio, normalized correlation, and least-squares objectives with automatic or explicit histogram bins.
- Replaced human-specific 4/2 mm presets with fixed-image-relative physical resolution levels, including deformable presets.
- Separated global automatic alignment from the existing constrained local rigid refinement.
- Added linear and nearest-neighbor output interpolation choices while retaining trilinear metric interpolation.
- Added automatic-linear, full-orientation, positive-determinant, model-control, and human/macaque/tree-shrew regressions.
- Corrected Electron packaging to obtain its version from `VERSION.json`.

## 0.17.23-ui.1

- Replaced the vertically stacked all-purpose sidebar with six visible workflow stages: Landmarks, Linear, Landmark warp, Intensity, Deformable, and Review.
- Each stage now opens a contextual inspector containing only the controls and diagnostics relevant to that operation.
- Promoted nonlinear landmark warping from a nested disclosure beneath linear fitting to a first-class workflow stage.
- Separated rigid intensity refinement and deformable refinement into independent panels with independent candidate summaries and accept/reject actions.
- Moved overlay display mode and opacity into a persistent viewer footer rather than treating them as a registration stage.
- Added a persistent accepted-transform stack showing linear, landmark-warp, intensity, and deformable state.
- Added keyboard-accessible left/right workflow navigation and regression checks for all stage/panel mappings, unique IDs, and the absence of nested landmark-warp controls.
- Registration algorithms, coordinate handling, remote access, sessions, and export calculations are unchanged.

## 0.17.20-status-ui.1

- Removed the narrow top-right toolbar status text and reclaimed that space for application controls.
- Removed the misleading initial `Load MRI and CT to begin…` toolbar fragment; the empty MRI and CT panels already communicate their state directly.
- All non-processing guidance, completion, cancellation, and error messages now use the full-width dismissible notification.
- Active work continues to use the centered processing overlay with a dimmed workspace.
- Retained a visually hidden `aria-live` status region for assistive technology without consuming layout space.

## 0.17.19-deformable.1

- Fixed repeated deformable refinement: a new pass now initializes its control grid from the accepted deformation instead of silently restarting from identity and then failing the NMI comparison.
- A pending deformable proposal must now be accepted or rejected before another pass; the button and full status message make that state explicit.
- Accepting or rejecting re-enables refinement and explains whether the next pass starts from the accepted or restored deformation.
- Replaced the generic clipped custom-settings message with a complete field-by-field list of invalid values.
- Added hover and keyboard-focus explanations for the deformable preset, grid levels, regularization, maximum displacement, iterations, and minimum/maximum Jacobian limits.
- Parameter explanations describe units, expected effects, runtime implications, and distortion/overfitting tradeoffs.

## 0.17.18-status.1

- Replaced clipped toolbar-only progress reporting with a centered processing overlay over a dimmed workspace.
- The overlay presents wrapped, live progress text and a spinner during image loading, landmark fitting, automatic refinement, deformable refinement, resampling, compression, saving, and remote operations that report active progress.
- Completion and failure messages move into a full-width dismissible notification rather than disappearing into the narrow top-right toolbar area.
- Added bounded automatic dismissal, longer visibility for errors, Escape-independent processing state, and `aria-live`/`aria-busy` accessibility state.
- Every explicitly managed processing path clears the overlay in `finally`, including cancellation and exceptions.

## 0.17.17-correspondence.1

- Replaced the reflection-first handedness prompt with a correspondence-repair dialog when one crossed landmark pair is uniquely supported.
- The dialog identifies the actual landmark IDs and offers separate **Swap CT points** and **Swap MRI points** actions because geometry alone cannot determine which modality was assigned incorrectly.
- Added **Review landmarks manually** and **Enable reflection instead** alternatives.
- Suggested swaps are recorded in undo history, clear stale residuals, and never run or apply a transform automatically; the user must review the edit and explicitly fit again with reflection still disabled.
- Reflection remains off by default and is visually separated as the higher-risk fallback.

## 0.17.16-handedness.1

- Added a diagnostic reflected-affine comparison only after the requested proper affine fails plausibility QC; reflected fitting remains disabled during the requested fit.
- Requires at least six landmarks, a plausible reflected candidate, and at least 80% weighted orientation reversal across independent landmark tetrahedra before showing a reflection recommendation.
- Reports reflected-candidate RMS, scales, shear, and the weighted handedness-reversal fraction in a clear popup and inline result.
- For sets of at most 20 landmarks, evaluates individual correspondence swaps and names a suspected pair only when one stable swap is uniquely and substantially better.
- The popup recommends reviewing left/right correspondence and coordinate headers first. It can enable the reflection option for a subsequent fit but never applies or runs that fit automatically.
- The subsequent reflected fit still requires the existing explicit confirmation before application and export.

## 0.17.15-affine-qc.1

- Added an affine plausibility gate that rejects extreme axis collapse, expansion, shear, or near-singularity instead of displaying a scientifically unusable overlay.
- Failure details report the fitted scales, shear, and determinant and direct the user to inspect landmark correspondence, especially left/right pairing.
- Added the reported six-landmark coordinate set as a regression fixture.
- Confirmed that the reported coordinates encode opposite handedness because CT landmarks 5 and 6 are crossed relative to MRI.
- Confirmed that swapping only those two CT assignments changes the fit from an implausible collapsed solution to a stable affine with RMS below 3 mm, positive determinant, and low shear.
- Reflection remains disabled by default and the proper affine solver remains orientation preserving.

## 0.17.14-affine.1

- Corrected the reflection-disabled affine model fundamentally: it now optimizes only over orientation-preserving affine transforms instead of computing an unrestricted reflected solution and rejecting it.
- Parameterized proper affine transforms as a proper rotation multiplied by an upper-triangular scale/shear matrix with strictly positive scale entries.
- Reflection-enabled affine fitting remains a separate, explicit unrestricted least-squares path and remains off by default.
- Added broad positive scale bounds to keep the proper optimizer away from singular solutions while retaining diagnostic warnings for implausible scale.
- Added regression fixtures proving a reflected dataset receives a non-reflected best-effort solution when reflection is disabled, and the exact reflected solution only when reflection is explicitly enabled.
- Added exact recovery of a strongly oblique 137-degree proper affine transform containing anisotropic scale and shear.

## 0.17.13-landmark-errors.1

- Replaced the truncated toolbar-only landmark-fit failure with a complete explanation inside the Landmark alignment panel.
- Added an explicit safety dialog when an affine candidate is rejected because it reflects anatomy.
- Scrolls and highlights the opt-in reflection control after this rejection while leaving it disabled.
- Keeps the full toolbar status available as a tooltip and uses a concise visible status message.
- General landmark-fit errors now identify the selected model and remain visible beside its controls.

## 0.17.12-landmarks.1

- Kept rigid six-parameter landmark alignment as the default and added a constrained nine-parameter model with independent X, Y, and Z scales and no shear.
- Retained seven-parameter similarity, full twelve-parameter affine, and custom selectable linear models in the same Landmark transform menu.
- Replaced the rigid quaternion eigenvector solver with a symmetric eigensolver that is stable for large, oblique, and near-180-degree rotations.
- Added determinant, axis-scale, shear, condition, reflection, and plausibility diagnostics to the landmark result panel and registration metadata.
- Made reflected affine fitting explicitly opt-in and off by default. Reflected results require confirmation before application, session restoration, aligned-image export, or transform export.
- Added physical-coordinate fixtures using distinct oblique, anisotropic MRI and CT NIfTI geometries in both transform directions.
- Updated sessions to schema 5 while retaining migration of rigid-only and earlier sessions.
- Added exact rigid, similarity, anisotropic-scale, affine, inverse, reflection, degeneracy, session, and export regressions.

## 0.17.11-deformable.1

- Added automatic image-driven deformable refinement as a separate option from the existing rigid NMI refinement.
- Added a coarse-to-fine target-grid inverse displacement field optimized with normalized mutual information.
- Added Conservative, Balanced, Flexible, and validated Custom settings for grid levels, regularization, maximum displacement, iterations, and Jacobian bounds.
- Existing MRI and CT optimization windows constrain deformable sampling; every undefined plane remains unrestricted.
- Added cancellation, progress reporting, review-before-acceptance, rejection, and independent deformable reset.
- Added first-derivative smoothness regularization, maximum-displacement limits, sampled Jacobian bounds, fold rejection, and rejection of proposals that worsen NMI.
- Composed the accepted deformation after the linear and optional nonlinear-landmark transforms for crosshairs, landmarks, overlays, aligned-volume export, sessions, and registration metadata.
- Added deterministic control-grid, interpolation, inverse-consistency, Jacobian, regularization, optimization, and cancellation fixtures.

## 0.17.10-nonlinear.2

- Added a Custom nonlinear regularization option alongside the five existing presets.
- Added a nonnegative numeric field that accepts zero for exact interpolation and any finite value above zero for user-selected smoothing.
- Preset values remain unchanged and Balanced remains the default.
- Added validation for empty, negative, and non-finite custom values plus regression checks proving the entered value reaches the nonlinear fitter unchanged.

## 0.17.9-nonlinear.1

- Added an optional regularized three-dimensional landmark warp after the accepted linear landmark fit, using an independently implemented order-2 polyharmonic radial basis (`U(r) = r`) with an affine polynomial tail.
- Added Exact, Very flexible, Balanced, Smooth, and Very smooth regularization presets; Balanced is the default.
- Added a separate Reset nonlinear warp action that retains the linear alignment and all landmarks.
- Applied the nonlinear transform consistently to linked navigation, review overlays, residual reporting, aligned-volume resampling, sessions, and exports.
- Added forward and independently fitted reverse maps for practical inverse resampling.
- Added sampled Jacobian determinant, folding, displacement, and forward/reverse consistency checks. A warp with a sampled nonpositive Jacobian is rejected.
- Automatic image-intensity refinement remains rigid and is disabled while a nonlinear landmark warp is active. Image-driven deformable refinement is intentionally reserved for a separately validated milestone.

## 0.17.8-linear.1

- Added Reset alignment, which removes the fitted transform and review overlay while retaining landmarks and optimization windows.
- Kept rigid six-parameter landmark registration as the default.
- Added seven-parameter similarity registration with one uniform scale.
- Added full twelve-parameter affine landmark registration.
- Added an advanced custom model for independently enabling translation, rotation, axis scale, and shear parameters; unchecked parameters remain fixed at identity.
- Added general affine inversion throughout image resampling, manual adjustments, automatic rigid refinement around the accepted base transform, sessions, and exports.
- Added determinant and rank validation to reject reflected, singular, or underconstrained affine solutions.
- Updated sessions to schema 2 while migrating older saved fits as rigid transformations.
- Added exact synthetic regression fixtures for similarity, affine, affine inverse round trips, and shear-only custom fitting.
- Nonlinear and elastic deformation are not included in this release.

## 0.17.7-structure.1

- Reorganized the readable renderer into explicit `app`, `domain`, `viewer`, `infrastructure`, and `ui` layers without changing application behavior.
- Moved registration, NIfTI/ROI, optimization, session, and export logic under the domain layer.
- Moved coordinate projection, crosshairs, landmarks, viewport behavior, and optimization-window interaction under the viewer layer.
- Moved runtime, Electron bridge, remote endpoint, transfer, and filesystem clients under infrastructure.
- Added architecture checks that reject a return to the previous flat source directory.
- Standardized development and release commands on pnpm 11.13.0 and removed the duplicate npm lockfile.
- Renamed the optimization-window test so it cannot be mistaken for Windows platform validation.
- Replaced the TSX command wrapper with Node's import hook, avoiding an unnecessary local IPC socket during tests.
- Scientific calculations, UI behavior, remote access, import/export, and packaged platform behavior are unchanged.

## 0.17.4-export-browser.3

- Corrected a global `main` layout collision that forced the export browser content to full application height and pushed its footer outside the dialog.
- Replaced the nested semantic `main` element with the same neutral panel structure used by the remote import browser.
- Added the missing top-right Close button using the same modal header pattern as import.
- Cancel and Select this folder now occupy the visible fixed footer.

## 0.17.4-export-browser.2

- Fixed the remote export browser footer being pushed below the visible dialog.
- The Cancel and Select this folder buttons now remain visible at all supported window heights, including empty directories.
- Constrained the folder list and sidebar to scroll internally without displacing the action footer.

## 0.17.4-export-browser.1

- Rebuilt the remote export-folder chooser with the same Finder-style navigation model as remote import.
- Added Back, Forward, Up, Home, breadcrumbs, Command-L path entry, folder search, sortable Name/Date Modified columns, hidden-folder control, starting/root/recent shortcuts, and remembered export locations.
- Retained new-folder creation and current-folder selection.
- Kept remote export atomic-write behavior, SSH permissions, SFTP transport, and scientific calculations unchanged.

## 0.17.3-remote-export.1

- Added Local/Workstation destination selection to Export whenever an Electron remote connection is active.
- Made remote connection state reactive so connecting or disconnecting immediately updates the open export interface.
- Remote folder selection begins at the profile's starting directory and permits navigation to any SSH-authorized location.
- Remote exports use the existing structured SFTP transport, overwrite confirmation, temporary upload, verification, backup, replacement, and rollback flow.
- Local export and scientific behavior are unchanged.

## 0.17.2-remote-browser.1

- Replaced the basic remote file list with a Finder-style SFTP browser.
- Added a Locations sidebar for the profile starting directory, remote filesystem root, and last visited folder.
- Added Back, Forward, Up, and Home navigation, clickable path breadcrumbs, and direct path entry with Command-L.
- Added current-folder search, sortable Name/Size/Date Modified columns, readable file sizes and dates, hidden-file control, keyboard navigation, multi-selection, and remembered locations.
- Added modification timestamps to local and SFTP directory listings.
- Kept SSH authentication, SFTP transport, unrestricted SSH-permission-based navigation, image loading, and scientific calculations unchanged.

## 0.17.1-remote.3

- Reclassified each remote profile path as a starting directory rather than a filesystem containment boundary.
- Remote browsing can now navigate upward to `/` and then into any directory the authenticated SSH account is permitted to access.
- Preserved normalized path handling, loopback session authentication, structured SFTP operations, and the existing SSH account permission model.
- Kept browser-mode and local filesystem containment unchanged.

## 0.17.1-remote.2

- Fixed the Electron remote controls not appearing by replacing the ES-module preload with a sandbox-compatible CommonJS preload.
- Added package checks requiring the sandbox preload and its restricted remote IPC surface.

## 0.17.1-remote.1

- Added on-demand remote MRI and CT loading to the Electron desktop application.
- Each MRI/CT load control now offers local or remote workstation sources without a startup mode prompt.
- Added versioned, atomically written JSON connection profiles containing only non-secret metadata.
- Added Terminal-based system SSH authentication compatible with passwords, keys, passphrases, SSH configuration, and institutional interactive authentication.
- Added reusable connection status, disconnect behavior, structured SFTP browsing, progress, and cancellation.
- Local and remote volumes can be mixed in one registration session without restarting or reloading the application.
- Isolated the remote filesystem behind its own localhost port, session token, origin policy, and Electron preload bridge.

## 0.17.0-electron.4

- Corrected the recurring axial optimization-window failure by making the rendered orthographic plane—not NiiVue's sometimes-degenerate axial pointer result—the primary screen-to-image mapping.
- Added a deterministic regression test that emulates two visibly separated axial drag endpoints being reported by NiiVue as the same coordinate.
- A rejected drag on a plane without a window is now a valid unrestricted state rather than a red application error.
- Definition and completion messages now explicitly state that every plane without a window uses all voxels.

## 0.17.0-electron.3

- Replaced optimization-window pointer handling with one shared view-card capture path for all MRI and CT planes.
- Added a projection fallback for letterboxed image regions where NiiVue does not return a direct pointer coordinate.
- Invalid or very small replacement drags now preserve the previous valid window instead of deleting it.
- Reasserted and regression-tested the constraint rule: every plane without a usable window is completely unrestricted, equivalent to selecting the full plane.

## 0.17.0-electron.2

- Fixed packaged runtime discovery so the desktop app resolves `Resources/runtime/server.mjs` from the actual bundle layout instead of relying on Electron's executable-name-sensitive `app.isPackaged` heuristic.
- Added a regression check for packaged runtime discovery.

## 0.17.0-electron.1

- Added an Apple Silicon Electron desktop target using the complete readable TypeScript/NiiVue frontend.
- Added an internal secure local-server lifecycle with no external browser launch.
- Added single-instance handling, bounded startup, authenticated handshakes, shutdown cleanup, navigation restrictions, and hardened renderer settings.
- Preserved the browser target and its existing macOS launcher.
- Electron remote-workstation mode is intentionally deferred; the first desktop prototype supports Local mode.

# 0.17.22-multires.1

- Added Fast (4 mm), Balanced (4→2 mm), Precise (4→2→native), and validated Custom intensity-registration resolution controls.
- Added Gaussian-prefiltered, physical-extent-preserving MRI/CT image pyramids for rigid and deformable NMI optimization.
- Kept landmark fitting, transforms, displacement fields, review resampling, and exported aligned images in physical/native-resolution space.
- Recorded the optimization resolution levels in session and registration state metadata.
- Separated opacity-only rendering from colormap changes and coalesced slider input to one GPU refresh per animation frame.
- Added regression tests for pyramid dimensions, world origin and endpoint preservation, presets, custom validation, native-resolution metadata, and the opacity fast path.

# 0.17.21-feedback.1

- Guaranteed that the blocking processing overlay is painted before synchronous landmark fitting and image rebuilding begin.
- Added processing feedback while accepting or rejecting rigid and deformable proposals, applying or resetting nonlinear landmark deformation, resetting deformable refinement, and saving or restoring sessions.
- Suppressed NiiVue's built-in startup `loading ...` label and left unloaded MRI/CT panels visually blank.
- Replaced focus-sticky CSS parameter explanations with one controlled viewport tooltip that closes on pointer exit, focus exit, scrolling, resizing, outside interaction, or Escape.
- Added source regression checks covering every new feedback path, blank startup state, and tooltip dismissal behavior.

## 0.16.26-docs.1

- Replaced per-release architecture, changelog, and validation Markdown files with one maintained document of each type.
- Removed the duplicated `Documentation-release/` tree.
- Consolidated browser-support policy into the current architecture and validation documents.
- Added an automated documentation-layout test that fails on version-stamped release documents, duplicate documentation trees, or missing maintained documents.
- Updated package manifests and release verification requirements for the consolidated documentation model.
- No application, scientific, SSH, local-mode, remote-mode, or export behavior changed.

## 0.16.25-local-start.1

- Local mode now opens directly without a startup MRI/CT folder chooser.

# Brainana Align 0.16.24-ssh.1

## SSH reliability correction

- Removed the explicit OpenSSH `-f` option from the interactive persistent-master command.
- The Terminal helper now uses the command form verified successfully on the affected macOS/OpenSSH 10.2 system: `ssh -M -S <socket> -o ControlPersist=600 -o ExitOnForwardFailure=yes -NT <target>`.
- OpenSSH may background the control master through `ControlPersist` only after authentication succeeds.
- The launcher continues to require a successful `ssh -O check` before starting the application server.
- Failed and timed-out SSH helper directories are retained and their paths are reported for diagnosis.
- Added a regression test that rejects any generated helper containing `-fNT`.
- Browser-matrix functionality from 0.16.23-browser-matrix.1 is retained unchanged.

# Brainana Align 0.16.21-lifecycle.6

## Fixed
- Replaced the unreliable AppleScript `do script` SSH helper launch with an executable `.command` helper opened by macOS LaunchServices in Terminal.
- Added a positive startup acknowledgement so the launcher distinguishes “Terminal accepted the open request” from “the helper actually ran.”
- Added explicit user-facing errors when Terminal does not launch or execute the helper.
- Corrected the successful-SSH race by waiting for the persistent control socket to become ready after SSH returns success.
- Preserved clean cancellation behavior for Control-C, Terminal-tab closure, HUP, INT, and TERM.

## Packaging
- Restored the complete application documentation set, release manifest, build metadata, and checksums to the macOS package.
- Added package-content validation so required release files cannot be silently omitted.

## 0.16.22-lifecycle.7
- Fixed nounset-safe generation of the Terminal SSH authentication helper.
# 0.17.5-remote-ui.1

- Corrected the remote import browser layout so **Cancel**, **Load selected**, and the top-right close control remain visible.
- Removed the same nested `main` layout collision previously found in the export browser.
- Successful SSH authentication now closes only its matching temporary Terminal tab by recorded TTY; unsuccessful authentication stays open for diagnosis.
# 0.17.5-remote-ui.2

- Added the missing minimum-height and overflow constraints that keep the remote import footer inside the dialog even for long directory listings.
- Replaced tab-object Terminal cleanup with exact authentication-window cleanup after the command has completed.
- Added a retained `terminal-close.log` for precise diagnosis if macOS refuses the close request.
- Added an AppleScript compilation gate; it caught and corrected the invalid nested Terminal property expression shipped in the previous attempt.
# 0.17.6-platforms.1

- Added Intel macOS, Ubuntu/Linux x64, and Windows x64 Electron packaging from the shared source tree.
- Added Linux terminal discovery for interactive SSH password, key-passphrase, and keyboard-interactive authentication.
- Removed hard-coded macOS SSH executable paths from shared connection lifecycle operations.
- Windows local loading, registration, sessions, and export are enabled; unsupported remote workstation controls are explicitly disabled.
