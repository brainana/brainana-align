#ifndef JIP_ENGINE_H
#define JIP_ENGINE_H
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
#define JIP_ENGINE_VERSION "0.8.3"
typedef struct {
  uint32_t struct_size,threads,budget_mb,map_arithmetic,map_architecture;
  char requested[16],active[16],device[160],reason[320];
  uint64_t calls,gpu_calls,cpu_calls,fallbacks,device_bytes;
  double gpu_seconds,cpu_seconds;
} jip_execution;
/* Execution choices are context-local and never change the optimizer. GPU
   absence/errors use CPU; get_execution explains the actual route taken. */
typedef struct jip_engine jip_engine;
/* Optional diagnostics ABI: inclusive monotonic wall timers, never numerical
   state/checkpoint data. struct_size keeps older callers independent. */
typedef struct {
  uint32_t struct_size;
  uint64_t transform_calls,metric_calls,basis_calls,prepared_samples;
  uint64_t workspace_bytes,workspace_allocations;
  double run_seconds,transform_seconds,metric_seconds,basis_seconds;
  double prepare_seconds,dispatch_seconds,scatter_seconds;
} jip_profile;

typedef enum { JIP_OK=0,JIP_INVALID=1,JIP_IO=2,JIP_BUSY=3,JIP_CANCELLED=4,JIP_MEMORY=5,JIP_INTERNAL=6 } jip_status;
jip_status jip_engine_set_execution(jip_engine*,const char *backend,uint32_t threads,uint32_t budget_mb);
jip_status jip_engine_get_execution(jip_engine*,jip_execution*);
jip_status jip_engine_get_profile(jip_engine*,jip_profile*);
void jip_engine_devices(char *json,size_t capacity);
const char *jip_engine_arithmetic(void);
const char *jip_engine_build_info(void);
typedef struct {
  jip_status code;
  char phase[64],message[256];
  uint64_t requested_bytes,allocation_index;
} jip_error;
/* Defaults preserve 0.2.0 nonlinear-only behavior. Struct size is required;
   initialize with jip_config_default before changing fields. Settings are
   immutable for a context: create a new context to change preprocessing. */
enum { JIP_STAGE_AFFINE=1, JIP_STAGE_NONLINEAR=2 };
enum { JIP_AFFINE_MI=0, JIP_AFFINE_NMI=1 };
typedef struct {
  uint32_t struct_size,stages;
  int32_t affine_model;       /* 6 rigid, 9 independent scales, 12 full affine */
  int32_t affine_metric;      /* MI or NMI; nonlinear metric remains in options */
  int32_t nonlinear_axes[3];  /* each 0/1; at least one for nonlinear runs */
  int32_t matching_volume;    /* 0 nonzero fixed voxels; 1 entire fixed grid */
  int32_t wrap_around[3];     /* legacy source wrapping, each 0/1 */
} jip_config;
typedef struct {
  uint32_t completed_stages;
  float affine_parameters[12],centered_pull_affine[16];
  float fixed_voxel_to_native_mm[16],moving_voxel_to_native_mm[16];
  float fixed_voxel_to_centered_mm[16],moving_centered_mm_to_voxel[16];
  const float *displacement[3]; /* borrowed centered-mm arrays on fixed grid */
  uint64_t count;
  float nonlinear_objective,affine_objective;
} jip_transform;
/* Contiguous float32, x fastest. Spacing is positive mm; native_affine is a
   row-major voxel-to-physical matrix. Inputs must have finite, nonuniform
   intensities. Inputs are copied; caller retains them. */
typedef struct {
  int32_t dim[3];
  float spacing[3],native_affine[16];
  const float *data;
  uint64_t count;
} jip_image;
typedef struct {
  int metric;                 /* 0: original JIP MI, 1: uncentered NC */
  int grid_down;              /* 1,2,4,8,16; original nonlinear hierarchy */
  int regularization;         /* legacy Lambda_reg, 0..46340 (square fits int) */
  float jacobian_limit;       /* reciprocal lower and upper limits, >1 */
  float stiffness;            /* original 0..1 bending/stiffness control */
  float affine_parameters[12];/* JIP centered pull convention, NOT world RAS */
} jip_options;
/* Event views are borrowed for the duration of the callback. Never call the
   same context from its callback. A separate context may be used there. */
typedef struct {
  /* Events: 1 nonlinear coefficient, 2 nonlinear pass, 3 basis rebuild,
     4 final trial, 5 affine pass, 6 affine completed (before transaction commit).
     A callback notification is a proposal view, not an accepted-result signal. */
  int event,grid_down;
  int64_t basis_count,basis_index,voxel_count;
  float objective,image_objective;
  const float *coefficients_xyz,*displacement[3];
} jip_trace;
typedef void (*jip_trace_callback)(const jip_trace*,void*);
/* Optional immutable inclusion masks (1 included, 0 excluded), on each
   supplied working grid. Copied during creation. Membership gates metric
   samples and normalization, never interpolation taps or exported intensities.
   Moving membership follows the current pull at nearest voxel centers. */
typedef struct {float minimum[3],maximum[3];uint32_t axes;} jip_window;
typedef struct {uint32_t struct_size;const uint8_t *fixed,*moving;uint64_t fixed_count,moving_count;const jip_window *moving_windows;uint32_t moving_window_count;} jip_sampling;
jip_status jip_engine_create_sampled(const jip_image*,const jip_image*,const jip_options*,const jip_config*,const jip_sampling*,jip_engine**);
void jip_options_default(jip_options*);
void jip_config_default(jip_config*);
jip_status jip_engine_create_configured(const jip_image*,const jip_image*,
                                        const jip_options*,const jip_config*,jip_engine**);
jip_status jip_engine_get_config(jip_engine*,jip_config*);
jip_status jip_engine_get_transform(jip_engine*,jip_transform*);
/* Run selected stages in one transaction. register() uses config.stages.
   Affine uses the original full-resolution parameter hierarchy, then nonlinear
   uses the unchanged basis hierarchy. No new automatic orientation search. */
jip_status jip_engine_run(jip_engine*,uint32_t stages);
jip_status jip_engine_create(const jip_image *fixed,const jip_image *moving,
                             const jip_options*,jip_engine**);
void jip_engine_destroy(jip_engine*);
/* Checked destruction refuses active jobs. Callers still own synchronization
   of handle lifetime: no API may be called after successful release. */
jip_status jip_engine_release(jip_engine*);
jip_status jip_engine_set_trace(jip_engine*,jip_trace_callback,void*);
jip_status jip_engine_register(jip_engine*);
/* Cancels the active registration only; an idle request returns JIP_INVALID.
   This call performs only lock-free atomics and may run on another thread. */
jip_status jip_engine_request_cancel(jip_engine*);
jip_status jip_engine_get_error(jip_engine*,jip_error*);
jip_status jip_engine_result(jip_engine*,jip_image *aligned,float *objective);
/* Preview/checkpoint API is in development. The runner exports a float32
   aligned NIfTI on the fixed grid and a three-volume JIP displacement NIfTI. */
jip_status jip_engine_export(jip_engine*,const char *aligned_path,const char *field_path);
jip_status jip_engine_checkpoint_save(jip_engine*,const char*);
jip_status jip_engine_checkpoint_load(const char*,jip_engine**);
const char *jip_engine_status_string(jip_status);
#ifdef __cplusplus
}
#endif
#endif
