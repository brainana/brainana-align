#include "common-display.h"

// Image selectable text fields (continue number from common fields)
enum SpecificSelectAbleTextFields_Image
  {
    TEXTFIELD_LABEL_PLANES   = 27, // label planes with positions and/or grids
    TEXTFIELD_COLORBAR_RANGE = 28, // range for color or grayscale
    TEXTFIELD_COLOR_OR_GRAY  = 29, // range for color or grayscale
  };
// Graph selectable text fields (continue number from common fields)
enum SpecificSelectAbleTextFields_Graph
  {
    TEXTFIELD_OUTPUT_ROI      = 10,  // write ROI
    TEXTFIELD_LAST_TIME_POINT = 11,  // last image in series
    TEXTFIELD_GLM_OPTIONS     = 12,  // GLM options field
    TEXTFIELD_GLM_SETTINGS    = 13,  // GLM settings field (for each option)
    TEXTFIELD_GLM_BASELINE    = 14,  // GLM settings field (for each option)
    //
    TEXTFIELD_SRTM_OPTIONS     = 15,  // SRTM options field
    TEXTFIELD_SRTM_SETTINGS    = 16,  // SRTM settings field
    TEXTFIELD_SRTM_PARVALUES   = 17,  // SRTM values field
    //
    TEXTFIELD_CHI2            = 18
  };

/////////////////////////////////////////////////////////////////////////////////
//////////////////// G L O B A L     V A R I A B L E S //////////////////////////
/////////////////////////////////////////////////////////////////////////////////
struct file_list image_files;      // files of images included in GLM
struct file_list anatomy_files;    // files of images NOT included in GLM
struct file_list activation_files; // files of activation maps

struct data_stack anatomy_stack;     // anatomical images excluded from the time series

struct roi_file_list roi_file_root;

struct GLM_MODEL
{
  int options;             // keeps track of options below: 0=event,1=shape,2=exclude,3=include,4=baseline
  int event_number_active; // active GLM event number
  int shape;               // GLM: -1 = none, 0 = square, 1 = gamma, 2 = table; SRTM: 0=constant, 1=gamma, 2=table
  int show_glm_windows;    // 2=show shaded GLM regions & ignored regions, 1=show GLM, 0=show none
  char residue_string[LEN_STR]; // ""= none, "1" = event 1, "12" = events 1&2, ...
  // fMRI GLM only:
  int baseline_terms;      // 0, 1, 2, 3
  int baseline;            // 1 = show baseline, 0 = remove baseline
  // SRTM GLM only:
  int view_type;           // SRTM: -1=TAC, 0=R1, 1=tau2', 2=BP
  int iParameter;          // parameter value at current time point: 0=R1, 1=k2, 2=k2a
  int iCondition;          // condition index
  float effect_size;       // effect size from GLM
  float effect_sdev;       // stdev from GLM
  float chi2_dof;          // chi2 per degree of freedom
} glm_model;

struct SRTM_MODEL
{
  int options;             // keeps track of options below: 0=exclude,1=include
  char residue_string[LEN_STR]; // ""= none, "1" = event 1, "12" = events 1&2, ...
} srtm_model;

enum glm_event_shapes
  {
    SHAPE_SQUARE   = 0,
    SHAPE_GAMMA    = 1,
    SHAPE_TABLE    = 2,
    SHAPE_DRUG_IRF = 3
  };

enum SRTM_types
  {
    TYPE_R1  = 0, // R1   : static  basis function dependent upon ref region
    TYPE_k2  = 1, // k2   : dynamic basis function dependent upon integral of ref region
    TYPE_k2a = 2  // k2a  : static  basis function dependent upon integral of target region
  };

// miscellaneous global variables
char ROI_file_name[LEN_STR];


#include "display-prototypes.h"
