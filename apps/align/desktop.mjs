// Composition root: the Align domain supplies identity and backend; core-desktop owns the shell.
import path from 'node:path'
import {fileURLToPath} from 'node:url'
import {app} from 'electron'
import {runDesktop} from '@brainana/core-desktop/main.mjs'
import {VERSION,BUILD_ID} from './version.mjs'
import {installJipActions} from './desktop/jipActions.mjs'
import {connectWorkstation} from './desktop/workstation.mjs'
const here=path.dirname(fileURLToPath(import.meta.url))
runDesktop({installDomainActions:installJipActions,appLabel:'Brainana Align',version:VERSION,buildId:BUILD_ID,serverPath:path.join(here,'server.mjs'),connectRemote:process.platform==='darwin'&&app.isPackaged?connectWorkstation:null})
