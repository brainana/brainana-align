import type {Mat4} from './rigid'
const rad=Math.PI/180,deg=180/Math.PI
/** Rz Ry Rx pose rotation, independent of scale/shear and translation. */
export function rotationFromDegrees(p:number[]):number[][]{
 const [x,y,z]=p.map(v=>v*rad),cx=Math.cos(x),sx=Math.sin(x),cy=Math.cos(y),sy=Math.sin(y),cz=Math.cos(z),sz=Math.sin(z)
 return [[cz*cy,cz*sy*sx-sz*cx,cz*sy*cx+sz*sx],[sz*cy,sz*sy*sx+cz*cx,sz*sy*cx-cz*sx],[-sy,cy*sx,cy*cx]]
}
export function rotationDistanceDegrees(a:number[],b:number[]){const x=rotationFromDegrees(a),y=rotationFromDegrees(b);let trace=0;for(let i=0;i<3;i++)for(let j=0;j<3;j++)trace+=x[i][j]*y[i][j];return Math.acos(Math.max(-1,Math.min(1,(trace-1)/2)))*deg}
function euler(r:number[][]):number[]{const y=Math.asin(Math.max(-1,Math.min(1,-r[2][0]))),c=Math.cos(y);return [Math.abs(c)>1e-8?Math.atan2(r[2][1],r[2][2]):Math.atan2(-r[1][2],r[1][1]),y,Math.abs(c)>1e-8?Math.atan2(r[1][0],r[0][0]):0].map(v=>v*deg)}
/** Deterministic distributed SO(3) starts plus the 24 proper cube orientations.
 * This is an initializer, not a global convergence guarantee or a reflection search. */
export function distributedOrientationSeeds(count=512):number[][]{
 if(!Number.isInteger(count)||count<32||count>1024)throw Error('Orientation seed count must be between32 and1024')
 const result:number[][]=[]
 const add=(p:number[])=>{if(!result.some(q=>rotationDistanceDegrees(p,q)<1e-5))result.push(p)}
 for(const x of [0,90,180,-90])for(const y of [0,90,180,-90])for(const z of [0,90,180,-90])add([x,y,z])
 const fractional=(n:number)=>n-Math.floor(n)
 for(let i=0;i<count;i++){
  const u=(i+.5)/count,a=2*Math.PI*fractional(i*((Math.sqrt(5)-1)/2)),b=2*Math.PI*fractional(i*Math.SQRT2),x=Math.sqrt(1-u)*Math.sin(a),y=Math.sqrt(1-u)*Math.cos(a),z=Math.sqrt(u)*Math.sin(b),w=Math.sqrt(u)*Math.cos(b)
  add(euler([[1-2*(y*y+z*z),2*(x*y-z*w),2*(x*z+y*w)],[2*(x*y+z*w),1-2*(x*x+z*z),2*(y*z-x*w)],[2*(x*z-y*w),2*(y*z+x*w),1-2*(x*x+y*y)]]))
 }
 return result
}
export function rotationMatrix4(angles:number[]):Mat4{return [...rotationFromDegrees(angles).map(r=>[...r,0]),[0,0,0,1]]}
