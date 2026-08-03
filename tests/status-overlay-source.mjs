import assert from 'node:assert/strict'
import fs from 'node:fs'
import path from 'node:path'
import { fileURLToPath } from 'node:url'

const root = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '..')
const bootstrap = fs.readFileSync(path.join(root, 'source/src/app/bootstrap.ts'), 'utf8')
const style = fs.readFileSync(path.join(root, 'source/src/ui/style.css'), 'utf8')

assert.match(bootstrap, /id="activity-overlay"[^>]*role="status"[^>]*aria-live="polite"/)
assert.match(bootstrap, /id="status-toast"[^>]*role="alert"/)
assert.doesNotMatch(bootstrap.match(/<header class="topbar">[\s\S]*?<\/header>/)?.[0] ?? '', /id="status"/, 'the status display must not consume toolbar space')
assert.match(bootstrap, /id="status" class="visually-hidden" role="status" aria-live="polite"/)
assert.match(bootstrap, /function beginProcessing\(/)
assert.match(bootstrap, /function endProcessing\(/)
assert.match(bootstrap, /activityText\.textContent = text/)
assert.match(bootstrap, /setAttribute\('aria-busy', 'true'\)/)
assert.match(bootstrap, /removeAttribute\('aria-busy'\)/)
assert.match(bootstrap, /error \? 12000 : 5000/)
assert.match(style, /\.activity-overlay\{position:fixed/)
assert.match(style, /background:rgba\(2,7,12,\.58\)/)
assert.match(style, /overflow-wrap:anywhere/)
assert.match(style, /\.status-toast\.error/)
assert.match(style, /\.visually-hidden\{/)
assert.match(bootstrap, /else showStatusToast\(text, error\)/, 'non-processing completion and guidance messages must use the full notification')

for (const start of [
  'Loading ${displayName}',
  'Fitting ${transformModelLabel(model).toLowerCase()}',
  'Preparing deformable NMI refinement',
  'Preparing constrained mutual-information refinement',
  'Saving transform files',
  'Resampling aligned image',
]) assert.ok(bootstrap.includes(`beginProcessing(\`${start}`) || bootstrap.includes(`beginProcessing('${start}`), `missing processing overlay start: ${start}`)

assert.ok((bootstrap.match(/finally[^\n]*endProcessing\(\)/g) ?? []).length >= 5, 'long-running workflows must clear the overlay in finally blocks')
assert.match(bootstrap, /processingCue = !error && \/\(\?:…\|\\\.\\\.\\\.\)\$\//, 'remote progress ellipses should activate the overlay')

console.log('processing status overlay source checks passed')
