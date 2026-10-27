type Inspection={mode:HTMLSelectElement;flip:HTMLButtonElement;label:HTMLElement;show:(image:'fixed'|'moving')=>void;names:()=>{fixed:string;moving:string};available:()=>boolean}
/** Launch-local inspection: one RAF loop; flip changes only on one-second boundaries. */
export function installOpacityCycle(slider:HTMLInputElement,button:HTMLButtonElement,update:()=>void,inspection?:Inspection){
 let frame:number|null=null,start:number|null=null,shown:'fixed'|'moving'='fixed',initial:'fixed'|'moving'='fixed',phase=-1
 const flipping=()=>inspection?.mode.value==='flip'
 const buttonLabel=()=>flipping()?'Auto flip · 1 s':'Animate opacity · 25–75%'
 const stop=()=>{if(frame!==null)cancelAnimationFrame(frame);frame=null;start=null;phase=-1;button.setAttribute('aria-pressed','false');button.textContent=buttonLabel()}
 const refresh=()=>{
  if(!inspection)return
  const flip=flipping(),available=inspection.available(),names=inspection.names()
  slider.disabled=Boolean(flip);inspection.flip.classList.toggle('hidden',!flip);inspection.label.classList.toggle('hidden',!flip)
  inspection.flip.disabled=!available;button.disabled=!available
  inspection.flip.textContent=`Show ${names[shown==='fixed'?'moving':'fixed']}`
  inspection.label.textContent=available?`${names[shown]} only (${shown})`:'Create an aligned overlay to compare images.'
  if(flip&&available)inspection.show(shown)
 }
 const tick=(time:number)=>{
  if(document.hidden||(inspection&&!inspection.available())){stop();refresh();return}
  start??=time
  if(flipping()){
   const next=Math.floor((time-start)/1000)
   if(next!==phase){phase=next;shown=next%2===0?initial:(initial==='fixed'?'moving':'fixed');refresh()}
  }else{slider.value=String(50-25*Math.cos((time-start)*Math.PI*1.4/4000));update()}
  frame=requestAnimationFrame(tick)
 }
 button.addEventListener('click',()=>{if(frame!==null){stop();return}if(inspection&&!inspection.available())return;initial=shown;phase=-1;button.setAttribute('aria-pressed','true');button.textContent=flipping()?'Stop automatic flipping':'Stop opacity animation';frame=requestAnimationFrame(tick)})
 inspection?.mode.addEventListener('change',()=>{stop();refresh();if(!flipping())update()})
 inspection?.flip.addEventListener('click',()=>{if(!inspection.available())return;stop();shown=shown==='fixed'?'moving':'fixed';refresh()})
 slider.addEventListener('input',stop);document.addEventListener('visibilitychange',()=>{if(document.hidden)stop()});window.addEventListener('align-workspace-reset',stop);window.addEventListener('pagehide',stop)
 refresh()
 return {stop,refresh}
}
