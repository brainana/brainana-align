#ifndef JIP_ACCELERATION_H
#define JIP_ACCELERATION_H
#include <stddef.h>
#include <stdint.h>
#include "jip_engine.h"
#include "accel_kernel.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct jip_accelerator jip_accelerator;
jip_accelerator *jip_accel_create(const char*,unsigned,unsigned);
void jip_accel_destroy(jip_accelerator*);
void jip_accel_info(jip_accelerator*,jip_execution*);
/* All errors are returned; no exception or legacy longjmp crosses C++/GPU. */
int jip_accel_run(jip_accelerator*,const float*,uint64_t,uint64_t,const jip_sample*,const jip_kernel_params*,uint32_t*);
void jip_accel_probe(char*,size_t);
void jip_accel_disable(jip_accelerator*,const char*);
void jip_accel_test_fault(jip_accelerator*,uint64_t);
double jip_monotonic_seconds(void);
/* Context-owned, bounded scratch. Failure falls back to the serial region. */
uint32_t jip_accel_tile_size(jip_accelerator*,uint64_t);
int jip_accel_workspace(jip_accelerator*,uint32_t,jip_sample**,uint32_t**,int64_t**);
void jip_accel_workspace_info(jip_accelerator*,uint64_t*,uint64_t*);
#ifdef __cplusplus
}
#endif
#endif
