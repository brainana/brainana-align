import {initialManualState,applyManualAffine} from '../apps/align/src/manualAffine'
import assert from 'node:assert/strict'
import fs from 'node:fs'
import path from 'node:path'
import {createPointMapping,validateSpline} from '../packages/imaging-math/nonlinearSpline'
import {validateJip,type JipDeformation} from '../packages/imaging-math/jipDeformation'
import {multiplyMat4,type Mat4,type Vec3} from '../packages/imaging-math/rigid'
import {createRegistrationArtifacts} from '../apps/align/src/exportArtifacts'
import {createSessionPayload,parseSessionPayload} from '../apps/align/src/sessionPersistence'
const records=JSON.parse(fs.readFileSync(path.resolve('tests/fixtures/jip/mapping-fixtures.json'),'utf8'));let worst=0
for(const {result,samples} of records){const mapping=createPointMapping(result.matrix,result.deformation??undefined);if(result.deformation)validateSpline(result.deformation);for(const sample of samples){const actual=mapping.inverse(sample.target),error=Math.hypot(...actual.map((v,i)=>v-sample.source[i]));worst=Math.max(worst,error);assert(error<2e-4,`Native pull mismatch ${error}`);assert(Math.hypot(...mapping.forward(actual).map((v,i)=>v-sample.target[i]))<2e-4)}}
const identity:Mat4=[[1,0,0,0],[0,1,0,0],[0,0,1,0],[0,0,0,1]],dims:[number,number,number]=[8,8,8],values=new Float32Array(3*8**3)
for(let z=0;z<8;z++)for(let y=0;y<8;y++)for(let x=0;x<8;x++)values[x+8*(y+8*z)]=.05*Math.sin(x)*Math.sin(y)*Math.sin(z)
const s:JipDeformation={version:1,kernel:'jip-dense-pull',baseAffine:identity,dims,affine:identity,residualBase64:Buffer.from(values.buffer).toString('base64'),settings:{axes:'xyz'},execution:{active:'cpu'},warnings:[]};validateJip(s)
const d:Mat4=[[1.1,.01,0,3],[0,.9,0,-2],[0,0,1,1],[0,0,0,1]],p:Mat4=[[1,0,0,.1],[0,1,0,.2],[0,0,1,.3],[0,0,0,1]],map=createPointMapping(multiplyMat4(d,p),{...s,inputAffine:p})
for(const v of [[2,3,4],[4,3,2],[30,40,50]] as Vec3[])assert(Math.hypot(...map.inverse(map.forward(v)).map((n,i)=>n-v[i]))<1e-4)
assert.throws(()=>validateJip({...s,residualBase64:'AA=='}),/encoding|length/);assert.throws(()=>validateJip({...s,dims:[200,200,200]}),/large|dimensions/);assert.throws(()=>map.inverse([NaN,0,0]),/Invalid/)
const fit:any={direction:'ct-mri',model:'jip',parameterCount:1536,matrix:identity,inverse:identity,landmarkMatrix:identity,baseMatrix:identity,manual:[0,0,0,0,0,0],nonlinear:s,initializationSource:'automatic',rms:0,proposal:null,landmarkSnapshot:[],landmarksChanged:false,fittedAt:new Date().toISOString()}
const payload=createSessionPayload({appVersion:'0.17.0',loaded:{mri:null,ct:null},landmarks:[],selectedId:null,nextId:1,direction:'ct-mri',landmarkModel:'rigid',fit,optimizationWindows:{mri:{},ct:{}}} as any)
assert.equal(parseSessionPayload(JSON.parse(JSON.stringify(payload))).fit?.nonlinear?.kernel,'jip-dense-pull')
for(const direction of ['ct-mri','mri-ct'])for(const selected of ['mri','ct'] as const){const base={...fit,direction},state=initialManualState(base,{mri:[3,3,3],ct:[3,3,3]},selected);state.parameters[selected][0]=.1;state.parameters[selected][6]=Math.log(1.01);const edited=applyManualAffine(base,state);const session=createSessionPayload({appVersion:'test',loaded:{mri:null,ct:null},landmarks:[],selectedId:null,nextId:1,direction,fit:edited,optimizationWindows:{mri:{},ct:{}}});const loaded=parseSessionPayload(JSON.parse(JSON.stringify(session)));assert.equal(loaded.fit?.model,'jip');assert.equal(loaded.fit?.manualAffine?.selected,selected);const map=createPointMapping(loaded.fit!.matrix,loaded.fit!.nonlinear);assert(Math.hypot(...map.inverse(map.forward([3,3,3])).map(v=>v-3))<1e-4)}
const artifacts=createRegistrationArtifacts({appVersion:'0.17.0',fit,landmarks:[]});assert(!artifacts.some(a=>a.filename.endsWith('.txt')));const metadata=JSON.parse(await artifacts[0].blob.text());assert.equal(metadata.transform_model,'jip');assert.equal(metadata.parameter_count,null)
assert.throws(()=>createRegistrationArtifacts({appVersion:'0.17.0',fit:{...fit,proposal:{kind:'jip'}},landmarks:[]}),/Accept|reject/)
console.log(`JIP native coordinate fixtures, same-map inverses, manual composition, sessions and export gating passed; worst native pull error ${worst} mm`)
