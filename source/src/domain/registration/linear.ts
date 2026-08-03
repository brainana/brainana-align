import { applyMat4, fitRigid, multiplyMat4, type Mat4, type Vec3 } from './rigid'

export const LINEAR_PARAMETERS = [
  'tx', 'ty', 'tz', 'rx', 'ry', 'rz', 'sx', 'sy', 'sz', 'shxy', 'shxz', 'shyz',
] as const
export type LinearParameter = typeof LINEAR_PARAMETERS[number]
export type TransformModel = 'rigid' | 'similarity' | 'anisotropic' | 'affine' | 'custom'

export type LinearDiagnostics = {
  parameterCount: number
  scales: Vec3
  shearMagnitude: number
  reflected: boolean
  conditionEstimate: number
  warnings: string[]
}

export type LinearFit = {
  matrix: Mat4
  inverse: Mat4
  residuals: number[]
  rms: number
  determinant: number
  model: TransformModel
  enabledParameters: LinearParameter[]
  diagnostics: LinearDiagnostics
}

export type ReflectionAlternativeDiagnostic={
  reflectedRms:number
  determinant:number
  scales:Vec3
  shearMagnitude:number
  handednessReversalFraction:number
  suspectedSwappedPair:[number,number]|null
}

const centroid = (points: Vec3[]): Vec3 => {
  const total: Vec3 = [0, 0, 0]
  for (const point of points) for (let axis = 0; axis < 3; axis++) total[axis] += point[axis]
  return total.map(value => value / points.length) as Vec3
}

export function determinant3(matrix: Mat4): number {
  const a = matrix[0][0], b = matrix[0][1], c = matrix[0][2]
  const d = matrix[1][0], e = matrix[1][1], f = matrix[1][2]
  const g = matrix[2][0], h = matrix[2][1], i = matrix[2][2]
  return a * (e * i - f * h) - b * (d * i - f * g) + c * (d * h - e * g)
}

export function linearDiagnostics(matrix:Mat4,parameterCount:number):LinearDiagnostics{
  const columns=([0,1,2] as const).map(column=>[matrix[0][column],matrix[1][column],matrix[2][column]] as Vec3)
  const scales=columns.map(column=>Math.hypot(...column)) as Vec3
  const normalized=columns.map((column,index)=>column.map(value=>value/Math.max(scales[index],1e-15)) as Vec3)
  const dot=(a:Vec3,b:Vec3)=>a.reduce((sum,value,index)=>sum+value*b[index],0)
  const shearMagnitude=Math.hypot(dot(normalized[0],normalized[1]),dot(normalized[0],normalized[2]),dot(normalized[1],normalized[2]))
  const determinant=determinant3(matrix),minimum=Math.min(...scales),maximum=Math.max(...scales),conditionEstimate=maximum/Math.max(minimum,1e-15)
  const warnings:string[]=[]
  if(minimum<.5||maximum>2)warnings.push(`Extreme axis scale detected (${scales.map(value=>value.toFixed(3)).join(', ')}).`)
  if(shearMagnitude>.15)warnings.push(`Substantial affine shear detected (${shearMagnitude.toFixed(3)}).`)
  if(Math.abs(determinant)<1e-4)warnings.push('The linear determinant is close to zero and the transform is unstable.')
  if(determinant<0)warnings.push('Reflection detected: this transform mirrors anatomy and may indicate a left-right or coordinate-convention problem.')
  if(conditionEstimate>20)warnings.push(`The linear transform is poorly conditioned (${conditionEstimate.toFixed(1)}).`)
  return {parameterCount,scales,shearMagnitude,reflected:determinant<0,conditionEstimate,warnings}
}

function withDiagnostics(fit:Omit<LinearFit,'diagnostics'>,parameterCount:number):LinearFit{return {...fit,diagnostics:linearDiagnostics(fit.matrix,parameterCount)}}

