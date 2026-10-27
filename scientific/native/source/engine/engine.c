#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include <float.h>
#include <stdatomic.h>
#include <errno.h>
#include <zlib.h>
#include "fftw3.h"
#include "nifti1_io.h"
#include "util.h"
#define JIP_LEGACY_STORAGE extern
#include "align.h"
#include "jip_engine.h"
#include "engine_runtime.h"
#include "acceleration.h"
#define NBINS 50
#define MAXIT 10
#define JIP_USE_CONTEXT 1
struct jip_engine {
  struct jip_alignment alignment_state;
  struct jip_align_aid aid;
  struct jip_align_nl nl;
  struct data_stack template_stack,source_original,source_stack,target_stack,mimic_stack,bases_stack;
  struct data_stack dx,dy,dz,jac;
  struct mosaic_display mosaic_state;
  struct graph_struct graph_state;
  struct ovl_list overlay_state;
  int time_template,time_source;
  BOOL first_nonlinear,first_affine,first_affine_iteration,first_target,first_source,first_basis;
  IPOINT3D basis_target_dim;
  double nc_i2,nc_t2,nc_it;
  float mean_mimic,mean_target,minimum,maximum;
  JIP_INDEX mi_count,def_count;
  double prob_i[50],prob_t[50],prob_it[50][50],def_sum2;
  float result_objective,affine_objective;
  uint32_t completed_stages;
  uint8_t *fixed_selection,*moving_selection,*sample_selection;
  jip_window moving_windows[3];uint32_t moving_window_count;
  atomic_flag busy;
  atomic_int run_state;
  jip_arena arena;
  struct jip_engine *owner;
  jip_error last_error;
  jip_options options;
  jip_config config;
  jip_trace_callback trace;
  void *trace_user;
  jip_accelerator *accelerator;
  uint64_t source_epoch;
  uint32_t saved_arithmetic,saved_architecture;
  jip_profile profile;
  /* Bounded control cache: never checkpointed, borrowed or heap-owned. */
  int coordinate_cache_ready;IPOINT3D coordinate_cache_dim;mat44 coordinate_cache_matrix;
  float coordinate_cache[3][4096];
  struct {uint32_t bits,width;float values[6];} coefficient_cache[3][256];
  int memoize_coefficients[3];
};
static _Thread_local struct jip_engine *jip_active;
#define Alignment (jip_active->alignment_state)
#define AlignAid (jip_active->aid)
#define AlignNL (jip_active->nl)
#define template (jip_active->template_stack)
#define source_unfiltered (jip_active->source_original)
#define source (jip_active->source_stack)
#define target (jip_active->target_stack)
#define mimic (jip_active->mimic_stack)
#define NL_bases (jip_active->bases_stack)
#define distort_x (jip_active->dx)
#define distort_y (jip_active->dy)
#define distort_z (jip_active->dz)
#define Jac_deter (jip_active->jac)
#define mosaic (jip_active->mosaic_state)
#define graph (jip_active->graph_state)
#define ovl_root (jip_active->overlay_state)
#define iTime_Template (jip_active->time_template)
#define iTime_Source (jip_active->time_source)
static void engine_trace(int event);
static int accelerated_transform(int all_voxels,int most_accurate);
static void acceleration_source_changed(void);
#ifdef JIP_REPRODUCIBLE
#define JIP_ARITHMETIC_ID 2u
#define JIP_ARITHMETIC_NAME "reproducible-no-contract"
#else
#define JIP_ARITHMETIC_ID 1u
#define JIP_ARITHMETIC_NAME "compatibility-native"
#endif
#if defined(__aarch64__) || defined(__arm64__)
#define JIP_ARCHITECTURE_ID 1u
#define JIP_ARCHITECTURE_NAME "arm64"
#elif defined(__x86_64__) || defined(_M_X64)
#define JIP_ARCHITECTURE_ID 2u
#define JIP_ARCHITECTURE_NAME "x86_64"
#else
#define JIP_ARCHITECTURE_ID 0u
#define JIP_ARCHITECTURE_NAME "other-64bit"
#endif
#define JIP_CHECKPOINT() jip_runtime_poll()
/* Rendering hooks only. These never own or mutate numerical state. */
void Update_Regions(void) {}
void refresh_display_image(int area) {(void)area;}
void show_on_info_line(int a,int b,char *c,char *d,char *e) {(void)a;(void)b;(void)c;(void)d;(void)e;}
void action_filter_overlay(void) {} /* no UI overlays in the image-view API */
void action_key_d(void) {}
void update_XWindow_image(void) {}
void update_graph_data(int a) {(void)a;}
void print_parameter_set(int a,int b) {(void)a;(void)b;}
void point_mosaic_to_stack_data(void) {mosaic.stack=&target;mosaic.fSpaceMatrix=target.SpaceTime.fData;}
void select_target(int,IPOINT3D);
void select_source(int);
void flatten_volume(int,struct data_stack*);
#include "engine_geometry.inc"
#include "shared/NL_Basis_function.inc"
#include "shared/ParabolicInterpolation.inc"
#include "shared/SetGridDown.inc"
#include "shared/Set_AlignNL_Value.inc"
#include "shared/action_auto_adjust_affine.inc"
#include "shared/action_auto_adjust_nonlinear.inc"
#include "shared/action_change_display_type.inc"
#include "shared/action_change_resolution.inc"
#include "shared/action_set_increments.inc"
#include "shared/adjust_1basis.inc"
#include "shared/adjust_affine_1pass.inc"
#include "shared/adjust_nonlinear_1pass.inc"
#include "shared/adjust_the_adjust_flags.inc"
#include "shared/align_mimic_target_centroids.inc"
#include "shared/auto_adjust_affine_iterations.inc"
#include "shared/auto_adjust_nonlinear_iterations.inc"
#include "shared/bending_penalty.inc"
#include "shared/calc_NL_basis_function_locations.inc"
#include "shared/calculate_orientation_rotation.inc"
#include "shared/calculate_volume_averages.inc"
#include "shared/calculate_volume_averages_nonlinear.inc"
#include "shared/compute_mixing_matrix.inc"
#include "shared/cost_deformation.inc"
#include "shared/cost_function.inc"
#include "shared/cost_function_nonlinear.inc"
#include "shared/create_basis_function_image.inc"
#include "shared/define_affine_matrix.inc"
#include "shared/define_basis_functions.inc"
#include "shared/define_cosine_sine.inc"
#include "shared/define_slice_matrix.inc"
#include "shared/determine_adjustment_order.inc"
#include "shared/flatten_volume.inc"
#include "shared/ft_1d.inc"
#include "shared/get_NL_increment.inc"
#include "shared/get_NL_increment_old.inc"
#include "shared/get_starting_increments.inc"
#include "shared/index4d.inc"
#include "shared/index4d_stack.inc"
#include "shared/initialize_adjustment_order.inc"
#include "shared/initialize_alignment.inc"
#include "shared/initialize_data_stacks.inc"
#include "shared/line_scan_1d.inc"
#include "shared/line_scan_1d_nonlinear.inc"
#include "shared/mutual_information.inc"
#include "shared/my_mergesort.inc"
#include "shared/number_NL_basis_functions.inc"
#include "shared/number_invalid_distortions.inc"
#include "shared/select_source.inc"
#include "shared/select_target.inc"
#include "shared/smooth_or_sharpen_isotropic.inc"
#include "shared/transform_coordinates.inc"
#include "shared/update_alignment.inc"
#include "shared/update_display_set.inc"
#include "shared/update_nonlinear_alignment_parameters.inc"
#include "shared/voxel3D_to_1D.inc"
static void engine_trace(int event) {
  jip_runtime_poll();
  if(!jip_active||!jip_active->trace)return;
  jip_trace t={event,AlignNL.iGridDown,AlignNL.NBasis,AlignNL.iBasis,target.n3d,
    AlignAid.CF_NonLinear,AlignAid.CF_Affine,(const float*)AlignNL.value,
    {distort_x.SpaceTime.fData,distort_y.SpaceTime.fData,distort_z.SpaceTime.fData}};
  jip_active->trace(&t,jip_active->trace_user);
  jip_runtime_poll();
}
#include "engine_api.inc"
#include "engine_accel.inc"
