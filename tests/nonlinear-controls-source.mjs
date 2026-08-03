import assert from 'node:assert/strict'
import fs from 'node:fs'

const source=fs.readFileSync(new URL('../source/src/ui/nonlinearLandmarkControls.ts',import.meta.url),'utf8')
assert.match(source,/<option value="custom">Custom…<\/option>/,'nonlinear regularization selector must expose Custom')
assert.match(source,/id="tps-custom-regularization" type="number" min="0" step="any"/,'custom value must use a nonnegative numeric field')
assert.match(source,/resolveNonlinearRegularization\(regularization\.value,customRegularization\.value\)/,'the applied fit must use the resolved custom value')
assert.match(source,/customRow\.classList\.toggle\('hidden',!custom\)/,'the custom field must follow preset selection')
console.log('nonlinear landmark custom-control source checks passed')
