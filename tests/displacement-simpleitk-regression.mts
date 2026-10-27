import assert from 'node:assert/strict'
import fs from 'node:fs/promises'
import os from 'node:os'
import path from 'node:path'
import {spawnSync} from 'node:child_process'
import {NIFTI1} from 'nifti-reader-js'
import {splineDisplacementField} from '../packages/imaging-math/splineFields'
import {fitNonlinearSpline,createPointMapping,resampleSpline} from '../packages/imaging-math/nonlinearSpline'
import {applyMat4,rigidDelta,type Vec3} from '../packages/imaging-math/rigid'
import type {RawNifti} from '../packages/imaging-math/roiWarp'

// Independent reader qualification, not a graphical Slicer test. A SimpleITK
// runtime is explicit so release checks cannot silently skip interoperability.
const python=process.env.BRAINANA_SIMPLEITK_PYTHON??'/opt/anaconda3/bin/python',temporary=await fs.mkdtemp(path.join(os.tmpdir(),'brainana-field-reader-'))
const spacing:Vec3=[.8,1.3,2.1],affine=rigidDelta([11,-17,23,17,-11,29],[0,0,0]),translation:Vec3=[2.25,-3.5,4.75],dims:[number,number,number]=[7,8,9]
for(let axis=0;axis<3;axis++)for(let row=0;row<3;row++)affine[row][axis]*=spacing[axis]
const fixedAffine=affine.map(row=>[...row]);for(let row=0;row<3;row++)fixedAffine[row][3]+=translation[row]
const values=Float32Array.from({length:dims.reduce((a,b)=>a*b,1)},(_,i)=>{const x=i%dims[0],y=Math.floor(i/dims[0])%dims[1],z=Math.floor(i/(dims[0]*dims[1]));return 10+x+3*y+5*z+.1*x*y})
const moving:RawNifti={dims,pixDims:spacing,affine,values,frameCount:1,datatypeCode:16,littleEndian:true,slope:1,intercept:0},fixed:RawNifti={...moving,affine:fixedAffine}
const controls:Vec3[]=[];for(const x of [-10,0,10])for(const y of [-10,0,10])for(const z of [-10,0,10])controls.push([x,y,z])
const fit=fitNonlinearSpline(controls,controls.map(p=>p.map((v,a)=>v+translation[a]) as Vec3),.01),mapping=createPointMapping(fit.matrix,fit.spline)
function scalarNifti(image:RawNifti){
 const header=new NIFTI1();Object.assign(header,{littleEndian:true,dims:[3,...image.dims,1,1,1,1],pixDims:[1,...image.pixDims,1,1,1,1],datatypeCode:16,numBitsPerVoxel:32,vox_offset:352,scl_slope:1,scl_inter:0,xyzt_units:2,qform_code:0,sform_code:2,affine:image.affine,magic:'n+1\0'})
 const bytes=new Uint8Array(352+image.values.byteLength);bytes.set(new Uint8Array(header.toArrayBuffer()).subarray(0,352));const output=new DataView(bytes.buffer);for(let i=0;i<image.values.length;i++)output.setFloat32(352+4*i,image.values[i],true);return bytes
}
try{
 for(const [name,image] of [['moving',moving],['fixed',fixed]] as const)await fs.writeFile(path.join(temporary,name+'.nii'),scalarNifti(image))
 for(const direction of ['forward','inverse'] as const)await fs.writeFile(path.join(temporary,direction+'.nii'),await splineDisplacementField(direction==='forward'?moving:fixed,fit.matrix,fit.spline,direction))
 const indices:Vec3[]=[[0,0,0],[2,3,4],[5,6,7]],expected={dims,spacing,translation,fields:Object.fromEntries((['forward','inverse'] as const).map(direction=>{const grid=direction==='forward'?moving:fixed;return [direction,{affine:grid.affine,points:indices.map(index=>{const point=applyMat4(grid.affine,index);return {index,point,mapped:mapping[direction](point)}})}]})),forwardResampling:Array.from(await resampleSpline(moving,fixed,fit.matrix,fit.spline)),reverseResampling:Array.from(await resampleSpline(fixed,moving,fit.matrix,fit.spline,()=>{},true))}
 await fs.writeFile(path.join(temporary,'expected.json'),JSON.stringify(expected))
 const script=String.raw`
import json, pathlib, sys
import SimpleITK as sitk
import numpy as np
root=pathlib.Path(sys.argv[1])
expected=json.loads((root/'expected.json').read_text())
def lps(point): return np.asarray(point,dtype=float)*[-1,-1,1]
def close(actual,wanted,label,tolerance=2e-5):
    error=float(np.max(np.abs(np.asarray(actual)-np.asarray(wanted))))
    assert error<tolerance, f'{label}: error={error}, actual={actual}, wanted={wanted}'
transforms={}
reader_vectors={}
for direction in ('forward','inverse'):
    image=sitk.ReadImage(str(root/(direction+'.nii')))
    assert image.GetDimension()==3 and image.GetNumberOfComponentsPerPixel()==3
    assert list(image.GetSize())==expected['dims']
    assert int(float(image.GetMetaData('intent_code')))==1006
    close(image.GetSpacing(),expected['spacing'],direction+' spacing')
    affine=np.asarray(expected['fields'][direction]['affine'])
    close(image.GetOrigin(),lps(affine[:3,3]),direction+' LPS origin')
    desired_direction=np.diag([-1,-1,1])@affine[:3,:3]@np.diag(1/np.asarray(expected['spacing']))
    close(np.asarray(image.GetDirection()).reshape(3,3),desired_direction,direction+' oblique LPS direction')
    transforms[direction]=sitk.DisplacementFieldTransform(sitk.Cast(image,sitk.sitkVectorFloat64))
    for sample in expected['fields'][direction]['points']:
        index=tuple(sample['index'])
        point=lps(sample['point']);mapped=lps(sample['mapped'])
        close(image.TransformIndexToPhysicalPoint(index),point,direction+' physical sample')
        # ReadImage converts both NIfTI RAS geometry and intent1006 RAS
        # displacement components to ITK physical LPS; do not flip twice.
        known_vector=lps(expected['translation'])*(1 if direction=='forward' else -1)
        close(mapped-point,known_vector,direction+' known native RAS map in LPS')
        close(image.GetPixel(index),known_vector,direction+' known translation vector')
        close(image.GetPixel(index),mapped-point,direction+' reader LPS vector')
        close(transforms[direction].TransformPoint(point),mapped,direction+' physical TransformPoint')
    reader_vectors[direction]=list(image.GetPixel((2,3,4)))
moving=sitk.ReadImage(str(root/'moving.nii'))
fixed=sitk.ReadImage(str(root/'fixed.nii'))
# Resample takes an output-to-input transform. Fixed output pulls through the
# inverse field; moving output pulls through the forward field.
aligned=sitk.GetArrayFromImage(sitk.Resample(moving,fixed,transforms['inverse'],sitk.sitkLinear,-1000,sitk.sitkFloat32))
reverse_output=sitk.GetArrayFromImage(sitk.Resample(fixed,moving,transforms['forward'],sitk.sitkLinear,-1000,sitk.sitkFloat32))
shape=tuple(expected['dims'][::-1])
interior=(slice(1,-1),)*3
close(aligned[interior],np.asarray(expected['forwardResampling']).reshape(shape)[interior],'inverse pull matches Brainana forward image resampling',2e-4)
close(reverse_output[interior],np.asarray(expected['reverseResampling']).reshape(shape)[interior],'forward pull matches Brainana reverse image resampling',2e-4)
wrong=sitk.GetArrayFromImage(sitk.Resample(moving,fixed,transforms['forward'],sitk.sitkLinear,-1000,sitk.sitkFloat32))
assert float(np.max(np.abs(wrong[interior]-aligned[interior])))>1, 'Wrong field direction must visibly fail the asymmetric image control'
print(json.dumps({'simpleitk':sitk.Version_VersionString(),'readerVectorsLPS':reader_vectors,'pointMappings':6,'interiorResamplingVoxelsPerDirection':int(aligned[interior].size)}))
`
 const result=spawnSync(python,['-c',script,temporary],{encoding:'utf8',timeout:25000,maxBuffer:1024*1024})
 assert.equal(result.error,undefined,result.error?.message);assert.equal(result.status,0,`Independent SimpleITK field-reader regression failed:\n${result.stdout}\n${result.stderr}`)
 const report=JSON.parse(result.stdout.trim());assert.equal(report.pointMappings,6);assert.equal(report.interiorResamplingVoxelsPerDirection,210)
 console.log(`Independent SimpleITK ${report.simpleitk}: actual intent1006 reader converts RAS geometry/vectors to LPS, anisotropic oblique TransformPoint agrees in both directions, and inverse/forward pull resampling matches Brainana (${report.interiorResamplingVoxelsPerDirection} interior voxels per direction). No graphical Slicer test was run.`)
}finally{await fs.rm(temporary,{recursive:true,force:true})}
