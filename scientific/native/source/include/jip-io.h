// requires util.h

// Store data as real or complex
typedef union
{
  float *fData;
  FCOMPLEX *fcData;
} SpaceTime;

// Define a structure for a stack of images
struct data_stack
{
  IPOINT3D dim;          // dimensions in x, y, z
  POINT3D  resolution;   // mm per voxel in x, y, and z
  POINT3D  origin;       // origin for x, y, z
  POINT3D  direction;    // direction of increments in x, y, z
  POINT3D  fov;          // fov for x, y, z
  float    resolution_t; // seconds (usually)
  int nt;                // # of time points
  JIP_INDEX n2d;               // nx*ny
  JIP_INDEX n3d;               // nx*ny*nz
  JIP_INDEX n4d;              // nx*ny*nz*nt
  JIP_INDEX npts_volume;       // either n3d (magnitude only) or 2*n3d (complex)
  mat44 ijk_to_xyz;      // transformation from data indices (i,j,k) to space (x,y,z)
  mat44 xyz_to_ijk;      // transformation from data space (x,y,z) to indices (i,j,k)
  BOOL off_diagonal;     // if true, the transformation matrix does not have parallel voxels & coordinates
  int registration;      // paxinos-rat, etc.
  int status;            // bit-coded flag giving the volume status
  int data_type;         // 0 = magnitude, 1 = real-imaginary, 2 = magnitude-phase, 3 = RGB
  int file_storage_type; // same as XD_HEADER storage_type (bshort, bfloat, ...)
  int lDOF;              // degrees of freedom for statistics; default value 0
  SpaceTime SpaceTime;   // 4-dimensional data array
  char name[LEN_STR];    // name of stack
  struct data_stack *next; // pointer to the next stack (or NULL)
};

struct XD_HEADER
{
  char file_name[LEN_STR];
  IPOINT3D dim;          // # voxels in x, y, z
  POINT3D resolution;    // resolutions in x, y, z
  POINT3D origin;        // resolutions in x, y, z
  POINT3D direction;     // direction of increment in x, y, z
  POINT3D fov;           // field of view in x, y, z
  float   resolution_t;  // seconds (usually)
  int nt;                // # of time points
  JIP_INDEX n2d;               // voxels per slice
  JIP_INDEX n3d;               // voxels per volume
  JIP_INDEX n4d;              // voxels per data set
  int byte_order;        // byte order 0 = Mac, 1 = PC
  mat44 ijk_to_xyz;      // transformation from data indices (i,j,k) to space (x,y,z)
  mat44 xyz_to_ijk;      // inverse transformation
  BOOL off_diagonal;     // if true, the transformation matrix does not have parallel voxels & coordinates
  int file_type;         // e.g., bshort, bfloat, blong, nifti_nii, nifti_img
  int storage_type;      // storage type: bshort, bfloat, blong
  int data_type;         // 0 = float, 1 = real-imaginary, 2 = magnitude-phase, 3 = RGB
  int lDOF;              // degrees of freedom for statistics; default value 0
  double voxel_offset;    // offset to the first voxel data in the file
  float scale_slope;     // scaling factor for data (data --> slope*data + offset); ignore if 0; default 0
  float scale_offset;    // offset for data; default 0
  float qfac;            // quaternion factor; see nifti documentation
  POINT3D quaternion;    // quaternion values for b,c,d; see nifti documentation
  int registration;      // 0 = none, 1 = paxinos-rat
};

struct FS_HEADER
{
  float tlr, tla, tls;
  float trr, tra, trs;
  float brr, bra, brs;
  float normal_r, normal_a, normal_s;
  float te, tr, ti, flip_angle;
};

enum
  {
    JIP_COM    = 0,
    BSHORT     = 1,
    BFLOAT     = 2,
    BLONG      = 3,
    USHORT     = 4,
    UINT8      = 5,
    NIFTI_NII  = 6,
    NIFTI_IMG  = 7,
    FREESURFER = 8,
    OVL        = 9,
    RGB        = 10
  };

enum
  {
    REG_PW_RAT_OLD = -1,
    REG_PW_RAT     = 1
  };

// prototypes
int read_volume_header( BOOL Verbose, char *file_name, struct XD_HEADER *header );
int read_nifti_header( BOOL Verbose, char *file_name, struct XD_HEADER *header );
int read_xdisplay_header( char *file_name, struct XD_HEADER *header );
int read_freesurfer_header( char *file_name, struct XD_HEADER *header );
int decode_xdisplay_header_keyword( char *line, struct XD_HEADER *header);
int decode_freesurfer_header_keyword( char *line,struct XD_HEADER *xd_header,struct FS_HEADER *fs_header);
int get_extension( char *file_name, int *itype, char *extension );
int replace_extension( char *file_name, int *itype, char *extension, char *new_file_name);
int machine_byte_order();
void swap_16( void *ptr );
void swap_32( void *ptr );
int xdisplay_format( char *file_name );
