import assert from 'node:assert/strict'
import {JSDOM} from 'jsdom'
import {alignmentLayout} from '../apps/align/src/alignmentLayout'
import {installDisplayIntensity} from '../apps/align/src/displayIntensity'
const dom=new JSDOM('<main></main>',{pretendToBeVisual:true});Object.assign(globalThis,{window:dom.window,document:dom.window.document});document.querySelector('main')!.innerHTML=alignmentLayout('test')
const item=(min:number,max:number)=>({raw:{values:new Float32Array([min,0,max])},nvImage:{cal_min:min,cal_max:max,global_min:min,global_max:max}} as any)
let loaded:any={mri:item(0,100),ct:item(-1000,2000)}
const all:any={mri:[0,1,2,3,4,5].map(()=>({nv:{volumes:[{cal_min:0,cal_max:1}],updateGLVolume(){this.updates=(this.updates??0)+1}},index:0})),ct:[0,1,2,3,4,5].map(()=>({nv:{volumes:[{cal_min:0,cal_max:1}],updateGLVolume(){this.updates=(this.updates??0)+1}},index:0}))}
const controller=installDisplayIntensity({loaded:()=>loaded,viewers:m=>all[m]});controller.refresh();controller.apply();const before=[...loaded.ct.raw.values]
const set=(id:string,value:string)=>{const el=document.querySelector<HTMLInputElement>(`#${id}`)!;el.value=value;el.dispatchEvent(new dom.window.Event('input'))}
set('ct-display-min-number','-500');set('ct-display-max-number','1500');controller.apply();for(const {nv} of all.ct){assert.equal(nv.volumes[0].cal_min,-500);assert.equal(nv.volumes[0].cal_max,1500)}assert.equal(all.mri[0].nv.volumes[0].cal_min,0);assert.deepEqual([...loaded.ct.raw.values],before)
set('ct-contrast','2');controller.apply();assert.equal(all.ct[0].nv.volumes[0].cal_max-all.ct[0].nv.volumes[0].cal_min,1500)
const min=all.ct[0].nv.volumes[0].cal_min;set('ct-display-min-number','9000');controller.apply();assert.equal(all.ct[0].nv.volumes[0].cal_min,min,'Reject inverted range')
all.ct.push({nv:{volumes:[{}],updateGLVolume(){}},index:0});controller.refresh();controller.apply();assert.equal(all.ct.at(-1).nv.volumes[0].cal_min,min,'New review volumes inherit display range')
document.querySelector<HTMLButtonElement>('#ct-display-reset')!.click();controller.apply();assert.equal(all.ct[0].nv.volumes[0].cal_min,-1000)
loaded.ct=item(4,4);controller.refresh();controller.apply();assert.equal(all.ct[0].nv.volumes[0].cal_max,5,'Constant image has nonzero display window')
loaded.ct=null;controller.refresh();assert(document.querySelector<HTMLInputElement>('#ct-contrast')!.disabled);assert(!document.querySelector('#initial-processing-panel')!.contains(document.querySelector('#define-windows')));assert(document.querySelector('#refine-panel')!.contains(document.querySelector('#define-windows')));assert(document.querySelector('#followup-panel')!.contains(document.querySelector('#define-windows-followup')))
await new Promise(r=>setTimeout(r,100));dom.window.close();console.log('Display-only intensity controls: independent negative/constant ranges, contrast, reset, invalid bounds, rebuilt review and immutable voxels passed.')
