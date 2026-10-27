#!/usr/bin/env node
// Align server composition root; shared core never imports the app domain.
import path from 'node:path'
import {fileURLToPath} from 'node:url'
import {runServerCli} from '@brainana/core-server/runtime.mjs'
import {VERSION,BUILD_ID,SOURCE_BASE,BEHAVIOR_REFERENCE} from './version.mjs'
runServerCli({version:VERSION,buildId:BUILD_ID,sourceBase:SOURCE_BASE,behaviorReference:BEHAVIOR_REFERENCE,distRoot:path.join(path.dirname(fileURLToPath(import.meta.url)),'dist')})
