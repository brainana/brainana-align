"""Exercise the actual compact ITK ABI, physical maps, refusals and N4."""
from pathlib import Path
import json,sys,tempfile
import nibabel as nib,numpy as np
sys.path.insert(0,str(Path(__file__).resolve().parents[1]))
from brainana_registration.itk_registration import register
from brainana_registration.registration_settings import n4_module

def refuses(request,needle):
    try:register(request)
    except (ValueError,InterruptedError) as e:assert needle.lower() in str(e).lower(),str(e)
    else:raise AssertionError('Invalid request accepted: '+needle)
records=[]
with tempfile.TemporaryDirectory() as temporary:
 p=Path(temporary);x,y,z=np.indices((13,15,17));v=(4+90*np.exp(-((x-4)**2/8+(y-6)**2/12+(z-8)**2/16))+45*np.exp(-((x-9)**2/4+(y-10)**2/6+(z-11)**2/8))+.1*x*y).astype(np.float32)
 a=np.array([[np.sqrt(3)/2,-.6,0,-5],[.5,1.2*np.sqrt(3)/2,0,-8],[0,0,1.4,-10],[0,0,0,1.]])
 fixed=nib.Nifti1Image(v,a);fixed.header.set_xyzt_units('mm');moving=nib.Nifti1Image(v*(1+.03*np.sin(z/3)),a).as_reoriented([[2,1],[0,-1],[1,1]]);moving.header.set_xyzt_units('mm');fixed_before=fixed.get_fdata().copy();moving_before=moving.get_fdata().copy()
 # Affine accepted bases may have scale/shear; the native voxel grids may not.
 start=np.array([[1.01,.005,0,.15],[0,.995,0,-.1],[0,0,1.005,.05],[0,0,0,1.]])
 settings=dict(engine='itk',metric='mattes-mi',threads=1,nonlinear_iterations=2,shrink_factors=[1],smoothing_sigmas_mm=[0],bspline_spacing_mm=10,maximum_step_mm=.2)
 for reverse in [False,True]:
  f,m=(moving,fixed) if reverse else(fixed,moving);nib.save(f,p/'fixed.nii');nib.save(m,p/'moving.nii')
  req=dict(stages='nonlinear',start=(np.linalg.inv(start) if reverse else start).tolist(),settings=settings,fixed=str(p/'fixed.nii'),moving=str(p/'moving.nii'))
  for metric in ['mattes-mi','local-correlation']:
   r,field,*_=register(dict(req,settings=dict(settings,metric=metric)));assert r['matrix']==req['start'];assert r['deformation']['baseAffine']==req['start'];assert field.shape==f.shape+(3,);assert np.isfinite(field).all();assert np.max(np.abs(field))>1e-7;assert r['diagnostics']['relative_jacobian_range'][0]>0;assert len(r['execution']['stages'])==1;assert r['execution']['stages'][0]['stage']=='nonlinear';assert r['settings']['optimized_model_dof']==0;records.append(dict(metric=metric,reverse=reverse,accepted_affine_unchanged=True,max_displacement_mm=float(np.abs(field).max())))
  for stage in ['affine','both',None]:refuses(dict(req,stages=stage),'nonlinear only')
  refuses(dict(req,start=None),'linear alignment');refuses(dict(req,settings=dict(settings,model=6)),'unsupported');refuses(dict(req,sampling={'version':1,'moving_windows':[{'min':[-100]*3,'max':[100]*3,'axes':[0,1]}]}),'moving continuous')
  roi={'version':1,'fixed_windows':[{'min':[-100]*3,'max':[100]*3,'axes':[0,1]}]};r,field,*_=register(dict(req,sampling=roi));assert r['settings']['sampling']['version']==1
  refuses(dict(req,sampling=roi,settings=dict(settings,shrink_factors=[2,1],smoothing_sigmas_mm=[1,0])),'dense')
  try:register(req,cancelled=lambda:True)
  except InterruptedError:pass
  else:raise AssertionError('Cancellation ignored')
  stop=[False]
  try:register(req,progress=lambda event:stop.__setitem__(0,True),cancelled=lambda:stop[0])
  except InterruptedError:pass
  else:raise AssertionError('Native iteration cancellation ignored')
  mask=np.ones(f.shape,dtype=np.uint8);mask[:2]=0;(p/'fixed-mask.u8').write_bytes(mask.tobytes(order='F'))
  r,*_=register(dict(req,sampling={'version':1,'fixed_excluded':str(p/'fixed-mask.u8')}));assert r['matrix']==req['start']
  mask.fill(1);(p/'fixed-mask.u8').write_bytes(mask.tobytes(order='F'));refuses(dict(req,sampling={'version':1,'fixed_excluded':str(p/'fixed-mask.u8')}),'sample')
 n4=n4_module()
 for preset in ['macaque-functional','macaque-anatomical']:
  config=n4.N4Settings.from_dict(dict(n4.N4Settings.for_preset(preset).to_dict(),iterations=[2],initial_spline_spacing_mm=20,estimation_spacing_mm=1))
  corrected,bias,mask,meta=n4.correct(fixed,config);assert corrected.shape==fixed.shape;assert np.all(bias>0);assert np.isfinite(corrected).all();assert set(np.unique(mask))<={0,1};assert np.allclose(corrected*bias,v,rtol=2e-7);assert meta['library']=='ITK native'
  user=nib.Nifti1Image(np.ones(fixed.shape,dtype=np.uint8),a);user.header.set_xyzt_units('mm');_,_,user_mask,_=n4.correct(fixed,config,user);assert user_mask.all()
  records.append(dict(n4=preset,finite=True,binary_mask=True,geometry_unchanged=True))
 assert np.array_equal(fixed.get_fdata(),fixed_before);assert np.array_equal(moving.get_fdata(),moving_before)
 report=dict(status='passed',records=records,refusals=['duplicate linear stages','missing linear base','legacy linear settings','moving physical windows','non-dense fixed windows','empty exclusions','initial and native-iteration cancellation'],inputs_immutable=True)
 (Path(__file__).resolve().parents[1]/'native-itk-validation.json').write_text(json.dumps(report,indent=2)+'\n')
 print('PASS actual native ITK metrics, oblique/axis-swapped both directions, accepted affine preservation, ROI/exclusions, cancellation, N4 presets and immutable inputs')
