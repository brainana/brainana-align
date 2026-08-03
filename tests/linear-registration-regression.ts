import assert from 'node:assert/strict'
import { applyMat4, fitRigid, multiplyMat4, type Mat4, type Vec3 } from '../source/src/domain/registration/rigid.ts'
import { diagnoseReflectionAlternative, fitAffine, fitAnisotropic, fitCustomLinear, fitLinearModel, fitSimilarity, invertAffineMatrix } from '../source/src/domain/registration/linear.ts'

const points: Vec3[] = [[-12,-8,-5],[14,-7,-3],[-9,15,-2],[11,12,4],[-5,-4,17],[7,3,-14]]
const transformed = (matrix: Mat4) => points.map(point => applyMat4(matrix, point))
const close = (actual: number, expected: number, tolerance = 1e-6) => assert.ok(Math.abs(actual-expected) <= tolerance, `${actual} differs from ${expected}`)
const applyAffine=(matrix:Mat4,point:Vec3):Vec3=>applyMat4(matrix,point)
const invertAffine=(matrix:Mat4):Mat4=>invertAffineMatrix(matrix)
const rotation=(axis:Vec3,degrees:number,translation:Vec3=[0,0,0]):Mat4=>{const length=Math.hypot(...axis),[x,y,z]=axis.map(value=>value/length),angle=degrees*Math.PI/180,c=Math.cos(angle),s=Math.sin(angle),t=1-c;return [[t*x*x+c,t*x*y-s*z,t*x*z+s*y,translation[0]],[t*x*y+s*z,t*y*y+c,t*y*z-s*x,translation[1]],[t*x*z-s*y,t*y*z+s*x,t*z*z+c,translation[2]],[0,0,0,1]]}

for(const degrees of [4,90,137,179.9]){
  const expected=rotation([.3,.7,-.2],degrees,[12,-8,5]),fit=fitRigid(points,transformed(expected))
  assert.ok(fit.rms<1e-7,`rigid ${degrees}° fixture must recover exactly`)
  fit.matrix.slice(0,3).forEach((row,r)=>row.forEach((value,c)=>close(value,expected[r][c],1e-6)))
}

const similarity: Mat4 = [[0,-1.2,0,10],[1.2,0,0,-5],[0,0,1.2,3],[0,0,0,1]]
const similarityFit = fitSimilarity(points, transformed(similarity))
assert.equal(similarityFit.model, 'similarity')
assert.ok(similarityFit.rms < 1e-8)
similarityFit.matrix.slice(0,3).forEach((row,r)=>row.forEach((value,c)=>close(value,similarity[r][c],1e-7)))

const anisotropicRotation=rotation([.2,-.5,.8],73,[0,0,0]),axisScales:Vec3=[1.25,.82,1.08]
const anisotropic:Mat4=anisotropicRotation.map((row,r)=>r<3?[row[0]*axisScales[0],row[1]*axisScales[1],row[2]*axisScales[2],[9,-6,4][r]]:[0,0,0,1])
const anisotropicFit=fitAnisotropic(points,transformed(anisotropic))
assert.equal(anisotropicFit.model,'anisotropic');assert.equal(anisotropicFit.diagnostics.parameterCount,9);assert.ok(anisotropicFit.rms<1e-7)
anisotropicFit.matrix.slice(0,3).forEach((row,r)=>row.forEach((value,c)=>close(value,anisotropic[r][c],1e-6)))
axisScales.forEach((value,axis)=>close(anisotropicFit.diagnostics.scales[axis],value,1e-6))

const affine: Mat4 = [[1.08,.12,-.04,7],[-.03,.93,.08,-4],[.05,-.02,1.15,2],[0,0,0,1]]
const affineFit = fitAffine(points, transformed(affine))
assert.equal(affineFit.model, 'affine')
assert.ok(affineFit.rms < 1e-8)
affineFit.matrix.slice(0,3).forEach((row,r)=>row.forEach((value,c)=>close(value,affine[r][c],1e-8)))
const inverse = invertAffineMatrix(affineFit.matrix)
for (const point of points) {
  const roundTrip = applyMat4(inverse, applyMat4(affineFit.matrix, point))
  roundTrip.forEach((value,axis)=>close(value,point[axis],1e-8))
}
const reverseFit=fitAffine(transformed(affine),points)
for(const point of points){const recovered=applyMat4(reverseFit.matrix,applyMat4(affine,point));recovered.forEach((value,axis)=>close(value,point[axis],1e-7))}

const obliqueAffine=multiplyMat4(rotation([.4,-.2,.7],137,[11,-5,8]),[[1.18,.16,-.09,0],[0,.87,.11,0],[0,0,1.06,0],[0,0,0,1]])
const obliqueAffineFit=fitAffine(points,transformed(obliqueAffine))
assert.ok(obliqueAffineFit.determinant>0);assert.ok(obliqueAffineFit.rms<1e-7)
obliqueAffineFit.matrix.slice(0,3).forEach((row,r)=>row.forEach((value,c)=>close(value,obliqueAffine[r][c],1e-6)))

