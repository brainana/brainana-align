#!/usr/bin/env python3
"""Build the committed numerical source for this runner. Never downloads an app.
Windows uses the explicit MSYS2 MINGW64 toolchain (not MSVC). Numerical code,
FFTW configuration and native API remain unchanged; only build orchestration is new.
"""
import argparse, hashlib, json, os, platform, shutil, subprocess, sys, tarfile
from pathlib import Path
ROOT = Path(__file__).resolve().parents[2]
SCI = ROOT / 'scientific'
P = argparse.ArgumentParser()
P.add_argument('--plan', action='store_true')
P.add_argument('--jobs', type=int, default=4)
P.add_argument('--skip-freeze', action='store_true', help='Developer native-library checks only; never for packaging')
A = P.parse_args()
SYSTEM = platform.system()
ARCH = {'AMD64':'x86_64','aarch64':'arm64'}.get(platform.machine(), platform.machine())
if SYSTEM not in ['Darwin','Linux','Windows'] or ARCH not in ['arm64','x86_64']:
    raise SystemExit('Unsupported native build target: '+SYSTEM+'/'+ARCH)
if not 1 <= A.jobs <= 16: raise SystemExit('jobs must be between1 and16')
BUILD = (Path(os.environ['RUNNER_TEMP'])/'align-science') if SYSTEM=='Windows' and os.environ.get('RUNNER_TEMP') else ROOT/'.ci-build'/('native-'+SYSTEM.lower()+'-'+ARCH)
ENV = {k:v for k,v in os.environ.items() if k not in ['JIP_ENGINE_LIBRARY','PYTHONPATH','PYTHONHOME']}
ENV['PATH']=str(Path(sys.executable).parent)+os.pathsep+str(Path(sys.executable).parent/'Scripts')+os.pathsep+ENV.get('PATH','')
ENV['PYINSTALLER_CONFIG_DIR']=str(BUILD/'pyinstaller-cache')
JOBS = str(A.jobs)
MAC = ['-DCMAKE_OSX_DEPLOYMENT_TARGET=14.0', '-DCMAKE_OSX_ARCHITECTURES='+ARCH] if SYSTEM=='Darwin' else []
if SYSTEM=='Darwin': ENV['MACOSX_DEPLOYMENT_TARGET']='14.0'
CC = 'gcc' if SYSTEM=='Windows' else 'cc'
CXX = 'g++' if SYSTEM=='Windows' else 'c++'
COMMON = ['-G','Ninja','-DCMAKE_BUILD_TYPE=Release','-DCMAKE_POSITION_INDEPENDENT_CODE=ON', '-DCMAKE_C_COMPILER='+CC, '-DCMAKE_CXX_COMPILER='+CXX, *MAC]
plan={'platform':SYSTEM,'architecture':ARCH,'python':sys.version.split()[0], 'source':'scientific/native/source', 'fftw':'vendored3.2.2 static', 'itk':'pinned5.4.6 static', 'build':str(BUILD), 'frozen':not A.skip_freeze, 'windowsToolchain':'MSYS2 MINGW64 GCC; matching system CPython', 'numericalSourceChanged':False}
if A.plan:
    print(json.dumps(plan,indent=2)); raise SystemExit(0)

def run(args, cwd=ROOT, env=None):
    print('+',subprocess.list2cmdline([str(v) for v in args]),flush=True)
    subprocess.run([str(v) for v in args],cwd=cwd,env=env or ENV,check=True)

def configure(source, target, flags):
    target.mkdir(parents=True,exist_ok=True)
    cache=target/'CMakeCache.txt'
    # GitHub caches may have belonged to a checkout with another absolute path.
    if cache.exists() and 'CMAKE_HOME_DIRECTORY:INTERNAL='+str(source).replace('\\','/') not in cache.read_text(errors='replace'):
        shutil.rmtree(target); target.mkdir()
    run(['cmake','-S',source,'-B',target,*COMMON,*flags])

def build(target, *targets):
    run(['cmake','--build',target,'--parallel',JOBS,*(['--target',*targets] if targets else [])])

def find(folder, patterns):
    for pattern in patterns:
        found=sorted(folder.rglob(pattern))
        if found:return found[0]
    raise RuntimeError('Missing native output: '+str(folder)+' '+repr(patterns))

def msys(path):
    path=str(Path(path).resolve()).replace('\\','/')
    return '/'+path[0].lower()+path[2:] if len(path)>2 and path[1]==':' else path

BUILD.mkdir(parents=True,exist_ok=True)
if SYSTEM=='Windows':
    prefix=Path(os.environ['ALIGN_MINGW_ROOT'])
    ENV['PATH']=str(prefix/'bin')+os.pathsep+ENV['PATH']
    ENV['MSYSTEM']='MINGW64';ENV['CHERE_INVOKING']='1'
    bash=Path(os.environ['ALIGN_MSYS_ROOT'])/'usr/bin/bash.exe'
