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
#include "util.h"
#include "jip.h"

extern char error_string[LEN_STR];


int average_volumes( char *new_stack_name )
/*
** Average a list of volumes, and put the result in a new stack with the
** name given by `new_stack_name'.
*/
{
  IPOINT3D dim;
  int nt;JIP_INDEX jv;
  FCOMPLEX average;
  struct data_stack *new_stack;
  struct volume_list  *volume;
  extern struct volume_list *volume_list_root;

  /*
  ** Make sure that all volumes have the same dimensions.
  */
  if ( inconsistent_3d_dimensions(&dim,&nt) ) return(1);

  /*
  ** Install the new_stack in memory.
  */
  new_stack = install_stack( new_stack_name, volume_list_root->stack, dim, 1 );

  /*
  ** Begin a loop over each pixel in the new_stack.  Calculate the average
  ** over all volumes in the list for that pixel.
  */
  for ( jv=0; jv<new_stack->n3d; jv++ )
    {
      /* Calculate the mean value across all volumes for the pixel. */
      average.real = average.imag = 0.;
      for (volume=volume_list_root; volume; volume=volume->next)
	{
	  average.real += volume->SpaceTime.fcData[jv].real;
	  average.imag += volume->SpaceTime.fcData[jv].imag;
	}
      average.real /= (float)nt;
      average.imag /= (float)nt;
      /*
      ** Put the average values into the new volume.
      */
      new_stack->SpaceTime.fcData[jv] = average;
    }

  return(0);
}

int average_stacks( char *new_stack_name )
/*
** Average a set of volume stacks, and put the result in a new stack with the
** name given by `new_stack_name'.  Each volume in the average stack will
** be the average of the corresponding volumes in the other stacks.
*/
{
  IPOINT3D dim;
  int nt;JIP_INDEX jv;
  int *npoints;
  struct data_stack *stack, *stack_save, *new_stack;
  struct volume_list  *volume;
  extern struct volume_list *volume_list_root;

  /*
  ** Make sure that all stacks have the same dimensions.
  */
  stack_save = 0;
  for ( volume=volume_list_root; volume; volume=volume->next )
    {
      if ( (stack=volume->stack) == stack_save ) continue;
      stack_save = stack;

      /* Make sure all stacks have the same dimensions. */
      if ( volume == volume_list_root )
	{
	  dim = stack->dim;
	  nt = stack->nt;
	}
      else
	{
	  if ( stack->dim.x != dim.x ||
	       stack->dim.y != dim.y ||
	       stack->dim.z != dim.z ||
	       stack->nt != nt )
	    {
	      sprintf(error_string,"Stacks %s and %s have different dimensions.",
		      volume_list_root->stack->name,stack->name);
	      jip_text(2,error_string);
	      return(1);
	    }
	}
    }

  /*
  ** Install the new_stack in memory.
  */
  new_stack = install_stack( new_stack_name, volume_list_root->stack, dim, nt );
  /*
  ** Set the average stack data to zero for all pixels in the stack.
  */
  for ( jv=0; jv<new_stack->n4d; jv++ )
    new_stack->SpaceTime.fcData[jv].real = new_stack->SpaceTime.fcData[jv].imag = 0.;
  // Keep track of the number of points for averaging.
  // 0 values will be excluded. This eases averaging across registered data sets.
  npoints = int_vector(((JIP_INDEX)(dim.x))*dim.y*dim.z*nt);
  for ( jv=0; jv<new_stack->n4d; jv++ )
    npoints[jv] = 0;
  /*
  ** Begin a loop over each volume in the stack.  Calculate the average for
  ** each pixel across all stacks.
  */
  stack_save = 0;
  for (volume=volume_list_root; volume; volume=volume->next)
    {
      if ( (stack=volume->stack) == stack_save ) continue;
      stack_save = stack;
      for ( jv=0; jv<new_stack->n4d; jv++ )
	{
	  /* Exclude 0 value pixels from the average. */
	  if ( !(stack->SpaceTime.fcData[jv].real==0. && stack->SpaceTime.fcData[jv].real==0.) )
	    {
	      new_stack->SpaceTime.fcData[jv].real += stack->SpaceTime.fcData[jv].real;
	      new_stack->SpaceTime.fcData[jv].imag += stack->SpaceTime.fcData[jv].imag;
	      npoints[jv]++;
	    }
	}  /* end loop over jv */
    }  /* end of loop over volume list */
  /*
  ** Now normalize the average to the number of stacks.
  */
  for ( jv=0; jv<new_stack->n4d; jv++ )
    {
      if ( npoints[jv] != 0 )
	{
	  new_stack->SpaceTime.fcData[jv].real /= (float)(npoints[jv]);
	  new_stack->SpaceTime.fcData[jv].imag /= (float)(npoints[jv]);
	}
    }
  return(0);
}

