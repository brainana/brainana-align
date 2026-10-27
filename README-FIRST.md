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
