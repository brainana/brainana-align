"""Compare original, incorporated source and frozen workers on physical test data."""
from pathlib import Path
import base64,hashlib,json,os,shutil,subprocess,sys,tempfile
import nibabel as nib
import numpy as np
ROOT=Path(__file__).resolve().parents[2];S=ROOT/'scientific';sys.path.insert(0,str(S))
from brainana_registration.align_bridge import run
B=ROOT/'generated/scientific-runtime'/('BrainanaAlignRegistration.exe' if sys.platform=='win32' else 'BrainanaAlignRegistration')
def clean():
 # Windows normalizes os.environ keys to uppercase. Preserve OS essentials,
 # matching the production worker policy, while excluding Python/engine overrides.
 allowed={'HOME','TMPDIR','TEMP','TMP','USER','LANG','LC_ALL','SYSTEMROOT','WINDIR'}
 env={k:v for k,v in os.environ.items() if k.upper() in allowed}
 return env|{'PATH':str(Path(os.environ.get('SystemRoot','C:/Windows'))/'System32') if sys.platform=='win32' else '/usr/bin:/bin','JIP_ENGINE_LIBRARY':'/nonexistent/ignored-engine','PYTHONPATH':'/nonexistent/ignored-python'}
def frozen_run(args,timeout=120):
 with subprocess.Popen([str(x) for x in args],env=clean(),stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True) as process:
  try:
   stdout,stderr=process.communicate(timeout=timeout)
  except subprocess.TimeoutExpired:
   # Kill the PyInstaller process tree before its parent disappears on Windows.
   if sys.platform=='win32':subprocess.run(['taskkill','/F','/T','/PID',str(process.pid)],capture_output=True,timeout=30)
   process.kill();stdout,stderr=process.communicate(timeout=15)
   evidence=ROOT/'artifacts/validation/frozen-timeout';evidence.mkdir(parents=True,exist_ok=True)
   (evidence/'stdout.txt').write_text(stdout);(evidence/'stderr.txt').write_text(stderr)
   if '--align-request' in args:
    request=Path(args[args.index('--align-request')+1]);shutil.copytree(request.parent,evidence/'synthetic-case',dirs_exist_ok=True)
   raise RuntimeError('Frozen worker timed out; process tree stopped; see frozen-timeout artifacts')
  if process.returncode:raise RuntimeError(f'Frozen worker failed ({process.returncode}): {stderr}')
  return stdout
cap=json.loads(frozen_run([B,'--align-capabilities']));assert cap['runtime']=='brainana-align-registration';assert cap['version']=='0.17.7'
assert not any('qt' in p.name.lower() or p.name=='jip_workbench' or p.name.endswith('.app') for p in B.parent.rglob('*'))
records=[]
with tempfile.TemporaryDirectory(prefix='align-private-runtime-') as temporary:
 root=Path(temporary);x,y,z=np.indices((13,15,17));v=(4+90*np.exp(-((x-4)**2/8+(y-6)**2/12+(z-8)**2/16))+45*np.exp(-((x-9)**2/4+(y-10)**2/6+(z-11)**2/8))+.1*x*y).astype(np.float32)
 a=np.array([[np.sqrt(3)/2,-.6,0,-5],[.5,1.2*np.sqrt(3)/2,0,-8],[0,0,1.4,-10],[0,0,0,1.]])
 fixed=nib.Nifti1Image(v,a);fixed.header.set_xyzt_units('mm');moving=nib.Nifti1Image(v*(1+.03*np.sin(z/3)),a).as_reoriented([[2,1],[0,-1],[1,1]])
 moving.header.set_xyzt_units('mm')
 for action in ['register','itk-register']:
  for stage in (['both','nonlinear'] if action=='register' else ['nonlinear']):
   for reverse in [False,True]:
    folder=root/(action+stage+str(reverse));folder.mkdir();f,m=(moving,fixed) if reverse else(fixed,moving);nib.save(f,folder/'fixed.nii');nib.save(m,folder/'moving.nii')
    settings=dict(model=6,metric='nc',axes='z',grid=4,regularization=1) if action=='register' else dict(engine='itk',metric='mattes-mi',threads=1,nonlinear_iterations=2,shrink_factors=[1],smoothing_sigmas_mm=[0],bspline_spacing_mm=10,maximum_step_mm=.2)
    request=dict(action=action,fixed=str(folder/'fixed.nii'),moving=str(folder/'moving.nii'),stages=stage,settings=settings)
    if stage=='nonlinear':
     start=np.eye(4);start[:3,3]=[.15,-.1,.05];request['start']=start.tolist()
    req=folder/'request.json';req.write_text(json.dumps(request));source=folder/'source.json';frozen=folder/'frozen.json';assert run(req,source)==0,source.read_text()
    frozen_run([B,'--align-request',req,'--align-output',frozen,'--events',folder/'frozen-events.jsonl'])
    sr=json.loads(source.read_text())['result'];fr=json.loads(frozen.read_text())['result'];assert sr['matrix']==fr['matrix'];assert sr['diagnostics']==fr['diagnostics'];assert sr['deformation']['affine']==fr['deformation']['affine'];assert sr['deformation']['residualBase64']==fr['deformation']['residualBase64']
    records.append(dict(action=action,stages=stage,reverse=reverse,source_frozen_exact=True,field_sha256=hashlib.sha256(base64.b64decode(fr['deformation']['residualBase64'])).hexdigest()))
# N4 uses the same compact native library; compare actual frozen outputs.
with tempfile.TemporaryDirectory(prefix='align-native-n4-') as temporary:
 folder=Path(temporary);x,y,z=np.indices((13,15,17));values=(5+30*np.exp(-((x-6)**2+(y-7)**2+(z-8)**2)/30))*(1+.02*x);image=nib.Nifti1Image(values.astype(np.float32),np.diag([1.,1.2,1.4,1.]));image.header.set_xyzt_units('mm');nib.save(image,folder/'image.nii')
 request=dict(action='n4',image=str(folder/'image.nii'),settings=dict(schema_version=1,preset='macaque-functional',estimation_spacing_mm=1.,initial_spline_spacing_mm=20.,iterations=[2],convergence=.0001,histogram_bins=200,bias_field_fwhm=.15,wiener_noise=.01,threads=1))
 req=folder/'request.json';req.write_text(json.dumps(request));assert run(req,folder/'source.json')==0
 frozen_run([B,'--align-request',req,'--align-output',folder/'frozen.json','--events',folder/'frozen-events.jsonl'])
 sr=json.loads((folder/'source.json').read_text())['result'];fr=json.loads((folder/'frozen.json').read_text())['result'];assert sr['corrected']==fr['corrected'];assert sr['bias']==fr['bias'];assert sr['affine']==fr['affine'];records.append(dict(action='n4',source_frozen_exact=True))
report=dict(version='0.17.7',status='passed',runtime='private Align numerical worker',no_qt_or_standalone_gui=True,host_python_and_engine_override_ignored=True,records=records)
(ROOT/'scientific/runtime-validation.json').write_text(json.dumps(report,indent=2)+'\n');print('PASS source/frozen complete fields and physical directions for JIP and ITK; no Qt, external app or Python installation needed.')
