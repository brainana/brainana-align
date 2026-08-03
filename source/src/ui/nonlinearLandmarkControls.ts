import type { FitState, Landmark } from '../app/types'
import { fitLandmarkWarp } from '../domain/registration/nonlinear'

export const nonlinearLandmarkControlsHtml = `
  <p class="hint">Applies a regularized 3D thin-plate spline after the current linear landmark alignment. At least four well-distributed landmarks are required.</p>
  <label>Regularization<select id="tps-regularization"><option value="0">Exact interpolation</option><option value="0.01">Very flexible</option><option value="0.05" selected>Balanced</option><option value="0.2">Smooth</option><option value="1">Very smooth</option><option value="custom">Custom…</option></select></label>
  <label id="tps-custom-regularization-row" class="hidden">Custom regularization value<input id="tps-custom-regularization" type="number" min="0" step="any" value="0.05" inputmode="decimal"><span class="hint">Must be a finite value greater than or equal to zero. Zero exactly interpolates the landmarks; larger values produce smoother warps.</span></label>
  <button id="apply-landmark-warp" disabled>Apply nonlinear landmark warp</button>
  <button id="reset-landmark-warp" disabled>Reset nonlinear warp</button>
  <div id="landmark-warp-summary" class="summary">No nonlinear landmark warp.</div>`

export function resolveNonlinearRegularization(preset: string, customValue: string): number {
  const value=preset==='custom'?Number(customValue):Number(preset)
  if(customValue.trim()===''&&preset==='custom')throw new Error('Enter a custom nonlinear regularization value.')
  if(!Number.isFinite(value)||value<0)throw new Error('Nonlinear regularization must be a finite value greater than or equal to zero.')
  return value
}

export function installNonlinearLandmarkControls(options: {
  getFit: () => FitState | null
  residualRms: () => number
  rebuild: () => void
  render: () => void
  setStatus: (message: string, error?: boolean) => void
  setRigidRefinementEnabled: (enabled: boolean) => void
  beginProcessing: (message: string) => void
  endProcessing: () => void
  yieldForPaint: () => Promise<void>
}) {
  const regularization=document.querySelector<HTMLSelectElement>('#tps-regularization')!
  const customRow=document.querySelector<HTMLLabelElement>('#tps-custom-regularization-row')!
  const customRegularization=document.querySelector<HTMLInputElement>('#tps-custom-regularization')!
  const applyButton=document.querySelector<HTMLButtonElement>('#apply-landmark-warp')!
  const resetButton=document.querySelector<HTMLButtonElement>('#reset-landmark-warp')!
  const summary=document.querySelector<HTMLDivElement>('#landmark-warp-summary')!
  const complete=(landmarks:Landmark[])=>landmarks.filter(landmark=>landmark.enabled&&landmark.mri&&landmark.ct)
  const syncRegularization=()=>{
    const custom=regularization.value==='custom'
    customRow.classList.toggle('hidden',!custom)
    customRegularization.disabled=!custom
  }
  const sync=()=>{
    const fit=options.getFit(), count=fit?complete(fit.landmarkSnapshot).length:0
    applyButton.disabled=!fit||count<4;resetButton.disabled=!fit?.landmarkWarp
    options.setRigidRefinementEnabled(Boolean(fit&&!fit.landmarkWarp))
    if(!fit?.landmarkWarp)summary.textContent='No nonlinear landmark warp.'
    else {const warp=fit.landmarkWarp;summary.innerHTML=`<strong>TPS landmark RMS: ${warp.landmarkRms.toFixed(3)} mm</strong><br>Sampled Jacobian ${warp.jacobian.minimum.toFixed(3)}–${warp.jacobian.maximum.toFixed(3)}; maximum displacement ${warp.jacobian.maximumDisplacement.toFixed(2)} mm.`}
  }
  applyButton.addEventListener('click',()=>void (async()=>{
    const fit=options.getFit();if(!fit)return
    const pairs=complete(fit.landmarkSnapshot);if(pairs.length<4)return options.setStatus('At least four complete, enabled landmark pairs are required for nonlinear landmark warping.',true)
    const source=pairs.map(landmark=>fit.direction==='ct-mri'?landmark.ct!:landmark.mri!),target=pairs.map(landmark=>fit.direction==='ct-mri'?landmark.mri!:landmark.ct!)
    options.beginProcessing('Applying nonlinear landmark deformation…')
    await options.yieldForPaint()
    try{
      const lambda=resolveNonlinearRegularization(regularization.value,customRegularization.value)
      fit.landmarkWarp=fitLandmarkWarp(fit.matrix,source,target,lambda);fit.deformation=null;fit.deformationProposal=null;fit.rms=options.residualRms();sync()
      const jacobian=fit.landmarkWarp.jacobian
      if(jacobian.minimum<.5||jacobian.maximum>2)summary.insertAdjacentHTML('beforeend','<br><span class="warning">The sampled Jacobian extends beyond the conservative 0.5–2.0 range.</span>')
      options.rebuild();options.render();options.setStatus('Nonlinear landmark warp applied. Review the overlay and Jacobian summary before export.')
    }catch(error){options.setStatus(error instanceof Error?error.message:String(error),true)}finally{options.endProcessing()}
  })())
  regularization.addEventListener('change',()=>{syncRegularization();if(regularization.value==='custom'){customRegularization.focus();customRegularization.select()}})
  resetButton.addEventListener('click',()=>void (async()=>{const fit=options.getFit();if(!fit)return;options.beginProcessing('Restoring the linear landmark alignment…');await options.yieldForPaint();try{fit.landmarkWarp=null;fit.deformation=null;fit.deformationProposal=null;fit.rms=options.residualRms();sync();options.rebuild();options.render();options.setStatus('Nonlinear landmark warp reset. The linear alignment and landmarks were retained; any downstream deformable refinement was cleared.')}finally{options.endProcessing()}})())
  syncRegularization()
  return {sync}
}
