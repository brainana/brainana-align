# Brainana Align 0.18.4

**Landmark alignment:** choose any landmark transform model and enable **Allow left–right reflection** when appropriate. The app compares ordinary and mirrored fits using that same model; it does not add scale or shear to a rigid fit. The default is unchanged. Four or more spatially distributed pairs are needed to distinguish handedness from a rigid rotation; planar/tied cases retain the non-reflected fit and show a warning. The checkbox also controls affine fitting. A reflected affine optimum is rejected while it is off; when on, confirmation is still required.

**Manual adjustment:** select **Adjust MRI** or **Adjust CT**, then **Flip … left–right**. Confirm the change; click the same control again to undo it. The flip is separate from scale and from export direction, and original image values remain unchanged. It works with linear alignments. Pending proposals must first be accepted or rejected. Nonlinear maps cannot be flipped here; their disable reason explains that a linear alignment is required.

The status/fit diagnostics identify mirrored results. Application, restore and export retain explicit reflection confirmation. Sessions save the selected fitting option and manual flags; exported images and transform metadata record reflection. These are true anatomical reflections, distinct from switching the displayed fixed/moving image.

Save by filename and folder · 0.18.3

Open saved session from the home screen or toolbar now reopens the original images and restores the saved working resolution, processing and alignment. If a file has moved, select its current location when prompted. New sessions save original local file paths; older sessions also look beside the JSON file. Original image files are not embedded in the session. Browser runtimes ask for source-file selection.

Native export update · 0.18.1

Native NIfTI export now reads source voxels in encoded chunks or cached pages and resamples tiled output, removing the former 1.5 GiB native input-array cutoff. Native geometry, interpolation and processing rules are preserved. The working-image allocation limit remains separate. See Documentation/CHANGELOG.md and Validation/REPORT.json in the release bundle.

## Saving images and sessions

Click **Save aligned image…**, **Save selected generated image…**, **Save transforms and landmarks…**, or **Save reloadable session…**. On desktop, choose the filename and folder together in the native Save dialog. The last-used folder is a starting suggestion; every save still asks. An image’s JSON metadata is saved beside it with the same stem. Renamed transform groups retain their directional suffixes. Existing accompanying files require confirmation. Cancelling stops the remaining files and never reports success.

When saving to a workstation, the Save dialog contains the remote folder browser and filename field together. In a web browser, use its file picker where supported; otherwise enter the filename and the browser’s download settings determine the folder.

# Brainana Align 0.17.10 · image flipping

In **Alignment display → Image comparison**, choose **Flip between images**. Use **Show MRI/CT** (or the selected image pairing) to switch manually, or **Auto flip · 1 s** to alternate each second. The label identifies the image currently shown and whether it is fixed or moving. Each view keeps its current slice/zoom and the same aligned grid. The other image is hidden, including through empty moving-image background. Display colors remain controlled by Display mode; choose Grayscale opacity for grayscale inspection.

Choose **Smooth opacity** to return to the existing slider and 25–75% animation at the retained blend setting. Flip state/playback is launch-local and does not overwrite the saved opacity or alter image data, transforms, landmarks, regions, registration or exports. Playback stops on manual flip, slider input, mode change, tab hiding, overlay clearing and workspace reset. Native graphical validation remains pending.

# Brainana Align 0.17.9

Viewer fix: trackpad/wheel zoom now follows scroll distance within safe bounds. Crosshairs retain a thin, consistent screen width even at high zoom. Use **Reset view** in the toolbar to restore every pane's zoom and field of view. Wheel zooms; Shift + wheel changes slices. Crosshair color, visibility and thickness remain in Settings.

The change is limited to viewer input and drawing. Existing registration options, linear software-default profiles, image processing, sessions and exports remain available. This Apple Silicon review release includes the self-contained app, complete source, documentation, validation, manifest and SHA-256 checksums. Native graphical verification remains pending in the agent environment.

# Brainana Align 0.17.8 · Apple Silicon

Open Image-based registration → Linear registration → Defaults. Select AFNI, FSL, ANTs, JIP or Elastix and click Apply defaults. Keep my selected transform model is optional; every numerical parameter stays editable. Selecting a profile previews it; applying defaults never runs registration. Click Run linear registration separately, inspect the result and explicitly accept/reject.

Profiles map documented settings into Align’s shared engine. Each includes source references and differences; it does not run or reproduce another program’s optimizer. Modified settings and effective profile provenance are recorded with registration sessions/exports. JIP and ITK nonlinear methods still follow accepted linear alignment.

Complete source, documentation, validation, manifests and checksums accompany the self-contained app. Compact scientific runtime unchanged. macOS14+, Apple Silicon review candidate. Native graphical/GPU tests and Developer ID notarization remain pending.
