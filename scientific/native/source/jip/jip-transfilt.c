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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "fftw3.h"
#include "util.h"
#include "jip.h"

extern char error_string[LEN_STR];


int fft_volumes( int idim, int inverse )
/*
** idim:      0) transform x (readout dimension)
**            1) transform y (phase-encode dimension)
**            2) transform x & y (2dfft)
** inverse:  -1) forward transform ("raw data" to "volume"),
**            1) inverse transform ("volume" to "raw data")
*/
{
  int nt, next, last;
  struct volume_list  *volume;
  extern struct volume_list *volume_list_root;
  
  /*
  ** Make sure the dimensions are powers of 2.
  */
  for (volume=volume_list_root; volume; volume=volume->next)
    {
      if ( ((idim==0||idim==2) && !(is_power_of_2(volume->dim.x,&last,&next)))
	|| ((idim==1||idim==2) && !(is_power_of_2(volume->dim.y,&last,&next))) )
	{
	  jip_text(2,"Data dimensions must be integral powers of 2 for a FFT.");
	  return(1);
	}
    }

  for ( volume=volume_list_root, nt=0; volume; volume=volume->next, nt++ )
    fft_volume( volume, idim, inverse );

  if ( idim != 2 )
    printf("%d volumes were transformed in one dimension.\n",nt);
  else
    printf("%d volumes were transformed in two dimensions.\n",nt);

  return(0);
}

void fft_volume( struct volume_list *volume, int idim, int inverse )
// idim:      0) transform x (readout dimension)
//            1) transform y (phase-encode dimension)
//            2) transform x & y (2dfft)
// inverse:  -1) forward transform (k-space to image space),
//            1) inverse transform (image space to k-space)
{
  fftw_complex *fftwData;
  fftw_plan plan_ft1d;
  FCOMPLEX *Data1d;
  float div;
  int direction, lInner;
  int jx, jy, jz;JIP_INDEX index;
  
  printf("ft in direction %d\n",inverse);

  if ( inverse == -1 )
    direction = FFTW_FORWARD;
  else
    direction = FFTW_BACKWARD;

  if ( inverse == 1 )
    {
      /*
      ** Phase roll the volume by -PI over the field of view since we wish
      ** to transform a volume without any phase roll to raw data that is
      ** symmetric in kspace (e.g., falls on integral+1/2 k-values instead
      ** of integral k values).
      */
      if ( idim == 2 )
	{
	  apply_2d_phase_roll( volume, 0, -PI, 0. );
	  apply_2d_phase_roll( volume, 1, -PI, 0. );
	}
      else
	apply_2d_phase_roll( volume, idim, -PI, 0. );
    }

  /*
  ** Transform the data.
  */
  if ( idim==0 || idim==2 )
    {
      lInner = volume->dim.x;
      if ( inverse == 1 )
	div = 1.;
      else
	div = (float)lInner;
      // Allocate the 1d arrays
      Data1d   = fcomplex_vector(lInner); // floats
      fftwData = (fftw_complex *)fftw_malloc((size_t)jip_product(sizeof(fftw_complex),lInner));
      // Transform in the read-out dimension.
      for ( jz=0; jz<volume->dim.z; jz++ )
	{
	  for ( jy=0; jy<volume->dim.y; jy++ )
	    {
	      for ( jx=0; jx<volume->dim.x; jx++ )
		{
		  index = index3d(jx,jy,jz,volume);
		  Data1d[jx].real = volume->SpaceTime.fcData[index].real;
		  Data1d[jx].imag = volume->SpaceTime.fcData[index].imag;
		}	      
	      // On the 1st line of data, define the FT "plan".
	      if ( jy == 0 && jz == 0 )
		plan_ft1d = fftw_plan_dft_1d(lInner,fftwData,fftwData,direction,FFTW_MEASURE);

	      // swap on input
	      swap_x( Data1d, lInner, 0 );
	      // Assign the fftw directly (rather than use pointers) in case
	      // the byte size differs for the 2 arrays.
	      for (jx=0; jx<lInner; jx++)
		{
		  fftwData[jx][0] = Data1d[jx].real;
		  fftwData[jx][1] = Data1d[jx].imag;
		}
	      // Transform the data.
	      fftw_execute(plan_ft1d);
	      // Put the real data back into the complex vector.
	      for ( jx=0; jx<lInner; jx++ )
		{
		  Data1d[jx].real = fftwData[jx][0];
		  Data1d[jx].imag = fftwData[jx][1];
		}
	      // The data comes out of the fft routine in wrap-around order.
	      swap_x( Data1d, lInner, 1 );
	      // Put the transformed data back into the original matrix.
	      for (jx=0; jx<lInner; jx++)
		{
		  index = index3d(jx,jy,jz,volume);
		  volume->SpaceTime.fcData[index].real = Data1d[jx].real / div;
		  volume->SpaceTime.fcData[index].imag = Data1d[jx].imag / div;
		}
	    }
	}
      // Free data and plan allocation
      free_fcomplex_vector(Data1d);
      fftw_destroy_plan(plan_ft1d);
      fftw_free(fftwData);
    }

  if ( idim==1 || idim==2 )
    {
      lInner = volume->dim.y;
      if ( inverse == 1 )
	div = 1.;
      else
	div = (float)volume->dim.y;
      /*
      ** Put column data in the proper order.
      ** Use a temporary array for this purpose.
      */
      // Allocate the 1d arrays
      Data1d   = fcomplex_vector(lInner); // floats
      fftwData = (fftw_complex *)fftw_malloc((size_t)jip_product(sizeof(fftw_complex),lInner));
      for ( jz=0; jz<volume->dim.z; jz++ )
	{
	  for ( jx=0; jx<volume->dim.x; jx++ )
	    {
	      for ( jy=0; jy<volume->dim.y; jy++ )
		{
		  index = index3d(jx,jy,jz,volume);
		  Data1d[jx].real = volume->SpaceTime.fcData[index].real;
		  Data1d[jx].imag = volume->SpaceTime.fcData[index].imag;
		}
	      // On the 1st line of data, define the FT "plan".
	      if ( jy == 0 && jz == 0 )
		plan_ft1d = fftw_plan_dft_1d(lInner,fftwData,fftwData,direction,FFTW_MEASURE);

	      // swap on input
	      swap_x( Data1d, lInner, 0 );
	      // Assign the fftw directly (rather than use pointers) in case
	      // the byte size differs for the 2 arrays.
	      for (jy=0; jy<lInner; jy++)
		{
		  fftwData[jy][0] = Data1d[jy].real;
		  fftwData[jy][1] = Data1d[jy].imag;
		}
	      // Transform the data.
	      fftw_execute(plan_ft1d);
	      // Put the real data back into the complex vector.
	      for ( jy=0; jy<lInner; jy++ )
		{
		  Data1d[jy].real = fftwData[jy][0];
		  Data1d[jy].imag = fftwData[jy][1];
		}
	      // The data comes out of the fft routine in wrap-around order.
	      swap_x( Data1d, lInner, 1 );
	      // Put the transformed data back into the original matrix.
	      for ( jy=0; jy<volume->dim.y; jy++ )
		{
		  index     = index3d(jx,jy,jz,volume);
		  volume->SpaceTime.fcData[index].real = Data1d[jy].real / div;
		  volume->SpaceTime.fcData[index].imag = Data1d[jy].imag / div;
		}
	    }
	}
      // Free data and plan allocation
      free_fcomplex_vector(Data1d);
      fftw_destroy_plan(plan_ft1d);
      fftw_free(fftwData);
    }
  
  /*
  ** The data comes out of the fft routine in wrap-around order, so
  ** reorder it.  At the same time, divide by the number of points for
  ** inverse transforms.
  */
  if ( inverse != 1 )
    {
      /*
      ** Phase roll the volume by PI over the field of view since we wish
      ** to transform data that is symmetric in kspace to a volume without
      ** phase roll, whereas the routine we are using assumes that the
      ** k-space values fall on integral (as opposed to integral+1/2)
      ** values.
      */
      if ( idim == 2 )
	{
	  apply_2d_phase_roll( volume, 0, PI, 0. );
	  apply_2d_phase_roll( volume, 1, PI, 0. );
	}
      else
	apply_2d_phase_roll( volume, idim, PI, 0. );
    }

  return;
}

