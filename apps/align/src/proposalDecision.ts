import type {FitState} from './appTypes'
import {isLocalRegistration} from './registrationWorkflow'
/** Delegate to the existing decision handlers; never silently accept a preview. */
export function proposalDecisionControl(fit:FitState|null,accept:boolean){
 const id=fit?.proposal?.kind==='spline'?(accept?'accept-spline':'reject-spline'):isLocalRegistration(fit)?(accept?'accept-refine-followup':'reject-refine-followup'):(accept?'accept-refine':'reject-refine')
 return document.querySelector<HTMLButtonElement>('#'+id)
}
export function decideProposal(fit:FitState|null,accept:boolean){if(!fit?.proposal)return false;const control=proposalDecisionControl(fit,accept);if(!control||control.disabled)return false;control.click();return true}
