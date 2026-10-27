import {authorizedFetch,requestJson} from './runtimeClient'
import {imageLabel} from './registrationPairing'
import type {LoadFiles,Modality,Status} from './workstationBrowser'
import type {ServerList} from './filesystemClient'
export function createNetworkFileBrowser(loadFiles:LoadFiles,setStatus:Status) {
  const modal=document.createElement('div');modal.id='network-browser-modal';modal.className='modal hidden';modal.setAttribute('role','dialog');modal.setAttribute('aria-modal','true');modal.setAttribute('aria-labelledby','network-title')
  modal.innerHTML=`<div class="modal-card" style="width:min(740px,94vw);max-height:90vh;overflow:auto"><div class="modal-head"><h2 id="network-title">Network files</h2><button id="network-close" aria-label="Close">×</button></div><form id="network-connection"><p>Connect to a workstation, then browse its files. Your current workspace stays open.</p><label>SSH configuration <select id="network-profile"><option value="">Enter connection details</option></select></label><div style="display:grid;grid-template-columns:1fr 1fr;gap:12px"><label>Host<input id="network-host" required autocomplete="off"></label><label>Username<input id="network-user" required autocomplete="username"></label><label>Port<input id="network-port" type="number" min="1" max="65535" value="22" required></label><label>Starting folder<input id="network-start" value="." placeholder="Home folder"></label><label>Password (optional)<input id="network-password" type="password" autocomplete="off"></label><label>Private key path (optional)<input id="network-key" placeholder="~/.ssh/id_ed25519" autocomplete="off"></label><label>Key passphrase (optional)<input id="network-passphrase" type="password" autocomplete="off"></label></div><p>Leave password and key blank to use your SSH agent. Host identity is checked against known_hosts. Passwords and passphrases are not saved.</p><button id="network-connect" type="submit" class="primary">Connect and browse</button></form><section id="network-files" class="hidden"><div style="display:flex;gap:8px;align-items:center"><button id="network-up">Up</button><input id="network-path" aria-label="Remote folder" style="flex:1"><button id="network-go">Go</button><button id="network-disconnect">Change connection</button></div><div id="network-list" style="min-height:180px;max-height:45vh;overflow:auto"></div><button id="network-load" class="primary" disabled>Load selected image</button></section><p id="network-message" role="status" aria-live="polite"></p></div>`
  document.body.append(modal)
  const q=<T extends HTMLElement>(id:string)=>modal.querySelector<T>('#network-'+id)!
  const value=(id:string)=>q<HTMLInputElement>(id).value
  let token='',directory='.',parent:string|null=null,modality:Modality='mri',label='',busy=false,generation=0,controller:AbortController|null=null
  const headers=()=>({'x-brainana-remote-token':token})
  const error=(e:unknown)=>{q('message').textContent=e instanceof Error?e.message:String(e)}
  const setBusy=(active:boolean)=>{busy=active;for(const id of ['connect','load','go','up','disconnect'])q<HTMLButtonElement>(id).disabled=active;if(!active){q<HTMLButtonElement>('load').disabled=!q('list').querySelector('input:checked');q<HTMLButtonElement>('up').disabled=parent===null}}
  const showFiles=()=>{q('connection').classList.toggle('hidden',!!token);q('files').classList.toggle('hidden',!token);q('title').textContent=`Load ${imageLabel(modality)} · ${label||'network workstation'}`}
  async function list(folder:string){
    const current=++generation;setBusy(true);q('message').textContent='Reading network folder…'
    try {
      const result=await requestJson<ServerList>('/api/network/list?path='+encodeURIComponent(folder),{headers:headers()})
      if(current!==generation)return
      directory=result.path;parent=result.parent;q<HTMLInputElement>('path').value=directory;q('list').replaceChildren()
      for(const entry of result.entries){
        const row=document.createElement('div');row.style.padding='8px';row.style.borderBottom='1px solid var(--border-2)'
        if(entry.directory){const button=document.createElement('button');button.type='button';button.textContent='📁 '+entry.name;button.addEventListener('click',()=>{if(!busy)void list(entry.path)});row.append(button)}
        else {const item=document.createElement('label'),check=document.createElement('input');check.type='checkbox';check.value=entry.path;check.addEventListener('change',()=>{q<HTMLButtonElement>('load').disabled=!q('list').querySelector('input:checked')});item.append(check,document.createTextNode(' '+entry.name));row.append(item)}
        q('list').append(row)
      }
      if(!result.entries.length)q('list').textContent='No supported images or folders in this location.'
      q('message').textContent='Select an image, or both files for a paired format.'
    }catch(e){if(current===generation)error(e)}finally{if(current===generation)setBusy(false)}
  }
  const disconnect=async()=>{const previous=token;token='';directory='.';parent=null;showFiles();if(previous)await requestJson('/api/network/disconnect',{method:'POST',headers:{'x-brainana-remote-token':previous}})}
  const close=()=>{generation++;controller?.abort();modal.classList.add('hidden');setBusy(false)}
  q('close').addEventListener('click',close);modal.addEventListener('click',e=>{if(e.target===modal)close()});modal.addEventListener('keydown',e=>{if(e.key==='Escape'){e.preventDefault();close()}if(e.key==='Tab'){const fields=[...modal.querySelectorAll<HTMLInputElement>('button,input,select')].filter(el=>!el.disabled&&!el.closest('.hidden'));const first=fields[0],last=fields.at(-1);if(e.shiftKey&&document.activeElement===first){e.preventDefault();last?.focus()}else if(!e.shiftKey&&document.activeElement===last){e.preventDefault();first?.focus()}}})
  q('connection').addEventListener('submit',async e=>{
    e.preventDefault();if(busy)return;setBusy(true);controller=new AbortController();q('message').textContent='Connecting…'
    try {
      const result=await requestJson<{token:string;path:string;label:string}>('/api/network/connect',{method:'POST',headers:{'content-type':'application/json'},signal:controller.signal,body:JSON.stringify({host:value('host'),username:value('user'),port:Number(value('port')),path:value('start'),password:value('password'),identityFile:value('key'),passphrase:value('passphrase')})})
      token=result.token;directory=result.path;label=result.label;showFiles();await list(directory)
    }catch(e){if(!controller?.signal.aborted)error(e)}finally{q<HTMLInputElement>('password').value='';q<HTMLInputElement>('passphrase').value='';controller=null;setBusy(false)}
  })
  q('disconnect').addEventListener('click',()=>{if(!busy)void disconnect().catch(error)})
  q('up').addEventListener('click',()=>{if(!busy&&parent)void list(parent)})
  q('go').addEventListener('click',()=>{if(!busy)void list(value('path'))})
  q('path').addEventListener('keydown',e=>{if(e.key==='Enter'){e.preventDefault();if(!busy)void list(value('path'))}})
  q('load').addEventListener('click',async()=>{
    if(busy)return
    const paths=[...q('list').querySelectorAll<HTMLInputElement>('input:checked')].map(e=>e.value);if(!paths.length)return
    setBusy(true);controller=new AbortController()
    try {
      const files:File[]=[]
      for(const filename of paths){q('message').textContent='Loading '+filename.split('/').pop()+'…';const response=await authorizedFetch('/api/network/file?path='+encodeURIComponent(filename),{headers:headers(),signal:controller.signal});if(!response.ok)throw Error((await response.json()).error||'Network download failed');const blob=await response.blob();files.push(new File([blob],filename.split('/').pop()||'volume',{type:'application/octet-stream'}))}
      if(controller.signal.aborted)return
      await loadFiles(modality,files);close()
    }catch(e){if(!controller?.signal.aborted){error(e);setStatus(e instanceof Error?e.message:String(e),true)}}finally{controller=null;setBusy(false)}
  })
  let profiles: {host:string;hostName?:string;user?:string;port?:number}[]=[]
  void requestJson<typeof profiles>('/api/network/profiles').then(items=>{if(!Array.isArray(items))return;profiles=items;for(const [i,p]of items.entries()){const option=document.createElement('option');option.value=String(i);option.textContent=p.host;q<HTMLSelectElement>('profile').append(option)}}).catch(()=>{})
  q('profile').addEventListener('change',()=>{const selected=value('profile');if(selected==='')return;const p=profiles[Number(selected)];if(p){q<HTMLInputElement>('host').value=p.hostName||p.host;q<HTMLInputElement>('user').value=p.user||'';q<HTMLInputElement>('port').value=String(p.port||22)}})
  return {async open(selected:Modality){modality=selected;showFiles();modal.classList.remove('hidden');if(token)await list(directory);else {q<HTMLInputElement>('host').focus();q('message').textContent=''}},close}
}
