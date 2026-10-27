import {fileURLToPath} from 'node:url'
import assert from 'node:assert/strict'
import { randomBytes } from 'node:crypto'
import { spawn } from 'node:child_process'
import fs from 'node:fs'
import os from 'node:os'
import path from 'node:path'

const root = fs.mkdtempSync(path.join(os.tmpdir(), 'ba-security-root-'))
const handshake = fs.mkdtempSync(path.join(os.tmpdir(), 'ba-security-port-'))
const portFile = path.join(handshake, 'port')
const token = randomBytes(32).toString('hex')
const serverPath = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '../apps/align/server.mjs')
const child = spawn(process.execPath, [serverPath, '--port', '0', '--port-file', portFile, '--root', root, '--session-token', token, '--max-json-bytes', '64', '--max-upload-bytes', '128'], { stdio: ['ignore', 'pipe', 'pipe'] })
let stderr = ''
child.stderr.on('data', data => { stderr += data })
try {
  let port = 0
  for (let index = 0; index < 300; index += 1) {
    if (fs.existsSync(portFile)) { port = Number(fs.readFileSync(portFile, 'utf8').trim()); break }
    if (child.exitCode !== null) throw new Error(`server exited early: ${stderr}`)
    await new Promise(resolve => setTimeout(resolve, 20))
  }
  assert.ok(port)
  const base = `http://127.0.0.1:${port}`
  const authorized = { 'x-brainana-session': token }
  assert.equal((await fetch(`${base}/api/health`)).status, 401)
  assert.equal((await fetch(`${base}/api/health`, { headers: { 'x-brainana-session': 'wrong' } })).status, 401)
  assert.equal((await fetch(`${base}/api/health`, { headers: { ...authorized, origin: 'https://example.com' } })).status, 403)
  assert.equal((await fetch(`${base}/api/health`, { method: 'POST', headers: authorized })).status, 405)
  assert.equal((await fetch(`${base}/api/health`, { headers: authorized })).status, 200)
  assert.equal((await fetch(`${base}/api/save-mkdir`, { method: 'POST', headers: { ...authorized, 'content-type': 'text/plain' }, body: '{}' })).status, 415)
  assert.equal((await fetch(`${base}/api/save-mkdir`, { method: 'POST', headers: { ...authorized, 'content-type': 'application/json' }, body: JSON.stringify({ path: 'x'.repeat(100) }) })).status, 413)
  assert.equal((await fetch(`${base}/api/save-file?path=large.nii`, { method: 'POST', headers: { ...authorized, 'content-type': 'application/octet-stream' }, body: new Uint8Array(256) })).status, 413)
  assert.equal(fs.existsSync(path.join(root, 'large.nii')), false)
  const index = await fetch(`${base}/`)
  assert.equal(index.status, 200)
  assert.match(index.headers.get('content-security-policy') || '', /default-src 'self'/)
  assert.equal(index.headers.get('referrer-policy'), 'no-referrer')
  const outside = fs.mkdtempSync(path.join(os.tmpdir(), 'ba-security-outside-'))
  try {
    fs.writeFileSync(path.join(outside, 'private.nii'), 'private bytes')
    fs.symlinkSync(outside, path.join(root, 'escape'))
    const escapedRead = await fetch(`${base}/api/file?path=escape/private.nii`, { headers: authorized })
    assert.notEqual(escapedRead.status, 200, 'symlink reads must stay within the data root')
    const escapedWrite = await fetch(`${base}/api/save-file?path=escape/new.nii`, {method:'POST',headers:{...authorized,'content-type':'application/octet-stream'},body:new Uint8Array(8)})
    assert.notEqual(escapedWrite.status,200)
    assert.equal(fs.existsSync(path.join(outside,'new.nii')),false)
  } finally { fs.rmSync(outside,{recursive:true,force:true}) }
  console.log('server security checks passed, including symlink escape rejection')
} finally {
  child.kill('SIGTERM')
  fs.rmSync(root, { recursive: true, force: true })
  fs.rmSync(handshake, { recursive: true, force: true })
}
