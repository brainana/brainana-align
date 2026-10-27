import {cloneFitState} from './fitSnapshot'
import {imageLabel} from './registrationPairing'
import type {NVImage} from '@niivue/niivue'
import type {FitState,Loaded,Modality,Plane,View} from './appTypes'
import type {Vec3} from './rigid'
import {registrationPoint} from './manualAffine'
import {sharedViewerImage} from './volumeImages'
export type SourcePreviewState={modality:Modality;reference:Modality;image:NVImage;fit:FitState;pan:Record<Plane,number[]>}
export function installManualSourcePreview(o:{loaded:(m:Modality)=>Loaded|null;views:(m:Modality)=>View[];current:(m:Modality)=>Vec3|null;gotoNative:(m:Modality,p:Vec3)=>void;state:(s:SourcePreviewState|null)=>void;beforeShow:()=>void;afterShow:()=>void}){
 let current:SourcePreviewState|null=null
 function clear(){const previous=current;if(!previous)return;current=null;o.state(null);const item=o.loaded(previous.modality)
  for(const view of o.views(previous.modality)){view.canvas.closest('.view-card')?.classList.remove('source-transform-preview');while(view.nv.volumes.length)view.nv.removeVolumeByIndex(0);if(item)view.nv.addVolume(sharedViewerImage(item.nvImage));view.nv.scene.pan2Dxyzmm=[...previous.pan[view.plane]] as any}
  const head=document.querySelector(`#${previous.modality}-coords`)?.parentElement?.querySelector('strong');if(head){head.textContent=imageLabel(previous.modality);head.removeAttribute('title')}
  const point=o.current(previous.modality);if(item&&point)o.gotoNative(previous.modality,point)
 }
 function show(modality:Modality,reference:Modality,image:NVImage,fit:FitState){const same=current?.modality===modality&&current.reference===reference,pan=same?current!.pan:Object.fromEntries(o.views(modality).map(v=>[v.plane,[...v.nv.scene.pan2Dxyzmm]])) as Record<Plane,number[]>
  const reuse=same&&current?.image.img===image.img;
  if(!same)clear();current={modality,reference,image,fit:cloneFitState(fit),pan};o.state(current);o.beforeShow()
  for(const view of o.views(modality)){const camera=[...view.nv.scene.pan2Dxyzmm];view.canvas.closest('.view-card')?.classList.add('source-transform-preview');if(reuse&&view.nv.volumes.length===1)view.nv.setVolumeAffine(0,image.hdr!.affine);else{while(view.nv.volumes.length)view.nv.removeVolumeByIndex(0);view.nv.addVolume(sharedViewerImage(image))}view.nv.scene.pan2Dxyzmm=(same?camera:[0,0,0,1]) as any}
  const head=document.querySelector(`#${modality}-coords`)?.parentElement?.querySelector('strong');if(head){head.textContent=`${imageLabel(modality)} · preview`;head.setAttribute('title',`Transformed preview in ${imageLabel(reference)} space; original data unchanged`)}
  const point=o.current(reference);if(point)o.gotoNative(modality,registrationPoint(fit,reference,modality,point));o.afterShow()
 }
 return {clear,show,get current(){return current}}
}
