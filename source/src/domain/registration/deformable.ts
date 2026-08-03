import type { JacobianSummary } from './nonlinear'
import type { Vec3 } from './rigid'

export type DeformationGrid = {
  kind: 'inverse-control-grid'
  origin: Vec3
  spacing: Vec3
  dims: [number,number,number]
  displacements: Vec3[]
  jacobian: JacobianSummary
  settings: DeformableSettings
  scoreBefore: number
  scoreAfter: number
}

export type DeformableSettings = {
  levelsMm: number[]
  regularization: number
  maximumDisplacement: number
  iterationsPerLevel: number
  jacobianMinimum: number
  jacobianMaximum: number
}

const index=(dims:[number,number,number],x:number,y:number,z:number)=>x+dims[0]*(y+dims[1]*z)
const clamp=(value:number,low:number,high:number)=>Math.max(low,Math.min(high,value))

export function createDeformationGrid(bounds:{min:Vec3;max:Vec3},spacingMm:number,settings:DeformableSettings):DeformationGrid{
  if(!Number.isFinite(spacingMm)||spacingMm<=0)throw new Error('Deformation grid spacing must be positive.')
  const padding=spacingMm
  const origin=bounds.min.map(value=>value-padding) as Vec3
  const dims=[0,1,2].map(axis=>Math.max(2,Math.min(7,Math.ceil((bounds.max[axis]-bounds.min[axis]+2*padding)/spacingMm)+1))) as [number,number,number]
  const spacing=[0,1,2].map(axis=>(bounds.max[axis]-bounds.min[axis]+2*padding)/(dims[axis]-1)) as Vec3
  return {kind:'inverse-control-grid',origin,spacing,dims,displacements:Array.from({length:dims[0]*dims[1]*dims[2]},()=>[0,0,0]),jacobian:{minimum:1,maximum:1,nonPositive:0,evaluated:0,maximumDisplacement:0},settings:structuredClone(settings),scoreBefore:NaN,scoreAfter:NaN}
}

export function sampleGridDisplacement(grid:DeformationGrid|null,point:Vec3):Vec3{
  if(!grid)return [0,0,0]
  const coordinate=[0,1,2].map(axis=>(point[axis]-grid.origin[axis])/grid.spacing[axis]) as Vec3
  const base=coordinate.map((value,axis)=>clamp(Math.floor(value),0,grid.dims[axis]-2)) as Vec3
  const fraction=coordinate.map((value,axis)=>clamp(value-base[axis],0,1)) as Vec3
  const out:Vec3=[0,0,0]
  for(let dz=0;dz<2;dz++)for(let dy=0;dy<2;dy++)for(let dx=0;dx<2;dx++){
    const weight=(dx?fraction[0]:1-fraction[0])*(dy?fraction[1]:1-fraction[1])*(dz?fraction[2]:1-fraction[2])
    const value=grid.displacements[index(grid.dims,base[0]+dx,base[1]+dy,base[2]+dz)]
    for(let axis=0;axis<3;axis++)out[axis]+=weight*value[axis]
  }
  return out
}

// The stored target-grid field is an inverse map: final fixed coordinate ->
// coordinate in the pre-deformation fixed space. This is the map required by resampling.
export function applyInverseDeformation(grid:DeformationGrid|null,point:Vec3):Vec3{
  const displacement=sampleGridDisplacement(grid,point)
  return point.map((value,axis)=>value+displacement[axis]) as Vec3
}

// Fixed-point inversion supplies the forward map used by crosshairs and landmarks.
export function applyForwardDeformation(grid:DeformationGrid|null,point:Vec3,iterations=12):Vec3{
  if(!grid)return [...point] as Vec3
  let result=[...point] as Vec3
  for(let iteration=0;iteration<iterations;iteration++){
    const displacement=sampleGridDisplacement(grid,result)
    const next=point.map((value,axis)=>value-displacement[axis]) as Vec3
    if(Math.hypot(...next.map((value,axis)=>value-result[axis]))<1e-5)return next
    result=next
  }
  return result
}

export function gridRegularization(grid:DeformationGrid):number{
  let total=0,count=0
  const [nx,ny,nz]=grid.dims
  for(let z=0;z<nz;z++)for(let y=0;y<ny;y++)for(let x=0;x<nx;x++)for(let axis=0;axis<3;axis++){
    const here=grid.displacements[index(grid.dims,x,y,z)]
    if(x+1<nx){const next=grid.displacements[index(grid.dims,x+1,y,z)];total+=(next[axis]-here[axis])**2;count++}
    if(y+1<ny){const next=grid.displacements[index(grid.dims,x,y+1,z)];total+=(next[axis]-here[axis])**2;count++}
    if(z+1<nz){const next=grid.displacements[index(grid.dims,x,y,z+1)];total+=(next[axis]-here[axis])**2;count++}
  }
  return count?total/count:0
}

