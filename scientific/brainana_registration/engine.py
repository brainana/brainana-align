"""Thin ctypes binding to the unchanged JIP C API. No registration math here."""
import ctypes as C
import os
from pathlib import Path
import sys
import time
import numpy as np
import json
import hashlib
import struct
import threading
from functools import lru_cache

P=C.c_void_p
F=C.c_float
I=C.c_int32
U=C.c_uint32


class Image(C.Structure):
    _fields_=[('dim',I*3),('spacing',F*3),('native_affine',F*16),('data',C.POINTER(F)),('count',C.c_uint64)]


class Options(C.Structure):
    _fields_=[('metric',C.c_int),('grid_down',C.c_int),('regularization',C.c_int),('jacobian_limit',F),('stiffness',F),('affine_parameters',F*12)]


class Config(C.Structure):
    _fields_=[('struct_size',U),('stages',U),('affine_model',I),('affine_metric',I),('nonlinear_axes',I*3),('matching_volume',I),('wrap_around',I*3)]


class Window(C.Structure):
    _fields_=[('minimum',F*3),('maximum',F*3),('axes',U)]

class Sampling(C.Structure):
    _fields_=[('struct_size',U),('fixed',C.POINTER(C.c_uint8)),('moving',C.POINTER(C.c_uint8)),('fixed_count',C.c_uint64),('moving_count',C.c_uint64),('moving_windows',C.POINTER(Window)),('moving_window_count',U)]

class Transform(C.Structure):
    _fields_=[('completed_stages',U),('affine_parameters',F*12),('centered_pull_affine',F*16),
              ('fixed_voxel_to_native_mm',F*16),('moving_voxel_to_native_mm',F*16),
              ('fixed_voxel_to_centered_mm',F*16),('moving_centered_mm_to_voxel',F*16),
              ('displacement',C.POINTER(F)*3),('count',C.c_uint64),('nonlinear_objective',F),('affine_objective',F)]


class Trace(C.Structure):
    _fields_=[('event',C.c_int),('grid_down',C.c_int),('basis_count',C.c_int64),('basis_index',C.c_int64),
              ('voxel_count',C.c_int64),('objective',F),('image_objective',F),('coefficients_xyz',C.POINTER(F)),('displacement',C.POINTER(F)*3)]

class Execution(C.Structure):
    _fields_=[('struct_size',U),('threads',U),('budget_mb',U),('map_arithmetic',U),('map_architecture',U),('requested',C.c_char*16),('active',C.c_char*16),
        ('device',C.c_char*160),('reason',C.c_char*320),('calls',C.c_uint64),('gpu_calls',C.c_uint64),('cpu_calls',C.c_uint64),
        ('fallbacks',C.c_uint64),('device_bytes',C.c_uint64),('gpu_seconds',C.c_double),('cpu_seconds',C.c_double)]


class Error(C.Structure):
    _fields_=[('code',C.c_int),('phase',C.c_char*64),('message',C.c_char*256),('requested_bytes',C.c_uint64),('allocation_index',C.c_uint64)]

class Profile(C.Structure):
    _fields_=[('struct_size',U)]+[(n,C.c_uint64) for n in ['transform_calls','metric_calls','basis_calls','prepared_samples','workspace_bytes','workspace_allocations']]+[(n,C.c_double) for n in ['run_seconds','transform_seconds','metric_seconds','basis_seconds','prepare_seconds','dispatch_seconds','scatter_seconds']]


CALLBACK=C.CFUNCTYPE(None,C.POINTER(Trace),P)


def library_path(arithmetic='compatibility'):
    if not getattr(sys,'frozen',False) and os.environ.get('JIP_ENGINE_LIBRARY'):return Path(os.environ['JIP_ENGINE_LIBRARY'])
    suffix='.dll' if sys.platform=='win32' else '.dylib' if sys.platform=='darwin' else '.so'
    stem='jip-engine-repro' if arithmetic=='reproducible' else 'jip-engine'
    names=[stem+'.dll','lib'+stem+'.dll'] if sys.platform=='win32' else ['lib'+stem+suffix]
    roots=[Path(getattr(sys,'_MEIPASS',Path(__file__).parents[1])),Path(__file__).parents[1]]
    for root in roots:
        for path in build_candidates(root,names):
            if path.exists():return path
    raise FileNotFoundError('The JIP registration library is missing. Reinstall the app or build jip-engine-shared.')


