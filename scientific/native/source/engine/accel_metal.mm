#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#include "accel_device.hpp"
#include "accel_shader.h"
#include <algorithm>
#include <cstring>
struct MetalDevice: JipDevice {
  id<MTLDevice> device;id<MTLCommandQueue> queue;id<MTLComputePipelineState> pipeline;
  id<MTLBuffer> image=nil,jobs=nil,output=nil;
  MetalDevice(id<MTLDevice> d,id<MTLCommandQueue> q,id<MTLComputePipelineState> p):device(d),queue(q),pipeline(p){name=d.name.UTF8String;}
  bool run(const float *source,uint64_t count,bool changed,const jip_sample *samples,const jip_kernel_params *params,uint32_t *result,std::string &error) override {
    @autoreleasepool { @try {
      size_t image_bytes=(size_t)count*4,job_bytes=(size_t)params->count*sizeof(jip_sample),out_bytes=(size_t)params->count*4;
      if(image_bytes>device.maxBufferLength||job_bytes>device.maxBufferLength||out_bytes>device.maxBufferLength){error="Metal buffer limit exceeded.";return false;}
      if(!image||image.length!=image_bytes){image=[device newBufferWithLength:image_bytes options:MTLResourceStorageModeShared];changed=true;}
      if(!jobs||jobs.length<job_bytes)jobs=[device newBufferWithLength:job_bytes options:MTLResourceStorageModeShared];
      if(!output||output.length<out_bytes)output=[device newBufferWithLength:out_bytes options:MTLResourceStorageModeShared];
      if(!image||!jobs||!output){error="Metal could not allocate its buffers.";return false;}
      bytes=image.length+jobs.length+output.length;
      if(changed)std::memcpy(image.contents,source,image_bytes);std::memcpy(jobs.contents,samples,job_bytes);
      id<MTLCommandBuffer> command=[queue commandBuffer];id<MTLComputeCommandEncoder> encoder=[command computeCommandEncoder];
      if(!command||!encoder){error="Metal could not create a command.";return false;}
      [encoder setComputePipelineState:pipeline];[encoder setBuffer:image offset:0 atIndex:0];[encoder setBuffer:jobs offset:0 atIndex:1];[encoder setBuffer:output offset:0 atIndex:2];[encoder setBytes:params length:sizeof(*params) atIndex:3];
      NSUInteger width=std::min((NSUInteger)128,pipeline.maxTotalThreadsPerThreadgroup);
      [encoder dispatchThreads:MTLSizeMake(params->count,1,1) threadsPerThreadgroup:MTLSizeMake(width,1,1)];[encoder endEncoding];[command commit];[command waitUntilCompleted];
      if(command.status!=MTLCommandBufferStatusCompleted){error=command.error?command.error.localizedDescription.UTF8String:"Metal command failed.";return false;}
      std::memcpy(result,output.contents,out_bytes);return true;
    } @catch(NSException *exception) {error=exception.reason.UTF8String?:"Metal raised an exception; CPU resumed.";return false;}
    }
  }
};
std::unique_ptr<JipDevice> jip_metal_device(std::string &error) {
  @autoreleasepool { @try {
    id<MTLDevice> device=MTLCreateSystemDefaultDevice();if(!device){error="Metal returned no device; CPU is available.";return nullptr;}
    NSString *source=[[NSString alloc] initWithUTF8String:jip_shared_shader];
    source=[source stringByAppendingString:@"\nkernel void jip_interpolate(device const uint *source [[buffer(0)]],device const jip_sample *jobs [[buffer(1)]],device uint *output [[buffer(2)]],constant jip_kernel_params &params [[buffer(3)]],uint i [[thread_position_in_grid]]) {if(i<params.count)output[i]=jip_sample_bits(jobs+i,source,&params);}\n"];
    NSError *problem=nil;MTLCompileOptions *options=[MTLCompileOptions new];options.fastMathEnabled=NO;
    id<MTLLibrary> library=[device newLibraryWithSource:source options:options error:&problem];
    if(!library){error="Metal shader compilation failed: ";error+=problem?problem.localizedDescription.UTF8String:"the runtime compiler returned no library.";return nullptr;}
    id<MTLComputePipelineState> pipeline=[device newComputePipelineStateWithFunction:[library newFunctionWithName:@"jip_interpolate"] error:&problem];
    id<MTLCommandQueue> queue=[device newCommandQueue];
    if(!pipeline||!queue){error=problem?problem.localizedDescription.UTF8String:"Metal pipeline unavailable.";return nullptr;}
    return std::make_unique<MetalDevice>(device,queue,pipeline);
  } @catch(NSException *exception) {error=exception.reason.UTF8String?:"Metal discovery failed; CPU remains available.";return nullptr;}
  }
}
