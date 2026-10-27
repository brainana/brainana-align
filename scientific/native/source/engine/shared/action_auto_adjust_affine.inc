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
#define first_time (jip_active->first_affine)
#endif

void action_auto_adjust_affine( int button )
{
#ifdef JIP_USE_CONTEXT
  JIP_CHECKPOINT();
#endif

  
#ifndef JIP_USE_CONTEXT
  static BOOL first_time=TRUE;
#endif

  BOOL AdjustSave[NAFFINE], AdjustSet, MultiResolution;
  int j;

  if ( AlignAid.DisplaySet != 1 )
    {
      AlignAid.DisplaySet = 1;
      update_display_set();
      update_alignment(0);
    }
  MultiResolution = (button == 1);

  // Check for the rolloff filter.
  if (AlignAid.smooth_fov_fraction == 0 )
    printf("\nThe rolloff filter is not applied. Is this a mistake??\n");

  // The first time, adjust affine parameters in a hierarchical way.
  if ( first_time )
    {
      // Apply the overlay filter if an overlay is active.
      if ( mosaic.toggle_overlays && ovl_root.npts != 0 )
	{
	  printf("\nApply the overlay filter to remove extra-cerebral tissue.\n");
	  action_filter_overlay();
	}
      else
	{
	  printf("An overlay is not active.\n");
	  printf("This is OK if the template has already been skull-stripped.\n");
	  printf("Otherwise, use a brain overlay before this step.\n");
	}

      // Save the set of parameter selections.
      for (j=0; j<NAFFINE; j++)
	AdjustSave[j] = AlignAid.Adjust[j];

      // Start from scratch, using 3 parameters (shifts)
      AlignAid.iParameter = 0;
      for (j=0; j<NAFFINE; j++)
	{
	  AlignAid.Adjust[j] = 0;
	  if ( j < 3 && AdjustSave[j] )
	    AlignAid.Adjust[j] = 1;
	}
      for (j=0, AdjustSet=FALSE; j<NAFFINE; j++)
	AdjustSet |= AlignAid.Adjust[j];
      if ( AdjustSet )
	{
	  AlignAid.DisplaySet = 1;
	  if ( MultiResolution &&
	       AlignAid.SubSampleSetting.x *
	       AlignAid.SubSampleSetting.y *
	       AlignAid.SubSampleSetting.z != 1 )
	    action_change_resolution();
	  update_alignment(0);
	  auto_adjust_affine_iterations(3,3);
	}

      // Use 6 parameters (rigid body)
      AlignAid.iParameter = 0;
      update_alignment(0);
      for (j=0; j<NAFFINE; j++)
	{
	  AlignAid.Adjust[j] = 0;
	  if ( j < 6 && AdjustSave[j] )
	    AlignAid.Adjust[j] = 1;
	}
      for (j=0, AdjustSet=FALSE; j<NAFFINE; j++)
	AdjustSet |= AlignAid.Adjust[j];
      if ( AdjustSet && MultiResolution &&
	   AlignAid.SubSample.x*AlignAid.SubSample.y*AlignAid.SubSample.z != 1 )
	{ // Multi-resolution
	  auto_adjust_affine_iterations(3,2);
	  AlignAid.SubSampleSetting.x = MAX(1,AlignAid.SubSample.x/2);
	  AlignAid.SubSampleSetting.y = MAX(1,AlignAid.SubSample.y/2);
	  AlignAid.SubSampleSetting.z = MAX(1,AlignAid.SubSample.z/2);
	  action_change_resolution();  // improve resolution
	  AlignAid.iParameter = 0;
	  update_alignment(0);
	  auto_adjust_affine_iterations(2,2);
	}
      else if ( AdjustSet )
	{ // not multi-resolution, so detailed alignment of rigid-body
	  auto_adjust_affine_iterations(3,1); // detailed alignment
	}

      // Use 9 parameters
      AlignAid.iParameter = 0;
      update_alignment(0);
      for (j=0; j<NAFFINE; j++)
	{
	  AlignAid.Adjust[j] = 0;
	  if ( j < 9 && AdjustSave[j] )
	    AlignAid.Adjust[j] = 1;
	}
      for (j=0, AdjustSet=FALSE; j<NAFFINE; j++)
	AdjustSet |= AlignAid.Adjust[j];
      if ( AdjustSet )
	{
	  update_alignment(0);
	  auto_adjust_affine_iterations(2,1);
	}
      // Use all selected parameters.
      for (j=0; j<NAFFINE; j++)
	AlignAid.Adjust[j] = AdjustSave[j];
      first_time = FALSE;
      action_auto_adjust_affine(button); // recall this function
    }
  else
    {
      // Find the 1st parameter adjustment.
      AlignAid.iParameter = 0;
      while ( !AlignAid.Adjust[AlignAid.iParameter] )
	AlignAid.iParameter++;

      if ( !MultiResolution )
	{
	  update_alignment(0);
	  auto_adjust_affine_iterations(1,1);
	}
      else
	{
	  action_change_resolution();
	  update_alignment(0);
	  auto_adjust_affine_iterations(2,2);
	  if ( AlignAid.SubSample.x*AlignAid.SubSample.y*AlignAid.SubSample.z != 1 )
	    {
	      AlignAid.SubSampleSetting.x = MAX(1,AlignAid.SubSample.x/2);
	      AlignAid.SubSampleSetting.y = MAX(1,AlignAid.SubSample.y/2);
	      AlignAid.SubSampleSetting.z = MAX(1,AlignAid.SubSample.z/2);
	      action_change_resolution();
	    }
	  update_alignment(0);
	  auto_adjust_affine_iterations(1,1);
	}
      first_time = FALSE;
      print_parameter_set(2,0);
      // Final transformation: use most accurate interpolation
      update_alignment(1);
      show_on_info_line(0,0,"done","","");
      fprintf(stdout,"MI = %g\n",AlignAid.CF_Affine);
      fflush(stdout);
    }
}

#ifdef JIP_USE_CONTEXT
#undef first_time
#endif
