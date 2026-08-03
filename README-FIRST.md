# Brainana Align 0.17.26-workflow-ui.1

This release provides native Electron packages for Apple Silicon macOS, Intel macOS, Ubuntu/Linux x64, and Windows x64. The shared imaging, registration, session, and export workflows are built from the same source. Remote workstation access is supported on macOS and Ubuntu; it is explicitly unavailable in the initial Windows package because Windows OpenSSH does not provide the validated multiplexed authentication transport used by this application.

This milestone reorganizes the widescreen interface around the actual alignment workflow. Landmark-guided stages, image-driven stages, and output review are now visibly grouped; New landmark and Undo are contextual Landmarks actions; and tab subtitles clarify prerequisites without hiding advanced workflows. Registration and scientific calculations are unchanged.

The release contains self-contained desktop packages, complete shared source, consolidated documentation, version metadata, manifests, and checksums. Start with `Documentation/README.md`; current validation status is in `Documentation/VALIDATION.md`.
