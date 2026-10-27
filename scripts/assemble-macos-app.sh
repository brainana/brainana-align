#!/bin/bash
set -euo pipefail

usage() {
  echo "Usage: $0 --arm64-node PATH --x64-node PATH --out DIRECTORY" >&2
  exit 2
}
ARM64_NODE=""; X64_NODE=""; OUT=""
while [[ $# -gt 0 ]]; do
  case "$1" in
    --arm64-node) ARM64_NODE="$2"; shift 2 ;;
    --x64-node) X64_NODE="$2"; shift 2 ;;
    --out) OUT="$2"; shift 2 ;;
    *) usage ;;
  esac
done
[[ -f "$ARM64_NODE" && -f "$X64_NODE" && -n "$OUT" ]] || usage
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
SOURCE="$ROOT/apps/align"
APP="$OUT/Brainana Align.app"

node "$ROOT/scripts/verify-macos-runtimes.mjs" "$ARM64_NODE" "$X64_NODE"
node "$ROOT/scripts/generate-version.mjs"
(cd "$ROOT" && npm ci && npm run build)
node "$ROOT/scripts/prepare-desktop.mjs" "$ARM64_NODE" "$X64_NODE"
rm -rf "$APP"
mkdir -p "$APP"
cp -a "$ROOT/generated/browser-support/Contents" "$APP/Contents"
cp "$ROOT/generated/Info.plist" "$APP/Contents/Info.plist"
echo "$APP"