export function invertAffineMatrix(matrix: Mat4): Mat4 {
  const determinant = determinant3(matrix)
  if (!Number.isFinite(determinant) || Math.abs(determinant) < 1e-10) {
    throw new Error('The fitted transform is singular and cannot be inverted.')
  }
  const a = matrix[0][0], b = matrix[0][1], c = matrix[0][2]
  const d = matrix[1][0], e = matrix[1][1], f = matrix[1][2]
  const g = matrix[2][0], h = matrix[2][1], i = matrix[2][2]
  const inverse3 = [
    [(e*i-f*h)/determinant, (c*h-b*i)/determinant, (b*f-c*e)/determinant],
    [(f*g-d*i)/determinant, (a*i-c*g)/determinant, (c*d-a*f)/determinant],
    [(d*h-e*g)/determinant, (b*g-a*h)/determinant, (a*e-b*d)/determinant],
  ]
  const translation: Vec3 = [matrix[0][3], matrix[1][3], matrix[2][3]]
  const inverseTranslation = inverse3.map(row => -row.reduce((sum, value, axis) => sum + value * translation[axis], 0))
  return [
    [...inverse3[0], inverseTranslation[0]],
    [...inverse3[1], inverseTranslation[1]],
    [...inverse3[2], inverseTranslation[2]],
    [0, 0, 0, 1],
  ]
}

function residualSummary(matrix: Mat4, source: Vec3[], target: Vec3[]) {
  const residuals = source.map((point, index) => {
    const transformed = applyMat4(matrix, point)
    return Math.hypot(...transformed.map((value, axis) => value - target[index][axis]))
  })
  return { residuals, rms: Math.sqrt(residuals.reduce((sum, value) => sum + value * value, 0) / residuals.length) }
}

function validatePairs(source: Vec3[], target: Vec3[], minimum: number): void {
  if (source.length !== target.length || source.length < minimum) {
    throw new Error(`At least ${minimum} complete landmark pairs are required for this transform model.`)
  }
  if (![...source, ...target].flat().every(Number.isFinite)) throw new Error('Landmark coordinates must be finite numbers.')
}

export function fitSimilarity(source: Vec3[], target: Vec3[]): LinearFit {
  validatePairs(source, target, 3)
  const rigid = fitRigid(source, target)
  const sourceCenter = centroid(source), targetCenter = centroid(target)
  const rotation = rigid.matrix.slice(0, 3).map(row => row.slice(0, 3))
  let numerator = 0, denominator = 0
  for (let index = 0; index < source.length; index++) {
    const centeredSource = source[index].map((value, axis) => value - sourceCenter[axis]) as Vec3
    const centeredTarget = target[index].map((value, axis) => value - targetCenter[axis]) as Vec3
    const rotated = rotation.map(row => row.reduce((sum, value, axis) => sum + value * centeredSource[axis], 0))
    numerator += rotated.reduce((sum, value, axis) => sum + value * centeredTarget[axis], 0)
    denominator += centeredSource.reduce((sum, value) => sum + value * value, 0)
  }
  if (denominator < 1e-10) throw new Error('The landmarks do not span enough space to estimate scale.')
  const scale = numerator / denominator
  if (!Number.isFinite(scale) || scale <= 0) throw new Error('The fitted uniform scale is invalid.')
  const linear = rotation.map(row => row.map(value => value * scale))
  const mappedCenter = linear.map(row => row.reduce((sum, value, axis) => sum + value * sourceCenter[axis], 0))
  const translation = targetCenter.map((value, axis) => value - mappedCenter[axis])
  const matrix: Mat4 = [
    [...linear[0], translation[0]], [...linear[1], translation[1]], [...linear[2], translation[2]], [0,0,0,1],
  ]
  const summary = residualSummary(matrix, source, target)
  return withDiagnostics({ ...summary, matrix, inverse: invertAffineMatrix(matrix), determinant: determinant3(matrix), model: 'similarity', enabledParameters: ['tx','ty','tz','rx','ry','rz','sx'] },7)
}

