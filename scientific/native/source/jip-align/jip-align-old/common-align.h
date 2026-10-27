#define NAFFINE 12

struct
{
  IPOINT3D dimension;      // dimensions of alignment target
  POINT3D  resolution;     // resolution of alignment target
  POINT3D  origin;         // origin     of alignment target
  POINT3D  direction;      // direction  of alignment target
  BOOL     RigidBody;      // generally true; refers to affine part
  IPOINT3D OrientRotation; // rotations in x, y, and z for initial orientation
  IPOINT3D WrapAround;     // sets wrapping on/off in each dimension during alignment
  int      OrientSwapZ;    // swap Z; changes parity of coordinate system (RHC<-->LHC)
  float value[NAFFINE];    // parameter values (if >= NAFFINE, 0/1 indicate adjusted or not)
  float Orient[4][4];      // rotation matrix for initial orientation
  float Affine[4][4];      // Affine matrix
} Alignment;

// Prototypes
void calculate_orientation_rotation();
void define_cosine_sine( int iAngle, float *cs, float *sn);
void define_affine_matrix();
void distort_non_linear( POINT3D *Space, float x, float y, float z,
			 IPOINT3D AdjustNL, POINT3D fov );
