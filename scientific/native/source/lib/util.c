// Copyright (c) The General Hospital Corporation
// All rights reserved.
// 
// This agreement ("Software License") covers downloads from the JIP fMRI Analysis Toolkit
// maintained by The General Hospital Corporation. Your downloading, copying, modifying,
// distributing, displaying, or use of any software and/or data from JIP fMRI Analysis Toolkit
// constitutes acceptance of all of the terms and conditions of this Agreement. If you do not
// agree to such terms and conditions, you have no right to download, copy, modify, distribute,
// display, or use the Software.
// 
// 1. As used in this Software License,
// "you" means the individual downloading and/or using, reproducing, modifying, and/or displaying
// the Software and the institution or entity which employs or is otherwise affiliated with such
// individual in connection therewith; The General Hospital Corporation d/b/a Massachusetts
// General Hospital ("Hospital") hereby grants you with respect to Hospital's rights in the
// software, and data, if any, which is the subject of this software license (collectively,
// the "Software"), a royalty-free, non-exclusive license under Hospital's copyright rights,
// to redistribute, reproduce, modify, and display the Software (in source or binary forms),
// provided that: 
// 
// (a) you accept and adhere to all of the terms and conditions of this Software License; 
// 
// (b) redistributions of source code must retain the above copyright notice, this list of conditions
// and the following disclaimer;
// 
// (c) redistributions in binary form must reproduce the above copyright notice, this list of conditions
// and the following disclaimer in the documentation and/or other materials provided with distribution;
// 
// 2. The Software is provided "AS IS" and neither Hospital nor any contributor to the software
// (each a "Contributor") shall have any obligation to provide maintenance, support, updates,
// enhancements or modifications thereto. HOSPITAL AND ALL CONTRIBUTORS SPECIFICALLY DISCLAIM
// ALL EXPRESS AND IMPLIED WARRANTIES OF ANY KIND INCLUDING, BUT NOT LIMITED TO, ANY WARRANTIES
// OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
// HOSPITAL OR ANY CONTRIBUTOR BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL,
// EXEMPLARY OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
// GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY ARISING IN ANY WAY RELATED TO THE SOFTWARE, EVEN IF HOSPITAL OR ANY
// CONTRIBUTOR HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES. TO THE MAXIMUM EXTENT NOT
// PROHIBITED BY LAW OR REGULATION, YOU FURTHER ASSUME ALL LIABILITY FOR YOUR USE, REPRODUCTION,
// DISPLAY, LICENSE OR DISTRIBUTION OF THE SOFTWARE AND AGREE TO INDEMNIFY AND HOLD HARMLESS HOSPITAL
// AND ALL CONTRIBUTORS FROM AND AGAINST ANY AND ALL CLAIMS, SUITS, ACTIONS, DEMANDS AND JUDGMENTS
// ARISING THEREFROM. 
// 
// 3. None of the names, logos or trademarks of Hospital or any of Hospital's affiliates or any
// of the Contributors, or any funding agency, may be used to endorse or promote products produced
// in whole or in part by operation of the Software or derived from or based on the Software without
// specific prior written permission from the applicable party.  In the case of Hospital, permission
// needs to be obtained from its Public Affairs Office.
// 
// 4. You agree to provide acknowledgement to Hospital when you publish and present results of your
// use of the Software in peer-reviewed journals in accordance with standard academic practice. 
// 
// 5. This Software License does not grant any rights with respect to third party software,
// except those rights that Hospital has been authorized by a third party to grant to you,
// and accordingly you are solely responsible for obtaining any permissions from third parties
// that you need to use, reproduce, distribute, modify, and display the Software. 
// 
// 6. The Software has been designed for research purposes only and has not been reviewed or approved
// by the Food and Drug Administration or by any other agency. YOU ACKNOWLEDGE AND AGREE THAT
// CLINICAL APPLICATIONS ARE NEITHER RECOMMENDED NOR ADVISED. You further agree to use, reproduce,
// and display the Software in compliance with all applicable governmental laws, regulations and orders,
// including without limitation those relating to export and import control.



//
// Author: Joe Mandeville
//         Athinoula A. Martinos Center for Biomedical Imaging
//         Massachusetts General Hospital

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "nifti1_io.h"
#include "util.h"
static char CNULL = '\0';

/*
** Allocation error routine
*/
void alloc_error(char *text)
{
  fprintf(stderr,"Fatal Error:\n");
  fprintf(stderr,"%s\n",text);
  fprintf(stderr,"Program aborted.\n");
  exit(1);
}

/*
** char vectors
*/
char *char_vector(JIP_INDEX length)
{
  char *vec;

  vec = (char *)jip_allocate(length,sizeof(*vec));
  if (!vec) alloc_error("Allocation error in char_vector().");
  return(vec);
}
void free_char_vector(char *vec)
{
  free(vec);
}
/*
** char vectors
*/
unsigned char *unsigned_char_vector(JIP_INDEX length)
{
  unsigned char *vec;

  vec = (unsigned char *)jip_allocate(length,sizeof(*vec));
  if (!vec) alloc_error("Allocation error in char_vector().");
  return(vec);
}
void free_unsigned_char_vector(unsigned char *vec)
{
  free(vec);
}

/*
** Floating point vectors
*/
float *float_vector(JIP_INDEX length)
{
  float *vec;

  vec = (float *)jip_allocate(length,sizeof(*vec));
  if (!vec) alloc_error("Allocation error in float_vector().");
  return(vec);
}
void free_float_vector(float *vec)
{
  free((char *)vec);
}

/*
** Floating point vectors
*/
float *float_vector_long(JIP_INDEX length)
{
  float *vec;

  vec = (float *)jip_allocate(length,sizeof(*vec));
  if (!vec) alloc_error("Allocation error in float_vector().");
  return(vec);
}

// Floating point 2-vector
POINT2D *point2d_vector(JIP_INDEX length)
{
  POINT2D *vec;

  vec = (POINT2D *)jip_allocate(length,sizeof(*vec));
  if (!vec) alloc_error("Allocation error in point2d_vector().");
  return(vec);
}
void free_point2d_vector(POINT2D *vec)
{
  free((char *)vec);
}
// Floating point 2-vector
IPOINT2D *ipoint2d_vector(JIP_INDEX length)
{
  IPOINT2D *vec;

  vec = (IPOINT2D *)jip_allocate(length,sizeof(*vec));
  if (!vec) alloc_error("Allocation error in ipoint2d_vector().");
  return(vec);
}
void free_ipoint2d_vector(IPOINT2D *vec)
{
  free((char *)vec);
}

// Floating point 3-vector
POINT3D *point3d_vector(JIP_INDEX length)
{
  POINT3D *vec;

  vec = (POINT3D *)jip_allocate(length,sizeof(*vec));
  if (!vec) alloc_error("Allocation error in point3d_vector().");
  return(vec);
}
void free_point3d_vector(POINT3D *vec)
{
  free((char *)vec);
}
// Floating point 3-vector
IPOINT3D *ipoint3d_vector(JIP_INDEX length)
{
  IPOINT3D *vec;

  vec = (IPOINT3D *)jip_allocate(length,sizeof(*vec));
  if (!vec) alloc_error("Allocation error in ipoint3d_vector().");
  return(vec);
}
void free_ipoint3d_vector(IPOINT3D *vec)
{
  free((char *)vec);
}

/*
** Double precision vectors
*/
double *double_vector(JIP_INDEX length)
{
  double *vec;

  vec = (double *)jip_allocate(length,sizeof(*vec));
  if (!vec) alloc_error("Allocation error in double_vector().");
  return(vec);
}
void free_double_vector(double *vec)
{
  free((char *)vec);
}

/*
** Integer vectors
*/
int *int_vector(JIP_INDEX length)
{
  int *vec;

  vec = (int *)jip_allocate(length,sizeof(*vec));
  if (!vec) alloc_error("Allocation error in int_vector().");
  return(vec);
}
void free_int_vector(int *vec)
{
  free((char *)vec);
}

/*
** Short vectors
*/
short *short_vector(JIP_INDEX length)
{
  short *vec;

  vec = (short *)jip_allocate(length,sizeof(*vec));
  if (!vec) alloc_error("Allocation error in short_vector().");
  return(vec);
}
void free_short_vector(short *vec)
{
  free((char *)vec);
}

/*
** Unsigned short vectors
*/
unsigned short *unsigned_short_vector(JIP_INDEX length)
{
  unsigned short *vec;

  vec = (unsigned short *)jip_allocate(length,sizeof(*vec));
  if (!vec) alloc_error("Allocation error in short_vector().");
  return(vec);
}
void free_unsigned_short_vector(unsigned short *vec)
{
  free((char *)vec);
}

/*
** Long vectors
*/
long *long_vector(JIP_INDEX length)
{
  long *vec;

  vec = (long *)jip_allocate(length,sizeof(*vec));
  if (!vec) alloc_error("Allocation error in long_vector().");
  return(vec);
}
void free_long_vector(long *vec)
{
  free((char *)vec);
}

/*
** Complex floats
*/
FCOMPLEX *fcomplex_vector(JIP_INDEX length)
{
  FCOMPLEX *vec;

  vec = (FCOMPLEX *)jip_allocate(length,sizeof(*vec));
  if (!vec) alloc_error("Allocation error in fcomplex_vector().");
  return(vec);
}
void free_fcomplex_vector(FCOMPLEX *vec)
{
  free((char *)vec);
}

/*
** Complex doubles
*/
DCOMPLEX *dcomplex_vector(JIP_INDEX length)
{
  DCOMPLEX *vec;

  vec = (DCOMPLEX *)jip_allocate(length,sizeof(*vec));
  if (!vec) alloc_error("Allocation error in fcomplex_vector().");
  return(vec);
}
void free_dcomplex_vector(DCOMPLEX *vec)
{
  free((char *)vec);
}

/*
** Complex longs
*/
LCOMPLEX *lcomplex_vector(JIP_INDEX length)
{
  LCOMPLEX *vec;

  vec = (LCOMPLEX *)jip_allocate(length,sizeof(*vec));
  if (!vec) alloc_error("Allocation error in lcomplex_vector().");
  return(vec);
}
void free_lcomplex_vector(LCOMPLEX *vec)
{
  free((char *)vec);
}

