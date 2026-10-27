from platform_tools import executable
from pathlib import Path
import argparse,json,math,struct,subprocess,zlib
p=argparse.ArgumentParser();p.add_argument('--build-dir',type=Path,required=True);a=p.parse_args()
R=Path(__file__).resolve().parents[1];O=R/'validation'/('configured-cli-'+a.build_dir.name);O.mkdir(parents=True,exist_ok=True)
runner=executable(a.build_dir.resolve(),'jip-register')
side=9;header=bytearray(352);struct.pack_into('<i',header,0,348);struct.pack_into('<8h',header,40,3,side,side,side,1,1,1,1)
struct.pack_into('<2h',header,70,16,32);struct.pack_into('<8f',header,76,1,1,1.5,2,1,0,0,0);struct.pack_into('<f',header,108,352);header[123]=2
struct.pack_into('<2h',header,252,0,1);struct.pack_into('<12f',header,280,1,0,0,0,0,1.5,0,0,0,0,2,0);header[344:348]=b'n+1\0'
values=[1+80*math.exp(-((x-3)**2+(y-4)**2+(z-4)**2)/8)+15*math.exp(-((x-6)**2+(y-2)**2+(z-3)**2)/3) for z in range(9) for y in range(9) for x in range(9)]
(O/'input.nii').write_bytes(header+struct.pack('<729f',*values))
def run(name,args,expected=0):
    p=subprocess.run([str(runner),*args],cwd=O,capture_output=True,text=True,timeout=90)
    (O/(name+'.log')).write_text(p.stdout+p.stderr);assert p.returncode==expected,(name,p.returncode,p.stderr)
base=['--fixed','input.nii','--moving','input.nii','--metric','nc','--grid','4']
run('affine',base+['--stages','affine','--affine-model','9','--affine-metric','nmi','--jacobian-limit','2.5','--stiffness','.2','--nonlinear-axes','z','--matching','all','--wrap','y','--output','affine.nii','--checkpoint','affine.jipcp'])
assert (O/'affine.jipcp').read_bytes()[:8]==b'JIPCP003'
meta=json.loads((O/'affine.nii.json').read_text());assert meta['affine_model']==9 and meta['affine_metric']==1 and meta['completed_stages']==1 and meta['nonlinear_axes']==[0,0,1] and meta['wrap_around']==[0,1,0]
run('restore',['--resume','affine.jipcp','--output','restored.nii']);assert (O/'affine.nii').read_bytes()==(O/'restored.nii').read_bytes()
run('follow-with-nonlinear',['--resume','affine.jipcp','--continue','--stages','nonlinear','--output','nonlinear.nii','--checkpoint','nonlinear.jipcp'])
assert json.loads((O/'nonlinear.nii.json').read_text())['completed_stages']==3
# Bad extended configuration is rejected even with a valid recomputed CRC.
corrupt=bytearray((O/'affine.jipcp').read_bytes());struct.pack_into('<I',corrupt,88 if corrupt[:8]==b'JIPCP003' else 80,7);struct.pack_into('<I',corrupt,len(corrupt)-4,zlib.crc32(corrupt[:-4]));(O/'bad.jipcp').write_bytes(corrupt)
run('bad-checkpoint',['--resume','bad.jipcp','--output','bad.nii'],4);assert not (O/'bad.nii').exists()
for i,extra in enumerate([['--affine-model','7'],['--nonlinear-axes','xx'],['--stages','other'],['--stiffness','2'],['--jacobian-limit','1'],['--affine-metric','nc']]):
    run('bad-option-'+str(i),base+extra+['--output','bad.nii'],2)
run('resume-needs-continue',['--resume','affine.jipcp','--stages','nonlinear','--output','bad.nii'],2)
(O/'results.json').write_text(json.dumps({'status':'PASS','configured_affine':'PASS','restore':'byte exact','explicit_followup_stage':'PASS','invalid_options':7,'bad_config_with_valid_crc':'rejected'},indent=2)+'\n')
print('CONFIGURED_CLI_RESULT options, extended checkpoint, explicit followup, invalid config/arguments pass')
