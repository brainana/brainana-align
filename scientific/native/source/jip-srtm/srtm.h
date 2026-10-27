// This file contains the SRTM-GLM structure.
// For data analysis, there is no need to include this file
// into the calling program, as the functions in srtm.c should handle
// everything without the calling program needing to know details.
// -------------------------------------------------------------------------------------
// Each event has a separate basis function, corresponding to an event number.
#define MAX_RUNS          50    // Max number of runs
#define MAX_CONDITIONS   100    // # conditions that can be tested in one data pass
#define MAX_COLUMNS       20    // # columns in a table file; almost no memory used here
#define MAX_COEFF         61    // # a-z (26) + A-Z (26) + 1-9 (9)
#define MAX_PERIODS       20    // # separate on periods for given event

struct srtm_glm_struct
{
  ////////////////////////////////////////////////////////////////////////////////////////////
  // General Linear Model (GLM):
  //
  // Y_t (data) = X_tc * Beta_c
  //
  // where
  //        Y_t     = data of dimension time (t)
  //        X_tc    = design matrix
  //        Beta_c  = hemodynamic response function of dimension c (coefficients)
  //
  ////////////////////////////////////////////////////////////////////////////////////////////

  ///////////////////////////
  // Constants //////////////
  ///////////////////////////
  int initialized;           // 0 = uninitialized, 1 = read control file, 2 = defined static bases, 3 = defined all bases
  int ntime;                 // IN: # of time points TOTAL including concatenations across runs
  int ncoeff;                // IN: # columns in design matrix = # coefficients in beta = # events
  int nruns;                 // IN: # of runs
  /////////////////////////////
  // per run //////////////////
  /////////////////////////////
  int nt[MAX_RUNS];               // IN: # time steps per run
  int run_istart[MAX_RUNS];       // IN: the starting time index for each run (in a appended series of runs)
  int run_istop[MAX_RUNS];        // IN: the stopping time index for each run (in a appended series of runs)
  int table_ncolumns[MAX_RUNS];   // IN: # of columns in an optional table for each run
  int table_nt[MAX_RUNS];         // IN: # of columns in an optional table for each run
  char table_column_name[MAX_COLUMNS][LEN_STR];// names of column headers in table
  float **table_value[MAX_RUNS];  // values in table for each run; dimensions are [table_ncolumns][table_ntime]
  float *reference_region[MAX_RUNS]; // Data from reference region overlay; this is necessary for SRTM
  float *ref_region_deriv[MAX_RUNS]; // derivative of reference region
  float *target_region[MAX_RUNS]; //    Data from target region. This is optional and only used for basis.dat
  int inclusion_cutoff[MAX_RUNS]; // Ignore data in each run past this "challenge" point
  float k2prime[MAX_RUNS];        // either 0 (fit k2' for each voxel) or a global constant
  /////////////////////////////
  // per event/coefficient ////
  /////////////////////////////
  // The index of the following variables in the event index
  int event_id[MAX_COEFF];         // IN: event identification
  int event_type[MAX_COEFF];       // IN: type: 0=R1 , 1=k2a, 2=k2AB (constants)
  int event_shape[MAX_COEFF];      // IN: see fields below
  int nperiods[MAX_COEFF];         // IN: # of on/off cycles across all runs for a given event id
  int icolumn[MAX_COEFF];          // IN: tau for shape gamma
  float tau[MAX_COEFF];            // IN: tau for shape gamma
  int iTimeMax[MAX_COEFF];         // COMPUTED: time point with max value for table condition
  /////////////////////////////
  // per event, period ///
  /////////////////////////////
  float period_start[MAX_COEFF][MAX_PERIODS];  // IN: the ON  time for each stimulus (total time across concatenated runs)
  float period_stop [MAX_COEFF][MAX_PERIODS];  // IN: the OFF time for each stimulus (total time across concatenated runs)
  /////////////////////////////
  float dof;               // CALC: # degrees of freedom (usually ntime - ncoeff)
  ///////////////////////////
  // time & coeff arrays ////
  ///////////////////////////
  int *include_time;    // IN:   array of values (between 0 & 1) for excluding data [ntime]
  float *target_t;         // IN:   the target region to be analyzed prior to the volumetric data
  float  *Weight_t;        // IN:   array of values (between 0 & 1) for excluding data [ntime]
  float **Xorig_tc;        // CALC: the basis functions (original)     - [ntime][ncoeff]
  float **Xmix_tc;         // CALC: mixed functions for S & NS binding - [ntime][ncoeff]
  float **X_tc;            // CALC: the basis functions (filtered)     - [ntime][ncoeff]
  float **XTXm1_cc;        // CALC: (X_transpose * X)^-1               - [ncoeff][ncoeff]
  float **XTXm1XT_ct;      // CALC: (X_transpose * X)^-1 * X_transpose - [ncoeff][ntime]
  float  *Beta_c;          // CALC: coefficients from fit              - [ncoeff]
  ///////////////////////////
  // statistical conditions //
  ///////////////////////////
  int nconditions;                       // the number of conditions to be tested
  int ncontrasts[MAX_CONDITIONS];        // # of contrasts to test in each condition (T test for 1, F test for > 1)
  int **contrast_matrix[MAX_CONDITIONS]; // the condition vector of 0s, 1s, and -1s; indices = [ncontrasts][ncoeff]
  char condition_name[MAX_CONDITIONS][LEN_STR];
  float **FCovar[MAX_CONDITIONS];        // covariance matrix in T denomenator; indiced = [ncontrasts][ncontrasts]
  float **FCovarInv[MAX_CONDITIONS];     // inverse of covariance matrix in F numerator; indiced = [ncontrasts][ncontrasts]
  BOOL BP_condition[MAX_CONDITIONS];     // valid condition to compute BP
  int k2_condition_for_BP[MAX_CONDITIONS]; // k2 condition (if it exists) used to compute error bars for BP (k2_prime variable)
  int R1_condition_for_BP[MAX_CONDITIONS]; // R1 condition (if it exists) used to compute error bars for BP (k2_prime fixed)
  // Overlay masks
  float *Mask_Reference;  // reference region mask (from overlay file)
  float *Mask_Target;     // target region mask (from overlay file)
} SRTM;

// enumerations

enum GLM_types
  {
    TYPE_R1  = 0, // R1   : static  basis function dependent upon ref region
    TYPE_k2  = 1, // k2   : dynamic basis function dependent upon integral of ref region    (k2  = R1*k2')
    TYPE_k2a = 2  // k2a  : static  basis function dependent upon integral of target region (k2a = R1*k1'/[1+B])
  };
enum GLM_shapes
  {
    SHAPE_CONSTANT = 0,
    SHAPE_GAMMA    = 1,
    SHAPE_GAMMAINT = 2,
    SHAPE_SIGMOID  = 3,
    SHAPE_TABLE    = 4
  };

#include "srtm-prototypes.h"