/*
** Complex shorts
*/
SCOMPLEX *scomplex_vector(JIP_INDEX length)
{
  SCOMPLEX *vec;

  vec = (SCOMPLEX *)jip_allocate(length,sizeof(*vec));
  if (!vec) alloc_error("Allocation error in scomplex_vector().");
  return(vec);
}
void free_scomplex_vector(SCOMPLEX *vec)
{
  free((char *)vec);
}

/*
** Floating point matrix.
*/
float **float_matrix(JIP_INDEX length1, JIP_INDEX length2)
{
  JIP_INDEX j;
  float **mat;

  mat = (float **)jip_allocate(length1,sizeof(*mat));
  if (!mat) alloc_error("Allocation failure 1 in float_matrix().");
  
  for (j=0; j<length1; j++)
    {
      mat[j] = (float *)jip_allocate(length2,sizeof(**mat));
      if (!mat[j]) alloc_error("Allocation failure 2 in float_matrix().");
    }
  return(mat);
}
void free_float_matrix(float **mat, JIP_INDEX length1, JIP_INDEX length2)
{
  JIP_INDEX j;
  
  for (j=length1-1; j>=0; j--) free((char*)(mat[j]));
  free((char*)(mat));
}

/*
** Double matrix
*/
double **double_matrix(JIP_INDEX length1, JIP_INDEX length2)
{
  JIP_INDEX j;
  double **mat;
  
  mat = (double **)jip_allocate(length1,sizeof(*mat));
  if (!mat) alloc_error("Allocation failure 1 in double_matrix().");
  
  for (j=0; j<length1; j++)
    {
      mat[j] = (double *) jip_allocate(length2,sizeof(**mat));
      if (!mat[j]) alloc_error("Allocation failure 2 in double_matrix().");
    }
  return(mat);
}
void free_double_matrix(double **mat, JIP_INDEX length1, JIP_INDEX length2)
{
  JIP_INDEX j;
  
  for (j=length1-1; j>=0; j--) free((char*)(mat[j]));
  free((char*)(mat));
}

/*
** Integer matrix
*/
int **int_matrix(JIP_INDEX length1, JIP_INDEX length2)
{
  int **mat;JIP_INDEX j;
  
  mat = (int **)jip_allocate(length1,sizeof(*mat));
  if (!mat) alloc_error("Allocation failure 1 in integer_matrix().");
  
  for (j=0; j<length1; j++)
    {
      mat[j]=(int *)jip_allocate(length2,sizeof(**mat));
      if (!mat[j]) alloc_error("Allocation failure 2 in integer_matrix().");
    }
  return(mat);
}
void free_int_matrix(int **mat, JIP_INDEX length1, JIP_INDEX length2)
{
  JIP_INDEX j;
  
  for (j=length1-1; j>=0; j--) free((char*)(mat[j]));
  free((char*)(mat));
}

/*
** Short integer matrix
*/
short int **short_matrix(JIP_INDEX length1, JIP_INDEX length2)
{
  JIP_INDEX j;
  short **mat;
  
  mat = (short **)jip_allocate(length1,sizeof(*mat));
  if (!mat) alloc_error("Allocation failure 1 in short_matrix().");
  
  for (j=0; j<length1; j++)
    {
      mat[j]=(short *)jip_allocate(length2,sizeof(**mat));
      if (!mat[j]) alloc_error("Allocation failure 2 in short_matrix().");
    }
  return(mat);
}
void free_short_matrix(short **mat, JIP_INDEX length1, JIP_INDEX length2)
{
  JIP_INDEX j;
  
  for (j=length1-1; j>=0; j--) free((char*)(mat[j]));
  free((char*)(mat));
}

/*
** Long integer matrix
*/
long int **long_matrix(JIP_INDEX length1, JIP_INDEX length2)
{
  JIP_INDEX j;
  long **mat;
  
  mat = (long **)jip_allocate(length1,sizeof(*mat));
  if (!mat) alloc_error("Allocation failure 1 in long_matrix().");
  
  for (j=0; j<length1; j++)
    {
      mat[j]=(long *)jip_allocate(length2,sizeof(**mat));
      if (!mat[j]) alloc_error("Allocation failure 2 in long_matrix().");
    }
  return(mat);
}
void free_long_matrix(long **mat, JIP_INDEX length1, JIP_INDEX length2)
{
  JIP_INDEX j;
  
  for (j=length1-1; j>=0; j--) free((char*)(mat[j]));
  free((char*)(mat));
}

void convert_to_lower( char *string_pointer )
{

  while (*string_pointer != '\0')
    {
      if ( is_upper_case(*string_pointer) )
	*string_pointer = to_lower_case(*string_pointer);
      string_pointer++;
    }
}

int is_upper_case(char ch)
/*
** These next two functions should be built into string.h
** as "isupper" and "tolower" on most systems, but to be
** sure, make them here...
*/
{
  if ( ch>=65 && ch<=90 )
    return(1);
  else
    return(0);
}

char to_lower_case(char ch)
{
  char lower;

  lower = ch + 32;
  return(lower);
}

char *next_non_white( char *cptr )
{
  /* A white space is a space (ASCII 32) or tab (ASCII 9). */
  while ( *cptr == ' ' || *cptr == 9 ) cptr++;
  return( cptr );
}

int my_getopt( int argc, char *arg_list[], const char *optstring, int *iarg )
     // argc is number of arguments
     // arg_list contains the arguments
     // optstring is, e.g., "A:o:l:stv", where : means an argument follows
     // iarg points to argument in array arg_list
{
  static int argc_index = 1;
  char *test_arg, *opt;
  int ioption;

  // return an option of zero when all the arguments have been read.
  if ( argc_index >= argc )
    {
      ioption = 0;
      return(ioption);
    }
  // As a special case, return "i" when the argument is not preceded by a dash.
  test_arg = arg_list[argc_index];
  if ( *test_arg != '-' )
    {
      ioption = 'i';
      *iarg = argc_index;
      argc_index++;
      return(ioption);
    }
  // Subsequent tests should always start with '-'
  test_arg++; // get the next character after '-'
  for (opt=(char *)optstring; *opt!=CNULL; opt++)
    {
      if ( *test_arg == *opt )
	{
	  ioption = *opt;
	  if ( *(opt+1) == ':' ) argc_index++;
	  *iarg = argc_index;
	  if ( *iarg > argc ) *iarg = argc;
	  argc_index++;
	  return(ioption);
	}
    }
  //
  *iarg = argc_index;
  return(-1);
}

char *get_line( FILE *stream, char *string, int length )
{
  char *cptr;

  cptr = fgets(string, LEN_STR, stream);
  if ( cptr == NULL )
    return(NULL);
  else
    {
      while ( cptr != NULL && *cptr != CNULL )
	cptr++;
      cptr = string;
      return(cptr);
    }
}

char *get_line_with_comment( FILE *stream, char *string, int length )
{
  char *cptr;

  cptr = fgets(string, LEN_STR, stream);
  if ( cptr == NULL )
    return(NULL);
  else
    {
      while ( cptr != NULL && *cptr != CNULL )
	{
	  if ( *cptr == '#' )
	    *cptr = CNULL;
	  cptr++;
	}
      cptr = string;
      return(cptr);
    }
}

int invert_3x3_matrix( float **matrix )
{
  double determinant;
  float inverse[3][3];
  int j1, j2;

  determinant =
    matrix[0][0]*matrix[1][1]*matrix[2][2] -
    matrix[0][0]*matrix[2][1]*matrix[1][2] -
    matrix[1][0]*matrix[0][1]*matrix[2][2] +
    matrix[1][0]*matrix[2][1]*matrix[0][2] +
    matrix[2][0]*matrix[0][1]*matrix[1][2] -
    matrix[2][0]*matrix[1][1]*matrix[0][2] ;

  if( determinant != 0.0l )
    determinant = 1.0l / determinant ;
  else
    return(1); // singular

  inverse[0][0] = determinant*( matrix[1][1]*matrix[2][2]-matrix[2][1]*matrix[1][2]);
  inverse[0][1] = determinant*(-matrix[0][1]*matrix[2][2]+matrix[2][1]*matrix[0][2]);
  inverse[0][2] = determinant*( matrix[0][1]*matrix[1][2]-matrix[1][1]*matrix[0][2]);

  inverse[1][0] = determinant*(-matrix[1][0]*matrix[2][2]+matrix[2][0]*matrix[1][2]);
  inverse[1][1] = determinant*( matrix[0][0]*matrix[2][2]-matrix[2][0]*matrix[0][2]);
  inverse[1][2] = determinant*(-matrix[0][0]*matrix[1][2]+matrix[1][0]*matrix[0][2]);

  inverse[2][0] = determinant*( matrix[1][0]*matrix[2][1]-matrix[2][0]*matrix[1][1]);
  inverse[2][1] = determinant*(-matrix[0][0]*matrix[2][1]+matrix[2][0]*matrix[0][1]);
  inverse[2][2] = determinant*( matrix[0][0]*matrix[1][1]-matrix[1][0]*matrix[0][1]);

  for (j1=0; j1<3; j1++)
    {
      for (j2=0; j2<3; j2++)
	matrix[j1][j2] = inverse[j1][j2];
    }

  return(0); // success
}

int rInvertSquareMatrix( int nMat, float **fSquareMatrix, int *iRowFail )
     // alternative floating point call to matrix inversion on type double
{
  int j1, j2, ireturn;
  double **dSquareMatrix;

  dSquareMatrix = double_matrix(nMat,nMat);
  for (j1=0; j1<nMat; j1++)
    {
      for (j2=0; j2<nMat; j2++)
	dSquareMatrix[j1][j2] = fSquareMatrix[j1][j2];
    }
  ireturn = dInvertSquareMatrix( nMat, dSquareMatrix, iRowFail );
  for (j1=0; j1<nMat; j1++)
    {
      for (j2=0; j2<nMat; j2++)
	fSquareMatrix[j1][j2] = dSquareMatrix[j1][j2];
    }
  free_double_matrix(dSquareMatrix,nMat,nMat);

  return(ireturn);
}

