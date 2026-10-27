"""Opt-in X11 integration tests. Requires numpy, nibabel, Xvfb and libXtst.

Uses a private virtual display only. No real desktop control or data required.
"""
from pathlib import Path
import argparse
import ctypes as C
import ctypes.util
import json
import os
import shutil
import subprocess
import tempfile
import time
import nibabel as nib
import numpy as np


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--binary',type=Path,required=True)
    parser.add_argument('--xvfb',default=shutil.which('Xvfb') or '/opt/X11/bin/Xvfb')
    parser.add_argument('--output',type=Path,required=True)
    parser.add_argument('--case',default=None)
    args=parser.parse_args();args.binary=args.binary.resolve();out=args.output.resolve();out.mkdir(parents=True,exist_ok=True)
    lib=lambda name:ctypes.util.find_library(name) or f'/opt/X11/lib/lib{name}.dylib'
    x=C.CDLL(lib('X11'));t=C.CDLL(lib('Xtst'))
    x.XOpenDisplay.argtypes=[C.c_char_p];x.XOpenDisplay.restype=C.c_void_p
    x.XDefaultRootWindow.argtypes=[C.c_void_p];x.XDefaultRootWindow.restype=C.c_ulong
    x.XQueryTree.argtypes=[C.c_void_p,C.c_ulong,C.POINTER(C.c_ulong),C.POINTER(C.c_ulong),C.POINTER(C.POINTER(C.c_ulong)),C.POINTER(C.c_uint)]
    x.XFetchName.argtypes=[C.c_void_p,C.c_ulong,C.POINTER(C.c_void_p)]
    x.XFree.argtypes=[C.c_void_p];x.XFlush.argtypes=[C.c_void_p]
    x.XSetInputFocus.argtypes=[C.c_void_p,C.c_ulong,C.c_int,C.c_ulong]
    x.XStringToKeysym.argtypes=[C.c_char_p];x.XStringToKeysym.restype=C.c_ulong
    x.XKeysymToKeycode.argtypes=[C.c_void_p,C.c_ulong];x.XKeysymToKeycode.restype=C.c_uint
    x.XCloseDisplay.argtypes=[C.c_void_p]
    t.XTestFakeKeyEvent.argtypes=[C.c_void_p,C.c_uint,C.c_int,C.c_ulong]
    t.XTestFakeMotionEvent.argtypes=[C.c_void_p,C.c_int,C.c_int,C.c_int,C.c_ulong]
    t.XTestFakeButtonEvent.argtypes=[C.c_void_p,C.c_uint,C.c_int,C.c_ulong]
    x.XGetGeometry.argtypes=[C.c_void_p,C.c_ulong,C.POINTER(C.c_ulong),C.POINTER(C.c_int),C.POINTER(C.c_int),C.POINTER(C.c_uint),C.POINTER(C.c_uint),C.POINTER(C.c_uint),C.POINTER(C.c_uint)]
    number=next(n for n in range(201,250) if not Path(f'/tmp/.X11-unix/X{n}').exists() and not Path(f'/tmp/.X{n}-lock').exists())
    display_name=f':{number}';display=None;app=None;results={}
    with (out/'xvfb.log').open('w') as serverlog:
        server=subprocess.Popen([args.xvfb,display_name,'-screen','0','1024x768x24','-nolisten','tcp'],stdout=serverlog,stderr=serverlog)
        try:
            for _ in range(50):
                assert server.poll() is None,'Xvfb exited'
                display=x.XOpenDisplay(display_name.encode())
                if display:break
                time.sleep(.1)
            assert display,'Cannot open virtual display'
            def windows():
                root=C.c_ulong();parent=C.c_ulong();children=C.POINTER(C.c_ulong)();count=C.c_uint()
                x.XQueryTree(display,x.XDefaultRootWindow(display),C.byref(root),C.byref(parent),C.byref(children),C.byref(count))
                found=[]
                for i in range(count.value):
                    name=C.c_void_p()
                    if x.XFetchName(display,children[i],C.byref(name)) and name.value:
                        if 'align' in C.string_at(name).decode(errors='replace').lower():found.append(int(children[i]))
                        x.XFree(name)
                if children:x.XFree(children)
                return found
            def key(window,name):
                x.XSetInputFocus(display,window,2,0)
                code=x.XKeysymToKeycode(display,x.XStringToKeysym(name.encode()));assert code
                t.XTestFakeKeyEvent(display,code,1,0);t.XTestFakeKeyEvent(display,code,0,0);x.XFlush(display)
            for case in ['valid_restore','source_geometry_mismatch','truncated_field','missing_alignment','nonlinear_optimization']:
                if args.case and case!=args.case:continue
                folder=out/case;folder.mkdir(exist_ok=True)
                shape=(17,19,21)
                coords=np.indices(shape).astype(np.float32)
                values=(1+100*np.exp(-sum(((coords[i]-(shape[i]-1)/2)/(3+i))**2 for i in range(3))))
                affine=np.diag([-1.,-1.,1.,1.]);affine[:3,3]=[8.1234567,9.7654321,-10.1234567]
                header=nib.Nifti1Header();header.set_data_dtype(np.float32);header.set_xyzt_units('mm')
                im=nib.Nifti1Image(values.astype(np.float32),affine,header)
                im.set_qform(affine,1);im.set_sform(affine,1)
                nib.save(im,folder/'image.nii')
                field=np.zeros(shape+(3,),dtype=np.float32)
                field[...,2]=.1*np.sin(coords[0]/4)*np.cos(coords[1]/5)
                nib.save(nib.Nifti1Image(field,im.affine,header),folder/'field.nii')
                if case=='truncated_field':
                    p=folder/'field.nii';p.write_bytes(p.read_bytes()[:-12])
                target=im.affine.copy();source=target.copy()
                if case=='source_geometry_mismatch':source[0,3]+=4
                lines=['set target-voxel-affine '+' '.join(format(float(v),'.9g') for v in target.ravel()),
                       'set source-voxel-affine '+' '.join(format(float(v),'.9g') for v in source.ravel()),
                       'set registration-translations 0.123456791 -0.234567896 0.345678896',
                       'set registration-rotations 0 0 0','set registration-inflations 1 1 1',
                       'set registration-skews 0 0 0','set registration-distortions '+str(folder/'field.nii')]
                saved='\n'.join(lines)+'\n'
                if case!='missing_alignment':(folder/'align.com').write_text(saved)
                with (folder/'gui.log').open('w') as log:
                    app=subprocess.Popen([str(args.binary),str(folder/'image.nii'),'-t',str(folder/'image.nii'),'-3','-I','-r','0'],cwd=folder,env={**os.environ,'DISPLAY':display_name},stdout=log,stderr=log)
                    if case in ['source_geometry_mismatch','truncated_field','missing_alignment']:
                        assert app.wait(timeout=15)==1
                        assert not (folder/'align.mat').exists()
                        assert not (folder/'distortions.nii').exists()
                        if case!='missing_alignment':assert (folder/'align.com').read_text()==saved
                        else:assert not (folder/'align.com').exists()
                        results[case]='PASS: rejected before GUI/save; existing align.com unchanged'
                        continue
                    window=None
                    for _ in range(150):
                        assert app.poll() is None,'GUI exited before opening'
                        found=windows()
                        if found:window=found[0];break
                        time.sleep(.1)
                    assert window
                    time.sleep(.2)
                    if case=='nonlinear_optimization':
                        root=C.c_ulong();xx=C.c_int();yy=C.c_int();w=C.c_uint();h=C.c_uint();border=C.c_uint();depth=C.c_uint()
                        x.XGetGeometry(display,window,C.byref(root),C.byref(xx),C.byref(yy),C.byref(w),C.byref(h),C.byref(border),C.byref(depth))
                        (folder/'window-geometry.json').write_text(json.dumps(dict(x=xx.value,y=yy.value,width=w.value,height=h.value)))
                        dumper=Path(args.xvfb).with_name('xwd')
                        def dump(label):
                            if dumper.exists():subprocess.run([str(dumper),'-display',display_name,'-id',str(window),'-silent','-out',str(folder/(label+'.xwd'))],check=True)
                        dump('before-switch')
                        # Right-click inside the parameter panel switches Setup -> Nonlinear.
                        t.XTestFakeMotionEvent(display,-1,xx.value+border.value+w.value-40,yy.value+border.value+65,0)
                        t.XTestFakeButtonEvent(display,3,1,0);t.XTestFakeButtonEvent(display,3,0,0);x.XFlush(display);time.sleep(.2)
                        dump('after-switch')
                        key(window,'a')
                        for _ in range(300):
                            assert app.poll() is None
                            if '\ndone\n' in (folder/'gui.log').read_text():break
                            time.sleep(.1)
                        else:raise AssertionError('GUI nonlinear optimization did not finish')
                    key(window,'q');time.sleep(.2);key(window,'y');assert app.wait(timeout=15)==0
                text=(folder/'gui.log').read_text()
                assert 'Alignment parameters were recalled' in text and 'Cannot restore' not in text
                output=nib.load(folder/'distortions.nii');actual=np.asarray(output.dataobj)
                assert np.array_equal(output.affine,im.affine) and np.isfinite(actual).all()
                if case=='valid_restore':
                    assert np.array_equal(actual,field),'Merely opening/saving changed dense map'
                    saved_line=next(line for line in (folder/'align.com').read_text().splitlines() if line.startswith('set registration-translations'))
                    assert np.array_equal(np.array(saved_line.split()[2:],dtype=np.float32),np.array([.123456791,-.234567896,.345678896],dtype=np.float32))
                    results[case]='PASS: exact imported dense map, native geometry and float32 translations preserved'
                else:
                    assert 'Relative full-map Jacobian' in text
                    assert not np.array_equal(actual,field),'No nonlinear fit change observed'
                    results[case]='PASS: nonlinear fitting completed through actual GUI events; finite saved field'
                app=None
            (out/'results.json').write_text(json.dumps(results,indent=2)+'\n')
            print(json.dumps(results,indent=2))
        finally:
            if app and app.poll() is None:app.terminate();app.wait(timeout=5)
            if display:x.XCloseDisplay(display)
            server.terminate();server.wait(timeout=5)


if __name__=='__main__':main()
