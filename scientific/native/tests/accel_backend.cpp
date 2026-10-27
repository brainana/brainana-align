#include "accel_device.hpp"
#include <cstdio>
#include <cstring>
#include <random>
#include <vector>
#include <thread>
#include <chrono>
struct SimulatedDevice:JipDevice {
  std::vector<uint32_t> cache;int behavior=0;
  explicit SimulatedDevice(int mode):behavior(mode){name="CPU shader simulator — not GPU execution";}
  bool run(const float *source,uint64_t count,bool changed,const jip_sample *jobs,const jip_kernel_params *params,uint32_t *output,std::string &error) override {
    if(behavior==3)std::this_thread::sleep_for(std::chrono::milliseconds(20));
    if(behavior==1){output[0]=42;error="Injected dispatch failure";return false;}
    if(changed){cache.resize((size_t)count);std::memcpy(cache.data(),source,(size_t)count*4);}
    for(uint32_t i=0;i<params->count;i++)output[i]=jip_sample_bits(jobs+i,cache.data(),params);
    if(behavior==2)output[0]^=1;return true;
  }
};
#define CHECK(x) do{if(!(x)){std::fprintf(stderr,"BACKEND_FAIL %d: %s\n",__LINE__,#x);return 1;}}while(0)
int main(){
 std::mt19937 random(8741);jip_kernel_params p={{7,8,9},{1,1,1},300};std::vector<float> source(7*8*9);
 for(size_t i=0;i<source.size();i++)source[i]=i%11==0?0.f:(float)((int)(random()%10000)-5000)/127.f;
 std::vector<jip_sample> jobs(p.count);std::vector<uint32_t> expected(p.count),actual(p.count);
 for(unsigned i=0;i<p.count;i++){
  auto &j=jobs[i];j.valid=i%13!=0;
  for(int axis=0;axis<3;axis++){j.low[axis]=(int)(random()%15)-6;j.length[axis]=1+random()%8;
   for(unsigned k=0;k<j.length[axis];k++){float value=((int)(random()%100)-45)/71.f;std::memcpy(&j.coeff[axis*JIP_KERNEL_TAPS+k],&value,4);}}
 }
 auto threaded=jip_accel_create("threads",4,64);CHECK(threaded&&jip_accel_run(threaded,source.data(),source.size(),1,jobs.data(),&p,expected.data()));
 auto simulated=jip_accel_create("cpu",1,64);jip_accel_install_test_device(simulated,std::make_unique<SimulatedDevice>(0));
 CHECK(jip_accel_run(simulated,source.data(),source.size(),1,jobs.data(),&p,actual.data()));CHECK(expected==actual);
 jobs[0]={};jobs[0].valid=1;for(int axis=0;axis<3;axis++){jobs[0].length[axis]=1;jobs[0].coeff[axis*JIP_KERNEL_TAPS]=0x3f800000;}
 source[0]=42;CHECK(jip_accel_run(simulated,source.data(),source.size(),2,jobs.data(),&p,actual.data()));float first;std::memcpy(&first,actual.data(),4);CHECK(first==42);
 jip_accel_destroy(simulated);
 jip_sample *workspace_jobs,*same_jobs;uint32_t *workspace_output;int64_t *workspace_indices;uint64_t allocations,workspace_bytes;
 CHECK(jip_accel_workspace(threaded,4096,&workspace_jobs,&workspace_output,&workspace_indices));
 CHECK(jip_accel_workspace(threaded,300,&same_jobs,&workspace_output,&workspace_indices)&&same_jobs==workspace_jobs);
 jip_accel_workspace_info(threaded,&workspace_bytes,&allocations);CHECK(allocations==1&&workspace_bytes>0);
 CHECK(!jip_accel_workspace(threaded,16385,&same_jobs,&workspace_output,&workspace_indices));
 CHECK(jip_accel_tile_size(threaded,100000)==4096);
 for(unsigned stage=1;stage<=3;stage++){
  auto partial=jip_accel_create("threads",2,64);
  CHECK(jip_accel_workspace(partial,64,&workspace_jobs,&workspace_output,&workspace_indices));
  jip_accel_test_workspace_failure(partial,stage);
  CHECK(!jip_accel_workspace(partial,4096,&workspace_jobs,&workspace_output,&workspace_indices));
  CHECK(jip_accel_workspace(partial,4096,&workspace_jobs,&workspace_output,&workspace_indices));
  // Probe the last slot of EVERY vector after failure and retry, then run the
  // interpolation kernel using that returned storage (not just a source match).
  workspace_jobs[4095]={};workspace_output[4095]=42;workspace_indices[4095]=4095;
  std::memcpy(workspace_jobs,jobs.data(),jobs.size()*sizeof(jip_sample));
  CHECK(jip_accel_run(partial,source.data(),source.size(),2,workspace_jobs,&p,workspace_output));
  CHECK(!std::memcmp(workspace_output,actual.data(),p.count*4));jip_accel_destroy(partial);
 }
 simulated=jip_accel_create("cpu",1,64);jip_accel_install_test_device(simulated,std::make_unique<SimulatedDevice>(3),true);
 for(int trial=0;trial<3;trial++)CHECK(jip_accel_run(simulated,source.data(),source.size(),2,jobs.data(),&p,actual.data()));
 CHECK(!jip_accel_run(simulated,source.data(),source.size(),2,jobs.data(),&p,actual.data()));
 jip_execution warm{};jip_accel_info(simulated,&warm);CHECK(warm.gpu_calls==4&&warm.fallbacks==1&&std::strstr(warm.reason,"Warm GPU"));jip_accel_destroy(simulated);
 for(int failure=1;failure<=2;failure++){
  simulated=jip_accel_create("cpu",1,64);jip_accel_install_test_device(simulated,std::make_unique<SimulatedDevice>(failure));CHECK(!jip_accel_run(simulated,source.data(),source.size(),1,jobs.data(),&p,actual.data()));
  jip_execution info{};jip_accel_info(simulated,&info);CHECK(!std::strcmp(info.active,"cpu")&&info.fallbacks==1&&info.reason[0]);jip_accel_destroy(simulated);
 }
 simulated=jip_accel_create("cpu",1,32);jip_accel_install_test_device(simulated,std::make_unique<SimulatedDevice>(0));
 CHECK(!jip_accel_run(simulated,source.data(),100000000,1,jobs.data(),&p,actual.data()));jip_accel_destroy(simulated);jip_accel_destroy(threaded);
 std::puts("ACCEL_BACKEND_RESULT CPU shader simulation exact; epoch invalidation, dispatch/verification failure and budget fallback pass. No hardware GPU test.");return 0;
}
