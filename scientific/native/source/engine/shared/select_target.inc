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


/* Shared numerical source: legacy X11 frontend and context-based engine. */
#ifdef JIP_USE_CONTEXT
#define first_time (jip_active->first_target)
#endif

void select_target( int iTime, IPOINT3D TargetDim )
// This function changes the target either in time/overlay or in matrix size (but not both!).
// Template holds the original data at full resolution.
{
  
#ifndef JIP_USE_CONTEXT
  static BOOL first_time = TRUE;
#endif

  IPOINT3D TargetDimOriginal;
  BOOL AdjustedNL;
  JIP_INDEX j;

  TargetDimOriginal = target.dim;
  /////////////////// stacks at target resolution /////////////////////
  if ( !first_time )
    free_float_vector(target.SpaceTime.fData);
  target     = template;
  target.nt  = 1;  // the template may have multiple points, but not the target
  target.n4d = jip_product(target.n3d,target.nt);
  target.SpaceTime.fData = float_vector(target.n4d);

  // Set the target using time point iTime
  for (j=0; j<template.n3d; j++)
    target.SpaceTime.fData[j] = template.SpaceTime.fData[j+iTime*template.n3d];
  // Flatten the target?
  if ( AlignAid.smooth_fov_fraction != 0 )
    flatten_volume( 0, &target );
  // Filter with the overlay mask
  if ( AlignAid.nmask != 0 )
    {
      for (j=0; j<target.n3d; j++)
	{
	  if ( AlignAid.mask[j] == 0 )
	    target.SpaceTime.fData[j] = 0;
	}
    }
  if ( target.dim.x != TargetDim.x ||
       target.dim.y != TargetDim.y ||
       target.dim.z != TargetDim.z )
    downsample_stack(&target,TargetDim); // downsample the template to get the target.
  
  AlignAid.ijk_to_xyz = centered_ijk_to_xyz(target); // target/mimic voxel to space
  
  // The target is defined. Now set other stacks at target resolution.
  if ( !first_time )
    { // Free the other stacks, which may have been down-sampled.
      free_float_vector(mimic.SpaceTime.fData);
      free_float_vector(NL_bases.SpaceTime.fData);
    }
  mimic    = target;
  NL_bases = target;
  mimic.SpaceTime.fData    = float_vector(mimic.n4d);
  NL_bases.SpaceTime.fData = float_vector(NL_bases.n4d);
  // Initialize the bases function
  for (j=0; j<NL_bases.n4d; j++)
    NL_bases.SpaceTime.fData[j] = 0.;
  // The mimic will be filled later by transformation of the source.
  
  AdjustedNL = AlignNL.Adjusted.x || AlignNL.Adjusted.y || AlignNL.Adjusted.z;
  if ( first_time || !AdjustedNL )
    { // distortion fields have not been adjusted
      if ( !first_time )
	{ // free distortion fields, which may been down-sampled.
	  free_float_vector(distort_x.SpaceTime.fData);
	  free_float_vector(distort_y.SpaceTime.fData);
	  free_float_vector(distort_z.SpaceTime.fData);
	  free_float_vector(Jac_deter.SpaceTime.fData);
	}
      distort_x = target;
      distort_y = target;
      distort_z = target;
      Jac_deter = target;
      distort_x.nt  = distort_y.nt  = distort_z.nt  = Jac_deter.nt = 2; // 2nd time point for saved values
      distort_x.n4d = distort_y.n4d = distort_z.n4d = Jac_deter.n4d = jip_product(distort_x.n3d,distort_x.nt);
      distort_x.SpaceTime.fData  = float_vector(distort_x.n4d);
      distort_y.SpaceTime.fData  = float_vector(distort_y.n4d);
      distort_z.SpaceTime.fData  = float_vector(distort_z.n4d);
      Jac_deter.SpaceTime.fData  = float_vector(Jac_deter.n4d);
      for (j=0; j<distort_x.n4d; j++)
	distort_x.SpaceTime.fData[j]=distort_y.SpaceTime.fData[j]=distort_z.SpaceTime.fData[j]= Jac_deter.SpaceTime.fData[j] = 0.;
    }
  else
    { // distortion fields have been adjusted
      if ( TargetDim.x < TargetDimOriginal.x ||
	   TargetDim.y < TargetDimOriginal.y ||
	   TargetDim.z < TargetDimOriginal.z )
	{
	  downsample_stack(&distort_x,TargetDim); // downsample distortion fields
	  downsample_stack(&distort_y,TargetDim);
	  downsample_stack(&distort_z,TargetDim);
	  downsample_stack(&Jac_deter,TargetDim);
	}
      else if ( TargetDim.x > TargetDimOriginal.x ||
		TargetDim.y > TargetDimOriginal.y ||
		TargetDim.z > TargetDimOriginal.z )
	{
	  upsample_stack(&distort_x,TargetDim);   // upsample distortion fields
	  upsample_stack(&distort_y,TargetDim);
	  upsample_stack(&distort_z,TargetDim);
	  upsample_stack(&Jac_deter,TargetDim);
	}
    }
  update_nonlinear_alignment_parameters();

  first_time = FALSE;
}

#ifdef JIP_USE_CONTEXT
#undef first_time
#endif
