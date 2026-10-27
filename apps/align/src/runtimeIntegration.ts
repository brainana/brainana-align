import {createNetworkFileBrowser} from './networkFileBrowser'
import {installDataSourceChooser} from './dataSourceChooser'
import { installExportDestination } from './exportDestination'
import { isRemoteRuntime, loadRuntimeConfig } from './runtimeClient'
import {
  createWorkstationBrowser,
  type LoadFiles,
  type Status,
} from './workstationBrowser'

export type { Modality } from './workstationBrowser'
export type { RuntimeConfig } from './runtimeClient'
export type { ServerEntry, ServerList } from './filesystemClient'

/**
 * Installs the browser-facing runtime adapters.
 *
 * This module intentionally contains no file-browser, transfer, or export UI
 * implementation. It only composes the smaller runtime modules.
 */
export function installRuntimeIntegration(loadFiles: LoadFiles, setStatus: Status): void {
  installExportDestination()
  const workstationBrowser = createWorkstationBrowser(loadFiles, setStatus)
  const networkBrowser=createNetworkFileBrowser(loadFiles,setStatus)
  installDataSourceChooser({
    connection: async () => {
      const config = await loadRuntimeConfig()
      return {remote: isRemoteRuntime(config), label: config.label, canConnect: config.enabled === true}
    },
    browse: modality => workstationBrowser.open(modality),
    connect: modality => networkBrowser.open(modality),
  })
}
