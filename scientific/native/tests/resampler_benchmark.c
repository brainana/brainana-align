/* Pure-operation benchmark; same-process frozen 0.4.0 versus current sampler.
   Synthetic arrays, no medical data and no optimizer changes. */
#include "../source/engine/engine.c"
#include "jip_engine_test.h"
#include <time.h>
#if defined(__APPLE__) || defined(__linux__)
#include <sys/resource.h>
#endif
#undef transform_coordinates
#define transform_coordinates reference_transform_coordinates
#include "fixtures/resampler-0.4.0.inc"
#undef transform_coordinates
#define transform_coordinates jip_private_transform_coordinates
#undef malloc
#undef calloc
#undef free
#undef exit
#define REQUIRE(x) do {if(!(x)){fprintf(stderr,"RESAMPLER_BENCH_FAIL %d: %s\n",__LINE__,#x);exit(1);}}while(0)
static double now(void) {struct timespec t;timespec_get(&t,TIME_UTC);return (double)t.tv_sec+t.tv_nsec*1.e-9;}
int main(int argc,char **argv) {
  REQUIRE(argc==2);int kind=atoi(argv[1]);REQUIRE(kind>=0&&kind<5);
  int dims[3]={32,36,28},mdims[3]={36,40,32};float spacing[3]={1,1,1},mspacing[3]={1,1,1};
  if(kind==1){dims[0]=96;dims[1]=112;dims[2]=80;mdims[0]=112;mdims[1]=104;mdims[2]=96;mspacing[1]=.9f;mspacing[2]=1.1f;}
  if(kind==2||kind==4){dims[0]=160;dims[1]=176;dims[2]=144;mdims[0]=176;mdims[1]=160;mdims[2]=160;spacing[1]=.8f;spacing[2]=1.2f;}
  if(kind==3){for(int a=0;a<3;a++){dims[a]=32;mdims[a]=96;spacing[a]=3.25f;}}
  jip_image fixed={0},moving={0};fixed.count=moving.count=1;
  for(int a=0;a<3;a++){fixed.dim[a]=dims[a];moving.dim[a]=mdims[a];fixed.spacing[a]=spacing[a];moving.spacing[a]=mspacing[a];fixed.count*=(uint32_t)dims[a];moving.count*=(uint32_t)mdims[a];fixed.native_affine[a*4+a]=spacing[a];moving.native_affine[a*4+a]=mspacing[a];}
  fixed.native_affine[15]=moving.native_affine[15]=1;
  float *f=malloc((size_t)fixed.count*sizeof(float)),*m=malloc((size_t)moving.count*sizeof(float));REQUIRE(f&&m);
  for(uint64_t i=0;i<fixed.count;i++)f[i]=(float)(10.+3.*sin((double)i*.0013));
  for(uint64_t i=0;i<moving.count;i++)m[i]=(float)(10.+4.*sin((double)i*.0011)+2.*cos((double)i*.00017));
  fixed.data=f;moving.data=m;
  jip_options options;jip_options_default(&options);options.metric=1;options.grid_down=8;
  options.affine_parameters[0]=.37f;options.affine_parameters[1]=-.63f;options.affine_parameters[2]=.41f;
  options.affine_parameters[3]=3;options.affine_parameters[4]=-2;options.affine_parameters[5]=4;
  jip_config config;jip_config_default(&config);config.matching_volume=1;
  jip_engine *c=NULL;REQUIRE(!jip_engine_create_configured(&fixed,&moving,&options,&config,&c));
  jip_operation *op=jip_runtime_begin("resampler operation benchmark");REQUIRE(op);op->arena=&c->arena;jip_engine *previous=jip_active;jip_active=c;
  if(setjmp(op->recover)){fprintf(stderr,"Benchmark error: %s\n",op->error.message);return 1;}
  int all=kind!=4,inner=kind==4?20:1;
  if(kind==4){AlignNL.iBasis=0;AlignNL.Index3DStart[0]=(IPOINT3D){16,16,16};AlignNL.Index3DEnd[0]=(IPOINT3D){40,40,40};}
  size_t bytes=(size_t)target.n3d*sizeof(float);float *expected=malloc(bytes);REQUIRE(expected);
  /* Warm each route once; then alternate measured order. Full output is
     compared outside timing, including untouched local-update voxels. */
  reference_transform_coordinates(all,0,1);transform_coordinates(all,0,1);
  printf("{\"kind\":%d,\"target_voxels\":%llu,\"source_voxels\":%llu,\"inner_calls\":%d,\"runs\":[",kind,(unsigned long long)fixed.count,(unsigned long long)moving.count,inner);
  for(int repeat=0;repeat<3;repeat++) {
    for(int order=0;order<2;order++) {
      int variant=(order+repeat)%2;
      double start=now();clock_t cpu=clock();
      for(int n=0;n<inner;n++) {
        if(variant)transform_coordinates(all,0,1);else reference_transform_coordinates(all,0,1);
      }
      double seconds=now()-start,cpu_seconds=(double)(clock()-cpu)/CLOCKS_PER_SEC;
      if(order==0)memcpy(expected,mimic.SpaceTime.fData,bytes);
      else REQUIRE(!memcmp(expected,mimic.SpaceTime.fData,bytes));
      printf("%s{\"variant\":\"%s\",\"repeat\":%d,\"seconds\":%.9g,\"cpu_seconds\":%.9g}",repeat||order?",":"",variant?"candidate":"reference",repeat,seconds,cpu_seconds);
    }
  }
  free(expected);jip_active=previous;jip_runtime_end(op);jip_engine_destroy(c);free(f);free(m);
  uint64_t blocks,owned,handles,resources;jip_engine_test_live(&blocks,&owned,&handles,&resources);REQUIRE(!blocks&&!owned&&!handles&&!resources);
  uint64_t rss=0;
#if defined(__APPLE__) || defined(__linux__)
  struct rusage usage;REQUIRE(!getrusage(RUSAGE_SELF,&usage));rss=(uint64_t)usage.ru_maxrss;
#ifndef __APPLE__
  rss*=1024;
#endif
#endif
  printf("],\"parity\":\"byte-exact\",\"remaining_ownership\":0,\"process_peak_rss_bytes\":%llu}\n",(unsigned long long)rss);
  return 0;
}