def build_candidates(root,names):
    return [Path(root)/directory/name for directory in ['native','build/engine','build/engine/Release','build/engine/RelWithDebInfo','build/engine/Debug',''] for name in names]


def native_runner(root):
    names=['jip-register.exe'] if sys.platform=='win32' else ['jip-register']
    return next((p for p in build_candidates(root,names) if p.is_file()),None)


def native_image(image):
    data=np.asfortranarray(image.get_fdata(dtype=np.float32))
    view=Image((I*3)(*data.shape),(F*3)(*image.header.get_zooms()[:3]),
               (F*16)(*image.affine.ravel()),data.ctypes.data_as(C.POINTER(F)),data.size)
    return view,data


DEFINITIONS={
    'jip_options_default':([C.POINTER(Options)],None),
    'jip_config_default':([C.POINTER(Config)],None),
    'jip_engine_create_configured':([C.POINTER(Image),C.POINTER(Image),C.POINTER(Options),C.POINTER(Config),C.POINTER(P)],C.c_int),
    'jip_engine_create_sampled':([C.POINTER(Image),C.POINTER(Image),C.POINTER(Options),C.POINTER(Config),C.POINTER(Sampling),C.POINTER(P)],C.c_int),
    'jip_engine_checkpoint_load':([C.c_char_p,C.POINTER(P)],C.c_int),
    'jip_engine_checkpoint_save':([P,C.c_char_p],C.c_int),
    'jip_engine_set_trace':([P,CALLBACK,P],C.c_int),
    'jip_engine_run':([P,U],C.c_int),
    'jip_engine_request_cancel':([P],C.c_int),
    'jip_engine_get_transform':([P,C.POINTER(Transform)],C.c_int),
    'jip_engine_export':([P,C.c_char_p,C.c_char_p],C.c_int),
    'jip_engine_destroy':([P],None),
    'jip_engine_status_string':([C.c_int],C.c_char_p),
    'jip_engine_set_execution':([P,C.c_char_p,U,U],C.c_int),
    'jip_engine_get_execution':([P,C.POINTER(Execution)],C.c_int),
    'jip_engine_result':([P,C.POINTER(Image),C.POINTER(F)],C.c_int),
    'jip_engine_get_error':([P,C.POINTER(Error)],C.c_int),
    'jip_engine_get_profile':([P,C.POINTER(Profile)],C.c_int),
    'jip_engine_arithmetic':([],C.c_char_p),
    'jip_engine_build_info':([],C.c_char_p),
    'jip_engine_devices':([C.c_char_p,C.c_size_t],None)}

_LIBRARY_LOCK=threading.RLock()


def file_identity(path):
    info=Path(path).stat()
    return (info.st_dev,info.st_ino,info.st_size,info.st_mtime_ns)


@lru_cache(maxsize=8)
def _configured_library(path):
    library=C.CDLL(path)
    for name,(arguments,result) in DEFINITIONS.items():
        function=getattr(library,name);function.argtypes=arguments;function.restype=result
    with Path(path).open('rb') as stream:
        library._jip_sha256=hashlib.file_digest(stream,'sha256').hexdigest()
    library._jip_identity=file_identity(path)
    return library


def native_library(path):
    # Repeated CDLL instances create fresh ctypes function-pointer classes and
    # reopen the same dylib. Configure/reuse one binding per immutable library.
    with _LIBRARY_LOCK:
        path=str(Path(path).resolve())
        library=_configured_library(path)
        if library._jip_identity!=file_identity(path):
            raise RuntimeError('The native engine changed on disk. Restart JIP before using it.')
        return library