int dInvertSquareMatrix( int nMat, double **dSquareMatrix, int *iRowFail )
{
  //  Purpose
  //  =======
  //  Compute an LU factorization of a square matrix using partial pivoting with row interchanges.
  //
  //  The factorization has the form
  //     dSquareMatrix = P * L * U
  //  where P is a permutation matrix, L is lower triangular with unit
  //  diagonal elements (lower trapezoidal if m > n), and U is upper
  //  triangular (upper trapezoidal if m < n).
  //
  //  This is based upon LAPACK using:
  //  1) dgetf2.f to create the PLU decomposition,
  //  2) 
  //
  //  Arguments
  //  =========
  //
  //  nMat    The dimensions of the square matrix.
  //
  //  dSquareMatrix       (input/output) dimension (nMat,nMat)
  //          On entry, the matrix to be factored.
  //          On exit, the factors L and U from the factorization
  //          dSquareMatrix = P*L*U; the unit diagonal elements of L are not stored.
  //
  //  iPivot    (output) INTEGER array, dimension (nMat,nMat)
  //          The pivot indices; for 1 <= i <= min(nMat,nMat), row i of the
  //          matrix was interchanged with row iPivot(i).
  //
  //  iRowFail    (output) INTEGER
  //         >= 0: the row number that produce the failure due to 0 pivot value.
  //         <  0: some other failure.
  //  =====================================================================
  int jRow, jCol, j1, j2;
  double dMax, dTemp, dDiag, *dWork;
  int *iPivot;

  *iRowFail = -1; // default value
  if ( nMat < 0 )
    {
      printf("Error: the matrix order is %d\n",nMat);
      return(1);
    }

  // Allocate working space.
  iPivot = int_vector(nMat);
  dWork = double_vector(nMat);

  ////////////////////// dgetf2.f from LAPACK ////////////////////////////////
  for (jRow=0; jRow<nMat; jRow++)
    {
      // Choose the largest element to use as the pivot, & test for singularity.
      iPivot[jRow] = jRow;
      dMax = fabs(dSquareMatrix[jRow][jRow]);
      for (j1=jRow+1; j1<nMat; j1++)
	{
	  if ( fabs(dSquareMatrix[j1][jRow]) > dMax )
	    {
	      dMax = fabs(dSquareMatrix[j1][jRow]);
	      iPivot[jRow] = j1;
	    }
	}
      if ( dSquareMatrix[iPivot[jRow]][jRow] != 0. )
	{
	  // Interchange rows for all columns.
	  // E.g., the pivot from the 1st row reorders all columns,
	  // the pivot from the 2nd row reorders all columns from the 2nd row down, ...
	  if ( iPivot[jRow] != jRow )
	    {
	      for (jCol=0; jCol<nMat; jCol++)
		{
		  dTemp                             = dSquareMatrix[jRow][jCol];
		  dSquareMatrix[jRow][jCol]         = dSquareMatrix[iPivot[jRow]][jCol];
		  dSquareMatrix[iPivot[jRow]][jCol] = dTemp;
		}
	    }
	  // Compute elements of jRow-th column.
	  for (j1=jRow+1; j1<nMat; j1++)
	    dSquareMatrix[j1][jRow] /= dSquareMatrix[jRow][jRow] ;
	}
      else
	{
	  *iRowFail = jRow;
	  free_double_vector(dWork);
	  free_int_vector(iPivot);
	  return(1);
	}

      // Update trailing submatrix.
      for (j2=jRow+1; j2<nMat; j2++)
	{
	  for (j1=jRow+1; j1<nMat; j1++)
	    dSquareMatrix[j1][j2] -= dSquareMatrix[j1][jRow]*dSquareMatrix[jRow][j2];
	}
    }
  ////////////////////// end of dgetf2.f ////////////////////////////////

  ////////////////////// dtrti2.f from LAPACK ////////////////////////////////
  // Compute the inverse of the upper triangular matrix.
  for (jCol=0; jCol<nMat; jCol++)
    {
      dSquareMatrix[jCol][jCol] = 1. / dSquareMatrix[jCol][jCol];
      dDiag = -dSquareMatrix[jCol][jCol];
      // Compute elements 1:j-1 of jCol-th column.
      for ( jRow=0; jRow<=jCol-1; jRow++)
	{
	  for (j1=0; j1<=jRow-1; j1++)
	    dSquareMatrix[j1][jCol] += dSquareMatrix[j1][jRow] * dSquareMatrix[jRow][jCol];
	  dSquareMatrix[jRow][jCol] *= dSquareMatrix[jRow][jRow];
	}
      for (j1=0; j1<=jCol-1; j1++)
	dSquareMatrix[j1][jCol] *= dDiag;
    }
  ////////////////////// end of dtrti2.f ////////////////////////////////

  ////////////////////// dgetri.f from LAPACK ////////////////////////////////
  // Solve the equation inv(dSquareMatrix)*L = inv(U) for inv(dSquareMatrix).
  for (jCol = nMat-1; jCol>=0; jCol--)
    {
      // Copy current column of L to dWork and replace with zeros.
      for (jRow=jCol+1; jRow<nMat; jRow++)
	{
	  dWork[jRow] = dSquareMatrix[jRow][jCol];
	  dSquareMatrix[jRow][jCol] = 0.;
	}
      // Compute current column of inv(dSquareMatrix).
      if (jCol < nMat-1 )
	{
	  for ( jRow=0; jRow<nMat-jCol-1; jRow++ )
	    {
	      for (j2=0; j2<nMat; j2++)
		dSquareMatrix[j2][jCol] -= dSquareMatrix[j2][jCol+jRow+1] * dWork[jCol+jRow+1];;
	    }									    
	}
    }
  // Interchange columns for each row.
  for (jCol=nMat-2; jCol>=0; jCol--)
    {
      if ( iPivot[jCol] != jCol )
	{
	  for (jRow=0; jRow<nMat; jRow++)
	    {
	      dTemp                             = dSquareMatrix[jRow][jCol];
	      dSquareMatrix[jRow][jCol]         = dSquareMatrix[jRow][iPivot[jCol]];
	      dSquareMatrix[jRow][iPivot[jCol]] = dTemp;
	    }
	}
    }
  ////////////////////// end of dgetri.f ////////////////////////////////
  free_double_vector(dWork);
  free_int_vector(iPivot);

  return(0); // success
}

float Kaiser_Bessel( double x, double W, double beta )
{
  double bess;
  double arg;

  arg = 1. - 4.*x*x/W/W;

  if ( arg < 0. )
    return(0.);
  else
    {
      arg = beta * sqrt(arg);
      bess = bessi0( arg );
      return( bess/W );
    }
}

FCOMPLEX inverse_Kaiser_Bessel( double x, double W, double beta )
{
  FCOMPLEX invKB;
  double arg, dsinc, norm;

  norm = sinh(beta)/beta; // makes sure result = 1 when x = 0
  arg  = PI*PI * W*W * x*x - beta*beta;
  if ( arg >= 0. )
    {
      arg  = sqrt( arg );
      if ( arg < 1.0e-4 )
      	dsinc = 1.0 ;
      else
        dsinc = sin( arg ) / arg;
      invKB.real = dsinc / norm;
      invKB.imag = 0.;
    }
  else
    {
      arg *= -1.;
      arg  = sqrt( arg );
      if ( arg < 1.0e-4 )
      	dsinc = 1.0;
      else
        dsinc = sinh( arg ) / arg;
      invKB.real = 0.;
      invKB.imag = dsinc/norm;
    }
  return( invKB );
}

double bessi0( double x )
{
  double ax, ans, y;

  if ( (ax=fabs(x)) < 3.75 )
    {
      y=x/3.75;
      y*=y;
      ans=1.0+y*(3.5156229+y*(3.0899424+y*(1.2067492
      +y*(0.2659732+y*(0.360768e-1+y*0.45813e-2)))));
    }
  else
    {
      y=3.75/ax;
      ans=(exp(ax)/sqrt(ax))*(0.39894228+y*(0.1328592e-1
      +y*(0.225319e-2+y*(-0.157565e-2+y*(0.916281e-2
      +y*(-0.2057706e-1+y*(0.2635537e-1+y*(-0.1647633e-1
      +y*0.392377e-2))))))));
    }
  return ans;
}

float Gauss(float x, float y, float sigma)
{
  double arg;
  float gauss;

  arg = - 0.5 * (x*x+y*y) / sigma / sigma;
  gauss = exp( arg );
  return( gauss );
}

float inverse_Gauss(float x, float y,float sigma)
{
  double arg;
  float gauss;

  /*  arg = - 0.5 * sigma * sigma * (x*x+y*y); */
  arg = - 2 * PI * PI * sigma * sigma * (x*x+y*y);
  gauss = exp( arg );
  return( gauss );
}

// Compute the phase of complex number.  Use the interval -PI to PI.
float compute_phase(FCOMPLEX FC)
{
  float phase;

  if ( FC.real != 0. )
    {
      phase = atan((double)(FC.imag/FC.real));
      if ( FC.real < 0. ) phase += PI;
      if ( phase > PI ) phase -= 2*PI;
     }
  else
    {
      if ( FC.imag == 0. )
        phase = 0.;
      else if ( FC.imag > 0. )
        phase = PI/2.;
      else
        phase = - PI/2.;
    }
  return(phase);
}

float compute_amplitude( FCOMPLEX FC )
{
  float amp;

  amp = sqrt(FC.real*FC.real + FC.imag*FC.imag);
  return(amp);
}

void swap_x( FCOMPLEX *FCData, int lDim, int lAdd )
     // FCData: floating complex data
     // lDim:   dimensionality of FCData
     // lAdd:   for lDim odd, call as
     //         lAdd = 0 before FT
     //         lAdd = 1 after FT  (this put array back into original order)
{
  FCOMPLEX FC1, *FCStore;
  int j, lSwap1, lSwap2, lShift;
  
  if ( lDim%2 )
    { // the dimensionality is odd
      FCStore = fcomplex_vector(lDim);
      for (j=0; j<lDim; j++)
	FCStore[j] = FCData[j];
      for (j=0; j<lDim; j++)
	{
	  lShift = j + lDim/2 + lAdd;
	  if ( lShift >= lDim )
	    lShift -= lDim;
	  FCData[lShift] = FCStore[j];
	}
      free_fcomplex_vector(FCStore);
    }
  else
    {  // the dimensionality is even (typical case)
      // The shift becomes a swap.
      for (j=0; j<lDim/2; j++)
	{
	  lSwap1 = j;
	  lSwap2 = lSwap1 + lDim/2;
	  FC1       = FCData[lSwap1];
	  FCData[lSwap1] = FCData[lSwap2];
	  FCData[lSwap2] = FC1;
	}
    }
}

