import type {Mat4,Vec3} from './rigid'
export type AcquisitionMetadata={version:1;source:'manual'|'bids';readout:'unknown'|'cartesian'|'epi';phaseEncodingDirection?:'i'|'i-'|'j'|'j-'|'k'|'k-';totalReadoutTimeSeconds?:number;echoTimeSeconds?:number;repetitionTimeSeconds?:number;sequenceName?:string;pulseSequenceType?:string}
export function validateAcquisition(value:unknown):AcquisitionMetadata{
 if(!value||typeof value!=='object')throw Error('Invalid acquisition metadata')
 const a=value as AcquisitionMetadata
 if(a.version!==1||!['manual','bids'].includes(a.source)||!['unknown','cartesian','epi'].includes(a.readout))throw Error('Invalid acquisition metadata version or readout type')
 if(a.phaseEncodingDirection!==undefined&&!['i','i-','j','j-','k','k-'].includes(a.phaseEncodingDirection))throw Error('Phase encoding must be a NIfTI voxel axis i, j or k, optionally followed by −.')
 for(const key of ['totalReadoutTimeSeconds','echoTimeSeconds','repetitionTimeSeconds'] as const)if(a[key]!==undefined&&(typeof a[key]!=='number'||!Number.isFinite(a[key])||a[key]!<=0||a[key]!>1000))throw Error('Acquisition timing must be positive finite seconds')
 for(const key of ['sequenceName','pulseSequenceType'] as const)if(a[key]!==undefined&&(typeof a[key]!=='string'||a[key]!.length>256))throw Error('Sequence descriptions must be at most256 characters')
 return {version:1,source:a.source,readout:a.readout,...(a.phaseEncodingDirection?{phaseEncodingDirection:a.phaseEncodingDirection}:{}),...(a.totalReadoutTimeSeconds!==undefined?{totalReadoutTimeSeconds:a.totalReadoutTimeSeconds}:{}),...(a.echoTimeSeconds!==undefined?{echoTimeSeconds:a.echoTimeSeconds}:{}),...(a.repetitionTimeSeconds!==undefined?{repetitionTimeSeconds:a.repetitionTimeSeconds}:{}),...(a.sequenceName!==undefined?{sequenceName:a.sequenceName}:{}),...(a.pulseSequenceType!==undefined?{pulseSequenceType:a.pulseSequenceType}:{})}
}
export function acquisitionFromBids(value:unknown):AcquisitionMetadata{
 if(!value||typeof value!=='object'||Array.isArray(value))throw Error('Choose a BIDS MRI JSON sidecar')
 const b=value as Record<string,unknown>,scan=Array.isArray(b.ScanningSequence)?b.ScanningSequence.join(' '):String(b.ScanningSequence??''),pulse=typeof b.PulseSequenceType==='string'?b.PulseSequenceType:undefined
 return validateAcquisition({version:1,source:'bids',readout:/\bEP\b/i.test(scan)||/epi|echo.?planar/i.test(pulse??'')?'epi':'unknown',phaseEncodingDirection:b.PhaseEncodingDirection,totalReadoutTimeSeconds:b.TotalReadoutTime,echoTimeSeconds:b.EchoTime,repetitionTimeSeconds:b.RepetitionTime,sequenceName:b.SequenceName,pulseSequenceType:pulse})
}
/** BIDS i/j/k are native voxel axes. Working preparation retains their axis directions. */
export function physicalPhaseDirection(a:AcquisitionMetadata,affine:Mat4):{axis:0|1|2;sign:1|-1;rasUnitVector:Vec3}|null{
 const valid=validateAcquisition(a);if(!valid.phaseEncodingDirection)return null
 const axis='ijk'.indexOf(valid.phaseEncodingDirection[0]) as 0|1|2,sign=valid.phaseEncodingDirection.endsWith('-')?-1:1,vector=affine.slice(0,3).map(r=>r[axis]),length=Math.hypot(...vector)
 if(!Number.isFinite(length)||length<1e-8)throw Error('The acquisition image has invalid physical geometry')
 return {axis,sign,rasUnitVector:vector.map(v=>v===0?0:sign*v/length) as Vec3}
}
