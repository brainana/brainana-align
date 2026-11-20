const {contextBridge,ipcRenderer,webUtils}=require('electron')
contextBridge.exposeInMainWorld('brainanaPreferences',{memory:()=>ipcRenderer.invoke('align-system-memory'),chooseExportFolder:()=>ipcRenderer.invoke('align-preferences-export-folder'),read:key=>ipcRenderer.invoke('align-preferences-read',key),write:(key,value)=>ipcRenderer.invoke('align-preferences-write',key,value)})

contextBridge.exposeInMainWorld('brainanaDataSources',{status:()=>ipcRenderer.invoke('align-data-source','status'),connect:()=>ipcRenderer.invoke('align-data-source','connect')})

// Paths come only from actual browser-selected Files, never renderer strings.
contextBridge.exposeInMainWorld('brainanaSessionFiles',{path:file=>webUtils.getPathForFile(file)||null,open:(file,wanted)=>{const path=webUtils.getPathForFile(file);return path?ipcRenderer.invoke('align-session-files','open',{path,wanted}):Promise.resolve({unavailable:true})},choose:(signature,label,problem)=>ipcRenderer.invoke('align-session-files','choose',{signature,label,problem})})

// Fixed, application-owned scientific actions; absent when no domain installs them.
contextBridge.exposeInMainWorld('brainanaScientific',{status:()=>ipcRenderer.invoke('brainana-scientific','status'),run:request=>ipcRenderer.invoke('brainana-scientific','run',request),poll:()=>ipcRenderer.invoke('brainana-scientific','poll'),cancel:()=>ipcRenderer.invoke('brainana-scientific','cancel')})