float ham_sinc( float fx, int iX )
{ // iX is width of filter (+- iX); good tradeoff for ringing vs. smoothing
  double filter;
  double x,X;
  x = fx;
  X = iX;
  
  if ( fabs(x) < 1.0e-5 )    // avoid division by zero
    filter = 1.0;
  else if ( fabs(x/X) > 1. ) // outside range of filter
    filter = 0.;
  else
    {
      filter = sin(PI*x)/(PI*x);
      filter *= 0.54 + 0.46 * cos(PI*x/X);
    }
  return(filter);
}

float Lanczos_filter( float fx, int iX )
{ // iX is width of filter (+- iX); good tradeoff for ringing vs. smoothing
  // This filter is almost identical to ham_sinc above
  double x, X, filter;
  x = fx;
  X = iX;
 
  if ( fabs(x) < 1.0e-5 )    // avoid division by zero
    filter = 1.0;
  else if ( fabs(x/X) > 1. ) // outside range of filter
    filter = 0.;
  else
    {
      filter = X/SQR(PI*x) * sin(PI*x) * sin(PI*x/X);
    }
  return(filter);
}

BOOL is_power_of_2( int lInput, int *lLastPower, int *lNextPower )
{
  int lTest, lFound=0;

  lTest  = 1;
  do
    if ( (lTest*=2) == lInput ) lFound = lTest;
  while ( lTest < lInput );

  if ( lFound )
    {
      *lLastPower = lInput/2;
      *lNextPower = 2*lInput;
      return(TRUE);
    }
  else
    {
      *lLastPower = lTest/2;
      *lNextPower = lTest;
      return(FALSE);
    }
}

FCOMPLEX FCMult(FCOMPLEX FC1, FCOMPLEX FC2)
{
  // FC1 * FC2
  FCOMPLEX FC;

  FC.real = FC1.real * FC2.real - FC1.imag * FC2.imag;
  FC.imag = FC1.real * FC2.imag + FC1.imag * FC2.real;
  return(FC);
}

FCOMPLEX FCMultStar(FCOMPLEX FC1, FCOMPLEX FC2)
{
  // FC1 * (FC2*)
  FCOMPLEX FC;

  FC.real = FC1.real * FC2.real + FC1.imag * FC2.imag;
  FC.imag = FC1.imag * FC2.real - FC1.real * FC2.imag;
  return(FC);
}

FCOMPLEX FCAdd(FCOMPLEX FC1, FCOMPLEX FC2)
{
  // FC1 * FC2
  FCOMPLEX FC;

  FC.real = FC1.real + FC2.real;
  FC.imag = FC1.imag + FC2.imag;

  return(FC);
}

float Trace_of_Matrix( int NSquare, float **RealMatrix )
{
  float trace;
  int j;
  
  trace = 0.;
  for (j=0; j<NSquare; j++)
    trace += RealMatrix[j][j];
  return(trace);
}

void Matrix_Product( float **X1X2, int Length1, int Length2,
		     float **X1, int iTranspose1, int Length12, float **X2, int iTranspose2)
     // create X1X2[Length1][Length2] as inner product of X1 and X2
     //
     // X1X2_ij = X1_ik * X2_kj,   where
     //       i has Length1
     //       j has Length2
     //       k has Length12; this is the summation index
     //
     // If iTranpose1 == 1, then switch indices of matrix 1
     // If iTranpose2 == 1, then switch indices of matrix 2
{
  int j1, j12, j2;

  for (j1=0; j1<Length1; j1++)
    {
      for (j2=0; j2<Length2; j2++)
	{
	  X1X2[j1][j2] = 0.;
	  if ( iTranspose1 == 0 )
	    {
	      if ( iTranspose2 == 0 )
		{ // no transpose: X1X2 = X1 * X2
		  for (j12=0; j12<Length12; j12++)
		    X1X2[j1][j2] += X1[j1][j12] * X2[j12][j2];
		}
	      else
		{ // transpose 2nd matrix: X1X2 = X1 * X2^Transpose
		  for (j12=0; j12<Length12; j12++)
		    X1X2[j1][j2] += X1[j1][j12] * X2[j2][j12];
		}
	    }
	  else
	    {
	      if ( iTranspose2 == 0 )
		{ // transpose the 1st matrix: X1X2 = X1^Transpose * X2
		  for (j12=0; j12<Length12; j12++)
		    X1X2[j1][j2] += X1[j12][j1] * X2[j12][j2];
		}
	      else
		{ // transpose both matrices: X1X2 = X1^Transpose * X2^Transpose
		  for (j12=0; j12<Length12; j12++)
		    X1X2[j1][j2] += X1[j12][j1] * X2[j2][j12];
		}
	    }
	}
    }
}

void Matrix_Vector_Product( float *X1X2, int Length, float **X1, int iTranspose,
			    int Length12, float *X2)
     // create X1X2[Length] as inner product of X1 and X2
     //
     // X1X2_i = X1_ik * X2_k,   where
     //       i has Length
     //       k has Length12; this is the summation index
     //
     // If iTranpose == 1, then switch indices of matrix X1
{
  int j1, j12;

  for (j1=0; j1<Length; j1++)
    {
      X1X2[j1] = 0.;
      if ( iTranspose == 0 )
	{ // no transpose: X1X2 = X1 * X2
	  for (j12=0; j12<Length12; j12++)
	    X1X2[j1] += X1[j1][j12] * X2[j12];
	}
      else
	{ // transpose 2nd matrix: X1X2 = X1 * X2^Transpose
	  for (j12=0; j12<Length12; j12++)
	    X1X2[j1] += X1[j12][j1] * X2[j12];
	}
    }
}

void Matrix_PseudoInverse( float **X_12, int Length1, int Length2, float **piX_21 )
{
  // Calculate the pseudo-inverse of X_12[i1][i2]: piX_21[i2][i1]
  float **XTXm1_22;
  int iRowFail;

  XTXm1_22 = float_matrix(Length2,Length2);

  // Calculate XTX
  //           mat_product  dim1     dim2    mat1  T  dim_sum  mat2  T
  Matrix_Product(XTXm1_22, Length2, Length2, X_12, 1, Length1, X_12, 0);

  // Invert the matrix to get (XTX)^-1
  rInvertSquareMatrix( Length2, XTXm1_22, &iRowFail );

  // Calculate the pseudo-inverse as (XTX)^-1 XT
  //           mat_product  dim1     dim2    mat1    T  dim_sum  mat2  T
  Matrix_Product(piX_21, Length2, Length1, XTXm1_22, 0, Length2, X_12, 1);

  // Free the temporary working matrix.
  free_float_matrix(XTXm1_22,Length2, Length2);

}

mat44 invert_mat44( mat44 Input44 )
{
  mat44 Invert44;
  float **matrix;
  int j1, j2, Error, iRowFail;

  matrix = float_matrix(4,4);
  for (j1=0; j1<4; j1++)
    {
      for (j2=0; j2<4; j2++)
	matrix[j1][j2] = Input44.m[j1][j2];
    }
  Error = rInvertSquareMatrix(4, matrix, &iRowFail);
  if ( Error )
    {
      free_float_matrix(matrix,4,4);
      printf("failed on row %d\n",iRowFail);
      exit(1);
    }
  for (j1=0; j1<4; j1++)
    {
      for (j2=0; j2<4; j2++)
	Invert44.m[j1][j2] = matrix[j1][j2];
    }
  /* Called for every coordinate transformation during fitting. This temporary
     used to leak five allocations on every inversion (matrix + four rows). */
  free_float_matrix(matrix,4,4);
  return Invert44;
}

// Set up a simple scale-and-shift transformation from the pixel dimesions and offsets.
// Relative to the quaternion formulation in "nifti_quatern_to_mat44",
// the rotation matrix is the identity matrix, and "qfac" = 1;
mat44 scale_and_shift_to_mat44(POINT3D resolution, POINT3D origin, POINT3D direction)
{
  mat44 R;
  int j1, j2;

  // Create a 4x4 identity matrix.
  for (j1=0; j1<4; j1++)
    {
      for (j2=0; j2<4; j2++)
	{
	  if ( j1==j2 )
	    R.m[j1][j2] = 1.;
	  else
	    R.m[j1][j2] = 0.;
	}
    }
  // Attach the pixel dimensions.
  R.m[0][0] *= resolution.x * direction.x;
  R.m[1][1] *= resolution.y * direction.y;
  R.m[2][2] *= resolution.z * direction.z;
  // Attach the pixel offsets.
  R.m[0][3] = origin.x;
  R.m[1][3] = origin.y;
  R.m[2][3] = origin.z;

  return R ;
}

mat44 general_affine_to_mat44( float *srow_x, float *srow_y, float *srow_z )
{
  mat44 R;
  int j1, j2;
  BOOL AllZero[3];

  R.m[0][0] = srow_x[0];
  R.m[0][1] = srow_x[1];
  R.m[0][2] = srow_x[2];
  R.m[0][3] = srow_x[3];
    
  R.m[1][0] = srow_y[0];
  R.m[1][1] = srow_y[1];
  R.m[1][2] = srow_y[2];
  R.m[1][3] = srow_y[3];
    
  R.m[2][0] = srow_z[0];
  R.m[2][1] = srow_z[1];
  R.m[2][2] = srow_z[2];
  R.m[2][3] = srow_z[3];
    
  // last row is always [ 0 0 0 1 ]
  R.m[3][0]=R.m[3][1]=R.m[3][2]=0.0;  R.m[3][3]=1.0;

  for (j2=0; j2<3; j2++)
    {
      AllZero[j2] = TRUE;
      for (j1=0; j1<3; j1++)
	AllZero[j2] &= R.m[j2][j1] == 0.;
    }
  if ( AllZero[0] || AllZero[1] || AllZero[2] )
    {
      printf("Invalid Affine transformation matrix: resetting to identity matrix.\n");
      for (j2=0; j2<3; j2++)
	{
	  for (j1=0; j1<3; j1++)
	    {
	      if ( j1 == j2 )	
		R.m[j1][j2] = 1.;
	      else
		R.m[j1][j2] = 0.;
	    }
  	}
    }
  return R ;
}

mat44 centered_ijk_to_xyz( struct data_stack target )
{ // target voxels into space
  struct data_stack temp;
  mat44 mat;

  temp = target;  // this is the template
  temp.origin.x = -(temp.dim.x-1)*temp.resolution.x/2.;
  temp.origin.y = -(temp.dim.y-1)*temp.resolution.y/2.;
  temp.origin.z = -(temp.dim.z-1)*temp.resolution.z/2.;
  //  temp.origin.x = temp.origin.y = temp.origin.z = 0.;
  temp.direction.x = temp.direction.y = temp.direction.z = 1.;
  temp.ijk_to_xyz = scale_and_shift_to_mat44(temp.resolution,temp.origin,temp.direction);

  mat = temp.ijk_to_xyz;
  return(mat);
}

