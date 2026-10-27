import assert from 'node:assert/strict'
import {JSDOM} from 'jsdom'
import {createNetworkFileBrowser} from '../apps/align/src/networkFileBrowser'
const dom=new JSDOM('',{url:'http://localhost'});Object.assign(globalThis,{window:dom.window,document:dom.window.document,File:dom.window.File})
const calls:{url:string;init?:RequestInit}[]=[];const loads:any[]=[]
let fail=false
const originalFetch=globalThis.fetch
globalThis.fetch=async(input,init)=>{const url=String(input);calls.push({url,init});if(url.includes('/profiles'))return Response.json([{host:'lab',hostName:'host.example',user:'user',port:2222}]);if(url.includes('/connect'))return fail?Response.json({error:'Authentication failed'},{status:400}):Response.json({token:'private-token',path:'/home/user',label:'user@host.example'});if(url.includes('/list'))return Response.json({path:'/home/user',parent:'/home',entries:[{name:'image.nii.gz',path:'/home/user/image.nii.gz',directory:false}]});if(url.includes('/file'))return new Response(new Uint8Array([1,2,3]));return Response.json({ok:true})}
const browser=createNetworkFileBrowser(async(m,files)=>{loads.push({m,files})},()=>{})
const $=(id:string)=>document.getElementById('network-'+id) as HTMLInputElement
const tick=()=>new Promise(r=>setTimeout(r,15))
try {
 await browser.open('ct');await tick();assert.match($('title').textContent!,/CT/)
 $('profile').value='0';$('profile').dispatchEvent(new dom.window.Event('change'));assert.equal($('host').value,'host.example');assert.equal($('port').value,'2222')
 $('password').value='secret';$('connection').dispatchEvent(new dom.window.Event('submit',{cancelable:true}));await tick();assert.equal($('password').value,'');assert.equal($('files').classList.contains('hidden'),false)
 const checkbox=$('list').querySelector<HTMLInputElement>('input')!;checkbox.checked=true;checkbox.dispatchEvent(new dom.window.Event('change'));assert.equal(($('load') as unknown as HTMLButtonElement).disabled,false)
 $('load').click();await tick();assert.equal(loads.length,1);assert.equal(loads[0].m,'ct');assert.equal(loads[0].files[0].name,'image.nii.gz');assert(document.querySelector('#network-browser-modal')!.classList.contains('hidden'))
 await browser.open('mri');assert.match($('title').textContent!,/MRI/);assert.equal(calls.filter(c=>c.url.includes('/connect')).length,1,'Reuse connection without new authentication')
 $('disconnect').click();await tick();fail=true;$('password').value='secret';$('connection').dispatchEvent(new dom.window.Event('submit',{cancelable:true}));await tick();assert.match($('message').textContent!,/Authentication failed/);assert.equal($('password').value,'');assert.equal(loads.length,1)
 for(const call of calls.filter(c=>c.url.includes('/list')||c.url.includes('/file'))) {assert(!call.url.includes('private-token'));assert.equal(new Headers(call.init!.headers).get('x-brainana-remote-token'),'private-token')}
 browser.close()
}finally{globalThis.fetch=originalFetch;dom.window.close()}
console.log('Network UI: profile recall, connect/browse/load, selected image slot, connection reuse, credential clearing, errors and header-only tokens passed.')
