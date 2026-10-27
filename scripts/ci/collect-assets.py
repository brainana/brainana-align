from pathlib import Path
import hashlib,json,shutil,subprocess,re
root=Path.cwd();version=json.loads((root/'VERSION.json').read_text())['version'];out=root/'artifacts/installers';out.mkdir(parents=True,exist_ok=True)
assets=[]
for p in (root/'apps/align/release').iterdir():
 if p.is_file() and re.match(r'^brainana-align(?:-setup)?-'+re.escape(version)+r'-',p.name) and p.name.endswith(('.dmg','.exe','.AppImage','.deb','.zip')):
  shutil.copy2(p,out/p.name);assets.append({'file':p.name,'bytes':p.stat().st_size,'sha256':hashlib.sha256(p.read_bytes()).hexdigest()})
if not assets:raise SystemExit('No installers were produced')
import platform
arch={'AMD64':'x64','x86_64':'x64','aarch64':'arm64'}.get(platform.machine(),platform.machine());system=platform.system().lower()
commit=subprocess.check_output(['git','rev-parse','HEAD']).decode().strip()
(out/('assets-'+system+'-'+arch+'.json')).write_text(json.dumps({'version':version,'sourceCommit':commit,'platform':system,'architecture':arch,'assets':assets},indent=2)+'\n')
