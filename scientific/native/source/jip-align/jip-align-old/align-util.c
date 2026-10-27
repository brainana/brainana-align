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
#include "align.h"


void fill_TimeSeriesModel( IPOINT3D voxel )
{
}

void plot_TimeSeriesModel( POINT2D *Scale )
{
}

void increment_graph_location_time()
{
}

void decrement_graph_location_time()
{
}

void rescale_image_settings()
{
  if ( AlignAid.DisplayType == 0 )
    {
      individual_scale_min = image_scale_min;
      individual_scale_max = image_scale_max;
    }
  else if ( AlignAid.DisplayType == 1 )
    {
      template_scale_min = image_scale_min;
      template_scale_max = image_scale_max;
    }
  else
    {
      distort_scale_min = image_scale_min;
      distort_scale_max = image_scale_max;
    }
}

void initialize_alignment()
{
  int itest, ipower, j;
  float resolution;

  graph.iTime = iTime_Template = 0; // time indices for individual/original & template

  AlignAid.DisplayType    = 0;      // 0 = individual, 1 = template
  AlignAid.DisplaySet     = 0;      // 0=orientation, 1=affine, 2=2nd-order NL, 3=3rd-order NL
  AlignAid.iParameter     = 0;      // index in alignment array
  AlignAid.iFirstParShown = 0;
  AlignAid.iLastParShown  = 9;      // 8 total: 4 orient, 3 downsample, 1 rolloff, 1 overlay
  AlignAid.ChangeValue    = TRUE;   // for interaction: 0 = change increment, 1 = change value of parameters
  AlignAid.ChangeValue    = TRUE;   // for interaction: 0 = change increment, 1 = change value of parameters
  Alignment.RigidBody     = TRUE;   // fix this; use "deck-of-cards" only for data sets with 1 or 2 slices
  Alignment.OrientRotation.x = Alignment.OrientRotation.y = Alignment.OrientRotation.z = 0;
  Alignment.OrientSwapZ   = 0;      // swap Z; changes parity of coordinate system (RHC<-->LHC)
  AlignAid.SubSampleSetting.x = AlignAid.SubSampleSetting.y = AlignAid.SubSampleSetting.z = 1;
  AlignAid.SubSample.x        = AlignAid.SubSample.y        = AlignAid.SubSample.z        = 1;
  AlignAid.LowResolution  = FALSE;
  AlignAid.CostFunction   = 0;
  AlignAid.smooth_fov_fraction = 0.;
  Alignment.origin       = template_unfiltered.origin;
  Alignment.direction    = template_unfiltered.direction;

  calculate_orientation_rotation();
  // Alignment names: Affine (12 total)
  sprintf(AlignAid.name[0],"  x");
  sprintf(AlignAid.name[1],"  y");
  sprintf(AlignAid.name[2],"  z");
  sprintf(AlignAid.name[3],"phi_xy");
  sprintf(AlignAid.name[4],"phi_xz");
  sprintf(AlignAid.name[5],"phi_yz");
  sprintf(AlignAid.name[6],"size-x");
  sprintf(AlignAid.name[7],"size-y");
  sprintf(AlignAid.name[8],"size-z");
  sprintf(AlignAid.name[9],"skew_xy");
  sprintf(AlignAid.name[10],"skew_xz");
  sprintf(AlignAid.name[11],"skew_yz");
  // Alignment names for non-linear sets
  sprintf(AlignAid.name[12],"X field");
  sprintf(AlignAid.name[13],"Y field");
  sprintf(AlignAid.name[14],"Z field");

  for (j=0; j<NAFFINE; j++)
    {
      if ( j>=6 && j<9 ) // inflations
	Alignment.value[j] = 1.;
      else // set all other values to zero
	Alignment.value[j] = 0.;

      // begin by showing all 12 affine parameters
      if ( j < NAFFINE )
	AlignAid.Adjust[j] = 1;
    }
  for (j=0; j<NAFFINE; j++)
    AlignAid.saved_value[j] = Alignment.value[j];

  // Increments:
  action_set_increments(2);
  // Set the alignment matrix, origin, and resolution
  // based upon the template.
  Alignment.resolution = template_unfiltered.resolution;
  Alignment.dimension  = template_unfiltered.dim;

  // Sub-sampling: assume that x (readout) has the best "anatomical resolution",
  // or number of pixels, in the original data.
  // dimension dimension/32  2^n subsample=2*n
  //    32        1           0    1
  //    64        2           1    2
  //   128        4           2    4
  //   256        8           3    6
  //   512       16           4    8
  itest  = original_unfiltered.dim.x;

  itest /= 32;
  ipower = 0;
  while ( itest > 1 )
    {
      itest/=2;
      ipower++;
    }
  if ( ipower == 0 )
    AlignAid.SubSampleSetting.x = 1;
  else
    AlignAid.SubSampleSetting.x = MIN(8,2*ipower);
  resolution = original_unfiltered.resolution.x * AlignAid.SubSampleSetting.x;
  AlignAid.SubSampleSetting.y = rint((double)(resolution /original_unfiltered.resolution.y));
  AlignAid.SubSampleSetting.z = rint((double)(resolution /original_unfiltered.resolution.z));
  AlignAid.SubSampleSetting.y = MAX(1,AlignAid.SubSampleSetting.y);
  AlignAid.SubSampleSetting.z = MAX(1,AlignAid.SubSampleSetting.z);

  // mask for template
  AlignAid.nmask = 0; // means no values currently set
  AlignAid.mask  = float_vector(template_unfiltered.n3d);
  for (j=0; j<template_unfiltered.n3d; j++)
    AlignAid.mask[j] = 0;

  /////////////////////////// Non-linear alignment /////////////////////////
  //
  AlignNL.Adjust.x     = AlignNL.Adjust.y      = AlignNL.Adjust.z      = TRUE;
  AlignNL.Adjusted.x   = AlignNL.Adjusted.y    = AlignNL.Adjusted.z    = FALSE;
  AlignNL.ViewBasis    = FALSE;
  AlignNL.Lambda_reg   = 5;
  AlignNL.Jacob_limits = 5;
  AlignNL.stiffness    = 0.;
  // Set the basis function down-sampling based upon a single down-sampling factor.
  SetGridDown( 2 );
  update_nonlinear_alignment_parameters();
}