export function fitAnisotropic(source:Vec3[],target:Vec3[]):LinearFit{
  validatePairs(source,target,4)
  const sourceCenter=centroid(source),targetCenter=centroid(target)
  const centeredSource=source.map(point=>point.map((value,axis)=>value-sourceCenter[axis]) as Vec3)
  const centeredTarget=target.map(point=>point.map((value,axis)=>value-targetCenter[axis]) as Vec3)
  let scales:Vec3=[1,1,1],rotation=fitRigid(centeredSource,centeredTarget).matrix
  for(let iteration=0;iteration<40;iteration++){
    const next=([0,1,2] as const).map(axis=>{
      let numerator=0,denominator=0
      for(let index=0;index<source.length;index++){
        const rotatedTarget=rotation[0][axis]*centeredTarget[index][0]+rotation[1][axis]*centeredTarget[index][1]+rotation[2][axis]*centeredTarget[index][2]
        numerator+=centeredSource[index][axis]*rotatedTarget;denominator+=centeredSource[index][axis]**2
      }
      if(denominator<1e-10)throw new Error('The landmarks do not span all three axes sufficiently to estimate anisotropic scale.')
      return numerator/denominator
    }) as Vec3
    if(next.some(value=>!Number.isFinite(value)||value<=0))throw new Error('The anisotropic fit requires a reflected or invalid axis scale. Check landmark correspondence and orientation.')
    const scaled=centeredSource.map(point=>point.map((value,axis)=>value*next[axis]) as Vec3)
    const nextRotation=fitRigid(scaled,centeredTarget).matrix
    const change=Math.max(...next.map((value,axis)=>Math.abs(value-scales[axis])),...nextRotation.slice(0,3).flatMap((row,r)=>row.slice(0,3).map((value,c)=>Math.abs(value-rotation[r][c]))))
    scales=next;rotation=nextRotation;if(change<1e-11)break
  }
  const linear=rotation.slice(0,3).map(row=>row.slice(0,3).map((value,axis)=>value*scales[axis]))
  const mappedCenter=linear.map(row=>row.reduce((sum,value,axis)=>sum+value*sourceCenter[axis],0))
  const translation=targetCenter.map((value,axis)=>value-mappedCenter[axis])
  const matrix:Mat4=[[...linear[0],translation[0]],[...linear[1],translation[1]],[...linear[2],translation[2]],[0,0,0,1]]
  const determinant=determinant3(matrix);if(determinant<=1e-8)throw new Error('The anisotropic fit is singular or reflected.')
  const summary=residualSummary(matrix,source,target)
  return withDiagnostics({...summary,matrix,inverse:invertAffineMatrix(matrix),determinant,model:'anisotropic',enabledParameters:['tx','ty','tz','rx','ry','rz','sx','sy','sz']},9)
}

function solveLinearSystem(matrix: number[][], values: number[]): number[] {
  const augmented = matrix.map((row, index) => [...row, values[index]])
  for (let column = 0; column < values.length; column++) {
    let pivot = column
    for (let row = column + 1; row < values.length; row++) if (Math.abs(augmented[row][column]) > Math.abs(augmented[pivot][column])) pivot = row
    if (Math.abs(augmented[pivot][column]) < 1e-10) throw new Error('The landmark geometry is rank deficient. Use more widely distributed, non-coplanar landmarks.')
    ;[augmented[column], augmented[pivot]] = [augmented[pivot], augmented[column]]
    const divisor = augmented[column][column]
    for (let index = column; index <= values.length; index++) augmented[column][index] /= divisor
    for (let row = 0; row < values.length; row++) {
      if (row === column) continue
      const factor = augmented[row][column]
      for (let index = column; index <= values.length; index++) augmented[row][index] -= factor * augmented[column][index]
    }
  }
  return augmented.map(row => row[values.length])
}

function validateAffineGeometry(points:Vec3[]):void{
  const normal=Array.from({length:4},()=>Array(4).fill(0))
  for(const point of points){const row=[...point,1];for(let a=0;a<4;a++)for(let b=0;b<4;b++)normal[a][b]+=row[a]*row[b]}
  solveLinearSystem(normal,[0,0,0,0])
}

