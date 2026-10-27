// The following structure provides an interface to other programs that
// need the basic information stored in the GLM control file.
#define MAX_RUNS1 50             // Max number of runs
struct GLM_INPUT
{
  int normalize_runs;                  // 0 = none, 1 = BOLD, 2 = IRON
  int nbaseline_terms;                 // # of terms to fit to baseline (1->4)
  int notify;                          // turn on output information
  int multiple_comparisons;            // number of resolution elements
  int AR_order;                        // order for auto-regressive noise correlation model
  float AR_smooth_mm;                  // width of smoothing kernal (mm) for AR maps
  BOOL nifti_output;                   // if true, all P-x, T-x, S-x files will use NIFTI format
  char HRF_file_name[LEN_STR];         // file name for impulse response function
  char condition_string[LEN_STR];      // list of statistical conditions
  int nruns;                           // number of runs
  char run_name[MAX_RUNS1][LEN_STR];   // file name for run
  char glm_name[MAX_RUNS1][LEN_STR];   // name of glm timing file for each run
  char table_name[MAX_RUNS1][LEN_STR]; // name of table file for each run
} GLM_input;

BOOL Verbose;

/* functions: */
int GLM_read_control_file( char *file_name, int notify );
int GLM_read_hrf_file( int notify, char *file_name );
int GLM_read_event_file( char *file_name, int irun );
int GLM_read_table_file( char *file_name, int irun );
int  GLM_initialize( int ntime, int nruns );
void GLM_initialize_run( int index_run, float start, float stop );

void GLM_describe_input_files();
void GLM_describe_stimuli();
void GLM_describe_fit();
void GLM_describe_basis_functions();
void GLM_describe_conditions();
void GLMInternal_describe_glm_format();
void GLMInternal_describe_event_format( int event_shape );
void GLMInternal_drift_correct_table( int irun, int icolumn, int nterms );

int GLM_define_basis_functions();
int GLM_define_table_event_interactive(int event_number, float *table_data);
int GLM_define_1_stimulus(int ievent, int ishape,
			  float TimePoint_on, float TimePoint_off, float TimePoint_tau);
int GLM_delete_1_stimulus(int event_number, float location_TimePoint);
int GLM_identify_time_point(int iTime, char *residue_string, char *output_string);
int GLMInternal_create_table(int ievent, int nsteps_per_tr, float *paradigm);
void GLMInternal_create_interaction(int index_event, float *paradigm);
int GLMInternal_create_stimulus_paradigm(int ievent, int nsteps_per_tr, float *paradigm);
float GLMInternal_normalize_paradigm( int ntime, float *paradigm, float max_value);
int GLMInternal_convolve_paradigm_IRF( int convolution_flag, int nsteps_per_tr,
				       float *paradigm, int index_event );
int GLM_generate_synthetic_data1( int ibasis, float contrast_scale, float snr, int ntime, float *simdata );
int GLM_generate_synthetic_data2( int ibasis1, int ibasis2, float contrast_scale1, float constrast_scale2,
				  float noise_max, int ntime, float *simdata );

int GLM_fit_data( int ntime, float *data_vector, float *fit_vector, float *sigma2  );
int GLM_remove_baseline( int option, int ntime, float *data, float *fit );
int GLM_correct_baseline( int ntime, float *data_vector, float *fit_vector, float *average );
void GLM_define_table_residue(char *residue_string);
int GLM_create_selective_residue( char *residue_string, int ntime, float *data, float *fit );
int GLM_create_residue( int ntime, float *data_vector, float *fit_vector );
int GLM_event_fit( int baseline, char *residue_string, int index_event, int index_time, float *fit );
int GLM_create_all_conditions( char *condition_string );
int GLM_define_conditions( int DefineContrasts, char *condition_string, int *nconditions );
int GLM_get_condition_string( int ncondition, char *condition_string );
int GLM_calculate_F(int ntime, float sigma2, JIP_INDEX resels,
		    int lCondition, float *cnr, float *p_raw, float *p_cor,
		    float *effect_size, float *effect_sdev);
int GLM_calculate_relative_CNR( int lCondition, float *cnr );
int GLM_calculate_unbiased_efficiency( float *eff );
int GLM_get_number_events();
int GLM_get_nconditions();
int GLM_get_nruns();
int GLM_get_nbaseline_terms();
int GLM_set_nbaseline_terms( int nbaseline );
int GLM_get_event_number(int index_event);
int GLM_get_event_index(int event_number);
int GLM_get_DOF();
float GLM_get_time_constant(int index_event );
void GLM_set_time_constant( int index_event, float tau );
float GLM_get_effect_size(int index_coeff );
float GLM_get_maximal_effect_size(int lCondition);
int GLM_get_tr( int irun, float *tr );
int GLM_get_stimulus(int index_event, float *stim_on, float *stim_off);
int GLM_increment_event_number( int ievent );
int GLM_write_glm_files( char *dat_file_name, char *stim_file_root );
int GLM_set_paradigm_shape( int event_number_input, int event_shape );
int GLM_get_paradigm_shape( int event_number );
void GLM_modify_excluded_region( int mode, int start, int end );
int GLM_get_number_excluded_regions();
int GLM_get_excluded_region( int index_exclude, int *exclude_start, int *exclude_end);
int GLM_is_excluded( int index_time );
int GLM_is_initialized();
int GLM_fit_per_run();
int GLM_invalid_residue(char *residue_string);
int GLM_get_number_events_inside_residue( char *residue_string );
int GLM_event_is_in_string( char *residue_string, int index_event );
void GLM_define_residue_from_inclusion(char *include_string, char *residue_string);
int GLM_event_is_defined( int event_number );
int GLM_get_number_events();
int GLM_get_event(int index_event);
int GLM_get_event_index(int event_number);
int GLMInternal_decode_keyword( char *line );
void GLMInternal_whiten_data(float *data);
void GLMInternal_whiten_model( int mode );
int GLM_calculate_whitening_filter(int ndim, float *auto_cor, short *norder);
int GLM_EventNumberFromChar(char event_char);
int GLM_EventCharFromInteger(int event_number,char *event_char);
char GLM_EventChar(int event_number);
int GLM_EventStringFromInteger(int event_number, char *event_string);

void GLMInternal_calculate_AR_mixing_matrix();
int GLM_calculate_AR_coefficient( int ntime, float *data_vector, int norder, float *auto_cor );
float Trace_of_Matrix( int NSquare, float **RealMatrix );
int smooth_AR_maps();
void GLMInternal_smooth_data( int ntime, float *vector );

double ibeta(double aa, double bb, double xx);   // from cephes