void flatten_volumes( float fov_fraction )
// Flatten time point iTime of stack InputStack and put the result in flatData.
// "InputStack" can have multiple time points.
// "flatData" should be a vector with the same dimensions but 1 time point.
{
  struct volume_list  *volume;
  extern struct volume_list *volume_list_root;
  struct data_stack temporary;
  POINT3D fwhm_mm, sigma2_k, fov2, sigma2;
  float x, y, z, scale;
  int jx, jy, jz;JIP_INDEX j, index;

  if ( fov_fraction <= 0. )
    return;

  for ( volume=volume_list_root; volume; volume=volume->next )
    {
      temporary = *(volume->stack);
      temporary.nt = 1;
      temporary.n4d = jip_product(temporary.n3d,temporary.nt);
      temporary.SpaceTime.fcData = fcomplex_vector(temporary.n4d);

      // Smooth the copy based upon the FOV
      fwhm_mm.x = temporary.fov.x / fov_fraction;
      fwhm_mm.y = temporary.fov.y / fov_fraction;
      fwhm_mm.z = temporary.fov.z / fov_fraction;
      // Determine the 3D sigma_k
      sigma2_k.x = 2.355*2.355/4./PI/PI /fwhm_mm.x/fwhm_mm.x;
      sigma2_k.y = 2.355*2.355/4./PI/PI /fwhm_mm.y/fwhm_mm.y;
      sigma2_k.z = 2.355*2.355/4./PI/PI /fwhm_mm.z/fwhm_mm.z;
      fov2.x = temporary.resolution.x*temporary.resolution.x * temporary.dim.x*temporary.dim.x;
      fov2.y = temporary.resolution.y*temporary.resolution.y * temporary.dim.y*temporary.dim.y;
      fov2.z = temporary.resolution.z*temporary.resolution.z * temporary.dim.z*temporary.dim.z;
      sigma2.x = sigma2_k.x * fov2.x;
      sigma2.y = sigma2_k.y * fov2.y;
      sigma2.z = sigma2_k.z * fov2.z;

      for (j=0; j<temporary.n3d; j++)
	{
	  temporary.SpaceTime.fcData[j].real = volume->SpaceTime.fcData[j].real;
	  temporary.SpaceTime.fcData[j].imag = volume->SpaceTime.fcData[j].imag;
	}
      // FT to k-space
      ft_1d_volume( 1,1, volume );
      ft_1d_volume( 2,1, volume );
      ft_1d_volume( 3,1, volume );

      for ( jz=0; jz<volume->dim.z; jz++ ) // these are steps in k-space
	{
	  z = jz - volume->dim.z/2 + 0.5;
	  for ( jy=0; jy<volume->dim.y; jy++ )
	    {
	      y = jy - volume->dim.y/2 + 0.5;
	      for ( jx=0; jx<volume->dim.x; jx++ )
		{
		  x = jx - volume->dim.x/2 + 0.5;
		  scale = exp(-(double)(x*x/2/sigma2.x))
		    * exp(-(double)(y*y/2/sigma2.y))
		    * exp(-(double)(z*z/2/sigma2.z));
		  index = index3d(jx,jy,jz,volume);
		  volume->SpaceTime.fcData[index].real *= scale;
		  volume->SpaceTime.fcData[index].imag *= scale;
		}
	    }
	}
      // FT back to real space
      ft_1d_volume( 1,-1, volume );
      ft_1d_volume( 2,-1, volume );
      ft_1d_volume( 3,-1, volume );

      for (j=0; j<volume->n3d; j++)
	{
	  if ( volume_amp(volume,j) != 0. )
	    temporary.SpaceTime.fcData[j].real /= volume_amp(volume,j);
	  else
	    temporary.SpaceTime.fcData[j].real = 0.;
	  volume->SpaceTime.fcData[j].real = temporary.SpaceTime.fcData[j].real;
	  volume->SpaceTime.fcData[j].imag = 0.;
	}
      free_fcomplex_vector(temporary.SpaceTime.fcData);
    }
}

