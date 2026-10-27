// This file contains the GLM and HRF data structures.
// For data analysis, there is no need to include this file
// into the calling program, as the functions in glm.c should handle
// everything without the calling program needing to know details.
// -------------------------------------------------------------------------------------
// Each event has a separate basis function, corresponding to an event number.
#define MAX_EVENTS        62    // Allowed number are 1-9, 10-35 (a-z), and 36-61 (A-Z)
#define MAX_RUNS          50    // Max number of runs
#define MAX_STIMS      10000    // Max stims per event; for simulations, make this a big number
#define MAX_CONDITIONS   100    // # conditions that can be tested in one data pass
#define MAX_COLUMNS       20    // # columns in a table file; almost no memory used here

struct glm_struct
{
  ////////////////////////////////////////////////////////////////////////////////////////////
  // General Linear Model (GLM) procedure:
  //
  // Y (data) = X (SCM) * Beta (coeff)
  //
  // where
  //        Y_t     = data of dimension time (t)
  //        X_tc    = stimulus convolved matrix (SCM) of dimensions time (t) by coefficients (c)
  //        Beta_c  = hemodynamic response function of dimension c (coefficients)
  //
  // To include drifts and other parameters of no interest,
  //  X -> [X D]  and h -> [h d], where D are the basis functions and d the amplitudes.
  ////////////////////////////////////////////////////////////////////////////////////////////

  ///////////////////////////
  // Constants //////////////
  ///////////////////////////
  int initialized;           // 0,1 0 = uninitialized, 1 = initialized
  int ntime;                 // IN: # of time points (tr/duration)
  int nbaseline;             // IN: # baseline basis functions (generally 2 or 3) for all runs
  int nevents;               // IN: # stimulus events
  int ntable_events;         // IN: # tables events; assume effect of no interest
  int nruns;                 // IN: # of runs
  /////////////////////////////
  // per run //////////////////
  /////////////////////////////
  float tr[MAX_RUNS];             // IN: time per point (e.g., per MRI volume)
  int run_start[MAX_RUNS];        // IN: the starting time index for each run (in a appended series of runs)
  int run_stop[MAX_RUNS];         // IN: the stopping time index for each run (in a appended series of runs)
  int table_ncolumns[MAX_RUNS];   // IN: # of columns in an optional table for each run
  int table_ntime[MAX_RUNS];      // length of table vectors, as read from file corresponding to run
  char table_column_name[MAX_COLUMNS][LEN_STR];// names of column headers in table
  float **table_value[MAX_RUNS];  // values in table for each run; dimensions are [table_ncolumns][table_ntime]
  /////////////////////////////
  // per stimulus type ////////
  /////////////////////////////
  // The index of the following variables in the event index
  int event_number[MAX_EVENTS];        // IN: stimulus number
  int event_shape[MAX_EVENTS];         // IN: before HRF: 0 = square, 1 = gamma, [2 = square conv. gamma]
  int nstims[MAX_EVENTS];              // IN: # of stimulus on/off cycles per basis function for each event
  float event_tau1[MAX_EVENTS];        // IN: tau for shape `gamma' OR for shape `drug-IRF' or shape 'sine/cos'
  float event_tau2[MAX_EVENTS];        // IN: tau1 for plasma convolution function (exponential)
  int length_drugIRF[MAX_EVENTS];      // length of drug IRF for `drug-IRF' event shape (5 x tau)
  float *drugIRF[MAX_EVENTS];          // amplitude of `drug-IRF' event shape (length ntime)
  float *plasmaIRF[MAX_EVENTS];        // IN: only active for shape `drug-IRF'
  // The index of the following variables in the event number, NOT the event index
  int convolve_table[MAX_EVENTS];      // convolve table with HRF (default=0)
  int drift_correct_table[MAX_EVENTS]; // drift-correct table event to othogonalize with baseline
  float event_scale[MAX_EVENTS];       // IN: magnitude of event (regressor scaled to this value, not 1)
  /////////////////////////////
  // per stim. type, stim # ///
  /////////////////////////////
  float stim_on [MAX_EVENTS][MAX_STIMS];      // IN: the ON  time for each stimulus
  float stim_off[MAX_EVENTS][MAX_STIMS];      // IN: the OFF time for each stimulus
  float stim_mag[MAX_EVENTS][MAX_STIMS];      // IN: magnitude of stimulus before convolution
  int table_icolumn[MAX_EVENTS][MAX_STIMS];   // the column number for the given event and run
  //                                             each run is treated as new stimulus, with on & off times.
  /////////////////////////////
  // per stimulus # ///////////
  /////////////////////////////
  int nexclude;                 // IN:     # of exclusion on/off cycles
  int exclude_start[MAX_STIMS]; // IN: the starting time point for each segment to exclude
  int exclude_end[MAX_STIMS];   // IN: the ending   time point for each segment to exclude
  /////////////////////////////
  // basis functions:
  // GLM.ncoeff = GLM.nevents + GLM.nbaseline * GLM.nrun
  //            = GLM.nevents + GLM.no_interest
  // order:
  // 1) GLM.nevents   - all addressable events
  // 2) GLM.nbaseline - baseline coefficients
  int ncoeff;
  float dof;               // CALC: # degrees of freedom (usually ntime - ncoeff)
  ///////////////////////////
  // time & coeff arrays ////
  ///////////////////////////
  int *include_time;    // IN:   array of values (between 0 & 1) for excluding data [ntime]
  float **Xorig_tc;        // CALC: the basis functions (non-filtered)       - [ntime][ncoeff]
  float **X_tc;            // CALC: the basis functions (whitened/excluded)  - [ntime][ncoeff]
  float **XTXm1_cc;        // CALC: (X_transpose * X)^-1                     - [ncoeff][ncoeff]
  float **XTXm1XT_ct;      // CALC: (X_transpose * X)^-1 * X_transpose       - [ncoeff][ntime]
  float  *Beta_c;          // CALC: coefficients from fit                    - [ncoeff]
  /////////////////////////////
  // auto-correlation arrays //
  /////////////////////////////
  float **WhiteFilter_tt;  // CALC: whitening matrix/filter                  - [ntime][ntime]
  float **ResMat_tt;       // CALC: the residue matrix: I - X (X'X)^-1 X'    - [ntime][ntime]
  float **ARMixm1_rr;      // CALC: the AR mixing matrix M^-1 (Worsley)      - [AR_order][AR_order]
  int AR_order;            // CALC: this is the AR order for a given pixel (<= GLM_input.AR_order)
  ///////////////////////////
  // statistical conditions //
  ///////////////////////////
  int nconditions;                  // the number of conditions to be tested
  int ncontrasts[MAX_CONDITIONS];   // # of contrasts to test in each condition (T test for 1, F test for > 1)
  int **contrast_matrix[MAX_CONDITIONS]; // the condition vector of 0s, 1s, and -1s; indices = [ncontrasts][ncoeff]
  char condition_string[MAX_CONDITIONS][LEN_STR];
  float **FCovar[MAX_CONDITIONS];    // covariance matrix in T denomenator; indiced = [ncontrasts][ncontrasts]
  float **FCovarInv[MAX_CONDITIONS]; // inverse of covariance matrix in F numerator; indiced = [ncontrasts][ncontrasts]
} GLM;

