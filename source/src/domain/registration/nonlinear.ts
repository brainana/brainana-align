import { applyMat4, type Mat4, type Vec3 } from './rigid'

export type TpsTransform = {
  controlPoints: Vec3[]
  weights: Vec3[]
  affine: [Vec3, Vec3, Vec3, Vec3]
  regularization: number
}

export type LandmarkWarp = {
  kind: 'landmark-tps'
  forward: TpsTransform
  reverse: TpsTransform
  regularization: number
  landmarkRms: number
  jacobian: JacobianSummary
}

export type JacobianSummary = {
  minimum: number
  maximum: number
  nonPositive: number
  evaluated: number
  maximumDisplacement: number
}

function solve(matrix: number[][], rhs: number[]): number[] {
  const augmented = matrix.map((row, index) => [...row, rhs[index]])
  for (let column = 0; column < rhs.length; column++) {
    let pivot = column
    for (let row = column + 1; row < rhs.length; row++) {
      if (Math.abs(augmented[row][column]) > Math.abs(augmented[pivot][column])) pivot = row
    }
    if (Math.abs(augmented[pivot][column]) < 1e-11) {
      throw new Error('The nonlinear landmark geometry is rank deficient. Add more widely distributed, non-coplanar landmarks.')
    }
    ;[augmented[column], augmented[pivot]] = [augmented[pivot], augmented[column]]
    const divisor = augmented[column][column]
    for (let index = column; index <= rhs.length; index++) augmented[column][index] /= divisor
    for (let row = 0; row < rhs.length; row++) {
      if (row === column) continue
      const factor = augmented[row][column]
      if (factor === 0) continue
      for (let index = column; index <= rhs.length; index++) augmented[row][index] -= factor * augmented[column][index]
    }
  }
  return augmented.map(row => row[rhs.length])
}

const distance = (a: Vec3, b: Vec3) => Math.hypot(a[0] - b[0], a[1] - b[1], a[2] - b[2])

// The three-dimensional order-2 polyharmonic spline kernel is U(r)=r.
// It is the 3D analogue commonly called a thin-plate-spline landmark warp.
export function fitTps(source: Vec3[], target: Vec3[], regularization = 0.05): TpsTransform {
  if (source.length !== target.length || source.length < 4) throw new Error('At least four complete landmark pairs are required for nonlinear landmark warping.')
  if (![...source, ...target].flat().every(Number.isFinite)) throw new Error('Landmark coordinates must be finite.')
  if (!Number.isFinite(regularization) || regularization < 0) throw new Error('TPS regularization must be zero or positive.')
  const count = source.length, size = count + 4
  const system = Array.from({ length: size }, () => Array(size).fill(0))
  let scale = 0, pairs = 0
  for (let i = 0; i < count; i++) for (let j = i + 1; j < count; j++) { scale += distance(source[i], source[j]); pairs++ }
  scale = pairs ? scale / pairs : 1
  const lambda = regularization * Math.max(1e-6, scale)
  for (let row = 0; row < count; row++) {
    for (let column = 0; column < count; column++) system[row][column] = distance(source[row], source[column])
    system[row][row] += lambda
    system[row][count] = 1
    system[row][count + 1] = source[row][0]
    system[row][count + 2] = source[row][1]
    system[row][count + 3] = source[row][2]
    system[count][row] = 1
    system[count + 1][row] = source[row][0]
    system[count + 2][row] = source[row][1]
    system[count + 3][row] = source[row][2]
  }
  const solutions = ([0, 1, 2] as const).map(axis => solve(system.map(row => [...row]), [...target.map(point => point[axis]), 0, 0, 0, 0]))
  return {
    controlPoints: source.map(point => [...point] as Vec3),
    weights: source.map((_, index) => solutions.map(solution => solution[index]) as Vec3),
    affine: [0, 1, 2, 3].map(offset => solutions.map(solution => solution[count + offset]) as Vec3) as [Vec3, Vec3, Vec3, Vec3],
    regularization,
  }
}

