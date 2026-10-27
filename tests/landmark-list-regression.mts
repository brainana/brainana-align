import assert from 'node:assert/strict'
import {JSDOM} from 'jsdom'
import {installLandmarkList} from '../apps/align/src/landmarkList'
import {setRegistrationPairing} from '../apps/align/src/registrationPairing'
import type {Landmark} from '../apps/align/src/appTypes'
const dom=new JSDOM('<div id="pair-count"></div><div id="selected-landmark-summary"></div><div id="landmark-list"></div>')
Object.assign(globalThis,{document:dom.window.document})
let landmarks:Landmark[]=[{id:1,mri:[1,2,3],ct:null,enabled:true}],selected:number|null=1,events:any[]=[]
const render=installLandmarkList({getLandmarks:()=>landmarks,getSelected:()=>selected,select:id=>events.push(['select',id]),toggle:(id,on)=>{events.push(['toggle',id,on]);landmarks.find(l=>l.id===id)!.enabled=on},remove:id=>events.push(['delete',id]),setPoint:(id,m)=>events.push(['set',id,m]),go:(id,m)=>events.push(['go',id,m])})
render();const row=document.querySelector('.landmark-row')!,button=row.querySelector<HTMLButtonElement>('.set-ct')!;button.focus()
for(let i=0;i<1000;i++)render()
assert.equal(document.querySelector('.landmark-row'),row);assert.equal(document.activeElement,button)
// Identical restored values may reuse DOM, but actions must reach current state.
landmarks=structuredClone(landmarks);render();const check=row.querySelector<HTMLInputElement>('.enabled')!;check.checked=false;check.dispatchEvent(new dom.window.Event('change'));assert(!landmarks[0].enabled)
button.click();row.querySelector<HTMLButtonElement>('.go-mri')!.click();row.querySelector<HTMLButtonElement>('.select')!.click();row.querySelector<HTMLButtonElement>('.delete')!.click()
assert.deepEqual(events,[['toggle',1,false],['set',1,'ct'],['go',1,'mri'],['select',1],['delete',1]])
landmarks[0].ct=[4,5,6];landmarks[0].residual=.123;render();assert.notEqual(document.querySelector('.landmark-row'),row);assert.equal(document.querySelector('.set-ct')!.textContent,'Replace');assert(!document.querySelector<HTMLButtonElement>('.go-ct')!.disabled);assert(document.querySelector('#selected-landmark-summary')!.textContent!.includes('0.12 mm'))
setRegistrationPairing('t1-t2');render();assert.equal(document.querySelector('.point-row span')!.textContent,'T1');assert.equal(document.querySelectorAll('.point-row span')[1].textContent,'T2');setRegistrationPairing('mri-ct')
selected=null;landmarks=[];render();assert.equal(document.querySelector('#pair-count')!.textContent,'0 pairs');assert.equal(document.querySelector('#selected-landmark-summary')!.textContent,'No landmark selected');assert(document.querySelector('.empty'));dom.window.close()
console.log('Landmark list: 1,000 unchanged refreshes retain row/focus, restored model callbacks stay current, point/residual/label/selection/deletion updates passed.')
