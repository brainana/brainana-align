import assert from 'node:assert/strict'
import { readFileSync } from 'node:fs'

const bootstrap = readFileSync(new URL('../source/src/app/bootstrap.ts', import.meta.url), 'utf8')
const styles = readFileSync(new URL('../source/src/ui/style.css', import.meta.url), 'utf8')
const nonlinear = readFileSync(new URL('../source/src/ui/nonlinearLandmarkControls.ts', import.meta.url), 'utf8')
const navigation = readFileSync(new URL('../source/src/ui/workflowNavigation.ts', import.meta.url), 'utf8')

const stages = ['landmarks', 'linear', 'landmark-warp', 'intensity', 'deformable', 'review']
for (const stage of stages) {
  assert.match(bootstrap, new RegExp(`data-workflow-tab="${stage}"`), `missing ${stage} workflow tab`)
  assert.match(bootstrap, new RegExp(`data-workflow-panel="${stage}"`), `missing ${stage} contextual panel`)
}

assert.match(bootstrap, /class="viewer-footer"/)
assert.match(bootstrap, /id="stack-linear"/)
assert.match(bootstrap, /id="stack-landmark-warp"/)
assert.match(bootstrap, /id="stack-intensity"/)
assert.match(bootstrap, /id="stack-deformable"/)
assert.match(bootstrap, /id="review-mode"/)
assert.match(bootstrap, /id="review-opacity"/)
for (const group of ['Landmark guided','Image driven','Output']) assert.match(bootstrap, new RegExp(group))
assert.match(styles, /grid-template-columns:3fr 2fr 1fr/)
assert.match(styles, /\.landmark-guided/)
assert.match(styles, /\.image-driven/)
assert.match(styles, /\.output-group/)
assert.match(styles, /\.workflow-panel\.active\{display:block\}/)
assert.match(navigation, /ArrowLeft/)
assert.match(navigation, /ArrowRight/)
assert.match(navigation, /Home/)
assert.match(navigation, /End/)
assert.match(navigation, /aria-hidden/)
assert.doesNotMatch(nonlinear, /<details/, 'landmark warp controls must not be hidden in a nested disclosure')

const topbar = bootstrap.match(/<header class="topbar">[\s\S]*?<\/header>/)?.[0] ?? ''
const landmarkPanel = bootstrap.match(/id="panel-landmarks"[\s\S]*?data-workflow-panel="linear"/)?.[0] ?? ''
assert.doesNotMatch(topbar, /id="new-pair"/, 'New landmark must be contextual, not permanent in the top toolbar')
assert.doesNotMatch(topbar, /id="undo"/, 'Landmark Undo must be contextual, not permanent in the top toolbar')
assert.match(landmarkPanel, /id="new-pair"/)
assert.match(landmarkPanel, /id="undo"/)
for (const stage of stages) {
  assert.match(bootstrap, new RegExp(`id="tab-${stage}"[\\s\\S]*?aria-controls="panel-${stage}"`))
  assert.match(bootstrap, new RegExp(`id="panel-${stage}"[\\s\\S]*?aria-labelledby="tab-${stage}"`))
}

const ids = [...bootstrap.matchAll(/id="([^"]+)"/g)].map(match => match[1])
const duplicates = [...new Set(ids.filter((id, index) => ids.indexOf(id) !== index))]
assert.deepEqual(duplicates, [], `duplicate static element IDs: ${duplicates.join(', ')}`)

console.log('Workflow UI source checks passed.')
