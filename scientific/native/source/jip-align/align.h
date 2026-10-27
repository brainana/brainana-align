#ifndef JIP_LEGACY_STORAGE
#define JIP_LEGACY_STORAGE
#endif
#include "common-display.h"
#include "common-align.h"

// Image selectable text fields (continue number from common fields)
enum SpecificSelectAbleTextFields_Image
  {
    TEXTFIELD_MIMIC_TARGET      = 27, // individual/template toggle text
    TEXTFIELD_SELECT_PARAMETERS = 28, // a text field to simplify parameter selection
    TEXTFIELD_COST_FUNCTION     = 29, // a text field to show cost functions
    TEXTFIELD_RESET_RECALL      = 30, // a text field to show cost functions
    TEXTFIELD_AUTO_ALIGN        = 31, // a text field to show cost functions
    TEXTFIELD_DO_EVERYTHING     = 32  // a text field to show cost functions
  };

JIP_LEGACY_STORAGE struct jip_align_aid
{ // NAFFINE from align.h
  int DisplayType;            // 0 = mimic, 1 = template, 2 = NL-x, 3=NL-y, 4=NL-z
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
  int matching_volume;        // 0 = template, 1 = everything
  float saved_value[NAFFINE]; // saved parameter values for reset/recall (0,1 for non-linear params)
  float CF_Affine;            // Cost function without non-linear penalties
  float CF_NonLinear;         // Cost function with    non-linear penalties
  mat44 ijk_to_xyz;           // data indices (i,j,k) to space (x,y,z) in aligned,     centered coordinates
  mat44 xyz_to_ijk;           // space (x,y,z) to data indices (i,j,k) in non-aligned, centered coordinates
  JIP_INDEX nmask;                  // # of points in current alignment mask (overlay)
  float *mask;                // mask; dimensions match alignment.matrix
} AlignAid;

JIP_LEGACY_STORAGE struct jip_align_nl
{
  union
  {
    IPOINT3D axis;
    int direction[3];
  } Adjust;                 // 1=adjust   during iterations, 0 = ignore
  IPOINT3D Adjusted;        // 1=adjusted during iterations, 0 = ignore
  IPOINT3D GridDown;        // grid downsampling for basis functions in x, y, z (based upon iGridDown)
  IPOINT3D NBasis1D;        // number of basis functions in (x,y,z)
  JIP_INDEX NBasis;          // total number of basis functions (product of x,y,z)
  int iGridDown;       // basis function down-sampling factor
  JIP_INDEX iBasis;          // current basis function (0 to NBasis)
  JIP_INDEX3D *NeighborLow;    // index of basis functions that are low  neighbors in x,y,z
  JIP_INDEX3D *NeighborHigh;   // index of basis functions that are high neighbors in x,y,z
  int Lambda_reg;          // regularization constant
  float Jacob_limits;       // Jacobian limits (n and 1/n)
  float stiffness;          // sets sliding scale between 1st and 2nd derivatives in cost function
  int ViewBasis;            // 0/1
  // vectors of length NBasis
  IPOINT3D *Index3DStart;   // start of each basis function in (i,j,k)
  IPOINT3D *Index3DEnd;     // end of each basis function in (i,j,k)
  IPOINT3D *iBasis1D;       // 1d basis function indices
  float   **BasisX;         // Basis functions in X [NBasis1D.x][target.dim.x]
  float   **BasisY;         // Basis functions in Y [NBasis1D.y][target.dim.y]
  float   **BasisZ;         // Basis functions in Z [NBasis1D.z][target.dim.z]
  float    *Scale;          // Scaling of basis functions (generally 1 except at edges)
  // more vectors of length NBasis: adjustments in x,y,z
  POINT3D *value;           // scaling factor for each basis function
  int *Adjust3D;            // 0/1 for each basis function triplet
  JIP_INDEX *AdjustOrder;         // order of basis functions in adjustment
  //
} AlignNL;

/////////////////////////////////////////////////////////////////////////////////
//////////////////// G L O B A L     V A R I A B L E S //////////////////////////
/////////////////////////////////////////////////////////////////////////////////
JIP_LEGACY_STORAGE struct file_list template_files;   // template files
JIP_LEGACY_STORAGE struct file_list source_files;     // mimic files
JIP_LEGACY_STORAGE struct file_list activation_files; // activation files; not used currently

// source space with potentially multiple time points
JIP_LEGACY_STORAGE struct data_stack source_unfiltered; // original source data structure as read from file
JIP_LEGACY_STORAGE struct data_stack source;            // potentially modified by rolloff filter

// template/target space at full resolution, with potentially multiple time points
JIP_LEGACY_STORAGE struct data_stack template;            // potentially modified by rolloff & overlay filters

// template/target space with 1 time point; potentially down-sampled
JIP_LEGACY_STORAGE struct data_stack target;        // down-sampled template
JIP_LEGACY_STORAGE struct data_stack mimic;         // transformed source
JIP_LEGACY_STORAGE struct data_stack NL_bases;      // image of basis functions
// template/target space with 2 time points (2nd one is for storage)
JIP_LEGACY_STORAGE struct data_stack distort_x;     // image of x distortions (1 time point)
JIP_LEGACY_STORAGE struct data_stack distort_y;     // image of y distortions (1 time point)
JIP_LEGACY_STORAGE struct data_stack distort_z;     // image of z distortions (1 time point)
JIP_LEGACY_STORAGE struct data_stack Jac_deter;     // image of Jacobian determinant

// The "individual/original" stack uses graph.iTime as the time index.
// The current template time point uses a separate time index, iTime_Template.
JIP_LEGACY_STORAGE BOOL Verbose;
JIP_LEGACY_STORAGE int iTime_Template, iTime_Source;
// global variables
JIP_LEGACY_STORAGE float target_scale_min,  target_scale_max;
JIP_LEGACY_STORAGE float mimic_scale_min,   mimic_scale_max;
JIP_LEGACY_STORAGE float distort_scale_min, distort_scale_max;


#include "align-prototypes.h"
