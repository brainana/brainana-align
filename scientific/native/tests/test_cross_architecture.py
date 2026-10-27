"""Compare actual 64-bit ARM and Rosetta Intel reproducible full-fit traces.
Requires both local macOS builds; does not claim physical Intel qualification.
"""
from pathlib import Path
import argparse,hashlib,json,subprocess
p=argparse.ArgumentParser();p.add_argument('--arm',type=Path,required=True);p.add_argument('--intel',type=Path,required=True);p.add_argument('--output',type=Path,required=True);a=p.parse_args()
a.output.mkdir(parents=True,exist_ok=True);report={'scope':'ARM execution versus Intel execution through Rosetta on one Apple Silicon Mac','cases':[]}
for metric in [0,1]:
    for grid in [1,4]:
        results={}
        for arch,build in [('arm64',a.arm),('x86_64',a.intel)]:
            folder=(a.output/f'{arch}-{metric}-{grid}').resolve();folder.mkdir(exist_ok=True)
            executable=build.resolve()/'engine/jip-api-test-repro'
            with (folder/'run.log').open('w') as log:subprocess.run([str(executable),str(metric),str(grid),str(folder/'trace.bin')],cwd=folder,stdout=log,stderr=log,check=True,timeout=180)
            results[arch]={name:hashlib.sha256((folder/name).read_bytes()).hexdigest() for name in ['trace.bin','aligned.nii','distortions.nii']}
        checks={name:results['arm64'][name]==results['x86_64'][name] for name in results['arm64']}
        report['cases'].append(dict(metric=metric,grid=grid,exact=checks,sha256=results));assert all(checks.values()),report['cases'][-1]
(a.output/'report.json').write_text(json.dumps(report,indent=2)+'\n');print('PASS four reproducible full traces and image/field files exactly matched actual ARM and Rosetta Intel execution')
