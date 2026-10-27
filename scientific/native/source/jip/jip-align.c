#include "jip-warp-safety.h"
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
#include "util.h"
#include "jip.h"
#include "../jip-align/common-align.h"


int ishift_pixels( int ix_shift, int iy_shift, int iz_shift )
{
  struct data_stack *stack, *stack_save;
  FCOMPLEX *shift_data;
  int ix, iy, iz, jx, jy, jz;JIP_INDEX index, index_shift, jv;
  struct volume_list  *volume;
  extern struct volume_list *volume_list_root;

  //////////// Shift the voxels //////////////////////
  for ( volume=volume_list_root; volume; volume=volume->next )
    {
      /* Allocate space for the shifted data. */
      shift_data = fcomplex_vector(volume->n3d);
      /*
      ** Shift the volume.
      */
      for (jz=0; jz<volume->dim.z; jz++)
	{
	  iz = jz - iz_shift;
	  if ( iz < 0 )
	    iz += volume->dim.z;
	  else if ( iz >= volume->dim.z )
	    iz -= volume->dim.z;
	  for (jy=0; jy<volume->dim.y; jy++)
	    {
	      iy = jy - iy_shift;
	      if ( iy < 0 )
		iy += volume->dim.y;
	      else if ( iy >= volume->dim.y )
		iy -= volume->dim.y;

	      for (jx=0; jx<volume->dim.x; jx++)
		{
		  ix = jx - ix_shift;
		  if ( ix < 0 )
		    ix += volume->dim.x;
		  else if ( ix >= volume->dim.x )
		    ix -= volume->dim.x;
		  
		  index       = index3d(jx,jy,jz,volume);
		  index_shift = index3d(ix,iy,iz,volume);
		  shift_data[index] = volume->SpaceTime.fcData[index_shift];
		}
	    }
	}
      /* Copy the shifted data back into the volume data array. */
      for (jv=0; jv<volume->n3d; jv++)
	volume->SpaceTime.fcData[jv] = shift_data[jv];
      /* Deallocate space for the shifted data. */
      free_fcomplex_vector(shift_data);
    }

  //////////// Shift the coordinates //////////////////////
  stack_save = 0;
  for ( volume=volume_list_root; volume; volume=volume->next )
    {
      if ( (stack=volume->stack) == stack_save ) continue;
      stack_save = stack;
      stack->origin.x -= ix_shift*stack->resolution.x*stack->direction.x;
      stack->origin.y -= iy_shift*stack->resolution.y*stack->direction.y;
      stack->origin.z -= iz_shift*stack->resolution.z*stack->direction.z;
      stack->ijk_to_xyz = scale_and_shift_to_mat44(stack->resolution,stack->origin,stack->direction);
      stack->xyz_to_ijk = invert_mat44( stack->ijk_to_xyz );
    }

  return(0);
}

void downsample_stacks()
{
  struct data_stack *stack, *stack_save;
  struct volume_list  *volume;
  extern struct volume_list *volume_list_root;

  // If critical alignment values have not been set explicitly,
  // take values from the first stack in the source list.
  if ( ((JIP_INDEX)(Alignment.dimensionT.x))*Alignment.dimensionT.y*Alignment.dimensionT.z == 0 )             // matrix
    {
      Alignment.dimensionT = volume_list_root->stack->dim;
      printf("Using matrix %d %d %d\n",Alignment.dimensionT.x,Alignment.dimensionT.y,Alignment.dimensionT.z);
    }

  /* Cycle through the volume list, beginning with the root. */
  stack_save = 0;
  for ( volume=volume_list_root; volume; volume=volume->next )
    {
      if ( (stack=volume->stack) == stack_save ) continue;
      stack_save = stack;
      downsample_stack( stack, Alignment.dimensionT );
      printf("\nData stack %s was downsample into size (%d,%d,%d) & resolution (%g,%g,%g)\n\n",
	     stack->name,stack->dim.x,stack->dim.y,stack->dim.z,
	     stack->resolution.x,stack->resolution.y,stack->resolution.z);
    }  
}

/* Validate registration metadata before replacing any input image buffers. */
static void validate_registration_map(void)
{
  int r, c;JIP_INDEX j;double expected,actual,tolerance;
  for(r=0;r<4;r++)for(c=0;c<4;c++)
    if(!isfinite(Alignment.Affine[r][c])||!isfinite(Alignment.AffineI[r][c])) {
      fprintf(stderr,"Registration: invalid affine matrix.\n");exit(1);
    }
  if(!registration_distortions.nt)return;
  if(registration_distortions.nt!=3||registration_distortions.dim.x!=Alignment.dimensionT.x||
     registration_distortions.dim.y!=Alignment.dimensionT.y||registration_distortions.dim.z!=Alignment.dimensionT.z) {
    fprintf(stderr,"Registration: displacement field must have three components on the target grid.\n");exit(1);
  }
  if(fabs(registration_distortions.resolution.x-Alignment.resolutionT.x)>1.e-5*fmax(1.,fabs(Alignment.resolutionT.x))||
     fabs(registration_distortions.resolution.y-Alignment.resolutionT.y)>1.e-5*fmax(1.,fabs(Alignment.resolutionT.y))||
     fabs(registration_distortions.resolution.z-Alignment.resolutionT.z)>1.e-5*fmax(1.,fabs(Alignment.resolutionT.z))) {
    fprintf(stderr,"Registration: displacement spacing differs from the target grid.\n");exit(1);
  }
  if(Alignment.has_voxel_affineT)for(r=0;r<3;r++)for(c=0;c<4;c++) {
    expected=Alignment.voxel_affineT.m[r][c];actual=registration_distortions.ijk_to_xyz.m[r][c];
    tolerance=1.e-5*fmax(1.,fabs(expected));
    if(!isfinite(actual)||fabs(expected-actual)>tolerance) {
      fprintf(stderr,"Registration: displacement geometry differs from the saved target grid.\n");exit(1);
    }
  }
  for(j=0;j<registration_distortions.n4d;j++)if(!isfinite(registration_distortions.SpaceTime.fcData[j].real)) {
    fprintf(stderr,"Registration: nonfinite displacement rejected.\n");exit(1);
  }
}

