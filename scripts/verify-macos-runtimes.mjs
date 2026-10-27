import fs from 'node:fs'
import { inspectMachO } from './macho.mjs'
const metadata=JSON.parse(fs.readFileSync(new URL('../apps/align/VERSION.json',import.meta.url),'utf8'))
const declared=metadata.minimumMacOS.split('.').reduce((n,v,i)=>n+Number(v)*[65536,256,1][i],0)
for(const [arg,cpu,arch] of [[process.argv[2],0x100000c,'arm64'],[process.argv[3],0x1000007,'x86_64']]) {
 if(!arg)throw new Error('Provide both arm64 and x64 Node executables')
 const inspected=inspectMachO(fs.readFileSync(arg),cpu)
 if(declared<inspected.minimumMacOS)throw new Error(`${arch} runtime requires a higher macOS deployment target than VERSION.json`)
 console.log(`Validated ${arch} Mach-O runtime: ${inspected.bytes} bytes`)
}
