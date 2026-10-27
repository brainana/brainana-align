/* Optional CUDA Driver + NVRTC runtime loading. CPU launch never requires CUDA
   headers, an SDK, a driver, an NVIDIA device or an NVRTC shared library. */
#include "accel_device.hpp"
#include "accel_shader.h"
#include <cstring>
#include <vector>
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#define JIP_CUDA_CALL __stdcall
static void *open_library(const char *name){return (void*)LoadLibraryA(name);}
static void *symbol(void *h,const char *name){return (void*)GetProcAddress((HMODULE)h,name);}
static void close_library(void *h){if(h)FreeLibrary((HMODULE)h);}
#else
#include <dlfcn.h>
#define JIP_CUDA_CALL
static void *open_library(const char *name){return dlopen(name,RTLD_NOW|RTLD_LOCAL);}
static void *symbol(void *h,const char *name){return dlsym(h,name);}
static void close_library(void *h){if(h)dlclose(h);}
#endif
using CUcontext=void*;using CUmodule=void*;using CUfunction=void*;using CUstream=void*;using CUdeviceptr=uint64_t;using nvrtcProgram=void*;
struct CudaDevice: JipDevice {
  void *driver=nullptr,*rtc=nullptr;int device=0;CUcontext context=nullptr;CUmodule module=nullptr;CUfunction function=nullptr;CUstream stream=nullptr;
  CUdeviceptr image=0,jobs=0,output=0;uint64_t image_bytes=0,job_bytes=0,out_bytes=0;
#define FN(name,args) int (JIP_CUDA_CALL *name) args=nullptr
  FN(cuInit,(unsigned));FN(cuDeviceGet,(int*,int));FN(cuDeviceGetName,(char*,int,int));FN(cuDeviceGetAttribute,(int*,int,int));
  FN(cuDevicePrimaryCtxRetain,(CUcontext*,int));FN(cuDevicePrimaryCtxRelease_v2,(int));FN(cuCtxPushCurrent_v2,(CUcontext));FN(cuCtxPopCurrent_v2,(CUcontext*));
  FN(cuModuleLoadData,(CUmodule*,const void*));FN(cuModuleGetFunction,(CUfunction*,CUmodule,const char*));FN(cuModuleUnload,(CUmodule));
  FN(cuMemAlloc_v2,(CUdeviceptr*,size_t));FN(cuMemFree_v2,(CUdeviceptr));FN(cuMemcpyHtoD_v2,(CUdeviceptr,const void*,size_t));FN(cuMemcpyDtoH_v2,(void*,CUdeviceptr,size_t));
  FN(cuStreamCreate,(CUstream*,unsigned));FN(cuStreamSynchronize,(CUstream));FN(cuStreamDestroy_v2,(CUstream));
  FN(cuLaunchKernel,(CUfunction,unsigned,unsigned,unsigned,unsigned,unsigned,unsigned,unsigned,CUstream,void**,void**));
  FN(nvrtcCreateProgram,(nvrtcProgram*,const char*,const char*,int,const char*const*,const char*const*));
  FN(nvrtcCompileProgram,(nvrtcProgram,int,const char*const*));FN(nvrtcGetPTXSize,(nvrtcProgram,size_t*));FN(nvrtcGetPTX,(nvrtcProgram,char*));
  FN(nvrtcGetProgramLogSize,(nvrtcProgram,size_t*));FN(nvrtcGetProgramLog,(nvrtcProgram,char*));FN(nvrtcDestroyProgram,(nvrtcProgram*));
#undef FN
  bool pushed=false;
  bool push(){pushed=cuCtxPushCurrent_v2&&cuCtxPushCurrent_v2(context)==0;return pushed;}
  void pop(){if(pushed){CUcontext ignored;cuCtxPopCurrent_v2(&ignored);pushed=false;}}
  ~CudaDevice() override {
    if(pushed)pop();
    if(context&&push()){if(image)cuMemFree_v2(image);if(jobs)cuMemFree_v2(jobs);if(output)cuMemFree_v2(output);if(stream)cuStreamDestroy_v2(stream);if(module)cuModuleUnload(module);pop();}
    if(context&&cuDevicePrimaryCtxRelease_v2)cuDevicePrimaryCtxRelease_v2(device);close_library(rtc);close_library(driver);
  }
  bool run(const float *source,uint64_t count,bool changed,const jip_sample *samples,const jip_kernel_params *params,uint32_t *result,std::string &error) override {
    if(!push()){error="CUDA context is unavailable.";return false;}
    auto fail=[&]{error="CUDA allocation, transfer or dispatch failed; CPU resumed.";pop();return false;};
    size_t ib=(size_t)count*4,jb=(size_t)params->count*sizeof(jip_sample),ob=(size_t)params->count*4;
    auto allocate=[&](CUdeviceptr &pointer,uint64_t &capacity,size_t need){if(capacity<need){if(pointer){cuMemFree_v2(pointer);pointer=0;capacity=0;}if(cuMemAlloc_v2(&pointer,need))return false;capacity=need;}return true;};
    if(!allocate(image,image_bytes,ib)||!allocate(jobs,job_bytes,jb)||!allocate(output,out_bytes,ob))return fail();bytes=image_bytes+job_bytes+out_bytes;
    if((changed&&cuMemcpyHtoD_v2(image,source,ib))||cuMemcpyHtoD_v2(jobs,samples,jb))return fail();
    jip_kernel_params parameters=*params;void *arguments[]={&image,&jobs,&output,&parameters};
    if(cuLaunchKernel(function,(params->count+127)/128,1,1,128,1,1,0,stream,arguments,nullptr)||cuStreamSynchronize(stream)||cuMemcpyDtoH_v2(result,output,ob))return fail();
    pop();return true;
  }
};
std::unique_ptr<JipDevice> jip_cuda_device(std::string &error) {
  auto d=std::make_unique<CudaDevice>();
#ifdef _WIN32
  d->driver=open_library("nvcuda.dll");
  for(const char *name:{"nvrtc64_130_0.dll","nvrtc64_120_0.dll","nvrtc64_112_0.dll","nvrtc64_111_0.dll","nvrtc64_110_0.dll"})if(!d->rtc)d->rtc=open_library(name);
#else
  d->driver=open_library("libcuda.so.1");
  for(const char *name:{"libnvrtc.so.13","libnvrtc.so.12","libnvrtc.so.11.2","libnvrtc.so"})if(!d->rtc)d->rtc=open_library(name);
#endif
  if(!d->driver||!d->rtc){error="CUDA driver or runtime compiler (NVRTC) is unavailable; using CPU.";return nullptr;}
#define LOAD(lib,name) if(!(d->name=reinterpret_cast<decltype(d->name)>(symbol(d->lib,#name)))){error="Missing CUDA function " #name;return nullptr;}
  LOAD(driver,cuInit);LOAD(driver,cuDeviceGet);LOAD(driver,cuDeviceGetName);LOAD(driver,cuDeviceGetAttribute);
  LOAD(driver,cuDevicePrimaryCtxRetain);LOAD(driver,cuDevicePrimaryCtxRelease_v2);LOAD(driver,cuCtxPushCurrent_v2);LOAD(driver,cuCtxPopCurrent_v2);
  LOAD(driver,cuModuleLoadData);LOAD(driver,cuModuleGetFunction);LOAD(driver,cuModuleUnload);LOAD(driver,cuMemAlloc_v2);LOAD(driver,cuMemFree_v2);LOAD(driver,cuMemcpyHtoD_v2);LOAD(driver,cuMemcpyDtoH_v2);
  LOAD(driver,cuStreamCreate);LOAD(driver,cuStreamSynchronize);LOAD(driver,cuStreamDestroy_v2);LOAD(driver,cuLaunchKernel);
  LOAD(rtc,nvrtcCreateProgram);LOAD(rtc,nvrtcCompileProgram);LOAD(rtc,nvrtcGetPTXSize);LOAD(rtc,nvrtcGetPTX);LOAD(rtc,nvrtcGetProgramLogSize);LOAD(rtc,nvrtcGetProgramLog);LOAD(rtc,nvrtcDestroyProgram);
#undef LOAD
  if(d->cuInit(0)||d->cuDeviceGet(&d->device,0)){error="No usable NVIDIA CUDA device; using CPU.";return nullptr;}
  char name[160]={0};d->cuDeviceGetName(name,sizeof(name),d->device);d->name=name;
  int major=0,minor=0;d->cuDeviceGetAttribute(&major,75,d->device);d->cuDeviceGetAttribute(&minor,76,d->device);
  if(major<5){error="CUDA device compute capability is too old.";return nullptr;}
  if(d->cuDevicePrimaryCtxRetain(&d->context,d->device)||!d->push()){error="CUDA context creation failed.";return nullptr;}
  std::string code=jip_shared_shader;code+=R"cuda(
extern "C" __global__ void jip_interpolate(const jip_u32 *source,const jip_sample *jobs,jip_u32 *output,jip_kernel_params params) {
  unsigned i=blockIdx.x*blockDim.x+threadIdx.x;if(i<params.count)output[i]=jip_sample_bits(jobs+i,source,&params);
})cuda";
  nvrtcProgram program=nullptr;
  struct ProgramGuard {nvrtcProgram &program;decltype(d->nvrtcDestroyProgram) destroy;~ProgramGuard(){if(program)destroy(&program);}} program_guard{program,d->nvrtcDestroyProgram};
  if(d->nvrtcCreateProgram(&program,code.c_str(),"jip_interpolation.cu",0,nullptr,nullptr)){d->pop();error="CUDA compiler setup failed.";return nullptr;}
  std::string architecture="--gpu-architecture=compute_"+std::to_string(major)+std::to_string(minor);
  const char *options[]={architecture.c_str(),"--std=c++14","--fmad=false","--ftz=false","--prec-div=true"};
  int status=d->nvrtcCompileProgram(program,5,options);size_t bytes=0;
  if(status){d->nvrtcGetProgramLogSize(program,&bytes);std::vector<char> log(bytes+1);d->nvrtcGetProgramLog(program,log.data());error="CUDA kernel compilation failed: "+std::string(log.data());d->nvrtcDestroyProgram(&program);d->pop();return nullptr;}
  if(d->nvrtcGetPTXSize(program,&bytes)||!bytes){d->pop();error="CUDA compiler produced no executable kernel; CPU resumed.";return nullptr;}
  std::vector<char> ptx(bytes);
  status=d->nvrtcGetPTX(program,ptx.data());d->nvrtcDestroyProgram(&program);
  if(status||d->cuModuleLoadData(&d->module,ptx.data())||d->cuModuleGetFunction(&d->function,d->module,"jip_interpolate")||d->cuStreamCreate(&d->stream,0)){d->pop();error="CUDA module/stream creation failed.";return nullptr;}
  d->pop();return d;
}
