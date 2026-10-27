#include "common-display.h"
#include "common-align.h"

#define CURSOR_ON_INDIVIDUAL_TEMPLATE (1<<26) // individual/template toggle text
#define CURSOR_ON_SELECT_PARAMETERS   (1<<27) // a text field to simplify parameter selection
#define CURSOR_ON_COST_FUNCTION       (1<<28) // a text field to show cost functions
#define CURSOR_ON_RESET_RECALL        (1<<29) // a text field to show cost functions
#define CURSOR_ON_AUTO_ALIGN          (1<<30) // a text field to show cost functions
#define CURSOR_ON_DO_EVERYTHING       (1<<31) // a text field to show cost functions

struct
{ // NAFFINE from align.h
  int DisplayType;            // 0 = individual, 1 = template, 2 = NL-x, 3=NL-y, 4=NL-z
  int iParameter;             // currently selected parameter (0 to NAFFINE-1)
  int CostFunction;           // 0 = mutual information, 1 = normalized correlation
  BOOL ChangeValue;           // if TRUE, change value of iParameter; otherwise, change increment
  char name[LEN_STR][NAFFINE];// name of the parameter
  BOOL Adjust[NAFFINE];       // 1=adjust during iterations, 0 = ignore
  float increment[NAFFINE];   // increment for the parameter value (ignored when >= NAFFINE)
  int iFirstParShown;         //
  int iLastParShown;          //
  int DisplaySet;             // 0 = orientation, 1 = affine, 2 = non-linear
  IPOINT3D OrientRotation;    // rotations in x, y, and z for initial orientation
  IPOINT3D SubSampleSetting;  // sub-sampling displays settings in x, y, z
  IPOINT3D SubSample;         // current sub-sampling in x, y, z (to increase speed of alignment)
  BOOL LowResolution;         // true in sub-sampled state
  int smooth_fov_fraction;    // smoothing kernal width as a fraction of the FOV
  float saved_value[NAFFINE]; // saved parameter values for reset/recall (0,1 for non-linear params)
  int nmask;                  // # of points in current alignment mask (overlay)
  float *mask;                // mask; dimensions match alignment.matrix
  float CF_Affine;            // Cost function without non-linear penalties
  float CF_NonLinear;         // Cost function with    non-linear penalties
  mat44 ijk_to_xyz;           // data indices (i,j,k) to space (x,y,z) in aligned,     centered coordinates
  mat44 xyz_to_ijk;           // space (x,y,z) to data indices (i,j,k) in non-aligned, centered coordinates
} AlignAid;

struct
{
  // not-indexed
  IPOINT3D Adjust;          // 1=adjust during iterations, 0 = ignore
  IPOINT3D Adjusted;        // 1=adjust during iterations, 0 = ignore
  IPOINT3D GridDown;        // grid downsampling for basis functions in x, y, z (based upon iGridDown)
  IPOINT3D NBasis1D;        // number of basis functions in (x,y,z)
  int      NBasis;          // total number of basis functions (product of x,y,z)
  int      iGridDown;       // basis function down-sampling factor
  int      iBasis;          // current basis function (0 to NBasis)
  int      iCoeff;          // current basis function (0 to 3*NBasis)
  int  Lambda_reg;          // regularization constant
  int  Jacob_limits;        // Jacobian limits (n and 1/n)
  float stiffness;          // sets sliding scale between 1st and 2nd derivatives in cost function
  int ViewBasis;            // 0/1
  // vectors of length NBasis
  IPOINT3D *Index3DStart;   // start of each basis function in (i,j,k)
  IPOINT3D *Index3DEnd;     // end of each basis function in (i,j,k)
  IPOINT3D *iBasis1D;       // 1d basis function indices
  float   **BasisX;         // Basis functions in X [iBasis1D[iBasis.x]][iVoxel.x]
  float   **BasisY;         // Basis functions in Y [iBasis1D[iBasis.y]][iVoxel.y]
  float   **BasisZ;         // Basis functions in Z [iBasis1D[iBasis.z]][iVoxel.z]
  // vectors of length 2*NBasis for (x,y,z) fields
  float *value;             // scaling factor for each basis function
  float *increment;         // increments     for each basis function
  BOOL  *Adjust3D;          // 0/1 for each basis function triplet
  int   *AdjustOrder;       // order of basis functions in adjustment
  //
} AlignNL;

/////////////////////////////////////////////////////////////////////////////////
//////////////////// G L O B A L     V A R I A B L E S //////////////////////////
/////////////////////////////////////////////////////////////////////////////////
struct file_list template_files;   // template files
struct file_list individual_files; // individual files
struct file_list activation_files; // activation files; not used currently

// template/target space
struct data_stack template_unfiltered; // original template   data structure as read from file
struct data_stack template;    // filtered template data
struct data_stack individual;          // transformed data for individual (source)
struct data_stack NL_bases;    // image of x distortions (1 time point)
struct data_stack distort_x;   // image of x distortions (1 time point)
struct data_stack distort_y;   // image of y distortions (1 time point)
struct data_stack distort_z;   // image of z distortions (1 time point)

// source space
struct data_stack original_unfiltered; // original individual data structure as read from file
struct data_stack original;            // filtered original data for individual

// The "individual/original" stack uses graph.iTime as the time index.
// The current template time point uses a separate time index, iTime_Template.
int iTime_Template;
// global variables
float template_scale_min,   template_scale_max;
float individual_scale_min, individual_scale_max;
float distort_scale_min,    distort_scale_max;


#include "align-prototypes.h"
