import fs from 'node:fs'
import {execFileSync} from 'node:child_process'
const version=process.argv[2]
if(!version||!/^\d+\.\d+\.\d+(?:-[0-9A-Za-z.-]+)?$/.test(version))throw Error('Usage: npm run set-version -- 0.17.11')
const meta=JSON.parse(fs.readFileSync('VERSION.json','utf8'))
if(meta.version!==version)meta.bundleVersion=String(Number(meta.bundleVersion)+1)
meta.version=version;meta.buildId=version;meta.releaseName=`Brainana Align v${version}`
fs.writeFileSync('VERSION.json',JSON.stringify(meta,null,2)+'\n')
execFileSync(process.execPath,['scripts/generate-version.mjs'],{stdio:'inherit'})
console.log('Version and lockfile synchronized. Commit the changes before creating tag v'+version)
