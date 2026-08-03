import assert from 'node:assert/strict'
import fs from 'node:fs'
import path from 'node:path'
import { fileURLToPath } from 'node:url'

const root = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '..')
const bootstrap = fs.readFileSync(path.join(root, 'source', 'src', 'app', 'bootstrap.ts'), 'utf8')
const controls = fs.readFileSync(path.join(root, 'source', 'src', 'ui', 'linearRegistrationControls.ts'), 'utf8')
const interfaceSource = `${bootstrap}\n${controls}`
const clearFit = bootstrap.match(/function clearFit\(\) \{([\s\S]*?)\n\}/)?.[1] ?? ''
const resetHandler = bootstrap.match(/resetAlignment\.addEventListener\('click',\(\)=>\{([\s\S]*?)\n\}\)/)?.[1] ?? ''

assert.match(interfaceSource, /id="transform-model"/)
assert.match(interfaceSource, /Rigid · 6 parameters/)
assert.match(interfaceSource, /Similarity · 7 parameters/)
assert.match(interfaceSource, /Anisotropic scale · 9 parameters/)
assert.match(interfaceSource, /Affine · 12 parameters/)
assert.match(interfaceSource, /id="allow-reflection" type="checkbox"/)
assert.match(interfaceSource, /Allow reflected affine fit/)
assert.match(interfaceSource, /if\(result\.diagnostics\.reflected&&!window\.confirm/)
assert.match(interfaceSource, /fitResult\.linearDiagnostics\.reflected&&!window\.confirm/)
assert.match(fs.readFileSync(path.join(root, 'source', 'src', 'domain', 'registration', 'linear.ts'), 'utf8'), /if \(allowReflection\) return fitUnrestrictedAffine/)
assert.match(fs.readFileSync(path.join(root, 'source', 'src', 'domain', 'registration', 'linear.ts'), 'utf8'), /const constrained = fitCustomLinear/)
assert.match(bootstrap, /session contains a reflected landmark transform/)
assert.match(bootstrap, /Affine fit was not applied/)
assert.match(bootstrap, /reflection safeguard\. See Landmark alignment/)
assert.match(bootstrap, /window\.alert\('Affine fit not applied/)
assert.match(bootstrap, /reflection-option'\)\?\.scrollIntoView/)
assert.match(bootstrap, /statusEl\.title = text/, 'truncated toolbar status must expose its full text as a tooltip')
assert.match(bootstrap, /diagnoseReflectionAlternative\(source,target\)/)
assert.match(bootstrap, /Landmark handedness conflict/)
assert.match(bootstrap, /id="handedness-swap-ct"/)
assert.match(bootstrap, /id="handedness-swap-mri"/)
assert.match(bootstrap, /Review landmarks manually/)
assert.match(bootstrap, /Enable reflection instead/)
assert.match(bootstrap, /\[first\[modality\], second\[modality\]\] = \[second\[modality\], first\[modality\]\]/)
assert.match(bootstrap, /Click Fit affine transform to test the corrected non-reflected fit/)
assert.match(bootstrap, /allowReflection\.checked\s*=\s*true/)
assert.match(bootstrap, /const first=complete\[pair\[0\]-1\],second=complete\[pair\[1\]-1\]/, 'diagnostic correspondence indices must map through the complete enabled landmark list')
assert.match(bootstrap, /const modality: Modality = action === 'swap-ct' \? 'ct' : 'mri'/)
assert.match(bootstrap, /markLandmarksChanged\(\)/, 'a repaired correspondence must invalidate stale residuals')
assert.match(bootstrap, /const affine = image\.getAffine\(\)/, 'the active NIfTI physical affine must be retained')
assert.match(bootstrap, /lm\[modality\]=\[\.\.\.currentMm\[modality\]!\]/, 'landmarks must be stored in physical millimeter coordinates')
for (const parameter of ['tx','ty','tz','rx','ry','rz','sx','sy','sz','shxy','shxz','shyz']) {
  assert.match(interfaceSource, new RegExp(`data-linear-param="${parameter}"`))
}
assert.match(bootstrap, /id="reset-alignment"/)
assert.match(resetHandler, /clearFit\(\)/)
assert.doesNotMatch(clearFit, /landmarks\s*=/, 'reset alignment must retain landmark coordinates')
assert.doesNotMatch(clearFit, /optimizationWindows\s*=/, 'reset alignment must retain optimization windows')
assert.match(bootstrap, /invertAffineMatrix\(matrix\)/, 'resampling must use a general affine inverse')
assert.doesNotMatch(bootstrap, /invertRigid\(/, 'application workflows must not use a rigid-only inverse')
console.log('linear registration UI and reset behavior source checks passed')