void register_volume( BOOL downsample )
{
  struct data_stack *stack, *stack_save;
  struct volume_list  *volume;
  extern struct volume_list *volume_list_root;
  struct data_stack source;
  FCOMPLEX sum_data;
  IPOINT3D Index_reg, Low_orig, High_orig, jOrig, jRel, iOrig, iWidth;
  IPOINT3D SmoothKernel, GaussianKernel;
  POINT3D  Space_reg, Space_orig, Index_orig, fwhm;
  BOOL InVolume, NonLinear;
  mat44 ijk_to_xyz;        // data indices (i,j,k) to space (x,y,z) in aligned,     centered coordinates
  mat44 xyz_to_ijk;        // space (x,y,z) to data indices (i,j,k) in non-aligned, centered coordinates
  float sum_coeff, weight, *coeff_x, *coeff_y, *coeff_z, point4[4];
  int jt;JIP_INDEX index, j;

  // If critical alignment values have not been set explicitly,
  // take values from the first stack in the source list.
  if ( ((JIP_INDEX)(Alignment.dimensionT.x))*Alignment.dimensionT.y*Alignment.dimensionT.z == 0 )    // matrix
    {
      Alignment.dimensionT = volume_list_root->stack->dim;
      printf("Using matrix %d %d %d\n",Alignment.dimensionT.x,Alignment.dimensionT.y,Alignment.dimensionT.z);
    }
  if ( Alignment.resolutionT.x*Alignment.resolutionT.y*Alignment.resolutionT.z == 0 ) // resolution
    {
      Alignment.resolutionT = volume_list_root->stack->resolution;
      printf("Using resolution %g %g %g\n",Alignment.resolutionT.x,
	     Alignment.resolutionT.y,Alignment.resolutionT.z);
    }
  if ( Alignment.directionT.x*Alignment.directionT.y*Alignment.directionT.z == 0 )    // direction
    {
      Alignment.directionT = volume_list_root->stack->direction;
      printf("Using direction %g %g %g\n",Alignment.directionT.x,
	     Alignment.directionT.y,Alignment.directionT.z);
    }
  if ( Alignment.originT.x > 1.e5 )
    {
      Alignment.originT = volume_list_root->stack->origin;
      printf("Using origin %g %g %g\n",Alignment.originT.x,Alignment.originT.y,Alignment.originT.z);
    }

  if (Alignment.interpolation == INTERP_GAUSSIAN )
    {
      printf("Register data using a Gaussian interpolation kernel,\n");
      printf("which will smooth data by 1.5 x orginal voxel size but eliminate ringing.\n");
    }
  else
    {
      printf("Register data using a Lanczos interpolation kernel,\n");
      printf("which will not smooth data but will exhibit some ringing.\n");
      printf("For downsampling > 2x, a Gaussian kernel will be used instead.\n");
    }
  printf("Switch kernels by:\n");
  printf("    set interpolation-kernel [type] , where type is Gaussian or Lanczos\n");

  // general affine matrix multiplied by orientation matrix above
  define_affine_matrix();
  validate_registration_map();

  /* Cycle through the volume list, beginning with the root. */
  stack_save = 0;
  for ( volume=volume_list_root; volume; volume=volume->next )
    {
      if ( (stack=volume->stack) == stack_save ) continue;
      stack_save = stack;
      //////////////////////////////////////////////////////////////////////////////////////
      //////////////////////////////////////////////////////////////////////////////////////
      //////////////////////////////////////////////////////////////////////////////////////
      // Copy the stack data, which will be replaced during registration.
      source = *stack;
      source.SpaceTime.fcData = fcomplex_vector(source.n4d);
      // Copy the target data into the source.
      for (j=0; j<source.n4d; j++)
	source.SpaceTime.fcData[j] = stack->SpaceTime.fcData[j];

      // Now redefine aspects of the stack.
      stack->resolution   = Alignment.resolutionT;
      stack->dim          = Alignment.dimensionT;
      stack->registration = Alignment.registration;
      stack->n2d = jip_product(stack->dim.x,stack->dim.y);
      stack->n3d = jip_product(jip_product(stack->dim.x,stack->dim.y),stack->dim.z);
      stack->n4d = jip_product(stack->n3d,stack->nt);
      stack->npts_volume = jip_product(2,stack->n4d);
      // Define the coordinate transformations using centered coordinates.
      // The origin will be shifted later.
      ijk_to_xyz = centered_ijk_to_xyz( *stack );
      xyz_to_ijk = centered_xyz_to_ijk( source );
      // free the old data array and create a new one.
      free_fcomplex_vector(stack->SpaceTime.fcData);
      stack->SpaceTime.fcData = fcomplex_vector(stack->n4d);
      for (j=0; j<stack->n4d; j++)
	stack->SpaceTime.fcData[j].real = stack->SpaceTime.fcData[j].imag = 0.;
      // Attach coordinate transformations to the source and the registered stack.
      source.xyz_to_ijk = xyz_to_ijk;
      stack->ijk_to_xyz = ijk_to_xyz;
      //////////////////////////////////////////////////////////////////////////////////////
      //////////////////////////////////////////////////////////////////////////////////////
      //////////////////////////////////////////////////////////////////////////////////////

      fwhm.x = Alignment.resolutionT.x / source.resolution.x;
      fwhm.y = Alignment.resolutionT.y / source.resolution.y;
      fwhm.z = Alignment.resolutionT.z / source.resolution.z;

      iWidth.x = MAX(floor((double)(fwhm.x)),2);
      iWidth.y = MAX(floor((double)(fwhm.y)),2);
      iWidth.z = MAX(floor((double)(fwhm.z)),2);

      // Use iWidth to differentiate between interpolation and smoothing.
      SmoothKernel.x = iWidth.x > 2;
      SmoothKernel.y = iWidth.y > 2;
      SmoothKernel.z = iWidth.z > 2;

      GaussianKernel.x = (Alignment.interpolation == INTERP_GAUSSIAN ) || SmoothKernel.x || downsample;
      GaussianKernel.y = (Alignment.interpolation == INTERP_GAUSSIAN ) || SmoothKernel.y || downsample;
      GaussianKernel.z = (Alignment.interpolation == INTERP_GAUSSIAN ) || SmoothKernel.z || downsample;

      // For "standard" interpolation (not >2x down-sampling during registration),
      // set Gaussian FWHM to 1.5 voxels.
      if ( !SmoothKernel.x )
	fwhm.x = 1.5;
      if ( !SmoothKernel.y )
	fwhm.y = 1.5;
      if ( !SmoothKernel.z )
	fwhm.z = 1.5;

      if ( (Alignment.interpolation != INTERP_GAUSSIAN ) && SmoothKernel.x )
	printf("Interpolation type changed in Gaussian in x dimension.\n");
      if ( (Alignment.interpolation != INTERP_GAUSSIAN ) && SmoothKernel.y )
	printf("Interpolation type changed in Gaussian in y dimension.\n");
      if ( (Alignment.interpolation != INTERP_GAUSSIAN ) && SmoothKernel.z )
	printf("Interpolation type changed in Gaussian in z dimension.\n");

      coeff_x = float_vector(2*iWidth.x+2);   // total width is at least 2*width+1
      coeff_y = float_vector(2*iWidth.y+2);
      coeff_z = float_vector(2*iWidth.z+2);

      NonLinear =
	(registration_distortions.nt == 3) &&
	(registration_distortions.dim.x == stack->dim.x) &&
	(registration_distortions.dim.y == stack->dim.y) &&
	(registration_distortions.dim.z == stack->dim.z);

      for (jt=0; jt<stack->nt; jt++)
	{
	  // Loop over voxels in the standard resolution space.
	  for (Index_reg.z=0; Index_reg.z<stack->dim.z; Index_reg.z++)
	    {
	      for (Index_reg.y=0; Index_reg.y<stack->dim.y; Index_reg.y++)
		{
		  for (Index_reg.x=0; Index_reg.x<stack->dim.x; Index_reg.x++)
		    {
		      // Convert from data indices --> space in the registered system.
		      Space_reg = Convert_Voxel_to_Space( Index_reg, stack );
		      point4[0] = Space_reg.x;  point4[1] = Space_reg.y;
		      point4[2] = Space_reg.z;  point4[3] = 1;
		      // Rotate & translate.
		      Space_orig.x = Space_orig.y = Space_orig.z = 0.;
		      for (j=0; j<4; j++)
			{
			  Space_orig.x += Alignment.Affine[0][j] * point4[j];
			  Space_orig.y += Alignment.Affine[1][j] * point4[j];
			  Space_orig.z += Alignment.Affine[2][j] * point4[j];
			}
		      // non-linear distortion fields.
		      if ( NonLinear )
			{
			  index = index4d(Index_reg.x,Index_reg.y,Index_reg.z,0,&registration_distortions);
			  Space_orig.x += registration_distortions.SpaceTime.fcData[index].real;
			  index = index4d(Index_reg.x,Index_reg.y,Index_reg.z,1,&registration_distortions);
			  Space_orig.y += registration_distortions.SpaceTime.fcData[index].real;
			  index = index4d(Index_reg.x,Index_reg.y,Index_reg.z,2,&registration_distortions);
			  Space_orig.z += registration_distortions.SpaceTime.fcData[index].real;
			}
		      // Convert from space --> indices in the original (non-registered) space.
		      Index_orig = Convert_Space_to_rVoxel( Space_orig, &source );

		      // If the nearest data value in the original system is zero
		      // then ignore this point. This prevents values from being assigned
		      // to pixels due only to neighbors, which messes things up with restricted volumes.
		      jOrig.x = rint((double)Index_orig.x);
		      jOrig.y = rint((double)Index_orig.y);
		      jOrig.z = rint((double)Index_orig.z);
		      if ( Alignment.WrapAround.x )
			{ // Wrap x
			  if ( jOrig.x < 0 )
			    jOrig.x += source.dim.x;
			  else if ( jOrig.x >= source.dim.x )
			    jOrig.x -= source.dim.x;
			}
		      if ( Alignment.WrapAround.y )
			{ // Wrap y
			  if ( jOrig.y < 0 )
			    jOrig.y += source.dim.y;
			  else if ( jOrig.y >= source.dim.y )
			    jOrig.y -= source.dim.y;
			}
		      if ( Alignment.WrapAround.z )
			{ // Wrap z
			  if ( jOrig.z < 0 )
			    jOrig.z += source.dim.z;
			  else if ( jOrig.z >= source.dim.z )
			    jOrig.z -= source.dim.z;
			}
		      InVolume = ( jOrig.x>=0 && jOrig.x<source.dim.x &&
				   jOrig.y>=0 && jOrig.y<source.dim.y &&
				   jOrig.z>=0 && jOrig.z<source.dim.z );

		      if ( InVolume )
			{
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
			      if ( GaussianKernel.x )
				coeff_x[jRel.x] = Gauss((float)(Index_orig.x-jOrig.x), 0.,0.425*fwhm.x);
			      else
				coeff_x[jRel.x] = Lanczos_filter((float)(Index_orig.x-jOrig.x),iWidth.x);
			    }
			  for (jOrig.y=Low_orig.y-iWidth.y, jRel.y=0;
			       jOrig.y<=High_orig.y+iWidth.y; jOrig.y++, jRel.y++)
			    {
			      if ( GaussianKernel.y )
				coeff_y[jRel.y] = Gauss((float)(Index_orig.y-jOrig.y), 0.,0.425*fwhm.y);
			      else
				coeff_y[jRel.y] = Lanczos_filter((float)(Index_orig.y-jOrig.y),iWidth.y);
			    }
			  for (jOrig.z=Low_orig.z-iWidth.z, jRel.z=0;
			       jOrig.z<=High_orig.z+iWidth.z; jOrig.z++, jRel.z++)
			    {
			      if ( GaussianKernel.z )
				coeff_z[jRel.z] = Gauss((float)(Index_orig.z-jOrig.z), 0.,0.425*fwhm.z);
			      else
				coeff_z[jRel.z] = Lanczos_filter((float)(Index_orig.z-jOrig.z),iWidth.z);
			    }
			  sum_coeff = sum_data.real = sum_data.imag = 0.;
			  for (jOrig.z=Low_orig.z-iWidth.z, jRel.z=0;
			       jOrig.z<=High_orig.z+iWidth.z; jOrig.z++, jRel.z++)
			    {
			      iOrig.z = jOrig.z;  // don't wrap z
			      if ( Alignment.WrapAround.z )
				{ // Wrap z
				  if ( iOrig.z < 0 )
				    iOrig.z += source.dim.z;
				  else if ( iOrig.z >= source.dim.z )
				    iOrig.z -= source.dim.z;
				}
			      for (jOrig.y=Low_orig.y-iWidth.y, jRel.y=0;
				   jOrig.y<=High_orig.y+iWidth.y; jOrig.y++, jRel.y++)
				{
				  // Wrap y
				  iOrig.y = jOrig.y;
				  if ( Alignment.WrapAround.y )
				    { // Wrap y
				      if ( iOrig.y < 0 )
					iOrig.y += source.dim.y;
				      else if ( iOrig.y >= source.dim.y )
					iOrig.y -= source.dim.y;
				    }
				  for (jOrig.x=Low_orig.x-iWidth.x, jRel.x=0;
				       jOrig.x<=High_orig.x+iWidth.x; jOrig.x++, jRel.x++)
				    {
				      // Wrap x
				      iOrig.x = jOrig.x;
				      if ( Alignment.WrapAround.x )
					{ // Wrap x
					  if ( iOrig.x < 0 )
					    iOrig.x += source.dim.x;
					  else if ( iOrig.x >= source.dim.x )
					    iOrig.x -= source.dim.x;
					}
				      InVolume = ( iOrig.x>=0 && iOrig.x<source.dim.x &&
						   iOrig.y>=0 && iOrig.y<source.dim.y &&
						   iOrig.z>=0 && iOrig.z<source.dim.z );
				      if ( InVolume )
					{
					  // sum over neighbors in x
					  index = index4d(iOrig.x,iOrig.y,iOrig.z,jt,&source);
					  weight = coeff_x[jRel.x] * coeff_y[jRel.y] * coeff_z[jRel.z];
					  if ( index >= source.n4d )
					    exit(1);
					  sum_data.real  += weight * source.SpaceTime.fcData[index].real;
					  sum_data.imag  += weight * source.SpaceTime.fcData[index].imag;
					  sum_coeff += weight;
					}
				    }
				}
			    }
			  index = index4d(Index_reg.x,Index_reg.y,Index_reg.z,jt,stack);
			  if ( index < 0 || index >= stack->n4d )
			    {
			      printf("stack index out of bounds: x=%d, y=%d, z=%d, t=%d, index = %td, n=%td\n",
				     Index_reg.x,Index_reg.y,Index_reg.z,jt,index,stack->n4d);
			      exit(1);
			    }
			  if ( sum_coeff == 0. ) sum_coeff = 1;
			  stack->SpaceTime.fcData[index].real = sum_data.real / sum_coeff;
			  stack->SpaceTime.fcData[index].imag = sum_data.imag / sum_coeff;
			}
		    } // x
		} // y
	    } // z
	  if ( stack->nt > 5 )
	    {
	      fprintf(stdout,"\rFinished %d of %d time points for stack `%s'.",
		      jt+1,stack->nt,stack->name);
	      fflush(stdout);
	    }
	} // t
      // The following statements caused a memory error on one i386 machine for reasons unknown.
      free_float_vector(coeff_x);
      free_float_vector(coeff_y);
      free_float_vector(coeff_z);
      free_fcomplex_vector(source.SpaceTime.fcData);
      // replace the centered coordinate system using the correct origin.
      stack->origin = Alignment.originT;
      stack->direction = Alignment.directionT;
      stack->ijk_to_xyz = Alignment.has_voxel_affineT?Alignment.voxel_affineT:scale_and_shift_to_mat44(stack->resolution,stack->origin,stack->direction);
      stack->xyz_to_ijk = invert_mat44( stack->ijk_to_xyz );
      if ( stack->nt>5 ) printf("\n");
    }  
}

