import assert from 'node:assert/strict'
import { applyLandmarkWarpForward, applyLandmarkWarpReverse, applyTps, fitLandmarkWarp, fitTps, summarizeJacobian } from '../source/src/domain/registration/nonlinear'
import { resolveNonlinearRegularization } from '../source/src/ui/nonlinearLandmarkControls'
import type { Mat4, Vec3 } from '../source/src/domain/registration/rigid'

const identity: Mat4=[[1,0,0,0],[0,1,0,0],[0,0,1,0],[0,0,0,1]]
const source:Vec3[]=[[0,0,0],[30,0,0],[0,30,0],[0,0,30],[30,30,5],[20,10,25],[8,24,18]]
const target:Vec3[]=source.map((point,index)=>[
  point[0] + (index===4?2:0),
  point[1] + (index===5?-1.5:0),
  point[2] + (index===6?1:0),
] as Vec3)

const exact=fitTps(source,target,0)
for(let index=0;index<source.length;index++){
  const mapped=applyTps(exact,source[index])
  assert.ok(Math.hypot(...mapped.map((value,axis)=>value-target[index][axis]))<1e-6,'exact TPS must interpolate every control landmark')
}

const identityTps=fitTps(source,source,0.05)
const qc=summarizeJacobian(identityTps,{min:[0,0,0],max:[30,30,30]},5)
assert.ok(Math.abs(qc.minimum-1)<1e-6&&Math.abs(qc.maximum-1)<1e-6,'identity warp Jacobian must be one')
assert.equal(qc.nonPositive,0)

const pair=fitLandmarkWarp(identity,source,target,0.05)
assert.equal(pair.kind,'landmark-tps')
assert.equal(pair.jacobian.nonPositive,0)
for(let index=0;index<source.length;index++){
  const forward=applyLandmarkWarpForward(identity,pair,source[index])
  const reverse=applyLandmarkWarpReverse(identity,pair,forward)
  assert.ok(Math.hypot(...reverse.map((value,axis)=>value-source[index][axis]))<0.25,'regularized reverse warp must round-trip control landmarks closely')
}

assert.throws(()=>fitTps(source.slice(0,3),target.slice(0,3),0),/four complete landmark pairs/)
assert.equal(resolveNonlinearRegularization('0.05','999'),0.05,'a named preset must ignore the custom field')
assert.equal(resolveNonlinearRegularization('custom','0'),0,'custom zero must permit exact interpolation')
assert.equal(resolveNonlinearRegularization('custom','0.1375'),0.1375,'custom regularization must preserve the entered numeric value')
assert.throws(()=>resolveNonlinearRegularization('custom',''),/Enter a custom/)
assert.throws(()=>resolveNonlinearRegularization('custom','-0.1'),/greater than or equal to zero/)
assert.throws(()=>resolveNonlinearRegularization('custom','NaN'),/finite value/)
console.log('nonlinear landmark TPS regression checks passed')
