import assert from 'node:assert/strict'
import { automaticParameterMatrix, optimizeAutomaticLinear } from '../source/src/domain/registration/automaticLinear'
import { shrinkFactorLevels } from '../source/src/domain/registration/imagePyramid'
import type { Mat4 } from '../source/src/domain/registration/rigid'

assert.deepEqual(shrinkFactorLevels('standard'),[8,4,2,1])
assert.deepEqual(shrinkFactorLevels('fast'),[4,2,1])
assert.deepEqual(shrinkFactorLevels('custom','16,8,2,1'),[16,8,2,1])
assert.throws(()=>shrinkFactorLevels('custom','8,2'),/final shrink factor must be 1/i)
assert.throws(()=>shrinkFactorLevels('custom','2,4,1'),/coarse to fine/i)

const target=automaticParameterMatrix([5,-3,2,0,0,90,0,0,0,0,0,0],[0,0,0],[0,0,0])
const error=(matrix:Mat4)=>matrix.slice(0,3).flatMap((row,r)=>row.map((value,c)=>(value-target[r][c])**2)).reduce((a,b)=>a+b,0)
async function main(){
  const result=await optimizeAutomaticLinear({settings:{model:'rigid',enabledParameters:['tx','ty','tz','rx','ry','rz'],rotationRanges:[90,90,90],translationLimitMm:10,scaleLimit:.3,shearLimit:.2,candidatesToRefine:3,shrinkFactors:[8,4,2,1],sweepsPerLevel:[2,3,3,2]},movingCenter:[0,0,0],fixedCenter:[0,0,0],score:matrix=>-error(matrix)})
  assert(result.scoreAfter>result.scoreBefore)
  assert(result.determinant>0)
  assert(Math.abs(result.parameters[5]-90)<4)
  assert(Math.abs(result.parameters[0]-5)<1)
  assert.equal(result.warnings.some(message=>/reflec/i.test(message)),false)
  const affineTargetParameters=[2,-1,1,0,0,0,Math.log(1.1),Math.log(.9),Math.log(1.05),.04,-.03,.02]
  const affineTarget=automaticParameterMatrix(affineTargetParameters,[0,0,0],[0,0,0])
  const affineError=(matrix:Mat4)=>matrix.slice(0,3).flatMap((row,r)=>row.map((value,c)=>(value-affineTarget[r][c])**2)).reduce((a,b)=>a+b,0)
  const affine=await optimizeAutomaticLinear({settings:{model:'affine',enabledParameters:['tx','ty','tz','rx','ry','rz','sx','sy','sz','shxy','shxz','shyz'],rotationRanges:[0,0,0],translationLimitMm:5,scaleLimit:.3,shearLimit:.15,candidatesToRefine:1,shrinkFactors:[4,2,1],sweepsPerLevel:[2,4,4]},movingCenter:[0,0,0],fixedCenter:[0,0,0],score:matrix=>-affineError(matrix)})
  assert(affine.scoreAfter>affine.scoreBefore)
  assert(affineError(affine.matrix)<2e-3,`affine recovery error ${affineError(affine.matrix)}`)
  assert(affine.determinant>0)
  console.log('FLIRT-inspired automatic linear and input-relative pyramid regression tests passed')
}
void main()
