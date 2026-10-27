"""Small native ITK ABI. Only numerical arrays cross this private boundary."""
import ctypes as C
import json
import os
from pathlib import Path
import sys
import numpy as np
from functools import lru_cache

CALLBACK=C.CFUNCTYPE(C.c_int,C.c_char_p,C.c_void_p)
P=C.c_void_p
@lru_cache(maxsize=1)
def library():
    # A frozen Windows worker is a one-job process. ITK's persistent pool can
    # deadlock during DLL teardown after a fully written result. Platform
    # threads join within each operation and retain the requested thread count.
    if sys.platform=='win32':os.environ['ITK_GLOBAL_DEFAULT_THREADER']='Platform'
    root=Path(getattr(sys,'_MEIPASS',Path(__file__).parents[1]))
    suffix='.dll' if sys.platform=='win32' else '.dylib' if sys.platform=='darwin' else '.so'
    path=root/'native'/('align-itk.dll' if sys.platform=='win32' else 'libalign-itk'+suffix)
    lib=C.CDLL(str(path));lib.align_itk_version.restype=C.c_char_p
    lib.align_itk_bspline.argtypes=[P]*12+[C.c_size_t,CALLBACK,P];lib.align_itk_bspline.restype=C.c_int
    lib.align_itk_n4.argtypes=[P]*9+[C.c_size_t,CALLBACK,P];lib.align_itk_n4.restype=C.c_int
    return lib

def version():return library().align_itk_version().decode()
def image_arrays(image):
    if image.get_data_dtype().kind not in 'biuf':raise ValueError('ITK requires a real scalar image.')
    a=np.asarray(image.affine,dtype=np.float64,order='C');d=np.array(image.shape,dtype=np.int32);v=np.asfortranarray(image.get_fdata(dtype=np.float32))
    if len(image.shape)!=3 or min(image.shape)<8 or v.size>2_000_000:raise ValueError('Use 3D working grids with 8+ voxels per axis and at most 2 million voxels.')
    if image.header.get_xyzt_units()[0]!='mm':raise ValueError('Input spatial units must be millimetres.')
    if not np.isfinite(a).all() or not np.allclose(a[3],[0,0,0,1],rtol=0,atol=1e-8) or not np.isfinite(v).all():raise ValueError('Input geometry and scalar intensities must be finite.')
    scale=np.linalg.norm(a[:3,:3],axis=0)
    if np.any(scale<1e-5) or not np.allclose((a[:3,:3]/scale).T@(a[:3,:3]/scale),np.eye(3),atol=1e-5,rtol=0):raise ValueError('ITK cannot register a sheared voxel grid; prepare an explicitly orthogonal working grid first.')
    return v,d,a

def pointer(a):return a.ctypes.data_as(P) if a is not None else None

def _call(function,args,progress,cancelled):
    output=C.create_string_buffer(131072);errors=[]
    def notify(raw,context):
        try:
            if cancelled():return 1
            e=json.loads(raw)
            if e.get('message'):progress(e)
            return 0
        except BaseException as error:errors.append(error);return 1
    callback=CALLBACK(notify)
    if cancelled():raise InterruptedError('Cancelled')
    code=function(*args,output,len(output),callback,None)
    if errors:raise errors[0]
    if cancelled():raise InterruptedError('Cancelled')
    result=json.loads(output.value)
    if code:raise ValueError(result.get('error','Native ITK failed'))
    return result

def bspline(fixed,moving,selections,pull,settings,progress,cancelled):
    fv,fd,fa=image_arrays(fixed);mv,md,ma=image_arrays(moving)
    fm=np.asfortranarray(selections['fixed'],dtype=np.uint8) if selections.get('fixed') is not None else None
    mm=np.asfortranarray(selections['moving'],dtype=np.uint8) if selections.get('moving') is not None else None
    pull=np.asarray(pull,dtype=np.float64,order='C');residual=np.empty(tuple(fd)+(3,),dtype=np.float32,order='F')
    result=_call(library().align_itk_bspline,[pointer(x) for x in [fv,fd,fa,mv,md,ma,fm,mm,pull]]+[json.dumps(settings).encode(),pointer(residual)],progress,cancelled)
    if not np.isfinite(residual).all():raise ValueError('Native ITK returned a nonfinite field')
    return residual,result

def n4(image,settings,mask,progress,cancelled):
    values,dims,affine=image_arrays(image);selected=np.empty(tuple(dims),dtype=np.uint8,order='F');corrected=np.empty_like(values);bias=np.empty_like(values)
    mask=np.asfortranarray(mask,dtype=np.uint8) if mask is not None else None
    result=_call(library().align_itk_n4,[pointer(x) for x in [values,dims,affine,mask]]+[json.dumps(settings).encode()]+[pointer(x) for x in [corrected,bias,selected]],progress,cancelled)
    return corrected,bias,selected,result
