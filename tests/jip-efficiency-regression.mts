import assert from 'node:assert/strict'
import {cloneFitState} from '../apps/align/src/fitSnapshot'
import {jipData,validateJip,serializeJip,type JipDeformation} from '../packages/imaging-math/jipDeformation'
import {initialManualState,applyManualAffine} from '../apps/align/src/manualAffine'
import {createSessionPayload,parseSessionPayload} from '../apps/align/src/sessionPersistence'
import {createRegistrationArtifacts} from '../apps/align/src/exportArtifacts'
import {createPointMapping} from '../packages/imaging-math/nonlinearSpline'
import type {Mat4} from '../packages/imaging-math/rigid'
const identity:Mat4=[[1,0,0,0],[0,1,0,0],[0,0,1,0],[0,0,0,1]]
const values=new Float32Array(3*8**3)
for(let i=0;i<8**3;i++)values[i]=.01*Math.sin(i)
const field:JipDeformation={version:1,kernel:'jip-dense-pull',baseAffine:identity,dims:[8,8,8],affine:identity,residualValues:values,settings:{axes:'x'},execution:{active:'cpu'},warnings:[]}
validateJip(field)
const fit:any={direction:'ct-mri',model:'jip',parameterCount:values.length,matrix:identity,inverse:identity,landmarkMatrix:identity,baseMatrix:identity,manual:[0,0,0,0,0,0],nonlinear:field,initializationSource:'automatic',rms:0,proposal:null,landmarkSnapshot:[],landmarksChanged:false,fittedAt:new Date().toISOString()}
const copy=cloneFitState(fit)
assert.notEqual(copy,fit);assert.notEqual(copy.nonlinear,field)
assert.equal((copy.nonlinear as JipDeformation).residualValues,values)
assert.equal(jipData(copy.nonlinear as JipDeformation),jipData(field),'validated binary field reused across snapshots')
for(const selected of ['mri','ct'] as const){
 const state=initialManualState(fit,{mri:[3,3,3],ct:[3,3,3]},selected);state.parameters[selected][0]=.1
 const edited=applyManualAffine(fit,state)
 assert.equal((edited.nonlinear as JipDeformation).residualValues,values)
 assert.equal(field.inputAffine,undefined,'manual composition must not alter prior metadata')
 const payload=createSessionPayload({appVersion:'test',loaded:{mri:null,ct:null},landmarks:[],selectedId:null,nextId:1,direction:fit.direction,fit:edited,optimizationWindows:{mri:{},ct:{}}})
 const encoded=payload.fit!.nonlinear as JipDeformation
 assert.equal(encoded.residualValues,undefined);assert.equal(encoded.residualBase64,Buffer.from(values.buffer).toString('base64'))
 const restored=parseSessionPayload(JSON.parse(JSON.stringify(payload)))
 const point:[number,number,number]=[3,3,3],mapping=createPointMapping(restored.fit!.matrix,restored.fit!.nonlinear)
 assert(Math.hypot(...mapping.inverse(mapping.forward(point)).map((v,a)=>v-point[a]))<1e-4)
}
const legacy=serializeJip(field);validateJip(legacy)
const legacySnapshot=cloneFitState({...fit,nonlinear:legacy})
assert.equal(jipData(legacySnapshot.nonlinear as JipDeformation),jipData(legacy),'restored base64 sessions reuse validated decode too')
const pending=cloneFitState({...fit,proposal:{kind:'jip',matrix:identity,scoreBefore:0,scoreAfter:0,params:[],previous:fit}})
assert.equal((pending.proposal!.previous!.nonlinear as JipDeformation).residualValues,values)
assert.equal((cloneFitState(pending,true).proposal!.previous!.nonlinear as JipDeformation).residualValues,undefined)
const artifact=createRegistrationArtifacts({appVersion:'test',fit,landmarks:[]})
const json=JSON.parse(await artifact[0].blob.text())
assert.equal(json.nonlinear.residualValues,undefined);assert.equal(json.nonlinear.residualBase64,serializeJip(field).residualBase64)
assert.throws(()=>validateJip({...field,residualBase64:'also encoded'}),/binary/)
assert.throws(()=>validateJip({...field,residualValues:new Float32Array(2)}),/binary/)
const bad=values.slice();bad[5]=NaN;assert.throws(()=>validateJip({...field,residualValues:bad}),/Nonfinite/)
console.log('Binary fields share validated immutable values across manual/review/proposal snapshots; session/export compatibility and malformed data guards passed')
