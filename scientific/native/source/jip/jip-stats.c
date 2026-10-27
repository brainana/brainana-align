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

extern char error_string[LEN_STR];


int fill_histogram()
{
  FILE *file;
  extern OVERLAY overlay;
  extern float hist_start, hist_bin_size;
  extern int hist_nbins;extern JIP_INDEX *histogram;
  struct volume_list  *volume;
  extern struct volume_list *volume_list_root;
  IPOINT3D dim;
  int nt, ix, iy, iz, ibin, *mask;JIP_INDEX ivoxel, htotal;
  int j;JIP_INDEX jv;
  float value, xbin, sum, sum_weights;

  // Initialize the histogram.
  if ( histogram == NULL )
    {
      printf("Error: no histogram is defined.\n");
      return(1);
    }
  else
    {
      for (j=0; j<hist_nbins; j++)
	histogram[j] = 0;
    }

  // Make sure that all volumes have the same dimensions.
  if ( inconsistent_3d_dimensions(&dim,&nt) ) return(1);

  mask = int_vector(((JIP_INDEX)(dim.x))*dim.y*dim.z);
  // Overlay mask
  for (jv=0; jv<((JIP_INDEX)(dim.x))*dim.y*dim.z; jv++)
    mask[jv] = 0;  // initialize to 0
  for (jv=0; jv<overlay.nvoxels; jv++)
    {
      ix = overlay.voxel[jv].x;
      iy = overlay.voxel[jv].y;
      iz = overlay.voxel[jv].z;
      ivoxel = ((JIP_INDEX)(iz)) * dim.x*dim.y + ((JIP_INDEX)(iy))*dim.x + ix;
      if ( ivoxel < 0 || ivoxel >= ((JIP_INDEX)(dim.x))*dim.y*dim.z )
	{
	  printf("Programming error in glm_histogram_unbiased\n");
	  exit(1);
	}
      // Set the mask to 1 for all voxels in the overlay
      mask[ivoxel] = 1;
    }

  // Loop over all voxels in all volumes.
  for ( volume=volume_list_root, htotal=0;
	volume;
	volume=volume->next )
    {
      for (jv=0; jv<volume->n3d; jv++)
	{
	  // Include only voxels in overlay.
	  if ( mask[jv] == 1 )
	    {
	      value = volume_amp(volume,jv);
	      ibin = (value - hist_start)/hist_bin_size;
	      if ( ibin < 0 ) ibin = 0;
	      if ( ibin >= hist_nbins ) ibin = hist_nbins-1;
	      histogram[ibin]++;
	      htotal++;
	    }
	}
    }
  free_int_vector(mask);

  // Write out the histogram.
  if ( !(file = fopen("hist.dat","w")) )
    {
      printf("Error: cannot open file hist.dat\n");
      return(1);
    }
  fprintf(file,"x y\n");
  for (j=0, sum=sum_weights=0.; j<hist_nbins; j++)
    {
      xbin = hist_start + hist_bin_size/2. + j*hist_bin_size;
      sum += xbin * histogram[j];
      sum_weights += histogram[j];
      fprintf(file,"%g %td\n",xbin,histogram[j]);
    }
  fclose(file);
  printf("Output of %td entries to file hist.dat\n",htotal);
  if ( sum_weights != 0. )
    printf("Average value = %g\n",sum/sum_weights);
  else
    printf("Histogram is empty!\n");
  return(1);
}

int fill_weighted_histogram()
{
  FILE *file;
  extern OVERLAY overlay;
  extern float hist_start, hist_end, hist_bin_size;
  extern int hist_nbins;extern JIP_INDEX *histogram;
  float *fhistogram;
  struct volume_list  *volume;
  extern struct volume_list *volume_list_root;
  IPOINT3D dim;
  int nt, ix, iy, iz, *mask;JIP_INDEX ivoxel;
  int j, jt;JIP_INDEX jv;
  float xbin, sum, sum_weights;

  // Make sure that all volumes have the same dimensions.
  if ( inconsistent_3d_dimensions(&dim,&nt) ) return(1);

  // Set the number of histogram bins equal to the number of time points.
  if ( histogram != NULL )
    free(histogram);
  hist_nbins = nt;
  hist_start = 0.5;
  hist_end   = 0.5 + nt;
  hist_bin_size = 1.;
  histogram  = index_vector(hist_nbins);
  fhistogram = float_vector(hist_nbins);
  printf("Allocate histogram: %g %g %g (%d bins)\n",
	 hist_start,hist_end,hist_bin_size,hist_nbins);
  for (j=0; j<hist_nbins; j++)
    {
      fhistogram[j] = 0.;
      histogram[j] = 0;
    }

  mask = int_vector(((JIP_INDEX)(dim.x))*dim.y*dim.z);
  // Overlay mask
  for (jv=0; jv<((JIP_INDEX)(dim.x))*dim.y*dim.z; jv++)
    mask[jv] = 0;  // initialize to 0
  for (jv=0; jv<overlay.nvoxels; jv++)
    {
      ix = overlay.voxel[jv].x;
      iy = overlay.voxel[jv].y;
      iz = overlay.voxel[jv].z;
      ivoxel = ((JIP_INDEX)(iz)) * dim.x*dim.y + ((JIP_INDEX)(iy))*dim.x + ix;
      if ( ivoxel < 0 || ivoxel >= ((JIP_INDEX)(dim.x))*dim.y*dim.z )
	{
	  printf("Programming error in glm_histogram_unbiased\n");
	  exit(1);
	}
      // Set the mask to 1 for all voxels in the overlay
      mask[ivoxel] = 1;
    }

  printf("# voxels = %td\n",overlay.nvoxels);
  // Loop over all voxels in all volumes.
  for ( volume=volume_list_root, jt=0;
	volume;
	volume=volume->next, jt++ )
    {
      for (jv=0; jv<volume->n3d; jv++)
	{
	  // Include only voxels in overlay.
	  if ( mask[jv] == 1 )
	    fhistogram[jt] += volume_amp(volume,jv)/(float)overlay.nvoxels;
	}
    }
  free_int_vector(mask);

  // Write out the histogram.
  if ( !(file = fopen("hist.dat","w")) )
    {
      printf("Error: cannot open file hist.dat\n");
      return(1);
    }
  fprintf(file,"x y\n");
  for (j=0, sum=sum_weights=0.; j<hist_nbins; j++)
    {
      xbin = hist_start + hist_bin_size/2. + j*hist_bin_size;
      sum += xbin * fhistogram[j];
      sum_weights += fhistogram[j];
      fprintf(file,"%g %f\n",xbin,fhistogram[j]);
    }
  fclose(file);
  if ( sum_weights != 0. )
    printf("Average value = %g\n",sum/sum_weights);
  else
    printf("Histogram is empty!\n");
  free_float_vector(fhistogram);
  return(1);
}

