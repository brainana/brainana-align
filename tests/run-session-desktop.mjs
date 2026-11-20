import {spawnSync} from 'node:child_process'
import {createRequire} from 'node:module'
import path from 'node:path'
import {fileURLToPath} from 'node:url'
const root=path.resolve(path.dirname(fileURLToPath(import.meta.url)),'..'),require=createRequire(import.meta.url)
const executable=process.env.ELECTRON_PATH||require('electron'),env={...process.env};delete env.ELECTRON_RUN_AS_NODE
const result=spawnSync(executable,[path.join(root,'tests/session-desktop-e2e.mjs')],{cwd:root,env,stdio:'inherit',timeout:120000})
if(result.error)throw result.error
process.exitCode=result.status??1