int smooth_in_kspace( float fwhm, int idim )
// fwhm is in units of mm (or whatever spatial units are used)
// idim is 2 or 3
{
  struct volume_list  *volume;
  extern struct volume_list *volume_list_root;
  POINT3D resolution, fov2, sigma2;
  float sigma2_k, x, y, z, scale;
  int jx, jy, jz, jt, nt;JIP_INDEX index;

  // inverse FT in x, y, and z

  ft_1d( 1, 1 ); // x
  ft_1d( 2, 1 ); // y
  if ( idim == 3 )
    ft_1d( 3, 1 ); // z

  // Determine the 3D sigma_k
  // Multiply data by a k-space filter
  sigma2_k = 2.355*2.355/4./PI/PI /fwhm/fwhm;

  // Count the number of volumes
  for ( volume=volume_list_root, nt=0; volume; volume=volume->next )
    nt++;
  // Smooth data by multiplication in k-space
  for ( volume=volume_list_root, jt=0; volume; volume=volume->next, jt++ )
    {
      // Work on 1 volume at a time
      resolution = volume->stack->resolution;
      fov2.x = resolution.x*resolution.x * volume->dim.x*volume->dim.x;
      fov2.y = resolution.y*resolution.y * volume->dim.y*volume->dim.y;
      fov2.z = resolution.z*resolution.z * volume->dim.z*volume->dim.z;
      sigma2.x = sigma2_k * fov2.x; // use normalized fov in each dimension
      sigma2.y = sigma2_k * fov2.y;
      sigma2.z = sigma2_k * fov2.z;
      for ( jz=0; jz<volume->dim.z; jz++ ) // these are steps in k-space
	{
	  z = jz - volume->dim.z/2 + 0.5;
	  for ( jy=0; jy<volume->dim.y; jy++ )
	    {
	      y = jy - volume->dim.y/2 + 0.5;
	      for ( jx=0; jx<volume->dim.x; jx++ )
		{
		  x = jx - volume->dim.x/2 + 0.5;
		  if ( idim == 3 )
		    scale
		      = exp(-(double)(x*x/2/sigma2.x))
		      * exp(-(double)(y*y/2/sigma2.y))
		      * exp(-(double)(z*z/2/sigma2.z));
		  else if ( idim == 2 )
		    scale
		      = exp(-(double)(x*x/2/sigma2.x))
		      * exp(-(double)(y*y/2/sigma2.y));
		  index = index3d(jx,jy,jz,volume);
		  volume->SpaceTime.fcData[index].real *= scale;
		  volume->SpaceTime.fcData[index].imag *= scale;
		}
	    }
	}
      if ( jt>0 && nt>10 && (4*jt)%nt==0)
	printf("%d percent done\n",(100*jt)/nt);
    }

  // forward FT in x, y, and z
  ft_1d( 1, -1 ); // x
  ft_1d( 2, -1 ); // y
  if ( idim == 3 )
    ft_1d( 3, -1 ); // z
  return(0);
}

int sharpen_in_kspace( float fwhm, int idim )
     // fwhm is in units of mm (or whatever spatial units are used)
     // idim is 2 or 3
{
  struct volume_list  *volume;
  extern struct volume_list *volume_list_root;
  POINT3D resolution, fov2, sigma2, Scale;
  float sigma2_k, x, y, z, scale;
  int jx, jy, jz, jt, nt;JIP_INDEX index;

  // inverse FT in x, y, and z

  ft_1d( 1, 1 ); // x
  ft_1d( 2, 1 ); // y
  if ( idim == 3 )
    ft_1d( 3, 1 ); // z

  // Determine the 3D sigma_k
  // Multiply data by a k-space filter
  sigma2_k = 2.355*2.355/4./PI/PI /fwhm/fwhm;
  printf("fwhm = %g, sigma2_k = %g\n",fwhm,sigma2_k);

  // Count the number of volumes
  for ( volume=volume_list_root, nt=0; volume; volume=volume->next )
    nt++;
  // Smooth data by multiplication in k-space
  for ( volume=volume_list_root, jt=0; volume; volume=volume->next, jt++ )
    {
      // Work on 1 volume at a time
      resolution = volume->stack->resolution;
      fov2.x = resolution.x*resolution.x * volume->dim.x*volume->dim.x;
      fov2.y = resolution.y*resolution.y * volume->dim.y*volume->dim.y;
      fov2.z = resolution.z*resolution.z * volume->dim.z*volume->dim.z;
      sigma2.x = sigma2_k * fov2.x; // use normalized fov in each dimension
      sigma2.y = sigma2_k * fov2.y;
      sigma2.z = sigma2_k * fov2.z;
      printf("sigma2 = %g %g %g\n",sigma2.x,sigma2.y,sigma2.z);
      for ( jz=0; jz<volume->dim.z; jz++ ) // these are steps in k-space
	{
	  z = jz - volume->dim.z/2 + 0.5;
	  for ( jy=0; jy<volume->dim.y; jy++ )
	    {
	      y = jy - volume->dim.y/2 + 0.5;
	      for ( jx=0; jx<volume->dim.x; jx++ )
		{
		  x = jx - volume->dim.x/2 + 0.5;
		  Scale.x = (1.-exp(-(double)(x*x/2/sigma2.x)));
		  Scale.y = (1.-exp(-(double)(y*y/2/sigma2.y)));
		  Scale.z = (1.-exp(-(double)(z*z/2/sigma2.z)));
		  if ( idim == 3 )
		    scale = Scale.x * Scale.y * Scale.z;
		  else if ( idim == 2 )
		    scale = Scale.x * Scale.y;
		  index = index3d(jx,jy,jz,volume);
		  
		  printf("(x,y,z) = (%g,%g,%g), scale = %g (%g,%g,%g)\n",
			 x,y,z,scale,Scale.x,Scale.y,Scale.z);

		  volume->SpaceTime.fcData[index].real *= scale;
		  volume->SpaceTime.fcData[index].imag *= scale;
		}
	    }
	}
      if ( jt>0 && nt>10 && (4*jt)%nt==0)
	printf("%d percent done\n",(100*jt)/nt);
    }

  // forward FT in x, y, and z
  ft_1d( 1, -1 ); // x
  ft_1d( 2, -1 ); // y
  if ( idim == 3 )
    ft_1d( 3, -1 ); // z
  return(0);
}

