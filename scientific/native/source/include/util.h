#include "jip-memory.h"
#include <stdio.h> // need for FILE structure

/*
** Define commonly used constants.
*/
#define PI      3.141592654    /* This is clear                */
#define PI_180 (PI/180.)
#define LEN_STR 1024           /* Length for character strings */
#define TRUE  1
#define FALSE 0

/*
** Create some new data types.
*/
#define LOGICAL int         
#define BOOL int         
typedef struct {long real,imag;}   LCOMPLEX;
typedef struct {short real,imag;}  SCOMPLEX;
typedef struct {float real,imag;}  FCOMPLEX;
typedef struct {double real,imag;} DCOMPLEX;
typedef struct {unsigned char extra,red,green,blue;} RGB4;

#ifndef _NIFTI_IO_HEADER_
typedef struct {float m[4][4];}    mat44;
#endif

typedef struct {float x,y;}        POINT2D;
typedef struct {int x, y;}          IPOINT2D;
typedef struct {float x,y,z;}      POINT3D;
typedef struct {int x, y, z;}        IPOINT3D;
typedef struct {JIP_INDEX x,y,z;} JIP_INDEX3D;
typedef struct {int x, y, z, t;}      IPOINT4D;

#define SQR(a) ((a) == 0.0 ? 0.0 : (a)*(a))
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define SIGN(a,b) ( (b)>=0.0 ? fabs((double)a) : -fabs((double)a) )

#include "jip-io.h"



char *char_vector(JIP_INDEX length);
void free_char_vector(char *vec);

unsigned char *unsigned_char_vector(JIP_INDEX length);
void free_unsigned_char_vector(unsigned char *vec);

float *float_vector(JIP_INDEX length);
float *float_vector_long(JIP_INDEX length);
void free_float_vector(float *vec);

double *double_vector(JIP_INDEX length);
void free_double_vector(double *vec);

int *int_vector(JIP_INDEX length);
void free_int_vector(int *vec);

short *short_vector(JIP_INDEX length);
void free_short_vector(short *vec);

unsigned short *unsigned_short_vector(JIP_INDEX length);
void free_unsigned_short_vector(unsigned short *vec);

long *long_vector(JIP_INDEX length);
void free_long_vector(long *vec);

FCOMPLEX *fcomplex_vector(JIP_INDEX length);
void free_fcomplex_vector(FCOMPLEX *vec);

LCOMPLEX *lcomplex_vector(JIP_INDEX length);
void free_lcomplex_vector(LCOMPLEX *vec);

SCOMPLEX *scomplex_vector(JIP_INDEX length);
void free_scomplex_vector(SCOMPLEX *vec);

POINT2D *point2d_vector(JIP_INDEX length);
void free_point2d_vector(POINT2D *vec);

IPOINT2D *ipoint2d_vector(JIP_INDEX length);
void free_ipoint2d_vector(IPOINT2D *vec);

POINT3D *point3d_vector(JIP_INDEX length);
void free_point3d_vector(POINT3D *vec);

IPOINT3D *ipoint3d_vector(JIP_INDEX length);
void free_ipoint3d_vector(IPOINT3D *vec);

float **float_matrix(JIP_INDEX length1, JIP_INDEX length2);
void free_float_matrix(float **mat, JIP_INDEX length1, JIP_INDEX length2);

double **double_matrix(JIP_INDEX length1, JIP_INDEX length2);
void free_double_matrix(double **mat, JIP_INDEX length1, JIP_INDEX length2);

int **int_matrix(JIP_INDEX length1, JIP_INDEX length2);
void free_int_matrix(int **mat, JIP_INDEX length1, JIP_INDEX length2);

short int **short_matrix(JIP_INDEX length1, JIP_INDEX length2);
void free_short_matrix(short **mat, JIP_INDEX length1, JIP_INDEX length2);

int my_getopt( int argc, char *arg_list[], const char *optstring, int *iarg );
char *get_line( FILE *stream, char *string, int length );
char *get_line_with_comment( FILE *stream, char *string, int length );

mat44 invert_mat44( mat44 Input44 );
mat44 scale_and_shift_to_mat44(POINT3D resolution, POINT3D origin, POINT3D direction);
mat44 general_affine_to_mat44( float *srow_x, float *srow_y, float *srow_z );
int invert_3x3_matrix( float **matrix );
int dInvertSquareMatrix( int nMat, double **dSquareMatrix, int *iRowFail );
int rInvertSquareMatrix( int nMat, float  **fSquareMatrix, int *iRowFail );

