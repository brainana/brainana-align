/* White-box membership checks against an independent world/nearest reference,
   followed by public API checkpoint, rollback, ownership and backend tests. */
#include "../source/engine/engine.c"
#include "jip_engine_test.h"
#undef malloc
#undef calloc
#undef free
#undef exit
#define CHECK(x) do{if(!(x)){fprintf(stderr,"SAMPLING_FAIL %d %s\n",__LINE__,#x);exit(1);}}while(0)
static int expected(jip_engine *c,int x,int y,int z){
  int at=x+c->target_stack.dim.x*(y+c->target_stack.dim.y*z);
  if(c->fixed_selection&&!c->fixed_selection[at])return 0;
  float f[]={x,y,z,1},center[4]={0,0,0,1},pull[4]={0,0,0,1},voxel[4]={0,0,0,1};
  for(int a=0;a<3;a++)for(int b=0;b<4;b++)center[a]+=c->aid.ijk_to_xyz.m[a][b]*f[b];
  for(int a=0;a<3;a++)for(int b=0;b<4;b++)pull[a]+=c->alignment_state.Affine[a][b]*center[b];
  pull[0]+=c->dx.SpaceTime.fData[at];pull[1]+=c->dy.SpaceTime.fData[at];pull[2]+=c->dz.SpaceTime.fData[at];
  for(int a=0;a<3;a++)for(int b=0;b<4;b++)voxel[a]+=c->aid.xyz_to_ijk.m[a][b]*pull[b];
  float world[3]={0};for(int a=0;a<3;a++)for(int b=0;b<4;b++)world[a]+=c->source_original.ijk_to_xyz.m[a][b]*voxel[b];
  for(uint32_t w=0;w<c->moving_window_count;w++)for(int a=0;a<3;a++)if((c->moving_windows[w].axes&(1u<<a))&&(world[a]<c->moving_windows[w].minimum[a]||world[a]>c->moving_windows[w].maximum[a]))return 0;
  if(!c->moving_selection)return 1;
  int d[]={c->source_stack.dim.x,c->source_stack.dim.y,c->source_stack.dim.z},v[3];
  for(int a=0;a<3;a++){v[a]=(int)rint((double)voxel[a]);if(v[a]<0||v[a]>=d[a])return 0;}
  return c->moving_selection[v[0]+d[0]*(v[1]+d[1]*v[2])]!=0;
}
static void equal(jip_engine *a,jip_engine *b){
  jip_transform x,y;CHECK(!jip_engine_get_transform(a,&x));CHECK(!jip_engine_get_transform(b,&y));
  CHECK(!memcmp(x.affine_parameters,y.affine_parameters,sizeof(x.affine_parameters)));
  CHECK(x.count==y.count);for(int k=0;k<3;k++)CHECK(!memcmp(x.displacement[k],y.displacement[k],(size_t)x.count*4));
  CHECK(x.nonlinear_objective==y.nonlinear_objective);
}
static void cancel_trace(const jip_trace *trace,void *user){(void)trace;CHECK(jip_engine_request_cancel(user)==JIP_OK);}
int main(void){
 float data[990],other[990];uint8_t fm[990],mm[990];
 for(int z=0;z<11;z++)for(int y=0;y<10;y++)for(int x=0;x<9;x++){int i=x+9*(y+10*z);data[i]=(float)(2+70*exp(-((x-3.)*(x-3.)/8+(y-4.)*(y-4.)/10+(z-5.)*(z-5.)/12)));other[i]=data[i]*(1.f+.05f*sinf(z));fm[i]=(x>0&&y>0);mm[i]=(x<8&&z>0);}
 jip_image f={{9,10,11},{.7f,1.2f,1.6f},{.6f,-.5f,0,7,.36f,1.09f,0,-3,0,0,1.6f,4,0,0,0,1},data,990},m=f;m.data=other;
 jip_options o;jip_options_default(&o);o.metric=1;o.grid_down=4;o.regularization=1;
 jip_config config;jip_config_default(&config);config.nonlinear_axes[0]=config.nonlinear_axes[1]=0;
 jip_window window={{3,-100,10000},{13,100,10001},3}; /* depth must be ignored */
 jip_sampling s={.struct_size=sizeof(s),.fixed=fm,.moving=mm,.fixed_count=990,.moving_count=990,.moving_windows=&window,.moving_window_count=1};
 jip_engine *plain=NULL,*c=NULL;CHECK(!jip_engine_create_sampled(&f,&m,&o,&config,&s,&c));CHECK(!jip_engine_create_configured(&f,&m,&o,&config,&plain));
 CHECK(!memcmp(c->mimic_stack.SpaceTime.fData,plain->mimic_stack.SpaceTime.fData,990*4));jip_engine_destroy(plain);
 fm[30]^=1;CHECK(c->fixed_selection[30]!=fm[30]);fm[30]^=1;
 jip_operation *op=jip_runtime_begin("sampling reference");CHECK(op);op->arena=&c->arena;jip_active=c;
 if(setjmp(op->recover)){fprintf(stderr,"%s\n",op->error.message);return 2;}
 Alignment.value[0]=.37f;Alignment.value[1]=-.44f;Alignment.value[2]=.61f;
 for(int i=0;i<990;i++){distort_x.SpaceTime.fData[i]=.1f*sinf(i*.13f);distort_y.SpaceTime.fData[i]=.08f*cosf(i*.11f);distort_z.SpaceTime.fData[i]=.05f*sinf(i*.07f);}
 transform_coordinates(1,0,1);float full_image[990];memcpy(full_image,mimic.SpaceTime.fData,sizeof(full_image));
 for(int plane=0;plane<3;plane++){
  uint32_t axes[]={3,5,6};jip_window v={{3,-1,8},{13,8,16},axes[plane]};
  for(int axis=0;axis<3;axis++)if(!(v.axes&(1u<<axis))){v.minimum[axis]=10000;v.maximum[axis]=10001;}
  c->moving_windows[0]=v;transform_coordinates(1,0,1);int included=0;
  for(int z=0;z<11;z++)for(int y=0;y<10;y++)for(int x=0;x<9;x++){int at=x+9*(y+10*z);CHECK(c->sample_selection[at]==expected(c,x,y,z));included+=c->sample_selection[at];}
  CHECK(included>0&&included<990);CHECK(!memcmp(full_image,mimic.SpaceTime.fData,sizeof(full_image)));
 }
 c->moving_windows[0]=window;transform_coordinates(1,0,1);double i2=0,t2=0,it=0;int count=0;
 for(int z=0;z<11;z++)for(int y=0;y<10;y++)for(int x=0;x<9;x++){int at=x+9*(y+10*z),yes=expected(c,x,y,z);CHECK(c->sample_selection[at]==yes);if(yes){double a=mimic.SpaceTime.fData[at],b=target.SpaceTime.fData[at];i2+=a*a;t2+=b*b;it+=a*b;count++;}}
 CHECK(count>64&&count<990);int regularization=AlignNL.Lambda_reg;AlignNL.Lambda_reg=0;CHECK(fabs(cost_function_nonlinear(0)-it/sqrt(i2)/sqrt(t2))<1e-7);AlignNL.Lambda_reg=regularization;c->result_objective=cost_function_nonlinear(0);
 prepare_centered_coordinates();CHECK(c->coordinate_cache_ready);AlignAid.ijk_to_xyz.m[0][1]=.2f;prepare_centered_coordinates();CHECK(!c->coordinate_cache_ready);AlignAid.ijk_to_xyz.m[0][1]=0;prepare_centered_coordinates();CHECK(c->coordinate_cache_ready);IPOINT3D dims=target.dim;target.dim.x=4097;prepare_centered_coordinates();CHECK(!c->coordinate_cache_ready);target.dim=dims;prepare_centered_coordinates();CHECK(c->coordinate_cache_ready);
 jip_active=NULL;jip_runtime_end(op);
 CHECK(!jip_engine_checkpoint_save(c,"sampled.jipcp"));jip_engine *restored=NULL;CHECK(!jip_engine_checkpoint_load("sampled.jipcp",&restored));equal(c,restored);CHECK(!memcmp(c->sample_selection,restored->sample_selection,990));CHECK(!memcmp(c->fixed_selection,restored->fixed_selection,990));CHECK(restored->moving_window_count==1);
 jip_engine_destroy(c);jip_engine_destroy(restored);
 for(int metric=0;metric<2;metric++){
  o.metric=metric;jip_engine *a=NULL,*b=NULL;CHECK(!jip_engine_create_sampled(&f,&m,&o,&config,&s,&a));CHECK(!jip_engine_create_sampled(&f,&m,&o,&config,&s,&b));CHECK(!jip_engine_set_execution(b,"threads",4,1024));
  CHECK(!jip_engine_run(a,3));CHECK(!jip_engine_run(b,3));equal(a,b);CHECK(!jip_engine_checkpoint_save(a,"accepted.jipcp"));
  jip_engine *gold=NULL;CHECK(!jip_engine_checkpoint_load("accepted.jipcp",&gold));equal(a,gold);
  jip_engine_test_fault(1,0,JIP_MEMORY);CHECK(jip_engine_run(a,2)==JIP_MEMORY);equal(a,gold);
  CHECK(!jip_engine_set_trace(a,cancel_trace,a));CHECK(jip_engine_run(a,2)==JIP_CANCELLED);equal(a,gold);CHECK(!jip_engine_set_trace(a,NULL,NULL));
  CHECK(!jip_engine_run(a,2));CHECK(!jip_engine_run(gold,2));equal(a,gold);
  jip_engine_destroy(a);jip_engine_destroy(b);jip_engine_destroy(gold);
 }
 /* Every fallible sampled creation step must leave no retained ownership. */
 CHECK(!jip_engine_create_sampled(&f,&m,&o,&config,&s,&c));uint64_t allocations=jip_engine_test_last_operation().allocations;jip_engine_destroy(c);c=NULL;
 for(uint64_t allocation=1;allocation<=allocations;allocation++){jip_engine_test_fault(allocation,0,JIP_MEMORY);CHECK(jip_engine_create_sampled(&f,&m,&o,&config,&s,&c)==JIP_MEMORY&&!c);}
 mm[0]=2;CHECK(jip_engine_create_sampled(&f,&m,&o,&config,&s,&c)==JIP_INVALID&&!c);mm[0]=0;
 s.moving_count--;CHECK(jip_engine_create_sampled(&f,&m,&o,&config,&s,&c)==JIP_INVALID&&!c);s.moving_count++;
 window.minimum[2]=NAN;CHECK(jip_engine_create_sampled(&f,&m,&o,&config,&s,&c)==JIP_INVALID&&!c);window.minimum[2]=10000;
 s.struct_size--;CHECK(jip_engine_create_sampled(&f,&m,&o,&config,&s,&c)==JIP_INVALID&&!c);
 uint64_t blocks,bytes,handles,resources;jip_engine_test_live(&blocks,&bytes,&handles,&resources);CHECK(!blocks&&!bytes&&!handles&&!resources);
 puts("SAMPLING_RESULT physical_continuous_windows=passed masks=copied metrics=independent checkpoint=exact continuation=exact rollback=passed threads=exact ownership=zero");return 0;
}