int ft_1d( int idim, int inverse )
{
  // idim: 1 -> x,
  //       2 -> y,
  //       3 -> z
  // inverse: -1 -> forward  (k-space to image space)
  //          +1 -> backward (image space to k-space)
  struct data_stack *stack, *stack_save;
  struct volume_list  *volume;
  extern struct volume_list *volume_list_root;
  fftw_complex *fftwData;
  fftw_plan plan_ft1d;
  FCOMPLEX *Data1d;
  float div;
  int lInner, lOuter1, lOuter2, direction;
  int jt, j1, j2, jIn;JIP_INDEX index;

  if ( inverse == -1 )
    direction = FFTW_FORWARD;
  else
    direction = FFTW_BACKWARD;
  
  stack_save = 0;
  for ( volume=volume_list_root; volume; volume=volume->next )
    {
      // Work on 1 stack at a time
      if ( (stack=volume->stack) == stack_save ) continue;
      stack_save = stack;
      // Set the dimensions based upon the FT direction.
      if ( idim == 1 )
	{
	  lInner  = stack->dim.x;
	  lOuter1 = stack->dim.y;
	  lOuter2 = stack->dim.z;
	}
      else if ( idim == 2 )
	{
	  lOuter1 = stack->dim.x;
	  lInner  = stack->dim.y;
	  lOuter2 = stack->dim.z;
	}
      else if ( idim == 3 )
	{
	  lOuter1 = stack->dim.x;
	  lOuter2 = stack->dim.y;
	  lInner  = stack->dim.z;
	}
      if ( inverse == -1 )
	div = (float)lInner;
      else
	div = 1.;
      // Allocate the 1d arrays
      Data1d   = fcomplex_vector(lInner); // floats
      fftwData = (fftw_complex *)fftw_malloc((size_t)jip_product(sizeof(fftw_complex),lInner)); // doubles
      // Loop over time + the 3 spatial dimensions
      for (jt=0; jt<stack->nt; jt++)
	{
	  for (j1=0; j1<lOuter1; j1++)
	    {
	      for (j2=0; j2<lOuter2; j2++)
		{
		  for (jIn=0; jIn<lInner; jIn++)
		    {
		      if ( idim == 1 )
			index = index4d(jIn,j1,j2,jt,stack);
		      else if ( idim == 2 )
			index = index4d(j1,jIn,j2,jt,stack);
		      else if ( idim == 3 )
			index = index4d(j1,j2,jIn,jt,stack);
		      Data1d[jIn].real = stack->SpaceTime.fcData[index].real;
		      Data1d[jIn].imag = stack->SpaceTime.fcData[index].imag;
		    }
		  // On the 1st line of data, define the FT "plan".
		  if ( jt == 0 && j1 == 0 && j2 == 0 )
		    plan_ft1d = fftw_plan_dft_1d(lInner,fftwData,fftwData,direction,FFTW_MEASURE);

		  // swap on input
		  swap_x( Data1d, lInner, 0 );
		  // Assign the fftw directly (rather than use pointers) in case
		  // the byte size differs for the 2 arrays.
		  for (jIn=0; jIn<lInner; jIn++)
		    {
		      fftwData[jIn][0] = Data1d[jIn].real;
		      fftwData[jIn][1] = Data1d[jIn].imag;
		    }
		  // Transform the data.
		  fftw_execute(plan_ft1d);
		  // Use the original array.
		  for ( jIn=0; jIn<lInner; jIn++ )
		    {
		      Data1d[jIn].real = fftwData[jIn][0];
		      Data1d[jIn].imag = fftwData[jIn][1];
		    }
		  // The data comes out of the fft routine in wrap-around order.
		  swap_x( Data1d, lInner, 1 );
		  // Put the transformed data back into the original matrix.
		  for (jIn=0; jIn<lInner; jIn++)
		    {
		      if ( idim == 1 )
			index = index4d(jIn,j1,j2,jt,stack);
		      else if ( idim == 2 )
			index = index4d(j1,jIn,j2,jt,stack);
		      else if ( idim == 3 )
			index = index4d(j1,j2,jIn,jt,stack);
		      stack->SpaceTime.fcData[index].real = Data1d[jIn].real/div;
		      stack->SpaceTime.fcData[index].imag = Data1d[jIn].imag/div;
		    }
		}
	    }
	}
      // Free data and plan allocation
      free_fcomplex_vector(Data1d);
      fftw_destroy_plan(plan_ft1d);
      fftw_free(fftwData);
    }

  return(0);
}

