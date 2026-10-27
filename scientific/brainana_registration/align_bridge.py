"""Headless Align adapter. Reuses JIP/N4 implementations in a one-job process.
Inputs are private float32 working NIfTIs supplied by Align, never source files.
"""
from pathlib import Path
import base64,json,time,hashlib
import nibabel as nib,numpy as np
from .engine import Engine
from .images import centered,matrix_parameters,initial_parameters,orient_like,world_pull,image_like
from .registration_settings import validate_settings,diagnose_field,n4_module
from .private_io import atomic_json
from .sampling import prepare as prepare_sampling

def encode(values):return base64.b64encode(np.asarray(values,dtype='<f4').tobytes(order='F')).decode('ascii')
def run(request_path,output_path,events_path=None,cancel_path=None):
    request=json.loads(Path(request_path).read_text());output=Path(output_path);output.parent.mkdir(parents=True,exist_ok=True)
    cancelled=lambda:bool(cancel_path and Path(cancel_path).exists())
    def progress(event):
        if events_path:
            with Path(events_path).open('a') as stream:stream.write(json.dumps(event,allow_nan=False)+'\n')
    try:
        started=time.monotonic();action=request['action'];phases={};phase=started
        if action=='n4':
            image=nib.load(request['image']);module=n4_module();settings=module.N4Settings.from_dict(request['settings'])
            corrected,bias,mask,metadata=module.correct(image,settings,progress=progress,cancelled=cancelled)
            result=dict(kind='n4',dims=list(image.shape),affine=image.affine.tolist(),corrected=encode(corrected),bias=encode(bias),metadata=metadata,settings=settings.to_dict())
        elif action=='itk-register':
            from .itk_registration import register
            result,world_vectors,*_=register(request,progress,cancelled)
            deformation=result['deformation']
            if deformation is not None:
                if request.get('field_transport')=='binary-v1':
                    raw=memoryview(np.asarray(world_vectors,dtype='<f4',order='F').reshape(-1,order='F')).cast('B')
                    filename='residual.f32';(output.parent/filename).write_bytes(raw)
                    deformation['residualAsset']=dict(version=1,file=filename,bytes=len(raw),sha256=hashlib.sha256(raw).hexdigest(),encoding='float32-le-planar')
                else:deformation['residualBase64']=encode(world_vectors)
        elif action=='register':
            fixed=nib.load(request['fixed']);moving_original=nib.load(request['moving'])
            if any(len(im.shape)!=3 or min(im.shape)<8 or np.prod(im.shape)>2_000_000 for im in (fixed,moving_original)):raise ValueError('JIP requires 3D working grids of 8 or more voxels per axis and at most 2 million voxels. Choose a coarser working resolution.')
            moving,orientation=orient_like(moving_original,fixed)
            s=validate_settings(request['settings']);axes=s.get('axes','xyz');s['axes']=''.join('xyz'[int(orientation[i][0])] for i,a in enumerate('xyz') if a in axes)
            stages=request.get('stages','both')
            if stages not in ['both','affine','nonlinear']:raise ValueError('Unknown JIP registration stage')
            if request.get('start') is not None:
                start=np.asarray(request['start'],dtype=float)
                if start.shape!=(4,4) or not np.isfinite(start).all() or np.linalg.det(start[:3,:3])<=0:raise ValueError('JIP requires a proper affine starting alignment')
                pull=np.linalg.inv(start);parameters=matrix_parameters(centered(moving)@np.linalg.inv(moving.affine)@pull@fixed.affine@np.linalg.inv(centered(fixed)))
            else:
                if stages=='nonlinear':raise ValueError('Accept a linear alignment before nonlinear-only registration')
                parameters=initial_parameters(fixed,moving,'centers')
            sampling,sampling_metadata=prepare_sampling(request,fixed,moving_original,orientation,cancelled)
            phases['input_preparation_seconds']=time.monotonic()-phase;phase=time.monotonic()
            with Engine(fixed,moving,s,parameters,sampling=sampling) as engine:
                phases['engine_creation_seconds']=time.monotonic()-phase;phase=time.monotonic()
                engine.run({'both':3,'affine':1,'nonlinear':2}[stages],progress,cancelled)
                phases['registration_seconds']=time.monotonic()-phase;phase=time.monotonic()
                data,field_affine,field_spacing=engine.field();transform=engine.transform();execution=engine.execution()
                phases['field_copy_seconds']=time.monotonic()-phase
            phase=time.monotonic()
            affine=np.linalg.inv(world_pull(transform));deformation=None
            diag=diagnose_field(data,field_spacing,transform,s.get('jacobian_limit',2),cancelled=cancelled)
            phases['diagnostics_seconds']=time.monotonic()-phase;phase=time.monotonic()
            if stages!='affine':
                if diag['relative_jacobian_range'][0]<=0:raise ValueError('JIP proposal contains a folded displacement field')
                # Engine vectors are moving-centered mm, not RAS. Convert once
                # into fixed RAS residual v so K(y)=A * original_pull(y)=y+v(y).
                basis=np.asarray(transform['moving_voxel_to_native_mm']).reshape(4,4)[:3,:3]@np.asarray(transform['moving_centered_mm_to_voxel']).reshape(4,4)[:3,:3]
                world_vectors=np.einsum('ij,...j->...i',affine[:3,:3]@basis,data).astype(np.float32)
                if np.prod(data.shape[:3])>2_000_000:raise ValueError('JIP field exceeds Align’s 2 million working-voxel limit; choose a coarser working resolution')
                deformation=dict(version=1,kernel='jip-dense-pull',baseAffine=affine.tolist(),dims=list(data.shape[:3]),affine=field_affine.tolist(),settings=dict(request['settings'],effective_axes=s['axes'],sampling=sampling_metadata),execution=execution,warnings=['JIP uses its original nonzero-fixed foreground matching convention.','Warp residual is zero outside the fixed working field. Forward point inversion checks convergence.'])
                if request.get('field_transport')=='binary-v1':
                    # Fixed filename only; the desktop owns this private directory.
                    raw=memoryview(np.asarray(world_vectors,dtype='<f4',order='F').reshape(-1,order='F')).cast('B')
                    filename='residual.f32';(output.parent/filename).write_bytes(raw)
                    deformation['residualAsset']=dict(version=1,file=filename,bytes=len(raw),sha256=hashlib.sha256(raw).hexdigest(),encoding='float32-le-planar')
                else:deformation['residualBase64']=encode(world_vectors)
            phases['field_conversion_seconds']=time.monotonic()-phase
            phases['total_before_result_write_seconds']=time.monotonic()-started
            execution['phases']=phases
            result=dict(kind='registration',matrix=affine.tolist(),deformation=deformation,execution=execution,diagnostics=diag,settings=dict(request['settings'],sampling=sampling_metadata),stages=stages,moving_orientation_preparation=orientation)
        else:raise ValueError('Unknown Align scientific action')
        if cancelled():raise InterruptedError('Cancelled')
        atomic_json(output,dict(status='ready',result=result,elapsed_seconds=time.monotonic()-started));return 0
    except InterruptedError as error:
        if request.get('action')=='itk-register':(output.parent/'residual.f32').unlink(missing_ok=True)
        atomic_json(output,dict(status='cancelled',message=str(error)));return 130
    except Exception as error:
        if request.get('action')=='itk-register':(output.parent/'residual.f32').unlink(missing_ok=True)
        atomic_json(output,dict(status='failed',message=str(error)));return 1
