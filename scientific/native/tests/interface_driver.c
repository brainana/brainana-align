#define main old_api_main
#include "api_driver.c"
#undef main
#include "jip_engine_test.h"
#define CHECK(x) do{if(!(x)){fprintf(stderr,"INTERFACE_FAIL line %d: %s\n",__LINE__,#x);exit(1);}}while(0)
static jip_engine *configured(int model,int metric,int both) {
  float data[729];for(int z=0;z<9;z++)for(int y=0;y<9;y++)for(int x=0;x<9;x++)data[(z*9+y)*9+x]=phantom_value(x,y,z);
  jip_image im={{9,9,9},{1,1.5,2},{1,0,0,0,0,1.5,0,0,0,0,2,0,0,0,0,1},data,729};
  jip_options o;jip_options_default(&o);o.metric=1;o.grid_down=4;o.regularization=1;o.affine_parameters[0]=.8;
  jip_config config;jip_config_default(&config);config.affine_model=model;config.affine_metric=metric;config.stages=both?3:1;
  jip_engine *c=NULL;CHECK(!jip_engine_create_configured(&im,&im,&o,&config,&c));return c;
}
static void write_state(jip_engine *c,const char *path) {
  jip_transform tr;jip_image im;CHECK(!jip_engine_get_transform(c,&tr)&&!jip_engine_result(c,&im,NULL));
  FILE *f=fopen(path,"wb");CHECK(f);fwrite(tr.affine_parameters,4,12,f);fwrite(tr.centered_pull_affine,4,16,f);
  fwrite(&tr.affine_objective,4,1,f);fwrite(&tr.nonlinear_objective,4,1,f);fwrite(im.data,4,729,f);
  for(int a=0;a<3;a++)fwrite(tr.displacement[a],4,729,f);CHECK(!fclose(f));
}
static unsigned char *read_file(const char *path,size_t *size) {
  FILE *f=fopen(path,"rb");CHECK(f);CHECK(!fseek(f,0,SEEK_END));long n=ftell(f);CHECK(n>=0);rewind(f);
  unsigned char *bytes=malloc((size_t)n);CHECK(bytes&&fread(bytes,1,(size_t)n,f)==(size_t)n);CHECK(!fclose(f));*size=(size_t)n;return bytes;
}
static void same_file_bytes(const char *a,const char *b) {size_t x,y;unsigned char *p=read_file(a,&x),*q=read_file(b,&y);CHECK(x==y&&!memcmp(p,q,x));free(p);free(q);}
/* Immutable macOS-arm64 references remain byte exact there. Cross-platform
   comparisons bound float rounding; all same-host rollback/restore stays exact. */