void SetGridDown( int iGridDown )
// The goal is to get uniform resolution for initial steps, so that low-res basis functions
// are not collapsed too far down in any dimension.
{
  float resolution;

  AlignNL.iGridDown = iGridDown;
  resolution  = MIN(MIN(template.resolution.x,template.resolution.y),template.resolution.z);
  AlignNL.GridDown.x = MAX(1,rint((double)(iGridDown*resolution/template.resolution.x)));
  AlignNL.GridDown.y = MAX(1,rint((double)(iGridDown*resolution/template.resolution.y)));
  AlignNL.GridDown.z = MAX(1,rint((double)(iGridDown*resolution/template.resolution.z)));
}

void point_mosaic_to_stack_data()
{
  int iTime;

  // set the data pointer
  if ( AlignAid.DisplayType == 0 )      // individual
    {
      mosaic.stack    = &individual;
      image_scale_min = individual_scale_min;
      image_scale_max = individual_scale_max;
      iTime = graph.iTime;
    }
  else if ( AlignAid.DisplayType == 1 ) // template
    {
      mosaic.stack    = &template;
      image_scale_min = template_scale_min;
      image_scale_max = template_scale_max;
      iTime = iTime_Template;
    }
  else
    {
      iTime = 0;
      if ( AlignAid.DisplayType == 2 )
	mosaic.stack    = &distort_x;
      else if ( AlignAid.DisplayType == 3 )
	mosaic.stack    = &distort_y;
      else if ( AlignAid.DisplayType == 4 )
	mosaic.stack    = &distort_z;
      else if ( AlignAid.DisplayType == 5 )
	{
	  mosaic.stack  = &NL_bases;
	  if ( AlignNL.ViewBasis )
	    create_basis_function_image();
	}
      image_scale_min = distort_scale_min;
      image_scale_max = distort_scale_max;
    }
  mosaic.fSpaceMatrix = &(mosaic.stack->SpaceTime.fData[iTime*mosaic.stack->n3d]);
}

void update_alignment( BOOL most_accurate )
{
  if ( AlignAid.LowResolution )
    transform_coordinates(1,0);
  else
    transform_coordinates(1,most_accurate);
  build_image_display(1);
}

