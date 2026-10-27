#include "accel_device.hpp"
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <exception>
using Clock=std::chrono::steady_clock;
static double seconds(Clock::time_point at){return std::chrono::duration<double>(Clock::now()-at).count();}
static float fvalue(uint32_t bits){float value;std::memcpy(&value,&bits,4);return value;}
static uint32_t fbits(float value){uint32_t bits;std::memcpy(&bits,&value,4);return bits;}
static uint32_t cpu_sample(const jip_sample &job,const float *source,const jip_kernel_params &p) {
  if(!job.valid)return 0;double sum=0,denom=0;
  for(uint32_t z=0;z<job.length[2];z++) {
    int iz=jip_wrap(job.low[2]+(int)z,p.dim[2],p.wrap[2]);if(iz<0||(uint32_t)iz>=p.dim[2])continue;
    for(uint32_t y=0;y<job.length[1];y++) {
      int iy=jip_wrap(job.low[1]+(int)y,p.dim[1],p.wrap[1]);if(iy<0||(uint32_t)iy>=p.dim[1])continue;
      uint64_t row=((uint64_t)iz*p.dim[1]+(uint32_t)iy)*p.dim[0];
      for(uint32_t x=0;x<job.length[0];x++) {
        int ix=jip_wrap(job.low[0]+(int)x,p.dim[0],p.wrap[0]);if(ix<0||(uint32_t)ix>=p.dim[0])continue;
        float product=fvalue(job.coeff[x])*fvalue(job.coeff[JIP_KERNEL_TAPS+y]);
        product*=fvalue(job.coeff[2*JIP_KERNEL_TAPS+z]);double weight=product;
        sum+=weight*source[row+(uint32_t)ix];denom+=weight;
      }
    }
  }
  return denom==0?0:fbits((float)(sum/denom));
}
class WorkerPool {
  std::mutex mutex;std::condition_variable changed,finished;std::vector<std::thread> workers;
  bool stop=false;uint64_t generation=0;unsigned remaining=0;std::function<void(unsigned)> task;std::exception_ptr failure;
public:
  explicit WorkerPool(unsigned count){
    try {for(unsigned n=1;n<count;n++)workers.emplace_back([this,n]{uint64_t seen=0;for(;;){
      {std::unique_lock<std::mutex> lock(mutex);changed.wait(lock,[&]{return stop||generation!=seen;});if(stop)return;seen=generation;}
      try{task(n);}catch(...){std::lock_guard<std::mutex> lock(mutex);failure=std::current_exception();}
      {std::lock_guard<std::mutex> lock(mutex);if(!--remaining)finished.notify_one();}
    }});}
    catch(...){shutdown();throw;}
  }
  void shutdown(){{std::lock_guard<std::mutex> lock(mutex);stop=true;}changed.notify_all();for(auto &worker:workers)if(worker.joinable())worker.join();}
  ~WorkerPool(){shutdown();}
  void run(std::function<void(unsigned)> next){
    {std::lock_guard<std::mutex> lock(mutex);task=std::move(next);remaining=(unsigned)workers.size();failure=nullptr;generation++;}changed.notify_all();
    try{task(0);}catch(...){std::lock_guard<std::mutex> lock(mutex);failure=std::current_exception();}
    std::unique_lock<std::mutex> lock(mutex);finished.wait(lock,[&]{return !remaining;});if(failure)std::rethrow_exception(failure);
  }
};
struct jip_accelerator {
  jip_execution info{};std::unique_ptr<JipDevice> device;
  std::unique_ptr<WorkerPool> pool;
  const float *source=nullptr;uint64_t source_count=0,epoch=0;bool verified=false,disabled=false;
  uint64_t fail_on=0,workspace_allocations=0;
  std::vector<jip_sample> jobs;std::vector<uint32_t> output;std::vector<int64_t> indices;
  unsigned performance_trials=0,workspace_fail_stage=0;double trial_gpu=0,trial_cpu=0;
};
extern "C" double jip_monotonic_seconds(void){return std::chrono::duration<double>(Clock::now().time_since_epoch()).count();}
extern "C" uint32_t jip_accel_tile_size(jip_accelerator *a,uint64_t touched){
  uint64_t cap=a&&!std::strcmp(a->info.active,"threads")?4096:16384;
  return (uint32_t)std::min(touched,cap);
}
void jip_accel_test_workspace_failure(jip_accelerator *a,unsigned stage){if(a)a->workspace_fail_stage=stage;}
extern "C" int jip_accel_workspace(jip_accelerator *a,uint32_t count,jip_sample **jobs,uint32_t **output,int64_t **indices){
  if(!a||!count||count>16384||!jobs||!output||!indices)return 0;
  try{if(a->jobs.size()<count||a->output.size()<count||a->indices.size()<count){
      // A failed vector growth can leave another vector enlarged. Check ALL
      // lengths on retry; never publish undersized/null output/index storage.
      a->jobs.resize(count);if(a->workspace_fail_stage==1)throw std::bad_alloc();
      a->output.resize(count);if(a->workspace_fail_stage==2)throw std::bad_alloc();
      a->indices.resize(count);if(a->workspace_fail_stage==3)throw std::bad_alloc();
      a->workspace_allocations++;
    }
    *jobs=a->jobs.data();*output=a->output.data();*indices=a->indices.data();return 1;
  }catch(...){a->workspace_fail_stage=0;return 0;}
}
extern "C" void jip_accel_workspace_info(jip_accelerator *a,uint64_t *bytes,uint64_t *allocations){
  if(bytes)*bytes=a?a->jobs.capacity()*sizeof(jip_sample)+a->output.capacity()*4+a->indices.capacity()*8:0;
  if(allocations)*allocations=a?a->workspace_allocations:0;
}
static void message(char *target,size_t size,const char *text){std::snprintf(target,size,"%s",text);}
static void message(char *target,size_t size,const std::string &text){message(target,size,text.c_str());}
void jip_accel_install_test_device(jip_accelerator *a,std::unique_ptr<JipDevice> device,bool automatic){a->device=std::move(device);a->disabled=false;a->verified=false;message(a->info.active,sizeof(a->info.active),"test-device");message(a->info.requested,sizeof(a->info.requested),automatic?"auto":"test-device");}
extern "C" void jip_accel_test_fault(jip_accelerator *a,uint64_t after){if(a)a->fail_on=after?a->info.calls+after:0;}
extern "C" jip_accelerator *jip_accel_create(const char *requested,unsigned threads,unsigned budget) {
  try {
    auto a=std::make_unique<jip_accelerator>();a->info.struct_size=sizeof(jip_execution);a->info.threads=std::max(1u,std::min(threads,64u));a->info.budget_mb=std::max(32u,std::min(budget,16384u));
    std::string backend=requested?requested:"cpu";message(a->info.requested,sizeof(a->info.requested),backend);message(a->info.active,sizeof(a->info.active),"cpu");
    if(backend=="threads"){message(a->info.active,sizeof(a->info.active),"threads");return a.release();}
    if(backend=="cpu")return a.release();
    std::string error;
    std::string selected;
    try {
    if(backend=="metal"||backend=="auto"){a->device=jip_metal_device(error);if(a->device)selected="metal";}
    if(!a->device&&(backend=="cuda"||backend=="auto")){a->device=jip_cuda_device(error);if(a->device)selected="cuda";}
    }catch(const std::exception &problem){a->disabled=true;a->info.fallbacks++;message(a->info.reason,sizeof(a->info.reason),problem.what());return a.release();}
    catch(...){a->disabled=true;a->info.fallbacks++;message(a->info.reason,sizeof(a->info.reason),"GPU setup failed; CPU remains available.");return a.release();}
    if(a->device){message(a->info.active,sizeof(a->info.active),selected);message(a->info.device,sizeof(a->info.device),a->device->name);}
    else{a->disabled=true;a->info.fallbacks++;message(a->info.reason,sizeof(a->info.reason),error.empty()?"No usable GPU backend; CPU is available.":error);}
    return a.release();
  }catch(...){return nullptr;}
}
extern "C" void jip_accel_destroy(jip_accelerator *a){try{delete a;}catch(...){}}
extern "C" void jip_accel_info(jip_accelerator *a,jip_execution *info){if(!info)return;if(a)*info=a->info;else{std::memset(info,0,sizeof(*info));info->struct_size=sizeof(*info);message(info->requested,sizeof(info->requested),"cpu");message(info->active,sizeof(info->active),"cpu");}}
extern "C" void jip_accel_disable(jip_accelerator *a,const char *reason){if(!a)return;a->disabled=true;a->device.reset();a->info.fallbacks++;a->info.device_bytes=0;message(a->info.active,sizeof(a->info.active),"cpu");message(a->info.reason,sizeof(a->info.reason),reason?reason:"GPU failed; CPU resumed.");}
extern "C" int jip_accel_run(jip_accelerator *a,const float *source,uint64_t count,uint64_t epoch,const jip_sample *jobs,const jip_kernel_params *p,uint32_t *output) {
  if(!a||a->disabled||!p||!p->count||!source||!jobs||!output)return 0;
  try {
    a->info.calls++;
    if(a->fail_on&&a->info.calls==a->fail_on){output[0]=0x12345678;jip_accel_disable(a,"Injected device/worker failure; CPU resumed.");return 0;}
    if(!std::strcmp(a->info.active,"threads")) {
      auto start=Clock::now();unsigned n=a->info.threads;if(!a->pool)a->pool=std::make_unique<WorkerPool>(n);
      a->pool->run([=](unsigned thread){uint32_t first=(uint32_t)((uint64_t)p->count*thread/n),last=(uint32_t)((uint64_t)p->count*(thread+1)/n);for(uint32_t i=first;i<last;i++)output[i]=cpu_sample(jobs[i],source,*p);});
      a->info.cpu_calls++;a->info.cpu_seconds+=seconds(start);return 1;
    }
    if(!a->device)return 0;
    uint64_t required=count*4+(uint64_t)p->count*(sizeof(jip_sample)+4);
    if(count>SIZE_MAX/4||required<(count*4)||required>(uint64_t)a->info.budget_mb*1024*1024){jip_accel_disable(a,"GPU memory budget exceeded; using CPU.");return 0;}
    bool changed=a->source!=source||a->source_count!=count||a->epoch!=epoch;
    auto start=Clock::now();std::string error;
    if(!a->device->run(source,count,changed,jobs,p,output,error)){jip_accel_disable(a,error.c_str());return 0;}
    double elapsed=seconds(start);a->info.gpu_calls++;a->info.gpu_seconds+=elapsed;a->info.device_bytes=a->device->bytes;
    a->source=source;a->source_count=count;a->epoch=epoch;
    if(!a->verified){
      auto cpu_start=Clock::now();for(uint32_t i=0;i<p->count;i++)if(cpu_sample(jobs[i],source,*p)!=output[i]){jip_accel_disable(a,"GPU verification differed from the CPU reference; using CPU.");return 0;}
      (void)seconds(cpu_start);a->verified=true;
    }else if(!changed&&!std::strcmp(a->info.requested,"auto")&&a->performance_trials<3){
      auto reference=Clock::now();for(uint32_t i=0;i<p->count;i++)if(cpu_sample(jobs[i],source,*p)!=output[i]){jip_accel_disable(a,"GPU verification differed from the CPU reference; using CPU.");return 0;}
      a->trial_cpu+=seconds(reference);a->trial_gpu+=elapsed;a->performance_trials++;
      if(a->performance_trials==3&&a->trial_gpu*1.2>=a->trial_cpu){jip_accel_disable(a,"Warm GPU trials did not outperform CPU including transfer/dispatch; using CPU.");return 0;}
    }
    return 1;
  }catch(const std::exception &error){jip_accel_disable(a,error.what());return 0;}catch(...){jip_accel_disable(a,"Acceleration failed; using CPU.");return 0;}
}
extern "C" void jip_accel_probe(char *buffer,size_t size) {
  if(!buffer||!size)return;
  try {
  // Use the same creation path as a real job, including runtime compilation.
  std::string result="{\"cpu\":true,\"threaded_cpu\":true";
  for(const char *kind:{"metal","cuda"}) {
    auto a=jip_accel_create(kind,1,512);jip_execution info{};jip_accel_info(a,&info);
    auto escape=[](const char *s){std::string value;for(;*s;s++){if(*s=='"'||*s=='\\')value+='\\';if((unsigned char)*s>=32)value+=*s;}return value;};
    result+=",\""+std::string(kind)+"\":{\"available\":"+(a&&!a->disabled?"true":"false")+",\"device\":\""+escape(info.device)+"\",\"reason\":\""+escape(info.reason)+"\"}";jip_accel_destroy(a);
  }
  result+="}";message(buffer,size,result);
  }catch(...){std::snprintf(buffer,size,"{\"cpu\":true,\"threaded_cpu\":true,\"error\":\"GPU discovery failed; CPU remains available.\"}");}
}