int ft_1d_volume( int idim, int inverse, struct volume_list *volume )
{
  // idim: 1 -> x,
  //       2 -> y,
  //       3 -> z
  // inverse: -1 -> forward  (k-space to image space)
  //          +1 -> backward (image space to k-space)
  struct data_stack *stack;
  fftw_complex *fftwData;
  fftw_plan plan_ft1d;
  FCOMPLEX *Data1d;
  float div;
  int lInner, lOuter1, lOuter2, direction;
  int j1, j2, jIn;JIP_INDEX index;

  if ( inverse == -1 )
    direction = FFTW_FORWARD;
  else
    direction = FFTW_BACKWARD;
  
  stack=volume->stack;
  // Set the dimensions based upon the FT direction.
  if ( idim == 1 )
    {
      lInner  = stack->dim.x;
      lOuter1 = stack->dim.y;
      lOuter2 = stack->dim.z;
    }
  else if ( idim == 2 )
    {
      lOuter1 = stack->dim.x;
      lInner  = stack->dim.y;
      lOuter2 = stack->dim.z;
    }
  else if ( idim == 3 )
    {
      lOuter1 = stack->dim.x;
      lOuter2 = stack->dim.y;
      lInner  = stack->dim.z;
    }
  if ( inverse == -1 )
    div = (float)lInner;
  else
    div = 1.;
  // Allocate the 1d arrays
  Data1d   = fcomplex_vector(lInner); // floats
  fftwData = (fftw_complex *)fftw_malloc((size_t)jip_product(sizeof(fftw_complex),lInner)); // doubles
  // Loop over 3 spatial dimensions
  for (j1=0; j1<lOuter1; j1++)
    {
      for (j2=0; j2<lOuter2; j2++)
	{
	  for (jIn=0; jIn<lInner; jIn++)
	    {
	      if ( idim == 1 )
		index = index3d(jIn,j1,j2,volume);
	      else if ( idim == 2 )
		index = index3d(j1,jIn,j2,volume);
	      else if ( idim == 3 )
		index = index3d(j1,j2,jIn,volume);
	      Data1d[jIn].real = volume->SpaceTime.fcData[index].real;
	      Data1d[jIn].imag = volume->SpaceTime.fcData[index].imag;
	    }
	  // On the 1st line of data, define the FT "plan".
	  if ( j1 == 0 && j2 == 0 )
	    plan_ft1d = fftw_plan_dft_1d(lInner,fftwData,fftwData,direction,FFTW_MEASURE);
	  // swap on input
	  swap_x( Data1d, lInner, 0 );
	  // Assign the fftw directly (rather than use pointers) in case
	  // the byte size differs for the 2 arrays.
	  for (jIn=0; jIn<lInner; jIn++)
	    {
	      fftwData[jIn][0] = Data1d[jIn].real;
	      fftwData[jIn][1] = Data1d[jIn].imag;
	    }
	  // Transform the data.
	  fftw_execute(plan_ft1d);
	  // Use the original array.
	  for ( jIn=0; jIn<lInner; jIn++ )
	    {
	      Data1d[jIn].real = fftwData[jIn][0];
	      Data1d[jIn].imag = fftwData[jIn][1];
	    }
	  // The data comes out of the fft routine in wrap-around order.
	  swap_x( Data1d, lInner, 1 );
	  // Put the transformed data back into the original matrix.
	  for (jIn=0; jIn<lInner; jIn++)
	    {
	      if ( idim == 1 )
		index = index3d(jIn,j1,j2,volume);
	      else if ( idim == 2 )
		index = index3d(j1,jIn,j2,volume);
	      else if ( idim == 3 )
		index = index3d(j1,j2,jIn,volume);
	      stack->SpaceTime.fcData[index].real = Data1d[jIn].real/div;
	      stack->SpaceTime.fcData[index].imag = Data1d[jIn].imag/div;
	    }
	}
    }
  // Free data and plan allocation
  free_fcomplex_vector(Data1d);
  fftw_destroy_plan(plan_ft1d);
  fftw_free(fftwData);

  return(0);
}

int ft_time()
{
  struct data_stack *stack, *stack_save;
  struct volume_list  *volume;
  extern struct volume_list *volume_list_root;
  fftw_complex *fftwData;
  fftw_plan plan_ft1d;
  FCOMPLEX *FCData1d;
  IPOINT3D dim;
  int nt;
  int jt;JIP_INDEX jv, index;
  
  // Make sure all volumes have the same dimensions.
  if ( inconsistent_3d_dimensions( &dim,&nt) ) return(1);

  for ( volume=volume_list_root; volume; volume=volume->next )
    {
      if ( (stack=volume->stack) == stack_save ) continue;
      stack_save = stack;
      // Allocate a temporary array for tranposing the data.
      FCData1d = fcomplex_vector(stack->nt);
      for (jv=0; jv<stack->n3d; jv++)
	{
	  for (jt=0; jt<stack->nt; jt++)
	    {
	      index = jt * stack->n3d + jv;
	      FCData1d[jt].real = stack->SpaceTime.fcData[index].real;
	      FCData1d[jt].imag = stack->SpaceTime.fcData[index].imag;
	    }
	  // 1D Transform
	  // Reorder the data on input to "wrap-around" order.
	  swap_x( FCData1d, stack->nt, 0 );
	  // On the 1st pass, allocate the fftw object, and define the FT "plan".
	  if ( jv == 0 )
	    {
	      fftwData = (fftw_complex *)fftw_malloc((size_t)jip_product(sizeof(fftw_complex),stack->nt));
	      for (jt=0; jt<stack->nt; jt++)
		{
		  fftwData[jt][0] = FCData1d[jt].real;
		  fftwData[jt][1] = FCData1d[jt].imag;
		}
	      plan_ft1d = fftw_plan_dft_1d(stack->nt,fftwData,fftwData,FFTW_FORWARD,FFTW_MEASURE);
	    }
	  // Use the fftw array.
	  for ( jt=0; jt<stack->nt; jt++ )
	    {
	      fftwData[jt][0] = FCData1d[jt].real;
	      fftwData[jt][1] = FCData1d[jt].imag;
	    }
	  // Transform the data.
	  fftw_execute(plan_ft1d);
	  // Use the original array.
	  for ( jt=0; jt<stack->nt; jt++ )
	    {
	      FCData1d[jt].real = fftwData[jt][0];
	      FCData1d[jt].imag = fftwData[jt][1];
	    }
	  // The data comes out of the fft routine in wrap-around order.
	  swap_x( FCData1d, stack->nt, 1 );
	  // Put the transformed data back into the original matrix.
	  for ( jt=0; jt<stack->nt; jt++ )
	    {
	      index = jt * stack->n3d + jv;
	      stack->SpaceTime.fcData[index] = FCData1d[jt];
	    }
	}
      free_fcomplex_vector(FCData1d);
      fftw_destroy_plan(plan_ft1d);
      fftw_free(fftwData);
    }

  return(0);
}

