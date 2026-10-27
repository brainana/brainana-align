/* New checkpoint format: explicit little-endian fixed-width fields, no C
   struct dumps/pointers. It is distinct from legacy align.com sessions. */
#ifdef _WIN32
FILE *jip_windows_io_open_exclusive(const char *path);
int jip_windows_io_replace(const char *from,const char *to);
#define output_open_exclusive jip_windows_io_open_exclusive
#define output_replace jip_windows_io_replace
#else
static FILE *output_open_exclusive(const char *path) {return fopen(path,"wbx");}
static int output_replace(const char *from,const char *to) {return rename(from,to);}
#endif
static atomic_ullong output_sequence;
typedef struct {FILE *file;char *temporary;int resource;} jip_output;
static void cleanup_output(void *object) {
  jip_output *out=object;
  if(out->file)fclose(out->file);
  remove(out->temporary);
}
static void cleanup_nifti(void *object) {nifti_image_free(object);}
static void cleanup_file(void *object) {fclose(object);}
static jip_output *output_begin(const char *path) {
  if(!path||strlen(path)>SIZE_MAX-96)return NULL;
  jip_output *out=calloc(1,sizeof(*out));out->temporary=malloc(strlen(path)+96);
  for(int attempt=0;attempt<100;attempt++) {
    unsigned long long seq=atomic_fetch_add(&output_sequence,1);
    snprintf(out->temporary,strlen(path)+96,"%s.jip-%llx-%llu.tmp",path,(unsigned long long)(uintptr_t)out,seq);
    out->file=output_open_exclusive(out->temporary);
    if(out->file){out->resource=jip_runtime_defer(out,cleanup_output);return out;}
    if(errno!=EEXIST)break;
  }
  free(out->temporary);free(out);return NULL;
}
static int output_finish(jip_output *out,const char *path,int ok) {
  if(fclose(out->file))ok=0;out->file=NULL;
  if(ok&&output_replace(out->temporary,path))ok=0;
  if(!ok)remove(out->temporary);
  jip_runtime_forget(out->resource);free(out->temporary);free(out);return ok;
}
static int write_nifti_array(const char *path,struct data_stack *s,int nt,const float *const *planes) {
  if(!path)return 0;size_t len=strlen(path);
  if(len<4||strcmp(path+len-4,".nii"))return 0; /* uncompressed output in this milestone */
  int dims[8]={nt>1?4:3,s->dim.x,s->dim.y,s->dim.z,nt,1,1,1};
  nifti_image *nim=nifti_make_new_nim(dims,NIFTI_TYPE_FLOAT32,0);if(!nim)jip_runtime_raise(JIP_MEMORY,"cannot allocate NIfTI export header",sizeof(*nim));
  int nim_resource=jip_runtime_defer(nim,cleanup_nifti);
  nim->dx=s->resolution.x;nim->dy=s->resolution.y;nim->dz=s->resolution.z;
  nim->pixdim[1]=nim->dx;nim->pixdim[2]=nim->dy;nim->pixdim[3]=nim->dz;
  nim->sform_code=NIFTI_XFORM_SCANNER_ANAT;nim->qform_code=0;nim->sto_xyz=s->ijk_to_xyz;nim->sto_ijk=s->xyz_to_ijk;
  nim->xyz_units=NIFTI_UNITS_MM;nim->nifti_type=NIFTI_FTYPE_NIFTI1_1;nim->iname_offset=352;
  snprintf(nim->descrip,sizeof(nim->descrip),"JIP %s centered-mm pull map A*x+u(x)",JIP_ENGINE_VERSION);
  nifti_1_header h=nifti_convert_nim2nhdr(nim);h.vox_offset=352;memcpy(h.magic,"n+1",4);
  jip_output *out=output_begin(path);FILE *f=out?out->file:NULL;int ok=f!=NULL;
  if(ok){const unsigned char ext[4]={0};ok=fwrite(&h,sizeof(h),1,f)==1&&fwrite(ext,4,1,f)==1;
    for(int a=0;a<nt&&ok;a++)ok=fwrite(planes[a],sizeof(float),(size_t)s->n3d,f)==(size_t)s->n3d;
    ok=output_finish(out,path,ok);
  }
  nifti_image_free(nim);jip_runtime_forget(nim_resource);return ok;
}
static int json_string(FILE *file,const char *value) {
  int ok=fputc('"',file)!=EOF;
  for(const unsigned char *p=(const unsigned char*)value;*p&&ok;p++) {
    if(*p=='"'||*p=='\\')ok=fputc('\\',file)!=EOF&&fputc(*p,file)!=EOF;
    else if(*p<32)ok=fprintf(file,"\\u%04x",(unsigned)*p)>0;
    else ok=fputc(*p,file)!=EOF;
  }
  return ok&&fputc('"',file)!=EOF;
}
static int export_metadata(const char *image_path) {
  size_t n=strlen(image_path);char *path=malloc(n+6);if(!path)return 0;
  memcpy(path,image_path,n);memcpy(path+n,".json",6);
  jip_output *out=output_begin(path);FILE *f=out?out->file:NULL;if(!f){free(path);return 0;}
  int ok=fprintf(f,"{\n  \"engine_version\": \"%s\",\n  \"mapping\": \"fixed centered mm to moving centered mm; pull map A*x+u(x)\",\n  \"field_components\": \"three volumes: centered-mm dx, dy, dz on fixed grid\",\n  \"metric\": %d,\n  \"regularization\": %d,\n  \"grid_down\": %d,\n",JIP_ENGINE_VERSION,AlignAid.CostFunction,AlignNL.Lambda_reg,AlignNL.iGridDown)>0;
  jip_execution execution;jip_accel_info(jip_active->accelerator,&execution);
  if((!execution.gpu_calls&&(!strcmp(execution.active,"metal")||!strcmp(execution.active,"cuda")))||(!execution.cpu_calls&&!strcmp(execution.active,"threads")))snprintf(execution.active,sizeof(execution.active),"cpu");
  ok&=fprintf(f,"  \"execution_device\": ")>0;ok&=json_string(f,execution.device);ok&=fprintf(f,",\n  \"execution_reason\": ")>0;ok&=json_string(f,execution.reason);ok&=fprintf(f,",\n")>0;
  ok&=fprintf(f,"  \"execution\": {\"requested\":\"%s\",\"active\":\"%s\",\"gpu_calls\":%llu,\"cpu_tile_calls\":%llu,\"fallbacks\":%llu,\"map_arithmetic_id\":%u,\"map_architecture_id\":%u},\n  \"build\": %s,\n",execution.requested,execution.active,(unsigned long long)execution.gpu_calls,(unsigned long long)execution.cpu_calls,(unsigned long long)execution.fallbacks,jip_active->saved_arithmetic,jip_active->saved_architecture,jip_engine_build_info())>0;
  ok&=fprintf(f,"  \"stages\": %u,\n  \"completed_stages\": %u,\n  \"affine_model\": %d,\n  \"affine_metric\": %d,\n  \"nonlinear_axes\": [%d, %d, %d],\n  \"matching_volume\": %d,\n  \"wrap_around\": [%d, %d, %d],\n",jip_active->config.stages,jip_active->completed_stages,jip_active->config.affine_model,jip_active->config.affine_metric,jip_active->config.nonlinear_axes[0],jip_active->config.nonlinear_axes[1],jip_active->config.nonlinear_axes[2],jip_active->config.matching_volume,jip_active->config.wrap_around[0],jip_active->config.wrap_around[1],jip_active->config.wrap_around[2])>0;
  if(jip_active->sample_selection){
    uint64_t fixed_count=0,moving_count=0;for(JIP_INDEX v=0;v<target.n3d;v++)if(!jip_active->fixed_selection||jip_active->fixed_selection[v])fixed_count++;
    for(JIP_INDEX v=0;v<source.n3d;v++)if(!jip_active->moving_selection||jip_active->moving_selection[v])moving_count++;
    ok&=fprintf(f,"  \"sampling\": {\"version\":1,\"fixed_included_voxels\":%llu,\"moving_included_voxels\":%llu,\"moving_windows\":[",(unsigned long long)fixed_count,(unsigned long long)moving_count)>0;
    for(uint32_t w=0;w<jip_active->moving_window_count;w++){
      jip_window *v=jip_active->moving_windows+w;
      ok&=fprintf(f,"%s{\"ras_axes_bits\":%u,\"minimum_mm\":[%.9g,%.9g,%.9g],\"maximum_mm\":[%.9g,%.9g,%.9g]}",w?",":"",v->axes,v->minimum[0],v->minimum[1],v->minimum[2],v->maximum[0],v->maximum[1],v->maximum[2])>0;
    }
    ok&=fprintf(f,"],\"semantics\":\"Metric sample centers only; masks retained in JIPCP004 checkpoint; image values and interpolation taps unchanged\"},\n")>0;
  }
  const char *names[]={"centered_pull_affine","fixed_voxel_to_native_mm","moving_voxel_to_native_mm","fixed_voxel_to_centered_mm","moving_centered_mm_to_voxel"};
  float *matrices[]={&Alignment.Affine[0][0],&template.ijk_to_xyz.m[0][0],&source.ijk_to_xyz.m[0][0],&AlignAid.ijk_to_xyz.m[0][0],&AlignAid.xyz_to_ijk.m[0][0]};
  for(int a=0;a<5;a++) {
    ok&=fprintf(f,"  \"%s\": [",names[a])>0;
    for(int b=0;b<16;b++)ok&=fprintf(f,"%s%.9g",b?", ":"",matrices[a][b])>0;
    ok&=fprintf(f,"]%s\n",a<4?",":"")>0;
  }
  ok&=fprintf(f,"}\n")>0;ok=output_finish(out,path,ok);free(path);return ok;
}
static jip_status export_body(const char *aligned_path,const char *field_path) {
  const float *aligned[]={mimic.SpaceTime.fData};
  const float *fields[]={distort_x.SpaceTime.fData,distort_y.SpaceTime.fData,distort_z.SpaceTime.fData};
  int ok=(!aligned_path||write_nifti_array(aligned_path,&mimic,1,aligned))&&(!field_path||write_nifti_array(field_path,&target,3,fields));
  if(ok&&aligned_path)ok=export_metadata(aligned_path);
  if(ok&&field_path)ok=export_metadata(field_path);
  return ok?JIP_OK:JIP_IO;
}
typedef struct {FILE *f;int ok;uint32_t crc;} checkpoint_stream;
static void cp_byte(checkpoint_stream *s,unsigned char *v,int write) {
  if(write){if(fputc(*v,s->f)==EOF)s->ok=0;}
  else {int c=fgetc(s->f);if(c==EOF){s->ok=0;*v=0;}else *v=(unsigned char)c;}
  s->crc=(uint32_t)crc32(s->crc,v,1);
}
static void cp_u32(checkpoint_stream *s,uint32_t *v,int write) {
  uint32_t n=*v;if(!write)n=0;
  for(int a=0;a<4;a++){unsigned char b=(unsigned char)(n>>(8*a));cp_byte(s,&b,write);if(!write)n|=(uint32_t)b<<(8*a);}
  if(!write)*v=n;
}
static void cp_float(checkpoint_stream *s,float *v,int write) {uint32_t u=0;if(write)memcpy(&u,v,4);cp_u32(s,&u,write);if(!write)memcpy(v,&u,4);}
static void cp_floats(checkpoint_stream *s,float *v,uint64_t n,int write) {for(uint64_t i=0;i<n&&s->ok;i++){if((i&4095)==0)jip_runtime_poll();cp_float(s,v+i,write);}}
static void cp_image_header(checkpoint_stream *s,jip_image *im,int write) {
  for(int a=0;a<3;a++){uint32_t d=(uint32_t)im->dim[a];cp_u32(s,&d,write);if(!write)im->dim[a]=(int32_t)d;}
  cp_floats(s,im->spacing,3,write);cp_floats(s,im->native_affine,16,write);
}
static void cp_mask(checkpoint_stream *s,uint8_t *v,uint64_t n,int write){
  for(uint64_t i=0;i<n&&s->ok;i++){if(!(i&4095))jip_runtime_poll();cp_byte(s,v+i,write);if(!write&&v[i]>1)s->ok=0;}
}
static void cp_options(checkpoint_stream *s,jip_options *o,int write) {
  uint32_t a=(uint32_t)o->metric,b=(uint32_t)o->grid_down,c=(uint32_t)o->regularization;
  cp_u32(s,&a,write);cp_u32(s,&b,write);cp_u32(s,&c,write);
  if(!write){o->metric=(int)a;o->grid_down=(int)b;o->regularization=(int)c;}
  cp_float(s,&o->jacobian_limit,write);cp_float(s,&o->stiffness,write);cp_floats(s,o->affine_parameters,12,write);
}
static int config_is_default(const jip_config *config) {
  jip_config defaults;jip_config_default(&defaults);return !memcmp(config,&defaults,sizeof(defaults));
}
static void cp_config(checkpoint_stream *s,jip_config *config,uint32_t *completed,float *affine_score,uint32_t *first_affine,uint32_t *first_iteration,int write) {
  uint32_t fields[11];
  if(write) {
    fields[0]=config->stages;fields[1]=(uint32_t)config->affine_model;fields[2]=(uint32_t)config->affine_metric;
    for(int a=0;a<3;a++){fields[3+a]=(uint32_t)config->nonlinear_axes[a];fields[7+a]=(uint32_t)config->wrap_around[a];}
    fields[6]=(uint32_t)config->matching_volume;fields[10]=*completed;
  } else memset(fields,0,sizeof(fields));
  for(int i=0;i<11;i++)cp_u32(s,&fields[i],write);
  cp_float(s,affine_score,write);cp_u32(s,first_affine,write);cp_u32(s,first_iteration,write);
  if(!write) {
    config->stages=fields[0];config->affine_model=(int32_t)fields[1];config->affine_metric=(int32_t)fields[2];
    for(int a=0;a<3;a++){config->nonlinear_axes[a]=(int32_t)fields[3+a];config->wrap_around[a]=(int32_t)fields[7+a];}
    config->matching_volume=(int32_t)fields[6];*completed=fields[10];
  }
}
static int cp_footer(checkpoint_stream *s,int write) {
  uint32_t checksum=s->crc,saved=checksum;cp_u32(s,&saved,write);
  return s->ok&&(write||saved==checksum);
}
static jip_status checkpoint_save_body(jip_engine *c,const char *path) {
  jip_output *out=output_begin(path);if(!out)return JIP_IO;FILE *f=out->file;
  checkpoint_stream s={f,1,0};
  int extended=1;
  const char *magic=(c->fixed_selection||c->moving_selection||c->moving_window_count)?"JIPCP004":"JIPCP003";
  for(int i=0;i<8;i++){unsigned char b=(unsigned char)magic[i];cp_byte(&s,&b,1);}
  cp_u32(&s,&c->saved_arithmetic,1);cp_u32(&s,&c->saved_architecture,1);
  jip_options o=c->options;o.grid_down=AlignNL.iGridDown;memcpy(o.affine_parameters,Alignment.value,sizeof(Alignment.value));cp_options(&s,&o,1);
  if(extended) {
    jip_config config=c->config;uint32_t completed=c->completed_stages,first_affine=(uint32_t)c->first_affine,first_iteration=(uint32_t)c->first_affine_iteration;
    float score=c->affine_objective;cp_config(&s,&config,&completed,&score,&first_affine,&first_iteration,1);
  }
  jip_image fixed=image_view(&template),moving=image_view(&source_unfiltered);cp_image_header(&s,&fixed,1);cp_image_header(&s,&moving,1);
  uint32_t first=(uint32_t)c->first_nonlinear,adjusted=(uint32_t)(AlignNL.Adjusted.x||AlignNL.Adjusted.y||AlignNL.Adjusted.z);
  cp_u32(&s,&first,1);cp_u32(&s,&adjusted,1);
  cp_floats(&s,(float*)fixed.data,fixed.count,1);cp_floats(&s,(float*)moving.data,moving.count,1);
  if(c->fixed_selection||c->moving_selection||c->moving_window_count) {
    uint32_t flags=(!!c->fixed_selection)|((!!c->moving_selection)<<1)|((!!c->moving_window_count)<<2);cp_u32(&s,&flags,1);
    if(c->moving_window_count){cp_u32(&s,&c->moving_window_count,1);for(uint32_t w=0;w<c->moving_window_count;w++){cp_u32(&s,&c->moving_windows[w].axes,1);cp_floats(&s,c->moving_windows[w].minimum,3,1);cp_floats(&s,c->moving_windows[w].maximum,3,1);}}
    if(c->fixed_selection)cp_mask(&s,c->fixed_selection,fixed.count,1);
    if(c->moving_selection)cp_mask(&s,c->moving_selection,moving.count,1);
  }
  cp_floats(&s,(float*)AlignNL.value,3*(uint64_t)AlignNL.NBasis,1);
  cp_floats(&s,distort_x.SpaceTime.fData,fixed.count,1);cp_floats(&s,distort_y.SpaceTime.fData,fixed.count,1);cp_floats(&s,distort_z.SpaceTime.fData,fixed.count,1);
  cp_floats(&s,mimic.SpaceTime.fData,fixed.count,1);
  int ok=output_finish(out,path,cp_footer(&s,1));return ok?JIP_OK:JIP_IO;
}
static jip_engine *checkpoint_load_body(const char *path,jip_operation *op) {
  FILE *f=fopen(path,"rb");if(!f)jip_runtime_raise(JIP_IO,"cannot open checkpoint",0);
  int file_resource=jip_runtime_defer(f,cleanup_file);
  checkpoint_stream s={f,1,0};char magic[9]={0};
  for(int i=0;i<8;i++)cp_byte(&s,(unsigned char*)magic+i,0);
  jip_options o={0};jip_image fixed={0},moving={0};uint32_t first=0,adjusted=0;
  jip_config config;jip_config_default(&config);uint32_t completed=0,first_affine=1,first_iteration=1;float affine_score=0;
  int sampled=!strcmp(magic,"JIPCP004"),provenance=sampled||!strcmp(magic,"JIPCP003"),extended=provenance||!strcmp(magic,"JIPCP002");
  uint32_t arithmetic=0,architecture=0;
  if(provenance){cp_u32(&s,&arithmetic,0);cp_u32(&s,&architecture,0);}
  cp_options(&s,&o,0);
  if(extended)cp_config(&s,&config,&completed,&affine_score,&first_affine,&first_iteration,0);
  cp_image_header(&s,&fixed,0);cp_image_header(&s,&moving,0);cp_u32(&s,&first,0);cp_u32(&s,&adjusted,0);
  if(!s.ok||arithmetic>2||architecture>2||(!extended&&strcmp(magic,"JIPCP001"))||!options_valid(&o)||!config_valid(&config)||first>1||adjusted>1||first_affine>1||first_iteration>1||
     (completed&~(uint32_t)(JIP_STAGE_AFFINE|JIP_STAGE_NONLINEAR))||!isfinite(affine_score))jip_runtime_raise(JIP_INVALID,"invalid or truncated checkpoint header",0);
  uint64_t counts[2]={1,1};jip_image *ims[]={&fixed,&moving};
  for(int a=0;a<2;a++)for(int b=0;b<3;b++) {
    int32_t d=ims[a]->dim[b];if(d<2||counts[a]>(uint64_t)PTRDIFF_MAX/sizeof(float)/(uint32_t)d)jip_runtime_raise(JIP_INVALID,"invalid checkpoint image dimensions",0);counts[a]*=(uint32_t)d;
  }
  fixed.count=counts[0];moving.count=counts[1];
  JIP_INDEX at=jip_tell(f);if(jip_seek(f,0,SEEK_END))jip_runtime_raise(JIP_IO,"cannot seek checkpoint",0);
  JIP_INDEX end=jip_tell(f);
  if(end<at||counts[0]>(uint64_t)(end-at)/20||counts[1]>((uint64_t)(end-at)-20*counts[0])/4)
    jip_runtime_raise(JIP_INVALID,"checkpoint payload is too short",0);
  if(jip_seek(f,at,SEEK_SET))jip_runtime_raise(JIP_IO,"cannot seek checkpoint payload",0);
  float *a=malloc((size_t)fixed.count*sizeof(float)),*b=malloc((size_t)moving.count*sizeof(float));
  fixed.data=a;moving.data=b;cp_floats(&s,a,fixed.count,0);cp_floats(&s,b,moving.count,0);
  if(!s.ok)jip_runtime_raise(JIP_INVALID,"truncated checkpoint image data",0);
  jip_sampling sampling={.struct_size=sizeof(sampling)};uint8_t *fm=NULL,*mm=NULL;jip_window windows[3];
  if(sampled){uint32_t flags=0;cp_u32(&s,&flags,0);if(flags<1||flags>7)jip_runtime_raise(JIP_INVALID,"invalid checkpoint sampling flags",0);
    if(flags&4){cp_u32(&s,&sampling.moving_window_count,0);if(sampling.moving_window_count<1||sampling.moving_window_count>3)jip_runtime_raise(JIP_INVALID,"invalid checkpoint window count",0);sampling.moving_windows=windows;
      for(uint32_t w=0;w<sampling.moving_window_count;w++){cp_u32(&s,&windows[w].axes,0);cp_floats(&s,windows[w].minimum,3,0);cp_floats(&s,windows[w].maximum,3,0);}}
    if(flags&1){fm=malloc((size_t)fixed.count);cp_mask(&s,fm,fixed.count,0);sampling.fixed=fm;sampling.fixed_count=fixed.count;}
    if(flags&2){mm=malloc((size_t)moving.count);cp_mask(&s,mm,moving.count,0);sampling.moving=mm;sampling.moving_count=moving.count;}
    if(!s.ok)jip_runtime_raise(JIP_INVALID,"truncated checkpoint sampling mask",0);
  }
  jip_engine *c=create_body(&fixed,&moving,&o,&config,sampled?&sampling:NULL,op);free(a);free(b);free(fm);free(mm);
  c->saved_arithmetic=arithmetic;c->saved_architecture=architecture;
  cp_floats(&s,(float*)AlignNL.value,3*(uint64_t)AlignNL.NBasis,0);
  cp_floats(&s,distort_x.SpaceTime.fData,fixed.count,0);cp_floats(&s,distort_y.SpaceTime.fData,fixed.count,0);cp_floats(&s,distort_z.SpaceTime.fData,fixed.count,0);cp_floats(&s,mimic.SpaceTime.fData,fixed.count,0);
  int ok=cp_footer(&s,0)&&fgetc(f)==EOF;
  for(JIP_INDEX i=0;i<AlignNL.NBasis&&ok;i++)if(!isfinite(AlignNL.value[i].x)||!isfinite(AlignNL.value[i].y)||!isfinite(AlignNL.value[i].z))ok=0;
  for(uint64_t i=0;i<fixed.count&&ok;i++)if(!isfinite(distort_x.SpaceTime.fData[i])||!isfinite(distort_y.SpaceTime.fData[i])||!isfinite(distort_z.SpaceTime.fData[i])||!isfinite(mimic.SpaceTime.fData[i]))ok=0;
  if(!ok)jip_runtime_raise(JIP_INVALID,"checkpoint payload/checksum is invalid",0);
  c->completed_stages=extended?completed:(!first?JIP_STAGE_NONLINEAR:0);
  c->affine_objective=affine_score;c->first_affine=(BOOL)first_affine;c->first_affine_iteration=(BOOL)first_iteration;
  c->first_nonlinear=(BOOL)first;AlignNL.Adjusted=(IPOINT3D){(int)adjusted,(int)adjusted,(int)adjusted};
  if(c->sample_selection){
    struct data_stack dst=mimic,src=source;dst.ijk_to_xyz=AlignAid.ijk_to_xyz;src.xyz_to_ijk=AlignAid.xyz_to_ijk;IPOINT3D p;
    for(p.z=0;p.z<target.dim.z;p.z++)for(p.y=0;p.y<target.dim.y;p.y++)for(p.x=0;p.x<target.dim.x;p.x++){
      JIP_INDEX i=voxel3D_to_1D(p);sample_begin(i);POINT3D v=Convert_Voxel_to_Space(p,&dst);float point[]={v.x,v.y,v.z,1};POINT3D t={0,0,0};
      for(int j=0;j<4;j++){t.x+=Alignment.Affine[0][j]*point[j];t.y+=Alignment.Affine[1][j]*point[j];t.z+=Alignment.Affine[2][j]*point[j];}
      t.x+=distort_x.SpaceTime.fData[i];t.y+=distort_y.SpaceTime.fData[i];t.z+=distort_z.SpaceTime.fData[i];sample_moving(i,Convert_Space_to_rVoxel(t,&src));
    }
  }
  c->result_objective=cost_function_nonlinear(0);
  int close_status=fclose(f);jip_runtime_forget(file_resource);if(close_status)jip_runtime_raise(JIP_IO,"cannot close checkpoint",0);
  return c;
}
/* I/O uses a scratch arena and a resource ledger. A thrown allocation/size
   error closes files, removes unpublished temporary files and frees buffers. */
