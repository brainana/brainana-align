#define main original_nonlinear_fixture_main
#include "reference_driver.c"
#undef main
int main(int argc,char **argv) {
  if(argc!=5)return 2;
  template=image();source_unfiltered=image();mosaic.stack=&template;
  initialize_alignment();AlignAid.CostFunction=1;AlignNL.Lambda_reg=1;SetGridDown(4);
  initialize_data_stacks();Alignment.value[0]=.8;AlignAid.xyz_to_ijk=centered_xyz_to_ijk(source);
  transform_coordinates(1,0,1);cost_function_nonlinear(0);
  AlignAid.CostFunction=atoi(argv[2]);for(int i=0;i<12;i++)AlignAid.Adjust[i]=i<atoi(argv[1]);
  action_auto_adjust_affine(2);float affine_score=cost_function();AlignAid.CostFunction=1;
  if(atoi(argv[3]))action_auto_adjust_nonlinear(1);
  float score=cost_function_nonlinear(0);
  FILE *out=fopen(argv[4],"wb");if(!out)return 3;
  fwrite(Alignment.value,sizeof(float),12,out);fwrite(Alignment.Affine,sizeof(float),16,out);
  fwrite(&affine_score,sizeof(float),1,out);fwrite(&score,sizeof(float),1,out);
  fwrite(mimic.SpaceTime.fData,sizeof(float),729,out);
  fwrite(distort_x.SpaceTime.fData,sizeof(float),729,out);fwrite(distort_y.SpaceTime.fData,sizeof(float),729,out);fwrite(distort_z.SpaceTime.fData,sizeof(float),729,out);
  return fclose(out)!=0;
}
