import fs from 'node:fs/promises'
import path from 'node:path'
import * as nifti from 'nifti-reader-js'

/** Same native pixels, physical origins one millimeter apart. The saved CT→MRI
 * transform must therefore reproduce every original CT pixel on the MRI grid. */
export async function createSessionFixture(folder){
 await fs.mkdir(folder,{recursive:true})
 const dims=[21,23,25],count=dims.reduce((a,b)=>a*b,1),identity=[[1,0,0,0],[0,1,0,0],[0,0,1,0],[0,0,0,1]],matrix=[[1,0,0,1],[0,1,0,0],[0,0,1,0],[0,0,0,1]],ctAffine=[[.5,0,0,-5],[0,.5,0,-4],[0,0,.5,-3],[0,0,0,1]],mriAffine=ctAffine.map(r=>[...r]);mriAffine[0][3]+=1
 const values=Float32Array.from({length:count},(_,i)=>i%21+2*(Math.floor(i/21)%23)+3*Math.floor(i/(21*23))+10)
 async function image(filename,affine){const h=new nifti.NIFTI1();h.dims=[3,...dims,1,1,1,1];h.pixDims=[1,.5,.5,.5,1,1,1,1];h.affine=affine;h.sform_code=2;h.xyzt_units=2;h.datatypeCode=16;h.numBitsPerVoxel=32;h.vox_offset=352;h.magic='n+1\0';h.littleEndian=true;const b=new Uint8Array(352+values.byteLength);b.set(new Uint8Array(h.toArrayBuffer()));b.set(new Uint8Array(values.buffer),352);await fs.writeFile(filename,b)}
 const mri=path.join(folder,'anatomy.nii'),ct=path.join(folder,'ct.nii');await image(mri,mriAffine);await image(ct,ctAffine)
 const signature=(name,affine)=>({name,sourceFiles:[name],dims,pixDims:[.5,.5,.5],affine,workingDims:[9,10,11],workingSpacing:[1.25,11/9,1.2],selectedFrame:0})
 const landmarks=[[0,0,0],[3,0,0],[0,4,0],[0,0,5],[2,3,4]].map((ct,i)=>({id:i+1,ct,mri:[ct[0]+1,ct[1],ct[2]],enabled:true}))
 const payload={application:'Brainana Align',registrationPairing:'mri-ct',schemaVersion:13,appVersion:'0.18.0',savedAt:'2026-11-19T00:00:00Z',images:{mri:signature('anatomy.nii',mriAffine),ct:signature('ct.nii',ctAffine)},landmarks,selectedId:1,nextId:6,direction:'ct-mri',landmarkModel:'rigid',optimizationWindows:{mri:{axial:{min:[-2,-2,-2],max:[2,2,2]}},ct:{}},processing:{},fit:{direction:'ct-mri',initializationSource:'landmarks',model:'rigid',landmarkModel:'rigid',landmarkMatrix:matrix,baseMatrix:matrix,matrix,inverse:identity,manual:[0,0,0,0,0,0],rms:0,proposal:null,landmarkSnapshot:landmarks,landmarksChanged:false,fittedAt:'2026-11-19T00:00:00Z'}}
 const session=path.join(folder,'saved.json');await fs.writeFile(session,JSON.stringify(payload))
 return {dims,identity,matrix,ctAffine,mriAffine,values,mri,ct,payload,session}
}
