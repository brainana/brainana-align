# Publish Align through GitHub

This repository is the **source**, not an installer. GitHub Actions builds native
installers and attaches them to Releases, using the same source-first model as Viewer.
Do not upload a macOS app ZIP as the repository contents and expect Windows/Linux builds.

## One-time upload

1. Create an empty GitHub repository in the intended account/organization (for example,
   `brainana-align`). Do not initialize a second README or license over these files.
2. Upload the complete extracted **source tree**, including `.github`, lockfile,
   `scientific` vendor source and licenses. GitHub Desktop or git is recommended: the
   immutable original-source reference ZIP is larger than the web uploader's per-file
   limit. Dependencies, caches, generated workers and installers are deliberately absent.
3. Enable GitHub Actions in Settings → Actions. Do not commit credentials or `.env` files.
4. Run **Build and release installers → Run workflow** manually first. This builds all
   four native packages and provides a `complete-release` artifact, but **does not publish**.
5. Inspect each platform's logs and validation. Fix any platform failures before release.

With git, after extracting the source:

```sh
git init -b main
git add .
git commit -m "Add Brainana Align source and release workflows"
git remote add origin https://github.com/YOUR-OWNER/YOUR-REPO.git
git push -u origin main
```

Use your own configured commit identity. With GitHub Desktop, add/create a repository
in the extracted source directory and publish it to the desired organization/account.
No upload or GitHub publication is performed by the local preparation itself.

## Release a version

1. Update the changelog in `Documentation/CHANGELOG.md`.
2. Run `npm run set-version -- 0.17.11` (example). This updates VERSION.json, increments
   the native bundle number and regenerates the workspace/package-lock identity.
3. Commit and push the version and changelog.
4. In GitHub **Releases → Draft a new release**, create tag `v0.17.11` at that commit.
   The tag must exactly match `v` + VERSION.json's version.
5. Publish the release to start the native matrix. Installers are uploaded only after
   **all four native jobs, the browser checks and the aggregation job succeed**. A published download page
   can temporarily have no installers while builds run; it never gets partial platform
   uploads from this workflow.
6. Check attached assets, validation and SHA256SUMS.txt before announcing availability.

**Alternative:** pushing a `v*` tag starts the same pipeline, creates a **draft**
release after successful builds, and attaches the files. Review and publish the draft.
Use either this route or the Release UI route for a version, not both. Publishing an
existing draft may run checks again; uploads replace matching asset names safely.

## What is produced

- Apple Silicon and Intel macOS `.dmg` files.
- Windows x64 NSIS `.exe` and portable `.zip`.
- Linux x64 `.AppImage` and `.deb`.
- Exact-commit complete source ZIP and full release ZIP.
- Per-platform validation, build provenance, manifests and SHA-256 checksums.

The core app and scientific registration worker are compiled for the same OS/CPU.
JIP/ITK/N4 are never silently dropped to make a platform package succeed. No separate
registration app is required. CPU is supported without a GPU SDK; optional acceleration
is detected at runtime and must not be advertised as verified without target tests.

## Credentials and signing

The workflow uses GitHub's built-in GITHUB_TOKEN, scoped to this repository. Only the
final publish job requests `contents:write`; build and PR jobs have read access.
No personal access token is required for same-repository publication. Fork runs target
the fork, not a hard-coded Brainana repository.

macOS packages use ad-hoc signing, matching the existing unsigned-review distribution.
Developer ID/notarization and Windows Authenticode are separate future signing setups;
the present workflows do not claim trusted publisher certification. Never commit
signing keys, passwords or Apple credentials.

## First-run qualification

Workflow presence is not cross-platform test evidence. macOS Apple Silicon can be
checked locally here; Intel, Windows and Linux need their native GitHub jobs (and
native GUI/installer review). If a job fails, the release publisher remains blocked.
Use Actions artifacts/logs to diagnose it; do not disable numerical or package checks
just to publish. Build caches are not part of git or the release source.