void inverse_register_volume( BOOL downsample )
{
  struct data_stack *stack, *stack_save;
  struct volume_list  *volume;
  extern struct volume_list *volume_list_root;
  struct data_stack source;
  FCOMPLEX sum_data;
  IPOINT3D Index_reg, Low_orig, High_orig, jOrig, jRel, iOrig, iWidth;
  IPOINT3D SmoothKernel, GaussianKernel;
  POINT3D  Space_reg, Space_orig, Index_orig, fwhm;
  BOOL InVolume, NonLinear;
  JipWarpField warp;double query[3],solution[3];JIP_INDEX inverse_failed=0;
  mat44 ijk_to_xyz;        // data indices (i,j,k) to space (x,y,z) in aligned,     centered coordinates
  mat44 xyz_to_ijk;        // space (x,y,z) to data indices (i,j,k) in non-aligned, centered coordinates
  float sum_coeff, weight, *coeff_x, *coeff_y, *coeff_z, point4[4];
  int jt;JIP_INDEX index, j;

  // If critical alignment values have not been set explicitly,
  // take values from the first stack in the source list.
  if ( ((JIP_INDEX)(Alignment.dimensionS.x))*Alignment.dimensionS.y*Alignment.dimensionS.z == 0 )    // matrix
    {
      Alignment.dimensionS = volume_list_root->stack->dim;
      printf("Using matrix %d %d %d\n",Alignment.dimensionS.x,Alignment.dimensionS.y,Alignment.dimensionS.z);
    }
  if ( Alignment.resolutionS.x*Alignment.resolutionS.y*Alignment.resolutionS.z == 0 ) // resolution
    {
      Alignment.resolutionS = volume_list_root->stack->resolution;
      printf("Using resolution %g %g %g\n",Alignment.resolutionS.x,
	     Alignment.resolutionS.y,Alignment.resolutionS.z);
    }
  if ( Alignment.directionS.x*Alignment.directionS.y*Alignment.directionS.z == 0 )    // direction
    {
      Alignment.directionS = volume_list_root->stack->direction;
      printf("Using direction %g %g %g\n",Alignment.directionS.x,
	     Alignment.directionS.y,Alignment.directionS.z);
    }
  if ( Alignment.originS.x > 1.e5 )
    {
      Alignment.originS = volume_list_root->stack->origin;
      printf("Using origin %g %g %g\n",Alignment.originS.x,
	     Alignment.originS.y,Alignment.originS.z);
    }

  if (Alignment.interpolation == INTERP_GAUSSIAN )
    {
      printf("Register data using a Gaussian interpolation kernel,\n");
      printf("which will smooth data by 1.5x but eliminate ringing.\n");
    }
  else
    {
      printf("Register data using a Lanczos interpolation kernel,\n");
      printf("which will not smooth data but will exhibit some ringing.\n");
      printf("For downsampling > 2x, a Gaussian kernel will be used instead.\n");
    }
  printf("Switch kernels by:\n");
  printf("    set interpolation-kernel [type] , where type is Gaussian or Lanczos\n");

  // general affine matrix multiplied by orientation matrix above
  define_affine_matrix();
  validate_registration_map();

  memset(&warp,0,sizeof(warp));
  NonLinear=registration_distortions.nt!=0;
  if(NonLinear) {
    if(registration_distortions.nt!=3||registration_distortions.dim.x!=Alignment.dimensionT.x||
       registration_distortions.dim.y!=Alignment.dimensionT.y||registration_distortions.dim.z!=Alignment.dimensionT.z) {
      fprintf(stderr,"Inverse registration: displacement grid must match the target grid.\n");exit(1);
    }
    warp.dim[0]=registration_distortions.dim.x;warp.dim[1]=registration_distortions.dim.y;warp.dim[2]=registration_distortions.dim.z;
    warp.spacing[0]=Alignment.resolutionT.x;warp.spacing[1]=Alignment.resolutionT.y;warp.spacing[2]=Alignment.resolutionT.z;
    for(j=0;j<3;j++) {
      if(!isfinite(warp.spacing[j])||warp.spacing[j]<=0){fprintf(stderr,"Invalid warp spacing\n");exit(1);}
      warp.origin[j]=-(warp.dim[j]-1)*warp.spacing[j]/2.;
      warp.component[j]=(const float*)(registration_distortions.SpaceTime.fcData+j*registration_distortions.n3d);
    }
    warp.stride=sizeof(FCOMPLEX)/sizeof(float);
    if(!jip_warp_validate(&warp,Alignment.Affine)) {
      fprintf(stderr,"Inverse registration: folded, singular or nonfinite displacement map rejected.\n");exit(1);
    }
    for(j=0;j<registration_distortions.n4d;j++)if(!isfinite(registration_distortions.SpaceTime.fcData[j].real)){
      fprintf(stderr,"Inverse registration: nonfinite displacement rejected.\n");exit(1);
    }
  }

  /* Cycle through the volume list, beginning with the root. */
  stack_save = 0;
  for ( volume=volume_list_root; volume; volume=volume->next )
    {
      if ( (stack=volume->stack) == stack_save ) continue;
      stack_save = stack;
      //////////////////////////////////////////////////////////////////////////////////////
      //////////////////////////////////////////////////////////////////////////////////////
      //////////////////////////////////////////////////////////////////////////////////////
      // Copy the stack data, which will be replaced during registration.
      source = *stack;
      source.SpaceTime.fcData = fcomplex_vector(source.n4d);
      // Copy the target data into the source.
      for (j=0; j<source.n4d; j++)
	source.SpaceTime.fcData[j] = stack->SpaceTime.fcData[j];

      // Now redefine aspects of the stack.
      stack->resolution   = Alignment.resolutionS;
      stack->dim          = Alignment.dimensionS;
      stack->registration = Alignment.registration;
      stack->n2d = jip_product(stack->dim.x,stack->dim.y);
      stack->n3d = jip_product(jip_product(stack->dim.x,stack->dim.y),stack->dim.z);
      stack->n4d = jip_product(stack->n3d,stack->nt);
      stack->npts_volume = jip_product(2,stack->n4d);
      // Define the coordinate transformations using centered coordinates.
      // The origin will be shifted later.
      ijk_to_xyz = centered_ijk_to_xyz( *stack );
      xyz_to_ijk = centered_xyz_to_ijk( source );
      //
      // free the old data array and create a new one.
      free_fcomplex_vector(stack->SpaceTime.fcData);
      stack->SpaceTime.fcData = fcomplex_vector(stack->n4d);
      for (j=0; j<stack->n4d; j++)
	stack->SpaceTime.fcData[j].real = stack->SpaceTime.fcData[j].imag = 0.;
      // Attach coordinate transformations to the source and the registered stack.
      source.xyz_to_ijk = xyz_to_ijk;
      stack->ijk_to_xyz = ijk_to_xyz;
      //////////////////////////////////////////////////////////////////////////////////////
      //////////////////////////////////////////////////////////////////////////////////////
      //////////////////////////////////////////////////////////////////////////////////////

      // If the resolution of the new stack is lower than the resolution of the original,
      // then this is essentially smoothing. Use a gaussian smoothing kernel with a fwhm
      // equal to the difference in resolution.
      fwhm.x = Alignment.resolutionS.x / source.resolution.x;
      fwhm.y = Alignment.resolutionS.y / source.resolution.y;
      fwhm.z = Alignment.resolutionS.z / source.resolution.z;

      iWidth.x = MAX(floor((double)(fwhm.x)),2);
      iWidth.y = MAX(floor((double)(fwhm.y)),2);
      iWidth.z = MAX(floor((double)(fwhm.z)),2);

      // Use iWidth to differentiate between interpolation and smoothing.
      SmoothKernel.x = iWidth.x > 2;
      SmoothKernel.y = iWidth.y > 2;
      SmoothKernel.z = iWidth.z > 2;

      // For "standard" interpolation (not >2x down-sampling during registration), set Gaussian FWHM to 1.5 voxels.
      if ( !SmoothKernel.x )
	fwhm.x = 1.5;
      if ( !SmoothKernel.y )
	fwhm.y = 1.5;
      if ( !SmoothKernel.z )
	fwhm.z = 1.5;

      GaussianKernel.x = (Alignment.interpolation == INTERP_GAUSSIAN ) || SmoothKernel.x || downsample;
      GaussianKernel.y = (Alignment.interpolation == INTERP_GAUSSIAN ) || SmoothKernel.y || downsample;
      GaussianKernel.z = (Alignment.interpolation == INTERP_GAUSSIAN ) || SmoothKernel.z || downsample;

      if ( (Alignment.interpolation != INTERP_GAUSSIAN ) && SmoothKernel.x )
	printf("Interpolation type changed in Gaussian in x dimension.\n");
      if ( (Alignment.interpolation != INTERP_GAUSSIAN ) && SmoothKernel.y )
	printf("Interpolation type changed in Gaussian in y dimension.\n");
      if ( (Alignment.interpolation != INTERP_GAUSSIAN ) && SmoothKernel.z )
	printf("Interpolation type changed in Gaussian in z dimension.\n");

      coeff_x = float_vector(2*iWidth.x+2);   // total width is at least 2*width+1
      coeff_y = float_vector(2*iWidth.y+2);
      coeff_z = float_vector(2*iWidth.z+2);

      for (jt=0; jt<stack->nt; jt++)
	{
	  // Loop over voxels in the target resolution space.
	  for (Index_reg.z=0; Index_reg.z<stack->dim.z; Index_reg.z++)
	    {
	      for (Index_reg.y=0; Index_reg.y<stack->dim.y; Index_reg.y++)
		{
		  for (Index_reg.x=0; Index_reg.x<stack->dim.x; Index_reg.x++)
		    {
		      // Convert from data indices --> space in the registered system.
		      Space_reg = Convert_Voxel_to_Space( Index_reg, stack );
		      point4[0] = Space_reg.x;  point4[1] = Space_reg.y;
		      point4[2] = Space_reg.z;  point4[3] = 1;
		      // Rotate & translate.
		      Space_orig.x = Space_orig.y = Space_orig.z = 0.;
		      for (j=0; j<4; j++)
			{
			  Space_orig.x += Alignment.AffineI[0][j] * point4[j];
			  Space_orig.y += Alignment.AffineI[1][j] * point4[j];
			  Space_orig.z += Alignment.AffineI[2][j] * point4[j];
			}
                      if(NonLinear) {
                        query[0]=Space_reg.x;query[1]=Space_reg.y;query[2]=Space_reg.z;
                        if(!jip_warp_inverse(&warp,Alignment.Affine,Alignment.AffineI,query,solution)) {inverse_failed++;continue;}
                        Space_orig.x=solution[0];Space_orig.y=solution[1];Space_orig.z=solution[2];
                      }
		      // Convert from space --> indices in the original (non-registered) space.
		      Index_orig = Convert_Space_to_rVoxel( Space_orig, &source );

		      // If the nearest data value in the original system is zero
		      // then ignore this point. This prevents values from being assigned
		      // to pixels due only to neighbors, which messes things up with restricted volumes.
		      jOrig.x = rint((double)Index_orig.x);
		      jOrig.y = rint((double)Index_orig.y);
		      jOrig.z = rint((double)Index_orig.z);
		      if ( Alignment.WrapAround.x )
			{ // Wrap x
			  if ( jOrig.x < 0 )
			    jOrig.x += source.dim.x;
			  else if ( jOrig.x >= source.dim.x )
			    jOrig.x -= source.dim.x;
			}
		      if ( Alignment.WrapAround.y )
			{ // Wrap y
			  if ( jOrig.y < 0 )
			    jOrig.y += source.dim.y;
			  else if ( jOrig.y >= source.dim.y )
			    jOrig.y -= source.dim.y;
			}
		      if ( Alignment.WrapAround.z )
			{ // Wrap z
			  if ( jOrig.z < 0 )
			    jOrig.z += source.dim.z;
			  else if ( jOrig.z >= source.dim.z )
			    jOrig.z -= source.dim.z;
			}
		      InVolume = ( jOrig.x>=0 && jOrig.x<source.dim.x &&
				   jOrig.y>=0 && jOrig.y<source.dim.y &&
				   jOrig.z>=0 && jOrig.z<source.dim.z );

		      if ( InVolume )
			{
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
			      if ( GaussianKernel.x )
				coeff_x[jRel.x] = Gauss((float)(Index_orig.x-jOrig.x), 0.,0.425*fwhm.x);
			      else
				coeff_x[jRel.x] = Lanczos_filter((float)(Index_orig.x-jOrig.x),iWidth.x);
			    }
			  for (jOrig.y=Low_orig.y-iWidth.y, jRel.y=0;
			       jOrig.y<=High_orig.y+iWidth.y; jOrig.y++, jRel.y++)
			    {
			      if ( GaussianKernel.y )
				coeff_y[jRel.y] = Gauss((float)(Index_orig.y-jOrig.y), 0.,0.425*fwhm.y);
			      else
				coeff_y[jRel.y] = Lanczos_filter((float)(Index_orig.y-jOrig.y),iWidth.y);
			    }
			  for (jOrig.z=Low_orig.z-iWidth.z, jRel.z=0;
			       jOrig.z<=High_orig.z+iWidth.z; jOrig.z++, jRel.z++)
			    {
			      if ( GaussianKernel.z )
				coeff_z[jRel.z] = Gauss((float)(Index_orig.z-jOrig.z), 0.,0.425*fwhm.z);
			      else
				coeff_z[jRel.z] = Lanczos_filter((float)(Index_orig.z-jOrig.z),iWidth.z);
			    }
			  sum_coeff = sum_data.real = sum_data.imag = 0.;
			  for (jOrig.z=Low_orig.z-iWidth.z, jRel.z=0;
			       jOrig.z<=High_orig.z+iWidth.z; jOrig.z++, jRel.z++)
			    {
			      iOrig.z = jOrig.z;  // don't wrap z
			      if ( Alignment.WrapAround.z )
				{ // Wrap z
				  if ( iOrig.z < 0 )
				    iOrig.z += source.dim.z;
				  else if ( iOrig.z >= source.dim.z )
				    iOrig.z -= source.dim.z;
				}
			      for (jOrig.y=Low_orig.y-iWidth.y, jRel.y=0;
				   jOrig.y<=High_orig.y+iWidth.y; jOrig.y++, jRel.y++)
				{
				  // Wrap y
				  iOrig.y = jOrig.y;
				  if ( Alignment.WrapAround.y )
				    { // Wrap y
				      if ( iOrig.y < 0 )
					iOrig.y += source.dim.y;
				      else if ( iOrig.y >= source.dim.y )
					iOrig.y -= source.dim.y;
				    }
				  for (jOrig.x=Low_orig.x-iWidth.x, jRel.x=0;
				       jOrig.x<=High_orig.x+iWidth.x; jOrig.x++, jRel.x++)
				    {
				      // Wrap x
				      iOrig.x = jOrig.x;
				      if ( Alignment.WrapAround.x )
					{ // Wrap x
					  if ( iOrig.x < 0 )
					    iOrig.x += source.dim.x;
					  else if ( iOrig.x >= source.dim.x )
					    iOrig.x -= source.dim.x;
					}
				      InVolume = ( iOrig.x>=0 && iOrig.x<source.dim.x &&
						   iOrig.y>=0 && iOrig.y<source.dim.y &&
						   iOrig.z>=0 && iOrig.z<source.dim.z );
				      if ( InVolume )
					{
					  // sum over neighbors in x
					  index = index4d(iOrig.x,iOrig.y,iOrig.z,jt,&source);
					  weight = coeff_x[jRel.x] * coeff_y[jRel.y] * coeff_z[jRel.z];
					  if ( index >= source.n4d )
					    exit(1);
					  sum_data.real  += weight * source.SpaceTime.fcData[index].real;
					  sum_data.imag  += weight * source.SpaceTime.fcData[index].imag;
					  sum_coeff += weight;
					}
				    }
				}
			    }
			  index = index4d(Index_reg.x,Index_reg.y,Index_reg.z,jt,stack);
			  if ( index < 0 || index >= stack->n4d )
			    {
			      printf("stack index out of bounds: x=%d, y=%d, z=%d, t=%d, index = %td, n=%td\n",
				     Index_reg.x,Index_reg.y,Index_reg.z,jt,index,stack->n4d);
			      exit(1);
			    }
			  if ( sum_coeff == 0. ) sum_coeff = 1;
			  stack->SpaceTime.fcData[index].real = sum_data.real / sum_coeff;
			  stack->SpaceTime.fcData[index].imag = sum_data.imag / sum_coeff;
			}
		    } // x
		} // y
	    } // z
	  if ( stack->nt > 5 )
	    {
	      fprintf(stdout,"\rFinished %d of %d time points for stack `%s'.",
		      jt+1,stack->nt,stack->name);
	      fflush(stdout);
	    }
	} // t
      // The following statements caused a memory error on one i386 machine for reasons unknown.
      free_float_vector(coeff_x);
      free_float_vector(coeff_y);
      free_float_vector(coeff_z);
      free_fcomplex_vector(source.SpaceTime.fcData);
      // replace the centered coordinate system using the correct origin.
      stack->origin = Alignment.originS;
      stack->direction = Alignment.directionS;
      stack->ijk_to_xyz = Alignment.has_voxel_affineS?Alignment.voxel_affineS:scale_and_shift_to_mat44(stack->resolution,stack->origin,stack->direction);
      stack->xyz_to_ijk = invert_mat44( stack->ijk_to_xyz );
      if ( stack->nt>5 ) printf("\n");
      if(inverse_failed)fprintf(stderr,"Inverse mapping: %td output samples outside the field domain or not converged; filled with zero.\n",inverse_failed);
    }  
}

