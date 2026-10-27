export type Vec3 = [number, number, number]
export type Mat4 = number[][]

const centroid = (pts: Vec3[]): Vec3 => {
  const c: Vec3 = [0, 0, 0]
  for (const p of pts) { c[0] += p[0]; c[1] += p[1]; c[2] += p[2] }
  return [c[0] / pts.length, c[1] / pts.length, c[2] / pts.length]
}

function normalize4(q: number[]): number[] {
  const n = Math.hypot(...q) || 1
  return q.map(v => v / n)
}

/**
 * Return the eigenvector associated with the largest algebraic eigenvalue of a
 * real symmetric 4 x 4 matrix.
 *
 * Horn's quaternion rigid-fit matrix can have a negative eigenvalue whose
 * absolute magnitude exceeds that of the desired positive eigenvalue. Power
 * iteration therefore is not reliable here: it selects the largest-magnitude
 * eigenpair and can fail for large (especially near-180 degree) rotations.
 * Jacobi rotations explicitly diagonalize the symmetric matrix and let us
 * select the largest algebraic eigenvalue instead.
 */
function largestEigenvectorSymmetric4(input: number[][]): number[] {
  const scale = Math.max(...input.flat().map(Math.abs)) || 1
  const a = input.map(row => row.map(v => v / scale))
  const v: number[][] = Array.from({ length: 4 }, (_, r) => Array.from({ length: 4 }, (_, c) => r === c ? 1 : 0))
  const tolerance = 1e-14

  for (let iteration = 0; iteration < 80; iteration += 1) {
    let p: number = 0
    let q: number = 1
    let largest = Math.abs(a[p][q])
    for (let r = 0; r < 4; r += 1) {
      for (let c = r + 1; c < 4; c += 1) {
        const candidate = Math.abs(a[r][c])
        if (candidate > largest) {
          largest = candidate
          p = r
          q = c
        }
      }
    }
    if (largest < tolerance) break

    const app = a[p][p]
    const aqq = a[q][q]
    const apq = a[p][q]
    const angle = 0.5 * Math.atan2(2 * apq, aqq - app)
    const cosine = Math.cos(angle)
    const sine = Math.sin(angle)

    for (let k = 0; k < 4; k += 1) {
      if (k === p || k === q) continue
      const akp = a[k][p]
      const akq = a[k][q]
      a[k][p] = a[p][k] = cosine * akp - sine * akq
      a[k][q] = a[q][k] = sine * akp + cosine * akq
    }

    a[p][p] = cosine * cosine * app - 2 * sine * cosine * apq + sine * sine * aqq
    a[q][q] = sine * sine * app + 2 * sine * cosine * apq + cosine * cosine * aqq
    a[p][q] = a[q][p] = 0

    for (let k = 0; k < 4; k += 1) {
      const vkp = v[k][p]
      const vkq = v[k][q]
      v[k][p] = cosine * vkp - sine * vkq
      v[k][q] = sine * vkp + cosine * vkq
    }
  }

  let index = 0
  for (let i = 1; i < 4; i += 1) if (a[i][i] > a[index][index]) index = i
  return normalize4(v.map(row => row[index]))
}

function landmarkGeometryScale(points: Vec3[]): number {
  const c = centroid(points)
  return Math.sqrt(points.reduce((sum, point) => {
    const dx = point[0] - c[0]
    const dy = point[1] - c[1]
    const dz = point[2] - c[2]
    return sum + dx * dx + dy * dy + dz * dz
  }, 0) / points.length)
}

function maximumTriangleArea2(points: Vec3[]): number {
  let first = 0, second = 1, longest = 0
  for (let i = 0; i < points.length; i++) for (let j = i + 1; j < points.length; j++) {
    const length = Math.hypot(...points[i].map((v, k) => v - points[j][k]))
    if (length > longest) { longest = length; first = i; second = j }
  }
  const a = points[second].map((v, i) => v - points[first][i])
  let maximum = 0
  for (const p of points) {
    const b = p.map((v, i) => v - points[first][i])
    maximum = Math.max(maximum, Math.hypot(a[1]*b[2]-a[2]*b[1], a[2]*b[0]-a[0]*b[2], a[0]*b[1]-a[1]*b[0]))
  }
  return maximum
}

function validateLandmarkGeometry(points: Vec3[], label: string) {
  const scale = landmarkGeometryScale(points)
  if (!Number.isFinite(scale) || scale < 1e-6) throw new Error(`${label} landmarks occupy effectively one location`)
  const area2 = maximumTriangleArea2(points)
  if (area2 < scale * scale * 1e-6) throw new Error(`${label} landmarks are collinear or too narrowly distributed for a stable 3D rigid fit`)
}

function quaternionToRotation(qIn: number[]): number[][] {
  const [w, x, y, z] = normalize4(qIn)
  return [
    [1 - 2 * (y*y + z*z), 2 * (x*y - z*w), 2 * (x*z + y*w)],
    [2 * (x*y + z*w), 1 - 2 * (x*x + z*z), 2 * (y*z - x*w)],
    [2 * (x*z - y*w), 2 * (y*z + x*w), 1 - 2 * (x*x + y*y)]
  ]
}

export function applyMat4(m: Mat4, p: Vec3): Vec3 {
  return [
    m[0][0]*p[0] + m[0][1]*p[1] + m[0][2]*p[2] + m[0][3],
    m[1][0]*p[0] + m[1][1]*p[1] + m[1][2]*p[2] + m[1][3],
    m[2][0]*p[0] + m[2][1]*p[1] + m[2][2]*p[2] + m[2][3]
  ]
}