const reflected:Mat4=[[-1,0,0,20],[0,1,0,-3],[0,0,1,2],[0,0,0,1]]
assert.throws(()=>fitAffine(points,transformed(reflected)),/do not support a stable orientation-preserving affine/)
const reflectedFit=fitAffine(points,transformed(reflected),true)
assert.equal(reflectedFit.diagnostics.reflected,true);assert.ok(reflectedFit.determinant<0);assert.ok(reflectedFit.diagnostics.warnings.some(warning=>/mirrors anatomy/.test(warning)))
assert.throws(()=>fitLinearModel(points,transformed(reflected),'affine'),/do not support a stable orientation-preserving affine/)
assert.equal(fitLinearModel(points,transformed(reflected),'affine',[],{allowReflection:true}).diagnostics.reflected,true)

const symmetric: Vec3[] = [[-2,-1,-1],[2,-1,-1],[-2,1,-1],[2,1,-1],[-2,-1,1],[2,1,1]]
const shear: Mat4 = [[1,.25,0,0],[0,1,0,0],[0,0,1,0],[0,0,0,1]]
const custom = fitCustomLinear(symmetric, symmetric.map(point=>applyMat4(shear,point)), ['shxy'])
assert.ok(custom.rms < 1e-6)
close(custom.matrix[0][1], .25, 1e-6)
close(custom.matrix[0][0], 1, 1e-10)
close(custom.matrix[1][1], 1, 1e-10)
close(custom.matrix[2][2], 1, 1e-10)
close(custom.matrix[0][3], 0, 1e-10)

assert.throws(() => fitAffine(points.slice(0,3), transformed(affine).slice(0,3)), /At least 4 complete landmark pairs/)
const collinear:Vec3[]=[[0,0,0],[1,0,0],[2,0,0],[3,0,0]]
assert.throws(()=>fitAffine(collinear,collinear),/rank deficient/)
assert.throws(()=>fitAnisotropic(collinear,collinear),/span all three axes|span enough/)

// Landmark coordinates must be fitted in physical NIfTI space. These two
// deliberately different, oblique, anisotropic voxel geometries should not
// change recovery of the physical CT-to-MRI transform in either direction.
const ctVoxelToWorld:Mat4=[[0,-.7,.2,31],[.8,.1,0,-44],[.1,.2,2.4,18],[0,0,0,1]]
const mriVoxelToWorld:Mat4=[[-1.1,.15,0,75],[.1,1.3,-.2,-22],[0,.12,1.7,9],[0,0,0,1]]
const physicalCtToMri:Mat4=[[.98,-.15,.08,6],[.13,1.04,.05,-9],[-.04,.03,.94,3],[0,0,0,1]]
const ctVoxelLandmarks:Vec3[]=[[8,11,4],[40,9,7],[7,45,12],[37,39,16],[17,24,35],[51,31,29]]
const ctWorld=ctVoxelLandmarks.map(point=>applyAffine(ctVoxelToWorld,point))
const expectedMriWorld=ctWorld.map(point=>applyAffine(physicalCtToMri,point))
const mriWorldToVoxel=invertAffine(mriVoxelToWorld)
const mriVoxelLandmarks=expectedMriWorld.map(point=>applyAffine(mriWorldToVoxel,point))
const mriWorld=mriVoxelLandmarks.map(point=>applyAffine(mriVoxelToWorld,point))
const physicalForward=fitAffine(ctWorld,mriWorld)
const physicalReverse=fitAffine(mriWorld,ctWorld)
assert.ok(physicalForward.rms<1e-7);assert.ok(physicalReverse.rms<1e-7)
for(const point of ctWorld){const mapped=applyMat4(physicalForward.matrix,point),roundTrip=applyMat4(physicalReverse.matrix,mapped);roundTrip.forEach((value,axis)=>close(value,point[axis],1e-7))}

// Regression from a real six-landmark report. The original CT left/right pair
// was crossed. A proper affine must be rejected instead of collapsing an axis
// and presenting extreme shear as a usable overlay. Correcting only that pair
// restores a stable, low-residual affine solution.
const reportedMri:Vec3[]=[[0,45.2,21.1],[0,-22.5,15],[0,-2.7,35.9],[-.3,1.2,13.2],[-27.8,5.1,16.1],[28.3,5.1,16.6]]
const reportedCt:Vec3[]=[[2.5,-6.7,-31.8],[-.7,-2.6,30.9],[1.3,11.8,8.2],[1.4,-2.4,7.8],[26.1,-5.7,3.3],[-21.5,-3.5,.1]]
assert.throws(()=>fitAffine(reportedCt,reportedMri),/anatomically implausible.*left\/right pair/)
const reflectionDiagnostic=diagnoseReflectionAlternative(reportedCt,reportedMri)
assert.ok(reflectionDiagnostic);assert.ok(reflectionDiagnostic.handednessReversalFraction>.9);assert.deepEqual(reflectionDiagnostic.suspectedSwappedPair,[5,6]);assert.ok(reflectionDiagnostic.reflectedRms<3)
const correctedCt=structuredClone(reportedCt);[correctedCt[4],correctedCt[5]]=[correctedCt[5],correctedCt[4]]
const correctedFit=fitAffine(correctedCt,reportedMri)
assert.ok(correctedFit.determinant>0);assert.ok(correctedFit.rms<3);assert.ok(correctedFit.diagnostics.shearMagnitude<.1)
assert.equal(diagnoseReflectionAlternative(correctedCt,reportedMri),null)
console.log('linear registration model regression checks passed')
