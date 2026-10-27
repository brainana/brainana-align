"""Freeze only Align's numerical worker. No standalone JIP/Qt application."""
from pathlib import Path
import platform,sys
from importlib.metadata import distribution
root=Path(SPECPATH)
sys.path.insert(0,str(root))
from brainana_registration.engine import library_path
notices=[]
for package in ['numpy','nibabel','Python','pillow','packaging','psutil']:
    try:
        metadata=Path(distribution(package)._path)
        for p in metadata.rglob('*'):
            if p.is_file() and (p.name.lower().startswith(('license','copying','notice')) or 'licenses' in p.parts):
                notices.append((str(p),str(Path('licenses')/package/p.parent.relative_to(metadata))))
    except Exception:pass
windows_runtime=[(str(p),'native') for p in (root/'native').glob('*.dll') if p.name.lower() not in ['jip-engine.dll','libjip-engine.dll','jip-engine-repro.dll','libjip-engine-repro.dll','align-itk.dll']] if sys.platform=='win32' else []
analysis=Analysis([str(root/'launch.py')],pathex=[str(root)],
    binaries=[(str(library_path(mode)),'native') for mode in ['compatibility','reproducible']]+[(str(root/'native'/('align-itk.dll' if sys.platform=='win32' else 'libalign-itk.dylib' if sys.platform=='darwin' else 'libalign-itk.so')),'native')]+windows_runtime,
    datas=[(str(root/'native-itk/licenses'),'licenses/ITK'),(str(root/'native-itk/vendor/NLOHMANN-LICENSE.MIT'),'licenses'),(str(root/'native/licenses'),'licenses/Runtime'),
           (str(root/'native/source/engine/JIP-LICENSE.txt'),'licenses'),
           (str(root/'native/source/fftw-3.2.2/COPYING'),'licenses/FFTW'),
           (str(root/'preprocessing'),'preprocessing')]+notices,
    hiddenimports=['nibabel'],
    excludes=['SimpleITK','itk','scipy','sphinx','babel','docutils','jinja2','lxml','pytz','PyQt6','PyQt5','PySide6','PySide2','tkinter','wx',
              'jip_workbench','matplotlib','pandas','sympy','torch','tensorflow','IPython',
              'jupyter','pytest','numba','llvmlite','h5py','tables','sklearn','skimage',
              'dask','cupy','vtk','nibabel.tests','scipy.tests','numpy.tests'],noarchive=False)
pyz=PYZ(analysis.pure)
exe=EXE(pyz,analysis.scripts,[],exclude_binaries=True,name='BrainanaAlignRegistration',
    debug=False,bootloader_ignore_signals=False,strip=sys.platform.startswith('linux'),upx=False,console=True,
    target_arch=platform.machine() if sys.platform=='darwin' else None,
    codesign_identity=None,entitlements_file=None)
collection=COLLECT(exe,analysis.binaries,analysis.datas,strip=sys.platform.startswith('linux'),upx=False,name='scientific-runtime')
