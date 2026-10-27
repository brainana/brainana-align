import {Menu,app} from 'electron'
export function applyAppMenu({appLabel,window,connectRemote,returnLocal}) {
  const mac=process.platform==='darwin'
  const template=[...(mac?[{label:appLabel,submenu:[{role:'about'},{type:'separator'},{label:'Settings…',accelerator:'CmdOrCtrl+,',click:()=>window()?.webContents.executeJavaScript("document.getElementById('settings-open')?.click()")},{type:'separator'},{role:'services'},{role:'hide'},{role:'hideOthers'},{role:'unhide'},{type:'separator'},{role:'quit'}]}]:[]),{label:'File',submenu:[
    ...(connectRemote?[{label:'Connect to workstation…',click:connectRemote},{label:'Return to local files…',click:returnLocal},{type:'separator'}]:[]),
    {label:'Save session…',accelerator:'CmdOrCtrl+S',click:()=>window()?.webContents.executeJavaScript("document.getElementById('save-session')?.click()")},
    ...(mac?[]:[{label:'Settings…',accelerator:'CmdOrCtrl+,',click:()=>window()?.webContents.executeJavaScript("document.getElementById('settings-open')?.click()")},{type:'separator'},{role:'quit'}])]},
    {role:'editMenu'},{label:'View',submenu:[{role:'resetZoom'},{role:'zoomIn'},{role:'zoomOut'},{type:'separator'},{role:'togglefullscreen'},{label:'Developer tools',role:'toggleDevTools'}]},
    {role:'windowMenu'}, {label:'Help',submenu:[{label:'Guided setup',click:()=>window()?.webContents.executeJavaScript("document.getElementById('guided-setup')?.click()")},...(!mac?[{label:'About '+appLabel,click:()=>app.showAboutPanel()}]:[])]}]
  Menu.setApplicationMenu(Menu.buildFromTemplate(template))
}
