import assert from 'node:assert/strict'
import fs from 'node:fs'
import path from 'node:path'
import { fileURLToPath } from 'node:url'
const root=path.resolve(path.dirname(fileURLToPath(import.meta.url)),'..')
const source=fs.readFileSync(path.join(root,'source/src/app/bootstrap.ts'),'utf8')
assert.match(source,/function scheduleReviewOpacity\(\): void/)
assert.match(source,/if \(opacityFrame === null\) opacityFrame = requestAnimationFrame\(applyReviewOpacityNow\)/)
assert.match(source,/reviewOpacity\.addEventListener\('input', scheduleReviewOpacity\)/)
const fastPath=source.match(/function applyReviewOpacityNow\(\)[\s\S]*?\n}\n\nfunction scheduleReviewOpacity/)?.[0]??''
assert.doesNotMatch(fastPath,/setColormap/,'slider updates must not rebuild colormaps')
assert.match(source,/Registration pyramid<select id="registration-resolution"/)
assert.match(source,/fitResult\.optimizationResolutionLevels=\[\.\.\.resolutions\]/)
console.log('opacity scheduling and registration-resolution UI checks passed')
