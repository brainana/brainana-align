/* Direct differential test; no additional API or alternate production sampler.
   Include the engine only in this dedicated test executable, then compile the
   frozen 0.4.0 resampler under a separate name against identical helper math. */
#include "../source/engine/engine.c"
#include "jip_engine_test.h"
#undef transform_coordinates
#define transform_coordinates reference_transform_coordinates
#include "fixtures/resampler-0.4.0.inc"
#undef transform_coordinates
#define transform_coordinates jip_private_transform_coordinates
#undef malloc
#undef calloc
#undef free
#undef exit

#define CHECK(x) do {if(!(x)){fprintf(stderr,"RESAMPLER_FAIL line %d case=%d wrap=%d mask=%d accurate=%d region=%d: %s\n",__LINE__,kind,wrap,matching,accurate,region,#x);exit(1);}}while(0)
int main(void) {
  int kind=-1,wrap=-1,matching=-1,accurate=-1,region=-1,cases=0;
  for(kind=0;kind<8;kind++)for(wrap=0;wrap<8;wrap++)for(matching=0;matching<2;matching++) {
    jip_image fixed={{9,10,11},{1,1,1},{1,0,0,12,0,1,0,-4,0,0,1,3,0,0,0,1},NULL,0};
    jip_image moving=fixed;
    moving.dim[0]=11;moving.dim[1]=9;moving.dim[2]=10;
    if(kind==1){fixed.spacing[1]=.7f;fixed.spacing[2]=1.4f;moving.spacing[0]=.6f;moving.spacing[1]=1.1f;moving.spacing[2]=.9f;}
    if(kind==2){fixed.spacing[0]=3.5f;fixed.spacing[1]=4.2f;fixed.spacing[2]=3.2f;moving.spacing[0]=1;moving.spacing[1]=1.1f;moving.spacing[2]=.8f;moving.dim[0]=19;moving.dim[1]=21;moving.dim[2]=23;}
    if(kind==3){fixed.spacing[1]=3.4f;fixed.spacing[2]=2.7f;moving.spacing[0]=.6f;moving.spacing[1]=.9f;moving.spacing[2]=1.3f;}
    if(kind==4){moving.dim[0]=2;moving.dim[1]=3;moving.dim[2]=4;fixed.spacing[0]=3;fixed.spacing[1]=4;fixed.spacing[2]=3;}
    if(kind==7)for(int a=0;a<3;a++)moving.dim[a]=fixed.dim[a];
    /* Native geometry differs from centered calculation geometry. */
    moving.native_affine[0]=0;moving.native_affine[1]=-moving.spacing[1];
    moving.native_affine[4]=moving.spacing[0];moving.native_affine[5]=0;
    moving.native_affine[10]=moving.spacing[2];
    fixed.count=(uint64_t)fixed.dim[0]*(uint64_t)fixed.dim[1]*(uint64_t)fixed.dim[2];
    moving.count=(uint64_t)moving.dim[0]*(uint64_t)moving.dim[1]*(uint64_t)moving.dim[2];
    float *f=calloc((size_t)fixed.count,sizeof(float)),*m=calloc((size_t)moving.count,sizeof(float));CHECK(f&&m);
    for(uint64_t i=0;i<fixed.count;i++)f[i]=i%7==0?0.f:(float)(10.+4.*sin((double)i*.61)+3.*cos((double)i*.113));
    for(uint64_t i=0;i<moving.count;i++)m[i]=i%11==0?0.f:(float)(9.+5.*sin((double)i*.431)+2.*cos((double)i*.101));
    fixed.data=f;moving.data=m;
    jip_options options;jip_options_default(&options);options.metric=1;options.grid_down=4;
    options.affine_parameters[0]=.37f;options.affine_parameters[1]=-.63f;options.affine_parameters[2]=.41f;
    options.affine_parameters[3]=15;options.affine_parameters[4]=-6;options.affine_parameters[5]=8;
    options.affine_parameters[6]=1.03f;options.affine_parameters[7]=.97f;options.affine_parameters[9]=.03f;
    if(kind==6){options.affine_parameters[0]=5.3f;options.affine_parameters[1]=-4.7f;options.affine_parameters[2]=4.9f;}
    if(kind==7){jip_options_default(&options);options.metric=1;options.grid_down=4;}
    jip_config config;jip_config_default(&config);config.matching_volume=matching;
    /* Frozen-axis local probes exercise exact coefficient reuse as well. */
    for(int a=0;a<3;a++)config.nonlinear_axes[a]=(a==kind%3);
    for(int a=0;a<3;a++)config.wrap_around[a]=(wrap>>a)&1;
    jip_engine *c=NULL;CHECK(!jip_engine_create_configured(&fixed,&moving,&options,&config,&c));
    const char *backend=getenv("JIP_TEST_BACKEND");if(backend)CHECK(!jip_engine_set_execution(c,backend,4,1024));
    CHECK(c&&c->mimic_stack.n3d==(JIP_INDEX)fixed.count);
    jip_engine *previous=jip_active;
    jip_operation *op=jip_runtime_begin("direct differential resampler fixture");CHECK(op);
    op->arena=&c->arena;jip_active=c;
    if(setjmp(op->recover)){fprintf(stderr,"Unexpected fixture failure: %s\n",op->error.message);return 1;}
    /* Nonzero spatially varying fields exercise local nonlinear resampling. */
    if(kind!=7)for(JIP_INDEX i=0;i<target.n3d;i++) {
      distort_x.SpaceTime.fData[i]=(float)(.2*sin((double)i*.13));
      distort_y.SpaceTime.fData[i]=(float)(.17*cos((double)i*.19));
      distort_z.SpaceTime.fData[i]=(float)(.23*sin((double)i*.23));
    }
    size_t bytes=(size_t)mimic.n3d*sizeof(float);float *expected=malloc(bytes);CHECK(expected);
    mat44 target_matrix=mimic.ijk_to_xyz,source_matrix=source.xyz_to_ijk;
    for(accurate=0;accurate<2;accurate++)for(region=0;region<4;region++) {
      if(region){JIP_INDEX picks[]={0,AlignNL.NBasis/2,AlignNL.NBasis-1};AlignNL.iBasis=picks[region-1];}
      for(JIP_INDEX i=0;i<mimic.n3d;i++)mimic.SpaceTime.fData[i]=-123.25f;
      reference_transform_coordinates(region==0,accurate,1);
      CHECK(!memcmp(&mimic.ijk_to_xyz,&target_matrix,sizeof(mat44))&&!memcmp(&source.xyz_to_ijk,&source_matrix,sizeof(mat44)));
      memcpy(expected,mimic.SpaceTime.fData,bytes);
      for(JIP_INDEX i=0;i<mimic.n3d;i++)mimic.SpaceTime.fData[i]=-123.25f;
      transform_coordinates(region==0,accurate,1);
      CHECK(!memcmp(&mimic.ijk_to_xyz,&target_matrix,sizeof(mat44))&&!memcmp(&source.xyz_to_ijk,&source_matrix,sizeof(mat44)));
      CHECK(!memcmp(expected,mimic.SpaceTime.fData,bytes));
      cases++;
    }
    free(expected);jip_active=previous;jip_runtime_end(op);jip_engine_destroy(c);free(f);free(m);
  }
  /* These cases had undefined float-to-int or kernel-length narrowing in the
     old body, so they have explicit safe outcomes rather than old-code parity. */
  kind=8;float data[729];for(int i=0;i<729;i++)data[i]=(float)(1+i%13);
  jip_image test={{9,9,9},{1,1,1},{1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1},data,729};
  jip_options options;jip_options_default(&options);options.metric=1;options.grid_down=4;
  for(int sign=-1;sign<=1;sign+=2) {
    options.affine_parameters[0]=sign*1.e30f;jip_engine *c=NULL;
    CHECK(!jip_engine_create(&test,&test,&options,&c));jip_image result;
    CHECK(!jip_engine_result(c,&result,NULL));
    for(uint64_t i=0;i<result.count;i++)CHECK(result.data[i]==0);
    jip_engine_destroy(c);
  }
  jip_image tiny=test;tiny.spacing[0]=1.e-12f;jip_engine *c=NULL;
  CHECK(jip_engine_create(&test,&tiny,&options,&c)==JIP_INVALID&&!c);
  uint64_t blocks,bytes,handles,resources;jip_engine_test_live(&blocks,&bytes,&handles,&resources);
  CHECK(!blocks&&!bytes&&!handles&&!resources);
  printf("RESAMPLER_RESULT cases=%d reference=byte-exact geometry=restored ownership=zero\n",cases);
  return 0;
}
