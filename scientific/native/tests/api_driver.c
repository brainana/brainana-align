#include "jip_engine.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdint.h>
#include "phantom.h"
static FILE *trace_file;
static void trace(const jip_trace *t,void *user) {
  FILE *f=user;
  int64_t h[]={t->event,t->grid_down,t->basis_count,t->basis_index,t->voxel_count};
  fwrite(h,sizeof(h),1,f);fwrite(&t->image_objective,4,1,f);fwrite(&t->objective,4,1,f);
  fwrite(t->coefficients_xyz,12,(size_t)t->basis_count,f);
  for(int a=0;a<3;a++)fwrite(t->displacement[a],4,(size_t)t->voxel_count,f);
}
static jip_engine *make(int metric,int grid) {
  float data[729];for(int z=0;z<9;z++)for(int y=0;y<9;y++)for(int x=0;x<9;x++)data[x+9*(y+9*z)]=phantom_value(x,y,z);
  jip_image im={{9,9,9},{1,1.5,2},{1,0,0,0,0,1.5,0,0,0,0,2,0,0,0,0,1},data,729};
  jip_options o;jip_options_default(&o);o.metric=metric;o.grid_down=grid;o.regularization=1;o.affine_parameters[0]=.8;
  jip_engine *c=NULL;if(jip_engine_create(&im,&im,&o,&c))return NULL;return c;
}
int main(int argc,char **argv) {
  if(argc!=4)return 2;
  jip_engine *c=make(atoi(argv[1]),atoi(argv[2]));if(!c)return 3;
  jip_image im;float initial,final;
  if(jip_engine_result(c,&im,&initial))return 4;
  trace_file=fopen(argv[3],"wb");if(!trace_file)return 5;
  jip_engine_set_trace(c,trace,trace_file);
  if(jip_engine_register(c))return 6;
  fclose(trace_file);jip_engine_set_trace(c,NULL,NULL);
  if(jip_engine_result(c,&im,&final))return 7;
  if(jip_engine_export(c,"aligned.nii","distortions.nii"))return 8;
  if(jip_engine_checkpoint_save(c,"registration.jipcp"))return 9;
  jip_engine *restored=NULL;if(jip_engine_checkpoint_load("registration.jipcp",&restored))return 10;
  jip_image roundtrip;float score;if(jip_engine_result(restored,&roundtrip,&score)||score!=final||memcmp(im.data,roundtrip.data,729*4))return 11;
  if(jip_engine_export(restored,"restored.nii","restored-field.nii"))return 12;
  printf("API_RESULT initial=%.9g final=%.9g checkpoint=exact\n",initial,final);
  jip_engine_destroy(restored);jip_engine_destroy(c);return 0;
}
