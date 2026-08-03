import assert from 'node:assert/strict'
import { applyForwardDeformation, applyInverseDeformation, createDeformationGrid, gridRegularization, optimizeDeformationGrid, resampleGrid, sampleGridDisplacement, summarizeGridJacobian, type DeformableSettings } from '../source/src/domain/registration/deformable'
import type { Vec3 } from '../source/src/domain/registration/rigid'

const settings:DeformableSettings={levelsMm:[20],regularization:.05,maximumDisplacement:4,iterationsPerLevel:1,jacobianMinimum:.5,jacobianMaximum:2}
const bounds={min:[0,0,0] as Vec3,max:[40,40,40] as Vec3}
const identity=createDeformationGrid(bounds,20,settings)
assert.deepEqual(applyInverseDeformation(identity,[12,17,29]),[12,17,29])
assert.deepEqual(applyForwardDeformation(identity,[12,17,29]),[12,17,29])
let qc=summarizeGridJacobian(identity,5)
assert.ok(Math.abs(qc.minimum-1)<1e-8&&Math.abs(qc.maximum-1)<1e-8)
assert.equal(qc.nonPositive,0)
for(const displacement of identity.displacements)displacement[0]=2
const point:Vec3=[18,21,9],inverse=applyInverseDeformation(identity,point),roundtrip=applyForwardDeformation(identity,inverse)
assert.ok(Math.abs(inverse[0]-20)<1e-8)
assert.ok(Math.hypot(...roundtrip.map((value,axis)=>value-point[axis]))<1e-5)
assert.ok(gridRegularization(identity)<1e-12)
qc=summarizeGridJacobian(identity,5)
assert.ok(Math.abs(qc.minimum-1)<1e-6&&Math.abs(qc.maximum-1)<1e-6)
const finer=createDeformationGrid(bounds,10,settings);resampleGrid(identity,finer)
assert.ok(Math.abs(sampleGridDisplacement(finer,[25,15,30])[0]-2)<1e-8)
async function main(){
  const optimized=await optimizeDeformationGrid({bounds,settings,score:grid=>-grid.displacements.reduce((sum,value)=>sum+(value[0]-3)**2+value[1]**2+value[2]**2,0)/grid.displacements.length,cancelled:()=>false})
  assert.ok(optimized.displacements.some(value=>value[0]>0))
  assert.equal(optimized.jacobian.nonPositive,0)
  const accepted=createDeformationGrid(bounds,20,settings)
  for(const displacement of accepted.displacements)displacement[0]=1.75
  let initialMean=NaN
  await optimizeDeformationGrid({bounds,settings:{...settings,iterationsPerLevel:0},initialGrid:accepted,score:grid=>{initialMean=grid.displacements.reduce((sum,value)=>sum+value[0],0)/grid.displacements.length;return 0},cancelled:()=>false})
  assert.ok(Math.abs(initialMean-1.75)<1e-8,'a subsequent deformable pass must initialize from the accepted deformation rather than identity')
  await assert.rejects(()=>optimizeDeformationGrid({bounds,settings,score:()=>0,cancelled:()=>true}),/cancelled/)
  console.log('deformable control-grid regression checks passed')
}
main().catch(error=>{console.error(error);process.exitCode=1})
