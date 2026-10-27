"""Native ITK nonlinear-only registration. Align owns every linear fit.

The accepted moving-to-fixed RAS matrix is immutable. ITK optimizes a fixed-space
B-spline K; the full pull is inverse(A) composed with K. No affine optimizer.
"""
from pathlib import Path
import math,time,hashlib
import nibabel as nib
import numpy as np
from .sampling import prepare as prepare_sampling
from . import native_itk
MAX_VOXELS=2_000_000
RAS_LPS=np.diag([-1.,-1.,1.,1.])

def capabilities():
    return dict(version=1,library='ITK native',library_version=native_itk.version(),
                models=[],metrics=['mattes-mi','local-correlation'],stages=['nonlinear'],
                nonlinear='cubic-bspline',fixed_windows=True,moving_windows=False,
                exclusion_masks=True,fixed_window_sampling='dense-native-grid-only',
                shear=False,phase_encoding_constraint=False,
                field='fixed-RAS residual K(y)-y; full pull inverse(A) composed with K',
                jacobian='positive sampled voxel stencils; no continuous topology guarantee')

def validate_settings(settings):
    if not isinstance(settings,dict):raise ValueError('ITK settings must be an object.')
    defaults=dict(engine='itk',metric='mattes-mi',bins=32,local_radius=2,
                  sampling_fraction=1.,seed=1729,threads=1,
                  nonlinear_iterations=40,learning_rate=1.,min_step=.001,
                  shrink_factors=[4,2,1],smoothing_sigmas_mm=[2.,1.,0.],
                  bspline_spacing_mm=20.,maximum_step_mm=1.)
    unknown=set(settings)-set(defaults)
    if unknown:raise ValueError('Unsupported ITK setting(s): '+', '.join(sorted(unknown))+'. JIP/GPU settings do not apply to ITK.')
    s=dict(defaults,**settings)
    if s['engine']!='itk':raise ValueError('Choose the ITK nonlinear method.')
    if s['metric'] not in ('mattes-mi','local-correlation'):raise ValueError('Choose Mattes mutual information or ANTs neighborhood correlation.')
    for key,lo,hi in [('bins',16,256),('local_radius',1,8),('seed',1,2**32-1),('threads',1,64),('nonlinear_iterations',1,1000)]:
        if type(s[key]) is not int or not lo<=s[key]<=hi:raise ValueError('Invalid ITK '+key+'.')
    for key,lo,hi in [('sampling_fraction',.01,1.),('learning_rate',.001,10.),('min_step',1e-6,1.),('bspline_spacing_mm',1.,200.),('maximum_step_mm',.001,10.)]:
        if isinstance(s[key],bool) or not isinstance(s[key],(int,float)) or not math.isfinite(s[key]) or not lo<=s[key]<=hi:raise ValueError('Invalid ITK '+key+'.')
    factors=s['shrink_factors'];sigmas=s['smoothing_sigmas_mm']
    if not isinstance(factors,list) or not 1<=len(factors)<=5 or any(type(x) is not int or not 1<=x<=16 for x in factors) or factors[-1]!=1 or any(a<=b for a,b in zip(factors,factors[1:])):raise ValueError('ITK shrink factors must descend to 1, with at most five levels.')
    if not isinstance(sigmas,list) or len(sigmas)!=len(factors) or any(isinstance(x,bool) or not isinstance(x,(int,float)) or not math.isfinite(x) or not 0<=x<=20 for x in sigmas):raise ValueError('Specify one finite physical smoothing sigma (0–20 mm) per ITK pyramid level.')
    return s


def check_cancel(cancelled):
    if cancelled():raise InterruptedError('Cancelled')

def sampled_diagnostics(residual,fixed_affine,pull_affine,cancelled=lambda:False,jacobian_output=None):
    """One-sided boundary/forward voxel stencils in physical RAS coordinates."""
    inverse=np.linalg.inv(fixed_affine[:3,:3]);lower=math.inf;upper=-math.inf
    for z in range(residual.shape[2]):
        check_cancel(cancelled);j=np.empty(residual.shape[:2]+(3,3),dtype=float)
        for c in range(3):
            for d in range(3):
                if d==2:
                    at=min(z,residual.shape[2]-2);grad=residual[:,:,at+1,c].astype(float)-residual[:,:,at,c]
                else:
                    diff=np.diff(residual[:,:,z,c].astype(float),axis=d);grad=np.concatenate([diff,np.take(diff,[-1],axis=d)],axis=d)
                j[...,c,d]=grad
        relative=np.linalg.det(np.einsum('...ij,jk->...ik',j,inverse)+np.eye(3))
        if not np.isfinite(relative).all() or np.any(relative<=0):raise ValueError('ITK proposal has nonfinite or nonpositive sampled Jacobians. Reduce nonlinear iterations, use a coarser B-spline spacing, or retain the linear proposal.')
        if jacobian_output is not None:jacobian_output[:,:,z]=relative
        lower=min(lower,float(relative.min()));upper=max(upper,float(relative.max()))
    return dict(affine_determinant=float(np.linalg.det(pull_affine[:3,:3])),relative_jacobian_range=[lower,upper],
                nonlinear_max_abs_mm=np.max(np.abs(residual),axis=(0,1,2)).astype(float).tolist(),
                affine_scale_singular_values=np.linalg.svd(pull_affine[:3,:3])[1].tolist(),
                warnings=['ITK B-spline output is sampled onto the fixed working grid. Positive voxel stencils do not prove continuous topology.'] if np.any(residual) else [],
                jacobian_definition='Forward voxel stencil of K(y)=A*T(y) in physical RAS, including one-sided grid boundaries. No continuous folding proof.')


