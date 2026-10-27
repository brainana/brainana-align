// Optional browser composition root, sharing the same authenticated process bootstrap.
import path from 'node:path'
import {fileURLToPath} from 'node:url'
import {spawn} from 'node:child_process'
import {bootServer} from '@brainana/core-launcher/launch.mjs'
import {browserLaunchSpec,applicationPaths} from '@brainana/core-server/platformCore.mjs'
import {VERSION,BUILD_ID} from './version.mjs'
const here=path.dirname(fileURLToPath(import.meta.url))
const boot=await bootServer({serverPath:path.join(here,'server.mjs'),version:VERSION,buildId:BUILD_ID,cache:applicationPaths({version:VERSION}).cacheRoot})
const spec=browserLaunchSpec(boot.url)
spawn(spec.command,spec.args,{stdio:'ignore'}).on('error',error=>console.error('Browser launch failed:',error.message))
for(const signal of ['SIGINT','SIGTERM'])process.on(signal,()=>boot.stop().finally(()=>process.exit()))
console.log(`Brainana Align ${VERSION} opened in the default browser. Ctrl-C stops its data server.`)
