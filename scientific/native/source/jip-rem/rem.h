// This file contains the REM data structures.
// For data analysis, there is no need to include this file
// into the calling program, as the functions in glm.c should handle
// everything without the calling program needing to know details.
// -------------------------------------------------------------------------------------
#define MAX_EVENTS       100    // Max number of events: 1 for each (group,contrast)
#define MAX_STIMS        100    // Max stims per event; for simulations, make this a big number
#define MAX_CONDITIONS   100    // # conditions that can be tested in one data pass

struct glm_struct
{
  ////////////////////////////////////////////////////////////////////////////////////////////
  // General Linear Model (REM) procedure following Worsley et al., NeuroImage 2002
  // "A general statistical analysis for fMRI data.
  // This program is like Worsely's "Multistat"
  //
  // Y_s (data) = X_sk (design) * Beta_k (coefficients)
  //   ^samples
  // with residuals including fixed and random effects.
  //
  // "Samples" here means a vertical concatentation of runs or subjects.
  ////////////////////////////////////////////////////////////////////////////////////////////

  ///////////////////////////
  // Constants //////////////
  ///////////////////////////
  int initialized;           // 0,1 0 = uninitialized, 1 = initialized
  int ntime;                 // IN: # of "time points", including all events, subjects, samples
  int nevents;               // IN: # stimulus events
  int ncoeff;                // CALC: # coefficients in GLM fit; usually = nevents
  /////////////////////////////
  // per event/////////////////
  /////////////////////////////
  int event_number[MAX_EVENTS]; // IN: stimulus number
  int event_shape[MAX_EVENTS];  // IN: the event shape
  int nstims[MAX_EVENTS];       // IN: number of stimuli per event
  int nsamples[MAX_EVENTS];     // IN: number of samples in each event (subjects or runs)
  /////////////////////////////
  // per event, stim //////////
  /////////////////////////////
  int event_start[MAX_EVENTS][MAX_STIMS];  // IN: the starting time index for each event in a appended series
  int event_stop[MAX_EVENTS][MAX_STIMS];   // IN: the stopping time index for each event in a appended series
  /////////////////////////////
  float dof[MAX_CONDITIONS];    // CALC: # degrees of freedom in condition
  ///////////////////////////
  // time & coeff arrays ////
  //
  // _tc means dimensions are [time][coefficient]
  ///////////////////////////
  float  *Beta_c;      // CALC: coefficients from fit
  float **X_tc;        // CALC: the basis functions
  float **SIGm1_tt;    // CALC: the inverse of the variance matrix (FE + RE)
  float **Xw_tc;       // CALC: the basis functions (weighted): SIGm1 * X
  float **XTXw_cc;     // CALC: X'* Xw
  float **Variance_cc; // CALC: pseudo-inverse of (X' * Xw), or (X' * Xw)+
  float **XTSIGm1_ct;  // CALC: X' * SIG^-1
  float **Product_ct;  // CALC: (X' * Xw)+ * X'*SIG^-1
  float **XProduct_tt; // CALC: X * (X' * Xw)+ * X'*SIG^-1
  float **Identity_tt; // CALC: the residue matrix: SIG^-1 - SIG^-1 * X * Product
  float **Residue_tt;  // CALC: the residue matrix: SIG^-1 - SIG^-1 * X * Product
  float **Residue2_tt; // CALC: square of residue matrix
  float **SigFE_tt;    // CALC: the diagonal matrix of fixed-effects standard deviations
  float **SigFE2_tt;   // CALC: the square of the above
  float **SigFE2R_tt;  // CALC: SigFE2 * Residue
  ///////////////////////////
  // statistical conditions //
  ///////////////////////////
  int nconditions;         // the number of conditions to be tested
  int ncontrasts[MAX_CONDITIONS];   // # of contrasts to test in each condition (T test for 1, F test for > 1)
  int **contrast_matrix[MAX_CONDITIONS]; // the condition vector of 0s, 1s, and -1s; indices = [ncontrasts][ncoeff]
  char condition_string[MAX_CONDITIONS][LEN_STR];
} REM;

enum REM_shapes
  {
    SHAPE_SQUARE    = 0, // shape used for standard fMRI, assuming HRF convolution
    SHAPE_RAMP      = 1  // ramp (bipolar function)
  };

#include "rem-prototypes.h"
