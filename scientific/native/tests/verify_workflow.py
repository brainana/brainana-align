from platform_tools import executable
"""Real executable, complete search traces, native headers and restore checks."""
from pathlib import Path
import argparse,gzip,hashlib,json,math,struct,subprocess
P=argparse.ArgumentParser();P.add_argument('--build',default='headless');P.add_argument('--build-dir',type=Path);P.add_argument('--reference-results',type=Path,help='Independent architecture-matched legacy reference directory; never regenerate from the candidate');A=P.parse_args()
R=Path(__file__).resolve().parents[1];build=A.build_dir.resolve() if A.build_dir else R/'build'/A.build
reference_root=A.reference_results.resolve() if A.reference_results else R/'reference/results'
B=build/'engine';O=R/'validation'/('workflow-'+build.name);O.mkdir(parents=True,exist_ok=True)
records=[]
def run(args,cwd,success=True):
    p=subprocess.run([str(x) for x in args],cwd=cwd,capture_output=True,text=True,timeout=180)
    (cwd/'run.log').write_text(p.stdout+p.stderr)
    assert (p.returncode==0)==success,(args,p.returncode,p.stdout[-1000:],p.stderr[-1000:])
    return p
for metric in (0,1):
    for grid in (1,4):
        name=f'metric-{metric}-grid-{grid}';d=O/name;d.mkdir(exist_ok=True)
        run([executable(build,'jip-api-test'),metric,grid,d/'trace.bin'],d)
        actual=(d/'trace.bin').read_bytes();reference_path=reference_root/name/('legacy-trace.bin' if A.reference_results else 'trace.bin')
        reference=reference_path.read_bytes() if reference_path.exists() else gzip.decompress(reference_path.with_suffix('.bin.gz').read_bytes())
        assert actual==reference,name+' changed numerical trace'
        for first,second in [('aligned.nii','restored.nii'),('distortions.nii','restored-field.nii')]:
            assert (d/first).read_bytes()==(d/second).read_bytes()
        records.append({'id':'REF-'+name,'status':'PASS','comparison':'byte exact full coefficient, objective and displacement trace','bytes':len(actual),'sha256':hashlib.sha256(actual).hexdigest()})
N=9;count=N**3
native=[0,-1.5,0,20,1,0,0,-10,0,0,2,7,0,0,0,1]
values=[1+80*math.exp(-((x-3)**2+(y-4)**2+(z-4)**2)/8)+15*math.exp(-((x-6)**2+(y-2)**2+(z-3)**2)/3) for z in range(N) for y in range(N) for x in range(N)]
def image(path,scaled=False,qform=False):
    h=bytearray(352);struct.pack_into('<i',h,0,348);struct.pack_into('<8h',h,40,3,N,N,N,1,1,1,1);struct.pack_into('<2h',h,70,16,32)
    struct.pack_into('<8f',h,76,1,1,1.5,2,1,0,0,0);struct.pack_into('<f',h,108,352);h[123]=2
    if scaled:struct.pack_into('<2f',h,112,2,3)
    if qform:
        struct.pack_into('<2h',h,252,1,0);struct.pack_into('<6f',h,256,0,0,math.sqrt(.5),20,-10,7)
    else:
        struct.pack_into('<2h',h,252,0,1);struct.pack_into('<12f',h,280,*native[:12])
    h[344:348]=b'n+1\0';payload=h+struct.pack('<729f',*values)
    path.write_bytes(gzip.compress(payload) if path.suffix=='.gz' else payload)