void alloc_error(char *text);
char *next_non_white( char *cptr );
void convert_to_lower( char *string_pointer );
int is_upper_case(char ch);
char to_lower_case(char ch);

// Prototypes
float Kaiser_Bessel( double x, double W, double beta );
float Gauss(float x, float y, float sigma);
float inverse_Gauss(float x, float y, float sigma);
FCOMPLEX inverse_Kaiser_Bessel( double x, double W, double beta );
double bessi0( double x );
void swap_x( FCOMPLEX *FCData, int lDim, int lAdd );
float compute_phase    (FCOMPLEX FC);
float compute_amplitude(FCOMPLEX FC);
float ham_sinc( float x, int iX );
float Lanczos_filter( float x, int iX );
BOOL is_power_of_2( int lInput, int *lLastPower, int *lNextPower );
FCOMPLEX FCMult(FCOMPLEX FC1, FCOMPLEX FC2);
FCOMPLEX FCMultStar(FCOMPLEX FC1, FCOMPLEX FC2);
FCOMPLEX FCAdd(FCOMPLEX FC1, FCOMPLEX FC2);
float Trace_of_Matrix( int NSquare, float **RealMatrix );
void Matrix_Product( float **X1X2, int Length1, int Length2,
		     float **X1, int iTranspose1, int Length12, float **X2, int iTranspose2);
void Matrix_Vector_Product( float *X1X2, int Length, float **X1, int iTranspose,
			    int Length12, float *X2);
void Matrix_PseudoInverse( float **X_12, int Length1, int Length2, float **piX_21 );


// Fit prototypes
int PolynomialDefineGLM( int ncoeff, int ntime, int *include );
int PolynomialFitGLM( int ntime, float *data, float *fit );
int PolynomialRemoveGLM( int ibasis, int ntime, float *data, float *fit );
int GeneralDefineGLM( int ncoeff, int ntime, int *include, float **X_tc );
int GeneralFitGLM( int ntime, float *data, float *fit );
int GeneralRemoveGLM( int ibasis, int ntime, float *data, float *fit );
int CubicBSplineGLM_DefineGLM( int ntime );
int CubicBSplineGLM_FitGLM( int ntime, float *data, float *fit );
int fit_gaussian(int npoints, double *xarray, double *yarray, float *xcenter,
		 float *amplitude, float *sigma, float *yoffset);
int fit_1_over_r2(int npoints, double *xarray, double *yarray, float *xcenter,
		  float *amplitude, float *FWHM, float *yoffset);
int fit_r1_from_flip_angles(int npoints, double *xarray, double *yarray, float tr,
			    float *s0, float *r1, float *rel_angle);
int fit_r1_from_multiple_tr(int npoints, double *xarray, double *yarray, float flip,
			    float *s0, float *r1, float *rel_angle);
double func_gaussian(double x, double *p);
double func_1_over_r2(double x, double *p);
double func_r1_from_flip_angles(double x, double *p);
double func_r1_from_multiple_tr(double x, double *p);
void lm_print_maps(int n_par, double *par, int m_dat, double *fvec,
		   void *data, int iflag, int iter, int nfev);

POINT3D Convert_Voxel_to_Space(IPOINT3D DataIndex, struct data_stack *stack);
POINT3D Convert_rVoxel_to_Space(POINT3D DataIndex, struct data_stack *stack);
POINT3D Convert_Space_to_rVoxel(POINT3D Space, struct data_stack *stack);
mat44 centered_ijk_to_xyz( struct data_stack target );
mat44 centered_xyz_to_ijk( struct data_stack source );
void downsample_stack( struct data_stack *target, IPOINT3D DimensionLow );
int help_menu(int iLevel, char *file_name);

JIP_INDEX *index_vector(JIP_INDEX length);

JIP_INDEX3D *index3d_vector(JIP_INDEX length);

DCOMPLEX *dcomplex_vector(JIP_INDEX length);
void free_dcomplex_vector(DCOMPLEX *vec);
long **long_matrix(JIP_INDEX length1,JIP_INDEX length2);
void free_long_matrix(long **mat,JIP_INDEX length1,JIP_INDEX length2);
