import {getPreferences} from './preferences'
import { REVIEW_BLEND_SHADER } from './reviewBlendShader'
import type { Niivue } from '@niivue/niivue'

const shaderReady=new WeakMap<Niivue,boolean>()

/** A single moving overlay: keep textures at unit alpha and blend in the slice shader. */
export function updateReviewOpacity(viewers: Niivue[], value: number): void {
  const alpha=Math.max(0,Math.min(1,value))
  for(const nv of viewers) {
    if(nv.volumes.length<2)continue
    if(shaderReady.get(nv)){nv.overlayAlphaShader=alpha}else{nv.overlayAlphaShader=1;nv.volumes[0].opacity=1;nv.volumes[1].opacity=alpha;nv.updateGLVolume()}
    nv.drawScene()
  }
}
export function configureReviewAppearance(viewers: Niivue[], mode: string, alpha: number): void {
  for(const nv of viewers) {
    if(nv.volumes.length<2)continue
    if(!shaderReady.has(nv)){
      try{nv.setCustomSliceShader(REVIEW_BLEND_SHADER);shaderReady.set(nv,true)}
      catch(error){nv.setCustomSliceShader('');shaderReady.set(nv,false);console.warn('Review shader unavailable; retaining texture-based opacity.',error)}
    }
    const [fixed,moving]=nv.volumes
    fixed.opacity=1;moving.opacity=shaderReady.get(nv)?1:Math.max(0,Math.min(1,alpha))
    const colors=getPreferences().display
    const addTint=(name:string,color:string)=>{const rgb=[1,3,5].map(i=>parseInt(color.slice(i,i+2),16));nv.addColormap(name,{R:[0,Math.round(rgb[0]/2),rgb[0]],G:[0,Math.round(rgb[1]/2),rgb[1]],B:[0,Math.round(rgb[2]/2),rgb[2]],A:[0,64,128],I:[0,128,255]})}
    if(colors.fixedColor!=='#00ff00')addTint('brainanaFixed',colors.fixedColor)
    if(colors.movingColor!=='#ff00ff')addTint('brainanaMoving',colors.movingColor)
    fixed.colormap=mode==='falsecolor'?(colors.fixedColor==='#00ff00'?'green':'brainanaFixed'):'gray'
    moving.colormap=mode==='falsecolor'?(colors.movingColor==='#ff00ff'?'brainanaMagenta':'brainanaMoving'):'gray' 
    nv.overlayAlphaShader=shaderReady.get(nv)?Math.max(0,Math.min(1,alpha)):1
    nv.updateGLVolume()
  }
}

/** Pure-image inspection uses the existing textures and shared review grid. */
export function updateReviewFlip(viewers:Niivue[],image:'fixed'|'moving'):void{
  for(const nv of viewers){
    if(nv.volumes.length<2)continue
    if(shaderReady.get(nv))nv.overlayAlphaShader=image==='fixed'?-1:2
    else{nv.volumes[0].opacity=image==='fixed'?1:0;nv.volumes[1].opacity=image==='moving'?1:0;nv.overlayAlphaShader=1;nv.updateGLVolume()}
    nv.drawScene()
  }
}