else: bash=Path('/bin/sh')
# Keep the original bundled FFTW release and static/PIC switches on every target.
fftw=BUILD/'fftw'; install=BUILD/'fftw-prefix';fftw.mkdir(exist_ok=True)
if not (install/'lib/libfftw3.a').exists():
    source=SCI/'native/source/fftw-3.2.2/configure'
    if SYSTEM=='Windows':
        import shlex
        cmd='export PATH=/mingw64/bin:/usr/bin:$PATH; export CFLAGS=-O2 CC=gcc; '+shlex.quote(msys(source))+' --build=x86_64-w64-mingw32 --host=x86_64-w64-mingw32 --prefix='+shlex.quote(msys(install))+' --disable-fortran --disable-dependency-tracking --enable-static --disable-shared --with-pic && make -j'+JOBS+' MAKEINFO=true && make install MAKEINFO=true'
        run([bash,'-lc',cmd],cwd=fftw)
    else:
        ffenv=dict(ENV);ffenv['CFLAGS']='-O2'+(' -arch '+ARCH+' -mmacosx-version-min=14.0' if SYSTEM=='Darwin' else '')
        triplet='arm-apple-darwin' if ARCH=='arm64' else 'x86_64-apple-darwin'
        run([bash,source,*(['--build='+triplet,'--host='+triplet] if SYSTEM=='Darwin' else []),'--prefix='+str(install),'--disable-fortran','--disable-dependency-tracking','--enable-static','--disable-shared','--with-pic'],cwd=fftw,env=ffenv)
        # FFTW 3.2.2's legacy Texinfo rules cannot regenerate docs on modern hosts.
        # Build/install the unchanged library using bundled documentation instead.
        run(['make','-j'+JOBS,'MAKEINFO=true'],cwd=fftw);run(['make','install','MAKEINFO=true'],cwd=fftw)
# Vendored zlib avoids depending on a developer's Homebrew/Windows installation.
zsrc=BUILD/'zlib-source';zbuild=BUILD/'zlib'
if not zsrc.exists():shutil.copytree(SCI/'native/source/zlib-1.3.1',zsrc)
configure(zsrc,zbuild,['-DZLIB_BUILD_EXAMPLES=OFF']);build(zbuild,'zlibstatic')
zlib=find(zbuild,['libz.a','libzlibstatic.a','zlibstatic.lib'])
zinclude=BUILD/'zlib-include';zinclude.mkdir(exist_ok=True)
shutil.copy2(zsrc/'zlib.h',zinclude/'zlib.h');shutil.copy2(zbuild/'zconf.h',zinclude/'zconf.h')
engine=BUILD/'engine'
configure(SCI/'native/source',engine,['-DCMAKE_BUILD_TYPE=RelWithDebInfo','-DJIP_BUILD_X11=OFF','-DBUILD_TESTING=ON', '-DFFTW_INCLUDE_DIR='+str(install/'include'),'-DFFTW_LIBRARY='+str(install/'lib/libfftw3.a'),'-DZLIB_INCLUDE_DIR='+str(zinclude),'-DZLIB_LIBRARY='+str(zlib),'-DPython3_EXECUTABLE='+sys.executable])
targets=['jip-engine-shared','jip-engine-repro','jip-register','jip-register-repro','jip-api-test','jip-api-test-repro','jip-interface-test','jip-context-test','jip-sampling-test','jip-resampler-test','jip-mixed-link-test','jip-accel-math-test','jip-accel-backend-test','jip-accel-transaction-test']
if SYSTEM!='Windows':targets.append('jip-reliability-test')
build(engine,*targets)
checks=['engine-acceleration-arithmetic','engine-acceleration-backend','engine-acceleration-transaction','engine-private-linkage','engine-contexts','engine-registration-interface','engine-sampling','engine-threaded-resampler','engine-resampler-equivalence','engine-checkpoint-compatibility']
if SYSTEM!='Windows':checks.append('engine-reliability')
if SYSTEM=='Darwin' and ARCH=='arm64':checks.append('engine-reference-workflow')
else:ENV['JIP_PORTABLE_REFERENCE']='1'
try:
    run(['ctest','--test-dir',engine,'--output-on-failure','--no-tests=error','-R','^('+'|'.join(checks)+')$'])
finally:
    # Preserve synthetic failure evidence; never substitute candidate output for goldens.
    evidence=ROOT/'artifacts/validation/native';evidence.mkdir(parents=True,exist_ok=True)
    for folder in [engine/'engine/interface-test',SCI/'native/validation'/('compatibility-'+engine.name),engine/'Testing/Temporary']:
        if folder.exists():
            destination=evidence/folder.name
            shutil.copytree(folder,destination,dirs_exist_ok=True)

# Recover the checked upstream ITK source, never fetch unpinned source at build time.
pin=json.loads((SCI/'native-itk/UPSTREAM.json').read_text());archive=SCI/'native-itk/vendor'/pin['archive']
if hashlib.sha256(archive.read_bytes()).hexdigest()!=pin['sha256']:raise RuntimeError('ITK source checksum mismatch')
extract=BUILD/'itk-source';extract.mkdir(exist_ok=True)
# ITK explicitly rejects Windows source roots longer than 50 characters.
itksrc=BUILD/'s' if SYSTEM=='Windows' else extract/'InsightToolkit-5.4.6'
if not itksrc.exists():
    with tarfile.open(archive) as stream:stream.extractall(extract,filter='data')
    if SYSTEM=='Windows':(extract/'InsightToolkit-5.4.6').rename(itksrc)
