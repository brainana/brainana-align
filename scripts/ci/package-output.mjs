import path from 'node:path'

// electron-builder uses mac/ for its default x64 architecture and mac-arm64/
// for ARM. The verifier still checks the executable's actual lipo architecture.
export function macOutput(candidates, arch) {
  return candidates.find(p => path.basename(p) === 'mac-' + arch)
    ?? (arch === 'x64' ? candidates.find(p => path.basename(p) === 'mac') : undefined)
}