class Engine:
    def __init__(self,fixed=None,moving=None,settings=None,parameters=None,checkpoint=None,sampling=None):
        s=settings or {};self.path=library_path(s.get('arithmetic','compatibility'));self.lib=native_library(self.path)
        self.handle=P();self.callback=None
        actual=self.lib.jip_engine_arithmetic().decode()
        expected='reproducible-no-contract' if s.get('arithmetic','compatibility')=='reproducible' else 'compatibility-native'
        if actual!=expected:raise ValueError('The requested arithmetic mode does not match the loaded engine library.')
        if checkpoint:
            self.check(self.lib.jip_engine_checkpoint_load(os.fsencode(checkpoint),C.byref(self.handle)))
        else:
            options=Options();config=Config()
            self.lib.jip_options_default(C.byref(options));self.lib.jip_config_default(C.byref(config))
            options.metric=0 if s.get('metric','mi')=='mi' else 1
            options.grid_down=s.get('grid',8);options.regularization=s.get('regularization',5)
            options.jacobian_limit=s.get('jacobian_limit',2);options.stiffness=s.get('stiffness',0)
            if parameters is not None:options.affine_parameters[:]=parameters
            config.stages=3;config.affine_model=s.get('model',6)
            config.affine_metric=0 if s.get('affine_metric','mi')=='mi' else 1
            config.nonlinear_axes[:]=[int(a in s.get('axes','xyz')) for a in 'xyz']
            config.matching_volume=int(s.get('matching','fixed')=='all')
            config.wrap_around[:]=[0,0,0]
            fv,fa=native_image(fixed);mv,ma=native_image(moving)
            # C copies these arrays; retaining a second Python copy is unnecessary.
            if sampling is None:self.check(self.lib.jip_engine_create_configured(C.byref(fv),C.byref(mv),C.byref(options),C.byref(config),C.byref(self.handle)))
            else:
                arrays=[];native=Sampling();native.struct_size=C.sizeof(native)
                for name,shape in [('fixed',fixed.shape),('moving',moving.shape)]:
                    values=sampling.get(name)
                    if values is not None:
                        values=np.asarray(values)
                        if values.shape!=shape or not np.all((values==0)|(values==1)):raise ValueError('Invalid binary registration sampling mask')
                        array=np.asfortranarray(values,dtype=np.uint8);arrays.append(array)
                        setattr(native,name,array.ctypes.data_as(C.POINTER(C.c_uint8)));setattr(native,name+'_count',array.size)
                windows=sampling.get('moving_windows',[])
                if len(windows)>3:raise ValueError('At most three moving optimization windows are supported')
                collection=(Window*len(windows))()
                for index,w in enumerate(windows):
                    axes=w.get('axes',[0,1,2]);low=w['min'];high=w['max']
                    if len(low)!=3 or len(high)!=3 or not axes or len(set(axes))!=len(axes) or any(a not in [0,1,2] for a in axes) or not np.isfinite([low,high]).all() or any(low[a]>high[a] for a in axes):raise ValueError('Invalid physical optimization window')
                    collection[index]=Window((F*3)(*low),(F*3)(*high),sum(1<<a for a in axes))
                native.moving_windows=collection;native.moving_window_count=len(windows)
                self.check(self.lib.jip_engine_create_sampled(C.byref(fv),C.byref(mv),C.byref(options),C.byref(config),C.byref(native),C.byref(self.handle)))
        try:self.check(self.lib.jip_engine_set_execution(self.handle,s.get('backend','cpu').encode(),s.get('threads',4),s.get('gpu_budget_mb',1024)))
        except BaseException:self.close();raise

    def check(self,status):
        if status==4:raise InterruptedError('Cancelled; accepted result preserved')
        if status:
            message=self.lib.jip_engine_status_string(status).decode()
            if self.handle:
                error=Error()
                if self.lib.jip_engine_get_error(self.handle,C.byref(error))==0 and error.code==status and error.message:message=error.message.decode(errors='replace')
            raise RuntimeError(message)

    def run(self,stage,progress,cancelled,trace_sink=None,trace_details=True):
        if cancelled():raise InterruptedError('Cancelled')
        last=[0.]
        callback_errors=[]
        def trace(pointer,user):
            try:
                t=pointer.contents
                if trace_sink is not None:
                    trace_sink(struct.pack('<5q2f',t.event,t.grid_down,t.basis_count,t.basis_index,t.voxel_count,t.image_objective,t.objective))
                    if trace_details and t.basis_count:trace_sink(C.string_at(t.coefficients_xyz,int(t.basis_count)*12))
                    for axis in range(3):
                        if trace_details and t.voxel_count:trace_sink(C.string_at(t.displacement[axis],int(t.voxel_count)*4))
                if cancelled():self.lib.jip_engine_request_cancel(self.handle)
                now=time.monotonic()
                if now-last[0]>.15 or t.event in (2,3,5,6):
                    last[0]=now
                    progress(dict(stage='linear' if t.event in (5,6) else 'nonlinear',event=t.event,
                                  grid=t.grid_down,basis=int(t.basis_index),basis_count=int(t.basis_count),
                                  objective=float(t.objective) if np.isfinite(t.objective) else None,
                                  image_objective=float(t.image_objective) if np.isfinite(t.image_objective) else None))
            except Exception as error:
                callback_errors.append(error);self.lib.jip_engine_request_cancel(self.handle)
        self.callback=CALLBACK(trace)
        self.check(self.lib.jip_engine_set_trace(self.handle,self.callback,None))
        status=self.lib.jip_engine_run(self.handle,stage)
        if callback_errors:raise RuntimeError('Progress reporting failed: '+str(callback_errors[0]))
        self.check(status)
        if cancelled():raise InterruptedError('Cancelled')

    def transform(self):
        transform=Transform();self.check(self.lib.jip_engine_get_transform(self.handle,C.byref(transform)))
        result={name:list(getattr(transform,name)) for name in ['affine_parameters','centered_pull_affine','fixed_voxel_to_native_mm',
            'moving_voxel_to_native_mm','fixed_voxel_to_centered_mm','moving_centered_mm_to_voxel']}
        result.update(completed_stages=int(transform.completed_stages),nonlinear_objective=float(transform.nonlinear_objective),
                      affine_objective=float(transform.affine_objective))
        return result

    def field(self):
        """Own a copy of the exact accepted centered pull field; no resampling/I/O."""
        transform=Transform();image=Image()
        self.check(self.lib.jip_engine_get_transform(self.handle,C.byref(transform)))
        self.check(self.lib.jip_engine_result(self.handle,C.byref(image),None))
        shape=tuple(int(n) for n in image.dim);count=int(transform.count)
        if count!=int(np.prod(shape)) or count!=image.count:raise ValueError('Native field dimensions do not match the result')
        data=np.empty(shape+(3,),dtype=np.float32,order='F')
        for axis in range(3):
            if not transform.displacement[axis]:raise ValueError('Native field is missing')
            np.copyto(data[...,axis].reshape(-1,order='F'),np.ctypeslib.as_array(transform.displacement[axis],shape=(count,)))
        # Metadata uses float32 geometry exactly as the native NIfTI exporter.
        return data,np.array(transform.fixed_voxel_to_native_mm,dtype=np.float64).reshape(4,4),tuple(float(v) for v in image.spacing)

    def profile(self):
        record=Profile();record.struct_size=C.sizeof(Profile)
        self.check(self.lib.jip_engine_get_profile(self.handle,C.byref(record)))
        return {name:getattr(record,name) for name,_ in Profile._fields_ if name!='struct_size'}

    def export(self,folder):
        folder=Path(folder)
        self.check(self.lib.jip_engine_export(self.handle,os.fsencode(folder/'aligned.nii'),os.fsencode(folder/'field.nii')))
        self.check(self.lib.jip_engine_checkpoint_save(self.handle,os.fsencode(folder/'registration.jipcp')))

    def execution(self):
        record=Execution();record.struct_size=C.sizeof(Execution);self.check(self.lib.jip_engine_get_execution(self.handle,C.byref(record)))
        result={name:getattr(record,name) for name in ['threads','budget_mb','map_arithmetic','map_architecture','calls','gpu_calls','cpu_calls','fallbacks','device_bytes','gpu_seconds','cpu_seconds']}
        result.update({name:getattr(record,name).decode(errors='replace') for name in ['requested','active','device','reason']})
        result.update(build=json.loads(self.lib.jip_engine_build_info().decode()),library_sha256=self.lib._jip_sha256,profile=self.profile())
        return result
    def close(self):
        if self.handle:self.lib.jip_engine_destroy(self.handle);self.handle=P()
        self.callback=None

    def __enter__(self):return self
    def __exit__(self,*args):self.close()


def devices():
    library=native_library(library_path());function=library.jip_engine_devices
    buffer=C.create_string_buffer(4096);function(buffer,len(buffer));return json.loads(buffer.value.decode())
