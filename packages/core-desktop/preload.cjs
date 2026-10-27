const {contextBridge,ipcRenderer}=require('electron')
contextBridge.exposeInMainWorld('brainanaPreferences',{memory:()=>ipcRenderer.invoke('align-system-memory'),chooseExportFolder:()=>ipcRenderer.invoke('align-preferences-export-folder'),read:key=>ipcRenderer.invoke('align-preferences-read',key),write:(key,value)=>ipcRenderer.invoke('align-preferences-write',key,value)})

contextBridge.exposeInMainWorld('brainanaDataSources',{status:()=>ipcRenderer.invoke('align-data-source','status'),connect:()=>ipcRenderer.invoke('align-data-source','connect')})

// Fixed, application-owned scientific actions; absent when no domain installs them.
contextBridge.exposeInMainWorld('brainanaScientific',{status:()=>ipcRenderer.invoke('brainana-scientific','status'),run:request=>ipcRenderer.invoke('brainana-scientific','run',request),poll:()=>ipcRenderer.invoke('brainana-scientific','poll'),cancel:()=>ipcRenderer.invoke('brainana-scientific','cancel')})