int average_by_n(char *new_stack_name, int nbin)
{
  IPOINT3D dim;
  int nt, nt_new;
  int jbin, jt;JIP_INDEX jv, index_av;
  struct data_stack *new_stack;
  struct volume_list  *volume;
  extern struct volume_list *volume_list_root;

  /*
  ** Require that nbin > 1.
  */
  if ( nbin < 2 )
    {
      jip_text(2,"Volumes should be averaged in sets of 2 or more.");
      return(1);
    }
  /*
  ** Make sure that all volumes have the same dimensions.
  */
  if ( inconsistent_3d_dimensions(&dim,&nt) ) return(1);
  nt_new = nt / nbin;

  /*
  ** Install the new_stack in memory.
  */
  new_stack = install_stack( new_stack_name, volume_list_root->stack, dim, nt_new );
  /*
  ** Set the average stack data to zero for all pixels in the stack.
  */
  for ( jv=0; jv<new_stack->n4d; jv++ )
    new_stack->SpaceTime.fcData[jv].real = new_stack->SpaceTime.fcData[jv].imag = 0.;

  /*
  ** Loop over each pixel in the volume stack.  For each pixel, step through
  ** the stack computing averages.
  */
  jt = 0;
  for (volume=volume_list_root, jbin=0; volume; volume=volume->next, jbin++)
    {
      if ( jbin == nbin )
	{
	  jt++;
	  jbin = 0;
	}
      if ( jt < nt_new )
	{
	  for (jv=0; jv<new_stack->n3d; jv++)
	    {
	      index_av = jt * new_stack->n3d + jv;
	      new_stack->SpaceTime.fcData[index_av].real += volume->SpaceTime.fcData[jv].real;
	      new_stack->SpaceTime.fcData[index_av].imag += volume->SpaceTime.fcData[jv].imag;
	    }
	}
    }
  /*
  ** Normalize the volume sums to nbin.
  */
  for (jt=0; jt<new_stack->nt; jt++)
    {
      for (jv=0; jv<new_stack->n3d; jv++)
	{
	  index_av = jt*new_stack->n3d + jv;
	  new_stack->SpaceTime.fcData[index_av].real /= (float)nbin;
	  new_stack->SpaceTime.fcData[index_av].imag /= (float)nbin;
	}
    }
  return(0);
}

int average_epochs(char *new_stack_name, int nt_per_epoch)
{
  IPOINT3D dim;
  int nt, nepochs;
  int jt, jt_av, jepoch;JIP_INDEX jv, index_av;
  struct data_stack *new_stack;
  struct volume_list  *volume;
  extern struct volume_list *volume_list_root;

  /*
  ** Require that nepochs > 1.
  */
  if ( nt_per_epoch < 2 )
    {
      jip_text(2,"Volumes should be averaged in sets of 2 or more.");
      return(1);
    }
  /*
  ** Make sure that all volumes have the same dimensions.
  */
  if ( inconsistent_3d_dimensions(&dim,&nt) ) return(1);
  nepochs = nt / nt_per_epoch;

  /*
  ** Install the new_stack in memory.
  */
  new_stack = install_stack( new_stack_name, volume_list_root->stack, dim, nt_per_epoch );
  /*
  ** Set the average stack data to zero for all pixels in the stack.
  */
  for ( jv=0; jv<new_stack->n4d; jv++ )
    new_stack->SpaceTime.fcData[jv].real = new_stack->SpaceTime.fcData[jv].imag = 0.;

  /*
  ** Loop over each pixel in the volume stack.  For each pixel, step through
  ** the stack computing averages.
  */
  for (jt=0, volume=volume_list_root; volume; volume=volume->next, jt++)
    {
      jepoch = jt/nt_per_epoch;
      jt_av  = jt%nt_per_epoch;
      if ( jepoch < nepochs )  /* ignore volumes past the last full epoch */
	{
	  for (jv=0; jv<new_stack->n3d; jv++)
	    {
	      index_av = jt_av*new_stack->n3d + jv;
	      new_stack->SpaceTime.fcData[index_av].real += volume->SpaceTime.fcData[jv].real;
	      new_stack->SpaceTime.fcData[index_av].imag += volume->SpaceTime.fcData[jv].imag;
	    }
	}
    }
  /*
  ** Normalize the volume sums to nepochs.
  */
  for (jv=0; jv<new_stack->n4d; jv++)
    {
      new_stack->SpaceTime.fcData[jv].real /= (float)nepochs;
      new_stack->SpaceTime.fcData[jv].imag /= (float)nepochs;
    }
  return(0);
}