mat44 centered_xyz_to_ijk( struct data_stack source )
{ // space into source voxels
  struct data_stack temp;
  mat44 mat;

  temp = source;  // this is the data to align
  temp.origin.x  = -(temp.dim.x-1)*temp.resolution.x/2.;
  temp.origin.y  = -(temp.dim.y-1)*temp.resolution.y/2.;
  temp.origin.z  = -(temp.dim.z-1)*temp.resolution.z/2.;
  temp.direction.x = temp.direction.y = temp.direction.z = 1.;
  temp.ijk_to_xyz = scale_and_shift_to_mat44(temp.resolution,temp.origin,temp.direction);

  mat = invert_mat44( temp.ijk_to_xyz );
  return(mat);
}

POINT3D Convert_Voxel_to_Space(IPOINT3D iDataIndex, struct data_stack *stack)
{
  POINT3D DataIndex, Space;

  DataIndex.x = iDataIndex.x;
  DataIndex.y = iDataIndex.y;
  DataIndex.z = iDataIndex.z;
  Space = Convert_rVoxel_to_Space(DataIndex, stack);

  return(Space);
}

POINT3D Convert_rVoxel_to_Space(POINT3D DataIndex, struct data_stack *stack)
{
  POINT3D Space;
  float DataVector[4];
  int j;

  DataVector[0] = DataIndex.x;
  DataVector[1] = DataIndex.y;
  DataVector[2] = DataIndex.z;
  DataVector[3] = 1;

  for ( Space.x=0, j=0; j<4; j++)
    Space.x += stack->ijk_to_xyz.m[0][j] * DataVector[j];
  for ( Space.y=0, j=0; j<4; j++)
    Space.y += stack->ijk_to_xyz.m[1][j] * DataVector[j];
  for ( Space.z=0, j=0; j<4; j++)
    Space.z += stack->ijk_to_xyz.m[2][j] * DataVector[j];

  return(Space);
}

POINT3D Convert_Space_to_rVoxel(POINT3D Space, struct data_stack *stack)
{
  POINT3D DataIndex;
  float SpaceVector[4];
  int j;

  SpaceVector[0] = Space.x;
  SpaceVector[1] = Space.y;
  SpaceVector[2] = Space.z;
  SpaceVector[3] = 1;

  for ( DataIndex.x=0, j=0; j<4; j++)
    DataIndex.x += stack->xyz_to_ijk.m[0][j] * SpaceVector[j];
  for ( DataIndex.y=0, j=0; j<4; j++)
    DataIndex.y += stack->xyz_to_ijk.m[1][j] * SpaceVector[j];
  for ( DataIndex.z=0, j=0; j<4; j++)
    DataIndex.z += stack->xyz_to_ijk.m[2][j] * SpaceVector[j];

  return(DataIndex);
}

void downsample_stack( struct data_stack *target, IPOINT3D DimensionLow )
{ // Down-sample a data_stack into DimensionLow. Assume centered coordinates in the low-res stack.
  FCOMPLEX *fcData_orig, *fcData_reg;
  DCOMPLEX sum_data;
  float *fData_orig, *fData_reg;
  double sum_coeff, weight;
  int jt;JIP_INDEX n3d, n4d, index, j;
  IPOINT3D Index_reg, Low_orig, High_orig, jOrig, jRel, iWidth;
  POINT3D  ResolutionLow, fwhm, Space_reg, Index_orig, SaveOrigin, SaveDirection;
  float *coeff_x, *coeff_y, *coeff_z;
  BOOL InVolume;

  // This function assumes that the target matrix has the original
  // (NOT down-sampled) dimensions & resolutions at the start of the function.
  // DimensionLow should be <= target->dimension

  // FWHM refers to a gaussian smoothing kernel (= ResolutionLow/target->resolution)
  fwhm.x =  (float)target->dim.x / (float)DimensionLow.x;
  fwhm.y =  (float)target->dim.y / (float)DimensionLow.y;
  fwhm.z =  (float)target->dim.z / (float)DimensionLow.z;
  ResolutionLow.x = target->resolution.x * fwhm.x;
  ResolutionLow.y = target->resolution.y * fwhm.y;
  ResolutionLow.z = target->resolution.z * fwhm.z;

  // sample over +- fwhm
  iWidth.x = MAX(floor((double)(fwhm.x)),1);
  iWidth.y = MAX(floor((double)(fwhm.y)),1);
  iWidth.z = MAX(floor((double)(fwhm.z)),1);

  coeff_x = float_vector(2*iWidth.x+2);   // total width is at least 2*width+1
  coeff_y = float_vector(2*iWidth.y+2);
  coeff_z = float_vector(2*iWidth.z+2);

  n3d     = jip_product(jip_product(DimensionLow.x,DimensionLow.y),DimensionLow.z);
  n4d     = jip_product(n3d,target->nt);
  if ( target->data_type == 0 )
    {
      fData_orig = target->SpaceTime.fData;
      fData_reg  = float_vector(n4d);
      for (j=0; j<n4d; j++)
	fData_reg[j] = 0.;
    }
  else
    {
      fcData_orig = target->SpaceTime.fcData;
      fcData_reg  = fcomplex_vector(n4d);
      for (j=0; j<n4d; j++)
	fcData_reg[j].real = fcData_reg[j].imag = 0.;
    }

  // set the target coordinate transformation (based upon own resolution & matrix)
  target->xyz_to_ijk = centered_xyz_to_ijk(*target);  // space to voxels in high resolution

  // Save the origin & direction to attach later.
  SaveOrigin    = target->origin;
  SaveDirection = target->direction;

  // Reassign all target properties except for the matrix, which is needed below.
  target->resolution  = ResolutionLow;
  target->origin.x    = -(DimensionLow.x-1)*target->resolution.x/2.;
  target->origin.y    = -(DimensionLow.y-1)*target->resolution.y/2.;
  target->origin.z    = -(DimensionLow.z-1)*target->resolution.z/2.;
  target->direction.x = target->direction.y = target->direction.z = 1.;
  // The following is voxels to space in low resolution
  target->ijk_to_xyz = scale_and_shift_to_mat44(target->resolution,target->origin,target->direction);

  // At this point, the space->voxel transformation ijk->xyz is set for the downsampled target,
  // and the voxel->space transformation xyz->ijk is set for the original target,
  // which is what we want.

  for (jt=0; jt<target->nt; jt++)
    {
      // Loop over voxels in the downsampled resolution space.
      for (Index_reg.z=0; Index_reg.z<DimensionLow.z; Index_reg.z++)
	{
	  for (Index_reg.y=0; Index_reg.y<DimensionLow.y; Index_reg.y++)
	    {
	      for (Index_reg.x=0; Index_reg.x<DimensionLow.x; Index_reg.x++)
		{
		  // Convert from data indices --> space in the registered system.
		  Space_reg = Convert_Voxel_to_Space( Index_reg, target );
		  // Convert from space --> indices in the original space.
		  Index_orig = Convert_Space_to_rVoxel( Space_reg, target );
		  // define limits
		  Low_orig.x  = floor((double)Index_orig.x);
		  High_orig.x = ceil((double)Index_orig.x);
		  Low_orig.y  = floor((double)Index_orig.y);
		  High_orig.y = ceil((double)Index_orig.y);
		  Low_orig.z  = floor((double)Index_orig.z);
		  High_orig.z = ceil((double)Index_orig.z);
		  // Interpolate to rectilinear pixels in original resolution space.
		  for (jOrig.x=Low_orig.x-iWidth.x, jRel.x=0;
		       jOrig.x<=High_orig.x+iWidth.x; jOrig.x++, jRel.x++)
		    coeff_x[jRel.x] = Gauss((float)(Index_orig.x-jOrig.x), 0.,0.425*fwhm.x);
		  for (jOrig.y=Low_orig.y-iWidth.y, jRel.y=0;
		       jOrig.y<=High_orig.y+iWidth.y; jOrig.y++, jRel.y++)
		    coeff_y[jRel.y] = Gauss((float)(Index_orig.y-jOrig.y), 0.,0.425*fwhm.y);
		  for (jOrig.z=Low_orig.z-iWidth.z, jRel.z=0;
		       jOrig.z<=High_orig.z+iWidth.z; jOrig.z++, jRel.z++)
		    coeff_z[jRel.z] = Gauss((float)(Index_orig.z-jOrig.z), 0.,0.425*fwhm.z);
		  sum_coeff = sum_data.real = sum_data.imag = 0.;
		  for (jOrig.z=Low_orig.z-iWidth.z, jRel.z=0;
		       jOrig.z<=High_orig.z+iWidth.z; jOrig.z++, jRel.z++)
		    {
		      for (jOrig.y=Low_orig.y-iWidth.y, jRel.y=0;
			   jOrig.y<=High_orig.y+iWidth.y; jOrig.y++, jRel.y++)
			{
			  for (jOrig.x=Low_orig.x-iWidth.x, jRel.x=0;
			       jOrig.x<=High_orig.x+iWidth.x; jOrig.x++, jRel.x++)
			    {
			      InVolume = ( jOrig.x>=0 && jOrig.x<target->dim.x &&
					   jOrig.y>=0 && jOrig.y<target->dim.y &&
					   jOrig.z>=0 && jOrig.z<target->dim.z );
			      if ( InVolume )
				{
				  // sum over neighbors in x
				  index = ((JIP_INDEX)(jOrig.z))*target->dim.x*target->dim.y
				    +     ((JIP_INDEX)(jOrig.y))*target->dim.x + jOrig.x + jt*target->n3d;
				  weight = coeff_x[jRel.x] * coeff_y[jRel.y] * coeff_z[jRel.z];
				  sum_coeff += weight;
				  if ( target->data_type == 0 )
				    sum_data.real += weight * fData_orig[index];
				  else
				    {
				      sum_data.real += weight * fcData_orig[index].real;
				      sum_data.imag += weight * fcData_orig[index].imag;
				    }
				}
			    }
			}
		    }
		  index = ((JIP_INDEX)(Index_reg.z))*DimensionLow.x*DimensionLow.y
		    + ((JIP_INDEX)(Index_reg.y))*DimensionLow.x + Index_reg.x + jt*n3d;
		  if ( sum_coeff == 0. )
		    { // set values to zero without zero division
		      if ( target->data_type == 0 )
			fData_reg[index] = 0.;
		      else
			{
			  fcData_reg[index].real = fcData_reg[index].imag = 0.;
			}
		    }
		  else
		    { // normalize sums by coefficient sums
		      if ( target->data_type == 0 )
			fData_reg[index] = sum_data.real / sum_coeff;
		      else
			{
			  fcData_reg[index].real = sum_data.real / sum_coeff;
			  fcData_reg[index].imag = sum_data.imag / sum_coeff;
			}
		    }
		}
	    }
	}
      if ( target->nt > 5 )
	{
	  fprintf(stdout,"\rFinished %d of %d time points for stack `%s'.",
		  jt+1,target->nt,target->name);
	  fflush(stdout);
	}
    } // time loop
  if ( target->nt>5 ) printf("\n");
  free_float_vector(coeff_x);
  free_float_vector(coeff_y);
  free_float_vector(coeff_z);

  // Replace the space->voxel transformations with new ones.
  target->origin    = SaveOrigin;
  target->direction = SaveDirection;
  target->ijk_to_xyz = scale_and_shift_to_mat44(target->resolution,target->origin,target->direction);
  target->xyz_to_ijk = invert_mat44( target->ijk_to_xyz );
  // Reassign the data matrix to down-sampled values.
  target->dim = DimensionLow;
  target->n2d = jip_product(target->dim.x,target->dim.y);
  target->n3d = jip_product(target->n2d,target->dim.z);
  target->n4d = jip_product(target->n3d,target->nt);
  // Free the old data vector and point to the new one
  if ( target->data_type == 0 )
    {
      free_float_vector(target->SpaceTime.fData);
      target->SpaceTime.fData = fData_reg;
      target->npts_volume = target->n3d;
    }
  else
    {
      free_fcomplex_vector(target->SpaceTime.fcData);
      target->SpaceTime.fcData = fcData_reg;
      target->npts_volume = jip_product(2,target->n3d);
    }
}

