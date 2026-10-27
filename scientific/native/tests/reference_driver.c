/* Runs the immutable production engine. Only three drawing/label callbacks
   are intercepted; the numerical code is compiled directly from frozen source.
   Compiler function instrumentation records state without rescoring probes. */
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdint.h>
#include "util.h"
#include "align.h"
#include "phantom.h"
static FILE *trace_file;
static uint64_t events;
void Update_Regions(void) {}
void refresh_display_image(int area) {(void)area;}
void show_on_info_line(int a,int b,char *c,char *d,char *e) {(void)a;(void)b;(void)c;(void)d;(void)e;}
static void snapshot(int event) {
  int64_t header[]={event,AlignNL.iGridDown,AlignNL.NBasis,AlignNL.iBasis,target.n3d};
  if(!trace_file)return;
  fwrite(header,sizeof(header),1,trace_file);
  fwrite(&AlignAid.CF_Affine,sizeof(float),1,trace_file);
  fwrite(&AlignAid.CF_NonLinear,sizeof(float),1,trace_file);
  fwrite(AlignNL.value,sizeof(POINT3D),(size_t)AlignNL.NBasis,trace_file);
  fwrite(distort_x.SpaceTime.fData,sizeof(float),(size_t)target.n3d,trace_file);
  fwrite(distort_y.SpaceTime.fData,sizeof(float),(size_t)target.n3d,trace_file);
  fwrite(distort_z.SpaceTime.fData,sizeof(float),(size_t)target.n3d,trace_file);
  events++;
}
void __cyg_profile_func_enter(void *f,void *site){(void)f;(void)site;}
void __cyg_profile_func_exit(void *f,void *site){
  (void)site;
  if(f==(void*)adjust_1basis)snapshot(1);
  else if(f==(void*)adjust_nonlinear_1pass)snapshot(2);
  else if(f==(void*)update_nonlinear_alignment_parameters)snapshot(3);
}
static struct data_stack image(void) {
  struct data_stack s={0};s.dim=(IPOINT3D){9,9,9};s.resolution=(POINT3D){1,1.5,2};s.direction=(POINT3D){1,1,1};
  s.fov=(POINT3D){9,13.5,18};s.n2d=81;s.n3d=s.n4d=s.npts_volume=729;s.nt=1;
  s.SpaceTime.fData=float_vector(729);
  s.ijk_to_xyz=scale_and_shift_to_mat44(s.resolution,s.origin,s.direction);s.xyz_to_ijk=invert_mat44(s.ijk_to_xyz);
  for(int z=0;z<9;z++)for(int y=0;y<9;y++)for(int x=0;x<9;x++)s.SpaceTime.fData[x+9*(y+9*z)]=phantom_value(x,y,z);
  return s;
}
int main(int argc,char **argv) {
  if(argc!=4)return 2;
  template=image();source_unfiltered=image();mosaic.stack=&template;
  initialize_alignment();AlignAid.CostFunction=atoi(argv[1]);AlignNL.Lambda_reg=1;SetGridDown(atoi(argv[2]));
  initialize_data_stacks();Alignment.value[0]=.8;
  AlignAid.xyz_to_ijk=centered_xyz_to_ijk(source);
  transform_coordinates(1,0,1);float initial=cost_function_nonlinear(0);
  trace_file=fopen(argv[3],"wb");if(!trace_file)return 3;
  action_auto_adjust_nonlinear(1);
  snapshot(4);fclose(trace_file);trace_file=NULL;
  float final=cost_function_nonlinear(0);
  if(!isfinite(final)||final<=initial||number_invalid_distortions(1,0,0))return 4;
  printf("REFERENCE_RESULT metric=%d grid=%d initial=%.9g final=%.9g events=%llu\n",atoi(argv[1]),atoi(argv[2]),initial,final,(unsigned long long)events);
  return 0;
}
