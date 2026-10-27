"""Compare complete CPU/threaded fits to a preserved previous release."""
from pathlib import Path
import hashlib,json,os,sys,tempfile
import nibabel as nib
import numpy as np

ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'frontend'))
from jip_workbench.qualification import CONFIGURATIONS,run_case
from jip_workbench import __version__

def main():
    previous=Path(sys.argv[1]).resolve()
    previous_version=sys.argv[2] if len(sys.argv)>2 else '0.7.0'
    report={'scope':'Full synthetic fit comparisons; no physical GPU execution implied','cases':[]}
    original=os.environ.get('JIP_ENGINE_LIBRARY')
    try:
        with tempfile.TemporaryDirectory(prefix='jip-metal-fix-parity-') as folder:
            folder=Path(folder)
            for arithmetic in ['compatibility','reproducible']:
                library='libjip-engine'+('-repro' if arithmetic=='reproducible' else '')+'.dylib'
                for configuration in CONFIGURATIONS:
                    records={};images={}
                    for version,root in [(previous_version,previous),(__version__,ROOT/'build/engine')]:
                        os.environ['JIP_ENGINE_LIBRARY']=str(root/library)
                        for backend in ['cpu','threads']:
                            key=version+'-'+backend;output=folder/(arithmetic+'-'+configuration[0]+'-'+key);output.mkdir()
                            record=run_case(configuration,backend,output,arithmetic)
                            images[key]={}
                            for name in ['aligned.nii','field.nii','jacobian.nii.gz']:
                                image=nib.load(record['folder']/name)
                                images[key][name]=hashlib.sha256(np.asarray(image.dataobj).tobytes()+image.affine.tobytes()).hexdigest()
                            record.pop('folder');records[key]=record
                    traces={v['trace_sha256'] for v in records.values()}
                    exact=len(traces)==1 and all(value==images[previous_version+'-cpu'] for value in images.values())
                    report['cases'].append(dict(case=configuration[0],arithmetic=arithmetic,exact=exact,runs=records,output_hashes=images))
                    assert exact,(configuration[0],arithmetic)
        report['status']='passed'
    finally:
        if original is None:os.environ.pop('JIP_ENGINE_LIBRARY',None)
        else:os.environ['JIP_ENGINE_LIBRARY']=original
        (ROOT/'evidence/Previous-release-fit-parity.json').write_text(json.dumps(report,indent=2)+'\n')
    print('PASS: eight model/arithmetic configurations; 32 complete fits; traces, images, fields, Jacobians and geometry exact')

if __name__=='__main__':main()
