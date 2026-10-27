import {startBrowserHarness} from './browser-harness.mjs'
import assert from 'node:assert/strict'
import fs from 'node:fs'
import path from 'node:path'
import { createRequire } from 'node:module'
import { fileURLToPath } from 'node:url'
const require = createRequire(new URL('../apps/align/package.json', import.meta.url))
const { chromium } = require('playwright-core')

const root = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '..')
const dist = path.join(root, 'apps/align', 'dist')
const token = 'a'.repeat(64)
const harness=await startBrowserHarness()
let browser
try {
  browser = await chromium.launch({
    executablePath: process.env.CHROMIUM_PATH || chromium.executablePath(),
    headless: true,
    args: ['--no-sandbox', '--disable-dev-shm-usage', '--use-gl=swiftshader'],
  })
  const page = await browser.newPage({ viewport: { width: 1600, height: 1000 } })
  const pageErrors = []
  page.on('pageerror', error => pageErrors.push(error.message))
  await page.addInitScript(() => {
    const getContext=HTMLCanvasElement.prototype.getContext
    HTMLCanvasElement.prototype.getContext=function(type,...args){return type==='webgl2'?null:getContext.call(this,type,...args)}
  })
  await page.goto(harness.url+'/#session='+token)
  await page.waitForSelector('#welcome-screen')
  assert.equal(await page.locator('[data-registration-pairing]').count(),4)
  await page.locator('[data-registration-pairing="mri-ct"]').click()
  await page.waitForSelector('#mri-sagittal-placeholder')
  await page.waitForSelector('#browser-compatibility')

  for (const plane of ['sagittal', 'coronal', 'axial']) {
    await assertText(page, `#mri-${plane}-placeholder`, 'No MRI selected')
    await assertText(page, `#ct-${plane}-placeholder`, 'No CT selected')
    assert.equal(await page.locator(`#mri-${plane}-window-layer`).count(), 1)
    assert.equal(await page.locator(`#ct-${plane}-window-layer`).count(), 1)
  }
  const webglAvailable = await page.evaluate(() => !!document.createElement('canvas').getContext('webgl2'))
  if (!webglAvailable) await assertText(page, '#status', 'This browser cannot initialize the required WebGL2 viewer.')
  assert.equal(await page.locator('#landmark-transform').inputValue(), 'rigid')
  assert.deepEqual(await page.locator('#landmark-transform option').allTextContents(), ['Rigid, 6 parameters','Similarity, 7 parameters','Anisotropic scale, 9 parameters','Affine, 12 parameters'])
  await assertText(page, '#window-summary', 'No optimization windows defined. The full geometric overlap will be used.')

  assert.ok(harness.requests.some(entry => entry.path === '/api/config' && entry.token === 'a'.repeat(64)), 'Runtime API request did not include the per-launch session token')

  assert.equal(await page.locator('#mri-file').isDisabled(), true)
  assert.equal(await page.locator('#ct-file').isDisabled(), true)
  assert.equal(await page.locator('#export-open').isDisabled(), true)

  assert.deepEqual(pageErrors, [], `Unexpected browser page errors: ${pageErrors.join('\n')}`)
  assert.equal(await page.locator('#browser-compatibility').evaluate(element => element.classList.contains('error')), true)
  await page.evaluate(()=>document.fonts.ready)
  for(const viewport of [{width:1540,height:980},{width:1000,height:680}]) {
    await page.setViewportSize(viewport)
    const geometry=await page.evaluate(()=>{
      const rect=selector=>{const r=document.querySelector(selector).getBoundingClientRect();return {left:r.left,right:r.right,top:r.top,bottom:r.bottom,width:r.width,height:r.height}}
      return {rail:rect('.sidebar'),workspace:rect('.workspace'),status:rect('.statusbar'),toolbar:rect('.topbar'),overflow:document.documentElement.scrollWidth>innerWidth,panes:[...document.querySelectorAll('.view-card')].map(e=>({width:e.clientWidth,height:e.clientHeight})),font:document.fonts.check('13px "Source Sans 3"')}
    })
    assert.ok(!geometry.overflow,'Shell must fit the native minimum window width')
    assert.ok(geometry.rail.right<=geometry.workspace.left,'Control rail must remain left of imaging workspace')
    assert.ok(geometry.status.bottom<=viewport.height+1,'Status must remain inside the window')
    assert.ok(geometry.toolbar.bottom<=geometry.workspace.top,'Toolbar must not cover image panes')
    assert.ok(geometry.panes.every(p=>p.width>100&&p.height>100),'All nine image panes must remain usable')
    assert.ok(geometry.font,'Offline font must load')
    await page.locator('#guided-setup').click()
    const card=await page.locator('#guided-card').boundingBox()
    assert.ok(card.x>=0&&card.y>=0&&card.x+card.width<=viewport.width+1&&card.y+card.height<=viewport.height+1,'Guide must stay in the window')
    await page.locator('#guided-close').click()
  }
  console.log('browser UI regression checks passed')
} finally {
  if (browser) await browser.close()
  await harness.close()
}

async function assertText(page, selector, expected) {
  const actual = (await page.locator(selector).textContent())?.trim()
  assert.equal(actual, expected, `${selector} text mismatch`)
}
