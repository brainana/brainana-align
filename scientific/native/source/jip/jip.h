//
// Author: Joe Mandeville
//         MGH NMR Center
//         jbm@nmr.mgh.harvard.edu
//
// NOT FOR DISTRIBUTION WITHOUT THE AUTHOR'S EXPLICIT PERMISSION.
//

#define GAMMA_PRIME 4.27    // gamma/2/PI in kHz/Gauss
#define MAX_NSET   500000   // max # of volumes in the volume list
#define TINY       1.e-20

//
// Define a structure for an volume list, which is used differently than
// the volume stack.  The linked list of stacks (above) will define all
// the stacks in memory, whereas the linked list of volumes will define
// only the "current" list of volumes to process, which may contain multiple
// stacks or subsets of stacks.  The current volume list will be re-initialized
// prior to every user-entered command.
//
struct volume_list
{
  struct data_stack *stack;   // The volume stack in which the volume resides.
  IPOINT3D dim;        // dimensions in x, y, z
  JIP_INDEX n2d;             // nx*ny
  JIP_INDEX n3d;             // nx*ny*nz
  SpaceTime SpaceTime; // pointer to the start of data for a 3d time point
  struct volume_list  *next;    // pointer to the next list structure
};
struct data_stack registration_distortions;

typedef struct
{
  JIP_INDEX nvoxels;
  int io_logic;
  IPOINT3D *voxel;
  float *weight;
} OVERLAY;

//
// Define bits for the program flags integer.
//
#define VERBOSE_BIT            1
#define MUTE_BIT               2
#define MODE_BIT1              4
#define MODE_BIT2              8
#define PLOT_BIT              16
#define SCAN_BIT              32
#define COMPLEX_INPUT_BIT     64
#define MAG_PHASE_INPUT_BIT  128
#define PART_K_INPUT_BIT     256
#define COMPLEX_OUTPUT_BIT   512
#define PHASE_OUTPUT_BIT    1024
#define REFERENCE_LINES_BIT 2048
#define EPI_INPUT_BIT       4096
#define GRASE_INPUT_BIT     8192
#define INPUT_SWAPPING_BIT 16384
//
// Define bits for the image status integer.
//
#define MAG_PHASE_BIT 1
#define MAP_BIT       2
#define IGNORE_BIT    4
#define EPI_BIT       8
#define GRASE_BIT    16
#define SPIRAL_BIT   32
#define PART_K_BIT   64
#define ROI_BIT     128
#define REG_PAX_RAT 256
#define REG_MONKEY  512
#define REG_MOUSE  1024

#define INTERP_GAUSSIAN 0
#define INTERP_LANCZOS  1

#include "jip-prototypes.h"