for kind in ('sform','qform','gzip-scaled'):
    d=O/kind;d.mkdir(exist_ok=True);suffix='.nii.gz' if kind=='gzip-scaled' else '.nii'
    fixed=d/('fixed'+suffix);moving=d/('moving'+suffix);image(fixed,kind=='gzip-scaled',kind=='qform');image(moving,kind=='gzip-scaled',kind=='qform')
    run([executable(build,'jip-register'),'--fixed',fixed,'--moving',moving,'--metric','nc','--grid','4','--regularization','1','--output','aligned.nii','--field','field.nii','--checkpoint','saved.jipcp'],d)
    run([executable(build,'jip-register'),'--resume','saved.jipcp','--output','reopened.nii','--field','reopened-field.nii'],d)
    assert (d/'aligned.nii').read_bytes()==(d/'reopened.nii').read_bytes()
    assert (d/'field.nii').read_bytes()==(d/'reopened-field.nii').read_bytes()
    h=(d/'aligned.nii').read_bytes();m=struct.unpack_from('<12f',h,280)
    assert max(abs(a-b) for a,b in zip(m,native))<1e-4
    metadata=json.loads((d/'field.nii.json').read_text());assert 'pull map A*x+u(x)' in metadata['mapping']
    assert max(abs(a-b) for a,b in zip(metadata['fixed_voxel_to_native_mm'],native))<1e-4
    records.append({'id':'CLI-'+kind,'status':'PASS','native_geometry_preserved':True,'restore_export':'byte exact'})
    # Output validation errors and malformed sessions do not alter the inputs.
    before=hashlib.sha256(fixed.read_bytes()).hexdigest()
    run([executable(build,'jip-register'),'--fixed',fixed,'--moving',moving,'--output',fixed],d,False)
    assert hashlib.sha256(fixed.read_bytes()).hexdigest()==before
    (d/'broken.jipcp').write_bytes((d/'saved.jipcp').read_bytes()[:120])
    run([executable(build,'jip-register'),'--resume','broken.jipcp','--output','should-not-exist.nii'],d,False)
    assert not (d/'should-not-exist.nii').exists()
    records.append({'id':'ERROR-'+kind,'status':'PASS','cases':['input overwrite rejected','truncated checkpoint rejected without output']})
d=O/'wide-offset';d.mkdir(exist_ok=True);image(d/'normal.nii')
prefix=bytearray((d/'normal.nii').read_bytes()[:352]);offset=3*1024**3;struct.pack_into('<f',prefix,108,offset)
sparse=d/'sparse.nii'
with sparse.open('wb') as f:
    f.write(prefix);f.seek(offset);f.write(struct.pack('<729f',*values))
try:
    run([executable(build,'jip-register'),'--fixed',sparse,'--moving',d/'normal.nii','--metric','nc','--grid','4','--regularization','1','--output','aligned.nii','--checkpoint','wide.jipcp'],d)
    checkpoint=(d/'wide.jipcp').read_bytes();payload=324 if checkpoint[:8]==b'JIPCP003' else 316 if checkpoint[:8]==b'JIPCP002' else 260
    saved=struct.unpack_from('<729f',checkpoint,payload)
    expected=struct.unpack('<729f',struct.pack('<729f',*values));assert saved==expected
finally:sparse.unlink(missing_ok=True)
records.append({'id':'IO-WIDE-OFFSET','status':'PASS','payload_offset_bytes':offset,'input_float32_samples':'exact'})
# Boundary rejection belongs to the API; do not silently rewrite nonfinite data.
for name,changed in [('nonfinite',[math.nan]+values[1:]),('uniform',[5.]*count)]:
    d=O/name;d.mkdir(exist_ok=True);image(d/'input.nii');h=(d/'input.nii').read_bytes()[:352]
    (d/'input.nii').write_bytes(h+struct.pack('<729f',*changed))
    run([executable(build,'jip-register'),'--fixed',d/'input.nii','--moving',O/'wide-offset/normal.nii','--output','must-not-exist.nii'],d,False)
    assert not (d/'must-not-exist.nii').exists();records.append({'id':'INPUT-'+name,'status':'PASS','rejected_before_registration':True})
(O/'results.json').write_text(json.dumps(records,indent=2)+'\n')
print(f'{len(records)} workflow groups passed; {sum(r.get("bytes",0) for r in records):,} trace bytes matched.')
