import path from 'node:path'
import fs from 'node:fs'
import {fileURLToPath} from 'node:url'
import {preparePayload} from './prepare-payload.mjs'
const root=path.resolve(path.dirname(fileURLToPath(import.meta.url)),'..')
if(process.argv.length>2)throw new Error('Desktop uses embedded Electron Node. For standalone Node use prepare-browser.mjs ARM64_NODE X64_NODE.')
const scientific=path.join(root,'generated/scientific-runtime',process.platform==='win32'?'BrainanaAlignRegistration.exe':'BrainanaAlignRegistration')
if(!fs.existsSync(scientific))throw new Error('Build Align’s private scientific runtime first (scientific/Align-Registration.spec). No external JIP app is used.')
preparePayload(root,path.join(root,'generated/workstation-support'))
console.log('Slim desktop workstation payload prepared; no standalone Node runtimes')