int smooth_volumes( float fwhm, int through_plane ) // fwhm in mm, through_plane -> 3d
{
  FCOMPLEX *temp_data;
  POINT3D weight3d;
  int jx, jy, jz, jx_rel, jy_rel, jz_rel, jx_neighbor, jy_neighbor, jz_neighbor, jt;JIP_INDEX jv;
  int ix_rel, iy_rel, iz_rel;JIP_INDEX index_pixel, index_neighbor;
  float sigma2, dist2, dist, weight, weight_sum;
  float real_sum, imag_sum;
  struct data_stack *stack, *stack_save;
  struct volume_list  *volume;
  extern struct volume_list *volume_list_root;

  if ( fwhm == 0. )
    {
      printf("No spatial filtering - width = 0\n");
      return(0);
    }
  else
    printf("Spatial smoothing using gaussian FWHM of %5.2f mm\n",fwhm);

  sigma2  = 1.44 * fwhm/2 * fwhm/2;
  // Average over a distance such that the relative contribution of
  // voxels at extremes is at least e^-3
  dist = sqrt((double)(3.*sigma2));

  // Cycle through the volume list, beginning with the root.
  stack_save = 0;
  for ( volume=volume_list_root; volume; volume=volume->next )
    {
      if ( (stack=volume->stack) == stack_save ) continue;
      stack_save = stack;
      // Define the limits in x, y, and z
      ix_rel = ceil((double)(dist/stack->resolution.x));
      iy_rel = ceil((double)(dist/stack->resolution.y));
      if ( through_plane )
	iz_rel = ceil((double)(dist/stack->resolution.z));
      else
	iz_rel = 0;
      printf("smooth '%s': (nx,ny,nz,nt) = (%d, %d, %d, %d).\n",
      	     stack->name,stack->dim.x,stack->dim.y,stack->dim.z,stack->nt);
      printf("relative steps: (%d, %d, %d)\n",ix_rel,iy_rel,iz_rel);
      // Allocate space for the new (smoothed) data matrix.
      temp_data = fcomplex_vector(stack->n3d);
      for (jt=0; jt<stack->nt; jt++)
	{
	  for (jz=0; jz<stack->dim.z; jz++)
	    {
	      for (jy=0; jy<stack->dim.y; jy++)
		{
		  for (jx=0; jx<stack->dim.x; jx++)
		    {
		      real_sum = imag_sum = weight_sum = 0.;
		      for (jz_rel=-iz_rel; jz_rel<=iz_rel; jz_rel++)
			{
			  jz_neighbor = jz + jz_rel;
			  dist2 = jz_rel*jz_rel*stack->resolution.z*stack->resolution.z;
			  weight3d.z = exp(-(double)(dist2/sigma2));
			  for (jy_rel=-iy_rel; jy_rel<=iy_rel; jy_rel++)
			    {
			      jy_neighbor = jy + jy_rel;
			      dist2 = jy_rel*jy_rel*stack->resolution.y*stack->resolution.y;
			      weight3d.y = exp(-(double)(dist2/sigma2));
			      for (jx_rel=-ix_rel; jx_rel<=ix_rel; jx_rel++)
				{
				  jx_neighbor = jx + jx_rel;
				  dist2 = jx_rel*jx_rel*stack->resolution.x*stack->resolution.x;
				  weight3d.x = exp(-(double)(dist2/sigma2));
				  if ( jx_neighbor >= 0 && jx_neighbor < stack->dim.x &&
				       jy_neighbor >= 0 && jy_neighbor < stack->dim.y &&
				       jz_neighbor >= 0 && jz_neighbor < stack->dim.z )
				    {
				      index_neighbor = index4d(jx_neighbor,jy_neighbor,jz_neighbor,jt,stack);
				      weight = weight3d.x * weight3d.y * weight3d.z;
				      real_sum   += weight * stack->SpaceTime.fcData[index_neighbor].real;
				      imag_sum   += weight * stack->SpaceTime.fcData[index_neighbor].imag;
				      weight_sum += weight;
				    }
				} // jx_rel
			    } // jy_rel
			} // jz_rel
		      index_pixel = index3d(jx,jy,jz,volume);
		      temp_data[index_pixel].real = real_sum / weight_sum;
		      temp_data[index_pixel].imag = imag_sum / weight_sum;
		    } // jx
		} // jy
	    } // jz
	  /*
	  ** Copy the temporary array back into the stack array and deallocate
	  ** the temporary array.
	  */
	  for (jv=0; jv<stack->n3d; jv++)
	    stack->SpaceTime.fcData[jt*stack->n3d+jv] = temp_data[jv];
	  if ( jt>0 && stack->nt>10 && (4*jt)%stack->nt==0)
	    printf("%d percent done\n",(100*jt)/stack->nt);
	} // jt
      free_fcomplex_vector(temp_data);
    } // stacks
  return(0);
}

