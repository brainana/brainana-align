/** Guide-authored paragraphs and emphasis only; never interpret HTML. */
export function renderGuideText(target:HTMLElement,text:string){
 const doc=target.ownerDocument
 target.replaceChildren()
 for(const [index,paragraph] of text.split(/\n\s*\n/).entries()){
  if(index)target.append(doc.createTextNode('\n\n'))
  const p=doc.createElement('p')
  for(const part of paragraph.split(/(\*\*[^*]+\*\*)/g)){
   if(part.startsWith('**')&&part.endsWith('**')&&part.length>4){const strong=doc.createElement('strong');strong.textContent=part.slice(2,-2);p.append(strong)}
   else p.append(doc.createTextNode(part))
  }
  target.append(p)
 }
}