void upsample_stack( struct data_stack *target, IPOINT3D DimensionHigh )
{ // Up-sample a data_stack into DimensionHigh.
  FCOMPLEX *fcData_orig, *fcData_reg;
  DCOMPLEX sum_data;
  float *fData_orig, *fData_reg;
  double sum_coeff, weight;
  int jt;JIP_INDEX n3d, n4d, index, j;
  IPOINT3D Index_reg, Low_orig, High_orig, jOrig, jRel, iWidth;
  POINT3D  ResolutionHigh, fwhm, Space_reg, Index_orig, SaveOrigin, SaveDirection;
  float *coeff_x, *coeff_y, *coeff_z;
  BOOL InVolume;

  // This function assumes that the target matrix has the original
  // (NOT up-sampled) dimensions & resolutions at the start of the function.
  // DimensionHigh should be >= target->dimension

  // FWHM refers to an interpolation factor (= ResolutionHigh/target->resolution < 1.)
  fwhm.x =  (float)target->dim.x / (float)DimensionHigh.x;
  fwhm.y =  (float)target->dim.y / (float)DimensionHigh.y;
  fwhm.z =  (float)target->dim.z / (float)DimensionHigh.z;
  ResolutionHigh.x = target->resolution.x * fwhm.x;
  ResolutionHigh.y = target->resolution.y * fwhm.y;
  ResolutionHigh.z = target->resolution.z * fwhm.z;

  // sample over +- 2 in the low dimensional space
  iWidth.x = iWidth.y = iWidth.z = 2;

  coeff_x = float_vector(2*iWidth.x+2);   // total width is at least 2*width+1
  coeff_y = float_vector(2*iWidth.y+2);
  coeff_z = float_vector(2*iWidth.z+2);

  n3d     = jip_product(jip_product(DimensionHigh.x,DimensionHigh.y),DimensionHigh.z);
  n4d     = jip_product(n3d,target->nt);
  if ( target->data_type == 0 )
    {
      fData_orig = target->SpaceTime.fData;
      fData_reg  = float_vector(n4d);
      for (j=0; j<n4d; j++)
	fData_reg[j] = 0.;
    }
  else
    {
      fcData_orig = target->SpaceTime.fcData;
      fcData_reg  = fcomplex_vector(n4d);
      for (j=0; j<n4d; j++)
	fcData_reg[j].real = fcData_reg[j].imag = 0.;
    }

  // set the target coordinate transformation (based upon own resolution & matrix)
  target->xyz_to_ijk = centered_xyz_to_ijk(*target);  // space to voxels in high resolution

  // Save the origin & direction to attach later.
  SaveOrigin    = target->origin;
  SaveDirection = target->direction;

  // Reassign all target properties except for the matrix, which is needed below.
  target->resolution  = ResolutionHigh;
  target->origin.x    = -(DimensionHigh.x-1)*target->resolution.x/2.;
  target->origin.y    = -(DimensionHigh.y-1)*target->resolution.y/2.;
  target->origin.z    = -(DimensionHigh.z-1)*target->resolution.z/2.;
  target->direction.x = target->direction.y = target->direction.z = 1.;
  // The following is voxels to space in high resolution
  target->ijk_to_xyz = scale_and_shift_to_mat44(target->resolution,target->origin,target->direction);

  // At this point, the space->voxel transformation ijk->xyz is set for the downsampled target,
  // and the voxel->space transformation xyz->ijk is set for the original target,
  // which is what we want.

  for (jt=0; jt<target->nt; jt++)
    {
      // Loop over voxels in the downsampled resolution space.
      for (Index_reg.z=0; Index_reg.z<DimensionHigh.z; Index_reg.z++)
	{
	  for (Index_reg.y=0; Index_reg.y<DimensionHigh.y; Index_reg.y++)
	    {
	      for (Index_reg.x=0; Index_reg.x<DimensionHigh.x; Index_reg.x++)
		{
		  // Convert from data indices --> space in the registered system.
		  Space_reg = Convert_Voxel_to_Space( Index_reg, target );
		  // Convert from space --> indices in the original space.
		  Index_orig = Convert_Space_to_rVoxel( Space_reg, target );
		  // define limits
		  Low_orig.x  = floor((double)Index_orig.x);
		  High_orig.x = ceil((double)Index_orig.x);
		  Low_orig.y  = floor((double)Index_orig.y);
		  High_orig.y = ceil((double)Index_orig.y);
		  Low_orig.z  = floor((double)Index_orig.z);
		  High_orig.z = ceil((double)Index_orig.z);
		  // Interpolate to rectilinear pixels in original resolution space.
		  for (jOrig.x=Low_orig.x-iWidth.x, jRel.x=0;
		       jOrig.x<=High_orig.x+iWidth.x; jOrig.x++, jRel.x++)
		    {
		      coeff_x[jRel.x] = Lanczos_filter((float)(Index_orig.x-jOrig.x),iWidth.x);
		    }
		  for (jOrig.y=Low_orig.y-iWidth.y, jRel.y=0;
		       jOrig.y<=High_orig.y+iWidth.y; jOrig.y++, jRel.y++)
		    {
		      coeff_y[jRel.y] = Lanczos_filter((float)(Index_orig.y-jOrig.y),iWidth.y);
		    }
		  for (jOrig.z=Low_orig.z-iWidth.z, jRel.z=0;
		       jOrig.z<=High_orig.z+iWidth.z; jOrig.z++, jRel.z++)
		    {
		      coeff_z[jRel.z] = Lanczos_filter((float)(Index_orig.z-jOrig.z),iWidth.z);
		    }
		  sum_coeff = sum_data.real = sum_data.imag = 0.;
		  for (jOrig.z=Low_orig.z-iWidth.z, jRel.z=0;
		       jOrig.z<=High_orig.z+iWidth.z; jOrig.z++, jRel.z++)
		    {
		      for (jOrig.y=Low_orig.y-iWidth.y, jRel.y=0;
			   jOrig.y<=High_orig.y+iWidth.y; jOrig.y++, jRel.y++)
			{
			  for (jOrig.x=Low_orig.x-iWidth.x, jRel.x=0;
			       jOrig.x<=High_orig.x+iWidth.x; jOrig.x++, jRel.x++)
			    {
			      InVolume = ( jOrig.x>=0 && jOrig.x<target->dim.x &&
					   jOrig.y>=0 && jOrig.y<target->dim.y &&
					   jOrig.z>=0 && jOrig.z<target->dim.z );
			      if ( InVolume )
				{
				  // sum over neighbors in x
				  index = ((JIP_INDEX)(jOrig.z))*target->dim.x*target->dim.y
				    +     ((JIP_INDEX)(jOrig.y))*target->dim.x + jOrig.x + jt*target->n3d;
				  weight = coeff_x[jRel.x] * coeff_y[jRel.y] * coeff_z[jRel.z];
				  sum_coeff += weight;
				  if ( target->data_type == 0 )
				    sum_data.real += weight * fData_orig[index];
				  else
				    {
				      sum_data.real += weight * fcData_orig[index].real;
				      sum_data.imag += weight * fcData_orig[index].imag;
				    }
				}
			    }
			}
		    }
		  index = ((JIP_INDEX)(Index_reg.z))*DimensionHigh.x*DimensionHigh.y
		    + ((JIP_INDEX)(Index_reg.y))*DimensionHigh.x + Index_reg.x + jt*n3d;
		  if ( sum_coeff == 0. )
		    { // set values to zero without zero division
		      if ( target->data_type == 0 )
			fData_reg[index] = 0.;
		      else
			{
			  fcData_reg[index].real = fcData_reg[index].imag = 0.;
			}
		    }
		  else
		    { // normalize sums by coefficient sums
		      if ( target->data_type == 0 )
			fData_reg[index] = sum_data.real / sum_coeff;
		      else
			{
			  fcData_reg[index].real = sum_data.real / sum_coeff;
			  fcData_reg[index].imag = sum_data.imag / sum_coeff;
			}
		    }
		}
	    }
	}
      if ( target->nt > 5 )
	{
	  fprintf(stdout,"\rFinished %d of %d time points for stack `%s'.",
		  jt+1,target->nt,target->name);
	  fflush(stdout);
	}
    } // time loop
  if ( target->nt>5 ) printf("\n");
  free_float_vector(coeff_x);
  free_float_vector(coeff_y);
  free_float_vector(coeff_z);

  // Replace the space->voxel transformations with new ones.
  target->origin    = SaveOrigin;
  target->direction = SaveDirection;
  target->ijk_to_xyz = scale_and_shift_to_mat44(target->resolution,target->origin,target->direction);
  target->xyz_to_ijk = invert_mat44( target->ijk_to_xyz );
  // Reassign the data matrix to down-sampled values.
  target->dim = DimensionHigh;
  target->n2d = jip_product(target->dim.x,target->dim.y);
  target->n3d = jip_product(target->n2d,target->dim.z);
  target->n4d = jip_product(target->n3d,target->nt);
  // Free the old data vector and point to the new one
  if ( target->data_type == 0 )
    {
      free_float_vector(target->SpaceTime.fData);
      target->SpaceTime.fData = fData_reg;
      target->npts_volume = target->n4d;
    }
  else
    {
      free_fcomplex_vector(target->SpaceTime.fcData);
      target->SpaceTime.fcData = fcData_reg;
      target->npts_volume = jip_product(2,target->n4d);
    }
}

