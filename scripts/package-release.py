#!/usr/bin/env python3
"""Package an already built and validated Brainana Align release."""
import argparse
import hashlib
import json
import os
import stat
import shutil
import subprocess
import sys
import tempfile
import zipfile
from pathlib import Path

parser = argparse.ArgumentParser()
parser.add_argument('--app-package', required=True, type=Path)
parser.add_argument('--architecture', choices=['universal','arm64','x64'], default='universal')
parser.add_argument('--additional-app-package', action='append', default=[], help='architecture=staging-directory')
parser.add_argument('--browser-package', type=Path)
parser.add_argument('--evidence', required=True, type=Path)
parser.add_argument('--out', required=True, type=Path)
args = parser.parse_args()
root = Path(__file__).resolve().parent.parent
meta = json.loads((root/'apps/align/VERSION.json').read_text())
version = meta['version']
out = args.out.resolve()
out.mkdir(parents=True, exist_ok=True)
app_packages=[(args.architecture,args.app_package)]
for value in args.additional_app_package:
    architecture,directory=value.split('=',1)
    if architecture not in ['arm64','x64','universal'] or architecture in [a for a,p in app_packages]:
        raise ValueError('Invalid or duplicate architecture: '+architecture)
    app_packages.append((architecture,Path(directory)))
all_packages=[p for a,p in app_packages]+([args.browser_package] if args.browser_package else [])
if sys.platform == 'darwin':
  for package in all_packages:
    for app_bundle in package.glob('*.app'):
        subprocess.run(['codesign','--verify','--deep','--strict',str(app_bundle)],check=True)

from release_archives import digest, files, inventory, archive

with tempfile.TemporaryDirectory(prefix='brainana-release-') as scratch:
    scratch = Path(scratch)
    source = scratch/'apps/align'
    shutil.copytree(root, source, ignore=shutil.ignore_patterns('node_modules','.git','.DS_Store','__pycache__','release','browser-support','workstation-support','node-runtimes','*.app'))
    shutil.copytree(args.evidence, source/'Validation', dirs_exist_ok=True)
    inventory(source)
    source_zip = out/f'Brainana-Align-v{version}-source.zip'
    archive(source, source_zip)
    package_archives=[]
    staged=[]
    for architecture,directory in app_packages + ([('browser-universal',args.browser_package)] if args.browser_package else []):
        app=scratch/architecture
        # Payloads are immutable after signing/verification. Link them on the same
        # filesystem to avoid another full app allocation; copy across filesystems.
        def link_or_copy(src,dst):
            try: os.link(src,dst)
            except OSError: shutil.copy2(src,dst)
            return dst
        shutil.copytree(directory,app,symlinks=True,copy_function=link_or_copy)
        shutil.copytree(root/'Documentation',app/'Documentation',dirs_exist_ok=True)
        shutil.copytree(args.evidence,app/'Validation',dirs_exist_ok=True)
        inventory(app)
        suffix='' if architecture=='universal' else '-'+architecture
        app_zip=out/f'Brainana-Align-macOS-v{version}{suffix}.zip'
        archive(app,app_zip)
        package_archives.append(app_zip)
        staged.append((architecture,app))
    bundle=scratch/'full-release'
    bundle.mkdir()
    for architecture,app in staged:
        # Staged app files are immutable and share this temporary filesystem.
        # Hard links avoid a second complete payload copy while assembling the bundle.
        shutil.copytree(app,bundle/architecture,symlinks=True,copy_function=os.link)
    shutil.copytree(root/'Documentation',bundle/'Documentation')
    shutil.copytree(args.evidence,bundle/'Validation')
    shutil.copy2(root/'README-FIRST.md',bundle/'README-FIRST.md')
    (bundle/'Source').mkdir()
    shutil.copy2(source_zip,bundle/'Source'/source_zip.name)
    inventory(bundle)
    full_zip=out/f'Brainana-Align-v{version}-full-release.zip'
    archive(bundle,full_zip)

(root/'Documentation/TECHNICAL_FINDINGS.md').read_text()
(out/'Audit.md').write_text((root/'Documentation/TECHNICAL_FINDINGS.md').read_text().split('---\n\n## Historical reconstruction findings')[0])
shutil.copy2(root/'Documentation/VALIDATION.md',out/'Validation.md')
assets = [source_zip, *package_archives, full_zip, out/'Audit.md',out/'Validation.md']
manifest = {'application':meta['app'],'version':version,'buildId':meta['buildId'],'minimumMacOS':meta['minimumMacOS'],'validationStatus':'candidate; real-browser/native certification pending','desktopArchitectures':[a for a,p in app_packages],'separateBrowserEdition':bool(args.browser_package),'artifacts':[{'file':p.name,'bytes':p.stat().st_size,'sha256':digest(p)} for p in assets]}
(out/'RELEASE_ASSETS.json').write_text(json.dumps(manifest,indent=2)+'\n')
assets.append(out/'RELEASE_ASSETS.json')
(out/'SHA256SUMS.txt').write_text(''.join(f'{digest(p)}  {p.name}\n' for p in assets))
print('All release archives and internal inventories verified; external SHA-256 checksums generated.')
