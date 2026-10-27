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
#include <string.h>
#include <math.h>
#include "util.h"
#include "jip.h"

extern char error_string[LEN_STR];


void baseline_correct()
{
  int jx, jy, jz, jt, npts;JIP_INDEX j, index;
  LOGICAL test;
  float average_real, average_imag, x, y, mag;
  struct data_stack *stack, *stack_save;
  struct volume_list  *volume;
  extern struct volume_list *volume_list_root;

  /* Cycle through the volume list, beginning with the root. */
  stack_save = 0;
  for ( volume=volume_list_root; volume; volume=volume->next )
    {
      if ( (stack=volume->stack) == stack_save ) continue;
      stack_save = stack;

      /* First determine average values for the real and imaginary channels. */
      average_real = average_imag = 0.;
      npts = 0;
      for (jt=0; jt<stack->nt; jt++)
	{
	  for (jz=0; jz<stack->dim.z; jz++)
	    {
	      for (jy=0; jy<stack->dim.y; jy++)
		{
		  if ( stack->status&SPIRAL_BIT )
		    test = TRUE;
		  else
		    test = jy<stack->dim.y/16 || stack->dim.y-jy <= stack->dim.y/16;
		  if ( test )
		    {
		      for (jx=0; jx<stack->dim.x; jx++)
			{
			  if ( stack->status&SPIRAL_BIT )
			    {
			      x = (jx + 0.5 - stack->dim.x/2)/(float)(stack->dim.x/2);
			      y = (jy + 0.5 - stack->dim.y/2)/(float)(stack->dim.y/2);
			      mag = sqrt((double)(x*x+y*y));
			      test = mag>0.70 && mag<1.0;
			    }
			  else
			    test = jx<stack->dim.x/16 || stack->dim.x - jx
			      <= stack->dim.x/16;
			  if ( test )
			    {
			      /*
			      ** Ignore points that are zero.  These could be spiral points
			      ** outside the k-space radius or padded points.
			      */
			      index = index4d(jx,jy,jz,jt,stack);
			      if ( stack->SpaceTime.fcData[index].real != 0.
			        || stack->SpaceTime.fcData[index].imag != 0. )
				{
				  average_real += stack->SpaceTime.fcData[index].real ;
				  average_imag += stack->SpaceTime.fcData[index].imag ;
				  npts++;
				}
			    }
			}
		    }
		}
	    }
	}
      average_real  /= (float)npts;
      average_imag  /= (float)npts;
      /* Now subtract the average dc offset from all points in the stack. */
      for (j=0; j<stack->n4d; j++)
	{
	  /*
	  ** Ignore points that are zero.  These could be spiral points
	  ** outside the k-space radius or padded points.
	  */
	  if ( stack->SpaceTime.fcData[j].real != 0.
	    || stack->SpaceTime.fcData[j].imag != 0. )
	    {
	      stack->SpaceTime.fcData[j].real -= average_real ;
	      stack->SpaceTime.fcData[j].imag -= average_imag ;
	    }
	}
    }
}

