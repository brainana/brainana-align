import assert from 'node:assert/strict'
// Simulated human interactions for pre-existing route/worker tests. Teaching-specific
// regressions assert the intermediate prompts and absence of side effects separately.
export function followGuidePanels(document,refresh){
 for(let i=0;i<20;i++){
  const key=document.querySelector('#guided-card')?.dataset.step||''
  if(!key.startsWith('teach-open-')&&!key.startsWith('teach-close-'))return
  const heading=document.querySelector('summary.guided-target');assert(heading,'Navigation must highlight a real heading')
  heading.click();refresh()
 }
 throw Error('Guide navigation did not settle')
}
export function userGuideAction(document,refresh,label){
 followGuidePanels(document,refresh)
 let requested=label,invoke=false,manual=false
 const aliases={'Add landmark pair':'Show Add landmark pair','Apply landmark transformation':'Show landmark alignment','Accept registration':'Show Accept control','Accept spline':'Show Accept control','Reject proposal':'Show Reject control','Accept and adjust manually':'Show Accept control','Reject and adjust manually':'Show Reject control'}
 if(aliases[label]){requested=aliases[label];invoke=true;manual=label.endsWith('adjust manually')}
 if(/^(Set|Replace) (MRI|CT) point/.test(label)){requested='Show '+label.split(' ').slice(0,2).join(' ')+' control';invoke=true}
 const button=[...document.querySelectorAll('#guided-actions button')].find(b=>b.textContent===requested);assert(button,label);button.click();refresh();followGuidePanels(document,refresh)
 if(!invoke&&label.startsWith('Continue to ')&&document.querySelector('#guided-card')?.dataset.step.startsWith('teach-control-'))invoke=true
 if(invoke){const control=document.querySelector('.guided-target');assert(control&&!control.closest('#guided-card')&&control.matches('button'),'Show must point to the actual main-program button');control.click();refresh();followGuidePanels(document,refresh)}
 if(manual){const labels=[...document.querySelectorAll('#guided-actions button')].map(b=>b.textContent);userGuideAction(document,refresh,labels.includes('Guide manual adjustment')?'Guide manual adjustment':labels.includes('Start with manual adjustment')?'Start with manual adjustment':'Use manual adjustment')}
}