export function applyTps(transform: TpsTransform, point: Vec3): Vec3 {
  const result = transform.affine[0].map((value, axis) => value
    + transform.affine[1][axis] * point[0]
    + transform.affine[2][axis] * point[1]
    + transform.affine[3][axis] * point[2]) as Vec3
  for (let index = 0; index < transform.controlPoints.length; index++) {
    const radial = distance(point, transform.controlPoints[index])
    for (let axis = 0; axis < 3; axis++) result[axis] += transform.weights[index][axis] * radial
  }
  return result
}

export function applyLandmarkWarpForward(linear: Mat4, warp: LandmarkWarp | null, point: Vec3): Vec3 {
  const mapped = applyMat4(linear, point)
  return warp ? applyTps(warp.forward, mapped) : mapped
}

export function applyLandmarkWarpReverse(linearInverse: Mat4, warp: LandmarkWarp | null, point: Vec3): Vec3 {
  const unwarped = warp ? applyTps(warp.reverse, point) : point
  return applyMat4(linearInverse, unwarped)
}

function determinant3(matrix: number[][]): number {
  const [a,b,c]=matrix[0], [d,e,f]=matrix[1], [g,h,i]=matrix[2]
  return a*(e*i-f*h)-b*(d*i-f*g)+c*(d*h-e*g)
}

export function summarizeJacobian(transform: TpsTransform, bounds: { min: Vec3; max: Vec3 }, samples = 9): JacobianSummary {
  let minimum=Infinity, maximum=-Infinity, nonPositive=0, evaluated=0, maximumDisplacement=0
  const span = bounds.max.map((value, axis) => value - bounds.min[axis]) as Vec3
  const epsilon = Math.max(0.1, Math.min(...span.filter(value => value > 0)) / Math.max(16, samples * 2))
  for(let iz=0;iz<samples;iz++)for(let iy=0;iy<samples;iy++)for(let ix=0;ix<samples;ix++){
    const point:Vec3=[bounds.min[0]+span[0]*(ix+.5)/samples,bounds.min[1]+span[1]*(iy+.5)/samples,bounds.min[2]+span[2]*(iz+.5)/samples]
    const mapped=applyTps(transform,point)
    maximumDisplacement=Math.max(maximumDisplacement,distance(mapped,point))
    const jacobian=Array.from({length:3},()=>Array(3).fill(0))
    for(let input=0;input<3;input++){
      const low=[...point] as Vec3, high=[...point] as Vec3;low[input]-=epsilon;high[input]+=epsilon
      const a=applyTps(transform,low),b=applyTps(transform,high)
      for(let output=0;output<3;output++)jacobian[output][input]=(b[output]-a[output])/(2*epsilon)
    }
    const determinant=determinant3(jacobian)
    minimum=Math.min(minimum,determinant);maximum=Math.max(maximum,determinant);if(determinant<=0)nonPositive++;evaluated++
  }
  return {minimum,maximum,nonPositive,evaluated,maximumDisplacement}
}

export function fitLandmarkWarp(linear: Mat4, source: Vec3[], target: Vec3[], regularization: number): LandmarkWarp {
  const linearlyMapped = source.map(point => applyMat4(linear, point))
  const forward = fitTps(linearlyMapped, target, regularization)
  const reverse = fitTps(target, linearlyMapped, regularization)
  const residuals = linearlyMapped.map((point,index)=>distance(applyTps(forward,point),target[index]))
  const landmarkRms=Math.sqrt(residuals.reduce((sum,value)=>sum+value*value,0)/residuals.length)
  const all=[...linearlyMapped,...target]
  const bounds={min:[0,1,2].map(axis=>Math.min(...all.map(point=>point[axis]))) as Vec3,max:[0,1,2].map(axis=>Math.max(...all.map(point=>point[axis]))) as Vec3}
  const jacobian=summarizeJacobian(forward,bounds)
  if(jacobian.nonPositive)throw new Error(`The proposed landmark warp folds in ${jacobian.nonPositive} sampled locations. Increase regularization or improve landmark distribution.`)
  return {kind:'landmark-tps',forward,reverse,regularization,landmarkRms,jacobian}
}