void baseline_correct_volumes()
{
  float average_real, average_imag, x, y, mag;
  LOGICAL test;
  int jx, jy, jz, npts;JIP_INDEX j, index;
  struct data_stack *stack;
  struct volume_list  *volume;
  extern struct volume_list *volume_list_root;

  /* Cycle through the volume list, beginning with the root. */
  for ( volume=volume_list_root; volume; volume=volume->next )
    {
      stack = volume->stack;

      /* First determine average values for the real and imaginary channels. */
      average_real = average_imag = 0.;
      npts = 0;
      for (jz=0; jz<volume->dim.z; jz++)
	{
	  for (jy=0; jy<volume->dim.y; jy++)
	    {
	      if ( stack->status&SPIRAL_BIT )
		test = TRUE;
	      else
		test = jy<volume->dim.y/8 || volume->dim.y-jy<volume->dim.y/8;
	      if ( test )
		{
		  for (jx=0; jx<volume->dim.x; jx++)
		    {
		      if ( stack->status&SPIRAL_BIT )
			{
			  x = (jx + 0.5 - volume->dim.x/2)/(float)(volume->dim.x/2);
			  y = (jy + 0.5 - volume->dim.y/2)/(float)(volume->dim.y/2);
			  mag = sqrt((double)(x*x+y*y));
			  test = mag>0.70 && mag<1.0;
			}
		      else
			test = jx<volume->dim.x/8 || volume->dim.x-jx<volume->dim.x/8;
		      if ( test )
			{
			  /*
			  ** Ignore points that are zero.  These could be spiral points
			  ** outside the k-space radius or padded points.
			  */
			  index = index3d(jx,jy,jz,volume);
			  if ( volume->SpaceTime.fcData[index].real != 0.
			       || volume->SpaceTime.fcData[index].imag != 0. )
			    {
			      average_real  += volume->SpaceTime.fcData[index].real ;
			      average_imag  += volume->SpaceTime.fcData[index].imag ;
			      npts++;
			    }
			}
		    }
		}
	    }
	}
      average_real  /= (float)npts;
      average_imag  /= (float)npts;

      /* Now subtract the average dc offset from all points in the volume. */
      for (j=0; j<volume->n3d; j++)
	{
	  /*
	  ** Ignore points that are zero.  These could be spiral points
	  ** outside the k-space radius or padded points.
	  */
	  if ( volume->SpaceTime.fcData[j].real != 0.
	    || volume->SpaceTime.fcData[j].imag != 0. )
	    {
	      volume->SpaceTime.fcData[j].real -= average_real ;
	      volume->SpaceTime.fcData[j].imag -= average_imag ;
	    }
	}
    }
}

int magnitude_window()
{
  float amp;
  int ix, iy, iz, jx, jy, jz;JIP_INDEX j;
  IPOINT3D dim;
  int nt;JIP_INDEX index;
  float *mask_ovl, mask_default;
  struct data_stack *stack, *stack_save;
  struct volume_list  *volume;
  extern struct volume_list *volume_list_root;
  extern POINT3D filter_range;
  extern OVERLAY overlay;

  /*
  ** Make sure that all stacks have the same dimensions if an overlay is active.
  */
  if ( overlay.nvoxels != 0 && inconsistent_3d_dimensions(&dim,&nt) ) return(1);
  else if ( filter_range.x == filter_range.y )
    jip_text(1,"Zero width window for filter.  Set `filter-range'.");
  
  /*
  ** Set volume data points to zero which fall outside the overlay
  ** or which fall outside the range set by the "filter-range" variable.
  */
  stack_save = 0;
  for ( volume=volume_list_root; volume; volume=volume->next )
    {
      if ( (stack=volume->stack) != stack_save )
	{
	  if ( volume != volume_list_root ) free_float_vector(mask_ovl);
	  /*
	  ** Allocate a mask volume to allow masking by an volume overlay.
	  */
	  mask_ovl = float_vector(volume->n3d);
	  if ( overlay.nvoxels != 0 || filter_range.x==filter_range.y )           /* Overlay active */
	    mask_default = 0.;               /* All off but overlay */
	  else
	    mask_default = 1.;               /* All on */
	  /* Set a default mask. */
	  for (j=0; j<volume->n3d; j++)
	    mask_ovl[j] = mask_default;
	  /* Now set a mask from an overlay if one is active. */
	  if ( overlay.nvoxels != 0 )
	    {
	      for (j=0; j<overlay.nvoxels; j++)
		{
		  ix = overlay.voxel[j].x;
		  iy = overlay.voxel[j].y;
		  iz = overlay.voxel[j].z;
		  index = index3d(ix,iy,iz,volume);
		  if ( index > volume->n3d )
		    {
		      printf("(x,y,z) = (%d,%d,%d), index = %td, total = %td\n",
			     ix,iy,iz,index,volume->n3d);
		      jip_text(2,"The overlay falls outside the volume boundaries.");
		      return(1);
		    }
		  else
		    mask_ovl[index] = overlay.weight[j];
		}
	    }
	}
      stack_save = stack;
      for (j=0; j<volume->n3d; j++)      /*  loop over pixels   */
	{
	  if ( stack->status & MAP_BIT )
	    amp = volume->SpaceTime.fcData[j].real;
	  else
	    amp = volume_amp(volume,j);
	  if ( mask_ovl[j]==0. || (filter_range.x!=filter_range.y
				  && (amp<filter_range.x || amp>filter_range.y)) )
	    {
	      volume->SpaceTime.fcData[j].real = 0.;
	      volume->SpaceTime.fcData[j].imag = 0.;
	      if ( overlay.nvoxels != 0 ) mask_ovl[j] = 0.; /* If an overlay is active, modify it. */
	    }
	}  /* end loop over pixels */
      /*
      ** If an overlay was active, it has been modified.  Update the overlay.
      */
      if ( overlay.nvoxels != 0 )
	{
	  free_ipoint3d_vector(overlay.voxel);
	  overlay.nvoxels = 0;
	  /* Count the number of pixels now set. */
	  for (j=0; j<volume->n3d; j++)
	    if ( mask_ovl[j] != 0. ) overlay.nvoxels++;
	  if ( overlay.nvoxels != 0 )
	    {
	      overlay.voxel = ipoint3d_vector(overlay.nvoxels);
	      overlay.nvoxels = 0;
	      for (jz=0; jz<volume->dim.z; jz++)
		{
		  for (jy=0; jy<volume->dim.y; jy++)
		    {
		      for (jx=0; jx<volume->dim.x; jx++)
			{
			  index = index3d(jx,jy,jz,volume);
			  if ( mask_ovl[index] != 0. )
			    {
			      overlay.voxel[overlay.nvoxels].x = jx;
			      overlay.voxel[overlay.nvoxels].y = jy;
			      overlay.voxel[overlay.nvoxels].z = jz;
			      overlay.weight[overlay.nvoxels] = mask_ovl[index];
			      overlay.nvoxels++;
			    }
			}
		    }
		}
	    }
	}
    }
  free_float_vector(mask_ovl);
  return(0);
}

