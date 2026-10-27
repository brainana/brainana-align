import type {FitState} from './appTypes'
import {cloneJip,serializeJip} from '../../../packages/imaging-math/jipDeformation'

// Fit metadata is mutable; dense displacement arrays are immutable assets.
// Avoid cloning a full field for every nudge/review/proposal. Serialize only at
// persistence boundaries, preserving the existing portable JSON schema.
export function cloneFitState(fit:FitState,serializable=false):FitState{
 const {nonlinear,proposal,...metadata}=fit,copy=structuredClone(metadata) as FitState
 if(nonlinear)copy.nonlinear=nonlinear.kernel==='jip-dense-pull'?(serializable?serializeJip(nonlinear):cloneJip(nonlinear)):structuredClone(nonlinear)
 if(proposal){const {previous,...rest}=proposal;copy.proposal=structuredClone(rest);if(previous!==undefined)copy.proposal.previous=previous?cloneFitState(previous,serializable):null}else copy.proposal=proposal
 return copy
}