void apply_2d_phase_roll( struct volume_list *volume, int dir, float slope, float offset )
     //dir 0 = x,  1 = y
{
  int nlines, jline, jz;JIP_INDEX jpoint;JIP_INDEX npoints;JIP_INDEX index;
  float mag, phase, zero, xory;

  if ( dir == 0 )
    {
      npoints = volume->dim.x;   /* # points per line in direction "dir" */
      nlines  = volume->dim.y;   /* # lines in direction "dir" */
    }
  else
    {
      nlines  = volume->dim.x;   /* # lines in direction "dir" */
      npoints = volume->dim.y;   /* # points per line in direction "dir" */
    }
  
  /*
  ** Correct the phase roll on the volume.
  */
  zero = ( npoints - 1 ) / 2.;
  for (jz=0; jz<volume->dim.z; jz++)
    {
      for (jline=0; jline<nlines; jline++)
	{
	  for ( jpoint=0; jpoint<npoints; jpoint++ )
	    {
	      if ( dir == 0 )
		index = jz * volume->n2d + jline * npoints + jpoint;
	      else
		index = jz * volume->n2d + jpoint * nlines + jline;
	      mag   = volume_amp(volume,index);
	      phase = volume_phase(volume,index);
	      xory  = (jpoint - zero)/(float)npoints;
	      phase -= slope * xory + offset;
	      volume->SpaceTime.fcData[index].real = mag * cos( (double)phase );
	      volume->SpaceTime.fcData[index].imag = mag * sin( (double)phase );
	    }
	}
    }
}


void hamming_window( int mode )
     // mode: 0 = (x,y)  ,  1 = (x,y,z)
{
  POINT3D dist, hamming;
  float filter;
  int jx, jy, jz, jt;JIP_INDEX index;
  struct data_stack *stack, *stack_save;
  struct volume_list  *volume;
  extern struct volume_list *volume_list_root;

  /* Cycle through the volume list, beginning with the root. */
  for ( volume=volume_list_root; volume; volume=volume->next )
    {
      if ( (stack=volume->stack) == stack_save ) continue;
      stack_save = stack;
      /*
      ** Apply the filter to the data.
      */
      for (jt=0; jt<stack->nt; jt++)
	{
	  for ( jz=0; jz<stack->dim.z; jz++)
	    {
	      dist.z = (float)jz / (float)(stack->dim.z-1);  // 0 to 1
	      for (jy=0; jy<stack->dim.y; jy++)
		{
		  dist.y = (float)jy / (float)(stack->dim.y-1);  // 0 to 1
		  for (jx=0; jx<stack->dim.x; jx++)
		    {
		      dist.x = (float)jx / (float)(stack->dim.x-1);  // 0 to 1
		      hamming.x = 0.54 - 0.46 * cos( (double)(2.*PI*dist.x) );
		      if ( mode == 0 )
			hamming.z = 1.;
		      else
			hamming.z = 0.54 - 0.46 * cos( (double)(2.*PI*dist.z) );
		      filter = hamming.x * hamming.y * hamming.z;
		      index = index4d(jx,jy,jz,jt,stack);
		      stack->SpaceTime.fcData[index].real *= filter;
		      stack->SpaceTime.fcData[index].imag *= filter;
		    }
		}
	    }
	}
    }
}

void exponential_filter_1d( float tau )
     // multiply raw data by exp(-k/tau)
     // tau is in units of points
{
  float filter;
  int jx, jy, jz;JIP_INDEX index;
  struct volume_list  *volume;
  extern struct volume_list *volume_list_root;

  /* Cycle through the volume list, beginning with the root. */
  for ( volume=volume_list_root; volume; volume=volume->next )
    {
      for ( jz=0; jz<volume->dim.z; jz++)
	{
	  for (jy=0; jy<volume->dim.y; jy++)
	    {
	      for (jx=0; jx<volume->dim.x; jx++)
		{
		  //		  filter = exp(-(double)jx/(double)tau);
		  filter = exp((double)jx/(double)tau) * exp(-(double)(jx*jx)/(double)(tau*tau));
		  index = index3d(jx,jy,jz,volume);
		  volume->SpaceTime.fcData[index].real *= filter;
		  volume->SpaceTime.fcData[index].imag *= filter;
		}
	    }
	}
    }
}

void edge_filter()
{
  int jx, jy, jz, jfx, jfy;JIP_INDEX jv;
  int ix, iy;JIP_INDEX index, index_sum;
  float sum_x, sum_y, sum, amp, amp_sum;
  FCOMPLEX *FCData;
  struct volume_list  *volume;
  extern struct volume_list *volume_list_root;
  float filter[3][3];

  filter[0][0] =  1;
  filter[0][1] =  2;
  filter[0][2] =  1;
  filter[1][0] =  0;
  filter[1][1] =  0;
  filter[1][2] =  0;
  filter[2][0] = -1;
  filter[2][1] = -2;
  filter[2][2] = -1;

  /* Cycle through the volume list, beginning with the root. */
  for ( volume=volume_list_root; volume; volume=volume->next )
    {
      // Allocate a temporary array for tranposing the data.
      FCData = fcomplex_vector(volume->n3d);
      for ( jz=0; jz<volume->dim.z; jz++)
	{
	  for (jx=0; jx<volume->dim.x; jx++)
	    {
	      for (jy=0; jy<volume->dim.y; jy++)
		{
		  sum_x = sum_y = amp_sum = 0.;
		  for (jfx=0; jfx<3; jfx++)   // jfx = filter index in x
		    {
		      ix = jx + jfx - 1;      // add -1,0,1 to jx
		      if ( ix < 0 ) ix = volume->dim.x-1;
		      if ( ix == volume->dim.x-1 ) ix = 0;
		      for (jfy=0; jfy<3; jfy++)
			{
			  iy = jy + jfy - 1;  // add -1,0,1 to jy
			  if ( iy < 0 ) iy = volume->dim.y-1;
			  if ( iy == volume->dim.y-1 ) iy = 0;
			  index_sum = index3d(ix,iy,jz,volume);
			  amp      = volume_amp(volume,index_sum);
			  sum_x   += amp * filter[jfx][jfy];
			  sum_y   += amp * filter[jfy][jfx];
			  amp_sum += amp;
			}
		    }
		  index = index3d(jx,jy,jz,volume);
		  sum = sqrt((double)(sum_x*sum_x + sum_y*sum_y));
		  if ( amp_sum == 0. )
		    FCData[index].real = 0.;
		  else
		    //		    FCData[index].real = sum/amp_sum;
		    FCData[index].real = sum/amp_sum;
		  FCData[index].imag = 0;
		} // end jy
	    } // end jx
	} // end jz
      for (jv=0; jv<volume->n3d; jv++)
	volume->SpaceTime.fcData[jv] = FCData[jv];
      free_fcomplex_vector(FCData);
    } // end volumes
}