int weighted_average( char *new_stack_name )
//
// This "niche" function was written to reduce artifacts in conventional (multi-shot)
// diffusion images in anesthetized mice. Respiration artifacts periodically corrupt images,
// so some data are thrown away.  A better solution needs to found, but until then ...
//
{
  IPOINT3D dim;
  int nt, jz, jt;JIP_INDEX j2d;JIP_INDEX jv, index3d;
  float amp, **sos, weight, weight_sum, av_norm;
  FCOMPLEX average;
  struct data_stack *new_stack;
  struct volume_list  *volume;
  extern struct volume_list *volume_list_root;

  // Make sure that all volumes have the same dimensions.
  if ( inconsistent_3d_dimensions(&dim,&nt) ) return(1);

  if ( nt < 2 )
    {
      printf("Error: the requested number of time points (%d) is < 2!\n",nt);
      return(1);
    }

  // Install the new_stack in memory.
  new_stack = install_stack( new_stack_name, volume_list_root->stack, dim, 1 );

  // Begin a loop over each pixel in the new_stack.  Calculate the average
  // over all volumes in the list for that pixel.
  for ( jv=0; jv<new_stack->n3d; jv++ )
    {
      // Calculate the mean value across all volumes for the pixel.
      average.real = average.imag = 0.;
      for (volume=volume_list_root; volume; volume=volume->next)
	{
	  average.real += volume->SpaceTime.fcData[jv].real;
	  average.imag += volume->SpaceTime.fcData[jv].imag;
	}
      average.real /= (float)nt;
      average.imag /= (float)nt;
      // Put the average values into the new volume.
      new_stack->SpaceTime.fcData[jv] = average;
    }

  // Now, for each slice, compute the sum-of-squares difference with the average.
  sos = float_matrix(dim.z,nt);

  for (jz=0; jz<dim.z; jz++)
    {
      for (jt=0; jt<nt; jt++)
	sos[jz][jt] = 0.;
    }
  for (volume=volume_list_root, jt=0; volume; volume=volume->next, jt++)
    {
      for (jz=0; jz<dim.z; jz++)
	{
	  for (j2d=0; j2d<((JIP_INDEX)(dim.x))*dim.y; j2d++)
	    {
	      index3d = ((JIP_INDEX)(jz))*dim.x*dim.y + j2d;
	      // The amplitude of the new stack can be taken directly from the array, since nt=1
	      amp = sqrt((double)(SQR(new_stack->SpaceTime.fcData[index3d].real) +
				  SQR(new_stack->SpaceTime.fcData[index3d].imag)));
	      sos[jz][jt] += SQR(amp-volume_amp(volume,index3d))/(float)(new_stack->n3d);
	    }
	}
    }
  // Reset the new ("average") stack to zero prior to averaging below.
  for ( jv=0; jv<new_stack->n3d; jv++ )
    new_stack->SpaceTime.fcData[jv].real = new_stack->SpaceTime.fcData[jv].imag = 0.;

  // Normalize all weights to 1.
  av_norm = 0.;
  for (jz=0; jz<dim.z; jz++) // loop over slices
    {
      for (volume=volume_list_root, jt=0; volume; volume=volume->next, jt++)
	av_norm += sos[jz][jt] / (float)(((JIP_INDEX)(dim.z))*nt);
    }
  for (jz=0; jz<dim.z; jz++) // loop over slices
    {
      for (volume=volume_list_root, jt=0; volume; volume=volume->next, jt++)
	{
	  if ( av_norm == 0. )
	    sos[jz][jt] = 0.;
	  else
	    sos[jz][jt] /= av_norm;
	  printf("For slice %d, weight[%d] = %g\n",jz,jt,sos[jz][jt]);
	}
    }

  // Sort sos for each slice, and then average slices using each sorted set.
  for (jz=0; jz<dim.z; jz++) // loop over slices
    {
      for (j2d=0; j2d<((JIP_INDEX)(dim.x))*dim.y; j2d++)
	{
	  index3d = ((JIP_INDEX)(jz))*dim.x*dim.y + j2d;
	  weight_sum = 0;
	  for (volume=volume_list_root, jt=0; volume; volume=volume->next, jt++)
	    {
	      if ( sos[jz][jt] == 0. )
		weight = 0.;
	      else
		weight      = SQR(SQR(1./(double)sos[jz][jt]));
	      weight_sum += weight;
	      new_stack->SpaceTime.fcData[index3d].real += weight * volume->SpaceTime.fcData[index3d].real;
	      new_stack->SpaceTime.fcData[index3d].imag += weight * volume->SpaceTime.fcData[index3d].imag;
	    }
	  if ( weight_sum == 0. )
	    new_stack->SpaceTime.fcData[index3d].real = new_stack->SpaceTime.fcData[index3d].imag = 0.;
	  else
	    {
	      new_stack->SpaceTime.fcData[index3d].real /= weight_sum;
	      new_stack->SpaceTime.fcData[index3d].imag /= weight_sum;
	    }
	  new_stack->SpaceTime.fcData[index3d].real = sqrt((double)
							   (SQR(new_stack->SpaceTime.fcData[index3d].real) +
							    SQR(new_stack->SpaceTime.fcData[index3d].imag)));
	  new_stack->SpaceTime.fcData[index3d].imag = 0.;
	}
    }
  free_float_matrix(sos,dim.z,nt);
  return(0);
}

