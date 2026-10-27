#pragma once
#include "acceleration.h"
#include <memory>
#include <string>
struct JipDevice {
  std::string name;
  uint64_t bytes=0;
  virtual ~JipDevice()=default;
  virtual bool run(const float*,uint64_t,bool,const jip_sample*,const jip_kernel_params*,uint32_t*,std::string&)=0;
};
std::unique_ptr<JipDevice> jip_metal_device(std::string&);
std::unique_ptr<JipDevice> jip_cuda_device(std::string&);
/* Test-only dependency injection. No public backend name selects this seam. */
void jip_accel_install_test_device(jip_accelerator*,std::unique_ptr<JipDevice>,bool automatic=false);

/* Inject a vector-growth failure in tests; no public backend selects this. */
void jip_accel_test_workspace_failure(jip_accelerator*,unsigned stage);