export function invertRigid(m: Mat4): Mat4 {
  const r = m.slice(0,3).map(row => row.slice(0,3))
  const rt = [[r[0][0],r[1][0],r[2][0]],[r[0][1],r[1][1],r[2][1]],[r[0][2],r[1][2],r[2][2]]]
  const t: Vec3 = [m[0][3],m[1][3],m[2][3]]
  const ti: Vec3 = [-(rt[0][0]*t[0]+rt[0][1]*t[1]+rt[0][2]*t[2]),-(rt[1][0]*t[0]+rt[1][1]*t[1]+rt[1][2]*t[2]),-(rt[2][0]*t[0]+rt[2][1]*t[1]+rt[2][2]*t[2])]
  return [[...rt[0],ti[0]],[...rt[1],ti[1]],[...rt[2],ti[2]],[0,0,0,1]]
}

export function fitRigid(source: Vec3[], target: Vec3[]) {
  if (![...source, ...target].every(p => p.length === 3 && p.every(Number.isFinite))) throw new Error('Landmark coordinates must be finite physical millimeters')
  if (source.length !== target.length || source.length < 3) throw new Error('At least three complete landmark pairs are required')
  validateLandmarkGeometry(source, 'Moving')
  validateLandmarkGeometry(target, 'Fixed')
  const cs = centroid(source), ct = centroid(target)
  let sxx=0,sxy=0,sxz=0,syx=0,syy=0,syz=0,szx=0,szy=0,szz=0
  for (let i=0;i<source.length;i++) {
    const x=[source[i][0]-cs[0],source[i][1]-cs[1],source[i][2]-cs[2]]
    const y=[target[i][0]-ct[0],target[i][1]-ct[1],target[i][2]-ct[2]]
    sxx+=x[0]*y[0]; sxy+=x[0]*y[1]; sxz+=x[0]*y[2]
    syx+=x[1]*y[0]; syy+=x[1]*y[1]; syz+=x[1]*y[2]
    szx+=x[2]*y[0]; szy+=x[2]*y[1]; szz+=x[2]*y[2]
  }
  const trace=sxx+syy+szz
  const n=[
    [trace, syz-szy, szx-sxz, sxy-syx],
    [syz-szy, sxx-syy-szz, sxy+syx, szx+sxz],
    [szx-sxz, sxy+syx, -sxx+syy-szz, syz+szy],
    [sxy-syx, szx+sxz, syz+szy, -sxx-syy+szz]
  ]
  const r=quaternionToRotation(largestEigenvectorSymmetric4(n))
  const rc: Vec3=[r[0][0]*cs[0]+r[0][1]*cs[1]+r[0][2]*cs[2],r[1][0]*cs[0]+r[1][1]*cs[1]+r[1][2]*cs[2],r[2][0]*cs[0]+r[2][1]*cs[1]+r[2][2]*cs[2]]
  const t: Vec3=[ct[0]-rc[0],ct[1]-rc[1],ct[2]-rc[2]]
  const matrix: Mat4=[[...r[0],t[0]],[...r[1],t[1]],[...r[2],t[2]],[0,0,0,1]]
  const residuals=source.map((p,i)=>{const q=applyMat4(matrix,p);return Math.hypot(q[0]-target[i][0],q[1]-target[i][1],q[2]-target[i][2])})
  const rms=Math.sqrt(residuals.reduce((s,v)=>s+v*v,0)/residuals.length)
  return {matrix,inverse:invertRigid(matrix),residuals,rms}
}


export function multiplyMat4(a: Mat4, b: Mat4): Mat4 {
  return Array.from({ length: 4 }, (_, r) => Array.from({ length: 4 }, (_, c) =>
    a[r][0] * b[0][c] + a[r][1] * b[1][c] + a[r][2] * b[2][c] + a[r][3] * b[3][c]
  ))
}

export function rigidDelta(params: [number, number, number, number, number, number], center: Vec3): Mat4 {
  const [tx, ty, tz, rxDeg, ryDeg, rzDeg] = params
  const rx = rxDeg * Math.PI / 180
  const ry = ryDeg * Math.PI / 180
  const rz = rzDeg * Math.PI / 180
  const cx = Math.cos(rx), sx = Math.sin(rx)
  const cy = Math.cos(ry), sy = Math.sin(ry)
  const cz = Math.cos(rz), sz = Math.sin(rz)
  const Rx: Mat4 = [[1,0,0,0],[0,cx,-sx,0],[0,sx,cx,0],[0,0,0,1]]
  const Ry: Mat4 = [[cy,0,sy,0],[0,1,0,0],[-sy,0,cy,0],[0,0,0,1]]
  const Rz: Mat4 = [[cz,-sz,0,0],[sz,cz,0,0],[0,0,1,0],[0,0,0,1]]
  const Tneg: Mat4 = [[1,0,0,-center[0]],[0,1,0,-center[1]],[0,0,1,-center[2]],[0,0,0,1]]
  const Tpos: Mat4 = [[1,0,0,center[0] + tx],[0,1,0,center[1] + ty],[0,0,1,center[2] + tz],[0,0,0,1]]
  return multiplyMat4(Tpos, multiplyMat4(Rz, multiplyMat4(Ry, multiplyMat4(Rx, Tneg))))
}