int analyze_lectin()
{
  char file_name[LEN_STR];
  FILE *file;
  int imax, iamp, *histogram;JIP_INDEX i2d;JIP_INDEX ithresh, imax_intensity, ibin_max, error;
  int jt, jx_rel, jy_rel, jx_center, jy_center, ix, iy, nblock, increment_report;JIP_INDEX j;
  short *noise_floor, *noise_volume, *signal_volume, *threshold;
  float LinearAverage_micron=5., AreaThreshold_micron2=2.;
  struct volume_list *volume;
  extern struct volume_list *volume_list_root;

  /* Cycle through the volume list, beginning with the root. */
  for ( volume=volume_list_root, jt=0; volume; volume=volume->next, jt++ )
    {
      // Make sure that the "volume" is 2d.
      if (volume->dim.z != 1 )
	{
	  printf("Error: volumes should be 2-dimensional for this function.\n");
	  return(1);
	}
      increment_report = volume->dim.x / 100;
      nblock = LinearAverage_micron / volume->stack->resolution.x;
      printf("Determine local noise by averaging across %g micron squares (%d pixels)\n",
	     LinearAverage_micron,nblock);

      noise_floor   = short_vector(volume->n2d);  // determine from histogram
      threshold     = short_vector(volume->n2d);  // determine from histogram
      noise_volume  = short_vector(volume->n2d);  // original data - signal
      signal_volume = short_vector(volume->n2d);  // signal (based upon threshold after subtraction_
      for (j=0, imax_intensity=-1; j<volume->n2d; j++)
	if ( volume_amp(volume,j) > imax_intensity ) imax_intensity = volume_amp(volume,j);
      histogram = int_vector(imax_intensity+1);
      for (jx_center=0; jx_center<volume->dim.x; jx_center++)
	{
	  printf("\rDetermining local background: %d percent done",jx_center/increment_report);
	  fflush(stdout);
	  for (jy_center=0; jy_center<volume->dim.y; jy_center++)
	    {
	      // find the background
	      // Zero the histogram for each new block.
	      for (j=0; j<=imax_intensity; j++)
		histogram[j] = 0;
	      for (jx_rel=-nblock/2; jx_rel<=nblock/2; jx_rel++)
		{
		  ix = MAX(0,MIN(jx_center+jx_rel,volume->dim.x-1));
		  for (jy_rel=-nblock/2; jy_rel<=nblock/2; jy_rel++)
		    {
		      iy = MAX(0,MIN(jy_center+jy_rel,volume->dim.y-1));
		      i2d = ((JIP_INDEX)(iy)) * volume->dim.x + ix;
		      iamp = volume_amp(volume,i2d);
		      histogram[iamp] += 1;
		    } // end y block
		} // end x block
	      // Find the max histogram value = noise level
	      for (j=0, imax=ibin_max=-1; j<=imax_intensity; j++)
		{
		  if ( histogram[j] > imax )
		    {
		      imax = histogram[j];
		      ibin_max = j;
		    }
		}
	      // Find the low-side half-max = threshold
	      for (j=ibin_max; j>=0; j--)
		{
		  if ( histogram[j] < imax/8 )
		    {
		      ithresh = ibin_max - j + 1;
		      break;
		    }
		}
	      i2d = ((JIP_INDEX)(jy_center)) * volume->dim.x + jx_center;
	      noise_floor[i2d] = ibin_max;
	      threshold[i2d] = ithresh;
	    } // end y step
	} // end x step
      printf("\n");

      // Remove the noise.
      for (j=0; j<volume->n2d; j++)
	signal_volume[j] = volume_amp(volume,j) - noise_floor[j];
      // Make it binary
      for (j=0; j<volume->n2d; j++)
	{
	  if ( signal_volume[j] > threshold[j] ) // this is signal after subtraction of noise floor
	    {
	      volume->SpaceTime.fcData[j].real = 1.;
	      noise_volume[j]  = noise_floor[j];
	    }
	  else
	    { // This is all noise.
	      noise_volume[j]  = signal_volume[j] + noise_floor[j];
	      //	      signal_volume[j] = 0;
	      volume->SpaceTime.fcData[j].real = 0.;
	    }
	  volume->SpaceTime.fcData[j].imag = 0.;
	}
      //
      filter_small_islands(volume, AreaThreshold_micron2);

      // output the noise volume
      sprintf(file_name,"analyze-%d.hdr",jt);
      if ( !(file = fopen(file_name,"w")) )
	{
	  printf("Error: cannot open file %s.\n",file_name);
	  return(1);
	}
      fprintf(file,"x %d\n",volume->dim.x);
      fprintf(file,"y %d\n",volume->dim.y);
      fprintf(file,"z %d\n",volume->dim.z);
      fprintf(file,"t 4\n");
      fprintf(file,"resolution %g %g %g\n",
	      volume->stack->resolution.x,
	      volume->stack->resolution.y,
	      volume->stack->resolution.z);
      fclose(file);
      sprintf(file_name,"analyze-%d.bshort",jt);
      if ( !(file = fopen(file_name,"w")) )
	{
	  printf("Error: cannot open file %s.\n",file_name);
	  return(1);
	}
      error  = fwrite((char *)signal_volume,sizeof(short),volume->n2d,file) != volume->n2d;
      error &= fwrite((char *)noise_floor,sizeof(short),volume->n2d,file)   != volume->n2d;
      error &= fwrite((char *)threshold,sizeof(short),volume->n2d,file)  != volume->n2d;
      error &= fwrite((char *)noise_volume,sizeof(short),volume->n2d,file)  != volume->n2d;
      if ( error )
	{
	  printf("Error writing the output file %s.\n",file_name);
	  return(1);
	}
      free_short_vector(noise_floor);
      free_short_vector(noise_volume);
      free_short_vector(signal_volume);
      free_short_vector(threshold);
    } // loop over volumes
  return(0);
}

