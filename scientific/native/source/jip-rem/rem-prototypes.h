// The following structure provides an interface to other programs that
// need the basic information stored in the REM control file.
#define MAX_FILES 200    // Max number of signal files
struct REM_INPUT
{
  int nfiles;                          // # of files specified in REM input file
  char  signal_file_name[MAX_FILES][LEN_STR]; // file name for run
  float signal_magnitude[MAX_FILES];   // magnitude assigned to the file
  int multiple_comparisons;            // number of resolution elements
  float smoothed_mm;                   // width (mm) of prior smoothing kernel in analysis
  float smooth_ratio_mm;               // width (mm) of smoothing kernal for variance ratio
  BOOL nifti_output;                   // use nifti files for P-xx, T-xx, S-xx, ... (.nii)
  BOOL global_scaling;                 // true if scaling values are applied to input files
  BOOL overlapping_events;             // true if regressors are not independent
  char condition_string[LEN_STR];      // list of statistical conditions
} REM_input;

BOOL Verbose;

int REM_read_control_file( char *file_name );
int REMInternal_read_control_file_no_overlapping_events(FILE *file);
int REMInternal_read_control_file_overlapping_events(FILE *file);
int REMInternal_decode_keyword( char *line );
void REMInternal_describe_rem_format();
void REMInternal_define_design_matrix();
int REM_REML_iterate( int ntime, float *Signal_vector, float *SigFE_vector, int *lDOF_vector,
		      float *Sigma2_FE, float *Sigma2_RE );
int REM_fit_final( float Sigma2_RE, int ntime, float *Signal_vector, float *SigFE_vector,
		   float *fit_vector, float *fit_vector_final );
int REM_event_fit(int index_event, int npoints, float *fit);
void REMInternal_compute_residue_matrix();
float Trace_of_Matrix( int NSquare, float **RealMatrix );
void Matrix_Product( float **X1X2, int Length1, int Length2,
		     float **X1, int iTranspose1, int Length_12, float **X2, int iTranspose2);
void Matrix_PseudoInverse( float **X_12, int Length1, int Length2, float **piX_21 );
int REM_define_conditions( char *condition_string, int *nconditions );
int REM_calculate_T(int ntime, JIP_INDEX resels, int lCondition, int lDOF_effective,
		    float *cnr, float *p_raw, float *p_cor, float *effect_size, float *effect_sdev);
int REM_calculate_F(int ntime, JIP_INDEX resels, int lCondition, int lDOF_effective,
		    float *cnr, float *p_raw, float *p_cor, float *effect_size, float *effect_sdev);
void REM_describe_input();
void REM_describe_basis_functions();
void REM_describe_fit();
int REM_get_condition_string( int ncondition, char *condition_string );
int REM_is_initialized();
int REM_get_nconditions();
int REM_get_number_events();
int REM_get_event_number(int index_event);
int REM_get_event_index(int event_number);
float REM_get_maximal_effect_size(int lCondition );
int REM_get_number_time_points();
int REM_get_DOF();
int REM_EventNumberFromChar(char event_char);
int REM_EventCharFromInteger(int event_number, char *event_char);
char REM_EventChar(int event_number);
int REM_EventStringFromInteger(int event_number, char *event_string);
IPOINT2D REM_GetEventLimits( int event_number );
int REM_IdentifyEvent( int iEntry, char *event_char, int *nSample );
