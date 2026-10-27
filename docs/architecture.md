# Architecture

Align already uses the same broad organization as Brainana Viewer: an application
under `apps/`, shared `packages/`, scripts, tests and documentation. This publication
export adds GitHub CI/release orchestration without importing Viewer functionality.
Existing full documentation stays in `Documentation/` so application references and
maintenance contracts remain valid; these short `docs/` pages explain contributing.

The frontend and web workers are TypeScript/Vite. Electron supplies the dedicated
window, constrained native IPC and local server. Shared desktop/client/server/launcher
modules stay in their existing workspaces. Scientific registration/N4 is a private
Python/NumPy adapter over JIP and static ITK native libraries, frozen inside each app.
The native build driver chooses platform paths and toolchains; it does not rewrite
registration math, transform/region conventions or session/export semantics.

GitHub's source-check matrix validates ordinary commits and pull requests. The release
matrix builds one complete OS/CPU target per runner. Packaging and worker checks run
before upload. An aggregation job requires the browser checks and all four manifests and verifies every
installer hash before creating source/full-release archives. Only its successful
completion enables publication. Manual runs stop at inspectable candidate artifacts.

The workflow is intentionally stricter than independent per-OS publishing: a failed
numerical worker prevents a misleading partial release. Refer to AGENTS.md before
changing protected coordinate, region, rendering, registration, guide or export code.
