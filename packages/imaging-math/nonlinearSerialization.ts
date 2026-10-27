import {serializeJip} from './jipDeformation'
import {validateSpline,type NonlinearSpline} from './nonlinearSpline'

/** Portable JSON boundary for every nonlinear transform kind. Never stringify a live field. */
export function serializeNonlinear(spline:NonlinearSpline|null|undefined):NonlinearSpline|null {
 if(!spline)return null
 const portable=spline.kernel==='jip-dense-pull'?serializeJip(spline):structuredClone(spline)
 // Spline validation normalizes derived metadata, so validate the copy rather
 // than changing the accepted fit while saving it.
 validateSpline(portable)
 return portable
}
