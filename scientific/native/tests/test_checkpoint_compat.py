from platform_tools import executable
from pathlib import Path
import argparse,gzip,hashlib,json,math,os,struct,subprocess
p=argparse.ArgumentParser();p.add_argument('--build-dir',type=Path,required=True);a=p.parse_args()
R=Path(__file__).resolve().parents[1];F=R/'tests/fixtures';O=R/'validation'/('compatibility-'+a.build_dir.name);O.mkdir(parents=True,exist_ok=True)
expected=json.loads((F/'0.1.0-expected.json').read_text());checkpoint=F/'0.1.0-registration.jipcp'
assert hashlib.sha256(checkpoint.read_bytes()).hexdigest()==expected['checkpoint_sha256']
for case in expected['cases']:
    name='continued' if case['continue'] else 'restored'
    args=[str(executable(a.build_dir.resolve(),'jip-register')),'--resume',str(checkpoint),'--output',str(O/(name+'.nii')),'--field',str(O/(name+'-field.nii'))]
    if case['continue']:args.append('--continue')
    result=subprocess.run(args,capture_output=True,text=True,timeout=60);assert result.returncode==0,result.stderr
    (O/(name+'.log')).write_text(result.stdout+result.stderr)
    for suffix in ('','-field'):
        b=(O/(name+suffix+'.nii')).read_bytes();at=int(struct.unpack_from('<f',b,108)[0]);gold=case['outputs'][suffix or 'image']
        actual=b[at:]
        if case['continue'] and os.environ.get('JIP_PORTABLE_REFERENCE'):
            baseline=gzip.decompress((F/f'0.1.0-{name}{suffix}.f32.gz').read_bytes())
            assert hashlib.sha256(baseline).hexdigest()==gold['data_sha256']
            assert len(actual)==len(baseline)
            expected=struct.unpack('<'+str(len(baseline)//4)+'f',baseline)
            values=struct.unpack('<'+str(len(actual)//4)+'f',actual)
            assert all(math.isfinite(v) and abs(v-g)<=1e-4+1e-6*abs(g) for v,g in zip(values,expected)), f'{name}{suffix}: portable tolerance exceeded'
            print(f'PORTABLE_CHECKPOINT {name}{suffix} max_abs={max(abs(v-g) for v,g in zip(values,expected)):.9g}')
        else:
            assert hashlib.sha256(actual).hexdigest()==gold['data_sha256'], f'{name}{suffix}: voxel payload differs from stored 0.1.0 reference'
        assert list(struct.unpack_from('<12f',b,280))==gold['native_affine']
(O/'results.json').write_text(json.dumps({'status':'PASS','previous_version':'0.1.0','restore':'exact image/field payloads and geometry','continuation':'bounded cross-platform float comparison' if os.environ.get('JIP_PORTABLE_REFERENCE') else 'exact image/field payloads and geometry'},indent=2)+'\n')
print('CHECKPOINT_COMPAT_RESULT 0.1.0 restore exact; continuation follows declared platform policy')
