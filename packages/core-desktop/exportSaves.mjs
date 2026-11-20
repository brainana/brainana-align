import fs from 'node:fs/promises'
import path from 'node:path'
import {ipcMain, dialog} from 'electron'
import {exportFilename, exportFilenames} from '@brainana/core-client/exportNames.mjs'

// IPC carries only same-origin Blob URLs. Chromium streams the existing Blob
// to the user-chosen file; large volumes never become an IPC ArrayBuffer.
export function installExportSaves({directory, window}) {
  let active = false
  ipcMain.handle('brainana-export-save', async (event, files) => {
    const win = window(), wc = win?.webContents, frame = event.senderFrame
    const trusted = () => win && !win.isDestroyed() && window() === win && event.sender === wc && frame === wc.mainFrame
    if (!trusted()) throw Error('Untrusted export sender')
    if (active) throw Error('Another save dialog is already open')
    if (!Array.isArray(files) || !files.length || files.length > 16) throw Error('Invalid export file group')
    const origin = new URL(wc.getURL()).origin
    for (const file of files) {
      exportFilename(file?.filename)
      if (typeof file.url !== 'string' || file.url.length > 4096 || !file.url.startsWith('blob:' + origin + '/') || new URL(file.url).origin !== origin) throw Error('Invalid export Blob URL')
    }
    active = true
    try {
      let folder = ''
      try { const saved = JSON.parse(await fs.readFile(path.join(directory, 'align-export.json'), 'utf8')); if (typeof saved?.folder === 'string' && (await fs.stat(saved.folder)).isDirectory()) folder = saved.folder } catch {}
      const choice = await dialog.showSaveDialog(win, {title:'Save file — choose filename and folder', buttonLabel:'Save', defaultPath:folder ? path.join(folder, files[0].filename) : files[0].filename, properties:['createDirectory', 'showOverwriteConfirmation']})
      if (choice.canceled || !choice.filePath) return {canceled:true}
      if (!trusted()) throw Error('The export window changed. Save again from the current workspace.')
      const names = exportFilenames(files.map(f => f.filename), path.basename(choice.filePath)), paths = names.map(name => path.join(path.dirname(choice.filePath), name))
      const existing = []
      for (const file of (paths[0] === choice.filePath ? paths.slice(1) : paths)) { try { await fs.lstat(file); existing.push(path.basename(file)) } catch (error) { if (error.code !== 'ENOENT') throw error } }
      if (existing.length) {
        const answer = await dialog.showMessageBox(win, {type:'question', message:'Replace existing files?', detail:existing.join('\n'), buttons:['Cancel','Replace'], defaultId:0, cancelId:0})
        if (answer.response !== 1) return {canceled:true}
      }
      for (let i = 0; i < files.length; i++) {
        if (!trusted()) throw Error('The export window changed. Save again from the current workspace.')
        await saveDownload(wc, files[i].url, paths[i])
      }
      // Remember only the containing folder. Failure to remember it must not
      // turn a successfully written image into a reported save failure.
      try { await fs.mkdir(directory, {recursive:true}); const file = path.join(directory,'align-export.json'); await fs.writeFile(file + '.save-tmp', JSON.stringify({folder:path.dirname(choice.filePath)}), {mode:0o600}); await fs.rename(file + '.save-tmp', file) } catch {}
      return {canceled:false, paths}
    } finally { active = false }
  })
}

function saveDownload(wc, url, output) {
  return new Promise((resolve, reject) => {
    let item, settled = false
    const session = wc.session
    const cleanup = () => { clearTimeout(timer); session.removeListener('will-download', begin); wc.removeListener('destroyed', closed); wc.removeListener('render-process-gone', closed); wc.removeListener('did-start-navigation', navigate) }
    const finish = error => { if (settled) return; settled = true; cleanup(); error ? reject(error) : resolve() }
    const closed = () => { finish(Error('Save cancelled because the window closed or reloaded.')); item?.cancel() }
    const navigate = (event, _url, inPlace, main) => { if (!(event.isInPlace ?? inPlace) && (event.isMainFrame ?? main)) closed() }
    const begin = (_event, download, contents) => {
      if (contents !== wc || download.getURL() !== url) return
      item = download; clearTimeout(timer); session.removeListener('will-download', begin)
      try {
        download.setSavePath(output)
        download.once('done', (_event, state) => finish(state === 'completed' ? null : Error(state === 'cancelled' ? 'Save cancelled.' : 'File save failed or was interrupted. Check the destination and available disk space.')))
      } catch (error) { finish(error); download.cancel() }
    }
    const timer = setTimeout(() => finish(Error('Unable to start saving the file. Please try again.')), 15000)
    timer.unref?.()
    session.on('will-download', begin); wc.once('destroyed', closed); wc.once('render-process-gone', closed); wc.on('did-start-navigation', navigate)
    try { wc.downloadURL(url) } catch (error) { finish(error) }
  })
}
