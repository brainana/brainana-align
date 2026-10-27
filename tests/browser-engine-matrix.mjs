import {startBrowserHarness} from './browser-harness.mjs'
import assert from 'node:assert/strict'
import fs from 'node:fs'
import path from 'node:path'
import { createRequire } from 'node:module'
import { fileURLToPath } from 'node:url'

const require = createRequire(new URL('../apps/align/package.json', import.meta.url))
const { chromium, firefox, webkit } = require('playwright-core')
const root = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '..')
const dist = path.join(root, 'apps/align', 'dist')
const required = new Set((process.env.BRAINANA_REQUIRE_BROWSER_ENGINES || '').split(',').map(s => s.trim()).filter(Boolean))
const engines = [
  ['chromium', chromium, process.env.CHROMIUM_PATH || chromium.executablePath()],
  ['firefox', firefox, process.env.FIREFOX_PATH || firefox.executablePath()],
  ['webkit', webkit, process.env.WEBKIT_PATH || webkit.executablePath()],
]
const harness=await startBrowserHarness()
const results = []
try {
for (const [name, engine, executablePath] of engines) {
  if (!executablePath || !fs.existsSync(executablePath)) {
    results.push({ engine: name, status: 'not-installed', executablePath })
    if (required.has(name)) throw new Error(`${name} is required but its browser executable is unavailable: ${executablePath}`)
    continue
  }
  let browser
  try {
    browser = await engine.launch({ executablePath, headless: true, args: name === 'chromium' ? ['--no-sandbox', '--disable-dev-shm-usage'] : [] })
    const page = await browser.newPage({ viewport: { width: 1600, height: 1000 } })
    const errors = []
    page.on('pageerror', error => errors.push(error.message))
    await page.goto(harness.url+'/#session='+'b'.repeat(64))
    await page.waitForSelector('#welcome-screen')
    await page.locator('[data-registration-pairing="mri-ct"]').click()
    await page.waitForSelector('#mri-sagittal-placeholder')
    for (const plane of ['sagittal','coronal','axial']) {
      assert.equal((await page.locator(`#mri-${plane}-placeholder`).textContent())?.trim(), 'No MRI selected')
      assert.equal((await page.locator(`#ct-${plane}-placeholder`).textContent())?.trim(), 'No CT selected')
      assert.equal(await page.locator(`#mri-${plane}-window-layer`).count(), 1)
      assert.equal(await page.locator(`#ct-${plane}-window-layer`).count(), 1)
    }
    assert.equal(await page.locator('#landmark-transform').inputValue(), 'rigid')
    assert.deepEqual(await page.locator('#landmark-transform option').allTextContents(), ['Rigid, 6 parameters','Similarity, 7 parameters','Anisotropic scale, 9 parameters','Affine, 12 parameters'])
    const caps = await page.evaluate(() => ({
      ua: navigator.userAgent,
      pointerEvents: 'PointerEvent' in window,
      downloads: 'download' in document.createElement('a'),
      webgl2: !!document.createElement('canvas').getContext('webgl2'),
      devicePixelRatio,
    }))
    assert.equal(caps.pointerEvents, true, `${name} lacks Pointer Events`)
    assert.equal(caps.downloads, true, `${name} lacks download fallback`)
    assert.deepEqual(errors, [], `${name} page errors: ${errors.join('\n')}`)
    results.push({ engine: name, status: 'passed', version: await browser.version(), capabilities: caps })
  } catch (error) {
    results.push({ engine: name, status: 'failed', error: String(error?.stack || error) })
    if (required.has(name)) throw error
  } finally {
    if (browser) await browser.close()
  }
}
const reportPath = process.env.BRAINANA_BROWSER_REPORT || path.join(root, 'browser-engine-results.json')
fs.writeFileSync(reportPath, JSON.stringify({ generatedAt: new Date().toISOString(), required: [...required], results }, null, 2) + '\n')
console.log(JSON.stringify(results, null, 2))
if (!results.some(r => r.status === 'passed')) throw new Error('No browser engine completed the matrix')

} finally {await harness.close()}
