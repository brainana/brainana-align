import { LINEAR_PARAMETERS, type LinearParameter, type TransformModel } from '../domain/registration/linear'

export const linearRegistrationControlsHtml = `
  <label>Landmark transform<select id="transform-model"><option value="rigid" selected>Rigid · 6 parameters</option><option value="similarity">Similarity · 7 parameters</option><option value="anisotropic">Anisotropic scale · 9 parameters</option><option value="affine">Affine · 12 parameters</option><option value="custom">Custom linear parameters</option></select></label>
  <label id="reflection-option" class="check hidden"><input id="allow-reflection" type="checkbox"> Allow reflected affine fit</label><p id="reflection-warning" class="hint hidden">Off by default. A reflected transform mirrors anatomy and can swap left and right. Enable only after checking landmark correspondence and coordinate conventions.</p>
  <div id="custom-parameters" class="custom-parameters hidden">
    <p class="hint">Unchecked parameters remain fixed at identity. Scale and shear models require at least four well-distributed, non-coplanar landmark pairs.</p>
    <div class="parameter-grid">
      <label><input type="checkbox" data-linear-param="tx" checked> Translate X</label><label><input type="checkbox" data-linear-param="ty" checked> Translate Y</label><label><input type="checkbox" data-linear-param="tz" checked> Translate Z</label>
      <label><input type="checkbox" data-linear-param="rx" checked> Rotate X</label><label><input type="checkbox" data-linear-param="ry" checked> Rotate Y</label><label><input type="checkbox" data-linear-param="rz" checked> Rotate Z</label>
      <label><input type="checkbox" data-linear-param="sx"> Scale X</label><label><input type="checkbox" data-linear-param="sy"> Scale Y</label><label><input type="checkbox" data-linear-param="sz"> Scale Z</label>
      <label><input type="checkbox" data-linear-param="shxy"> Shear XY</label><label><input type="checkbox" data-linear-param="shxz"> Shear XZ</label><label><input type="checkbox" data-linear-param="shyz"> Shear YZ</label>
    </div>
  </div>`

export function transformModelLabel(model: TransformModel): string {
  return model === 'rigid' ? 'Rigid transform' : model === 'similarity' ? 'Similarity transform' : model==='anisotropic'?'Anisotropic-scale transform' : model === 'affine' ? 'Affine transform' : 'Custom linear transform'
}

export function selectedLinearParameters(root: ParentNode = document): LinearParameter[] {
  return Array.from(root.querySelectorAll<HTMLInputElement>('[data-linear-param]:checked'))
    .map(input => input.dataset.linearParam as LinearParameter)
    .filter(parameter => LINEAR_PARAMETERS.includes(parameter))
}

export function updateTransformModelControls(select: HTMLSelectElement, custom: HTMLElement, fitButton: HTMLButtonElement,reflectionOption:HTMLElement|null=document.querySelector('#reflection-option'),reflectionWarning:HTMLElement|null=document.querySelector('#reflection-warning')): void {
  const model = select.value as TransformModel
  custom.classList.toggle('hidden', model !== 'custom')
  reflectionOption?.classList.toggle('hidden',model!=='affine');reflectionWarning?.classList.toggle('hidden',model!=='affine')
  fitButton.textContent = model === 'rigid' ? 'Fit rigid transform' : model === 'similarity' ? 'Fit similarity transform' : model==='anisotropic'?'Fit anisotropic-scale transform' : model === 'affine' ? 'Fit affine transform' : 'Fit selected parameters'
}
