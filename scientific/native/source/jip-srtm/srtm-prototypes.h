// The following structure provides an interface to other programs that
// need the basic information stored in the SRTM control file.
#define MAX_RUNS1 50             // Max number of runs
struct SRTM_INPUT
{
  int notify;                          // turn on output information
  int multiple_comparisons;            // number of resolution elements
  int nruns;                           // number of runs
  BOOL nifti_output;                   // if true, all P-x, T-x, S-x files will use NIFTI format
  char condition_string[LEN_STR];      // list of test conditions
  char reference_region_file[LEN_STR]; // reference region overlay file name (required)
  char target_region_file[LEN_STR];    // target region overlay file name    (suggested)
  char run_name[MAX_RUNS1][LEN_STR];   // file name for run
  char glm_name[MAX_RUNS1][LEN_STR];   // file name for GLM description of run
  char table_name[MAX_RUNS1][LEN_STR]; // name of table file for each run
  BOOL append[MAX_RUNS1];              // append to existing columns, or create new columns
} SRTM_input;

/* functions: */

//double ibeta(double aa, double bb, double xx);   // from cephes
int SRTM_read_control_file( BOOL verbose, char *file_name );
int SRTM_read_GLM_file( char *file_name, int irun );
int SRTMInternal_decode_keyword( char *line );
int SRTM_read_table_file( char *file_name, int irun );
int read_ROIs_from_file( BOOL verbose,  int iRun, char *SRTM_FileName );
int read_xdisplay_data_file( char *FileName, struct XD_HEADER header, float *data );
int read_overlay_mask( char *FileName, IPOINT3D dim, float *weight_vector );
int SRTM_define_static_basis_functions();
int SRTM_define_dynamic_basis_functions( int ntime, float *tissue_vector );
void SRTMInternal_describe_glm_format();
int SRTM_fit_data( int ntime, float *data_vector, float *fit_vector, float *sigma2  );
int SRTM_define_conditions( char *condition_string );
int SRTM_define_conditions_old( BOOL DefineContrasts );
int SRTM_calculate_F(int ntime, float sigma2, JIP_INDEX resels, int lCondition,
		    float *cnr, float *p_raw, float *p_cor, float *effect_size, float *effect_sdev);
void SRTM_describe_input_files();
void SRTM_describe_basis_functions();
void SRTM_describe_conditions();
void SRTM_describe_fit();
int SRTM_get_condition_name( int ncondition, char *condition_name );
int SRTM_is_initialized();
int SRTM_get_nruns();
int SRTM_get_nconditions();
int SRTM_get_number_coefficients();
int SRTM_get_time_points();
int SRTM_get_target_region( int ntime, float *target_region );
float SRTM_get_effect_size(int index_coeff );
float SRTM_get_maximal_effect_size(int lCondition );
void SRTM_get_start_stop_times( int iRun, int *istart, int *istop);
int SRTM_get_DOF();
int SRTM_binding_fit( int icoeff, int itime, float *fit );
int SRTM_coeff_fit( int icoeff, int itime, float *fit );
int SRTM_EventNumberFromChar(char event_char);
int SRTM_EventCharFromInteger(int event_id, char *event_char);
char SRTM_EventChar(int event_id);
int SRTM_EventStringFromInteger(int event_id, char *event_string);
float SRTM_time_total_minutes( int itime );
float SRTM_time_run_minutes( int itime );
int SRTM_event_is_defined( int event_id );
void SRTM_define_residue_from_inclusion(char *include_string, char *residue_string);
int SRTM_invalid_residue(char *residue_string);
int SRTM_event_is_in_string(char *string, int index_event);
int SRTM_create_residue( int ntime, float *data_vector, float *fit_vector );
int SRTM_create_selective_residue( char *residue_string, int ntime, float *data_vector, float *fit_vector );
int SRTM_increment_event_id( int ievent );
int SRTM_get_run_number( int iTime );
int SRTM_get_run_number_from_time( float Time_total );
int SRTM_get_itime_in_run( int iTime_total );
int SRTM_get_itime_in_run_from_time( float Time_total );
int SRTM_get_itime_from_time( float Time_total );
int SRTM_set_inclusion_cutoff(int irun, int icutoff);
int SRTM_get_ignored_cutoff( int iRun );
int SRTM_get_number_events_inside_residue( char *residue_string );
float SRTM_get_parameter_value( int iParameter, int iTime_total );
float SRTM_get_parameter_value_from_time( int iParameter, float Time_total );
BOOL SRTMInternal_BP_condition( int iCondition );
BOOL SRTM_BP_condition( int iCondition );
float SRTM_calculate_BP( int iCondition, float sigma2, float *BP_error );
int SRTM_lSingleCondition( int iCoeff );


double ibeta(double aa, double bb, double xx);   // from cephes