itk=BUILD/'itk';configure(itksrc,itk,['-DBUILD_SHARED_LIBS=OFF','-DBUILD_TESTING=OFF','-DITK_BUILD_DEFAULT_MODULES=OFF',*['-DITKGroup_'+g+'=OFF' for g in ['Core','Filtering','IO','Numerics','Registration']], '-DModule_ITKRegistrationMethodsv4=ON','-DModule_ITKBiasCorrection=ON','-DModule_ITKThresholding=ON','-DITK_USE_SYSTEM_EXPAT=OFF','-DITK_USE_SYSTEM_ZLIB=OFF','-DITK_USE_SYSTEM_PNG=OFF'])
build(itk)
adapter=BUILD/'itk-adapter';configure(SCI/'native-itk',adapter,['-DITK_DIR='+str(itk)]);build(adapter)
(SCI/'native').mkdir(exist_ok=True)
for stem in ['jip-engine','jip-engine-repro']:
    names=[stem+'.dll','lib'+stem+'.dll'] if SYSTEM=='Windows' else ['lib'+stem+('.dylib' if SYSTEM=='Darwin' else '.so')]
    shutil.copy2(find(engine,names),SCI/'native'/names[0])
itkname='align-itk.dll' if SYSTEM=='Windows' else 'libalign-itk'+('.dylib' if SYSTEM=='Darwin' else '.so')
shutil.copy2(find(adapter,[itkname,'libalign-itk.dll']),SCI/'native'/itkname)
if SYSTEM=='Windows':
    # CPython's safe DLL lookup does not use PATH. Stage the actual compiler
    # imports beside our libraries and explicitly freeze them in the same folder.
    import re
    queue=list((SCI/'native').glob('*.dll'));seen=set()
    while queue:
        library=queue.pop()
        if library.name.lower() in seen:continue
        seen.add(library.name.lower())
        imports=subprocess.check_output(['objdump','-p',str(library)],env=ENV,text=True)
        for name in re.findall(r'DLL Name:\s*([^\s]+)',imports):
            if not re.fullmatch(r'[A-Za-z0-9_.+\-]+\.dll',name,re.I):raise RuntimeError('Invalid DLL import name')
            dependency=prefix/'bin'/name
            if dependency.is_file():
                target=SCI/'native'/name
                shutil.copy2(dependency,target);queue.append(target)
    licenses=SCI/'native/licenses/WindowsToolchain'
    shutil.copytree(prefix/'share/licenses',licenses,dirs_exist_ok=True)
    (licenses/'SOURCE.txt').write_text('MinGW runtime DLLs: MSYS2 packages. Compiler/runtime source and package recipes: https://packages.msys2.org/ and https://github.com/msys2/MINGW-packages\n')
plan['compiler']=subprocess.check_output([CC,'--version'],env=ENV,text=True).splitlines()[0]
plan['cmake']=subprocess.check_output(['cmake','--version'],env=ENV,text=True).splitlines()[0]

if not A.skip_freeze:
    # Remove the output first so no stale dylib/DLL from another OS can ship.
    shutil.rmtree(ROOT/'generated/scientific-runtime',ignore_errors=True)
    run([sys.executable,'-m','PyInstaller','--clean','--noconfirm','--distpath',ROOT/'generated','--workpath',BUILD/'freeze',SCI/'Align-Registration.spec'])
    run([sys.executable,SCI/'tests/validate_runtime.py'])
    runtime=ROOT/'generated/scientific-runtime'
    files=[p for p in runtime.rglob('*') if p.is_file() and not p.is_symlink()]
    size_report={'platform':SYSTEM,'architecture':ARCH,'bytes':sum(p.stat().st_size for p in files),'limit':90_000_000,'largest':sorted([{'path':str(p.relative_to(runtime)),'bytes':p.stat().st_size} for p in files],key=lambda v:v['bytes'],reverse=True)[:15]}
    report=ROOT/'artifacts/validation';report.mkdir(parents=True,exist_ok=True)
    (report/'scientific-size.json').write_text(json.dumps(size_report,indent=2)+'\n')
    if size_report['bytes']>size_report['limit']:raise RuntimeError('Frozen scientific runtime exceeds90MB: '+str(size_report['bytes'])+'; see scientific-size.json')
plan['nativeTests']=checks;plan['limitations']=['Windows POSIX file-descriptor fault-injection test is not compiled; source/frozen registration and rollback checks still run.','Full native GUI/GPU pixel validation is separate.']
(ROOT/'generated').mkdir(exist_ok=True)
(ROOT/'generated/scientific-build.json').write_text(json.dumps(plan,indent=2)+'\n')
print('Built and checked native numerical libraries'+(' and complete frozen worker.' if not A.skip_freeze else ' (developer native-only mode).'))
