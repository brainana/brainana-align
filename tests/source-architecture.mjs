import assert from 'node:assert/strict'
import fs from 'node:fs'
import path from 'node:path'
import { fileURLToPath } from 'node:url'
const root = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '..')
const src = path.join(root, 'source', 'src')
const requiredModules = [
  'app/bootstrap.ts', 'app/types.ts',
  'domain/registration/rigid.ts', 'domain/registration/linear.ts', 'domain/registration/automaticLinear.ts', 'domain/registration/nonlinear.ts', 'domain/registration/deformable.ts', 'domain/imaging/roiWarp.ts',
  'domain/optimization/windows.ts', 'domain/sessions/persistence.ts',
  'domain/export/artifacts.ts',
  'viewer/interaction.ts', 'viewer/coordinateProjection.ts',
  'viewer/crosshairController.ts', 'viewer/landmarkRenderer.ts',
  'viewer/optimizationWindowInteraction.ts',
  'infrastructure/runtime/client.ts', 'infrastructure/runtime/integration.ts',
  'infrastructure/runtime/desktopApi.ts',
  'infrastructure/remote/filesystemClient.ts', 'infrastructure/remote/volumeLoader.ts',
  'infrastructure/remote/endpoint.ts', 'infrastructure/remote/electronIntegration.ts',
  'ui/workstationBrowser.ts', 'ui/exportDestination.ts', 'ui/linearRegistrationControls.ts', 'ui/nonlinearLandmarkControls.ts', 'ui/workflowNavigation.ts',
]
for (const file of requiredModules) {
  assert.ok(fs.existsSync(path.join(src,file)), `missing layered source file ${file}`)
}
for (const obsolete of ['main.ts', 'appTypes.ts', 'runtimeIntegration.ts', 'filesystemClient.ts']) {
  assert.ok(!fs.existsSync(path.join(src, obsolete)), `flat source module must not return: ${obsolete}`)
}
const main = fs.readFileSync(path.join(src,'app','bootstrap.ts'),'utf8')
for (const module of ['domain/optimization/windows','viewer/interaction','viewer/coordinateProjection','viewer/crosshairController','viewer/landmarkRenderer','domain/sessions/persistence','domain/export/artifacts','viewer/optimizationWindowInteraction']) {
  assert.ok(main.includes(module), `application bootstrap must use ${module}`)
}
assert.doesNotMatch(main, /function withinOptimizationWindows/)
assert.doesNotMatch(main, /function installWheelZoomAndSlice/)
assert.doesNotMatch(main, /function markerDepthDistance/)
assert.doesNotMatch(main, /document\.createElementNS\([^\n]+review-marker/)
assert.doesNotMatch(main, /type SessionPayload/)
assert.doesNotMatch(main, /function geometryDifference/)
assert.doesNotMatch(main, /function matrixText/)
assert.doesNotMatch(main, /frac2canvasPos\(nv\.mm2frac\(point\)\)/)
const lines = main.split('\n').length
const codeLines = main.split('\n').filter(line => line.trim()).length
// The global linear workflow adds its panel orchestration here while all search,
// parameterization, and diagnostics remain isolated in automaticLinear.ts.
assert.ok(codeLines < 1550, `application bootstrap exceeded the current guarded baseline: ${codeLines} nonblank lines`)
console.log(`layered source architecture checks passed (${lines} total, ${codeLines} nonblank lines in application bootstrap)`)

const runtimeIntegration = fs.readFileSync(path.join(src,'infrastructure','runtime','integration.ts'),'utf8')
assert.ok(runtimeIntegration.includes('installExportDestination'), 'runtimeIntegration must compose export destination module')
assert.ok(runtimeIntegration.includes('createWorkstationBrowser'), 'runtimeIntegration must compose workstation browser module')
for (const forbidden of ['server-browser-modal','ba-export-panel','/api/save-file','/api/file?path=']) {
  assert.ok(!runtimeIntegration.includes(forbidden), `runtimeIntegration should not contain implementation detail ${forbidden}`)
}
console.log('runtime integration architecture checks passed')

const server = fs.readFileSync(path.join(root,'source','server.mjs'),'utf8')
assert.match(server, /from '\.\/sftpClient\.mjs'/)
for (const forbidden of ['stat -c %s','find ${','/usr/bin/scp',' cat ${']) assert.ok(!server.includes(forbidden), `server must not use remote shell transport: ${forbidden}`)
console.log('structured SFTP architecture checks passed')

const launcherSource = fs.readFileSync(path.resolve(root, 'packaging/brainana-align-launcher'), 'utf8')
if (launcherSource.includes("randomBytes(32)") || launcherSource.includes('SESSION_TOKEN="$("$NODE"')) {
  throw new Error('Launcher must not start a second Node process to generate the session token')
}
if (!launcherSource.includes('--handshake-file "$HANDSHAKE_FILE"')) {
  throw new Error('Launcher must use the server-owned atomic handshake file')
}
const serverSource = fs.readFileSync(path.resolve(root, 'source/server.mjs'), 'utf8')
if (!serverSource.includes("crypto.randomBytes(32).toString('hex')") || !serverSource.includes('sessionToken,')) {
  throw new Error('Server must generate and publish the per-launch session token')
}
