import fs from 'node:fs'
import {getConfig} from 'app-builder-lib/out/util/config/config.js'
const version=JSON.parse(fs.readFileSync('VERSION.json','utf8')).version
// Load the canonical payload lists, then REPLACE targets. electron-builder
// concatenates arrays under extends, which would retain stale installer types.
const config=await getConfig(process.cwd(),'apps/align/electron-builder.yml')
config.afterPack='./scripts/ci/after-pack.cjs'
config.mac={...config.mac,target:['dmg'],identity:null,artifactName:`brainana-align-${version}-\${arch}.\${ext}`}
config.win={...config.win,target:['nsis','zip'],artifactName:`brainana-align-${version}-\${arch}.\${ext}`}
config.nsis={...config.nsis,artifactName:`brainana-align-setup-${version}-\${arch}.\${ext}`}
config.linux={...config.linux,maintainer:'Brainana <noreply@users.noreply.github.com>',target:['AppImage','deb'],artifactName:`brainana-align-${version}-\${arch}.\${ext}`}
fs.mkdirSync('generated',{recursive:true});fs.writeFileSync('generated/github-builder.yml',JSON.stringify(config,null,2)+'\n')