int register_histology( BOOL downsample )
{
  struct data_stack *stack, *stack_save;
  struct volume_list  *volume;
  extern struct volume_list *volume_list_root;
  struct data_stack source;
  FCOMPLEX sum_data;
  IPOINT3D Index_reg;
  IPOINT2D Low_orig, High_orig, jOrig, jRel, iOrig, iWidth, SmoothKernel, GaussianKernel;
  POINT3D  Space_reg, Space_orig, Index_orig;
  POINT2D fwhm;
  BOOL InVolume, NonLinear;
  mat44 ijk_to_xyz;        // data indices (i,j,k) to space (x,y,z) in aligned,     centered coordinates
  mat44 xyz_to_ijk;        // space (x,y,z) to data indices (i,j,k) in non-aligned, centered coordinates
  float sum_coeff, weight, *coeff_x, *coeff_y, point4[4];
  int jt;JIP_INDEX index, j;

  // If critical alignment values have not been set explicitly,
  // take values from the first stack in the source list.
  if ( ((JIP_INDEX)(Alignment.dimensionT.x))*Alignment.dimensionT.y*Alignment.dimensionT.z == 0 )    // matrix
    {
      Alignment.dimensionT = volume_list_root->stack->dim;
      printf("Using matrix %d %d %d\n",Alignment.dimensionT.x,Alignment.dimensionT.y,Alignment.dimensionT.z);
    }
  if ( Alignment.resolutionT.x*Alignment.resolutionT.y*Alignment.resolutionT.z == 0 ) // resolution
    {
      Alignment.resolutionT = volume_list_root->stack->resolution;
      printf("Using resolution %g %g %g\n",Alignment.resolutionT.x,
	     Alignment.resolutionT.y,Alignment.resolutionT.z);
    }
  if ( Alignment.directionT.x*Alignment.directionT.y*Alignment.directionT.z == 0 )    // direction
    {
      Alignment.directionT = volume_list_root->stack->direction;
      printf("Using direction %g %g %g\n",Alignment.directionT.x,
	     Alignment.directionT.y,Alignment.directionT.z);
    }
  if ( Alignment.originT.x > 1.e5 )
    {
      Alignment.originT = volume_list_root->stack->origin;
      printf("Using origin %g %g %g\n",Alignment.originT.x,
	     Alignment.originT.y,Alignment.originT.z);
    }

  NonLinear =
    (registration_distortions.nt == 2) &&
    (registration_distortions.dim.x == Alignment.dimensionT.x) &&
    (registration_distortions.dim.y == Alignment.dimensionT.y);

  if (Alignment.interpolation == INTERP_GAUSSIAN )
    {
      printf("Register data using a Gaussian interpolation kernel,\n");
      printf("which will smooth data by 1.5x but eliminate ringing.\n");
    }
  else
    {
      printf("Register data using a Lanczos interpolation kernel,\n");
      printf("which will not smooth data but will exhibit some ringing.\n");
      printf("For downsampling > 2x, a Gaussian kernel will be used instead.\n");
    }
  printf("Switch kernels by:\n");
  printf("    set interpolation-kernel [type] , where type is Gaussian or Lanczos\n");

  /* Cycle through the volume list, beginning with the root. */
  stack_save = 0;
  for ( volume=volume_list_root; volume; volume=volume->next )
    {
      if ( (stack=volume->stack) == stack_save ) continue;
      stack_save = stack;
      //////////////////////////////////////////////////////////////////////////////////////
      //////////////////////////////////////////////////////////////////////////////////////
      //////////////////////////////////////////////////////////////////////////////////////
      // Copy the stack data, which will be replaced during registration.
      source = *stack;
      // Make sure that the z dimension is consistent.
      if ( source.dim.z != Alignment.dimensionT.z )
	{
	  printf("Error: the alignment slice dimension (%d) must be consistent\n",Alignment.dimensionT.z);
	  printf("       with the source slice dimension (%d).\n",source.dim.z);
	  return(1);
	}
      source.SpaceTime.fcData = fcomplex_vector(source.n4d);
      // Copy the target data into the source.
      for (j=0; j<source.n4d; j++)
	source.SpaceTime.fcData[j] = stack->SpaceTime.fcData[j];

      // Now redefine aspects of the stack.
      stack->resolution   = Alignment.resolutionT;
      stack->dim          = Alignment.dimensionT;
      stack->registration = Alignment.registration;
      stack->n2d = jip_product(stack->dim.x,stack->dim.y);
      stack->n3d = jip_product(jip_product(stack->dim.x,stack->dim.y),stack->dim.z);
      stack->n4d = jip_product(stack->n3d,stack->nt);
      stack->npts_volume = jip_product(2,stack->n4d);
      // free the old data array and create a new one.
      free_fcomplex_vector(stack->SpaceTime.fcData);
      stack->SpaceTime.fcData = fcomplex_vector(stack->n4d);
      for (j=0; j<stack->n4d; j++)
	stack->SpaceTime.fcData[j].real = stack->SpaceTime.fcData[j].imag = 0.;

      // Define the coordinate transformations using centered coordinates.
      // The origin will be shifted later.
      ijk_to_xyz = centered_ijk_to_xyz( *stack );
      xyz_to_ijk = centered_xyz_to_ijk( source );
      // Attach coordinate transformations to the source and the registered stack.
      stack->ijk_to_xyz = ijk_to_xyz;  // target
      source.xyz_to_ijk = xyz_to_ijk;  // original histology
      //////////////////////////////////////////////////////////////////////////////////////
      //////////////////////////////////////////////////////////////////////////////////////
      //////////////////////////////////////////////////////////////////////////////////////

      // If the resolution of the new stack is lower than the resolution of the original,
      // then this is essentially smoothing. Use a gaussian smoothing kernel with a fwhm
      // equal to the difference in resolution.
      fwhm.x = Alignment.resolutionT.x / source.resolution.x;
      fwhm.y = Alignment.resolutionT.y / source.resolution.y;
      //      fwhm.x = 2. / source.resolution.x;
      //      fwhm.y = 2. / source.resolution.y;
      //      fwhm.z = 2. / source.resolution.z;
      iWidth.x = MAX(floor((double)(fwhm.x)),2);
      iWidth.y = MAX(floor((double)(fwhm.y)),2);

      coeff_x = float_vector(2*iWidth.x+2);   // total width is at least 2*width+1
      coeff_y = float_vector(2*iWidth.y+2);
 
      // Use iWidth to differentiate between interpolation and smoothing.
      SmoothKernel.x = iWidth.x > 2;
      SmoothKernel.y = iWidth.y > 2;

      GaussianKernel.x = (Alignment.interpolation == INTERP_GAUSSIAN ) || SmoothKernel.x || downsample;
      GaussianKernel.y = (Alignment.interpolation == INTERP_GAUSSIAN ) || SmoothKernel.y || downsample;

      if ( (Alignment.interpolation != INTERP_GAUSSIAN ) && SmoothKernel.x )
	printf("Interpolation type changed in Gaussian in x dimension.\n");
      if ( (Alignment.interpolation != INTERP_GAUSSIAN ) && SmoothKernel.y )
	printf("Interpolation type changed in Gaussian in y dimension.\n");

      for (jt=0; jt<stack->nt; jt++)
	{
	  // Loop over voxels in the standard resolution space.
	  for (Index_reg.z=0; Index_reg.z<stack->dim.z; Index_reg.z++)
	    {
	      define_slice_matrix(Index_reg.z);
	      for (Index_reg.y=0; Index_reg.y<stack->dim.y; Index_reg.y++)
		{
		  for (Index_reg.x=0; Index_reg.x<stack->dim.x; Index_reg.x++)
		    {
		      // Convert from data indices --> space in the registered system.
		      Space_reg = Convert_Voxel_to_Space( Index_reg, stack );
		      point4[0] = Space_reg.x;  point4[1] = Space_reg.y;
		      point4[2] = Space_reg.z;  point4[3] = 1;
		      // Rotate & translate.
		      Space_orig.x = Space_orig.y = Space_orig.z = 0.;
		      for (j=0; j<4; j++)
			{
			  Space_orig.x += Alignment.AffineH[0][j] * point4[j];
			  Space_orig.y += Alignment.AffineH[1][j] * point4[j];
			  Space_orig.z += Alignment.AffineH[2][j] * point4[j];
			}
		      // non-linear distortion fields.
		      if ( NonLinear )
			{
			  index = index4d(Index_reg.x,Index_reg.y,Index_reg.z,0,&registration_distortions);
			  Space_orig.x += registration_distortions.SpaceTime.fcData[index].real;
			  index = index4d(Index_reg.x,Index_reg.y,Index_reg.z,1,&registration_distortions);
			  Space_orig.y += registration_distortions.SpaceTime.fcData[index].real;
			}
		      // Convert from space --> indices in the original (non-registered) space.
		      Index_orig = Convert_Space_to_rVoxel( Space_orig, &source );

		      // If the nearest data value in the original system is zero
		      // then ignore this point. This prevents values from being assigned
		      // to pixels due only to neighbors, which messes things up with restricted volumes.
		      jOrig.x = rint((double)Index_orig.x);
		      jOrig.y = rint((double)Index_orig.y);
		      if ( Alignment.WrapAround.x )
			{ // Wrap x
			  if ( jOrig.x < 0 )
			    jOrig.x += source.dim.x;
			  else if ( jOrig.x >= source.dim.x )
			    jOrig.x -= source.dim.x;
			}
		      if ( Alignment.WrapAround.y )
			{ // Wrap y
			  if ( jOrig.y < 0 )
			    jOrig.y += source.dim.y;
			  else if ( jOrig.y >= source.dim.y )
			    jOrig.y -= source.dim.y;
			}
		      InVolume = ( jOrig.x>=0 && jOrig.x<source.dim.x &&
				   jOrig.y>=0 && jOrig.y<source.dim.y );
		      /*
		      if ( InVolume )
			{
			  index = index4d(jOrig.x,jOrig.y,Index_reg.z,jt,&source);
			  if ( source.SpaceTime.fcData[index].real == 0. &&
			       source.SpaceTime.fcData[index].imag == 0. )
			    InVolume = FALSE;
			}
		      */

		      if ( InVolume )
			{ // point of interest is in source volume
			  // define limits
			  Low_orig.x  = floor((double)Index_orig.x);
			  High_orig.x = ceil((double)Index_orig.x);
			  Low_orig.y  = floor((double)Index_orig.y);
			  High_orig.y = ceil((double)Index_orig.y);
			  // Interpolate to rectilinear pixels in original resolution space.
			  for (jOrig.x=Low_orig.x-iWidth.x, jRel.x=0;
			       jOrig.x<=High_orig.x+iWidth.x; jOrig.x++, jRel.x++)
			    {
			      if ( GaussianKernel.x )
				coeff_x[jRel.x] = Gauss((float)(Index_orig.x-jOrig.x), 0.,0.425*fwhm.x);
			      else
				coeff_x[jRel.x] = Lanczos_filter((float)(Index_orig.x-jOrig.x),iWidth.x);
			    }
			  for (jOrig.y=Low_orig.y-iWidth.y, jRel.y=0;
			       jOrig.y<=High_orig.y+iWidth.y; jOrig.y++, jRel.y++)
			    {
			      if ( GaussianKernel.y )
				coeff_y[jRel.y] = Gauss((float)(Index_orig.y-jOrig.y), 0.,0.425*fwhm.y);
			      else
				coeff_y[jRel.y] = Lanczos_filter((float)(Index_orig.y-jOrig.y),iWidth.y);
			    }
			  sum_coeff = sum_data.real = sum_data.imag = 0.;
			  for (jOrig.y=Low_orig.y-iWidth.y, jRel.y=0;
			       jOrig.y<=High_orig.y+iWidth.y; jOrig.y++, jRel.y++)
			    {
			      // Wrap y
			      iOrig.y = jOrig.y;
			      if ( Alignment.WrapAround.y )
				{ // Wrap y
				  if ( iOrig.y < 0 )
				    iOrig.y += source.dim.y;
				  else if ( iOrig.y >= source.dim.y )
				    iOrig.y -= source.dim.y;
				}
			      for (jOrig.x=Low_orig.x-iWidth.x, jRel.x=0;
				   jOrig.x<=High_orig.x+iWidth.x; jOrig.x++, jRel.x++)
				{
				  // Wrap x
				  iOrig.x = jOrig.x;
				  if ( Alignment.WrapAround.x )
				    { // Wrap x
				      if ( iOrig.x < 0 )
					iOrig.x += source.dim.x;
				      else if ( iOrig.x >= source.dim.x )
					iOrig.x -= source.dim.x;
				    }
				  InVolume = ( iOrig.x>=0 && iOrig.x<source.dim.x &&
					       iOrig.y>=0 && iOrig.y<source.dim.y );
				  if ( InVolume )
				    {
				      // sum over neighbors in x
				      index = index4d(iOrig.x,iOrig.y,Index_reg.z,jt,&source);
				      weight = coeff_x[jRel.x] * coeff_y[jRel.y];
				      sum_data.real  += weight * source.SpaceTime.fcData[index].real;
				      sum_data.imag  += weight * source.SpaceTime.fcData[index].imag;
				      sum_coeff += weight;
				    }
				} // x
			    } // y
			} // point of interest is in source volume
		      index = index4d(Index_reg.x,Index_reg.y,Index_reg.z,jt,stack);
		      if ( index < 0 || index >= stack->n4d )
			{
			  printf("stack index out of bounds: x=%d, y=%d, z=%d, t=%d, index = %td, n=%td\n",
				 Index_reg.x,Index_reg.y,Index_reg.z,jt,index,stack->n4d);
			  exit(1);
			}
		      if ( sum_coeff == 0. ) sum_coeff = 1;
		      stack->SpaceTime.fcData[index].real = sum_data.real / sum_coeff;
		      stack->SpaceTime.fcData[index].imag = sum_data.imag / sum_coeff;
		    }
		} // x
	    } // y
	} // z
      if ( stack->nt > 5 )
	{
	  fprintf(stdout,"\rFinished %d of %d time points for stack `%s'.",
		  jt+1,stack->nt,stack->name);
	  fflush(stdout);
	}
    } // t
      // The following statements caused a memory error on one i386 machine for reasons unknown.
  free_float_vector(coeff_x);
  free_float_vector(coeff_y);
  free_fcomplex_vector(source.SpaceTime.fcData);
  // replace the centered coordinate system using the correct origin.
  stack->origin = Alignment.originT;
  stack->direction = Alignment.directionT;
  stack->ijk_to_xyz = scale_and_shift_to_mat44(stack->resolution,stack->origin,stack->direction);
  stack->xyz_to_ijk = invert_mat44( stack->ijk_to_xyz );
  if ( stack->nt>5 ) printf("\n");
  return(0);
}