static void same_reference(const char *a,const char *b) {
 if(!getenv("JIP_PORTABLE_REFERENCE")){same_file_bytes(a,b);return;}
 size_t x,y;unsigned char *p=read_file(a,&x),*q=read_file(b,&y);CHECK(x==y&&x%4==0);
 float *expected=(float*)p,*actual=(float*)q;double maximum=0;
 for(size_t i=0;i<x/4;i++) {double error=fabs((double)expected[i]-actual[i]);
  double tolerance=(i<30?1e-6:1e-4)+1e-6*fabs((double)expected[i]);
  CHECK(isfinite(expected[i])&&isfinite(actual[i])&&error<=tolerance);
  if(error>maximum)maximum=error;
 }
 printf("PORTABLE_REFERENCE %s max_abs=%.9g\n",a,maximum);free(p);free(q);
}
typedef struct{jip_engine *context;int event,done;} cancellation;
static void cancel_stage(const jip_trace *t,void *user) {
  cancellation *cancel=user;
  if(!cancel->done&&t->event==cancel->event){cancel->done=1;CHECK(!jip_engine_request_cancel(cancel->context));}
}
int main(int argc,char **argv) {
  CHECK(argc==2);const int cases[][3]={{6,0,0},{9,0,0},{12,0,0},{6,1,0},{12,1,0},{6,0,1}};
  for(size_t i=0;i<sizeof(cases)/sizeof(*cases);i++) {
    int model=cases[i][0],metric=cases[i][1],both=cases[i][2];jip_engine *c=configured(model,metric,both);
    CHECK(!jip_engine_register(c));char path[2048];snprintf(path,sizeof(path),"%s/model-%d-metric-%d-both-%d.bin",argv[1],model,metric,both);
    write_state(c,"actual.bin");same_reference(path,"actual.bin");
    jip_transform tr;CHECK(!jip_engine_get_transform(c,&tr)&&tr.completed_stages==(uint32_t)(both?3:1));
    if(model==6)for(int a=6;a<12;a++)CHECK(tr.affine_parameters[a]==(a<9?1:0));
    if(model==9)for(int a=9;a<12;a++)CHECK(tr.affine_parameters[a]==0);
    CHECK(!jip_engine_checkpoint_save(c,"configured.jipcp"));jip_engine *restored=NULL;CHECK(!jip_engine_checkpoint_load("configured.jipcp",&restored));
    write_state(restored,"restored.bin");same_file_bytes("actual.bin","restored.bin");jip_config first,second;
    CHECK(!jip_engine_get_config(c,&first)&&!jip_engine_get_config(restored,&second)&&!memcmp(&first,&second,sizeof(first)));
    CHECK(!jip_engine_register(c)&&!jip_engine_register(restored));write_state(c,"actual.bin");write_state(restored,"restored.bin");same_file_bytes("actual.bin","restored.bin");
    jip_engine_destroy(restored);jip_engine_destroy(c);
  }
  /* Separate stage calls and a checkpoint between stages must equal a single
     transaction that runs both stages, apart from checkpoint header history. */
  jip_engine *joint=configured(6,0,1),*separate=configured(6,0,1);
  CHECK(!jip_engine_register(joint)&&!jip_engine_run(separate,JIP_STAGE_AFFINE));
  CHECK(!jip_engine_checkpoint_save(separate,"between-stages.jipcp"));jip_engine *between=NULL;
  CHECK(!jip_engine_checkpoint_load("between-stages.jipcp",&between));
  CHECK(!jip_engine_run(separate,JIP_STAGE_NONLINEAR)&&!jip_engine_run(between,JIP_STAGE_NONLINEAR));
  write_state(joint,"joint.bin");write_state(separate,"separate.bin");write_state(between,"between.bin");
  same_file_bytes("joint.bin","separate.bin");same_file_bytes("joint.bin","between.bin");
  jip_engine_destroy(joint);jip_engine_destroy(separate);jip_engine_destroy(between);
  jip_engine *c=configured(6,0,1);CHECK(!jip_engine_checkpoint_save(c,"before.jipcp"));
  for(int event=5;event<=6;event++) {
    cancellation cancel={c,event,0};CHECK(!jip_engine_set_trace(c,cancel_stage,&cancel));
    CHECK(jip_engine_register(c)==JIP_CANCELLED&&cancel.done);CHECK(!jip_engine_checkpoint_save(c,"after.jipcp"));same_file_bytes("before.jipcp","after.jipcp");
  }
  CHECK(!jip_engine_set_trace(c,NULL,NULL));
  jip_engine *reference=configured(6,0,1);CHECK(!jip_engine_register(reference));uint64_t allocations=jip_engine_test_last_operation().allocations;
  uint64_t failures[]={1,2,5,30,allocations/2,allocations-1,allocations};
  for(size_t i=0;i<sizeof(failures)/sizeof(*failures);i++) {
    jip_engine_test_fault(failures[i],0,JIP_MEMORY);CHECK(jip_engine_register(c)==JIP_MEMORY);
    CHECK(!jip_engine_checkpoint_save(c,"after.jipcp"));same_file_bytes("before.jipcp","after.jipcp");
  }
  CHECK(!jip_engine_register(c));write_state(c,"actual.bin");write_state(reference,"restored.bin");same_file_bytes("actual.bin","restored.bin");
  CHECK(jip_engine_run(c,0)==JIP_INVALID&&jip_engine_run(c,4)==JIP_INVALID);jip_engine_destroy(reference);jip_engine_destroy(c);
  c=make(1,4);jip_image valid;CHECK(c&&!jip_engine_result(c,&valid,NULL));jip_options options;jip_options_default(&options);
  jip_config config;jip_config_default(&config);jip_engine *bad=NULL;
  config.struct_size--;CHECK(jip_engine_create_configured(&valid,&valid,&options,&config,&bad)==JIP_INVALID&&!bad);
  jip_config_default(&config);config.affine_model=7;CHECK(jip_engine_create_configured(&valid,&valid,&options,&config,&bad)==JIP_INVALID&&!bad);
  jip_config_default(&config);config.nonlinear_axes[0]=config.nonlinear_axes[1]=config.nonlinear_axes[2]=0;
  CHECK(jip_engine_create_configured(&valid,&valid,&options,&config,&bad)==JIP_INVALID&&!bad);
  jip_config_default(&config);config.stages=1;config.nonlinear_axes[0]=config.nonlinear_axes[1]=config.nonlinear_axes[2]=0;
  CHECK(!jip_engine_create_configured(&valid,&valid,&options,&config,&bad));CHECK(jip_engine_run(bad,2)==JIP_INVALID);jip_engine_destroy(bad);
  jip_config_default(&config);config.nonlinear_axes[0]=config.nonlinear_axes[1]=0;config.wrap_around[1]=1;config.matching_volume=1;
  CHECK(!jip_engine_create_configured(&valid,&valid,&options,&config,&bad));CHECK(!jip_engine_register(bad));
  jip_transform tr;CHECK(!jip_engine_get_transform(bad,&tr));for(uint64_t i=0;i<tr.count;i++)CHECK(tr.displacement[0][i]==0&&tr.displacement[1][i]==0);
  CHECK(!jip_engine_checkpoint_save(bad,"axes.jipcp"));jip_engine *restored=NULL;CHECK(!jip_engine_checkpoint_load("axes.jipcp",&restored));jip_config roundtrip;
  CHECK(!jip_engine_get_config(restored,&roundtrip)&&!memcmp(&config,&roundtrip,sizeof(config)));
  jip_engine_destroy(restored);jip_engine_destroy(bad);jip_engine_destroy(c);
  /* Legacy convergence divides by its initial information score. A zero
     overlap/score must return a recoverable error rather than loop on NaN. */
  c=make(1,4);CHECK(c&&!jip_engine_result(c,&valid,NULL));jip_options_default(&options);options.metric=1;options.affine_parameters[0]=1000;
  jip_config_default(&config);config.stages=JIP_STAGE_AFFINE;
  CHECK(!jip_engine_create_configured(&valid,&valid,&options,&config,&bad));
  CHECK(!jip_engine_checkpoint_save(bad,"no-overlap-before.jipcp"));CHECK(jip_engine_register(bad)==JIP_INVALID);
  CHECK(!jip_engine_checkpoint_save(bad,"no-overlap-after.jipcp"));same_file_bytes("no-overlap-before.jipcp","no-overlap-after.jipcp");
  jip_engine_destroy(bad);jip_engine_destroy(c);
  uint64_t blocks,bytes,handles,resources;jip_engine_test_live(&blocks,&bytes,&handles,&resources);CHECK(!blocks&&!bytes&&!handles&&!resources);
  puts("INTERFACE_RESULT references=6 platform_policy_checked configured_restore_continue=exact affine_cancel_rollback=exact failures=7 axes_matching_wrap=pass resources=zero");return 0;
}
