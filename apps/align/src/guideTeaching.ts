import type {GuideStep} from './guidedSetup'
// Some advanced settings disclosures have no ID. Describe their real heading
// through its permanent parent, without adding IDs or opening any disclosure.
function disclosureSelector(panel:HTMLDetailsElement):string{
 const parts:string[]=[];let node:Element|null=panel
 while(node){
  if(node.id){parts.unshift('#'+node.id);break}
  const parent:Element|null=node.parentElement,index=parent?[...parent.children].indexOf(node)+1:1
  parts.unshift(node.tagName.toLowerCase()+`:nth-child(${index})`);node=parent
 }
 return parts.join(' > ')
}
/** Teach disclosure navigation without ever opening, closing or invoking an app control. */
export function guideNavigationStep(step:GuideStep):GuideStep {
  // Starting choices and loading prerequisites are not sidebar actions. Show
  // them immediately; teach panel changes after a concrete task is chosen.
  if(['choose','registration','welcome','mri','ct','prepare','working','viewer'].includes(step.key)||document.querySelector('.modal:not(.hidden)'))return step
  const target=document.querySelector<HTMLElement>(step.target)
  const sidebar=document.querySelector('aside')
  // At the completed review hub, teach closing visible method peers before
  // their outer section. Hidden child state alone never reopens an outer panel.
  const hubMethod=step.key==='review'&&step.target==='#guided-setup'?Array.from(sidebar?.querySelectorAll<HTMLDetailsElement>('details.registration-method[open]')??[]).find(panel=>{
   for(let node=panel.parentElement;node;node=node.parentElement)if(node instanceof window.HTMLDetailsElement&&!node.open)return false
   return true
  }):undefined
  const context=hubMethod??(step.key==='pairs'||step.key==='teach-add-pair'||step.key.startsWith('point-')||step.target.includes('data-landmark-id')?(target?.closest('#landmarks-panel')??document.querySelector('#landmarks-panel')):target)
  const chain:HTMLDetailsElement[]=[]
  for(let node=context;node;node=node.parentElement)if(node instanceof window.HTMLDetailsElement)chain.unshift(node)
  const other=sidebar?[...sidebar.querySelectorAll<HTMLDetailsElement>('details[open]')].find(panel=>!panel.parentElement?.closest('details')&&!chain.includes(panel)):null
  const heading=(panel:HTMLDetailsElement)=>(panel.querySelector(':scope > summary > span:first-child')??panel.querySelector(':scope > summary'))?.textContent?.trim().replace(/\s+/g,' ')||'panel'
  if(other?.id)return {key:'teach-close-'+other.id,target:'#'+other.id+' > summary',title:'Close '+heading(other),text:'In the left sidebar, click the highlighted '+heading(other)+' heading to collapse it. Your images and settings stay unchanged. Then the guide will show the next panel to open.'}
  const closed=chain.find(panel=>!panel.open)
  if(closed){const selector=disclosureSelector(closed);return {key:'teach-open-'+(closed.id||selector),target:selector+' > summary',title:'Open '+heading(closed),text:'Click the highlighted '+heading(closed)+' heading in the main program to expand it. The guide will then point to the control to use. Opening a panel does not run a tool.'}}
  if(hubMethod)return {key:'teach-close-'+hubMethod.id,target:'#'+hubMethod.id+' > summary',title:'Close '+heading(hubMethod),text:'Click the highlighted '+heading(hubMethod)+' heading to collapse this registration method. Its settings and your alignment stay unchanged. Then close the remaining section to return to the next-step choices.'}
  return step
}
