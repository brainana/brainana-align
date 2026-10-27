#ifndef JIP_LEGACY_STORAGE
#define JIP_LEGACY_STORAGE
#endif
#define NAFFINE  12
#define NAFFINEH  9 // number of parameter used for histological templates
#define NSPAR     4 // number slice  parameters used in histology alignment

JIP_LEGACY_STORAGE struct jip_alignment
{
  /* Layout changed: clean-rebuild all consumers (legacy makefile deps are incomplete). */
  mat44 voxel_affineT, voxel_affineS; /* native geometry; registration stays centered */
  BOOL has_voxel_affineT, has_voxel_affineS;
  IPOINT3D dimensionT;     // dimensions of alignment target
  POINT3D  resolutionT;    // resolution of alignment target
  POINT3D  originT;        // origin     of alignment target
  POINT3D  directionT;     // direction  of alignment target
  IPOINT3D dimensionS;     // dimensions of alignment source
  POINT3D  resolutionS;    // resolution of alignment source
  POINT3D  originS;        // origin     of alignment source
  POINT3D  directionS;     // direction  of alignment source
  BOOL     RigidBody;      // generally true; refers to affine part
  IPOINT3D OrientRotation; // rotations in x, y, and z for initial orientation
  IPOINT3D WrapAround;     // sets wrapping on/off in each dimension during alignment
  int OrientSwapZ;    // swap Z; changes parity of coordinate system (RHC<-->LHC)
  float value[NAFFINE];    // parameter values (if >= NAFFINE, 0/1 indicate adjusted or not)
  float **slice_value;     // x,y,angle for each slice; dim = [nslices][3]
  float Orient[4][4];      // rotation matrix for initial orientation
  float Affine[4][4];      // Forward affine transformation matrix
  float AffineI[4][4];     // Affine matrix inverse for reverse transformation
  float AffineH[4][4];     // Affine matrix for slice-specific transformations for histology
  int registration;      // Define species type (unused)
  int interpolation;     // interpolation kernel
} Alignment;

// Prototypes: MRI
void define_affine_matrix();
void calculate_orientation_rotation();
void define_cosine_sine( int iAngle, float *cs, float *sn);
// Prototypes: histology
void define_slice_matrix( int iZ );
void calculate_orientation_rotation_hist();
void define_affine_matrix_hist();
void define_cosine_sine_hist( int iAngle, float *cs, float *sn);
void modify_alignment_parameter_names();