int calculate_overlay_volume_overlap( float smooth_fwhm_mm )
// Calculate the volume overlap (%) for two volumes.
// This function was written to compute overlaps of ROIs, converted in image volumes,
// for the purpose of estimating/removing the amount of correlation due to smoothing in analysis.
{
  struct data_stack *stack1, *stack2;
  struct volume_list *volume1, *volume2;
  extern struct volume_list *volume_list_root;
  IPOINT3D Index1, Index2, dim;
  POINT3D Space0, Space1, Space2, Diff;
  float sigma, amp0, amp1, amp2, amp1_sum, amp2_sum, dist;
  int nt;JIP_INDEX index1, index2;

  // Make sure that all volumes have the same dimensions.
  if ( inconsistent_3d_dimensions(&dim,&nt) ) return(1);

  // There should only be two volumes with matched dimensions.
  if ( nt != 2 )
    {
      printf("Error: use only two volumes (not %d) for this function.\n",nt);
      return(1);
    }
  volume1 = volume_list_root;
  volume2 = volume1->next;
  stack1  = volume1->stack;
  stack2  = volume2->stack;

  sigma = 0.425 * smooth_fwhm_mm;
  amp1_sum = amp2_sum = 0.;

  if ( sigma == 0. )
    {
      printf("Error: the FWHM of the Gaussian smoothing kernal was set to 0!\n");
      return(1);
    }

  for (Index1.z = 0; Index1.z<dim.z; Index1.z++)
    {
      for (Index1.y = 0; Index1.y<dim.y; Index1.y++)
	{
	  for (Index1.x = 0; Index1.x<dim.x; Index1.x++)
	    {
	      index1 = index3d(Index1.x,Index1.y,Index1.z,volume1);
	      amp0   = volume_amp(volume1,index1);
	      Space0 = Convert_Voxel_to_Space( Index1, stack1 );
	      if ( amp0 != 0. ) // this defines the volume of interest
		{ // sum over all voxels to find contributions to this voxel due to smoothing
		  for (Index2.z = 0; Index2.z<dim.z; Index2.z++)
		    {
		      for (Index2.y = 0; Index2.y<dim.y; Index2.y++)
			{
			  for (Index2.x = 0; Index2.x<dim.x; Index2.x++)
			    {
			      index2 = index3d(Index2.x,Index2.y,Index2.z,volume2);
			      amp1   = volume_amp(volume1,index2);
			      amp2   = volume_amp(volume2,index2);
			      if ( amp1 != 0. )
				{ // find how much of volume 1 remains in volume 1 after smoothing
				  Space1 = Convert_Voxel_to_Space( Index2, stack1 );
				  Diff.x = Space1.x - Space0.x;
				  Diff.y = Space1.y - Space0.y;
				  Diff.z = Space1.z - Space0.z;
				  dist = sqrt(Diff.x*Diff.x + Diff.y*Diff.y + Diff.z*Diff.z);
				  amp1_sum += amp1 * Gauss(dist,0.,sigma);
				}
			      if ( amp2 != 0. )
				{ // find how much of volume 2 is smoothed into volume1
				  Space2 = Convert_Voxel_to_Space( Index2, stack2 );
				  Diff.x = Space2.x - Space0.x;
				  Diff.y = Space2.y - Space0.y;
				  Diff.z = Space2.z - Space0.z;
				  dist = sqrt(Diff.x*Diff.x + Diff.y*Diff.y + Diff.z*Diff.z);
				  amp2_sum += amp2 * Gauss(dist,0.,sigma);
				}
			    }
			}
		    }
		}
	    }
	}
    }
  if ( amp1_sum != 0. )
    {
      printf("amp1_sum = %g,amp2_sum = %g\n",amp1_sum,amp2_sum);
      printf("fractional overlap = %g\n",amp2_sum / (amp1_sum + amp2_sum));
    }
  return(0);
}