int magnitude_window_together()
{
  float amp;
  IPOINT3D dim;
  int ix, iy, iz, nt, jx, jy, jz;JIP_INDEX n3d, jv;
  int nonzero_pixel, mask_default;JIP_INDEX index;
  short *mask_ovl;
  struct volume_list  *volume;
  extern struct volume_list *volume_list_root;
  extern POINT3D filter_range;
  extern OVERLAY overlay;

  if ( filter_range.x == filter_range.y )
    {
      jip_text(2,"Zero width window for filter.  Set `filter-range'.");
      return(1);
    }
  /*
  ** Make sure that all volumes have the same dimensions.
  */
  if ( inconsistent_3d_dimensions(&dim,&nt) ) return(1);
  volume = volume_list_root;
  n3d = volume->n3d;
  /*
  ** Allocate a mask volume to allow masking by an volume overlay.
  */
  mask_ovl = short_vector(n3d);
  if ( overlay.nvoxels != 0 )           /* Overlay active */
    mask_default = 0;               /* All off but overlay */
  else
    mask_default = 1;               /* All on */
  /* Set a default mask. */
  for (jv=0; jv<n3d; jv++)
    mask_ovl[jv] = mask_default;
  /* Now set a mask from an overlay if one is active. */
  if ( overlay.nvoxels != 0 )
    {
      for (jv=0; jv<overlay.nvoxels; jv++)
	{
	  ix = overlay.voxel[jv].x;
	  iy = overlay.voxel[jv].y;
	  iz = overlay.voxel[jv].z;
	  index = index3d(ix,iy,iz,volume);
	  if ( index > n3d )
	    {
	      jip_text(2,"The overlay falls outside the volume boundaries.");
	      free_short_vector(mask_ovl);
	      return(1);
	    }
	  else
	    mask_ovl[index] = 1;
	}
    }

  for (jv=0; jv<n3d; jv++)      /*  loop over pixels   */
    {
      nonzero_pixel = 0;
      for ( volume=volume_list_root; volume; volume=volume->next )
	{
	  if ( volume->stack->status & MAP_BIT )
	    amp = volume->SpaceTime.fcData[jv].real;
	  else
	    amp = volume_amp(volume,jv);
	  nonzero_pixel = nonzero_pixel || (amp>filter_range.x && amp<filter_range.y);
	}
      if ( mask_ovl[jv]==0 || !nonzero_pixel )
	{
	  for ( volume=volume_list_root; volume; volume=volume->next )
	    volume->SpaceTime.fcData[jv].real = volume->SpaceTime.fcData[jv].imag = 0.;
	  if ( overlay.nvoxels != 0 ) mask_ovl[jv] = 0;
	}
    }  /* end loop over pixels */
  /*
  ** If an overlay was active, it has been modified.  Update the overlay.
  */
  volume = volume_list_root;
  if ( overlay.nvoxels != 0 )
    {
      free_ipoint3d_vector(overlay.voxel);
      overlay.nvoxels = 0;
      /* Count the number of pixels now set. */
      for (jv=0; jv<n3d; jv++)
	if ( mask_ovl[jv] == 1 ) overlay.nvoxels++;
      if ( overlay.nvoxels != 0 )
	{
	  overlay.voxel = ipoint3d_vector(overlay.nvoxels);
	  overlay.nvoxels = 0;
	  for (jz=0; jz<volume->dim.z; jz++)
	    {
	      for (jy=0; jy<volume->dim.y; jy++)
		{
		  for (jx=0; jx<volume->dim.x; jx++)
		    {
		      index = index3d(jx,jy,jz,volume);
		      if ( mask_ovl[index] == 1 )
			{
			  overlay.voxel[overlay.nvoxels].x = jx;
			  overlay.voxel[overlay.nvoxels].y = jy;
			  overlay.voxel[overlay.nvoxels].z = jz;
			  overlay.nvoxels++;
			}
		    }
		}
	    }
	}
    }
  free_short_vector(mask_ovl);
  return(0);
}

