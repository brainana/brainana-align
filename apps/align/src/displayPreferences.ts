import type {Niivue} from '@niivue/niivue'
import {hexRGBA,type Preferences} from './preferences'
import {updateReviewOpacity} from './reviewAppearance'
type Display=Preferences['display']
export function applyDisplayPreferences(options:{mri:Niivue[];ct:Niivue[];review:Niivue[];mode:HTMLSelectElement;opacity:HTMLInputElement;opacityLabel:HTMLElement;appearance:()=>void},next:Display,previous?:Display){
 const colorsChanged=!previous||(['mode','fixedColor','movingColor'] as const).some(key=>next[key]!==previous[key])
 const opacityChanged=!previous||next.opacity!==previous.opacity
 if(colorsChanged){options.mode.value=next.mode;options.opacity.value=String(next.opacity);options.opacityLabel.textContent=`${next.opacity}%`;options.appearance()}
 else if(opacityChanged){options.opacity.value=String(next.opacity);options.opacityLabel.textContent=`${next.opacity}%`;updateReviewOpacity(options.review,next.opacity/100)}
 const crosshairsChanged=!previous||(['mriCrosshair','ctCrosshair','crosshairVisible','crosshairWidth'] as const).some(key=>next[key]!==previous[key])
 if(crosshairsChanged){
  for(const [modality,viewers] of [['mri',options.mri],['ct',options.ct]] as const)for(const nv of viewers){nv.setCrosshairColor(hexRGBA(modality==='mri'?next.mriCrosshair:next.ctCrosshair,next.crosshairVisible?1:0));nv.opts.crosshairWidth=next.crosshairVisible?next.crosshairWidth:0;nv.drawScene()}
  for(const nv of options.review){nv.setCrosshairColor([1,1,1,next.crosshairVisible?.85:0]);nv.opts.crosshairWidth=next.crosshairVisible?next.crosshairWidth:0;nv.drawScene()}
 }
}
