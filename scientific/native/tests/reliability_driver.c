#define main unused_api_fixture_main
#include "api_driver.c"
#undef main
#include "jip_engine_test.h"
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"RELIABILITY_FAIL line %d: %s\n",__LINE__,#x);exit(1);}}while(0)
typedef struct {uint64_t blocks,bytes,handles,resources;} usage;
static usage used(void){usage u;jip_engine_test_live(&u.blocks,&u.bytes,&u.handles,&u.resources);return u;}
static void same_usage(usage a){usage b=used();CHECK(!memcmp(&a,&b,sizeof(a)));}
static unsigned char *file_bytes(const char *name,size_t *size) {
  FILE *f=fopen(name,"rb");CHECK(f);CHECK(!fseek(f,0,SEEK_END));long n=ftell(f);CHECK(n>=0);rewind(f);
  unsigned char *b=malloc((size_t)n);CHECK(b);CHECK(fread(b,1,(size_t)n,f)==(size_t)n);CHECK(!fclose(f));*size=(size_t)n;return b;
}
static int fds(void){int n=0;for(int i=0;i<256;i++)if(fcntl(i,F_GETFD)!=-1)n++;return n;}
typedef struct {jip_image view;float data[729],objective;unsigned char *checkpoint;size_t bytes;usage memory;int descriptors;} accepted;
static accepted capture(jip_engine *c) {
  accepted a={0};CHECK(!jip_engine_result(c,&a.view,&a.objective));memcpy(a.data,a.view.data,sizeof(a.data));
  CHECK(!jip_engine_checkpoint_save(c,"accepted.jipcp"));a.checkpoint=file_bytes("accepted.jipcp",&a.bytes);a.memory=used();a.descriptors=fds();return a;
}
static void unchanged(jip_engine *c,const accepted *a) {
  jip_image im;float objective;CHECK(!jip_engine_result(c,&im,&objective));
  CHECK(im.data==a->view.data&&objective==a->objective&&!memcmp(a->data,im.data,sizeof(a->data)));
  CHECK(!jip_engine_checkpoint_save(c,"after.jipcp"));size_t n;unsigned char *b=file_bytes("after.jipcp",&n);
  CHECK(n==a->bytes&&!memcmp(b,a->checkpoint,n));free(b);same_usage(a->memory);CHECK(fds()==a->descriptors);
}
typedef struct {jip_engine *c;int event,occurrence,seen,cancelled;uint64_t first_rebuild,first_basis,first_pass,last_allocations;} observer;
static void observe(const jip_trace *t,void *user) {
  observer *o=user;uint64_t at=jip_engine_test_current_operation().allocations;
  if(t->event==3&&!o->first_rebuild)o->first_rebuild=at;
  if(t->event==1&&!o->first_basis)o->first_basis=at;
  if(t->event==2&&!o->first_pass)o->first_pass=at;
  o->last_allocations=at;
  if(t->event==o->event&&++o->seen==o->occurrence) {
    CHECK(jip_engine_release(o->c)==JIP_BUSY);
    CHECK(jip_engine_request_cancel(o->c)==JIP_OK);CHECK(jip_engine_request_cancel(o->c)==JIP_OK);o->cancelled=1;
  }
}
static void error_is(jip_engine *c,jip_status code) {
  jip_error e;CHECK(!jip_engine_get_error(c,&e));CHECK(e.code==code&&e.phase[0]&&e.message[0]);
}
static void equal_results(jip_engine *a,jip_engine *b) {
  jip_image x,y;float u,v;CHECK(!jip_engine_result(a,&x,&u)&&!jip_engine_result(b,&y,&v));CHECK(u==v&&!memcmp(x.data,y.data,729*4));
}
typedef struct {pthread_mutex_t mutex;pthread_cond_t cv;int ready,done; jip_engine *c;} cancel_thread;
static void pause_for_cancel(const jip_trace *t,void *user) {
  cancel_thread *s=user;if(t->event!=1||s->ready)return;
  CHECK(!pthread_mutex_lock(&s->mutex));s->ready=1;CHECK(!pthread_cond_broadcast(&s->cv));
  while(!s->done)CHECK(!pthread_cond_wait(&s->cv,&s->mutex));CHECK(!pthread_mutex_unlock(&s->mutex));
}
static void *cancel_worker(void *user) {
  cancel_thread *s=user;CHECK(!pthread_mutex_lock(&s->mutex));while(!s->ready)CHECK(!pthread_cond_wait(&s->cv,&s->mutex));
  CHECK(jip_engine_release(s->c)==JIP_BUSY);CHECK(jip_engine_request_cancel(s->c)==JIP_OK);s->done=1;
  CHECK(!pthread_cond_broadcast(&s->cv));CHECK(!pthread_mutex_unlock(&s->mutex));return NULL;
}
typedef struct {jip_engine *other;int fired;} nested_failure;
static void fail_nested(const jip_trace *t,void *user) {
  nested_failure *s=user;if(t->event!=1||s->fired)return;s->fired=1;
  jip_engine_test_fault(5,0,JIP_MEMORY);CHECK(jip_engine_register(s->other)==JIP_MEMORY);
}
typedef struct {jip_engine *c;int fail;jip_status status;} parallel_job;
static void *parallel_worker(void *user) {
  parallel_job *job=user;if(job->fail)jip_engine_test_fault(5,0,JIP_MEMORY);
  job->status=jip_engine_register(job->c);return NULL;
}
int main(void) {
  usage empty=used();CHECK(!empty.blocks&&!empty.bytes&&!empty.handles&&!empty.resources);
  jip_engine *c=make(1,4);CHECK(c);uint64_t create_allocations=jip_engine_test_last_operation().allocations;
  jip_engine_destroy(c);same_usage(empty);
  for(uint64_t i=1;i<=create_allocations;i++) {
    jip_engine_test_fault(i,0,JIP_MEMORY);c=make(1,4);CHECK(!c);
    CHECK(jip_engine_test_last_operation().status==JIP_MEMORY);same_usage(empty);
  }
  c=make(1,4);jip_engine *reference=make(1,4);CHECK(c&&reference);
  observer reference_trace={.c=reference};CHECK(!jip_engine_set_trace(reference,observe,&reference_trace));
  CHECK(!jip_engine_register(reference));uint64_t total=jip_engine_test_last_operation().allocations;
  uint64_t trial_peak=jip_engine_test_last_operation().peak_new_bytes;
  accepted initial=capture(c);uint64_t registration_failures=0;
  /* Every allocation through clone and the first hierarchy rebuild, then
     failures after real coefficient/pass work and near the final allocation. */
  for(uint64_t i=1;i<=reference_trace.first_rebuild;i++) {
    jip_engine_test_fault(i,0,JIP_MEMORY);CHECK(jip_engine_register(c)==JIP_MEMORY);error_is(c,JIP_MEMORY);unchanged(c,&initial);registration_failures++;
  }
  uint64_t later[]={reference_trace.first_basis,reference_trace.first_basis+1,reference_trace.first_pass,reference_trace.first_pass+1,total-2,total-1,total};
  for(size_t i=0;i<sizeof(later)/sizeof(*later);i++) {
    CHECK(later[i]>0&&later[i]<=total);jip_engine_test_fault(later[i],0,JIP_MEMORY);
    CHECK(jip_engine_register(c)==JIP_MEMORY);error_is(c,JIP_MEMORY);unchanged(c,&initial);registration_failures++;
  }
  for(int kind=JIP_MEMORY;kind<=JIP_INTERNAL;kind++) {
    jip_engine_test_fault(0,4,(jip_status)kind);CHECK(jip_engine_register(c)==kind);error_is(c,(jip_status)kind);unchanged(c,&initial);
  }
  for(int event=1;event<=4;event++) {
    observer o={.c=c,.event=event,.occurrence=1};CHECK(!jip_engine_set_trace(c,observe,&o));
    CHECK(jip_engine_register(c)==JIP_CANCELLED&&o.cancelled);error_is(c,JIP_CANCELLED);unchanged(c,&initial);
  }
  observer o={.c=c,.event=1,.occurrence=20};CHECK(!jip_engine_set_trace(c,observe,&o));
  CHECK(jip_engine_register(c)==JIP_CANCELLED&&o.cancelled);unchanged(c,&initial);
  cancel_thread sync={PTHREAD_MUTEX_INITIALIZER,PTHREAD_COND_INITIALIZER,0,0,c};pthread_t thread;
  CHECK(!jip_engine_set_trace(c,pause_for_cancel,&sync));CHECK(!pthread_create(&thread,NULL,cancel_worker,&sync));
  CHECK(jip_engine_register(c)==JIP_CANCELLED);CHECK(!pthread_join(thread,NULL));unchanged(c,&initial);
  CHECK(!pthread_mutex_destroy(&sync.mutex)&&!pthread_cond_destroy(&sync.cv));
  CHECK(!jip_engine_set_trace(c,NULL,NULL));CHECK(!jip_engine_register(c));equal_results(c,reference);
  CHECK(jip_engine_request_cancel(c)==JIP_INVALID);jip_error cleared;CHECK(!jip_engine_get_error(c,&cleared)&&cleared.code==JIP_OK);
  free(initial.checkpoint);accepted successful=capture(c);
  jip_engine *continued=make(1,4);CHECK(continued);CHECK(!jip_engine_register(continued));
  observer continued_trace={.c=continued};CHECK(!jip_engine_set_trace(continued,observe,&continued_trace));CHECK(!jip_engine_register(continued));
  /* The comparison snapshot predates creation of this extra reference job. */
  successful.memory=used();
  observer repeat_cancel={.c=c,.event=1,.occurrence=20};CHECK(!jip_engine_set_trace(c,observe,&repeat_cancel));
  CHECK(jip_engine_register(c)==JIP_CANCELLED&&repeat_cancel.cancelled);unchanged(c,&successful);
  CHECK(!jip_engine_set_trace(c,NULL,NULL));jip_engine_test_fault(continued_trace.first_basis,0,JIP_MEMORY);
  CHECK(jip_engine_register(c)==JIP_MEMORY);unchanged(c,&successful);registration_failures++;
  CHECK(!jip_engine_register(c));equal_results(c,continued);jip_engine_destroy(continued);
  free(successful.checkpoint);successful=capture(c);
  /* All allocation sites in save/export/load, including partial construction
     while the checkpoint file is open. Guarded failure must release its file. */
  CHECK(!jip_engine_checkpoint_save(c,"saved.jipcp"));uint64_t saves=jip_engine_test_last_operation().allocations;
  for(uint64_t i=1;i<=saves;i++){jip_engine_test_fault(i,0,JIP_MEMORY);CHECK(jip_engine_checkpoint_save(c,"saved.jipcp")==JIP_MEMORY);unchanged(c,&successful);}
  CHECK(!jip_engine_export(c,"aligned.nii","field.nii"));uint64_t exports=jip_engine_test_last_operation().allocations;
  for(uint64_t i=1;i<=exports;i++){jip_engine_test_fault(i,0,JIP_MEMORY);CHECK(jip_engine_export(c,"aligned.nii","field.nii")==JIP_MEMORY);unchanged(c,&successful);}
  jip_engine *loaded=NULL;CHECK(!jip_engine_checkpoint_load("saved.jipcp",&loaded));uint64_t loads=jip_engine_test_last_operation().allocations;jip_engine_destroy(loaded);
  for(uint64_t i=1;i<=loads;i++){loaded=NULL;jip_engine_test_fault(i,0,JIP_MEMORY);CHECK(jip_engine_checkpoint_load("saved.jipcp",&loaded)==JIP_MEMORY&&!loaded);unchanged(c,&successful);}
  jip_engine_test_fault(0,3,JIP_INTERNAL);CHECK(jip_engine_checkpoint_save(c,"saved.jipcp")==JIP_INTERNAL);unchanged(c,&successful);
  CHECK(jip_engine_export(c,"missing-parent/output.nii",NULL)==JIP_IO);error_is(c,JIP_IO);unchanged(c,&successful);
  nested_failure nested={c,0};jip_engine *outer=make(1,4);CHECK(outer);CHECK(!jip_engine_set_trace(outer,fail_nested,&nested));
  CHECK(!jip_engine_register(outer)&&nested.fired);equal_results(outer,reference);jip_engine_destroy(outer);unchanged(c,&successful);
  jip_engine *mi=make(0,4),*mi_reference=make(0,4);CHECK(mi&&mi_reference);
  observer mi_trace={.c=mi_reference};CHECK(!jip_engine_set_trace(mi_reference,observe,&mi_trace));CHECK(!jip_engine_register(mi_reference));
  accepted mi_accepted=capture(mi);observer mi_cancel={.c=mi,.event=1,.occurrence=10};CHECK(!jip_engine_set_trace(mi,observe,&mi_cancel));
  CHECK(jip_engine_register(mi)==JIP_CANCELLED&&mi_cancel.cancelled);unchanged(mi,&mi_accepted);CHECK(!jip_engine_set_trace(mi,NULL,NULL));
  jip_engine_test_fault(mi_trace.first_pass,0,JIP_MEMORY);CHECK(jip_engine_register(mi)==JIP_MEMORY);unchanged(mi,&mi_accepted);registration_failures++;
  CHECK(!jip_engine_register(mi));equal_results(mi,mi_reference);free(mi_accepted.checkpoint);
  parallel_job a={make(0,4),0,JIP_INVALID},b={make(1,4),1,JIP_INVALID};CHECK(a.c&&b.c);accepted b_accepted=capture(b.c);
  pthread_t one,two;CHECK(!pthread_create(&one,NULL,parallel_worker,&a)&&!pthread_create(&two,NULL,parallel_worker,&b));
  CHECK(!pthread_join(one,NULL)&&!pthread_join(two,NULL));CHECK(a.status==JIP_OK&&b.status==JIP_MEMORY);equal_results(a.c,mi_reference);
  b_accepted.memory=used();unchanged(b.c,&b_accepted);free(b_accepted.checkpoint);jip_engine_destroy(a.c);jip_engine_destroy(b.c);
  jip_engine_destroy(mi);jip_engine_destroy(mi_reference);
  free(successful.checkpoint);jip_engine_destroy(c);jip_engine_destroy(reference);same_usage(empty);
  /* Reuse one context, rather than destroying it after each fit: this caught
     the legacy invert_mat44 temporary-matrix leak. The basis shape is stable
     once the initial hierarchy has completed, so owned bytes must plateau. */
  c=make(1,4);CHECK(c);CHECK(!jip_engine_register(c));usage plateau=used();
  for(int i=0;i<10;i++){CHECK(!jip_engine_register(c));same_usage(plateau);}
  jip_engine_destroy(c);same_usage(empty);
  printf("RELIABILITY_RESULT create_faults=%llu registration_faults=%llu save_faults=%llu export_faults=%llu load_faults=%llu cancellations=8 rollback=byte_exact resources=zero trial_payload_peak=%llu\n",(unsigned long long)create_allocations,(unsigned long long)registration_failures,(unsigned long long)saves,(unsigned long long)exports,(unsigned long long)loads,(unsigned long long)trial_peak);
  return 0;
}
