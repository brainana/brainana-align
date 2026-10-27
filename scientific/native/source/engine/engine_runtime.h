#ifndef JIP_ENGINE_RUNTIME_H
#define JIP_ENGINE_RUNTIME_H
#include "jip_engine.h"
#include <setjmp.h>
#include <stdint.h>
#include <stddef.h>
typedef union jip_allocation jip_allocation;
typedef struct {jip_allocation *head;uint64_t bytes,blocks;} jip_arena;
typedef struct {
  void (*cleanup)(void*);
  void *object;
} jip_resource;
typedef struct jip_operation {
  jmp_buf recover;
  struct jip_operation *previous;
  jip_arena scratch,*arena;
  jip_error error;
  const char *phase;
  uint64_t allocations,polls,fail_allocation,fail_poll,peak_new_bytes;
  int injected_error;
  void (*injected_legacy_failure)(void);
  int (*cancelled)(void*);
  void *cancel_user,*handle;
  jip_resource resources[16];
} jip_operation;
jip_operation *jip_runtime_begin(const char *phase);
void jip_runtime_end(jip_operation*);
void jip_runtime_publish(const jip_operation*);
void jip_arena_clear(jip_arena*);
void jip_arena_move(jip_arena*,jip_arena*);
void *jip_runtime_handle_new(size_t);
void jip_runtime_handle_free(void*);
void *jip_runtime_malloc(size_t);
void *jip_runtime_calloc(size_t,size_t);
void *jip_runtime_realloc(void*,size_t);
void jip_runtime_free(void*);
_Noreturn void jip_runtime_raise(jip_status,const char*,size_t);
_Noreturn void jip_runtime_exit(int);
void jip_runtime_poll(void);
void jip_runtime_phase(const char*);
int jip_runtime_defer(void*,void(*)(void*));
void jip_runtime_forget(int);
#endif
