/* Geometry/coefficients stay in the original host arithmetic. Independent tap
   sums are tiled on threaded CPU or GPU; metrics/search/topology stay in JIP.
   A failed tile returns to the complete original touched-region calculation. */
static void acceleration_source_changed(void){if(jip_active&&jip_active->owner)jip_active->owner->source_epoch++;}
static int make_sample(jip_sample *job,IPOINT3D p,IPOINT3D width,POINT3D relative,struct data_stack *destination,struct data_stack *origin) {
  memset(job,0,sizeof(*job));JIP_INDEX index=voxel3D_to_1D(p);sample_begin(index);
  if(!AlignAid.matching_volume&&(target.SpaceTime.fData[index]==0||(!AlignAid.LowResolution&&template.SpaceTime.fData[index]==0)))return 1;
  POINT3D point=centered_coordinate(p,destination);float point4[]={point.x,point.y,point.z,1};
  POINT3D transformed={0,0,0};for(int j=0;j<4;j++){transformed.x+=Alignment.Affine[0][j]*point4[j];transformed.y+=Alignment.Affine[1][j]*point4[j];transformed.z+=Alignment.Affine[2][j]*point4[j];}
  transformed.x+=distort_x.SpaceTime.fData[index];transformed.y+=distort_y.SpaceTime.fData[index];transformed.z+=distort_z.SpaceTime.fData[index];
  POINT3D coordinates=Convert_Space_to_rVoxel(transformed,origin);sample_moving(index,coordinates);float coordinate[]={coordinates.x,coordinates.y,coordinates.z};
  int dimensions[]={source.dim.x,source.dim.y,source.dim.z},wrap[]={Alignment.WrapAround.x,Alignment.WrapAround.y,Alignment.WrapAround.z};
  int widths[]={width.x,width.y,width.z};float ratios[]={relative.x,relative.y,relative.z};int nearest[3];
  for(int axis=0;axis<3;axis++){
    if(!isfinite(coordinate[axis]))return 1;double near=rint((double)coordinate[axis]);if(near<INT_MIN||near>INT_MAX)return 1;
    JIP_INDEX n=(int)near;if(wrap[axis]){if(n<0)n+=dimensions[axis];else if(n>=dimensions[axis])n-=dimensions[axis];}
    if(n<0||n>=dimensions[axis])return 1;nearest[axis]=(int)n;
  }
  JIP_INDEX at=((JIP_INDEX)nearest[2]*source.dim.y+nearest[1])*source.dim.x+nearest[0];if(source.SpaceTime.fData[at]==0)return 1;
  for(int axis=0;axis<3;axis++){
    double low=floor((double)coordinate[axis])-widths[axis],high=ceil((double)coordinate[axis])+widths[axis];
    if(low<INT_MIN||high>=INT_MAX)return 0;
    int length=(int)(high-low+1);if(length<1||length>JIP_KERNEL_TAPS)return 0;
    job->low[axis]=(int)low;job->length[axis]=(uint32_t)length;
    for(int tap=0;tap<length;tap++){
      float delta=coordinate[axis]-(job->low[axis]+tap);
      float value=widths[axis]>2?Gauss(delta,0.,.425*ratios[axis]):Lanczos_filter(delta,widths[axis]);
      if(!isfinite(value))return 0;memcpy(&job->coeff[axis*JIP_KERNEL_TAPS+tap],&value,4);
    }
  }
  job->valid=1;return 1;
}
static int accelerated_transform(int all_voxels,int most_accurate) {
  if(!jip_active->accelerator)return 0;
  jip_execution info;jip_accel_info(jip_active->accelerator,&info);if(!strcmp(info.active,"cpu"))return 0;
  POINT3D relative={mimic.resolution.x/source.resolution.x,mimic.resolution.y/source.resolution.y,mimic.resolution.z/source.resolution.z};
  float ratios[]={relative.x,relative.y,relative.z};int widths[3];
  for(int a=0;a<3;a++){if(!isfinite(ratios[a])||ratios[a]>16)return 0;widths[a]=(int)MAX(floor((double)ratios[a]),most_accurate?2:1);}
  IPOINT3D width={widths[0],widths[1],widths[2]},start={0,0,0},end=target.dim;
  if(!all_voxels){start=AlignNL.Index3DStart[AlignNL.iBasis];end=AlignNL.Index3DEnd[AlignNL.iBasis];}
  JIP_INDEX touched=jip_product(jip_product(end.x-start.x,end.y-start.y),end.z-start.z);
  if(touched<256)return 0; /* dispatch/thread overhead dominates tiny regions */
  const uint32_t tile=jip_accel_tile_size(jip_active->accelerator,(uint64_t)touched);
  jip_sample *jobs;uint32_t *out;int64_t *indices;
  if(!jip_accel_workspace(jip_active->accelerator,tile,&jobs,&out,&indices))return 0;
  double phase=jip_monotonic_seconds();
  jip_kernel_params params={{(uint32_t)source.dim.x,(uint32_t)source.dim.y,(uint32_t)source.dim.z},{(uint32_t)Alignment.WrapAround.x,(uint32_t)Alignment.WrapAround.y,(uint32_t)Alignment.WrapAround.z},0};
  int count=0,ok=1;uint64_t visited=0;IPOINT3D point;
  struct data_stack destination=mimic,origin=source;destination.ijk_to_xyz=AlignAid.ijk_to_xyz;origin.xyz_to_ijk=AlignAid.xyz_to_ijk;
  for(point.z=start.z;point.z<end.z&&ok;point.z++)for(point.y=start.y;point.y<end.y&&ok;point.y++)for(point.x=start.x;point.x<end.x;point.x++){
    if(!(visited++&1023))JIP_CHECKPOINT();if(!make_sample(&jobs[count],point,width,relative,&destination,&origin)){ok=0;break;}
    indices[count]=voxel3D_to_1D(point);
    if(!jobs[count].valid){mimic.SpaceTime.fData[indices[count]]=0;continue;} /* Never dispatch masked/outside samples. */
    count++;
    if((uint32_t)count==tile){jip_active->profile.prepare_seconds+=jip_monotonic_seconds()-phase;phase=jip_monotonic_seconds();params.count=(uint32_t)count;ok=jip_accel_run(jip_active->accelerator,source.SpaceTime.fData,(uint64_t)source.n3d,jip_active->owner->source_epoch,jobs,&params,out);jip_active->profile.dispatch_seconds+=jip_monotonic_seconds()-phase;phase=jip_monotonic_seconds();if(!ok)break;
      for(int i=0;i<count;i++)memcpy(&mimic.SpaceTime.fData[indices[i]],&out[i],4);jip_active->profile.scatter_seconds+=jip_monotonic_seconds()-phase;jip_active->profile.prepared_samples+=(uint64_t)count;phase=jip_monotonic_seconds();count=0;}
  }
  jip_active->profile.prepare_seconds+=jip_monotonic_seconds()-phase;phase=jip_monotonic_seconds();
  if(ok&&count){params.count=(uint32_t)count;ok=jip_accel_run(jip_active->accelerator,source.SpaceTime.fData,(uint64_t)source.n3d,jip_active->owner->source_epoch,jobs,&params,out);jip_active->profile.dispatch_seconds+=jip_monotonic_seconds()-phase;phase=jip_monotonic_seconds();if(ok)for(int i=0;i<count;i++)memcpy(&mimic.SpaceTime.fData[indices[i]],&out[i],4);jip_active->profile.scatter_seconds+=jip_monotonic_seconds()-phase;jip_active->profile.prepared_samples+=(uint64_t)count;}
  return ok;
}
const char *jip_engine_arithmetic(void){
#ifdef JIP_REPRODUCIBLE
  return "reproducible-no-contract";
#else
  return "compatibility-native";
#endif
}
const char *jip_engine_build_info(void){return "{\"compiler\":\"" __VERSION__ "\",\"architecture\":\"" JIP_ARCHITECTURE_NAME "\",\"arithmetic\":\"" JIP_ARITHMETIC_NAME "\",\"pointer_bits\":64}";}
jip_status jip_engine_set_execution(jip_engine *c,const char *backend,uint32_t threads,uint32_t budget) {
  if(!backend||(!strcmp(backend,"cpu")?0:strcmp(backend,"threads")&&strcmp(backend,"auto")&&strcmp(backend,"metal")&&strcmp(backend,"cuda"))||threads<1||threads>64||budget<32||budget>16384)return JIP_INVALID;
  jip_engine *previous;if(!enter_context(c,&previous))return c?JIP_BUSY:JIP_INVALID;
  jip_accelerator *next=NULL;if(strcmp(backend,"cpu"))next=jip_accel_create(backend,threads,budget);
  if(strcmp(backend,"cpu")&&!next){leave_context(c,previous);return JIP_MEMORY;}
  jip_accel_destroy(c->accelerator);c->accelerator=next;c->source_epoch++;leave_context(c,previous);return JIP_OK;
}
jip_status jip_engine_get_execution(jip_engine *c,jip_execution *execution){
  if(!execution||execution->struct_size!=sizeof(*execution))return JIP_INVALID;jip_engine *previous;
  if(!enter_context(c,&previous))return c?JIP_BUSY:JIP_INVALID;jip_accel_info(c->accelerator,execution);execution->map_arithmetic=c->saved_arithmetic;execution->map_architecture=c->saved_architecture;
  if(!execution->gpu_calls&&(!strcmp(execution->active,"metal")||!strcmp(execution->active,"cuda"))){snprintf(execution->active,sizeof(execution->active),"cpu");if(!execution->reason[0])snprintf(execution->reason,sizeof(execution->reason),"Regions were below the GPU dispatch threshold; native CPU was used.");}
  if(!execution->cpu_calls&&!strcmp(execution->active,"threads")){snprintf(execution->active,sizeof(execution->active),"cpu");if(!execution->reason[0])snprintf(execution->reason,sizeof(execution->reason),"Regions were below the threaded dispatch threshold; native CPU was used.");}
  leave_context(c,previous);return JIP_OK;
}
void jip_engine_devices(char *json,size_t capacity){jip_accel_probe(json,capacity);}
void jip_engine_test_acceleration_fault(jip_engine *c,uint64_t calls){if(c)jip_accel_test_fault(c->accelerator,calls);}

jip_status jip_engine_get_profile(jip_engine *c,jip_profile *out){
  if(!out||out->struct_size!=sizeof(*out))return JIP_INVALID;
  jip_engine *previous;if(!enter_context(c,&previous))return c?JIP_BUSY:JIP_INVALID;
  *out=c->profile;out->struct_size=sizeof(*out);
  jip_accel_workspace_info(c->accelerator,&out->workspace_bytes,&out->workspace_allocations);
  leave_context(c,previous);return JIP_OK;
}
