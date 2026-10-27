/* Reuse exact legacy centered coordinates for diagonal engine grids only.
   Native NIfTI obliquity is represented separately; general matrices fall back.
   This cache never combines affine products or changes floating-point order. */
static void prepare_centered_coordinates(void) {
  jip_engine *c=jip_active;mat44 m=AlignAid.ijk_to_xyz;IPOINT3D d=target.dim;
  if(c->coordinate_cache_ready&&memcmp(&m,&c->coordinate_cache_matrix,sizeof(m))==0&&memcmp(&d,&c->coordinate_cache_dim,sizeof(d))==0)return;
  c->coordinate_cache_ready=0;
  int dims[]={d.x,d.y,d.z};
  for(int a=0;a<3;a++){if(dims[a]<1||dims[a]>4096)return;for(int b=0;b<3;b++)if(a!=b&&m.m[a][b]!=0)return;}
  struct data_stack stack=mimic;stack.ijk_to_xyz=m;
  for(int a=0;a<3;a++)for(int i=0;i<dims[a];i++) {
    IPOINT3D p={0,0,0};if(a==0)p.x=i;else if(a==1)p.y=i;else p.z=i;
    POINT3D v=Convert_Voxel_to_Space(p,&stack);
    c->coordinate_cache[a][i]=a==0?v.x:a==1?v.y:v.z;
  }
  c->coordinate_cache_dim=d;c->coordinate_cache_matrix=m;c->coordinate_cache_ready=1;
}
static POINT3D centered_coordinate(IPOINT3D p,struct data_stack *stack) {
  jip_engine *c=jip_active;
  if(c->coordinate_cache_ready)return (POINT3D){c->coordinate_cache[0][p.x],c->coordinate_cache[1][p.y],c->coordinate_cache[2][p.z]};
  return Convert_Voxel_to_Space(p,stack);
}
static int metric_sample(JIP_INDEX i){return !jip_active->sample_selection||jip_active->sample_selection[i];}
static void sample_begin(JIP_INDEX i) {
  if(jip_active->sample_selection)jip_active->sample_selection[i]=(!jip_active->fixed_selection||jip_active->fixed_selection[i])&&!jip_active->moving_selection&&!jip_active->moving_window_count;
}
static void sample_moving(JIP_INDEX i,POINT3D point) {
  jip_engine *c=jip_active;if(!c->moving_selection&&!c->moving_window_count)return;
  if(c->moving_window_count){
    float p[]={point.x,point.y,point.z,1},world[3]={0};
    for(int a=0;a<3;a++)for(int j=0;j<4;j++)world[a]+=source_unfiltered.ijk_to_xyz.m[a][j]*p[j];
    for(uint32_t w=0;w<c->moving_window_count;w++)for(int a=0;a<3;a++)if((c->moving_windows[w].axes&(1u<<a))&&
      (!isfinite(world[a])||world[a]<c->moving_windows[w].minimum[a]||world[a]>c->moving_windows[w].maximum[a]))return;
  }
  if(!c->moving_selection){c->sample_selection[i]=!c->fixed_selection||c->fixed_selection[i];return;}
  double xyz[]={rint((double)point.x),rint((double)point.y),rint((double)point.z)};
  int d[]={source.dim.x,source.dim.y,source.dim.z},wrap[]={Alignment.WrapAround.x,Alignment.WrapAround.y,Alignment.WrapAround.z};
  for(int a=0;a<3;a++){if(!isfinite(xyz[a]))return;if(wrap[a]){if(xyz[a]<0)xyz[a]+=d[a];else if(xyz[a]>=d[a])xyz[a]-=d[a];}if(xyz[a]<0||xyz[a]>=d[a])return;}
  JIP_INDEX at=((JIP_INDEX)xyz[2]*d[1]+(JIP_INDEX)xyz[1])*d[0]+(JIP_INDEX)xyz[0];
  c->sample_selection[i]=(!c->fixed_selection||c->fixed_selection[i])&&c->moving_selection[at];
}

/* Exact memoization of the common Lanczos kernels. Keys retain the complete
   float coordinate and width; no fractional quantization or table approximation.
   Six taps cover width<=2; smoothing/large kernels retain the original path. */
static void common_coefficients(int axis,float coordinate,int width,int low,int count,float *values){
  uint32_t bits;memcpy(&bits,&coordinate,4);
  uint32_t slot=(bits^(bits>>11)^(bits>>20)^((uint32_t)width*0x9e3779b9u))&255u;
  jip_engine *c=jip_active;
  if(c->coefficient_cache[axis][slot].width==(uint32_t)width&&c->coefficient_cache[axis][slot].bits==bits){
    memcpy(values,c->coefficient_cache[axis][slot].values,(size_t)count*sizeof(float));return;
  }
  for(int tap=0;tap<count;tap++)values[tap]=Lanczos_filter((float)(coordinate-(low+tap)),width);
  memcpy(c->coefficient_cache[axis][slot].values,values,(size_t)count*sizeof(float));
  c->coefficient_cache[axis][slot].bits=bits;c->coefficient_cache[axis][slot].width=(uint32_t)width;
}
