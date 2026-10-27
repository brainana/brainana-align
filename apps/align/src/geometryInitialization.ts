import type { FitState } from './appTypes'
import { diagnoseTransform } from './landmarkFit'
export function imageGeometryAlignment(direction: string): FitState {
  const matrix=[[1,0,0,0],[0,1,0,0],[0,0,1,0],[0,0,0,1]]
  return {direction,initializationSource:'image-geometry',model:'rigid',landmarkModel:'rigid',parameterCount:6,matrix,landmarkMatrix:structuredClone(matrix),baseMatrix:structuredClone(matrix),inverse:structuredClone(matrix),rms:0,manual:[0,0,0,0,0,0],proposal:null,landmarkSnapshot:[],landmarksChanged:false,fittedAt:new Date().toISOString(),diagnostics:diagnoseTransform(matrix),reflectionConfirmed:false}
}