int create_variances( char *new_stack_name )
// This function is used to estimate individual variance for data where no variance
// can be determined in principle.  Confusing? I know.  It is useful to apply a random-effects
// type of analysis to things like histology, where each individual has a single measurement
// and (unlike fMRI) no within-subject estimate of the variance.  However, histology sometimes
// can have severe artifacts like tearing, so it is useful to attempt to discount big outliers.
//
// Create an "estimate" of the within-subject variance for each subject by doing this:
// 1) find a cross-subject variance from all other subjects (random-effects variance, almost),
// 2) increase this variance IF it the point does not fall within "sigma_group" by 
//    sigma_subject^2 = sigma_group^2 + delta^2, where delta^2 = (value_subject - av_group)^2
{
  IPOINT3D dim;
  int nt, js, js1;JIP_INDEX jv;
  float average, var, delta2;
  struct data_stack *new_stack;
  struct volume_list  *volume;
  extern struct volume_list *volume_list_root;

  // Make sure that all volumes have the same dimensions.
  if ( inconsistent_3d_dimensions(&dim,&nt) ) return(1);

  if ( nt < 3 )
    {
      printf("Use at least 3 time points (not %d) for this function.\n",nt);
      return(1);
    }

  // Install the new_stack in memory.
  new_stack = install_stack( new_stack_name, volume_list_root->stack, dim, nt );

  // Loop over each voxel in the new_stack.
  for ( jv=0; jv<new_stack->n3d; jv++ )
    {
      for (js=0; js<nt; js++)
	{ // loop over subjects
	  // Calculate the mean value across all OTHER volumes for this voxel.
	  average = 0.;
	  for (volume=volume_list_root, js1=0; volume; volume=volume->next, js1++)
	    {
	      if ( js1 != js ) // only include other subjects to find average & stdev for this voxel
		average += volume_amp(volume,jv);
	    }
	  average /= (float)(nt-1);
	  var = 0.;
	  for (volume=volume_list_root, js1=0; volume; volume=volume->next, js1++)
	    {
	      if ( js1 != js ) // only include other subjects to find average & stdev for this voxel
		var += SQR(average-volume_amp(volume,jv));
	      else
		delta2 = SQR(average - volume_amp(volume,jv));
	    }
	  var /= (float)(nt-2);
	  if ( delta2 > var )
	    var += delta2;
	  //	  new_stack->SpaceTime.fcData[jv+js*dim.x*dim.y*dim.z].real = sqrt((double)var);
	  new_stack->SpaceTime.fcData[jv+((JIP_INDEX)(js))*dim.x*dim.y*dim.z].real = 0.1;
	  new_stack->SpaceTime.fcData[jv+((JIP_INDEX)(js))*dim.x*dim.y*dim.z].imag = 0.;
	}
    }

  return(0);
}
