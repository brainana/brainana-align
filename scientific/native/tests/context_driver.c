#define main unused_fixture_main
#include "api_driver.c"
#undef main
#include <pthread.h>
#define REQUIRE(x) do {if(!(x)){fprintf(stderr,"Failed at line %d: %s\n",__LINE__,#x);return 1;}}while(0)
typedef struct {jip_engine *current,*other;int called,busy_ok,other_ok;} nested;
static void nesting(const jip_trace *t,void *u) {
  nested *n=u;(void)t;if(n->called)return;n->called=1;
  n->busy_ok=jip_engine_register(n->current)==JIP_BUSY;
  n->other_ok=jip_engine_register(n->other)==JIP_OK;
}
static void *job(void *p) {return (void*)(uintptr_t)jip_engine_register(p);}
static int equal(jip_engine *a,jip_engine *b) {
  jip_image x,y;float c,d;
  return !jip_engine_result(a,&x,&c)&&!jip_engine_result(b,&y,&d)&&c==d&&x.count==y.count&&!memcmp(x.data,y.data,(size_t)x.count*4);
}
int main(void) {
  jip_engine *a=make(0,4),*b=make(1,4),*a_ref=make(0,4),*b_ref=make(1,4);
  REQUIRE(a&&b&&a_ref&&b_ref);
  REQUIRE(!jip_engine_register(a_ref)&&!jip_engine_register(b_ref));
  nested n={a,b,0,0,0};REQUIRE(!jip_engine_set_trace(a,nesting,&n));REQUIRE(!jip_engine_register(a));
  REQUIRE(n.called&&n.busy_ok&&n.other_ok&&equal(a,a_ref)&&equal(b,b_ref));
  jip_engine_set_trace(a,NULL,NULL);
  REQUIRE(!jip_engine_checkpoint_save(a,"context.jipcp"));
  jip_engine *loaded=NULL;REQUIRE(!jip_engine_checkpoint_load("context.jipcp",&loaded));
  REQUIRE(!jip_engine_register(a)&&!jip_engine_register(loaded)&&equal(a,loaded));
  jip_engine_destroy(loaded);jip_engine_destroy(a);jip_engine_destroy(b);
  a=make(0,4);b=make(1,4);REQUIRE(a&&b);pthread_t one,two;void *r1,*r2;
  REQUIRE(!pthread_create(&one,NULL,job,a)&&!pthread_create(&two,NULL,job,b));
  REQUIRE(!pthread_join(one,&r1)&&!pthread_join(two,&r2));
  REQUIRE(!r1&&!r2&&equal(a,a_ref)&&equal(b,b_ref));
  jip_engine_destroy(a);jip_engine_destroy(b);jip_engine_destroy(a_ref);jip_engine_destroy(b_ref);
  for(int i=0;i<50;i++){a=make(1,4);REQUIRE(a);REQUIRE(!jip_engine_register(a));jip_engine_destroy(a);}
  FILE *f=fopen("context.jipcp","r+b");REQUIRE(f);REQUIRE(!fseek(f,-5,SEEK_END));int byte=fgetc(f);REQUIRE(byte!=EOF);REQUIRE(!fseek(f,-1,SEEK_CUR));fputc(byte^1,f);fclose(f);
  loaded=(jip_engine*)(uintptr_t)1;REQUIRE(jip_engine_checkpoint_load("context.jipcp",&loaded)==JIP_INVALID&&!loaded);
  f=fopen("truncated.jipcp","wb");REQUIRE(f);fwrite("JIPCP001",8,1,f);fclose(f);
  REQUIRE(jip_engine_checkpoint_load("truncated.jipcp",&loaded)==JIP_INVALID&&!loaded);
  jip_options o;jip_options_default(&o);jip_image bad={0};REQUIRE(jip_engine_create(&bad,&bad,&o,&loaded)==JIP_INVALID&&!loaded);
  a=make(1,4);REQUIRE(a);jip_image valid;REQUIRE(!jip_engine_result(a,&valid,NULL));
  o.regularization=2147483647;REQUIRE(jip_engine_create(&valid,&valid,&o,&loaded)==JIP_INVALID&&!loaded);jip_engine_destroy(a);
  jip_engine_destroy(NULL);puts("CONTEXT_RESULT nested=exact concurrent=exact continuation=exact soak=50 malformed=rejected");return 0;
}
