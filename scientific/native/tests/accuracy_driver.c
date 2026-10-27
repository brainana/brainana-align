/* Analytic brain-shaped synthetic data, not a patient-data accuracy claim.
   True centered pull maps and error targets are fixed in accuracy-policy.json. */
#include "jip_engine.h"
#include "jip_engine_test.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#if defined(__APPLE__) || defined(__linux__)
#include <sys/resource.h>
#endif
#define CHECK(x) do{if(!(x)){fprintf(stderr,"ACCURACY_FAIL line %d: %s\n",__LINE__,#x);exit(1);}}while(0)
static double anatomy(double x,double y,double z) {
  double ell=x*x/90+y*y/125+z*z/100;
  if(ell>1.15)return 0;
  double v=15+45*exp(-1.5*ell)+30*exp(-((x+3)*(x+3)/9+(y-3)*(y-3)/14+(z+1)*(z+1)/18));
  v+=20*exp(-((x-4)*(x-4)/7+(y+4)*(y+4)/10+(z-3)*(z-3)/12));
  v-=25*exp(-(x*x/4+(y+1)*(y+1)/20+z*z/7));
  v+=4*sin(x*.9)*cos(y*.6)*cos(z*.7);return v;
}
static void map_point(int kind,const double p[3],double q[3]) {
  for(int a=0;a<3;a++)q[a]=p[a];
  if(kind==4){double cs=cos(.13962634015954636),sn=sin(.13962634015954636);q[0]=cs*p[0]-sn*p[1]+2.4;q[1]=sn*p[0]+cs*p[1]-1.6;q[2]=p[2]+1;return;}
  if(kind==3){q[0]+=.7*sin(p[0]*3.141592653589793/12.);return;}
  q[0]+=1.2;q[1]-=.8;q[2]+=.5;
  if(kind==1){q[0]-=.04*p[0];q[1]+=.03*p[1];q[2]-=.02*p[2];}
  if(kind==2){q[0]+=.035*p[1]-.02*p[2];q[1]+=-.02*p[1]+.03*p[2];q[2]+=.02*p[2];}
}
static void inverse_point(int kind,const double q[3],double p[3]) {
  for(int a=0;a<3;a++)p[a]=q[a];
  if(kind==4){double cs=cos(.13962634015954636),sn=sin(.13962634015954636);p[0]=cs*(q[0]-2.4)+sn*(q[1]+1.6);p[1]=-sn*(q[0]-2.4)+cs*(q[1]+1.6);p[2]=q[2]-1;return;}
  if(kind==3){for(int i=0;i<20;i++)p[0]=q[0]-.7*sin(p[0]*3.141592653589793/12.);return;}
  p[0]-=1.2;p[1]+=.8;p[2]-=.5;
  if(kind==1){p[0]/=.96;p[1]/=1.03;p[2]/=.98;}
  if(kind==2){p[2]/=1.02;p[1]=(p[1]-.03*p[2])/.98;p[0]-=.035*p[1]-.02*p[2];}
}
static double mse(const jip_image *fixed,const jip_image *moved) {double sum=0;for(uint64_t i=0;i<fixed->count;i++){double d=fixed->data[i]-moved->data[i];sum+=d*d;}return sum/(double)fixed->count;}
static double det3(double m[9]){return m[0]*(m[4]*m[8]-m[5]*m[7])-m[1]*(m[3]*m[8]-m[5]*m[6])+m[2]*(m[3]*m[7]-m[4]*m[6]);}
int main(int argc,char **argv) {
  CHECK(argc==2);int kind=atoi(argv[1]);CHECK(kind>=0&&kind<5);
  int dim[]={24,28,20};if(kind==4){dim[0]=64;dim[1]=72;dim[2]=56;}
  float spacing[]={1,1.1f,1.4f};uint64_t count=(uint64_t)dim[0]*(uint64_t)dim[1]*(uint64_t)dim[2];
  int mdim[]={dim[0],dim[1],dim[2]};float mspacing[]={spacing[0],spacing[1],spacing[2]};
  if(kind==4){mdim[0]=72;mdim[1]=64;mdim[2]=48;mspacing[0]=.9f;mspacing[1]=1.3f;mspacing[2]=1.6f;}
  uint64_t moving_count=(uint64_t)mdim[0]*(uint64_t)mdim[1]*(uint64_t)mdim[2];
  float *f=calloc((size_t)count,4),*m=calloc((size_t)moving_count,4);CHECK(f&&m);
  jip_image fixed={{24,28,20},{1,1.1f,1.4f},{.9396926f,-.37622216f,0,11,.34202014f,1.033662f,0,-6,0,0,1.4f,4,0,0,0,1},f,count};
  for(int a=0;a<3;a++)fixed.dim[a]=dim[a];
  jip_image moving=fixed;moving.data=m;moving.count=moving_count;
  for(int a=0;a<3;a++){moving.dim[a]=mdim[a];moving.spacing[a]=mspacing[a];}
  if(kind==4){moving.native_affine[0]=.8457233f;moving.native_affine[1]=-.4446262f;moving.native_affine[4]=.3078181f;moving.native_affine[5]=1.2216003f;moving.native_affine[10]=1.6f;}
  moving.native_affine[3]=-5;moving.native_affine[7]=7;
  for(int z=0;z<dim[2];z++)for(int y=0;y<dim[1];y++)for(int x=0;x<dim[0];x++) {
    double p[3]={(x-(dim[0]-1)/2.)*spacing[0],(y-(dim[1]-1)/2.)*spacing[1],(z-(dim[2]-1)/2.)*spacing[2]},q[3];
    (void)q;uint64_t i=(uint64_t)((z*dim[1]+y)*dim[0]+x);double scale=kind==4?2:1;f[i]=(float)anatomy(p[0]/scale,p[1]/scale,p[2]/scale);
  }
  for(int z=0;z<mdim[2];z++)for(int y=0;y<mdim[1];y++)for(int x=0;x<mdim[0];x++) {
    double p[3]={(x-(mdim[0]-1)/2.)*mspacing[0],(y-(mdim[1]-1)/2.)*mspacing[1],(z-(mdim[2]-1)/2.)*mspacing[2]},q[3];
    inverse_point(kind,p,q);uint64_t i=(uint64_t)((z*mdim[1]+y)*mdim[0]+x);double scale=kind==4?2:1;m[i]=(float)anatomy(q[0]/scale,q[1]/scale,q[2]/scale);
  }
  jip_options options;jip_options_default(&options);options.metric=1;options.grid_down=4;options.regularization=1;
  jip_config config;jip_config_default(&config);config.stages=kind==3?2:1;config.affine_model=kind==1?9:kind==2?12:6;
  if(kind==3){config.nonlinear_axes[1]=config.nonlinear_axes[2]=0;}
  jip_engine *c=NULL;CHECK(!jip_engine_create_configured(&fixed,&moving,&options,&config,&c));
  jip_image before,after;CHECK(!jip_engine_result(c,&before,NULL));double before_mse=mse(&fixed,&before);clock_t start=clock();
  jip_status status=jip_engine_register(c);
  if(status){jip_error e;jip_engine_get_error(c,&e);fprintf(stderr,"%s: %s\n",e.phase,e.message);}CHECK(!status);
  double seconds=(double)(clock()-start)/CLOCKS_PER_SEC;uint64_t trial=jip_engine_test_last_operation().peak_new_bytes;
  CHECK(!jip_engine_result(c,&after,NULL));double after_mse=mse(&fixed,&after),tre=0,initial_tre=0,jmin=1.e100,jmax=-1.e100;uint64_t samples=0;
  jip_transform tr;CHECK(!jip_engine_get_transform(c,&tr));
  for(int z=0;z<dim[2];z++)for(int y=0;y<dim[1];y++)for(int x=0;x<dim[0];x++) {
    uint64_t i=(uint64_t)((z*dim[1]+y)*dim[0]+x);double p[3]={(x-(dim[0]-1)/2.)*spacing[0],(y-(dim[1]-1)/2.)*spacing[1],(z-(dim[2]-1)/2.)*spacing[2]},truth[3];map_point(kind,p,truth);
    if(f[i]>0) {
      for(int a=0;a<3;a++){double v=tr.centered_pull_affine[a*4+3]+tr.displacement[a][i];for(int b=0;b<3;b++)v+=tr.centered_pull_affine[a*4+b]*p[b];tre+=(v-truth[a])*(v-truth[a]);initial_tre+=(p[a]-truth[a])*(p[a]-truth[a]);}samples++;
    }
    int stride[]={1,dim[0],dim[0]*dim[1]},at[]={x,y,z};double jac[9];
    for(int a=0;a<3;a++)for(int b=0;b<3;b++) {
      uint64_t low=at[b]>0?i-(uint64_t)stride[b]:i,high=at[b]<dim[b]-1?i+(uint64_t)stride[b]:i;
      int steps=(at[b]>0)+(at[b]<dim[b]-1);
      jac[3*a+b]=tr.centered_pull_affine[4*a+b]+(tr.displacement[a][high]-tr.displacement[a][low])/(steps*spacing[b]);
    }
    double determinant=det3(jac);if(determinant<jmin)jmin=determinant;if(determinant>jmax)jmax=determinant;
  }
  CHECK(samples);tre=sqrt(tre/(double)samples);initial_tre=sqrt(initial_tre/(double)samples);
  CHECK(!memcmp(after.native_affine,fixed.native_affine,sizeof(fixed.native_affine)));
  CHECK(!jip_engine_checkpoint_save(c,"accuracy.jipcp"));jip_engine *restored=NULL;CHECK(!jip_engine_checkpoint_load("accuracy.jipcp",&restored));jip_image im;float score_a,score_b;
  CHECK(!jip_engine_result(c,&after,&score_a)&&!jip_engine_result(restored,&im,&score_b));CHECK(score_a==score_b&&!memcmp(after.data,im.data,(size_t)count*4));
  CHECK(!jip_engine_export(c,"aligned.nii","field.nii"));uint64_t rss=0;
#if defined(__APPLE__) || defined(__linux__)
  struct rusage usage;CHECK(!getrusage(RUSAGE_SELF,&usage));rss=(uint64_t)usage.ru_maxrss;
#ifndef __APPLE__
  rss*=1024;
#endif
#endif
  printf("ACCURACY_JSON {\"case\":%d,\"voxels\":%llu,\"initial_tre_mm\":%.9g,\"final_tre_mm\":%.9g,\"initial_mse\":%.9g,\"final_mse\":%.9g,\"jacobian_min\":%.9g,\"jacobian_max\":%.9g,\"cpu_seconds\":%.9g,\"trial_payload_peak_bytes\":%llu,\"process_peak_rss_bytes\":%llu,\"restore\":\"exact\"}\n",kind,(unsigned long long)count,initial_tre,tre,before_mse,after_mse,jmin,jmax,seconds,(unsigned long long)trial,(unsigned long long)rss);
  jip_engine_destroy(restored);jip_engine_destroy(c);free(f);free(m);
  uint64_t blocks,bytes,handles,resources;jip_engine_test_live(&blocks,&bytes,&handles,&resources);CHECK(!blocks&&!bytes&&!handles&&!resources);
  CHECK(isfinite(tre)&&isfinite(after_mse)&&jmin>0);return 0;
}
