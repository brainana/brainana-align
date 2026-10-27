#!/usr/bin/env python3
"""Package a verified Windows desktop app, installer and complete shared source."""
import argparse,json,shutil,tempfile
from pathlib import Path
from release_archives import digest,inventory,archive,link_or_copy
parser=argparse.ArgumentParser()
parser.add_argument('--app',type=Path,required=True)
parser.add_argument('--installer',type=Path,required=True)
parser.add_argument('--evidence',type=Path,required=True)
parser.add_argument('--out',type=Path,required=True)
args=parser.parse_args()
root=Path(__file__).resolve().parent.parent
meta=json.loads((root/'VERSION.json').read_text());version=meta['version']
for name in ['packaging-windows.json','installer-integrity.json']:
    report=json.loads((args.evidence/name).read_text())
    if report['status']!='passed':raise ValueError('Missing successful verification: '+name)
out=args.out.resolve();out.mkdir(parents=True,exist_ok=True)
installer=out/args.installer.name;shutil.copy2(args.installer,installer)
with tempfile.TemporaryDirectory(prefix='brainana-windows-release-') as temporary:
    temporary=Path(temporary)
    source=temporary/'source'
    shutil.copytree(root,source,ignore=shutil.ignore_patterns('node_modules','.git','.DS_Store','__pycache__','release','browser-support','workstation-support','node-runtimes','*.app'))
    shutil.copytree(args.evidence,source/'Validation',dirs_exist_ok=True)
    inventory(source)
    source_zip=out/f'Brainana-Align-v{version}-source.zip';archive(source,source_zip)
    app=temporary/'portable';shutil.copytree(args.app,app,copy_function=link_or_copy)
    shutil.copytree(root/'Documentation',app/'Documentation')
    shutil.copytree(args.evidence,app/'Validation')
    for name in ['VERSION.json','AGENTS.md','LICENSE','NOTICE.txt']:shutil.copy2(root/name,app/name)
    (app/'README-FIRST.md').write_text(f'''# Brainana Align Windows v{version}

Extract the entire ZIP into a folder, then open **Brainana Align.exe**. Keep all files together. This x64 desktop app bundles Electron and needs no separate browser, Node, or Python. Alternatively, run the separately supplied Setup EXE to install the app and create shortcuts.

All four linear landmark models and the advanced nonlinear spline are included, along with automatic registration/refinement, manual adjustment, sessions, exports, guided setup and settings. Local files, mounted network shares and the in-app SSH/SFTP connection/file browser are supported. The older macOS interactive launcher is not included on Windows.

The app and installer are unsigned. Native Windows launch, GPU behavior, dialogs, install/uninstall and real-data testing remain pending; see Documentation/VALIDATION.md. See Documentation/README.md for usage and SHA256SUMS.txt for file checksums.
''')
    inventory(app)
    portable=out/f'Brainana-Align-Windows-v{version}-x64-portable.zip';archive(app,portable)
    full=temporary/'full';full.mkdir()
    shutil.copytree(app,full/'Windows-x64',copy_function=link_or_copy)
    (full/'Installer').mkdir();shutil.copy2(installer,full/'Installer'/installer.name)
    (full/'Source').mkdir();shutil.copy2(source_zip,full/'Source'/source_zip.name)
    shutil.copy2(app/'README-FIRST.md',full/'README-FIRST.md')
    shutil.copytree(root/'Documentation',full/'Documentation')
    inventory(full)
    bundle=out/f'Brainana-Align-Windows-v{version}-full-release.zip';archive(full,bundle)
shutil.copy2(root/'Documentation/VALIDATION.md',out/'Validation.md')
shutil.copy2(root/'Documentation/TECHNICAL_FINDINGS.md',out/'Audit.md')
assets=[installer,portable,source_zip,bundle,out/'Validation.md',out/'Audit.md']
manifest={'application':meta['app'],'version':version,'buildId':meta['buildId'],'platform':'win32','architecture':'x64','validationStatus':'cross-built candidate; native Windows launch and install/uninstall pending','authenticode':'unsigned','artifacts':[{'file':p.name,'bytes':p.stat().st_size,'sha256':digest(p)} for p in assets]}
(out/'RELEASE_ASSETS.json').write_text(json.dumps(manifest,indent=2)+'\n');assets.append(out/'RELEASE_ASSETS.json')
(out/'SHA256SUMS.txt').write_text(''.join(f'{digest(p)}  {p.name}\n' for p in assets))
print('Windows installer, portable app, source and full bundle verified; manifests and SHA-256 checksums generated.')
