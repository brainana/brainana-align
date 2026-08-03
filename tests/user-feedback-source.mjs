import assert from 'node:assert/strict'
import fs from 'node:fs'
import path from 'node:path'
import { fileURLToPath } from 'node:url'

const root = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '..')
const bootstrap = fs.readFileSync(path.join(root, 'source/src/app/bootstrap.ts'), 'utf8')
const nonlinear = fs.readFileSync(path.join(root, 'source/src/ui/nonlinearLandmarkControls.ts'), 'utf8')
const style = fs.readFileSync(path.join(root, 'source/src/ui/style.css'), 'utf8')

assert.match(bootstrap, /async function yieldForPaint\(\): Promise<void>/)
assert.match(bootstrap, /requestAnimationFrame\(\(\) => requestAnimationFrame/)
assert.match(bootstrap, /Fitting \$\{transformModelLabel\(model\).*?[\s\S]{0,180}await yieldForPaint\(\)/)
assert.match(bootstrap, /Applying deformable refinement…/)
assert.match(bootstrap, /Applying automatic refinement…/)
assert.match(bootstrap, /Restoring the previous deformation…/)
assert.match(bootstrap, /Restoring alignment without deformable refinement…/)
assert.match(bootstrap, /Preparing session file…/)
assert.match(bootstrap, /Restoring session…/)
assert.match(nonlinear, /Applying nonlinear landmark deformation…/)
assert.match(nonlinear, /Restoring the linear landmark alignment…/)
assert.match(nonlinear, /beginProcessing: \(message: string\) => void/)
assert.ok((bootstrap.match(/loadingText: ''/g) ?? []).length === 2, 'every NiiVue construction must suppress its startup loading label')
assert.doesNotMatch(bootstrap, /No \$\{modality\.toUpperCase\(\)\} selected/)
assert.match(bootstrap, /id="parameter-tooltip" class="parameter-tooltip hidden" role="tooltip"/)
assert.match(bootstrap, /target\.addEventListener\('pointerleave', hide\)/)
assert.match(bootstrap, /target\.addEventListener\('focusout', hide\)/)
assert.match(bootstrap, /window\.addEventListener\('scroll', hide, true\)/)
assert.doesNotMatch(style, /\.parameter-help:hover:after/)
assert.doesNotMatch(style, /content:attr\(data-help\)/)
assert.match(style, /\.parameter-tooltip\{position:fixed/)

console.log('comprehensive processing feedback and tooltip checks passed')
