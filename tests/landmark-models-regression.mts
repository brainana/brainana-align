import assert from 'node:assert/strict'
import { fitLandmarks, diagnoseTransform, type LandmarkModel } from '../apps/align/src/landmarkFit.ts'
import { applyMat4, multiplyMat4, rigidDelta, type Mat4, type Vec3 } from '../apps/align/src/rigid.ts'
import { invertAffine, sampleLinear, type RawNifti } from '../apps/align/src/roiWarp.ts'
import { resampleRegistration } from '../apps/align/src/registrationResampling.ts'
import { createSessionPayload, parseSessionPayload } from '../apps/align/src/sessionPersistence.ts'
import { createRegistrationArtifacts } from '../apps/align/src/exportArtifacts.ts'
import type { FitState } from '../apps/align/src/appTypes.ts'

const points: Vec3[] = [[-12,3,8],[19,-7,2],[1,22,-4],[-3,-5,24],[13,11,18],[-10,-18,-9],[9,4,-19]]
const rotation = rigidDelta([32,-18,11,179.8,-73,112], [8,-4,10])
const scale = (s: Vec3): Mat4 => [[s[0],0,0,0],[0,s[1],0,0],[0,0,s[2],0],[0,0,0,1]]
function close(a: Mat4, b: Mat4, tolerance=1e-6) {
  a.forEach((row, i) => row.forEach((v, j) => assert.ok(Math.abs(v-b[i][j]) < tolerance, `${i},${j}: ${v} != ${b[i][j]}`)))
}
const truths: Record<LandmarkModel, Mat4> = {
  rigid: rotation,
  similarity: multiplyMat4(rotation, scale([1.7,1.7,1.7])),
  anisotropic: multiplyMat4(rotation, scale([0.7,1.8,2.3])),
  affine: multiplyMat4(rotation, [[0.8,0.2,-0.3,0],[0,1.7,0.4,0],[0,0,2.1,0],[0,0,0,1]]),
}
let cases = 0
for (const model of Object.keys(truths) as LandmarkModel[]) {
  const truth = truths[model], target = points.map(p => applyMat4(truth,p))
  const result = fitLandmarks(points,target,model)
  assert.ok(result.rms < 1e-6, `${model} RMS ${result.rms}`)
  close(result.matrix, truth)
  close(multiplyMat4(result.matrix,result.inverse),scale([1,1,1]))
  assert.equal(result.diagnostics.reflection,false)
  if (model !== 'affine') assert.ok(result.diagnostics.shear < 1e-8)
  // A R diag(s) inverse generally is not R' diag(s'): the reverse fit has
  // moving-axis scale semantics; inversion itself must still be exact.
  if (model !== 'anisotropic') close(fitLandmarks(target,points,model).matrix,invertAffine(truth))
  close(fitLandmarks(target,points,'affine').matrix,invertAffine(truth))
  const fit: FitState = { direction:'ct-mri', model, parameterCount:result.parameterCount, diagnostics:result.diagnostics, reflectionConfirmed:false, landmarkMatrix:result.matrix, baseMatrix:result.matrix, matrix:result.matrix, inverse:result.inverse, rms:result.rms, manual:[0,0,0,0,0,0], proposal:null, landmarkSnapshot:points.map((p,i)=>({id:i+1,ct:p,mri:target[i],enabled:true})), landmarksChanged:false, fittedAt:new Date().toISOString() }
  const session = createSessionPayload({appVersion:'test',loaded:{mri:null,ct:null},landmarks:fit.landmarkSnapshot,selectedId:1,nextId:8,direction:'ct-mri',landmarkModel:model,fit,optimizationWindows:{mri:{},ct:{}}})
  const restored = parseSessionPayload(JSON.parse(JSON.stringify(session)))
  assert.equal(restored.fit?.model,model);close(restored.fit!.inverse,invertAffine(truth))
  const artifacts = createRegistrationArtifacts({appVersion:'test',fit,landmarks:fit.landmarkSnapshot})
  const metadata = JSON.parse(await artifacts[0].blob.text())
  assert.equal(metadata.parameter_count,result.parameterCount)
  assert.equal(metadata.fixed_space,'MRI');assert.equal(metadata.moving_space,'CT')
  assert.ok(artifacts[1].filename.endsWith(`_${model}.txt`))
  close(metadata.forward_matrix,truth)
  if (model === 'rigid') {
    const legacy = structuredClone(session);legacy.schemaVersion=1;delete legacy.fit!.model;delete legacy.fit!.diagnostics;delete legacy.fit!.parameterCount
    assert.equal(parseSessionPayload(legacy).fit!.model,'rigid')
  }
  const broken = structuredClone(session);broken.fit!.matrix[0][0]=NaN
  assert.throws(()=>parseSessionPayload(broken),/finite/)
  cases++
}
for (const params of [[0,0,0,2,-4,7],[0,0,0,90,0,0],[0,0,0,87,-73,112],[0,0,0,180,0,0]]) {
  const truth = rigidDelta(params as [number,number,number,number,number,number],[0,0,0])
  close(fitLandmarks(points,points.map(p=>applyMat4(truth,p))).matrix,truth)
  cases++
}
const mirror = multiplyMat4(rotation,scale([-1,1,1]))
const reflected = fitLandmarks(points,points.map(p=>applyMat4(mirror,p)),'affine')
assert.equal(reflected.diagnostics.reflection,true);assert.ok(reflected.diagnostics.warnings.some(w=>w.includes('mirrors')))
const proper = fitLandmarks(points,points.map(p=>applyMat4(mirror,p)),'rigid')
assert.ok(proper.diagnostics.warnings.some(w=>w.includes('mirrored affine candidate')))
const mirroredFit: FitState = {direction:'ct-mri',model:'affine',matrix:mirror,inverse:invertAffine(mirror),landmarkMatrix:mirror,baseMatrix:mirror,rms:0,manual:[0,0,0,0,0,0],proposal:null,landmarkSnapshot:[],landmarksChanged:false,fittedAt:''}
assert.throws(()=>createRegistrationArtifacts({appVersion:'test',fit:mirroredFit,landmarks:[]}),/confirmation/)
mirroredFit.reflectionConfirmed=true
assert.equal(JSON.parse(await createRegistrationArtifacts({appVersion:'test',fit:mirroredFit,landmarks:[]})[0].blob.text()).reflection,true)
for (const model of Object.keys(truths) as LandmarkModel[]) {
  assert.throws(()=>fitLandmarks(Array(5).fill([1,1,1]),Array(5).fill([2,2,2]),model),/one location/)
  assert.throws(()=>fitLandmarks([[0,0,0],[1,1,1],[2,2,2],[3,3,3]],[[0,0,0],[1,1,1],[2,2,2],[3,3,3]],model),/collinear/)
  assert.throws(()=>fitLandmarks([[NaN,0,0],...points],[[0,0,0],...points],model),/finite/)
}
assert.throws(()=>fitLandmarks([[0,0,0],[1,0,0],[0,1,0],[1,1,0]],[[0,0,0],[1,0,0],[0,1,0],[1,1,0]],'affine'),/degenerate|coplanar/)
assert.ok(diagnoseTransform(scale([0.1,1,8])).warnings.some(w=>w.includes('Extreme')))
const raw = (affine: Mat4): RawNifti => ({dims:[5,5,5],frameCount:1,affine,pixDims:[0.8,1.1,1.7],values:Float32Array.from({length:125},(_,i)=>i+1),datatypeCode:16,littleEndian:true,slope:1,intercept:0})
const voxelToWorld: Mat4 = [[0,-0.8,0.15,42],[1.1,0.05,0.2,-37],[-0.1,0.25,1.7,18],[0,0,0,1]]
const image = raw(voxelToWorld)
assert.equal(sampleLinear(image.values,image.dims,[4,4,4]),125,'last voxel must remain visible')
for (const truth of Object.values(truths).concat([mirror])) {
  const target = raw(multiplyMat4(truth,voxelToWorld))
  const out = resampleRegistration(image,target,truth)
  // Floating round-off at boundaries is handled by the sampler's tolerance.
  for(let k=1;k<4;k++)for(let j=1;j<4;j++)for(let i=1;i<4;i++)assert.ok(Math.abs(out[i+5*(j+5*k)]-image.values[i+5*(j+5*k)])<1e-4)
  cases++
}
console.log(`landmark model, inversion, reflection, session, export, and physical resampling regressions passed (${cases} recovery cases plus invalid-input cases)`)
// Noisy anisotropic fits should improve over a rigid fit without introducing shear.
const noisyTarget=points.map((p,i)=>applyMat4(truths.anisotropic,p).map((v,j)=>v+0.03*Math.sin(i*3+j)) as Vec3)
const noisy=fitLandmarks(points,noisyTarget,'anisotropic')
assert.ok(noisy.rms<fitLandmarks(points,noisyTarget,'rigid').rms)
assert.ok(noisy.diagnostics.shear<1e-8)
const narrow=points.map(p=>[p[0],p[1],p[2]*1e-4] as Vec3)
assert.ok(fitLandmarks(narrow,narrow,'affine').diagnostics.warnings.some(w=>w.includes('Landmark geometry')))
const sessionFit:FitState={direction:'ct-mri',model:'rigid',matrix:rotation,inverse:invertAffine(rotation),landmarkMatrix:rotation,baseMatrix:rotation,rms:0,manual:[0,0,0,0,0,0],proposal:{matrix:rotation,scoreBefore:1,scoreAfter:1.1,params:[0,0,0,0,0,0]},landmarkSnapshot:points.map((p,i)=>({id:i+1,ct:p,mri:applyMat4(rotation,p),enabled:true})),landmarksChanged:false,fittedAt:''}
assert.throws(()=>createRegistrationArtifacts({appVersion:'test',fit:sessionFit,landmarks:sessionFit.landmarkSnapshot}),/Accept or reject/)
const pendingSession=createSessionPayload({appVersion:'test',loaded:{mri:null,ct:null},landmarks:sessionFit.landmarkSnapshot,selectedId:null,nextId:8,direction:'ct-mri',fit:sessionFit,optimizationWindows:{mri:{},ct:{}}})
assert.ok(parseSessionPayload(pendingSession).fit!.proposal,'pending refinement must remain a proposal')
console.log('Noisy no-shear fit, unstable geometry warning, and pending-refinement export/restore checks passed')
