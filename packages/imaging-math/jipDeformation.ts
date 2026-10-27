import {applyMat4,multiplyMat4,type Mat4,type Vec3} from './rigid'
import {invertAffine,determinant3} from './roiWarp'
import {validateAffine} from './landmarkFit'

// Authoritative sampled JIP pull residual, in fixed RAS mm. K(y)=y+v(y).
// F(x)=D K^-1(A P x); F^-1(y)=P^-1 A^-1 K(D^-1 y).
// This is not a landmark spline and must never be exported as an affine alone.
export type JipDeformation={version:1;kernel:'jip-dense-pull';baseAffine:Mat4;inputAffine?:Mat4;dims:[number,number,number];affine:Mat4;residualBase64?:string;residualValues?:Float32Array;settings:Record<string,unknown>;execution:Record<string,unknown>;warnings:string[]}
const MAX_VOXELS=2_000_000
type FieldData={text?:string;geometry:string;values:Float32Array;inverse:Mat4;checked?:boolean}
const cache=new WeakMap<object,FieldData>()
const binaryCache=new WeakMap<Float32Array,FieldData>()
export function jipData(s:JipDeformation){
 const prior=cache.get(s)
 if(prior&&((s.residualValues!==undefined&&s.residualBase64===undefined&&prior.values===s.residualValues)||(s.residualValues===undefined&&typeof s.residualBase64==='string'&&prior.text===s.residualBase64)))return prior
 if(!Array.isArray(s.dims)||s.dims.length!==3||!s.dims.every(n=>Number.isInteger(n)&&n>=8)||s.dims.reduce((a,b)=>a*b,1)>MAX_VOXELS)throw Error('JIP field is too large or has invalid dimensions; use a coarser working resolution')
 validateAffine(s.affine);const n=s.dims.reduce((a,b)=>a*b,1),bytes=12*n,geometry=JSON.stringify([s.dims,s.affine])
 let values:Float32Array
 if(s.residualValues!==undefined){
  if(!(s.residualValues instanceof Float32Array)||s.residualValues.length!==3*n||s.residualBase64!==undefined)throw Error('Invalid binary JIP field')
  values=s.residualValues;const shared=binaryCache.get(values)
  if(shared?.geometry===geometry){cache.set(s,shared);return shared}
 }else{
  if(typeof s.residualBase64!=='string'||s.residualBase64.length!==4*Math.ceil(bytes/3)||!/^[A-Za-z0-9+/]*={0,2}$/.test(s.residualBase64))throw Error('Invalid JIP field encoding')
  const decoded=atob(s.residualBase64);if(decoded.length!==bytes)throw Error('Invalid JIP field length')
  values=new Float32Array(3*n);const view=new DataView(values.buffer)
  for(let i=0;i<bytes;i++)view.setUint8(i,decoded.charCodeAt(i))
  if(new Uint8Array(new Uint32Array([1]).buffer)[0]!==1)for(let i=0;i<values.length;i++)values[i]=view.getFloat32(i*4,true)
 }
 for(const value of values)if(!Number.isFinite(value))throw Error('Nonfinite JIP displacement')
 const result:FieldData={text:s.residualBase64,geometry,values,inverse:invertAffine(s.affine)};cache.set(s,result);binaryCache.set(values,result);return result
}
/** Session/export boundary only. Live immutable fields stay binary. */
export function serializeJip(s:JipDeformation):JipDeformation{
 validateJip(s);const {residualValues,...metadata}=s
 if(s.residualBase64!==undefined)return structuredClone(metadata)
 const values=jipData(s).values,bytes=new Uint8Array(values.length*4),view=new DataView(bytes.buffer)
 for(let i=0;i<values.length;i++)view.setFloat32(i*4,values[i],true)
 const chunks:string[]=[];for(let i=0;i<bytes.length;i+=32768)chunks.push(String.fromCharCode(...bytes.subarray(i,i+32768)))
 return {...structuredClone(metadata),residualBase64:btoa(chunks.join(''))}
}
/** Snapshot metadata independently; share only the immutable displacement array. */
export function cloneJip(s:JipDeformation):JipDeformation{
 const {residualValues,...metadata}=s
 const copy={...structuredClone(metadata),...(residualValues?{residualValues}:{})}
 // Restored base64 sessions also share their already validated decode. The
 // serialized text is immutable; validateJip still invalidates changed geometry.
 const prior=cache.get(s);if(prior)cache.set(copy,prior)
 return copy
}
export function validateJip(s:JipDeformation){
 if(!s||s.kernel!=='jip-dense-pull'||s.version!==1)throw Error('Unsupported JIP deformation')
 validateAffine(s.baseAffine);if(determinant3(s.baseAffine)<=0)throw Error('JIP deformation cannot reflect anatomy')
 if(s.inputAffine){validateAffine(s.inputAffine);if(determinant3(s.inputAffine)<=0)throw Error('JIP input composition cannot reflect anatomy')}
 if(!Array.isArray(s.warnings)||!s.warnings.every(w=>typeof w==='string')||!s.settings||!s.execution)throw Error('Missing JIP provenance')
 const prior=cache.get(s);if(prior&&prior.geometry!==JSON.stringify([s.dims,s.affine]))cache.delete(s)
 const data=jipData(s)
 if(!data.checked){
  const {values,inverse}=data,[nx,ny,nz]=s.dims,n=nx*ny*nz
  // Same forward voxel stencil checked by native JIP. This is a sampled
  // diagnostic, not a proof that a trilinear map is globally invertible.
  for(let z=0;z<nz;z++)for(let y=0;y<ny;y++)for(let x=0;x<nx;x++){
   const xyz=[x,y,z],j=[1,0,0,0,1,0,0,0,1]
   for(let axis=0;axis<3;axis++){
    const low=[x,y,z],high=[x,y,z];low[axis]=Math.min(xyz[axis],s.dims[axis]-2);high[axis]=low[axis]+1
    const i=low[0]+nx*(low[1]+ny*low[2]),k=high[0]+nx*(high[1]+ny*high[2])
    for(let c=0;c<3;c++){const dv=values[c*n+k]-values[c*n+i];for(let w=0;w<3;w++)j[3*c+w]+=dv*inverse[axis][w]}
   }
   const det=j[0]*(j[4]*j[8]-j[5]*j[7])-j[1]*(j[3]*j[8]-j[5]*j[6])+j[2]*(j[3]*j[7]-j[4]*j[6])
   if(!Number.isFinite(det)||det<=0)throw Error('JIP field has a folded or nonfinite voxel-stencil Jacobian')
  }
  data.checked=true
 }
}
export function jipResidual(s:JipDeformation,y:Vec3):Vec3{
 if(!y.every(Number.isFinite))throw Error('Invalid JIP point')
 const {values,inverse}=jipData(s),p=applyMat4(inverse,y),dims=s.dims,n=dims[0]*dims[1]*dims[2]
 // Same zero-residual extension as standalone JIP native-grid export.
 if(p.some((v,a)=>v<0||v>dims[a]-1))return [0,0,0]
 const lo=p.map(Math.floor),hi=lo.map((v,a)=>Math.min(v+1,dims[a]-1)),f=p.map((v,a)=>v-lo[a]),out:Vec3=[0,0,0]
 for(let dz=0;dz<2;dz++)for(let dy=0;dy<2;dy++)for(let dx=0;dx<2;dx++){
  const w=(dx?f[0]:1-f[0])*(dy?f[1]:1-f[1])*(dz?f[2]:1-f[2]);if(w===0)continue
  const i=(dx?hi[0]:lo[0])+dims[0]*((dy?hi[1]:lo[1])+dims[1]*(dz?hi[2]:lo[2]));for(let a=0;a<3;a++)out[a]+=w*values[a*n+i]
 }
 return out
}
export function jipPullResidual(s:JipDeformation,y:Vec3):Vec3{const v=jipResidual(s,y);return y.map((x,a)=>x+v[a]) as Vec3}
export function invertJipResidual(s:JipDeformation,q:Vec3):Vec3{
 let y:Vec3=[...q];const norm=(v:number[])=>Math.hypot(...v)
 for(let iteration=0;iteration<50;iteration++){
  const k=jipPullResidual(s,y),error=k.map((v,a)=>v-q[a]),size=norm(error);if(size<1e-5)return y
  const jac:Mat4=[[0,0,0,0],[0,0,0,0],[0,0,0,0],[0,0,0,1]],h=.001
  for(let a=0;a<3;a++){const plus:Vec3=[...y],minus:Vec3=[...y];plus[a]+=h;minus[a]-=h;const p=jipPullResidual(s,plus),m=jipPullResidual(s,minus);for(let r=0;r<3;r++)jac[r][a]=(p[r]-m[r])/(2*h)}
  if(determinant3(jac)<=0)throw Error('JIP forward mapping encountered a folded or unstable field')
  const step=applyMat4(invertAffine(jac),error as Vec3);let accepted=false
  for(let factor=1;factor>=1/64;factor/=2){const next=y.map((v,a)=>v-factor*step[a]) as Vec3;if(norm(jipPullResidual(s,next).map((v,a)=>v-q[a]))<size){y=next;accepted=true;break}}
  if(!accepted)break
 }
 throw Error('JIP forward mapping did not converge; the point was not used')
}
export function createJipMapping(matrix:Mat4,s:JipDeformation){
 const a=s.inputAffine?multiplyMat4(s.baseAffine,s.inputAffine):s.baseAffine,ai=invertAffine(a),d=multiplyMat4(matrix,ai),di=multiplyMat4(a,invertAffine(matrix))
 return {forward:(x:Vec3)=>applyMat4(d,invertJipResidual(s,applyMat4(a,x))),inverse:(y:Vec3)=>applyMat4(ai,jipPullResidual(s,applyMat4(di,y)))}
}