function determinant3(matrix:number[][]):number{const[a,b,c]=matrix[0],[d,e,f]=matrix[1],[g,h,i]=matrix[2];return a*(e*i-f*h)-b*(d*i-f*g)+c*(d*h-e*g)}
export function summarizeGridJacobian(grid:DeformationGrid,samples=7):JacobianSummary{
  let minimum=Infinity,maximum=-Infinity,nonPositive=0,evaluated=0,maximumDisplacement=0
  const max=grid.origin.map((value,axis)=>value+grid.spacing[axis]*(grid.dims[axis]-1)) as Vec3
  const span=max.map((value,axis)=>value-grid.origin[axis]) as Vec3
  const epsilon=Math.max(.1,Math.min(...grid.spacing)/8)
  for(let z=0;z<samples;z++)for(let y=0;y<samples;y++)for(let x=0;x<samples;x++){
    const point:gridpoint=[grid.origin[0]+span[0]*(x+.5)/samples,grid.origin[1]+span[1]*(y+.5)/samples,grid.origin[2]+span[2]*(z+.5)/samples]
    maximumDisplacement=Math.max(maximumDisplacement,Math.hypot(...sampleGridDisplacement(grid,point)))
    const jacobian=Array.from({length:3},()=>Array(3).fill(0))
    for(let input=0;input<3;input++){const low=[...point] as Vec3,high=[...point] as Vec3;low[input]-=epsilon;high[input]+=epsilon;const a=applyInverseDeformation(grid,low),b=applyInverseDeformation(grid,high);for(let output=0;output<3;output++)jacobian[output][input]=(b[output]-a[output])/(2*epsilon)}
    const determinant=determinant3(jacobian);minimum=Math.min(minimum,determinant);maximum=Math.max(maximum,determinant);if(determinant<=0)nonPositive++;evaluated++
  }
  return {minimum,maximum,nonPositive,evaluated,maximumDisplacement}
}
type gridpoint=Vec3

export function resampleGrid(source:DeformationGrid,target:DeformationGrid):void{
  for(let z=0;z<target.dims[2];z++)for(let y=0;y<target.dims[1];y++)for(let x=0;x<target.dims[0];x++){
    const point=[target.origin[0]+x*target.spacing[0],target.origin[1]+y*target.spacing[1],target.origin[2]+z*target.spacing[2]] as Vec3
    target.displacements[index(target.dims,x,y,z)]=sampleGridDisplacement(source,point)
  }
}

export async function optimizeDeformationGrid(options:{
  bounds:{min:Vec3;max:Vec3};settings:DeformableSettings;initialGrid?:DeformationGrid|null;score:(grid:DeformationGrid)=>number;cancelled:()=>boolean;progress?:(message:string)=>void
}):Promise<DeformationGrid>{
  let grid:DeformationGrid|null=null,baseline=-Infinity
  for(let level=0;level<options.settings.levelsMm.length;level++){
    const next=createDeformationGrid(options.bounds,options.settings.levelsMm[level],options.settings)
    if(grid)resampleGrid(grid,next)
    else if(options.initialGrid)resampleGrid(options.initialGrid,next)
    grid=next
    let step=Math.min(options.settings.maximumDisplacement,options.settings.levelsMm[level]/8)
    baseline=options.score(grid)-options.settings.regularization*gridRegularization(grid)
    for(let iteration=0;iteration<options.settings.iterationsPerLevel;iteration++){
      for(let node=0;node<grid.displacements.length;node++)for(let axis=0;axis<3;axis++){
        if(options.cancelled())throw new Error('Deformable refinement cancelled.')
        const original=grid.displacements[node][axis];let chosen=original,best=baseline
        for(const sign of [-1,1]){const candidate=clamp(original+sign*step,-options.settings.maximumDisplacement,options.settings.maximumDisplacement);grid.displacements[node][axis]=candidate;const qc=summarizeGridJacobian(grid,4);if(qc.nonPositive||qc.minimum<options.settings.jacobianMinimum||qc.maximum>options.settings.jacobianMaximum)continue;const objective=options.score(grid)-options.settings.regularization*gridRegularization(grid);if(objective>best+1e-7){best=objective;chosen=candidate}}
        grid.displacements[node][axis]=chosen;baseline=best
        if(node%8===0&&axis===2){options.progress?.(`Deformable level ${level+1}/${options.settings.levelsMm.length}, iteration ${iteration+1}/${options.settings.iterationsPerLevel}`);await new Promise(resolve=>setTimeout(resolve,0))}
      }
      step/=2
    }
  }
  if(!grid)throw new Error('No deformable grid levels were configured.')
  grid.jacobian=summarizeGridJacobian(grid)
  if(grid.jacobian.nonPositive||grid.jacobian.minimum<options.settings.jacobianMinimum||grid.jacobian.maximum>options.settings.jacobianMaximum)throw new Error('The proposed deformation violates its Jacobian safety limits.')
  return grid
}
