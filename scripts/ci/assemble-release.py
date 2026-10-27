"""Aggregate successful platform assets, exact tracked source and provenance."""
from pathlib import Path
import hashlib,json,subprocess,zipfile,shutil
root=Path.cwd();out=root/'release-output';out.mkdir(exist_ok=True);meta=json.loads((root/'VERSION.json').read_text());version=meta['version']
commit=subprocess.check_output(['git','rev-parse','HEAD']).decode().strip()
platforms=[]
for p in out.glob('assets-*.json'):
 record=json.loads(p.read_text());assert record['version']==version and record['sourceCommit']==commit;platforms.append((record['platform'],record['architecture']))
 for asset in record['assets']:
  payload=out/asset['file'];assert payload.is_file() and hashlib.sha256(payload.read_bytes()).hexdigest()==asset['sha256'],payload
assert len(platforms)==4 and set(platforms)=={('darwin','arm64'),('darwin','x64'),('windows','x64'),('linux','x64')},'All four required platform/architecture manifests must be present'
source=out/f'brainana-align-{version}-source.zip'
subprocess.run(['git','archive','--format=zip','--prefix=brainana-align/','--output='+str(source),commit],check=True)
full=out/f'brainana-align-{version}-full-release.zip';full.unlink(missing_ok=True)
shutil.copytree(root/'Documentation',out/'Documentation',dirs_exist_ok=True) if (root/'Documentation').exists() else None
shutil.copytree(root/'docs',out/'docs',dirs_exist_ok=True) if (root/'docs').exists() else None
shutil.copy2(root/'README.md',out/'README.md')
items=[]
for p in sorted(out.rglob('*')):
 if p.is_file() and p.name not in ['RELEASE_ASSETS.json','SHA256SUMS.txt']:
  items.append({'file':p.relative_to(out).as_posix(),'bytes':p.stat().st_size,'sha256':hashlib.sha256(p.read_bytes()).hexdigest()})
(out/'RELEASE_ASSETS.json').write_text(json.dumps({'application':meta['app'],'version':version,'commit':commit,'platforms':platforms,'assets':items,'nativeGPUReview':'not established by headless CI'},indent=2)+'\n')
(out/'SHA256SUMS.txt').write_text(''.join(f'{hashlib.sha256(p.read_bytes()).hexdigest()}  {p.relative_to(out).as_posix()}\n' for p in sorted(out.rglob('*')) if p.is_file() and p.name!='SHA256SUMS.txt'))
# Complete distribution includes every platform installer, source, checksums and validation.
full=out/f'brainana-align-{version}-full-release.zip'
with zipfile.ZipFile(full,'w',zipfile.ZIP_DEFLATED,compresslevel=6) as archive:
 for p in sorted(out.rglob('*')):
  if p.is_file() and p!=full:archive.write(p,p.relative_to(out).as_posix())
with (out/'SHA256SUMS.txt').open('a') as stream:stream.write(f'{hashlib.sha256(full.read_bytes()).hexdigest()}  {full.name}\n')
print('Assembled all platform installers, exact commit source, full release and checksums.')