void filter_small_islands( struct volume_list *volume, float AreaThresh)
{
  short *mask_tested, *mask_current;
  IPOINT2D voxel, neighbor, jRel;
  float Area;
  int expand, ignore, increment_report;JIP_INDEX nvoxels, nvoxels_save;
  JIP_INDEX i2d_voxel, i2d_neighbor;JIP_INDEX j;

  if ( volume->dim.z != 1 )
    {
      printf("Programming error: volumes should have only 1 slice in filter_small_islands().\n");
      exit(1);
    }

  mask_tested  = short_vector(volume->n2d);
  mask_current = short_vector(volume->n2d);

  // The current volume should be binary at this stage.
  // Initialize the "tested" mask by identifying zeroes.
  for (j=0; j<volume->n2d; j++)
    {
      mask_tested[j] = mask_current[j] = 0;
      if ( volume_amp(volume,j) == 0 )
	mask_tested[j] = 1;   // 0 values do not have to be tested
      else
      mask_tested[j] = 0;
    }

  printf("Remove small islands below %g microns^2\n",AreaThresh);
  increment_report = volume->dim.y / 100;
  for ( voxel.y=0; voxel.y<volume->dim.y; voxel.y++)
    {
      printf("\rRemoving small islands: %d percent done",voxel.y/increment_report);
      fflush(stdout);
      for ( voxel.x=0; voxel.x<volume->dim.x; voxel.x++)
	{
	  i2d_voxel = ((JIP_INDEX)(voxel.y)) * volume->dim.x + voxel.x;
	  // For every voxel, test connectivity with other voxels and evaluate size.
	  if ( ! mask_tested[i2d_voxel] )
	    { // Only test the voxel if it hasn't already been identified previously.
	      // Initialize a mask specifically for testing the voxel.
	      for (j=0; j<volume->n2d; j++)
		mask_current[j] = 0;
	      mask_current[i2d_voxel] = 1;  // set current voxel
	      // Now go through the neighborhood testing connectivity.
	      nvoxels = 1;
	      nvoxels_save = expand = 0;
	      while ( nvoxels > nvoxels_save )
		{ // find neighbor voxels that touch current voxel
		  nvoxels_save = nvoxels;
		  // Increment the expansion range.
		  expand++;
		  for (jRel.y=-expand; jRel.y<=expand; jRel.y++)
		    {
		      neighbor.y = voxel.y + jRel.y;
		      for (jRel.x=-expand; jRel.x<=expand; jRel.x++)
			{
			  neighbor.x = voxel.x + jRel.x;
			  // Ignore indices that are out of bounds.
			  ignore = (neighbor.x < 0 || neighbor.x >= volume->dim.x )
			    ||     (neighbor.y < 0 || neighbor.y >= volume->dim.y );
			  // Ignore center voxel.
			  //ignore |= (jRel.x==0 && jRel.y==0);
			  if ( !ignore )
			    {
			      i2d_neighbor = ((JIP_INDEX)(neighbor.y)) * volume->dim.x + neighbor.x;
			      // Ignore voxels that have already been identified.
			      if ( !mask_current[i2d_neighbor] && !mask_tested[i2d_neighbor] )
				{
				  if (voxel_touches_2d_neighbor(volume, neighbor, mask_current))
				    {
				      mask_current[i2d_neighbor] = 1;
				      nvoxels++;
				    }
				}
			    }
			} // end x neighbor
		    } // end y neighbor
		} // end find voxels
	      Area = nvoxels * volume->stack->resolution.x * volume->stack->resolution.y;
	      // Threshold according to area. Also, "found" pixels should be included in the "tested" set.
	      for (j=0; j<volume->n2d; j++)
		{
		  if ( mask_current[j] == 1 )  // This pixel belongs in the current contiguous set.
		    {
		      if ( Area < AreaThresh ) // The area of the signal island is too small.  Remove it.
			{
			  volume->SpaceTime.fcData[j].real = volume->SpaceTime.fcData[j].imag = 0.;
			}
		      mask_tested[j] = 1;
		    }
		}
	    } // if !mask_tested
	} // end x voxel
    } // end y voxel
  printf("\n");

  free_short_vector(mask_tested);
  free_short_vector(mask_current);

  return;
}

BOOL voxel_touches_2d_neighbor(struct volume_list  *volume, IPOINT2D voxel, short *mask_voxels)
{
  IPOINT2D neighbor, jRel;
  BOOL touch, ignore;
  JIP_INDEX i2d;

  touch = FALSE;
  for (jRel.y=-1; jRel.y<=1; jRel.y++)
    {
      neighbor.y = voxel.y + jRel.y;
      for (jRel.x=-1; jRel.x<=1; jRel.x++)
	{
	  neighbor.x = voxel.x + jRel.x;
	  // Ignore indices that are out of bounds.
	  ignore = (neighbor.x < 0 || neighbor.x >= volume->dim.x )
	    ||     (neighbor.y < 0 || neighbor.y >= volume->dim.y );
	  // Ignore center voxel (the one being tested)
	  ignore |= (jRel.x==0 && jRel.y==0 );
	  // Require a shared border, not just a point connection.
	  //ignore |= (jRel.x * jRel.y != 0);
	  if ( !ignore )
	    {
	      // Does the current voxel touch any non-zero neighbor?
	      i2d = ((JIP_INDEX)(neighbor.y)) * volume->dim.x + neighbor.x;
	      touch |= mask_voxels[i2d] != 0;
	    }
	}
    }
  return(touch);
}
