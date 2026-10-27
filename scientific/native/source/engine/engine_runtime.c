#include "engine_runtime.h"
#include "jip_engine_test.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdatomic.h>
#include <limits.h>
#define JIP_MEMORY_FAILURE(s) jip_runtime_raise(JIP_MEMORY,s,0)
#include "jip-memory.h"
union jip_allocation {
  struct {jip_allocation *previous,*next; jip_arena *owner;size_t bytes;uint64_t magic;} h;
  max_align_t alignment;
};
#define ALLOCATION_MAGIC UINT64_C(0x4a49504152454e41)
static _Thread_local jip_operation *current;
static _Thread_local struct {uint64_t allocation,poll;jip_status kind;} next_fault;
static _Thread_local jip_test_operation last;
static _Atomic uint64_t live_blocks,live_bytes,live_handles,live_resources;
void jip_engine_test_fault(uint64_t allocation,uint64_t poll,jip_status kind) {
  next_fault.allocation=allocation;next_fault.poll=poll;next_fault.kind=kind;
}
jip_test_operation jip_engine_test_last_operation(void){return last;}
jip_test_operation jip_engine_test_current_operation(void){return current?(jip_test_operation){current->allocations,current->polls,current->error.code,current->peak_new_bytes}:(jip_test_operation){0};}
void jip_engine_test_live(uint64_t *blocks,uint64_t *bytes,uint64_t *handles,uint64_t *resources) {
  if(blocks)*blocks=atomic_load(&live_blocks);if(bytes)*bytes=atomic_load(&live_bytes);if(handles)*handles=atomic_load(&live_handles);
  if(resources)*resources=atomic_load(&live_resources);
}
jip_operation *jip_runtime_begin(const char *phase) {
  uint64_t allocation=next_fault.allocation,poll=next_fault.poll;jip_status kind=next_fault.kind;
  memset(&next_fault,0,sizeof(next_fault));
  last=(jip_test_operation){1,0,JIP_MEMORY};
  if(allocation==1)return NULL;
  jip_operation *op=calloc(1,sizeof(*op));if(!op)return NULL;
  atomic_fetch_add(&live_handles,1);op->previous=current;op->arena=&op->scratch;
  op->allocations=1;op->phase=phase;op->fail_allocation=allocation;op->fail_poll=poll;op->injected_error=kind;
  current=op;return op;
}
void jip_runtime_publish(const jip_operation *op) {last=(jip_test_operation){op->allocations,op->polls,op->error.code,op->peak_new_bytes};}
static void unlink_allocation(jip_allocation *b) {
  jip_arena *a=b->h.owner;
  if(b->h.previous)b->h.previous->h.next=b->h.next;else a->head=b->h.next;
  if(b->h.next)b->h.next->h.previous=b->h.previous;
  a->blocks--;a->bytes-=b->h.bytes;atomic_fetch_sub(&live_blocks,1);atomic_fetch_sub(&live_bytes,b->h.bytes);
  b->h.magic=0;free(b);
}
void jip_arena_clear(jip_arena *a) {while(a->head)unlink_allocation(a->head);}
void jip_arena_move(jip_arena *to,jip_arena *from) {
  if(!from->head)return;
  jip_allocation *last_node=from->head;
  for(jip_allocation *b=from->head;b;b=b->h.next){b->h.owner=to;last_node=b;}
  last_node->h.next=to->head;if(to->head)to->head->h.previous=last_node;
  to->head=from->head;to->blocks+=from->blocks;to->bytes+=from->bytes;
  memset(from,0,sizeof(*from));
}
int jip_runtime_defer(void *object,void(*cleanup)(void*)) {
  if(!current)abort();
  for(int i=0;i<16;i++)if(!current->resources[i].cleanup){current->resources[i]=(jip_resource){cleanup,object};atomic_fetch_add(&live_resources,1);return i;}
  cleanup(object);jip_runtime_raise(JIP_INTERNAL,"engine resource ledger exhausted",0);
}
void jip_runtime_forget(int slot) {if(slot>=0&&current&&current->resources[slot].cleanup){current->resources[slot]=(jip_resource){0};atomic_fetch_sub(&live_resources,1);}}
void jip_runtime_end(jip_operation *op) {
  /* Resource cleanups run while their tracked buffers still exist. They never
     poll, allocate, or throw. Then scratch cleanup is allocation-free. */
  for(int i=15;i>=0;i--)if(op->resources[i].cleanup){op->resources[i].cleanup(op->resources[i].object);atomic_fetch_sub(&live_resources,1);}
  jip_arena_clear(&op->scratch);jip_runtime_publish(op);current=op->previous;
  atomic_fetch_sub(&live_handles,1);free(op);
}
_Noreturn void jip_runtime_raise(jip_status code,const char *message,size_t bytes) {
  if(!current){fprintf(stderr,"Unscoped private JIP helper failure: %s\n",message);abort();}
  current->error.code=code;current->error.requested_bytes=bytes;current->error.allocation_index=current->allocations;
  snprintf(current->error.phase,sizeof(current->error.phase),"%s",current->phase?current->phase:"");
  snprintf(current->error.message,sizeof(current->error.message),"%s",message);
  longjmp(current->recover,1);
}
_Noreturn void jip_runtime_exit(int status) {
  (void)status;jip_runtime_raise(JIP_INTERNAL,"legacy numerical helper reported an internal error",0);
}
void jip_runtime_phase(const char *phase) {if(current)current->phase=phase;}
void jip_runtime_poll(void) {
  if(!current)return;
  current->polls++;
  if(current->cancelled&&current->cancelled(current->cancel_user))jip_runtime_raise(JIP_CANCELLED,"registration cancelled; accepted result preserved",0);
  if(current->fail_poll&&current->polls==current->fail_poll) {
    /* Exercise the actual checked-size and mapped legacy-exit paths. */
    if(current->injected_error==JIP_INTERNAL) {
      if(current->injected_legacy_failure)current->injected_legacy_failure();
      jip_runtime_exit(1);
    }
    (void)jip_product(-1,1);
  }
}
static void allocation_point(size_t bytes) {
  if(!current)abort();jip_runtime_poll();current->allocations++;
  if(current->fail_allocation&&current->allocations==current->fail_allocation)jip_runtime_raise(JIP_MEMORY,"injected allocation failure",bytes);
}
void *jip_runtime_handle_new(size_t bytes) {
  allocation_point(bytes);void *p=calloc(1,bytes);
  if(!p)jip_runtime_raise(JIP_MEMORY,"cannot allocate engine context",bytes);
  atomic_fetch_add(&live_handles,1);return p;
}
void jip_runtime_handle_free(void *p) {if(p){atomic_fetch_sub(&live_handles,1);free(p);}}
void *jip_runtime_malloc(size_t bytes) {
  allocation_point(bytes);
  if(bytes>(size_t)PTRDIFF_MAX-sizeof(jip_allocation))jip_runtime_raise(JIP_MEMORY,"allocation exceeds addressable object range",bytes);
  jip_allocation *b=malloc(sizeof(*b)+(bytes?bytes:1));
  if(!b)jip_runtime_raise(JIP_MEMORY,"cannot allocate registration buffer",bytes);
  jip_arena *a=current->arena;
  b->h.previous=NULL;b->h.next=a->head;b->h.owner=a;b->h.bytes=bytes;b->h.magic=ALLOCATION_MAGIC;
  if(a->head)a->head->h.previous=b;a->head=b;a->blocks++;a->bytes+=bytes;
  uint64_t owned=a->bytes+(a==&current->scratch?0:current->scratch.bytes);
  if(owned>current->peak_new_bytes)current->peak_new_bytes=owned;
  atomic_fetch_add(&live_blocks,1);atomic_fetch_add(&live_bytes,bytes);return b+1;
}
void *jip_runtime_calloc(size_t n,size_t size) {
  if(size&&n>SIZE_MAX/size)jip_runtime_raise(JIP_MEMORY,"overflowing allocation size",0);
  size_t bytes=n*size;void *p=jip_runtime_malloc(bytes);memset(p,0,bytes);return p;
}
void jip_runtime_free(void *p) {
  if(!p)return;jip_allocation *b=(jip_allocation*)p-1;
  if(b->h.magic!=ALLOCATION_MAGIC)abort();
  unlink_allocation(b);
}
void *jip_runtime_realloc(void *p,size_t bytes) {
  if(!p)return jip_runtime_malloc(bytes);
  jip_allocation *b=(jip_allocation*)p-1;if(b->h.magic!=ALLOCATION_MAGIC)abort();
  void *q=jip_runtime_malloc(bytes);memcpy(q,p,bytes<b->h.bytes?bytes:b->h.bytes);jip_runtime_free(p);return q;
}
