"""Compare historical-checkpoint restore/continuation against an independent
same-architecture prior engine, without weakening the ARM gold-fixture gate."""
from pathlib import Path
import argparse,hashlib,json,struct,subprocess
p=argparse.ArgumentParser();p.add_argument('--candidate',type=Path,required=True);p.add_argument('--baseline',type=Path,required=True);p.add_argument('--output',type=Path,required=True);a=p.parse_args();a.output.mkdir(parents=True,exist_ok=True)
checkpoint=Path(__file__).resolve().parent/'fixtures/0.1.0-registration.jipcp';report=[]
for continued in [False,True]:
    results={}
    for route,binary in [('candidate',a.candidate),('independent-prior',a.baseline)]:
        folder=(a.output/f'{route}-{continued}').resolve();folder.mkdir(exist_ok=True)
        args=[str(binary.resolve()),'--resume',str(checkpoint),'--output',str(folder/'aligned.nii'),'--field',str(folder/'field.nii')]+(['--continue'] if continued else [])
        with (folder/'run.log').open('w') as log:subprocess.run(args,stdout=log,stderr=log,check=True,timeout=90)
        results[route]={}
        for name in ['aligned.nii','field.nii']:
            data=(folder/name).read_bytes();offset=int(struct.unpack_from('<f',data,108)[0]);results[route][name]=dict(payload_sha256=hashlib.sha256(data[offset:]).hexdigest(),geometry=list(struct.unpack_from('<12f',data,280)))
    assert results['candidate']==results['independent-prior'];report.append(dict(continued=continued,exact=True,results=results))
(a.output/'report.json').write_text(json.dumps(report,indent=2)+'\n');print('PASS restore and continuation matched independent prior Intel engine exactly')
