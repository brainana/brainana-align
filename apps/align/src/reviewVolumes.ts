import type {Niivue,NVImage} from '@niivue/niivue'
import {sharedViewerImage} from './volumeImages'
/** Retain the fixed viewer object/camera when its dataset and geometry are
 * unchanged. A target switch or processing replacement takes the full path. */
export function replaceReviewVolumes(nv:Niivue,fixed:NVImage,moving:NVImage,previousFixed:NVImage|null){
 if(previousFixed===fixed&&nv.volumes.length===2&&nv.volumes[0].img===fixed.img){nv.removeVolumeByIndex(1)}
 else {while(nv.volumes.length)nv.removeVolumeByIndex(0);nv.addVolume(sharedViewerImage(fixed))}
 nv.addVolume(sharedViewerImage(moving))
}
