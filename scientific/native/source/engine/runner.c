/* Headless CLI. No legacy GUI headers cross this neutral C ABI boundary. */
#include "jip_engine.h"
#include "nifti1_io.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include <limits.h>
#include <sys/stat.h>
#include <zlib.h>
#include <stddef.h>
#include <signal.h>
#include <stdatomic.h>
_Static_assert(ATOMIC_POINTER_LOCK_FREE==2&&ATOMIC_INT_LOCK_FREE==2,"signal cancellation requires lock-free atomics");
static _Atomic(jip_engine*) signal_context;
static atomic_int interrupted;
static void interrupt_registration(int sig) {
  (void)sig;atomic_store_explicit(&interrupted,1,memory_order_relaxed);
  jip_engine *c=atomic_load_explicit(&signal_context,memory_order_relaxed);
  if(c)(void)jip_engine_request_cancel(c);
}
typedef struct {jip_engine *context;int reported;} runner_progress;
static void registration_progress(const jip_trace *t,void *user) {
  runner_progress *p=user;
  if(atomic_load_explicit(&interrupted,memory_order_relaxed))(void)jip_engine_request_cancel(p->context);
  if((t->event==1||t->event==5)&&!p->reported){p->reported=1;puts("Registration is fitting coefficients. Press Ctrl+C to cancel.");fflush(stdout);}
}
static int same_file(const char *a,const char *b) {
  if(!strcmp(a,b))return 1;
  struct stat x,y;
  return !stat(a,&x)&&!stat(b,&y)&&x.st_ino!=0&&x.st_ino==y.st_ino&&x.st_dev==y.st_dev;
}
static void usage(void) {
  puts("jip-register " JIP_ENGINE_VERSION " — original JIP nonlinear CPU registration\n"
       "  --fixed fixed.nii[.gz] --moving moving.nii[.gz] --output aligned.nii\n"
       "  [--field displacement.nii] [--checkpoint state.jipcp]\n"
       "  [--metric mi|nc] [--grid 1|2|4|8|16] [--regularization integer]\n"
       "  [--stages affine|nonlinear|both] [--affine-model 6|9|12] [--affine-metric mi|nmi]\n"
       "  [--nonlinear-axes xyz|x|y|z|xy|xz|yz] [--matching fixed|all] [--wrap xyz|none]\n"
       "  [--jacobian-limit number>1] [--stiffness 0..1]\n"
       "  [--backend cpu|threads|auto|metal|cuda] [--threads 1..64] [--gpu-budget-mb 32..16384]\n"
       "  --devices reports runtime capabilities; --version reports arithmetic.\n"
       "  [--affine-parameters tx ty tz rxy ryz rxz sx sy sz hxy hxz hyz]\n"
       "  --resume state.jipcp --output aligned.nii [--continue [--stages affine|nonlinear|both]]\n\n"
       "Inputs must be scalar 3D NIfTI in millimeters. Output uses the fixed\n"
       "native grid. Parameters and displacement use JIP centered-mm pull\n"
       "coordinates A*x+u(x), not a forward world-coordinate displacement.\n"
       "A resumed checkpoint is exported unchanged unless --continue is given.\n"
       "Ctrl+C during fitting cancels the attempt and preserves the accepted state.\n"
       "Optional GPU interpolation falls back to CPU. See the platform validation report.");
}
static int load(const char *path,jip_image *im) {
  int swapped=0;nifti_1_header *header=nifti_read_header(path,&swapped,1);
  if(!header)return 0;
  if(memcmp(header->magic,"n+1",3)||header->dim[0]<3||header->dim[0]>4||
     header->dim[1]<2||header->dim[2]<2||header->dim[3]<2||
     (header->dim[0]==4&&header->dim[4]!=1)||(header->xyzt_units&7)!=NIFTI_UNITS_MM||
     !isfinite(header->vox_offset)||header->vox_offset<352||
     (double)header->vox_offset>=9223372036854775808.) {
    fprintf(stderr,"Expected one scalar 3D NIfTI volume in millimeters: %s\n",path);free(header);return 0;
  }
  int64_t offset_bytes=(int64_t)header->vox_offset;
  uint64_t count=(uint64_t)header->dim[1];
  for(int a=2;a<=3;a++) {
    if(count>(uint64_t)PTRDIFF_MAX/sizeof(double)/(uint32_t)header->dim[a]){free(header);return 0;}
    count*=(uint32_t)header->dim[a];
  }
  /* The old vendor converter stores iname_offset in int. Use it only for
     geometry, with a safe dummy offset; stream the payload with a wide offset.
     Avoid vendor nifti_read_buffer, which also silently replaces NaNs by zero. */
  header->vox_offset=352;
  nifti_image *n=nifti_convert_nhdr2nim(*header,NULL);free(header);
  if(!n||count!=n->nvox||n->nbyper<=0||n->nbyper>8){if(n)nifti_image_free(n);return 0;}
  float *data=malloc((size_t)count*4);if(!data){nifti_image_free(n);return 0;}
  double slope=n->scl_slope!=0?n->scl_slope:1,offset=n->scl_slope!=0?n->scl_inter:0;
  int compressed=nifti_is_gzfile(path);FILE *f=NULL;gzFile gz=NULL;
  _Static_assert(sizeof(z_off_t)>=8,"gzip requires a 64-bit zlib offset type");
  if(compressed)gz=gzopen(path,"rb");else f=fopen(path,"rb");
  int good=compressed?(gz&&gzseek(gz,(z_off_t)offset_bytes,SEEK_SET)>=0):(f!=NULL);
  if(good&&!compressed) {
#ifdef _WIN32
    good=_fseeki64(f,offset_bytes,SEEK_SET)==0;
#else
    good=fseeko(f,(off_t)offset_bytes,SEEK_SET)==0;
#endif
  }
  void *raw=malloc(65536);if(!raw)good=0;
  for(uint64_t start=0;start<count&&good;) {
    size_t chunk=(size_t)(count-start);if(chunk>65536/(size_t)n->nbyper)chunk=65536/(size_t)n->nbyper;
    size_t bytes=chunk*(size_t)n->nbyper;
    if(compressed){int got=gzread(gz,raw,(unsigned)bytes);good=got>=0&&(size_t)got==bytes;}
    else good=fread(raw,1,bytes,f)==bytes;
    if(!good)break;
    if(swapped&&n->swapsize>1)nifti_swap_Nbytes(chunk,n->swapsize,raw);
    for(size_t i=0;i<chunk;i++) {
    double v;
    switch(n->datatype){
      case NIFTI_TYPE_UINT8:v=((uint8_t*)raw)[i];break;
      case NIFTI_TYPE_INT8:v=((int8_t*)raw)[i];break;
      case NIFTI_TYPE_INT16:v=((int16_t*)raw)[i];break;
      case NIFTI_TYPE_UINT16:v=((uint16_t*)raw)[i];break;
      case NIFTI_TYPE_INT32:v=((int32_t*)raw)[i];break;
      case NIFTI_TYPE_UINT32:v=((uint32_t*)raw)[i];break;
      case NIFTI_TYPE_INT64:v=(double)((int64_t*)raw)[i];break;
      case NIFTI_TYPE_UINT64:v=(double)((uint64_t*)raw)[i];break;
      case NIFTI_TYPE_FLOAT32:v=((float*)raw)[i];break;
      case NIFTI_TYPE_FLOAT64:v=((double*)raw)[i];break;
      default:good=0;v=0;break;
    }
    data[start+i]=(float)(v*slope+offset);
  }
    start+=chunk;
  }
  free(raw);if(f&&fclose(f))good=0;if(gz&&gzclose(gz)!=Z_OK)good=0;
  if(!good){fprintf(stderr,"Incomplete or unsupported image payload: %s\n",path);free(data);nifti_image_free(n);return 0;}
  memset(im,0,sizeof(*im));im->dim[0]=n->nx;im->dim[1]=n->ny;im->dim[2]=n->nz;
  im->spacing[0]=n->dx;im->spacing[1]=n->dy;im->spacing[2]=n->dz;im->count=count;im->data=data;
  mat44 affine=n->sform_code>0?n->sto_xyz:n->qto_xyz;
  memcpy(im->native_affine,affine.m,sizeof(im->native_affine));nifti_image_free(n);return 1;
}
static int number(const char *s,int *out) {char *end;errno=0;long n=strtol(s,&end,10);if(errno||!*s||*end||n<0||n>INT_MAX)return 0;*out=(int)n;return 1;}
int main(int argc,char **argv) {
  if(argc==2&&!strcmp(argv[1],"--devices")){char json[4096];jip_engine_devices(json,sizeof(json));puts(json);return 0;}
  if(argc==2&&!strcmp(argv[1],"--version")){puts(JIP_ENGINE_VERSION);puts(jip_engine_build_info());return 0;}
  if(argc==1||!strcmp(argv[1],"--help")){usage();return 0;}
  const char *fixed=NULL,*moving=NULL,*output=NULL,*field=NULL,*checkpoint=NULL,*resume=NULL;
  const char *backend="cpu";int threads=4,budget_mb=1024;
  int again=0,configured=0,stages_given=0;jip_options options;jip_options_default(&options);jip_config config;jip_config_default(&config);
  for(int i=1;i<argc;i++) {
    const char *a=argv[i];
    if(!strcmp(a,"--continue")){again=1;continue;}
    if(i+1>=argc){fprintf(stderr,"Missing value for %s\n",a);return 2;}
    const char *v=argv[++i];
    if(!strcmp(a,"--fixed"))fixed=v;
    else if(!strcmp(a,"--backend")){if(strcmp(v,"cpu")&&strcmp(v,"threads")&&strcmp(v,"auto")&&strcmp(v,"metal")&&strcmp(v,"cuda"))return 2;backend=v;}
    else if(!strcmp(a,"--threads")){if(!number(v,&threads)||threads<1||threads>64)return 2;}
    else if(!strcmp(a,"--gpu-budget-mb")){if(!number(v,&budget_mb)||budget_mb<32||budget_mb>16384)return 2;}
    else if(!strcmp(a,"--moving"))moving=v;
    else if(!strcmp(a,"--output"))output=v;
    else if(!strcmp(a,"--field"))field=v;
    else if(!strcmp(a,"--checkpoint"))checkpoint=v;
    else if(!strcmp(a,"--resume"))resume=v;
    else if(!strcmp(a,"--grid")){configured=1;if(!number(v,&options.grid_down))return 2;}
    else if(!strcmp(a,"--regularization")){configured=1;if(!number(v,&options.regularization))return 2;}
    else if(!strcmp(a,"--metric")){configured=1;if(strcmp(v,"mi")&&strcmp(v,"nc"))return 2;options.metric=!strcmp(v,"nc");}
    else if(!strcmp(a,"--stages")) {
      stages_given=1;if(!strcmp(v,"affine"))config.stages=JIP_STAGE_AFFINE;
      else if(!strcmp(v,"nonlinear"))config.stages=JIP_STAGE_NONLINEAR;
      else if(!strcmp(v,"both"))config.stages=JIP_STAGE_AFFINE|JIP_STAGE_NONLINEAR;else return 2;
    }
    else if(!strcmp(a,"--affine-model")){configured=1;int model;if(!number(v,&model)||(model!=6&&model!=9&&model!=12))return 2;config.affine_model=model;}
    else if(!strcmp(a,"--affine-metric")){configured=1;if(strcmp(v,"mi")&&strcmp(v,"nmi"))return 2;config.affine_metric=!strcmp(v,"nmi");}
    else if(!strcmp(a,"--matching")){configured=1;if(strcmp(v,"fixed")&&strcmp(v,"all"))return 2;config.matching_volume=!strcmp(v,"all");}
    else if(!strcmp(a,"--nonlinear-axes")||!strcmp(a,"--wrap")) {
      configured=1;int32_t *axes=!strcmp(a,"--wrap")?config.wrap_around:config.nonlinear_axes;
      memset(axes,0,3*sizeof(*axes));
      if(strcmp(v,"none"))for(const char *q=v;*q;q++){int at=*q=='x'?0:*q=='y'?1:*q=='z'?2:-1;if(at<0||axes[at])return 2;axes[at]=1;}
      else if(!strcmp(a,"--nonlinear-axes"))return 2;
    }
    else if(!strcmp(a,"--jacobian-limit")||!strcmp(a,"--stiffness")) {
      configured=1;char *end;errno=0;float value=strtof(v,&end);if(errno||!*v||*end||!isfinite(value))return 2;
      if(!strcmp(a,"--stiffness")){if(value<0||value>1)return 2;options.stiffness=value;}
      else {if(value<=1)return 2;options.jacobian_limit=value;}
    }
    else if(!strcmp(a,"--affine-parameters")) {
      configured=1;--i;
      if(argc-i-1<12)return 2;
      for(int j=0;j<12;j++){char *end;errno=0;float x=strtof(argv[++i],&end);if(errno||!argv[i][0]||!isfinite(x)||*end)return 2;options.affine_parameters[j]=x;}
    } else {fprintf(stderr,"Unknown option: %s\n",a);return 2;}
  }
  if(!output||(!resume&&(!fixed||!moving))||(resume&&(fixed||moving||configured||(stages_given&&!again)))){usage();return 2;}
  /* Includes existing symlink/hardlink aliases, without changing directories. */
  const char *outs[]={output,field,checkpoint};const char *ins[]={fixed,moving,resume};
  for(int a=0;a<3;a++)if(outs[a]) {
    for(int b=0;b<3;b++)if(ins[b]&&same_file(outs[a],ins[b])){fprintf(stderr,"Output must differ from input.\n");return 2;}
    for(int b=0;b<a;b++)if(outs[b]&&same_file(outs[a],outs[b])){fprintf(stderr,"Output paths must differ.\n");return 2;}
  }
  jip_engine *c=NULL;jip_status status;
  if(resume)status=jip_engine_checkpoint_load(resume,&c);
  else {
    jip_image f={0},m={0};if(!load(fixed,&f)||!load(moving,&m)){free((void*)f.data);free((void*)m.data);return 3;}
    status=jip_engine_create_configured(&f,&m,&options,&config,&c);free((void*)f.data);free((void*)m.data);
  }
  if(status==JIP_OK)status=jip_engine_set_execution(c,backend,(uint32_t)threads,(uint32_t)budget_mb);
  if(status==JIP_OK&&(!resume||again)) {
    runner_progress progress={c,0};jip_engine_set_trace(c,registration_progress,&progress);
    atomic_store(&signal_context,c);atomic_store(&interrupted,0);
    void (*previous_handler)(int)=signal(SIGINT,interrupt_registration);
    if(previous_handler==SIG_ERR)status=JIP_INTERNAL;
    else {
#if defined(_WIN32) && defined(SIGBREAK)
      void (*previous_break)(int)=signal(SIGBREAK,interrupt_registration);
      if(previous_break==SIG_ERR)status=JIP_INTERNAL;
      else
#endif
      {
      puts("Registration running. Press Ctrl+C to cancel.");fflush(stdout);
      status=atomic_load(&interrupted)?JIP_CANCELLED:stages_given?jip_engine_run(c,config.stages):jip_engine_register(c);
      }
#if defined(_WIN32) && defined(SIGBREAK)
      if(previous_break!=SIG_ERR)signal(SIGBREAK,previous_break);
#endif
      signal(SIGINT,previous_handler);
    }
    atomic_store(&signal_context,NULL);jip_engine_set_trace(c,NULL,NULL);
  }
  if(status==JIP_OK)status=jip_engine_export(c,output,field);
  if(status==JIP_OK&&checkpoint)status=jip_engine_checkpoint_save(c,checkpoint);
  if(status==JIP_OK){jip_image im;float score;jip_engine_result(c,&im,&score);printf("Saved aligned image on fixed grid; objective %.9g\n",score);jip_execution execution={0};execution.struct_size=sizeof(execution);if(!jip_engine_get_execution(c,&execution)){printf("Execution: requested %s, active %s, GPU calls %llu, CPU tile calls %llu\n",execution.requested,execution.active,(unsigned long long)execution.gpu_calls,(unsigned long long)execution.cpu_calls);if(execution.reason[0])puts(execution.reason);}}
  else {
    fprintf(stderr,"Registration failed: %s\n",jip_engine_status_string(status));
    jip_error error;if(c&&!jip_engine_get_error(c,&error)&&error.code==status&&error.message[0])
      fprintf(stderr,"%s: %s (allocation %llu, requested %llu bytes)\n",error.phase,error.message,(unsigned long long)error.allocation_index,(unsigned long long)error.requested_bytes);
  }
  jip_engine_destroy(c);return status==JIP_OK?0:status==JIP_CANCELLED?130:4;
}
