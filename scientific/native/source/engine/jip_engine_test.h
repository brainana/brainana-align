/* Deterministic test controls. Not part of the embedding ABI. A configuration
   is thread-local and consumed by the next entered engine operation only. */
#ifndef JIP_ENGINE_TEST_H
#define JIP_ENGINE_TEST_H
#include "jip_engine.h"
typedef struct {uint64_t allocations,polls; jip_status status;uint64_t peak_new_bytes;} jip_test_operation;
void jip_engine_test_fault(uint64_t allocation,uint64_t poll,jip_status kind);
jip_test_operation jip_engine_test_last_operation(void);
jip_test_operation jip_engine_test_current_operation(void);
void jip_engine_test_live(uint64_t *blocks,uint64_t *bytes,uint64_t *handles,uint64_t *resources);
void jip_engine_test_acceleration_fault(jip_engine*,uint64_t calls_until_failure);
#endif
