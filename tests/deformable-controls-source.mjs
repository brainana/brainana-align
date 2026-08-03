import assert from 'node:assert/strict'
import fs from 'node:fs'
const source=fs.readFileSync(new URL('../source/src/app/bootstrap.ts',import.meta.url),'utf8')
for(const id of ['deform-preset','deform-levels','deform-regularization','deform-max','deform-iterations','deform-jac-min','deform-jac-max','deform-refine','cancel-deform','reset-deform'])assert.ok(source.includes(`id="${id}"`),`missing deformable control ${id}`)
assert.match(source,/withinOptimizationWindows\(world,fixedWindow\)/,'fixed optimization windows must constrain deformable metric samples')
assert.match(source,/withinOptimizationWindows\(sw,ctx\.movingWindow\)/,'moving optimization windows must constrain deformable metric samples')
assert.match(source,/after<before-1e-5/,'worsened NMI must reject the proposal')
assert.match(source,/deformationProposal\.previous/,'reject must restore the previous deformation')
assert.match(source,/deformCancelled=true/,'cancel action must reach the optimizer')
assert.match(source,/initialGrid:previous/,'a repeated pass must initialize from the accepted deformation')
assert.match(source,/deformationProposal\)return setStatus\('A deformable proposal is already awaiting review/,'a pending proposal must be reviewed before rerunning')
assert.match(source,/deformButton\.disabled=Boolean\(fitResult\?\.deformationProposal\)/,'the rerun control must reflect pending proposal state')
for(const label of ['Deformable preset','Grid levels (mm)','Regularization','Maximum displacement','Iterations per level','Minimum Jacobian','Maximum Jacobian']){
  const escaped=label.replace(/[.*+?^${}()|[\]\\]/g,'\\$&')
  assert.match(source,new RegExp(`class="parameter-help"[^>]*data-help="[^"]+">${escaped}`),`missing hover help for ${label}`)
}
assert.match(source,/Custom deformable settings need attention:/,'custom-setting failures must have a full explanatory heading')
console.log('deformable automatic-refinement UI and safety source checks passed')