int scale_volumes(float target_max)
{
  float amp, max, tmax;
  IPOINT3D dim;
  int ix, iy, iz, nt;JIP_INDEX index;
  int jx, jy, jz;JIP_INDEX jv;
  short *mask_ovl;
  struct volume_list  *volume;
  extern struct volume_list *volume_list_root;
  extern OVERLAY overlay;

  /*
  ** Make sure that all stacks have the same dimensions if an overlay is active.
  */
  if ( overlay.nvoxels != 0 )
    {
      if ( inconsistent_3d_dimensions(&dim,&nt) ) return(1);
      /*
      ** Allocate a mask volume to allow masking by an volume overlay.
      */
      volume   = volume_list_root;
      mask_ovl = short_vector(volume->n3d);
      /* Set a default mask. */
      for (jv=0; jv<volume->n3d; jv++)
	mask_ovl[jv] = 0;
      /* Now set a mask from an overlay if one is active. */
      for (jv=0; jv<overlay.nvoxels; jv++)
	{
	  ix = overlay.voxel[jv].x;
	  iy = overlay.voxel[jv].y;
	  iz = overlay.voxel[jv].z;
	  index = index3d(ix,iy,iz,volume);
	  if ( index >= volume->n3d )
	    {
	      jip_text(2,"The overlay falls outside the volume boundaries.");
	      free_short_vector(mask_ovl);
	      return(1);
	    }
	  else
	    mask_ovl[index] = 1;
	}
    }
  /*
  ** Cycle through the volume list, beginning with the root.  Find the maximum
  ** pixel amplitude for all volumes in the list.
  */
  max = 0.;
  for ( volume=volume_list_root; volume; volume=volume->next )
    {
      for (jz=0; jz<volume->dim.z; jz++)
	{
	  for (jy=0; jy<volume->dim.y; jy++)
	    {
	      for (jx=0; jx<volume->dim.x; jx++)
		{
		  index = index3d(jx,jy,jz,volume);
		  amp=volume_amp(volume,index);
		  amp = sqrt((double)(amp*amp));
		  if ( overlay.nvoxels != 0 )
		    {
		      if ( mask_ovl[index]==1 && amp>max ) max = amp;
		    }
		  else
		    {
		      if ( amp>max ) max = amp;
		    }
		}
	    }
	}
    }
  if ( overlay.nvoxels != 0 ) free_short_vector(mask_ovl);
  /*
  ** Now rescale all pixel values so that the maximum pixel amplitude is `target_max'.
  */
  if ( target_max == 0. )
    tmax = 32767.;
  else
    tmax = target_max;
  printf("Scaling factor = %f\n",tmax/max);
  for ( volume=volume_list_root; volume; volume=volume->next )
    {
      for (jv=0; jv<volume->n3d; jv++)
	{
	  volume->SpaceTime.fcData[jv].real *= tmax / max;
	  volume->SpaceTime.fcData[jv].imag *= tmax / max;
	}
    }
  return(0);
}

