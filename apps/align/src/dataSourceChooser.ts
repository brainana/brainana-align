import {imageLabel} from './registrationPairing'
import type {Modality} from './workstationBrowser'

type Connection = {remote: boolean; label?: string; canConnect: boolean}
export function installDataSourceChooser(options: {
  connection: () => Promise<Connection>
  browse: (modality: Modality) => Promise<void>
  connect: (modality: Modality) => Promise<unknown>
}): void {
  const modal = document.createElement('div')
  modal.id = 'data-source-modal'
  modal.className = 'modal hidden'
  modal.setAttribute('role', 'dialog')
  modal.setAttribute('aria-modal', 'true')
  modal.setAttribute('aria-labelledby', 'data-source-title')
  modal.innerHTML = `<div class="modal-card" style="width:min(560px,92vw)"><div class="modal-head"><h2 id="data-source-title">Load image</h2><button id="data-source-close" aria-label="Close">×</button></div><p>Choose where to load this image.</p><button id="data-source-local" class="primary" style="width:100%">Local computer / mounted network drive</button><p>Choose a file on this computer, including shared drives mounted in Finder or your system file browser.</p><button id="data-source-network" style="width:100%">Network workstation · SSH / SFTP</button><p id="data-source-description"></p><p id="data-source-message" role="status" aria-live="polite"></p></div>`
  document.body.append(modal)
  let modality: Modality = 'mri', opener: HTMLInputElement | null = null, bypass = false, busy = false, generation = 0
  const local = modal.querySelector<HTMLButtonElement>('#data-source-local')!
  const network = modal.querySelector<HTMLButtonElement>('#data-source-network')!
  const message = modal.querySelector<HTMLElement>('#data-source-message')!
  let connection: Connection | null = null
  const close = (): void => { if (busy) return; generation++; modal.classList.add('hidden'); opener?.closest('label')?.focus() }
  const show = async (input: HTMLInputElement, selected: Modality): Promise<void> => {
    if (busy) return
    const request = ++generation
    modality = selected; opener = input; connection = null
    modal.querySelector<HTMLElement>('#data-source-title')!.textContent = `Load ${imageLabel(modality)}`
    message.textContent = 'Checking available data sources…'; network.disabled = true
    modal.classList.remove('hidden'); local.focus()
    try {
      const result = await options.connection()
      if (request !== generation) return
      connection = result; network.disabled = false; message.textContent = ''
      network.textContent = result.remote ? 'Browse connected workstation' : 'Network workstation · SSH / SFTP'
      modal.querySelector<HTMLElement>('#data-source-description')!.textContent = result.remote
        ? `Browse files on the connected workstation${result.label ? ': '+result.label : ''}. Local files can also be selected without changing this connection.`
        : result.canConnect
          ? 'Browse a workstation over SSH/SFTP inside Align. Your images, landmarks and registration stay in this workspace.'
          : 'Network browsing is unavailable in this runtime. Mounted shares remain available through the local file picker.'
    } catch { if (request === generation) message.textContent = 'Could not check network access. Local files are still available; close and reopen to retry.' }
  }
  for (const selected of ['mri','ct'] as Modality[]) {
    const input = document.querySelector<HTMLInputElement>(`#${selected}-file`)
    input?.addEventListener('click', event => {
      if (bypass) return
      event.preventDefault(); event.stopImmediatePropagation()
      void show(input, selected)
    }, true)
  }
  local.addEventListener('click', () => {
    if (busy || !opener) return
    const input = opener; close(); bypass = true
    try { input.click() } finally { bypass = false }
  })
  network.addEventListener('click', async () => {
    if (busy || !connection) return
    if (!connection.remote && !connection.canConnect) { message.textContent = 'Use the local option for mounted network shares, or use the packaged macOS app for SSH/SFTP.'; return }
    busy = true; local.disabled = network.disabled = true; message.textContent = connection.remote ? 'Opening workstation files…' : 'Opening workstation connection…'
    try {
      if (connection.remote) { await options.browse(modality); busy = false; close() }
      else { busy = false; close(); await options.connect(modality) }
    } catch (error) { message.textContent = error instanceof Error ? error.message : String(error) }
    finally { busy = false; local.disabled = network.disabled = false }
  })
  modal.querySelector('#data-source-close')!.addEventListener('click', close)
  modal.addEventListener('click', event => { if (event.target === modal) close() })
  modal.addEventListener('keydown', event => {
    if (event.key === 'Escape') { event.preventDefault(); close() }
    if (event.key === 'Tab') {
      const buttons = [...modal.querySelectorAll<HTMLButtonElement>('button')].filter(button => !button.disabled)
      const first = buttons[0], last = buttons.at(-1)
      if (event.shiftKey && document.activeElement === first) { event.preventDefault(); last?.focus() }
      else if (!event.shiftKey && document.activeElement === last) { event.preventDefault(); first?.focus() }
    }
  })
}