void rescale_activation()
{
  float threshold_low, threshold_high, temp;
  int j;

  threshold_high = -1.e20;
  threshold_low = 1.e20;
  for (j=0; j<mosaic.stack->n3d; j++)
    {
      if ( mosaic.activation.fSpaceMatrix[j] < threshold_low )
	threshold_low = mosaic.activation.fSpaceMatrix[j];
      if ( mosaic.activation.fSpaceMatrix[j] > threshold_high )
	threshold_high = mosaic.activation.fSpaceMatrix[j];
    }
  // Make sure the the thresholds make sense
  if ( fabs((double)threshold_low) < fabs((double)threshold_high) )
    {
      threshold_low  = fabs((double)threshold_low);
      threshold_high = fabs((double)threshold_high);
    }
  else
    {
      temp = fabs((double)threshold_low);
      threshold_low  = fabs((double)threshold_high);
      threshold_high = temp;
    }

}

void convert_activation_scale( struct file_list *this_file )
{
  float conversion=0.4343; // log10 of e
  int offset_map, jmap, j;

  for (jmap=this_file->start_image; jmap<=this_file->end_image; jmap++)
    {
      offset_map = jmap * mosaic.activation.stack->n3d;
      for (j=0; j<mosaic.activation.stack->n3d; j++)
	mosaic.activation.stack->SpaceTime.fData[j+offset_map] *= conversion;
    }
}

void update_display_set()
{
  if ( AlignAid.DisplaySet == 0 )
    { // Orientation
      AlignAid.iFirstParShown = 0;
      AlignAid.iLastParShown  = 9;
    }
  else if ( AlignAid.DisplaySet == 1 )
    { // affine
      AlignAid.iFirstParShown = 0;
      AlignAid.iLastParShown  = NAFFINE-1;
    }
  else if ( AlignAid.DisplaySet == 2 )
    { // non-linear X
      AlignAid.iFirstParShown = 0;
      AlignAid.iLastParShown  = 7;
    }
}

void smooth_or_sharpen_isotropic( int sharpen, POINT3D fwhm, struct data_stack *stack )
     // fwhm is in units of mm (or whatever spatial units are used)
{
  POINT3D resolution, fov2, sigma2, sigma2_k;
  float x, y, z, scale, amp, phase;
  int jx, jy, jz, jt, jp, npts, index;
  struct data_stack stack_temp;

  stack_temp.dim = stack->dim;
  stack_temp.n2d = stack->n2d;
  stack_temp.n3d = stack->n3d;
  stack_temp.n4d = stack->n4d;
  stack_temp.nt  = stack->nt;
  stack_temp.resolution = stack->resolution;
  npts = stack->n4d;
  stack_temp.SpaceTime.fcData = fcomplex_vector(npts);
  for (jp=0; jp<npts; jp++)
    {
      stack_temp.SpaceTime.fcData[jp].real = stack->SpaceTime.fData[jp];
      stack_temp.SpaceTime.fcData[jp].imag = 0.;
    }

  // inverse FT in x, y, and z
  ft_1d( 1, -1, &stack_temp ); // x
  ft_1d( 2, -1, &stack_temp ); // y
  ft_1d( 3, -1, &stack_temp ); // z

  // Determine the 3D sigma_k
  // Multiply data by a k-space filter
  sigma2_k.x = 2.355*2.355/4./PI/PI /fwhm.x/fwhm.x;
  sigma2_k.y = 2.355*2.355/4./PI/PI /fwhm.y/fwhm.y;
  sigma2_k.z = 2.355*2.355/4./PI/PI /fwhm.z/fwhm.z;

  // Smooth data by multiplication in k-space
  for ( jt=0; jt<stack->nt; jt++ )
    {
      // Work on 1 volume at a time
      resolution = stack->resolution;
      fov2.x = resolution.x*resolution.x * stack->dim.x*stack->dim.x;
      fov2.y = resolution.y*resolution.y * stack->dim.y*stack->dim.y;
      fov2.z = resolution.z*resolution.z * stack->dim.z*stack->dim.z;
      sigma2.x = sigma2_k.x * fov2.x; // use normalized fov in each dimension
      sigma2.y = sigma2_k.y * fov2.y;
      sigma2.z = sigma2_k.z * fov2.z;
      for ( jz=0; jz<stack->dim.z; jz++ ) // these are steps in k-space
	{
	  z = jz - stack->dim.z/2 + 0.5;
	  for ( jy=0; jy<stack->dim.y; jy++ )
	    {
	      y = jy - stack->dim.y/2 + 0.5;
	      for ( jx=0; jx<stack->dim.x; jx++ )
		{
		  x = jx - stack->dim.x/2 + 0.5;
		  scale
		    = exp(-(double)(x*x/2/sigma2.x))
		    * exp(-(double)(y*y/2/sigma2.y))
		    * exp(-(double)(z*z/2/sigma2.z));
		  if ( sharpen )
		    scale = 1. - scale; // switch low-pass to high-pass
		  index = index4d_stack(jx,jy,jz,jt,&stack_temp);
		  stack_temp.SpaceTime.fcData[index].real *= scale;
		  stack_temp.SpaceTime.fcData[index].imag *= scale;
		}
	    }
	}
    }

  // forward FT in x, y, and z
  ft_1d( 1, 1, &stack_temp ); // x
  ft_1d( 2, 1, &stack_temp ); // y
  ft_1d( 3, 1, &stack_temp ); // z

  // The data should be real, but make sure here.
  for ( jp=0; jp<npts; jp++ )
    {
      amp   = compute_amplitude(stack_temp.SpaceTime.fcData[jp]);
      phase = compute_phase(stack_temp.SpaceTime.fcData[jp]);
      //      if ( phase < 0. ) amp = -amp;
      //      stack->SpaceTime.fData[jp] = amp;
      stack->SpaceTime.fData[jp] = stack_temp.SpaceTime.fcData[jp].real;
    }
  free_fcomplex_vector(stack_temp.SpaceTime.fcData);
}

