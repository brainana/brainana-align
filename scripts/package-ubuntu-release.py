#!/usr/bin/env python3
"""Package a verified Ubuntu x64 Debian installer, portable app and shared source."""
import argparse,json,shutil,tempfile,tarfile,stat
from pathlib import Path
from release_archives import digest,inventory,archive,link_or_copy
parser=argparse.ArgumentParser()
for name in ['app','deb','evidence','out']:parser.add_argument('--'+name,type=Path,required=True)
args=parser.parse_args();root=Path(__file__).resolve().parent.parent
meta=json.loads((root/'VERSION.json').read_text());version=meta['version']
for name in ['packaging-ubuntu.json','deb-integrity.json']:
    if json.loads((args.evidence/name).read_text())['status']!='passed':raise ValueError('Missing verified package: '+name)
out=args.out.resolve();out.mkdir(parents=True,exist_ok=True)
deb=out/args.deb.name;shutil.copy2(args.deb,deb)
with tempfile.TemporaryDirectory(prefix='brainana-ubuntu-release-') as temporary:
    temporary=Path(temporary);source=temporary/'source'
    shutil.copytree(root,source,ignore=shutil.ignore_patterns('node_modules','.git','.DS_Store','__pycache__','release','browser-support','workstation-support','node-runtimes','*.app'))
    shutil.copytree(args.evidence,source/'Validation',dirs_exist_ok=True);inventory(source)
    source_zip=out/f'Brainana-Align-v{version}-source.zip';archive(source,source_zip)
    app=temporary/'brainana-align';shutil.copytree(args.app,app,symlinks=True,copy_function=link_or_copy)
    shutil.copytree(root/'Documentation',app/'Documentation');shutil.copytree(args.evidence,app/'Validation')
    for name in ['VERSION.json','AGENTS.md','LICENSE','NOTICE.txt']:shutil.copy2(root/name,app/name)
    (app/'README-FIRST.md').write_text(f'''# Brainana Align Ubuntu x64 v{version}

Preferred installation: `sudo apt install ./Brainana-Align-Ubuntu-{version}-amd64.deb`. Then open Brainana Align from Applications or run `brainana-align`. Remove using `sudo apt remove brainana-align`. Ubuntu installs the declared system libraries and the normal per-app sandbox/desktop integration. No separate browser, Node or Python is required.

Portable use: extract the complete tar.gz, enter the brainana-align folder and run `./brainana-align`. Keep all files together. Portable use requires the system libraries and permitted user namespaces already to be available. If the sandbox cannot start, use the .deb; do not disable the sandbox.

All 6/7/9/12-parameter landmark options and the advanced spline are included, with automatic registration/refinement, manual adjustment, guided setup, settings, sessions and exports. Local files, mounted network shares and the in-app SSH/SFTP connection/file browser are supported. The older macOS interactive launcher is unavailable on Linux.

Cross-built on macOS. Ubuntu GUI/GPU, installation/removal and real-data validation remain pending. See Documentation/VALIDATION.md and SHA256SUMS.txt.
''');inventory(app)
    portable=out/f'Brainana-Align-Ubuntu-v{version}-x64-portable.tar.gz'
    def portable_metadata(info):
        info.uid=info.gid=0;info.uname=info.gname='root';info.mode &= ~0o6000
        return info
    with tarfile.open(portable,'w:gz',compresslevel=6) as tar:tar.add(app,arcname='brainana-align',filter=portable_metadata)
    with tarfile.open(portable,'r:gz') as tar:
        for line in (app/'SHA256SUMS.txt').read_text().splitlines():
            expected,name=line.split('  ',1)
            import hashlib
            assert hashlib.sha256(tar.extractfile('brainana-align/'+name).read()).hexdigest()==expected,name
        assert tar.getmember('brainana-align/brainana-align').mode & 0o111
        for link in json.loads((app/'SYMLINKS.json').read_text()):assert tar.getmember('brainana-align/'+link['path']).linkname==link['target']
    full=temporary/'full';full.mkdir();shutil.copytree(app,full/'Ubuntu-x64',symlinks=True,copy_function=link_or_copy)
    (full/'Installer').mkdir();shutil.copy2(deb,full/'Installer'/deb.name)
    (full/'Source').mkdir();shutil.copy2(source_zip,full/'Source'/source_zip.name)
    shutil.copy2(app/'README-FIRST.md',full/'README-FIRST.md');shutil.copytree(root/'Documentation',full/'Documentation')
    inventory(full);bundle=out/f'Brainana-Align-Ubuntu-v{version}-full-release.zip';archive(full,bundle)
shutil.copy2(root/'Documentation/VALIDATION.md',out/'Validation.md');shutil.copy2(root/'Documentation/TECHNICAL_FINDINGS.md',out/'Audit.md')
assets=[deb,portable,source_zip,bundle,out/'Validation.md',out/'Audit.md']
manifest={'application':meta['app'],'version':version,'buildId':meta['buildId'],'platform':'linux','architecture':'x64/amd64','validationStatus':'cross-built candidate; native Ubuntu GUI/install/remove pending','artifacts':[{'file':p.name,'bytes':p.stat().st_size,'sha256':digest(p)} for p in assets]}
(out/'RELEASE_ASSETS.json').write_text(json.dumps(manifest,indent=2)+'\n');assets.append(out/'RELEASE_ASSETS.json')
(out/'SHA256SUMS.txt').write_text(''.join(f'{digest(p)}  {p.name}\n' for p in assets))
print('Ubuntu installer, executable portable tar.gz, complete source and full bundle verified; SHA256 manifests generated.')