function fitUnrestrictedAffine(source: Vec3[], target: Vec3[]): LinearFit {
  validatePairs(source, target, 4)
  const normal = Array.from({ length: 4 }, () => Array(4).fill(0))
  const rhs = Array.from({ length: 3 }, () => Array(4).fill(0))
  for (let index = 0; index < source.length; index++) {
    const row = [...source[index], 1]
    for (let a = 0; a < 4; a++) {
      for (let b = 0; b < 4; b++) normal[a][b] += row[a] * row[b]
      for (let output = 0; output < 3; output++) rhs[output][a] += row[a] * target[index][output]
    }
  }
  const coefficients = rhs.map(values => solveLinearSystem(normal.map(row => [...row]), values))
  const matrix: Mat4 = [[...coefficients[0]], [...coefficients[1]], [...coefficients[2]], [0,0,0,1]]
  const determinant = determinant3(matrix)
  if (Math.abs(determinant) <= 1e-6) throw new Error('The affine solution is nearly singular. Use more widely distributed, non-coplanar landmarks.')
  const summary = residualSummary(matrix, source, target)
  return withDiagnostics({ ...summary, matrix, inverse: invertAffineMatrix(matrix), determinant, model: 'affine', enabledParameters: [...LINEAR_PARAMETERS] },12)
}

function parameterMatrix(parameters: number[], center: Vec3): Mat4 {
  const [tx,ty,tz,rx,ry,rz,lsx,lsy,lsz,shxy,shxz,shyz] = parameters
  const cx=Math.cos(rx), sx=Math.sin(rx), cy=Math.cos(ry), sy=Math.sin(ry), cz=Math.cos(rz), sz=Math.sin(rz)
  const rotation: Mat4 = [
    [cz*cy, cz*sy*sx-sz*cx, cz*sy*cx+sz*sx, 0],
    [sz*cy, sz*sy*sx+cz*cx, sz*sy*cx-cz*sx, 0],
    [-sy, cy*sx, cy*cx, 0], [0,0,0,1],
  ]
  const scaleShear: Mat4 = [[Math.exp(lsx),shxy,shxz,0],[0,Math.exp(lsy),shyz,0],[0,0,Math.exp(lsz),0],[0,0,0,1]]
  const before: Mat4 = [[1,0,0,-center[0]],[0,1,0,-center[1]],[0,0,1,-center[2]],[0,0,0,1]]
  const after: Mat4 = [[1,0,0,center[0]+tx],[0,1,0,center[1]+ty],[0,0,1,center[2]+tz],[0,0,0,1]]
  return multiplyMat4(after, multiplyMat4(rotation, multiplyMat4(scaleShear, before)))
}