int help_menu(int iLevel, char *file_name)
{
  static char saved_help_file_name[LEN_STR];
  char help_file_name[LEN_STR], string[LEN_STR];
  char *jip_home, *cptr;
  int nEntries, nLines, **Menu_entry, nMainMenus, *nSubMenu, **Menu_location, **Menu_length;
  int iMenu_expectation, iSubMenu_expectation, max_SubMenu, iLength, iMainMenu, iSubMenu, iChoice;
  int jEntry, jLine, nread;
  FILE *file;

  jip_home = getenv("JIP_HOME");
  if ( !jip_home )
    {
      printf("For help, quit and define the environment variable JIP_HOME, and then re-start.\n");
      return(1);
    }
  strcpy(help_file_name,jip_home);
  strcat(help_file_name,"/documentation/");
  strcat(help_file_name,file_name);

  // Open the file containing the list of runs, GLM files, ...
  if ( !(file = fopen(help_file_name,"r")) )
    {
      printf("Error: cannot open the file %s.\n",help_file_name);
      printf("The environment variable JIP_HOME is set to %s\n",jip_home);
      printf("If this is incorrect, quit, define this environment variable, and re-start.\n");
      return(1);
    }
  else if ( iLevel == 0 )
    strcpy(saved_help_file_name,file_name);

  printf("Help from file %s\n",help_file_name);

  // Find the number of menu entries in the file, indicated by '!' in the 1st column.
  nEntries = 0;
  while ( get_line(file, string, LEN_STR) )
    {
      if ( string[0] == '!' )
	nEntries++;
    }
  rewind(file);
  // Read the menu entries: 2 integers following '!'.
  Menu_entry = int_matrix(nEntries,2);
  nEntries   = 0;
  while ( get_line(file, string, LEN_STR) )
    {
      cptr = string;
      if ( *cptr == '!' )
	{
	  cptr++;
	  sscanf(cptr,"%d %d",&(Menu_entry[nEntries][0]),&(Menu_entry[nEntries][1]));
	  nEntries++;
	}
    }
  rewind(file);

  // Make sure the menu entries make sense.
  iMenu_expectation = iSubMenu_expectation = 0;
  for (jEntry=0, max_SubMenu=0; jEntry<nEntries; jEntry++)
    {
      if ( (Menu_entry[jEntry][0] != iMenu_expectation &&
	    Menu_entry[jEntry][1] != iSubMenu_expectation+1) &&
	   (Menu_entry[jEntry][0] != iMenu_expectation+1 &&
	    Menu_entry[jEntry][1] != iSubMenu_expectation) )
	{
	  printf("Warning: menu entry %d in file %s is %d %d,\n",
		 jEntry,help_file_name,Menu_entry[jEntry][0],Menu_entry[jEntry][1]);
	  printf("but it should be either %d %d or %d %d,\n",
		 iMenu_expectation,iSubMenu_expectation+1,
		 iMenu_expectation+1,iSubMenu_expectation);
	  return(1);
	}
      else if ( Menu_entry[jEntry][0] == iMenu_expectation+1 )
	{
	  iMenu_expectation++;
	  iSubMenu_expectation = 0;
	}
      else if ( Menu_entry[jEntry][1] == iSubMenu_expectation+1 )
	{
	  iSubMenu_expectation++;
	}
      if ( Menu_entry[jEntry][1] > max_SubMenu )
	max_SubMenu = Menu_entry[jEntry][1];
    }
  // Define the number of main menus and the number of SubMenus per main menu
  nMainMenus = Menu_entry[nEntries-1][0]+1;
  max_SubMenu++;  // 0-offset counter
  // Make a lookup table of main menu and sub menu locations.
  nSubMenu      = int_vector(nMainMenus);
  Menu_location = int_matrix(nMainMenus,max_SubMenu);
  Menu_length   = int_matrix(nMainMenus,max_SubMenu);
  jEntry = nLines = iLength = 0;
  while ( get_line(file, string, LEN_STR) )
    {
      cptr = string;
      if ( *cptr == '!' )
	{
	  cptr++;
	  sscanf(cptr,"%d %d",&(Menu_entry[jEntry][0]),&(Menu_entry[jEntry][1]));
	  nSubMenu[Menu_entry[jEntry][0]] = Menu_entry[jEntry][1];
	  Menu_location[Menu_entry[jEntry][0]][Menu_entry[jEntry][1]] = nLines;
	  if ( jEntry != 0 )
	    Menu_length[Menu_entry[jEntry-1][0]][Menu_entry[jEntry-1][1]] = iLength-1;
	  jEntry++;
	  iLength = 0;
	}
      nLines++;
      iLength++;
    }
  nSubMenu[0] = nMainMenus;
  Menu_length[nMainMenus-1][nSubMenu[Menu_entry[jEntry-1][0]]] = iLength-1;
  free_int_matrix(Menu_entry,nEntries,2);

  strcpy(string," ");
  iMainMenu = iSubMenu = 0;
  while ( strcmp(string,"quit") && strcmp(string,"q") )
    {
      // Always start with a menu, so iSubMenu = 0
      iSubMenu = 0;
      // When there is no sub-menu for the current option, go back to the main menu.
      if ( iMainMenu != 0 && nSubMenu[iMainMenu] == 0 )
	iMainMenu = 0;
      // Display the menu.
      rewind(file);
      for (jLine=0; jLine<=Menu_location[iMainMenu][iSubMenu]; jLine++)
	get_line(file, string, LEN_STR);
      for (jLine=0; jLine<Menu_length[iMainMenu][0]; jLine++)
	{
	  get_line(file, string, LEN_STR);
	  fprintf(stdout,"%s",string);
	}
      // get a command
      fprintf(stdout,"----------------------------------------------\n");
      fprintf(stdout,"--- choose item: 0 = main menu, return = back \n");
      fprintf(stdout,"----------------------------------------------\n");
      if ( iMainMenu == 0 )
	fprintf(stdout,"help > ");
      else
	fprintf(stdout,"help >> ");
      get_line(stdin, string, LEN_STR);
      nread = sscanf(string,"%d",&jLine); // see if anything was input, versus just a <return>
      if ( nread <= 0 )
	{
	  if ( iMainMenu != 0 )
	    strcpy(string,"b");
	  else
	    strcpy(string,"q");
	}
      else
	sscanf(string,"%s",string); // kill the trailing carriage return
      if ( !strcmp(string,"quit") || !strcmp(string,"q") )
	{
	  continue;
	}
      else if ( !strcmp(string,"0") )
	iMainMenu = iSubMenu = 0;
      else if ( !strcmp(string,"b") )
	{ // Could be main menu or sub menu
	  if ( iSubMenu == 0 )
	    iMainMenu = 0;
	  else
	    iSubMenu = 0;
	}
      else
	{ // choose from a sub menu
	  sscanf(string,"%d",&iChoice);
	  if ( iMainMenu == 0 )
	    {
	      iMainMenu = iChoice;
	      iSubMenu  = 0;
	    }
	  else
	    {
	      if ( iChoice <= nSubMenu[iMainMenu] )
		iSubMenu = iChoice;
	      else
		fprintf(stdout,"Bad option. Choose '0', 'b', 'q', or a number from 1 to %d\n",iSubMenu);
	    }
	}
      if ( iSubMenu != 0 || nSubMenu[iMainMenu] == 0 )
	{
	  // Display a menu selection, but not a menu, which is handled at the top of this loop.
	  rewind(file);
	  for (jLine=0; jLine<=Menu_location[iMainMenu][iSubMenu]; jLine++)
	    get_line(file, string, LEN_STR);
	  for (jLine=0; jLine<Menu_length[iMainMenu][iSubMenu]; jLine++)
	    {
	      get_line(file, string, LEN_STR);
	      if ( iMainMenu != 0 && string[0] == '>' )
		{
		  cptr = string;
		  cptr++;
		  sscanf(cptr,"%s",help_file_name);
		  fclose(file);
		  free_int_vector(nSubMenu);
		  free_int_matrix(Menu_location,nMainMenus,max_SubMenu);
		  free_int_matrix(Menu_length,nMainMenus,max_SubMenu);
		  help_menu(1,help_file_name);
		  return(0);
		}
	      fprintf(stdout,"%s",string);
	    }
	}
    }
  fclose(file);
  free_int_vector(nSubMenu);
  free_int_matrix(Menu_location,nMainMenus,max_SubMenu);
  free_int_matrix(Menu_length,nMainMenus,max_SubMenu);

  if ( iLevel == 1 )
    help_menu(0,saved_help_file_name);

  return(0);
}