def register(request,progress=lambda event:None,cancelled=lambda:False):
    started=time.monotonic();check_cancel(cancelled)
    if request.get('stages')!='nonlinear':raise ValueError('ITK in Align is nonlinear only. Run and accept Linear registration first.')
    a=np.asarray(request.get('start'),dtype=float)
    if a.shape!=(4,4) or not np.isfinite(a).all() or not np.allclose(a[3],[0,0,0,1],atol=1e-8,rtol=0) or np.linalg.det(a[:3,:3])<=1e-8:raise ValueError('Accept a finite proper linear alignment before ITK nonlinear registration.')
    s=validate_settings(request.get('settings',{}));spec=request.get('sampling')
    if spec is not None and (not isinstance(spec,dict) or spec.get('version')!=1):raise ValueError('Unsupported ITK registration sampling format.')
    if spec and spec.get('moving_windows'):raise ValueError('ITK does not support moving continuous physical optimization windows. Clear them explicitly or choose JIP; no window was dropped.')
    if spec and spec.get('fixed_windows') and (s['sampling_fraction']!=1 or s['shrink_factors']!=[1]):raise ValueError('Exact ITK fixed windows require dense working-grid metric sampling: sampling_fraction=1, shrink_factors=[1]. No window was dropped.')
    fixed=nib.load(request['fixed']);moving=nib.load(request['moving'])
    for image in (fixed,moving):native_itk.image_arrays(image)
    selections,sampling_metadata=prepare_sampling(request,fixed,moving,[[0,1],[1,1],[2,1]],cancelled);selections=selections or {}
    for side,image in [('fixed',fixed),('moving',moving)]:
        values=image.get_fdata(dtype=np.float32);selection=selections.get(side);chosen=values.ravel() if selection is None else values[selection.astype(bool)]
        if chosen.size<64 or not np.ptp(chosen)>0:raise ValueError('ITK requires at least 64 included, varying samples in each image.')
    pull=np.linalg.inv(a);residual,native=native_itk.bspline(fixed,moving,selections,pull,s,progress,cancelled)
    if not math.isfinite(native['metric_value']) or native['valid_metric_samples']<64:raise ValueError('ITK has fewer than 64 valid metric samples or an invalid metric. Improve starting overlap or enlarge regions.')
    diagnostics=sampled_diagnostics(residual,fixed.affine,pull,cancelled)
    record=dict(stage='nonlinear',optimizer='GradientDescentLineSearch',metric_direction='minimize',**{k:native[k] for k in ['metric_value','valid_metric_samples','iterations_final_level','stop_condition','levels']})
    record['reached_iteration_budget']='maximum number of iterations' in native['stop_condition'].lower()
    if record['reached_iteration_budget']:diagnostics['warnings'].append('ITK reached its iteration budget; inspect the proposal and stop conditions.')
    effective=dict(s,bspline_mesh_size=native['bspline_mesh_size'],bspline_domain=native['bspline_domain'],sampling=sampling_metadata,nonlinear_parameterization='cubic B-spline',optimized_model_dof=0,affine_composition='accepted linear matrix unchanged; no linear optimization',nonlinear_composition='accepted linear pull composed after optimized fixed-space B-spline')
    execution=dict(engine='itk',active='cpu',library='ITK native',library_version=native['library_version'],itk_version=native['library_version'],threads=s['threads'],seed=s['seed'],sampling_strategy='random' if s['sampling_fraction']<1 else 'dense',stages=[record],elapsed_seconds=time.monotonic()-started,build=dict(arithmetic='ITK native floating point'),provenance=dict(fixed_sha256=_sha256(request['fixed']),moving_sha256=_sha256(request['moving']),fixed_affine_RAS=fixed.affine.tolist(),moving_affine_RAS=moving.affine.tolist(),start_moving_to_fixed_RAS=a.tolist(),final_affine_pull_LPS=(RAS_LPS@pull@RAS_LPS).tolist(),coordinate_conversion='RAS ↔ LPS: diag(-1,-1,1); no reorientation or shear approximation',metric=s['metric'],interpolator='linear',proposal=True))
    warnings=['ITK uses the selected metric with no implicit foreground mask.','Residual is zero outside the fixed working field; forward point inversion checks convergence.']
    deformation=dict(version=1,kernel='jip-dense-pull',baseAffine=a.tolist(),dims=list(fixed.shape),affine=fixed.affine.tolist(),settings=effective,execution=execution,warnings=warnings)
    result=dict(kind='registration',matrix=a.tolist(),deformation=deformation,execution=execution,diagnostics=diagnostics,settings=effective,stages='nonlinear',itk_capability='itk-v1',sampling_capability='sampling-v1' if spec else None)
    return result,residual,None,fixed,moving,None

def _sha256(path):
    digest=hashlib.sha256()
    with Path(path).open('rb') as stream:
        for block in iter(lambda:stream.read(1024*1024),b''):digest.update(block)
    return digest.hexdigest()