export function fitCustomLinear(source: Vec3[], target: Vec3[], enabledParameters: LinearParameter[]): LinearFit {
  validatePairs(source, target, enabledParameters.some(name => name.startsWith('s') || name.startsWith('sh')) ? 4 : 3)
  if (!enabledParameters.length) throw new Error('Select at least one transform parameter.')
  const enabled = new Set(enabledParameters)
  const active = LINEAR_PARAMETERS.map((name, index) => enabled.has(name) ? index : -1).filter(index => index >= 0)
  const center = centroid(source)
  const parameters = Array(12).fill(0)
  // Begin general fits at the best proper rigid pose. This makes the
  // orientation-preserving parameterization robust to large rotations while
  // leaving disabled custom parameters fixed at identity.
  if (enabledParameters.length === LINEAR_PARAMETERS.length && LINEAR_PARAMETERS.every(parameter => enabled.has(parameter))) {
    const rigid = fitRigid(source, target).matrix
    const cy = Math.hypot(rigid[0][0], rigid[1][0])
    const rx = cy > 1e-10 ? Math.atan2(rigid[2][1], rigid[2][2]) : Math.atan2(-rigid[1][2], rigid[1][1])
    const ry = Math.atan2(-rigid[2][0], cy)
    const rz = cy > 1e-10 ? Math.atan2(rigid[1][0], rigid[0][0]) : 0
    const targetCenter = centroid(target)
    parameters.splice(0, 6, targetCenter[0]-center[0], targetCenter[1]-center[1], targetCenter[2]-center[2], rx, ry, rz)
  }
  const steps = LINEAR_PARAMETERS.map((_, index) => index < 3 ? 1e-3 : index < 6 ? 1e-5 : 1e-5)
  let damping = 1e-3
  const residualVector = (candidate: number[]) => {
    const matrix = parameterMatrix(candidate, center)
    return source.flatMap((point, index) => applyMat4(matrix, point).map((value, axis) => value - target[index][axis]))
  }
  let residual = residualVector(parameters)
  let cost = residual.reduce((sum, value) => sum + value * value, 0)
  for (let iteration = 0; iteration < 80; iteration++) {
    const jacobian = residual.map(() => Array(active.length).fill(0))
    for (let column = 0; column < active.length; column++) {
      const trial = [...parameters], parameterIndex = active[column]
      trial[parameterIndex] += steps[parameterIndex]
      const shifted = residualVector(trial)
      for (let row = 0; row < residual.length; row++) jacobian[row][column] = (shifted[row] - residual[row]) / steps[parameterIndex]
    }
    const normal = Array.from({ length: active.length }, () => Array(active.length).fill(0))
    const gradient = Array(active.length).fill(0)
    for (let row = 0; row < residual.length; row++) for (let a = 0; a < active.length; a++) {
      gradient[a] += jacobian[row][a] * residual[row]
      for (let b = 0; b < active.length; b++) normal[a][b] += jacobian[row][a] * jacobian[row][b]
    }
    for (let index = 0; index < active.length; index++) normal[index][index] += damping
    let delta: number[]
    try { delta = solveLinearSystem(normal, gradient.map(value => -value)) } catch { damping *= 10; continue }
    const trial = [...parameters]
    active.forEach((parameterIndex, index) => { trial[parameterIndex] += delta[index] })
    // Keep the proper affine search away from the singular boundary. These
    // broad 0.1x–10x bounds still trigger plausibility warnings long before
    // they become attractive for normal MRI/CT geometry.
    for (const parameterIndex of [6, 7, 8]) if (active.includes(parameterIndex)) {
      trial[parameterIndex] = Math.max(Math.log(.1), Math.min(Math.log(10), trial[parameterIndex]))
    }
    const trialResidual = residualVector(trial)
    const trialCost = trialResidual.reduce((sum, value) => sum + value * value, 0)
    if (trialCost < cost) {
      parameters.splice(0, parameters.length, ...trial); residual = trialResidual
      if (Math.abs(cost - trialCost) < 1e-10 || Math.hypot(...delta) < 1e-8) { cost = trialCost; break }
      cost = trialCost; damping = Math.max(1e-9, damping / 3)
    } else damping *= 10
  }
  const matrix = parameterMatrix(parameters, center), determinant = determinant3(matrix)
  if (!Number.isFinite(determinant) || determinant <= 1e-6) throw new Error('The selected parameters produced a singular or numerically invalid transform.')
  const summary = residualSummary(matrix, source, target)
  return withDiagnostics({ ...summary, matrix, inverse: invertAffineMatrix(matrix), determinant, model: 'custom', enabledParameters: [...enabledParameters] },enabledParameters.length)
}

export function fitAffine(source: Vec3[], target: Vec3[], allowReflection=false): LinearFit {
  validatePairs(source, target, 4)
  validateAffineGeometry(source)
  validateAffineGeometry(target)
  if (allowReflection) return fitUnrestrictedAffine(source, target)
  // R * U, where R is a proper rotation and U is upper triangular with
  // positive exponential diagonal entries, spans the orientation-preserving
  // affine group. A reflected candidate is therefore never evaluated or
  // returned when reflection is disabled.
  const constrained = fitCustomLinear(source, target, [...LINEAR_PARAMETERS])
  const diagnostics=linearDiagnostics(constrained.matrix,12)
  const minimumScale=Math.min(...diagnostics.scales),maximumScale=Math.max(...diagnostics.scales)
  if(minimumScale<.2||maximumScale>5||diagnostics.shearMagnitude>.75||constrained.determinant<.01){
    throw new Error(`The landmarks do not support a stable orientation-preserving affine transform. The best proper fit is anatomically implausible (scales ${diagnostics.scales.map(value=>value.toFixed(3)).join(', ')}, shear ${diagnostics.shearMagnitude.toFixed(3)}, determinant ${constrained.determinant.toFixed(5)}). Check landmark correspondence, especially the left/right pair. No transform was applied.`)
  }
  return { ...constrained, model: 'affine', diagnostics }
}