////////////////////////////////////////////////////////////////////////////
// The following 4 c-code functions:
//                          1) ibeta
//                          2) incbcf
//                          3) incbd
//                          4) pseries
// are modified versions (derivative code) of public domain software.
// This software distribution was originally based upon the
// Cephes Mathematical Library from www.netlib.org/cephes. The particular
// code underlying this derivative version was obtained from
//         www.codeproject.com/KB/cs/SpecialFunction.aspx?display=PRINT
// which uses the "The Code Project Open License" at
//                www.codeproject.com/info/cpol10.aspx
// The license states that ...
// " The main points subject to the terms of the License are:
//  * Source Code and Executable Files can be used in commercial applications;
//  * Source Code and Executable Files can be redistributed; and
//  * Source Code can be modified to create derivative works.
//  * No claim of suitability, guarantee, or any warranty whatsoever is provided.
//  The software is provided "as-is".
//  * The Article(s) accompanying the Work may not be distributed or republished without the Author's consent
//
// The header to the original c code is reproduced below. 
////////////////////////////////////////////////////////////////////////////
/*
**************************************************************************
**
**    Class  SpecialFunction (C#)
**
**************************************************************************
**    Copyright (C) 1984 Stephen L. Moshier (original C version - Cephes Math Library)
**    Copyright (C) 1996 Leigh Brookshaw	(Java version)
**    Copyright (C) 2005 Miroslav Stampar	(C# version [->this<-])
**
**    This program is free software; you can redistribute it and/or modify
**    it under the terms of the GNU General Public License as published by
**    the Free Software Foundation; either version 2 of the License, or
**    (at your option) any later version.
**
**    This program is distributed in the hope that it will be useful,
**    but WITHOUT ANY WARRANTY; without even the implied warranty of
**    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**    GNU General Public License for more details.
**
**    You should have received a copy of the GNU General Public License
**    along with this program; if not, write to the Free Software
**    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**************************************************************************
**
**    This class is an extension of System.Math. It includes a number
**    of special functions not found in the Math class.
**
*************************************************************************
** This class contains physical constants and special functions not found
** in the System.Math class.
** Like the System.Math class this class is final and cannot be
** subclassed.
** All physical constants are in cgs units.
** NOTE: These special functions do not necessarily use the fastest
** or most accurate algorithms.
**
** @version $Revision: 1.8 $, $Date: 2005/09/12 09:52:34 $
**/

#define MACHEP 1.11022302462515654042E-16
#define MAXLOG 7.09782712893383996732E2
#define MINLOG -7.451332191019412076235E2
#define MAXGAM 171.624376956302725

//prototypes
double incbcf(double a, double b, double x);
double incbd(double a, double b, double x);
double pseries(double a, double b, double x);
double tgamma(double x);


double ibeta(double aa, double bb, double xx)
{
  double a, b, t, x, xc, w, y;
  BOOL flag;

  if (aa <= 0.0 || bb <= 0.0)
    {
      printf("ibeta: Domain error!, a = %g, b = %g\n",aa,bb);
      exit(1);
    }
  if ((xx <= 0.0) || (xx >= 1.0))
    {
      if (xx == 0.0) return 0.0;
      if (xx == 1.0) return 1.0;
    }
  flag = FALSE;
  if ((bb*xx) <= 1.0 && xx <= 0.95)
    {
      t = pseries(aa, bb, xx);
      return t;
    }

  w = 1.0 - xx;

  /* Reverse a and b if x is greater than the mean. */
  if (xx > (aa/(aa + bb)))
    {
      flag = TRUE;
      a = bb;
      b = aa;
      xc = xx;
      x = w;
    }
  else
    {
      a = aa;
      b = bb;
      xc = w;
      x = xx;
    }

  if (flag && (b*x) <= 1.0 && x <= 0.95)
    {
      t = pseries(a, b, x);
      if (t <= MACHEP)
	t = 1.0 - MACHEP;
      else
	t = 1.0 - t;
      return t;
    }
  
  /* Choose expansion for better convergence. */
  y = x*(a + b - 2.0) - (a - 1.0);
  if (y < 0.0)
    w = incbcf(a, b, x);
  else
    w = incbd(a, b, x)/xc;
  
  /* Multiply w by the factor
     a      b   _             _     _
     x  (1-x)   | (a+b) / ( a | (a) | (b) ) .   */
  
  y = a*log(x);
  t = b*log(xc);
  if ((a + b) < MAXGAM && fabs(y) < MAXLOG && fabs(t) < MAXLOG)
    {
      t = pow(xc, b);
      t *= pow(x, a);
      t /= a;
      t *= w;
      t *= tgamma(a + b)/(tgamma(a)*tgamma(b));
      if (flag)
	{
	  if (t <= MACHEP) t = 1.0 - MACHEP;
	  else t = 1.0 - t;
	}
      return t;
    }
  /* Resort to logarithms.  */
  y += t + lgamma(a + b) - lgamma(a) - lgamma(b);
  y += log(w/a);
  if (y < MINLOG)
    t = 0.0;
  else
    t = exp(y);
  
  if (flag)
    {
      if (t <= MACHEP) t = 1.0 - MACHEP;
      else t = 1.0 - t;
    }
  return t;
}

/// Returns the continued fraction expansion #1 for incomplete beta integral.
double incbcf(double a, double b, double x)
{
  double xk, pk, pkm1, pkm2, qk, qkm1, qkm2;
  double k1, k2, k3, k4, k5, k6, k7, k8;
  double r, t, ans, thresh;
  int n;
  double big = 4.503599627370496e15;
  double biginv = 2.22044604925031308085e-16;

  k1 = a;
  k2 = a + b;
  k3 = a;
  k4 = a + 1.0;
  k5 = 1.0;
  k6 = b - 1.0;
  k7 = k4;
  k8 = a + 2.0;
  
  pkm2 = 0.0;
  qkm2 = 1.0;
  pkm1 = 1.0;
  qkm1 = 1.0;
  ans = 1.0;
  r = 1.0;
  n = 0;
  thresh = 3.0*MACHEP;
  do
    {
      xk = -(x*k1*k2)/(k3*k4);
      pk = pkm1 + pkm2*xk;
      qk = qkm1 + qkm2*xk;
      pkm2 = pkm1;
      pkm1 = pk;
      qkm2 = qkm1;
      qkm1 = qk;
      
      xk = (x*k5*k6)/(k7*k8);
      pk = pkm1 + pkm2*xk;
      qk = qkm1 + qkm2*xk;
      pkm2 = pkm1;
      pkm1 = pk;
      qkm2 = qkm1;
      qkm1 = qk;
      
      if (qk != 0) r = pk/qk;
      if (r != 0)
	{
	  t = fabs((ans - r)/r);
	  ans = r;
	}
      else
	t = 1.0;
      
      if (t < thresh) return ans;
      
      k1 += 1.0;
      k2 += 1.0;
      k3 += 2.0;
      k4 += 2.0;
      k5 += 1.0;
      k6 -= 1.0;
      k7 += 2.0;
      k8 += 2.0;
      
      if ((fabs(qk) + fabs(pk)) > big)
	{
	  pkm2 *= biginv;
	  pkm1 *= biginv;
	  qkm2 *= biginv;
	  qkm1 *= biginv;
	}
      if ((fabs(qk) < biginv) || (fabs(pk) < biginv))
	{
	  pkm2 *= big;
	  pkm1 *= big;
	  qkm2 *= big;
	  qkm1 *= big;
	}
    } while (++n < 300);
  
  return ans;
}

/// Returns the continued fraction expansion #2 for incomplete beta integral.
double incbd(double a, double b, double x)
{
  double xk, pk, pkm1, pkm2, qk, qkm1, qkm2;
  double k1, k2, k3, k4, k5, k6, k7, k8;
  double r, t, ans, z, thresh;
  int n;
  double big = 4.503599627370496e15;
  double biginv = 2.22044604925031308085e-16;
  
  k1 = a;
  k2 = b - 1.0;
  k3 = a;
  k4 = a + 1.0;
  k5 = 1.0;
  k6 = a + b;
  k7 = a + 1.0;
  ;
  k8 = a + 2.0;
  
  pkm2 = 0.0;
  qkm2 = 1.0;
  pkm1 = 1.0;
  qkm1 = 1.0;
  z = x/(1.0 - x);
  ans = 1.0;
  r = 1.0;
  n = 0;
  thresh = 3.0*MACHEP;
  do
    {
      xk = -(z*k1*k2)/(k3*k4);
      pk = pkm1 + pkm2*xk;
      qk = qkm1 + qkm2*xk;
      pkm2 = pkm1;
      pkm1 = pk;
      qkm2 = qkm1;
      qkm1 = qk;
      
      xk = (z*k5*k6)/(k7*k8);
      pk = pkm1 + pkm2*xk;
      qk = qkm1 + qkm2*xk;
      pkm2 = pkm1;
      pkm1 = pk;
      qkm2 = qkm1;
      qkm1 = qk;
      
      if (qk != 0) r = pk/qk;
      if (r != 0)
	{
	  t = fabs((ans - r)/r);
	  ans = r;
	}
      else
	t = 1.0;
      
      if (t < thresh) return ans;
      
      k1 += 1.0;
      k2 -= 1.0;
      k3 += 2.0;
      k4 += 2.0;
      k5 += 1.0;
      k6 += 1.0;
      k7 += 2.0;
      k8 += 2.0;
      
      if ((fabs(qk) + fabs(pk)) > big)
	{
	  pkm2 *= biginv;
	  pkm1 *= biginv;
	  qkm2 *= biginv;
	  qkm1 *= biginv;
	}
      if ((fabs(qk) < biginv) || (fabs(pk) < biginv))
	{
	  pkm2 *= big;
	  pkm1 *= big;
	  qkm2 *= big;
	  qkm1 *= big;
	}
    } while (++n < 300);
  
  return ans;
}

/// Returns the power series for incomplete beta integral. Use when b*x is small and x not too close to 1.
double pseries(double a, double b, double x)
{
  double s, t, u, v, n, t1, z, ai;

  ai = 1.0/a;
  u = (1.0 - b)*x;
  v = u/(a + 1.0);
  t1 = v;
  t = u;
  n = 2.0;
  s = 0.0;
  z = MACHEP*ai;
  while (fabs(v) > z)
    {
      u = (n - b)*x/n;
      t *= u;
      v = t/(a + n);
      s += v;
      n += 1.0;
    }
  s += t1;
  s += ai;
  
  u = a*log(x);
  if ((a + b) < MAXGAM && fabs(u) < MAXLOG)
    {
      t = tgamma(a + b)/(tgamma(a)*tgamma(b));
      s = s*t*pow(x, a);
    }
  else
    {
      t = lgamma(a + b) - lgamma(a) - lgamma(b) + u + log(s);
      if (t < MINLOG) s = 0.0;
      else s = exp(t);
    }
  return s;
}

JIP_INDEX *index_vector(JIP_INDEX length) { return jip_allocate(length,sizeof(JIP_INDEX)); }

JIP_INDEX3D *index3d_vector(JIP_INDEX length) { return jip_allocate(length,sizeof(JIP_INDEX3D)); }
