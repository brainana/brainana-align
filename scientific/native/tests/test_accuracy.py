from platform_tools import executable
from pathlib import Path
import argparse,json,subprocess
p=argparse.ArgumentParser();p.add_argument('--build-dir',type=Path,required=True);a=p.parse_args()
R=Path(__file__).resolve().parents[1];O=R/'validation'/('accuracy-'+a.build_dir.name);O.mkdir(parents=True,exist_ok=True)
policy=json.loads((R/'tests/accuracy-policy.json').read_text());results=[]
for case in policy['cases']:
    d=O/case['name'];d.mkdir(exist_ok=True)
    process=subprocess.run([str(executable(a.build_dir.resolve(),'jip-accuracy-test')),str(case['id'])],cwd=d,capture_output=True,text=True,timeout=240)
    (d/'run.log').write_text(process.stdout+process.stderr)
    assert process.returncode==0,process.stderr[-3000:]
    data=json.loads(next(line[len('ACCURACY_JSON '):] for line in process.stdout.splitlines() if line.startswith('ACCURACY_JSON ')))
    data['name']=case['name'];results.append(data)
    (O/'results.json').write_text(json.dumps({'policy':policy,'results':results},indent=2)+'\n')
    assert data['final_tre_mm']<=case['maximum_tre_mm'],data
    assert data['final_mse']<=data['initial_mse']*case['maximum_mse_ratio'],data
    assert data['jacobian_min']>0,data
print('ACCURACY_RESULT five known-truth cases pass mapping, image-error, topology and restore checks')
