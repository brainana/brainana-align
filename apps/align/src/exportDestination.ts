import {createExportDirectory, joinServerPath, listExportDirectories, saveRemoteBlob} from './filesystemClient'
import {isRemoteRuntime, loadRuntimeConfig} from './runtimeClient'
import {exportFilename, exportFilenames} from '@brainana/core-client/exportNames.mjs'
import type {ExportArtifact} from './exportArtifacts'

declare global {
  interface Window {
    brainanaAlignSaveBlob?: ((blob: Blob, filename: string) => Promise<void>) & {artifacts?:(artifacts:ExportArtifact[])=>Promise<void>}
    brainanaExports?: {save:(files:{url:string;filename:string}[])=>Promise<{canceled:boolean;paths?:string[]}>}
    showSaveFilePicker?: (options:{suggestedName:string})=>Promise<FileSystemFileHandle>
  }
}
const cancelled = () => Error('Save cancelled. No further files were saved.')

export function installExportDestination(): void {
  let remote = false, destination = 'local', browsePath = '', remotePath = '', chain = Promise.resolve(), finish:((error?:Error)=>void)|null = null
  const style = document.createElement('style')
  style.dataset.brainanaModule = 'export-destination'
  style.textContent = `#ba-export-panel{margin:12px 0;padding:12px;border:1px solid var(--border-2);border-radius:8px;background:var(--panel-2)}#ba-export-status{min-height:18px;margin-top:8px;font-size:12px;overflow-wrap:anywhere}.ba-overlay{position:fixed;inset:0;z-index:1000000;background:rgba(0,0,0,.72);display:flex;align-items:center;justify-content:center}.ba-dialog{width:min(680px,90vw);max-height:85vh;overflow:auto;background:var(--panel);border:1px solid var(--border-2);border-radius:10px;display:flex;flex-direction:column}.ba-dialog header{padding:14px 16px;font-weight:700;border-bottom:1px solid var(--border-2)}.ba-nav,.ba-actions{padding:10px 14px;display:flex;gap:8px;align-items:center}.ba-list{min-height:100px;max-height:30vh;overflow:auto;border-block:1px solid var(--border-2)}.ba-folder-row{width:100%;text-align:left;border:0;border-bottom:1px solid var(--border-2);border-radius:0;background:transparent;padding:10px 14px}.ba-actions{justify-content:flex-end}.ba-empty{padding:16px;color:var(--muted)}.ba-msg{padding:0 14px;min-height:20px;color:var(--danger)}.ba-save-fields{padding:14px}.ba-save-fields input{width:100%}#ba-save-related{overflow-wrap:anywhere}`
  document.head.append(style)
  const panel = document.createElement('div'); panel.id = 'ba-export-panel'
  panel.innerHTML = `<div id="ba-destination-row"><label>Save to <select id="ba-destination"><option value="local">This computer</option><option value="workstation">Workstation</option></select></label></div><p class="hint">Each Save button lets you choose the filename and folder together. Accompanying metadata uses the same name.</p><div id="ba-export-status" role="status" aria-live="polite"></div>`
  const exportBody = document.querySelector('#export-modal .modal-card'); exportBody?.insertBefore(panel, exportBody.querySelector('h3'))
  const status = (message:string, error = false) => { const element = panel.querySelector<HTMLElement>('#ba-export-status')!; element.textContent = message; element.style.color = error ? 'var(--danger)' : 'var(--muted)'; window.dispatchEvent(new CustomEvent('align-export-result',{detail:{message,error}})) }
  panel.querySelector<HTMLSelectElement>('#ba-destination')!.addEventListener('change', e => { destination = (e.target as HTMLSelectElement).value })
  const configReady = loadRuntimeConfig().then(config => {remote = isRemoteRuntime(config)}).catch(() => {}).then(() => {panel.querySelector<HTMLElement>('#ba-destination-row')!.hidden = !remote})

  // Browser/workstation fallback uses one filename + folder dialog. Desktop
  // saving goes straight to the operating system's Save dialog.
  const modal = document.createElement('div'); modal.id = 'ba-folder-modal'; modal.className = 'ba-overlay modal hidden'
  modal.innerHTML = `<form class="ba-dialog" role="dialog" aria-modal="true" aria-labelledby="ba-save-title"><header id="ba-save-title">Save file</header><div id="ba-save-folders"><div class="ba-nav"><button id="ba-remote-up" type="button">Up</button><button id="ba-remote-new" type="button">New folder</button><code id="ba-remote-path">/</code></div><div id="ba-remote-list" class="ba-list"></div></div><div class="ba-save-fields"><label>Filename<input id="ba-save-name" type="text" autocomplete="off" required></label><p id="ba-save-related" class="hint"></p><p id="ba-save-location" class="hint"></p></div><div id="ba-remote-msg" class="ba-msg" role="status"></div><div class="ba-actions"><button id="ba-remote-cancel" type="button">Cancel</button><button id="ba-save-submit" type="submit" class="primary">Save</button></div></form>`
  document.body.append(modal)
  const name = modal.querySelector<HTMLInputElement>('#ba-save-name')!, message = modal.querySelector<HTMLElement>('#ba-remote-msg')!, submit = modal.querySelector<HTMLButtonElement>('#ba-save-submit')!
  let listing = 0, writing = false
  const listRemote = async (path = '') => {
    if (writing) return
    const request = ++listing; submit.disabled = true
    const data = await listExportDirectories(path)
    if (request !== listing || writing || modal.classList.contains('hidden')) return
    submit.disabled = false; browsePath = data.path || ''; modal.querySelector('#ba-remote-path')!.textContent = '/' + browsePath
    const list = modal.querySelector('#ba-remote-list')!; list.replaceChildren()
    for (const entry of data.entries.filter(entry => entry.directory)) { const button = document.createElement('button'); button.type = 'button'; button.className = 'ba-folder-row'; button.textContent = entry.name; button.onclick = () => {void listRemote(entry.path).catch(error => {message.textContent = error.message})}; list.append(button) }
    if (!list.childNodes.length) {const empty = document.createElement('div'); empty.className = 'ba-empty'; empty.textContent = 'No subfolders'; list.append(empty)}
    modal.querySelector<HTMLButtonElement>('#ba-remote-up')!.disabled = !browsePath
  }
  modal.querySelector('#ba-remote-up')!.addEventListener('click', () => {void listRemote(browsePath.split('/').slice(0,-1).join('/')).catch(error => {message.textContent = error.message})})
  modal.querySelector('#ba-remote-new')!.addEventListener('click', () => { if (writing) return; const value = window.prompt('New folder name:'); if (value) void (async () => {await createExportDirectory(joinServerPath(browsePath,exportFilename(value))); await listRemote(browsePath)})().catch(error => {message.textContent = error.message}) })
  const cancel = () => {if (!writing) finish?.(cancelled())}
  modal.querySelector('#ba-remote-cancel')!.addEventListener('click', cancel)
  modal.addEventListener('keydown', e => {if (e.key === 'Escape') {e.preventDefault(); e.stopPropagation(); cancel()}})

  async function saveDialog(files:ExportArtifact[], workstation:boolean):Promise<{names:string[];download:boolean}> {
    let savedNames:string[] = [], download = false
    await new Promise<void>((resolve,reject) => {
      const previous = document.activeElement as HTMLElement|null
      finish = error => {writing = false; listing++; modal.classList.add('hidden'); finish = null; modal.querySelector('form')!.onsubmit = null; name.oninput = null; previous?.focus(); error ? reject(error) : resolve()}
      writing = false; browsePath = remotePath; modal.querySelectorAll<HTMLButtonElement>('button').forEach(button=>{button.disabled=false}); name.disabled=false; submit.disabled = false; name.value = files[0].filename; message.textContent = ''
      modal.querySelector<HTMLElement>('#ba-save-folders')!.hidden = !workstation
      modal.querySelector('#ba-save-title')!.textContent = workstation ? 'Save to workstation — filename and folder' : 'Save file'
      modal.querySelector('#ba-save-location')!.textContent = workstation ? 'Browse to the destination folder, enter the filename, then select Save.' : window.showSaveFilePicker ? 'Save opens your browser’s filename and folder picker.' : 'Your browser controls the destination folder for downloads.'
      const related = () => {try {const names=exportFilenames(files.map(f=>f.filename),name.value);modal.querySelector('#ba-save-related')!.textContent = files.length > 1 ? 'Also saves: ' + names.slice(1).join(', ') : ''; name.setCustomValidity('')} catch (error) {name.setCustomValidity((error as Error).message)}}
      name.oninput = related; related()
      modal.querySelector('form')!.onsubmit = e => {
        e.preventDefault(); if (writing || submit.disabled) return
        void (async () => {
          try {
            savedNames = exportFilenames(files.map(f=>f.filename),name.value); writing = true; modal.querySelectorAll<HTMLButtonElement>('button').forEach(button=>{button.disabled=true}); name.disabled=true
            const selectedPath = browsePath
            if (workstation) {
              for (let i=0;i<files.length;i++) {const path=joinServerPath(selectedPath,savedNames[i]); let result=await saveRemoteBlob(path,files[i].blob,false); if(result.exists){if(!window.confirm(`${savedNames[i]} already exists. Replace it?`))throw cancelled();result=await saveRemoteBlob(path,files[i].blob,true)}status('Saved to workstation: /'+(result.path||path))}
              remotePath = selectedPath
            } else if (window.showSaveFilePicker) {
              // Called from a visible Save click, preserving browser activation
              // after a lengthy resampling operation.
              const handle=await window.showSaveFilePicker({suggestedName:savedNames[0]}), writer=await handle.createWritable(); savedNames[0] = handle.name
              try {await writer.write(files[0].blob); await writer.close()} catch(error){await writer.abort?.().catch(()=>{});throw error}
            } else {
              download = true
              for(let i=0;i<files.length;i++){const url=URL.createObjectURL(files[i].blob),anchor=document.createElement('a');anchor.href=url;anchor.download=savedNames[i];anchor.click();setTimeout(()=>URL.revokeObjectURL(url),30000)}
            }
            finish?.()
          } catch(error) {finish?.(error instanceof Error ? error : Error(String(error)))}
        })()
      }
      modal.classList.remove('hidden'); name.focus(); name.select()
      if(workstation)void listRemote(remotePath).catch(error=>{message.textContent=error.message})
    })
    return {names:savedNames,download}
  }

  async function save(files:ExportArtifact[]) {
    files.forEach(file=>exportFilename(file.filename)); await configReady
    if(remote && destination === 'workstation') {const result=await saveDialog(files,true);status('Saved to workstation: /'+joinServerPath(remotePath,result.names[0]));return}
    if(window.brainanaExports) {
      status('Choose a filename and folder in the Save dialog…')
      const items=files.map(file=>({filename:file.filename,url:URL.createObjectURL(file.blob)}))
      try {const result=await window.brainanaExports.save(items);if(result.canceled)throw cancelled();status('Saved: '+result.paths!.join(', '))} finally {items.forEach(file=>URL.revokeObjectURL(file.url))}
    } else if(window.showSaveFilePicker) {
      let names = files.map(file=>file.filename)
      for (let i=0;i<files.length;i++) {const result=await saveDialog([{...files[i],filename:names[i]}],false);if(i===0)names=exportFilenames(names,result.names[0]);status('Saved: '+result.names[0])}
    } else {const result=await saveDialog(files,false);status('Download requested: '+result.names.join(', '))}
  }
  const queued = (files:ExportArtifact[]) => {const attempt=chain.then(()=>save(files)).catch(error=>{status(error instanceof Error?error.message:String(error),true);throw error});chain=attempt.catch(()=>{});return attempt}
  window.brainanaAlignSaveBlob = (blob,filename) => queued([{blob,filename}])
  window.brainanaAlignSaveBlob.artifacts = queued
}