int ft_1d( int idim, int forward, struct data_stack *stack )
{
  // idim: 1 -> x,
  //       2 -> y,
  //       3 -> z
  // forward: 1 -> forward  (k-space to image space)
  //         -1 -> backward (image space to k-space)
  fftw_plan plan_ft1d;
  fftw_complex *fftwData;
  FCOMPLEX *Data1d;
  float div;
  int lInner, lOuter1, lOuter2, direction;
  int j1, j2, jIn, jt, index;

  if ( forward == 1 )
    direction = FFTW_FORWARD;
  else
    direction = FFTW_BACKWARD;
  
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
  if ( forward == 1 )
    div = (float)lInner;
  else
    div = 1.;

  // Allocate the 1d arrays
  Data1d   = fcomplex_vector(lInner); // floats
  fftwData = (fftw_complex *)fftw_malloc(sizeof(fftw_complex)*lInner); // doubles
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
		    index = index4d_stack(jIn,j1,j2,jt,stack);
		  else if ( idim == 2 )
		    index = index4d_stack(j1,jIn,j2,jt,stack);
		  else if ( idim == 3 )
		    index = index4d_stack(j1,j2,jIn,jt,stack);
		  Data1d[jIn] = stack->SpaceTime.fcData[index];
		}
	      // On the 1st line of data, define the FT "plan".
	      if ( jt == 0 && j1==0 && j2==0 )
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
		    index = index4d_stack(jIn,j1,j2,jt,stack);
		  else if ( idim == 2 )
		    index = index4d_stack(j1,jIn,j2,jt,stack);
		  else if ( idim == 3 )
		    index = index4d_stack(j1,j2,jIn,jt,stack);
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

  return(0);
}

int index4d_stack(int ix, int iy, int iz, int it, struct data_stack *stack)
{
  if ( ix < 0 || ix >= stack->dim.x ||
       iy < 0 || iy >= stack->dim.y || 
       iz < 0 || iz >= stack->dim.z ||
       it < 0 || it >= stack->nt )
    {
      printf("Programming error: index out of range in index4d_stack.\n");
      printf("(x,y,z,t) = (%d,%d,%d,%d)\n",ix,iy,iz,it);
      exit(1);
    }
  return(it*stack->n3d + iz*stack->n2d + iy*stack->dim.x + ix);
}
