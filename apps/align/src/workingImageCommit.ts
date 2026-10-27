import type {Loaded} from './appTypes'

/** Saving a binary selection changes metadata only: preserve the live source
 * renderer, its texture, zoom and pan. Reupload only when image storage changes. */
export function sourceViewerNeedsReload(previous:Loaded|null|undefined,next:Loaded):boolean{
 return !previous||previous.raw!==next.raw||previous.nvImage!==next.nvImage
}

/** Preprocessing updates source voxels only. Never refit or resample an overlay as
 * a side effect: the user may explicitly refresh the existing transform later. */
export async function finishWorkingImageCommit(reason:'preparation'|'processing',changed:unknown[],hasFit:boolean,actions:{clearFit:()=>void;refreshOverlay:()=>Promise<void>;invalidateOverlay:()=>void}){
 if(changed.length)actions.clearFit()
 else if(hasFit){if(reason==='processing')actions.invalidateOverlay();else await actions.refreshOverlay()}
}
