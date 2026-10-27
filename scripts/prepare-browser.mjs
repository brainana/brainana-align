import fs from 'node:fs'
import path from 'node:path'
import {fileURLToPath} from 'node:url'
import {preparePayload} from './prepare-payload.mjs'
const root=path.resolve(path.dirname(fileURLToPath(import.meta.url)),'..')
if(!process.argv[2] || !process.argv[3])throw new Error('Usage: prepare-browser.mjs ARM64_NODE X64_NODE')
const target=path.join(root,'generated/Brainana Align Browser.app')
preparePayload(root,target,[['arm64',path.resolve(process.argv[2])],['x64',path.resolve(process.argv[3])]])
let plist=fs.readFileSync(path.join(root,'generated/Info.plist'),'utf8')
plist=plist.replaceAll('<string>Brainana Align</string>','<string>Brainana Align Browser</string>').replace('<string>org.brainana.align</string>','<string>org.brainana.align.browser</string>')
fs.writeFileSync(path.join(target,'Contents/Info.plist'),plist)
console.log('Separate universal browser edition prepared: '+target)