struct hrf_struct
{
  int unbiased;            // flag: 0 or 1
  float time_length;       // the time duration of the HRF
  float time_step;         // the time step
  int npoints;             // the number of time points in the HRF (length/step)
  int nexp;                // # exponentials in biased HRF (usually 3)
  int nhrf;                // # basis functions for HRF (=1 for biased)
  float *tau;              // HRF time constants for each impulse function [nhrf]
  float *weight;           // the HRF basis function weights [nhrf]
  float *amp;              // the hemodynamic (HRF) basis function amplitude [npoints]
  char file_name[LEN_STR];
} HRF;

// enumerations

enum GLM_shapes
  {
    SHAPE_SQUARE    = 0, // shape used for standard fMRI, assuming HRF convolution
    SHAPE_GAMMA     = 1, // shape used for phMRI, with or without hemodynamic HRF
    SHAPE_SIGMOID   = 2, // shape used for phMRI, with or without hemodynamic HRF
    SHAPE_TABLE     = 3, // use a table entry
    SHAPE_DRUGIRF   = 4, // shape used for phMRI; provides FIR convolution with inject(t) or plasma(t)
    SHAPE_SINE      = 5, // sinusoidal event
    SHAPE_COSINE    = 6, // cosine event
    SHAPE_RAMPUP    = 7, // ramp up
    SHAPE_RAMPDOWN  = 8, // ramp down
    SHAPE_INTERACT  = 9  // interaction term: product of two other basis functions
  };

#include "glm-prototypes.h"