function signedTetrahedron(a:Vec3,b:Vec3,c:Vec3,d:Vec3):number{
  const u=b.map((value,axis)=>value-a[axis]) as Vec3,v=c.map((value,axis)=>value-a[axis]) as Vec3,w=d.map((value,axis)=>value-a[axis]) as Vec3
  return u[0]*(v[1]*w[2]-v[2]*w[1])-u[1]*(v[0]*w[2]-v[2]*w[0])+u[2]*(v[0]*w[1]-v[1]*w[0])
}

function weightedHandednessReversal(source:Vec3[],target:Vec3[]):number{
  let reversed=0,total=0
  for(let a=0;a<source.length-3;a++)for(let b=a+1;b<source.length-2;b++)for(let c=b+1;c<source.length-1;c++)for(let d=c+1;d<source.length;d++){
    const sv=signedTetrahedron(source[a],source[b],source[c],source[d]),tv=signedTetrahedron(target[a],target[b],target[c],target[d])
    const weight=Math.sqrt(Math.abs(sv*tv));if(!Number.isFinite(weight)||weight<1e-6)continue
    total+=weight;if(sv*tv<0)reversed+=weight
  }
  return total>0?reversed/total:0
}

const plausibleDiagnostic=(fit:LinearFit)=>Math.min(...fit.diagnostics.scales)>=.5&&Math.max(...fit.diagnostics.scales)<=2&&fit.diagnostics.shearMagnitude<.25&&fit.diagnostics.conditionEstimate<5&&Math.abs(fit.determinant)>.1

export function diagnoseReflectionAlternative(source:Vec3[],target:Vec3[]):ReflectionAlternativeDiagnostic|null{
  if(source.length!==target.length||source.length<6)return null
  let reflected:LinearFit
  try{reflected=fitUnrestrictedAffine(source,target)}catch{return null}
  const handednessReversalFraction=weightedHandednessReversal(source,target)
  if(reflected.determinant>=0||!plausibleDiagnostic(reflected)||handednessReversalFraction<.8)return null
  const swaps:{pair:[number,number],fit:LinearFit}[]=[]
  if(target.length<=20)for(let first=0;first<target.length-1;first++)for(let second=first+1;second<target.length;second++){
    const swapped=target.map(point=>[...point] as Vec3);[swapped[first],swapped[second]]=[swapped[second],swapped[first]]
    try{const fit=fitAffine(source,swapped,false);if(plausibleDiagnostic(fit))swaps.push({pair:[first+1,second+1],fit})}catch{}
  }
  swaps.sort((a,b)=>a.fit.rms-b.fit.rms)
  const best=swaps[0],second=swaps[1]
  const suspectedSwappedPair=best&&best.fit.rms<=reflected.rms*1.25&&(!second||second.fit.rms>best.fit.rms*1.5)?best.pair:null
  return {reflectedRms:reflected.rms,determinant:reflected.determinant,scales:reflected.diagnostics.scales,shearMagnitude:reflected.diagnostics.shearMagnitude,handednessReversalFraction,suspectedSwappedPair}
}

export function fitLinearModel(source: Vec3[], target: Vec3[], model: TransformModel, enabledParameters: LinearParameter[] = [],options:{allowReflection?:boolean}={}): LinearFit {
  if (model === 'rigid') {
    const result = fitRigid(source, target)
    return withDiagnostics({ ...result, determinant: determinant3(result.matrix), model, enabledParameters: ['tx','ty','tz','rx','ry','rz'] },6)
  }
  if (model === 'similarity') return fitSimilarity(source, target)
  if(model==='anisotropic')return fitAnisotropic(source,target)
  if (model === 'affine') return fitAffine(source, target,Boolean(options.allowReflection))
  return fitCustomLinear(source, target, enabledParameters)
}
