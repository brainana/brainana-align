import assert from 'node:assert/strict'

const methods=[['linear-registration','Linear registration','auto-linear'],['jip-registration','JIP nonlinear registration','jip-run'],['itk-registration','ITK nonlinear B-spline','itk-run']]
export function assertRegistrationMethodStructure(document){
 const panels=methods.map(([id,label,run])=>{
  const panel=document.getElementById(id);assert(panel,id);assert.equal(panel.tagName,'DETAILS',id+' is a disclosure');assert(panel.classList.contains('nested')&&panel.classList.contains('registration-method'),id+' uses the shared peer-method presentation');assert.equal(panel.querySelector(':scope > summary')?.textContent.trim(),label);assert.equal(panel.open,false,id+' starts collapsed');assert.equal(panel.hasAttribute('open'),false,id+' has no initial open attribute');assert(panel.contains(document.getElementById(run)),run+' stays inside its own method');return panel
 })
 const parent=panels[0].parentElement
 for(const panel of panels)assert.equal(panel.parentElement,parent,'All three registration methods are peers')
 for(let i=0;i<panels.length;i++)for(let j=0;j<panels.length;j++)if(i!==j)assert(!panels[i].contains(panels[j]),'Registration methods cannot contain another method')
 assert(panels[0].compareDocumentPosition(panels[1])&4);assert(panels[1].compareDocumentPosition(panels[2])&4)
 const acquisition=document.getElementById('acquisition-details');assert(acquisition);assert.equal(acquisition.parentElement,parent,'Acquisition controls are shared above the method peers');assert(acquisition.compareDocumentPosition(panels[0])&4)
 for(const id of ['linear-defaults-select','linear-defaults-apply','auto-position','auto-histogram-rule','auto-density','auto-sample-fraction','auto-smoothing','auto-sigmas','auto-search-mode','auto-foreground','auto-normalization','auto-prepared-settings','auto-linear-model','auto-custom','auto-metric','registration-sampling','registration-seed','auto-initialization','auto-range','auto-bins','auto-pyramid','registration-spacing','registration-samples'])assert(panels[0].contains(document.getElementById(id)),id+' stays in Linear registration')
 assert(document.getElementById('auto-linear-model').closest('label').textContent.includes('Transform model'),'Transform model stays a label within Linear registration')
 assert.equal(document.getElementById('auto-linear').textContent.trim(),'Run linear registration')
 assert.deepEqual([...document.getElementById('auto-linear-model').options].map(option=>option.value),['rigid','similarity','anisotropic','affine','custom'])
 for(const id of ['itk-stages','itk-model','itk-iterations'])assert.equal(document.getElementById(id),null,'No duplicate linear controls in ITK');
 return panels
}

export async function exerciseRegistrationMethodDisclosures(document,{requests=()=>0,state=()=>null}={}){
 const panels=methods.map(([id])=>document.getElementById(id)),beforeRequests=requests(),beforeState=state()
 const controls=()=>panels.flatMap(panel=>[...panel.querySelectorAll('input,select')].map(input=>[input.id,input.value,input.checked??null,input.disabled]))
 const beforeControls=controls()
 for(const panel of panels){
  const prior=panels.map(other=>other.open);panel.querySelector(':scope > summary').click();await new Promise(resolve=>setTimeout(resolve,0));assert.equal(panel.open,!prior[panels.indexOf(panel)]);for(let i=0;i<panels.length;i++)if(panels[i]!==panel)assert.equal(panels[i].open,prior[i],'Opening one method preserves its peers');panel.querySelector(':scope > summary').click();await new Promise(resolve=>setTimeout(resolve,0));assert.deepEqual(panels.map(other=>other.open),prior)
 }
 assert.deepEqual(controls(),beforeControls,'Disclosure clicks preserve existing settings and run gates');assert.equal(requests(),beforeRequests,'Opening a method never starts registration');assert.equal(state(),beforeState,'Opening a method never replaces or accepts the alignment')
}
