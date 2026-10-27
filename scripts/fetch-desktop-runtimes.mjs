// Reproducible, SHA-256 verified optional browser/workstation Node payloads.
import fs from 'node:fs/promises'
import path from 'node:path'
import crypto from 'node:crypto'
import {execFileSync} from 'node:child_process'
const version='v24.17.0',out=path.resolve(process.argv[2] || 'generated/node-runtimes')
await fs.mkdir(out,{recursive:true})
const base=`https://nodejs.org/dist/${version}/`
async function download(url){const r=await fetch(url);if(!r.ok)throw new Error(`Download failed: ${r.status}`);return Buffer.from(await r.arrayBuffer())}
const manifest=(await download(base+'SHASUMS256.txt')).toString()
await fs.writeFile(path.join(out,'SHASUMS256.txt'),manifest)
for(const arch of ['arm64','x64']) {
 const name=`node-${version}-darwin-${arch}.tar.gz`,data=await download(base+name)
 const expected=manifest.split('\n').find(line=>line.endsWith('  '+name))?.split(' ')[0]
 if(!expected || crypto.createHash('sha256').update(data).digest('hex')!==expected)throw new Error('Node archive integrity mismatch: '+name)
 await fs.writeFile(path.join(out,name),data)
 execFileSync('/usr/bin/tar',['-xzf',path.join(out,name),'-C',out])
 console.log(path.join(out,`node-${version}-darwin-${arch}/bin/node`))
}
