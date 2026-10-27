#!/usr/bin/env python3
"""Verify Debian metadata, installation scripts and exact embedded app files."""
import sys,io,tarfile,json,hashlib,tempfile,subprocess
from pathlib import Path
package=Path(sys.argv[1]);app=Path(sys.argv[2]);root=Path(__file__).resolve().parent.parent
meta=json.loads((root/'VERSION.json').read_text())
b=package.read_bytes();assert b[:8]==b'!<arch>\n'
offset=8;archives={}
while offset<len(b):
    header=b[offset:offset+60];assert len(header)==60 and header[58:60]==b'`\n'
    name=header[:16].decode().strip().rstrip('/');size=int(header[48:58]);archives[name]=b[offset+60:offset+60+size];offset+=60+size+(size%2)
assert archives['debian-binary']==b'2.0\n'
control_data=next(value for name,value in archives.items() if name.startswith('control.tar'))
with tarfile.open(fileobj=io.BytesIO(control_data),mode='r:*') as control:
    texts={m.name.removeprefix('./'):control.extractfile(m).read().decode() for m in control if m.isfile()}
fields={line.split(':',1)[0]:line.split(':',1)[1].strip() for line in texts['control'].splitlines() if ':' in line and not line.startswith(' ')}
assert fields['Package']=='brainana-align';assert fields['Architecture']=='amd64'
assert fields['Version']==meta['version'].replace('-','~')
assert fields['Section']=='science'
for dependency in ['libasound2t64 | libasound2','libgtk-3-0t64 | libgtk-3-0','libnss3','libgbm1','libudev1']:assert dependency in fields['Depends']
for name in ['postinst','postrm']:
    assert '--no-sandbox' not in texts[name]
    with tempfile.NamedTemporaryFile(mode='w',suffix='.sh') as file:
        file.write(texts[name]);file.flush();subprocess.run(['/bin/bash','-n',file.name],check=True)
assert "chmod 4755 '/opt/Brainana Align/chrome-sandbox'" in texts['postinst']
assert 'apparmor_parser' in texts['postinst'] and 'apparmor_parser --remove' in texts['postrm']
assert "--remove 'brainana-align' '/opt/Brainana Align/brainana-align'" in texts['postrm']
data=next(value for name,value in archives.items() if name.startswith('data.tar'))
app_prefix='opt/Brainana Align/';embedded={};desktop='';profile=''
with tarfile.open(fileobj=io.BytesIO(data),mode='r:*') as tar:
    for member in tar:
        name=member.name.removeprefix('./')
        if name.startswith(app_prefix) and (member.isfile() or member.issym()):
            relative=name[len(app_prefix):];expected=app/relative;assert expected.exists(),relative
            assert member.uid==0 and member.gid==0,relative
            if member.issym():assert expected.is_symlink() and expected.readlink().as_posix()==member.linkname
            else:
                value=tar.extractfile(member).read();assert hashlib.sha256(value).digest()==hashlib.sha256(expected.read_bytes()).digest(),relative
                if relative=='brainana-align':assert member.mode&0o111
            embedded[relative]=True
        if name=='usr/share/applications/brainana-align.desktop':desktop=tar.extractfile(member).read().decode()
        if name==app_prefix+'resources/apparmor-profile':profile=tar.extractfile(member).read().decode()
expected={p.relative_to(app).as_posix() for p in app.rglob('*') if p.is_file() or p.is_symlink()};assert set(embedded)==expected
assert 'Exec="/opt/Brainana Align/brainana-align" %U' in desktop
assert 'StartupWMClass=brainana-align' in desktop and 'Categories=Science;' in desktop
assert '--no-sandbox' not in desktop
assert 'userns,' in profile and '"/opt/Brainana Align/brainana-align"' in profile
print(json.dumps({'status':'passed','format':'Debian binary 2.0','architecture':'amd64','version':fields['Version'],'packageBytes':package.stat().st_size,'embeddedFileCount':len(embedded),'embeddedApp':'all file SHA256s/symlink targets match verified app; root ownership and executable mode checked','dependencies':fields['Depends'],'desktopEntry':'matching executable and window identity; no sandbox bypass','installRemoveScripts':'Bash syntax, alternatives removal and per-app AppArmor/sandbox paths checked; not executed','nativeUbuntuInstallRemove':'pending'},indent=2))