static jip_status output_operation(jip_engine *c,const char *first,const char *second,int checkpoint) {
  jip_engine *previous;if(!enter_context(c,&previous))return c?JIP_BUSY:JIP_INVALID;
  const char *phase=checkpoint?"checkpoint save":"export";jip_operation *op=jip_runtime_begin(phase);
  if(!op){record_error(c,JIP_MEMORY,phase,"cannot allocate operation guard");leave_context(c,previous);return JIP_MEMORY;}
  if(setjmp(op->recover)) {jip_status status=op->error.code;c->last_error=op->error;jip_runtime_end(op);leave_context(c,previous);return status;}
  jip_status status=checkpoint?checkpoint_save_body(c,first):export_body(first,second);
  if(status!=JIP_OK){record_error(c,status,phase,"cannot write requested output");op->error=c->last_error;}
  jip_runtime_end(op);leave_context(c,previous);return status;
}
jip_status jip_engine_export(jip_engine *c,const char *aligned_path,const char *field_path) {
  if(!c||(!aligned_path&&!field_path)||(aligned_path&&field_path&&!strcmp(aligned_path,field_path)))return JIP_INVALID;
  return output_operation(c,aligned_path,field_path,0);
}
jip_status jip_engine_checkpoint_save(jip_engine *c,const char *path) {
  if(!c||!path)return JIP_INVALID;return output_operation(c,path,NULL,1);
}
jip_status jip_engine_checkpoint_load(const char *path,jip_engine **out) {
  if(!path||!out)return JIP_INVALID;*out=NULL;
  jip_engine *previous=jip_active;jip_operation *op=jip_runtime_begin("checkpoint load");if(!op)return JIP_MEMORY;
  if(setjmp(op->recover)){jip_status status=op->error.code;discard_handle(op->handle);jip_active=previous;jip_runtime_end(op);return status;}
  *out=checkpoint_load_body(path,op);op->handle=NULL;jip_active=previous;jip_runtime_end(op);return JIP_OK;
}
