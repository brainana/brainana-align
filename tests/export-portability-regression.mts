import assert from 'node:assert/strict'
import {JSDOM} from 'jsdom'
import * as nifti from 'nifti-reader-js'
import {saveNativeAlignedImage} from '../apps/align/src/nativeExport'
import {createRegistrationArtifacts} from '../apps/align/src/exportArtifacts'
import {initialManualState,applyManualAffine} from '../apps/align/src/manualAffine'
import {createPointMapping,fitNonlinearSpline,validateSpline,type NonlinearSpline} from '../packages/imaging-math/nonlinearSpline'
import {serializeJip,type JipDeformation} from '../packages/imaging-math/jipDeformation'
import {serializeNonlinear} from '../packages/imaging-math/nonlinearSerialization'
import {splineDisplacementField,DISPLACEMENT_FIELD_PROFILE} from '../packages/imaging-math/splineFields'
import {applyMat4,type Mat4,type Vec3} from '../packages/imaging-math/rigid'
import {invertAffine,type RawNifti} from '../packages/imaging-math/roiWarp'
import type {FitState,Loaded} from '../apps/align/src/appTypes'

const identity:Mat4=[[1,0,0,0],[0,1,0,0],[0,0,1,0],[0,0,0,1]],distance=(a:Vec3,b:Vec3)=>Math.hypot(...a.map((v,i)=>v-b[i]))
const points:Vec3[]=[];for(const x of [-4,0,4])for(const y of [-4,0,4])for(const z of [-4,0,4])points.push([x,y,z])
const spline=fitNonlinearSpline(points,points.map(([x,y,z])=>[x+.1*Math.sin(y),y+.1*Math.sin(z),z+.1*Math.sin(x)] as Vec3),.01)
const values=Float32Array.from({length:3*8**3},(_,i)=>.01*Math.sin(i/13))
const live:JipDeformation={version:1,kernel:'jip-dense-pull',baseAffine:identity,dims:[8,8,8],affine:[[3,0,0,-10],[0,3,0,-10],[0,0,3,-10],[0,0,0,1]],residualValues:values,settings:{axes:'xyz'},execution:{active:'cpu'},warnings:[]}
const legacy=serializeJip(live)
const raw:RawNifti={dims:[3,4,2],pixDims:[Math.hypot(.7,.1),Math.hypot(-1,.1,.1),Math.hypot(.1,1.5)],affine:[[0,-1,.1,4],[.7,.1,0,1],[.1,.1,1.5,2],[0,0,0,1]],values:new Float32Array(24),frameCount:1,datatypeCode:16,littleEndian:true,slope:1,intercept:0}
const loaded={name:'source.nii',raw,nvImage:null,sourceFiles:['source.nii']} as unknown as Loaded
const dom=new JSDOM('<button id="alignment-cancel"></button>');Object.assign(globalThis,{window:dom.window,document:dom.window.document})
const saved:{name:string;blob:Blob}[]=[];let terminated=0,ended=0
class ExportWorker{onmessage:any;onerror:any;postMessage(){queueMicrotask(()=>this.onmessage?.({data:{type:'result',blob:new Blob(['image'])}}))}terminate(){terminated++}}
Object.assign(globalThis,{Worker:ExportWorker});(window as any).brainanaAlignSaveBlob=async(blob:Blob,name:string)=>{saved.push({name,blob})}
const activity={begin:async()=>true,phase:async()=>{},end:()=>{ended++}}
for(const direction of ['ct-mri','mri-ct'])for(const nonlinear of [live,legacy,spline.spline] as NonlinearSpline[]){
 const base=nonlinear.kernel==='jip-dense-pull'?identity:spline.matrix
 const fit:FitState={direction,model:nonlinear.kernel==='jip-dense-pull'?'jip':'spline',nonlinear,parameterCount:nonlinear.kernel==='jip-dense-pull'?values.length:81,matrix:base,inverse:invertAffine(base),landmarkMatrix:base,baseMatrix:base,manual:[0,0,0,0,0,0],rms:0,proposal:null,landmarkSnapshot:[],landmarksChanged:false,fittedAt:'test'}
 const state=initialManualState(fit,{mri:[1,2,3],ct:[2,1,3]},'mri');state.parameters.mri[0]=.2;state.parameters.mri[4]=1;state.parameters.ct[1]=-.15;state.parameters.ct[3]=-.5
 const edited=applyManualAffine(fit,state),snapshot=structuredClone(edited),sourceMod=direction==='ct-mri'?'ct':'mri',targetMod=sourceMod==='ct'?'mri':'ct'
 const options={data:'native',fov:'reference-native',resolution:'reference-native',spacing:[1,1,1],interpolation:'linear'} as const
 assert.equal(await saveNativeAlignedImage(loaded,loaded,edited,sourceMod,targetMod,activity,()=>{},{...options,spacing:[1,1,1]}),true)
 const imageMetadata=JSON.parse(await saved.at(-1)!.blob.text())
 assert(saved.at(-1)!.name.endsWith('.json'));assert.equal(imageMetadata.direction,direction);assert.equal(imageMetadata.moving,sourceMod);assert.equal(imageMetadata.fixed,targetMod)
 assert.deepEqual(imageMetadata.outputGeometry.affine,raw.affine)
 assert.equal(imageMetadata.nonlinear.residualValues,undefined,'Actual aligned-image exporter must never JSON-stringify typed displacement values')
 if(nonlinear.kernel==='jip-dense-pull')assert.equal(imageMetadata.nonlinear.residualBase64,legacy.residualBase64)
 validateSpline(imageMetadata.nonlinear)
 const before=createPointMapping(edited.matrix,edited.nonlinear),after=createPointMapping(imageMetadata.matrix,imageMetadata.nonlinear)
 for(const point of [[1,2,3],[3,1,4],[0,0,0]] as Vec3[])for(const mapDirection of ['forward','inverse'] as const)assert(distance(before[mapDirection](point),after[mapDirection](point))<1e-9,`${direction} ${nonlinear.kernel} ${mapDirection} mapping survives actual save with manual pre/post composition`)
 assert.deepEqual(edited,snapshot,'Serialization cannot mutate the accepted fit or binary input')
 const artifacts=createRegistrationArtifacts({appVersion:'test',fit:edited,landmarks:[],fieldExport:{requestedSpacingMm:2,forwardGrid:{dims:raw.dims,affine:raw.affine},inverseGrid:{dims:raw.dims,affine:raw.affine}}})
 const metadata=JSON.parse(await artifacts[0].blob.text());assert.equal(metadata.displacement_field_profile.intentCode,1006);assert.deepEqual(metadata.displacement_field_profile.componentOrder,['R','A','S']);assert.equal(metadata.displacement_field_profile.forward.gridSpace,'moving');assert.equal(metadata.displacement_field_profile.inverse.gridSpace,'fixed');assert.equal(metadata.nonlinear.residualValues,undefined)
 // Inspect actual NIfTI output for both map directions, every voxel and every
 // physical component on an oblique axis-swapped grid.
 for(const mapDirection of ['forward','inverse'] as const){
  const bytes=await splineDisplacementField(raw,edited.matrix,edited.nonlinear!,mapDirection),header=nifti.readHeader(bytes.buffer)!,data=new Float32Array(nifti.readImage(header,bytes.buffer))
  assert.equal(header.intent_code,1006);assert.equal(header.intent_name,'RAS displacement');assert.deepEqual(header.dims.slice(0,6),[5,...raw.dims,1,3]);assert.equal(header.xyzt_units,2);assert.equal(header.sform_code,2);assert.equal(header.qform_code,0)
  for(let r=0;r<4;r++)for(let c=0;c<4;c++)assert(Math.abs(header.affine[r][c]-raw.affine[r][c])<1e-6)
  for(let z=0;z<raw.dims[2];z++)for(let y=0;y<raw.dims[1];y++)for(let x=0;x<raw.dims[0];x++){
   const index=x+raw.dims[0]*(y+raw.dims[1]*z),p=applyMat4(raw.affine,[x,y,z]),q=before[mapDirection](p)
   for(let a=0;a<3;a++)assert(Math.abs(data[a*24+index]-(q[a]-p[a]))<1e-6,'RAS vector components must not be rotated into voxel axes or converted to LPS')
  }
 }
}
assert.equal(terminated,6);assert.equal(ended,6);assert.equal(DISPLACEMENT_FIELD_PROFILE.intentCode,1006)
assert.throws(()=>serializeNonlinear({...live,residualValues:JSON.parse(JSON.stringify(values))}),/binary/)
assert.equal(serializeNonlinear(undefined),null)
dom.window.close()
console.log('Actual aligned-image JSON and displacement writer: live/legacy JIP and spline, both registration/map directions, manual pre/post composition, immutable inputs, portable roundtrips, displacement intent1006 and oblique RAS vectors passed.')