int normalize_volumes(float target_average)
{
  float amp, average, tav;
  IPOINT3D dim;
  int ix, iy, iz, nt, icount;JIP_INDEX index;
  int jx, jy, jz;JIP_INDEX jv;
  short *mask_ovl;
  struct volume_list  *volume;
  extern struct volume_list *volume_list_root;
  extern OVERLAY overlay;

  /*
  ** Make sure that all stacks have the same dimensions if an overlay is active.
  */
  if ( overlay.nvoxels != 0 )
    {
      if ( inconsistent_3d_dimensions(&dim,&nt) ) return(1);
      /*
      ** Allocate a mask volume to allow masking by an volume overlay.
      */
      volume   = volume_list_root;
      mask_ovl = short_vector(volume->n3d);
      /* Set a default mask. */
      for (jv=0; jv<volume->n3d; jv++)
	mask_ovl[jv] = 0;
      /* Now set a mask from an overlay if one is active. */
      for (jv=0; jv<overlay.nvoxels; jv++)
	{
	  ix = overlay.voxel[jv].x;
	  iy = overlay.voxel[jv].y;
	  iz = overlay.voxel[jv].z;
	  index = index3d(ix,iy,iz,volume);
	  if ( index > volume->n3d )
	    {
	      jip_text(2,"The overlay falls outside the volume boundaries.");
	      free_short_vector(mask_ovl);
	      return(1);
	    }
	  else
	    mask_ovl[index] = 1;
	}
    }
  /*
  ** Cycle through the volume list, beginning with the root.  Find the maximum
  ** pixel amplitude for all volumes in the list.
  */
  average = 0.;
  icount = 0;
  for ( volume=volume_list_root; volume; volume=volume->next )
    {
      for (jz=0; jz<volume->dim.z; jz++)
	{
	  for (jy=0; jy<volume->dim.y; jy++)
	    {
	      for (jx=0; jx<volume->dim.x; jx++)
		{
		  index = index3d(jx,jy,jz,volume);
		  amp=volume_amp(volume,index);
		  amp = sqrt((double)(amp*amp));
		  if ( overlay.nvoxels != 0 )
		    {
		      if ( mask_ovl[index]==1 )
			{
			  average += amp;
			  icount++;
			}
		    }
		  else
		    {
		      average += amp;
		      icount++;
		    }
		}
	    }
	}
    }
  if ( overlay.nvoxels != 0 ) free_short_vector(mask_ovl);
  average /= (float)icount;
  /*
  ** Now rescale all pixel values so that the average pixel amplitude is `target_average'.
  */
  if ( target_average == 0. )
    tav = 10000.;
  else
    tav = target_average;
  printf("Scaling factor = %f\n",tav/average);
  for ( volume=volume_list_root; volume; volume=volume->next )
    {
      for (jv=0; jv<volume->n3d; jv++)
	{
	  volume->SpaceTime.fcData[jv].real *= tav/average;
	  volume->SpaceTime.fcData[jv].imag *= tav/average;
	}
    }
  return(0);
}
