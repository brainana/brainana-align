import {imageLabel} from './registrationPairing'
import type {FitState,Loaded} from './appTypes'
import {multiplyMat4} from './rigid'
import type {RawNifti,Dims3} from './roiWarp'
export type Interpolation='nearest'|'linear'|'cubic'
export type AlignedExportOptions={data:'native'|'native-processed'|'processed';fov:'reference-native'|'reference-working'|'moving-native'|'moving-working';resolution:'moving-native'|'reference-native'|'working'|'custom';spacing:Dims3;interpolation:Interpolation}
export type ExportGrid=Pick<RawNifti,'dims'|'pixDims'|'affine'>
export function originalGrid(item:Loaded):ExportGrid{const n=item.native?.info;return n?{dims:n.dims,pixDims:n.spacing,affine:n.affine}:(item.processing?.base??item.raw)}
export function alignedExportGrid(source:Loaded,target:Loaded,fit:FitState,o:AlignedExportOptions):ExportGrid{
 if(!['native','native-processed','processed'].includes(o.data)||!['reference-native','reference-working','moving-native','moving-working'].includes(o.fov)||!['moving-native','reference-native','working','custom'].includes(o.resolution)||!['nearest','linear','cubic'].includes(o.interpolation))throw Error('Invalid aligned export options')
 const native=originalGrid(source),reference=originalGrid(target)
 if(o.fov.startsWith('moving-')&&fit.nonlinear)throw Error('Moving-image FoV requires a linear transform. For a nonlinear spline choose full native reference FoV or working reference FoV.')
 const base=o.fov==='reference-native'?reference:o.fov==='reference-working'?target.raw:{...(o.fov==='moving-working'?source.raw:native),affine:multiplyMat4(fit.matrix,o.fov==='moving-working'?source.raw.affine:native.affine)}
 const lengths=[0,1,2].map(a=>Math.hypot(...base.affine.slice(0,3).map(r=>r[a])))
 const wanted=o.resolution==='moving-native'?native.pixDims:o.resolution==='reference-native'?reference.pixDims:o.resolution==='working'?target.raw.pixDims:o.spacing
 if(!wanted.every(v=>Number.isFinite(v)&&v>=.01&&v<=100))throw Error('Export spacing must be 0.01–100 mm on every axis')
 const dims=base.dims.map((n,a)=>n===1?1:Math.max(2,Math.ceil((n-1)*lengths[a]/wanted[a]-1e-9)+1)) as Dims3
 if(dims.reduce((a,b)=>a*b,1)*4>6*1024**3)throw Error('Output grid exceeds the 6 GiB uncompressed export limit; choose coarser spacing')
 const affine=base.affine.map(r=>[...r]),pixDims=lengths.map((v,a)=>base.dims[a]===1?wanted[a]:v*(base.dims[a]-1)/(dims[a]-1)) as Dims3
 for(let a=0;a<3;a++)for(let r=0;r<3;r++)affine[r][a]*=pixDims[a]/lengths[a]
 return {dims,pixDims,affine}
}
export function readAlignedExportOptions():AlignedExportOptions{const value=(id:string)=>(document.querySelector<HTMLSelectElement>('#'+id)!).value;return {data:value('aligned-image-source') as AlignedExportOptions['data'],fov:value('export-fov') as AlignedExportOptions['fov'],resolution:value('export-resolution') as AlignedExportOptions['resolution'],spacing:[0,1,2].map(a=>Number((document.querySelector<HTMLInputElement>('#export-spacing-'+a)!).value)) as Dims3,interpolation:value('export-interpolation') as Interpolation}}
export function installAlignedExportOptions(read:()=>{fit:FitState|null;source:Loaded|null;target:Loaded|null}){
 const ids=['aligned-image-source','export-fov','export-resolution','export-interpolation',...Array.from({length:3},(_,a)=>'export-spacing-'+a)]
 function refresh(){const s=read(),custom=readAlignedExportOptions().resolution==='custom';for(let a=0;a<3;a++)(document.querySelector<HTMLInputElement>('#export-spacing-'+a)!).disabled=!custom
  const summary=document.querySelector<HTMLElement>('#aligned-export-summary')!;if(!s.fit||!s.source||!s.target){summary.textContent='Accept an alignment and load both images to export a transformed moving image.';return}
  document.querySelectorAll<HTMLOptionElement>('#export-fov option[value^="moving-"]').forEach(option=>option.disabled=Boolean(s.fit!.nonlinear))
  try{const o=readAlignedExportOptions(),g=alignedExportGrid(s.source,s.target,s.fit,o),moving=imageLabel(s.fit.direction==='ct-mri'?'ct':'mri'),fixed=imageLabel(s.fit.direction==='ct-mri'?'mri':'ct');summary.textContent=`${moving} transformed into ${fixed} physical space · ${g.dims.join(' × ')} voxels · ${g.pixDims.map(v=>v.toFixed(3)).join(' × ')} mm · ${(g.dims.reduce((a,b)=>a*b,1)*4/1024**2).toFixed(1)} MiB uncompressed. ${o.data==='native-processed'?'Crop, applied mask and voxel edits are projected from the working grid onto native intensities.':o.data==='processed'?'Uses the current processed working voxels.':'Uses original native intensities; no preprocessing applied.'} ${s.source.native?.streamable?'':'Native reload is unavailable for this format; original working voxels are used. '}Display adjustments are never exported.${o.interpolation==='cubic'?' Cubic interpolation may overshoot intensity bounds.':''}`;summary.classList.remove('error')}catch(e){summary.textContent=e instanceof Error?e.message:String(e);summary.classList.add('error')}
 }
 document.querySelectorAll<HTMLButtonElement>('[data-export-factor]').forEach(b=>b.addEventListener('click',()=>{const s=read();if(!s.source)return;const spacing=originalGrid(s.source).pixDims;document.querySelector<HTMLSelectElement>('#export-resolution')!.value='custom';spacing.forEach((v,a)=>document.querySelector<HTMLInputElement>('#export-spacing-'+a)!.value=String(v*Number(b.dataset.exportFactor)));refresh()}))
 ids.forEach(id=>document.querySelector('#'+id)!.addEventListener('input',refresh));return {refresh}
}
