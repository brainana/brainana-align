"""Verify the actual frozen numerical modules and dependency closure."""
from pathlib import Path
import hashlib,json,marshal,subprocess,sys,types
from PyInstaller.archive.readers import CArchiveReader
R=Path(__file__).resolve().parents[2];S=R/'scientific';D=Path(sys.argv[1]) if len(sys.argv)>1 else R/'generated/scientific-runtime';B=D/'BrainanaAlignRegistration'
reader=CArchiveReader(str(B));archive=reader.open_embedded_archive('PYZ.pyz')
def normalize(code):return code.replace(co_filename='',co_consts=tuple(normalize(c) if isinstance(c,types.CodeType) else c for c in code.co_consts))
checks={}
for path in (S/'brainana_registration').glob('*.py'):
 name='brainana_registration'+('' if path.stem=='__init__' else '.'+path.stem)
 assert normalize(archive.extract(name))==normalize(compile(path.read_bytes(),str(path),'exec')),name
 checks[name]=hashlib.sha256(path.read_bytes()).hexdigest()
assert normalize(marshal.loads(reader.extract('launch')))==normalize(compile((S/'launch.py').read_bytes(),str(S/'launch.py'),'exec'))
assert not any(name.startswith(('PyQt','PySide','jip_workbench','scipy','sphinx','babel','SimpleITK')) for name in archive.toc)
assert not any('SimpleITK' in p.parts for p in D.rglob('*'))
assert sum(p.stat().st_size for p in D.rglob('*') if p.is_file() and not p.is_symlink())<90_000_000,'Scientific runtime exceeded 90 MB budget'
files=[]
for p in D.rglob('*'):
 if not p.is_file() or p.is_symlink():continue
 with p.open('rb') as f:magic=f.read(4)
 if magic not in [b'\xcf\xfa\xed\xfe',b'\xfe\xed\xfa\xcf',b'\xca\xfe\xba\xbe',b'\xbe\xba\xfe\xca']:continue
 arch=subprocess.check_output(['lipo','-archs',str(p)],text=True).strip();assert arch=='arm64',(str(p),arch)
 deps=[]
 for line in subprocess.check_output(['otool','-L',str(p)],text=True).splitlines()[1:]:
  dep=line.strip().split(' (')[0]
  assert not dep.startswith('/') or dep.startswith(('/System/','/usr/lib/')),dep
  assert 'Qt' not in dep,dep
  deps.append(dep)
 files.append(dict(path=p.relative_to(D).as_posix(),architecture=arch,dependencies=deps))
report=dict(status='passed',version='0.17.7',source_modules_exact=checks,entry_point_exact=True,no_standalone_gui_or_qt=True,mach_o_files=files,bytes_regular=sum(p.stat().st_size for p in D.rglob('*') if p.is_file() and not p.is_symlink()))
(Path(sys.argv[2]) if len(sys.argv)>2 else S/'runtime-audit.json').write_text(json.dumps(report,indent=2)+'\n');print('PASS',len(checks),'source modules, entry point and',len(files),'arm64 native files; no Qt/standalone GUI or external non-system dependencies.')
