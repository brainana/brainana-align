"""Registration-only helpers; no standalone workspace or GUI."""
from pathlib import Path
import importlib.util,sys,math
import numpy as np

def n4_module():
    roots=[Path(getattr(sys,'_MEIPASS',Path(__file__).parents[1])),Path(__file__).parents[1]]
    path=next((r/'preprocessing/n4_bias.py' for r in roots if (r/'preprocessing/n4_bias.py').exists()),None)
    if path is None:raise FileNotFoundError('The N4 component is missing.')
    spec=importlib.util.spec_from_file_location('jip_n4_component',path)
    module=importlib.util.module_from_spec(spec);sys.modules[spec.name]=module;spec.loader.exec_module(module);return module

def validate_settings(settings):
    s=dict(settings)
    if s.get('model',6) not in (6,9,12):raise ValueError('Select a 6-, 9- or 12-parameter linear model.')
    if s.get('metric','mi') not in ('mi','nc') or s.get('affine_metric','mi') not in ('mi','nmi'):raise ValueError('Unknown image metric.')
    if s.get('grid',8) not in (1,2,4,8,16):raise ValueError('Unknown nonlinear basis grid.')
    axes=s.get('axes','xyz')
    if not axes or len(set(axes))!=len(axes) or any(x not in 'xyz' for x in axes):raise ValueError('Select at least one nonlinear axis.')
    reg=s.get('regularization',5)
    if type(reg) is not int or not 0<=reg<=46340:raise ValueError('Regularization must be an integer from 0 to 46340.')
    for key,default in [('jacobian_limit',2),('stiffness',0)]:
        x=s.get(key,default)
        if not isinstance(x,(int,float)) or not math.isfinite(x):raise ValueError('Registration settings must be finite.')
        if key=='jacobian_limit' and x<=1 or key=='stiffness' and not 0<=x<=1:raise ValueError('Invalid Jacobian/stiffness constraint.')
    if s.get('matching','fixed') not in ('fixed','all'):raise ValueError('Unknown matching domain.')
    if s.get('fixed_threshold') is not None and not math.isfinite(float(s['fixed_threshold'])):raise ValueError('Foreground threshold must be finite.')
    if s.get('backend','cpu') not in ['cpu','threads','auto','metal','cuda']:raise ValueError('Unknown execution backend.')
    if s.get('arithmetic','compatibility') not in ['compatibility','reproducible']:raise ValueError('Unknown arithmetic mode.')
    for key,default,low,high in [('threads',4,1,64),('gpu_budget_mb',1024,32,16384)]:
        value=s.get(key,default)
        if type(value) is not int or not low<=value<=high:raise ValueError('Invalid '+key+'.')
    return s

def diagnose_field(u,spacing,transform,limit,jacobian_output=None,cancelled=lambda:False):
    """Same native voxel stencil; adapter needs ranges, not an output image."""
    a=np.array(transform['centered_pull_affine']).reshape(4,4)[:3,:3]
    det=float(np.linalg.det(a));lower=float('inf');upper=-float('inf')
    for z in range(u.shape[2]):
        if cancelled():raise InterruptedError('Cancelled')
        j=np.empty(u.shape[:2]+(3,3),dtype=np.float64)
        for c in range(3):
            for d in range(3):
                if d==2:
                    at=min(z,u.shape[2]-2)
                    grad=(u[:,:,at+1,c].astype(np.float64)-u[:,:,at,c])/spacing[d]
                else:
                    difference=np.diff(u[:,:,z,c].astype(np.float64),axis=d)/spacing[d]
                    grad=np.concatenate((difference,np.take(difference,[-1],axis=d)),axis=d)
                j[...,c,d]=a[c,d]+grad
        # Preserve the float32 diagnostic rounding used by saved Jacobian images.
        slab=(np.linalg.det(j)/det).astype(np.float32)
        if not np.isfinite(slab).all() or np.any(slab<=0):raise ValueError('The proposed map has nonfinite or nonpositive voxel-stencil Jacobians.')
        lower=min(lower,float(slab.min()));upper=max(upper,float(slab.max()))
        if jacobian_output is not None:jacobian_output[:,:,z]=slab
    warnings=[]
    if lower<1/limit-1e-5 or upper>limit+1e-5:warnings.append('The voxel-stencil Jacobian exceeds the selected bound; inspect before use.')
    return dict(affine_determinant=det,relative_jacobian_range=[lower,upper],
                nonlinear_max_abs_mm=[float(np.max(np.abs(u[...,c]))) for c in range(3)],
                affine_scale_singular_values=np.linalg.svd(a)[1].tolist(),warnings=warnings,
                jacobian_definition='Forward voxel stencil of the full centered pull map, relative to its affine determinant; not a continuous-domain folding proof.')
