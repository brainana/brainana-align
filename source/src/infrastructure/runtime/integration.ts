import { installExportDestination } from '../../ui/exportDestination'
import { installElectronRemoteIntegration } from '../remote/electronIntegration'
import { isRemoteRuntime, loadRuntimeConfig } from './client'
import {
  createWorkstationBrowser,
  type LoadFiles,
  type Status,
} from '../../ui/workstationBrowser'

export type { Modality } from '../../ui/workstationBrowser'
export type { RuntimeConfig } from './client'
export type { ServerEntry, ServerList } from '../remote/filesystemClient'

/**
 * Installs the browser-facing runtime adapters.
 *
 * This module intentionally contains no file-browser, transfer, or export UI
 * implementation. It only composes the smaller runtime modules.
 */
export function installRuntimeIntegration(loadFiles: LoadFiles, setStatus: Status): void {
  installExportDestination()
  const workstationBrowser = createWorkstationBrowser(loadFiles, setStatus)
  if (installElectronRemoteIntegration(workstationBrowser, setStatus)) return
  void loadRuntimeConfig()
    .then(config => {
      if (isRemoteRuntime(config)) workstationBrowser.installButtons()
    })
    .catch(error => {
      console.warn('Unable to read runtime configuration; workstation controls remain disabled.', error)
    })
}
