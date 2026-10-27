from platform_tools import executable
from pathlib import Path
import argparse,hashlib,json,struct,subprocess
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
        assert hashlib.sha256(b[at:]).hexdigest()==gold['data_sha256']
        assert list(struct.unpack_from('<12f',b,280))==gold['native_affine']
(O/'results.json').write_text(json.dumps({'status':'PASS','previous_version':'0.1.0','restore':'exact image/field payloads and geometry','continuation':'exact image/field payloads and geometry'},indent=2)+'\n')
print('CHECKPOINT_COMPAT_RESULT 0.1.0 restore and continuation match exactly')
