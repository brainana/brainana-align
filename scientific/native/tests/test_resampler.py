from platform_tools import executable
from pathlib import Path
import argparse,hashlib,json,subprocess
R=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('--build-dir',type=Path,required=True);a=p.parse_args()
reference=R/'tests/fixtures/resampler-0.4.0.inc'
manifest=json.loads((R/'tests/fixtures/resampler-reference.json').read_text())
assert hashlib.sha256(reference.read_bytes()).hexdigest()==manifest['sha256'],'Frozen reference modified'
result=subprocess.run([str(executable(a.build_dir.resolve(),'jip-resampler-test'))],capture_output=True,text=True,timeout=180)
D=R/'validation'/('resampler-'+a.build_dir.name);D.mkdir(parents=True,exist_ok=True)
(D/'run.log').write_text(result.stdout+result.stderr)
assert result.returncode==0,(result.stdout+result.stderr)[-6000:]
assert 'cases=1024 reference=byte-exact geometry=restored ownership=zero' in result.stdout
(D/'result.json').write_text(json.dumps(dict(status='PASS',cases=1024,reference_sha256=manifest['sha256'],comparison='exact full output, including untouched local voxels; geometry restored; no tracked ownership'),indent=2)+'\n')
print(result.stdout.splitlines()[-1])
