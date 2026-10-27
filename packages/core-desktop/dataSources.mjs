import {ipcMain} from 'electron'
// Fixed actions only: renderer callers cannot supply SSH commands, paths or URLs.
export function installDataSourceActions({window,canConnect,connect}) {
  ipcMain.handle('align-data-source',async(event,action)=>{
    const win=window()
    if(!win || event.sender!==win.webContents || event.senderFrame!==win.webContents.mainFrame)throw new Error('Untrusted data-source sender')
    if(action==='status')return {canConnect}
    if(action!=='connect')throw new Error('Unknown data-source action')
    if(!canConnect)throw new Error('SSH/SFTP connection setup requires the packaged macOS app')
    await connect()
    return {completed:true}
  })
}
