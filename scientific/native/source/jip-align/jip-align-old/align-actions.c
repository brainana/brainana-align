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
#include "align.h"


void action_quit_specific()
{

  AlignAid.SubSampleSetting.x = AlignAid.SubSampleSetting.y = AlignAid.SubSampleSetting.z = 1;
  if ( (AlignAid.SubSampleSetting.x != AlignAid.SubSample.x) ||
       (AlignAid.SubSampleSetting.y != AlignAid.SubSample.y) ||
       (AlignAid.SubSampleSetting.z != AlignAid.SubSample.z) )
    {
      show_on_info_line(0,1,"Upsampling data to original resolution ...",
			"prior to saving file align.com"," ");
      action_change_resolution();
    }
  save_alignment_parameters();
}

void action_increment_image()
{
  // increment time
  if ( AlignAid.DisplayType == 0 )      // individual
    {
      graph.iTime++;
      if ( graph.iTime >= original.nt )
	graph.iTime = 0;
    }
  else if ( AlignAid.DisplayType == 1 ) // template
    {
      iTime_Template++;
      if ( iTime_Template >= template.nt )
	iTime_Template = 0;
    }

  point_mosaic_to_stack_data();  // set the data pointer
  update_alignment(1);
}

void action_decrement_image()
{
  // decrement time
  if ( AlignAid.DisplayType == 0 )      // individual
    {
      graph.iTime--;
      if ( graph.iTime < 0 )
	graph.iTime = original.nt-1;
    }
  else if ( AlignAid.DisplayType == 1 ) // template
    {
      iTime_Template--;
      if ( iTime_Template < 0 )
	iTime_Template = template.nt-1;
    }

  point_mosaic_to_stack_data();  // set the data pointer
  update_alignment(1);
}

void action_RightPanel_cursor()
{
  //  if ( AlignAid.DisplayType != 0 ) // don't change if template is shown
  //    return;
  // To speed display, an argument could be added to the following function
  // to just draw the outline (parameter field active) when the cursor hits it.
  show_alignment_info();
}

void action_RightPanel_button1()
{
  AlignAid.DisplaySet++;
  if ( AlignAid.DisplaySet > 2 )
    AlignAid.DisplaySet = 0;
  update_display_set();
  AlignAid.iParameter = AlignAid.iFirstParShown;
  draw_selectable_text_image();
  show_alignment_info();
  show_alignment_help_text();
}

void action_RightPanel_button23()
{
  AlignAid.DisplaySet--;
  if ( AlignAid.DisplaySet < 0 )
    AlignAid.DisplaySet = 2;
  update_display_set();
  AlignAid.iParameter = AlignAid.iFirstParShown;
  draw_selectable_text_image();
  show_alignment_info();
  show_alignment_help_text();
}

void action_RightPanel_arrow_up_down( int up )
{
  if ( up )
    {
      AlignAid.iParameter--;
      if ( AlignAid.iParameter < AlignAid.iFirstParShown )
	AlignAid.iParameter = AlignAid.iLastParShown;
      if ( AlignAid.DisplaySet == 0 )
	{
	  if ( AlignAid.iParameter==5 && AlignAid.LowResolution )
	    AlignAid.iParameter -= 2;
	  else if ( AlignAid.iParameter==5 &&   // overlay filter; sometimes invisible
		    (ovl_root.npts==AlignAid.nmask||AlignAid.LowResolution) )
	    AlignAid.iParameter--;
	  else if ( AlignAid.iParameter==9 &&   // change res.; sometimes invisible
		    (AlignAid.SubSampleSetting.x == AlignAid.SubSample.x &&
		     AlignAid.SubSampleSetting.y == AlignAid.SubSample.y &&
		     AlignAid.SubSampleSetting.z == AlignAid.SubSample.z) )
	    AlignAid.iParameter--;
	}
    }
  else
    {
      AlignAid.iParameter++;
      if ( AlignAid.DisplaySet == 0 )
	{
	  if ( AlignAid.iParameter==4 && AlignAid.LowResolution )
	    AlignAid.iParameter += 2;
	  else if ( AlignAid.iParameter==5 &&   // overlay filter; sometimes invisible
		    (ovl_root.npts==AlignAid.nmask||AlignAid.LowResolution) )
	    AlignAid.iParameter++;
	  else if ( AlignAid.iParameter==9 &&   // change res.; sometimes invisible
		    (AlignAid.SubSampleSetting.x == AlignAid.SubSample.x &&
		     AlignAid.SubSampleSetting.y == AlignAid.SubSample.y &&
		     AlignAid.SubSampleSetting.z == AlignAid.SubSample.z) )
	    AlignAid.iParameter++;
	}
      if ( AlignAid.iParameter > AlignAid.iLastParShown )
	AlignAid.iParameter = AlignAid.iFirstParShown;
    }
  show_alignment_info();
  show_alignment_help_text();
}

void action_RightPanel_arrow_left_right( int left )
{
  int iGrid;
  float step, resolution;

  if ( AlignAid.DisplaySet == 0 ) // orientation
    {
      if ( AlignAid.iParameter >=0 && AlignAid.iParameter <= 3 )
	{
	  if ( AlignAid.DisplayType != 0 )  // not individual
	    action_change_display_type(0);  // so display individual
	}
      if ( AlignAid.iParameter == 0 )
	{ // x rotation (-1,0,1,2)
	  if ( left )
	    Alignment.OrientRotation.x--;
	  else
	    Alignment.OrientRotation.x++;
	  if ( Alignment.OrientRotation.x < -1 )
	    Alignment.OrientRotation.x = 2;
	  else if ( Alignment.OrientRotation.x > 2 )
	    Alignment.OrientRotation.x = -1;
	}
      else if ( AlignAid.iParameter == 1 )
	{ // y rotation (-1,0,1,2)
	  if ( left )
	    Alignment.OrientRotation.y--;
	  else
	    Alignment.OrientRotation.y++;
	  if ( Alignment.OrientRotation.y < -1 )
	    Alignment.OrientRotation.y = 2;
	  else if ( Alignment.OrientRotation.y > 2 )
	    Alignment.OrientRotation.y = -1;
	}
      else if ( AlignAid.iParameter == 2 )
	{ // z rotation (-1,0,1,2)
	  if ( left )
	    Alignment.OrientRotation.z--;
	  else
	    Alignment.OrientRotation.z++;
	  if ( Alignment.OrientRotation.z < -1 )
	    Alignment.OrientRotation.z = 2;
	  else if ( Alignment.OrientRotation.z > 2 )
	    Alignment.OrientRotation.z = -1;
	}
      else if ( AlignAid.iParameter == 3 )
	{ // swap z
	  Alignment.OrientSwapZ = !Alignment.OrientSwapZ;
	}
      else if ( AlignAid.iParameter == 4 )
	{ // rolloff filter width
	  action_reduce_rolloff(1);
	}
      else if ( AlignAid.iParameter == 5 )
	{ // overlay filter
	  action_filter_overlay();
	}
      else if ( AlignAid.iParameter == 6 )
	{ // subsample x
	  if ( left )
	    AlignAid.SubSampleSetting.x--;
	  else
	    AlignAid.SubSampleSetting.x++;
	  if ( AlignAid.SubSampleSetting.x < 1 )
	    AlignAid.SubSampleSetting.x = 1;
	  else if ( AlignAid.SubSampleSetting.x > 8 )
	    AlignAid.SubSampleSetting.x = 8; // don't allow riduculous values
	  // Set y and z based upon x.
	  resolution = template_unfiltered.resolution.x * AlignAid.SubSampleSetting.x;
	  AlignAid.SubSampleSetting.y = rint((double)(resolution /template_unfiltered.resolution.y));
	  AlignAid.SubSampleSetting.z = rint((double)(resolution /template_unfiltered.resolution.z));
	  AlignAid.SubSampleSetting.y = MAX(1,AlignAid.SubSampleSetting.y);
	  AlignAid.SubSampleSetting.z = MAX(1,AlignAid.SubSampleSetting.z);
	}
      else if ( AlignAid.iParameter == 7 )
	{ // subsample y
	  if ( left )
	    AlignAid.SubSampleSetting.y--;
	  else
	    AlignAid.SubSampleSetting.y++;
	  if ( AlignAid.SubSampleSetting.y < 1 )
	    AlignAid.SubSampleSetting.y = 1;
	  else if ( AlignAid.SubSampleSetting.y > 8 )
	    AlignAid.SubSampleSetting.y = 8; // don't allow riduculous values
	}
      else if ( AlignAid.iParameter == 8 )
	{ // subsample z
	  if ( left )
	    AlignAid.SubSampleSetting.z--;
	  else
	    AlignAid.SubSampleSetting.z++;
	  if ( AlignAid.SubSampleSetting.z < 1 )
	    AlignAid.SubSampleSetting.z = 1;
	  else if ( AlignAid.SubSampleSetting.z > 8 )
	    AlignAid.SubSampleSetting.z = 8; // don't allow riduculous values
	}
      else if ( AlignAid.iParameter == 9 )
	{
	  action_change_resolution();
	}
      if ( AlignAid.iParameter < 4 )
	calculate_orientation_rotation();
    }
  else if ( AlignAid.DisplaySet == 1 ) // affine
    {
      if ( AlignAid.ChangeValue )
	{
	  if ( AlignAid.DisplayType != 0 ) // not individual
	    action_change_display_type(0);  // so display individual
	  if ( left )
	    Alignment.value[AlignAid.iParameter] -= AlignAid.increment[AlignAid.iParameter];
	  else
	    Alignment.value[AlignAid.iParameter] += AlignAid.increment[AlignAid.iParameter];
	  AlignAid.Adjust[AlignAid.iParameter] = TRUE;
	}
      else
	{
	  if ( AlignAid.iParameter < 0 )
	    return;
	  else if ( AlignAid.iParameter < 3 )    // x, y, z
	    step = 0.1;
	  else if ( AlignAid.iParameter < 6 )    // roll, pitch, yaw
	    step = 1.;
	  else if ( AlignAid.iParameter < 9 )    // inflation of x, y, z
	    step = 0.05;
	  else if ( AlignAid.iParameter < 12 )   // skews
	    step = 0.05;
	  if ( left )
	    AlignAid.increment[AlignAid.iParameter] -= step;
	  else
	    AlignAid.increment[AlignAid.iParameter] += step;
	  if ( AlignAid.increment[AlignAid.iParameter] <= 0. )
	    AlignAid.increment[AlignAid.iParameter] = step;
	  show_alignment_info();
	  return;
	}
    }
  else if ( AlignAid.DisplaySet == 2 ) // non-linear
    {
      if ( AlignAid.iParameter == 0 )
	AlignNL.Adjust.x = !AlignNL.Adjust.x;
      else if ( AlignAid.iParameter == 1 )
	AlignNL.Adjust.y = !AlignNL.Adjust.y;
      else if ( AlignAid.iParameter == 2 )
	AlignNL.Adjust.z = !AlignNL.Adjust.z;
      else if ( AlignAid.iParameter == 3 )
	{ // x
	  iGrid = AlignNL.iGridDown;
	  if ( left )
	    iGrid /= 2;
	  else
	    iGrid *= 2;
	  iGrid = MIN(MAX(2,iGrid),8);
	  SetGridDown(iGrid);
	  update_nonlinear_alignment_parameters();
	}
      else if ( AlignAid.iParameter == 4 )
	{
	  if ( left )
	    AlignNL.iBasis--;
	  else
	    AlignNL.iBasis++;
	  if ( AlignNL.iBasis < 0 )
	    AlignNL.iBasis = AlignNL.NBasis-1;
	  else if ( AlignNL.iBasis >= AlignNL.NBasis )
	    AlignNL.iBasis = 0;
	  if ( AlignNL.ViewBasis && AlignAid.DisplayType == 5 )
	    {
	      create_basis_function_image();
	      update_image_and_graph(1);
	    }
	}
      else if ( AlignAid.iParameter == 5 )
	{
	  if ( left )
	    {
	      if ( AlignNL.Lambda_reg == 10 )
		AlignNL.Lambda_reg = 5;
	      else if ( AlignNL.Lambda_reg == 5 )
		AlignNL.Lambda_reg = 3;
	      else if ( AlignNL.Lambda_reg == 3 )
		AlignNL.Lambda_reg = 1;
	      AlignNL.Lambda_reg  = MAX(1,AlignNL.Lambda_reg);
	    }
	  else
	    {
	      if ( AlignNL.Lambda_reg == 1 )
		AlignNL.Lambda_reg = 3;
	      else if ( AlignNL.Lambda_reg == 3 )
		AlignNL.Lambda_reg = 5;
	      else if ( AlignNL.Lambda_reg == 5 )
		AlignNL.Lambda_reg = 10;
	      AlignNL.Lambda_reg  = MIN(10,AlignNL.Lambda_reg);
	    }
	}
      else if ( AlignAid.iParameter == 6 )
	{
	  if ( left )
	    {
	      if ( AlignNL.Jacob_limits == 10 )
		AlignNL.Jacob_limits = 5;
	      else if ( AlignNL.Jacob_limits == 5 )
		AlignNL.Jacob_limits = 2;
	      AlignNL.Jacob_limits  = MAX(2,AlignNL.Jacob_limits);
	    }
	  else
	    {
	      if ( AlignNL.Jacob_limits == 2 )
		AlignNL.Jacob_limits = 5;
	      else if ( AlignNL.Jacob_limits == 5 )
		AlignNL.Jacob_limits = 10;
	      AlignNL.Jacob_limits  = MIN(10,AlignNL.Jacob_limits);
	    }
	}
      else if ( AlignAid.iParameter == 7 )
	{
	  if ( left )
	    AlignNL.stiffness -= 0.25;
	  else
	    AlignNL.stiffness += 0.25;
	  if ( AlignNL.stiffness > 1. )
	    AlignNL.stiffness = 1;
	  if ( AlignNL.stiffness < 0 )
	    AlignNL.stiffness = 0;
	}
    }
  show_alignment_info();
  draw_selectable_text_image();
  if ( (AlignAid.DisplaySet==0 && AlignAid.iParameter<4)  ||
       (AlignAid.DisplaySet==1 && AlignAid.ChangeValue) )
    update_alignment(1);
}

void action_RightPanel_tab()
{
  if ( AlignAid.DisplaySet == 2 && AlignAid.iParameter == 4 ) // non-linear
    { // non-linear basis functions
      AlignNL.iBasis += AlignNL.NBasis / 100;
      if ( AlignNL.iBasis >= AlignNL.NBasis )
	AlignNL.iBasis = 0;
      if ( AlignNL.ViewBasis && AlignAid.DisplayType == 5 )
	{
	  create_basis_function_image();
	  update_image_and_graph(1);
	  show_alignment_info();
	}
    }
  else
    {
      AlignAid.ChangeValue = !AlignAid.ChangeValue;
      show_alignment_info();
    }
}

void action_image_toggle_text_selection_specific( int button )
{
  BOOL AllSelected, affine, nonlinear;
  int j;

  if ( mosaic.text_selection&CURSOR_ON_INDIVIDUAL_TEMPLATE )
    {
      if ( button == 1 )
	action_change_display_type(1);
      else
	action_change_display_type(-1);
    }
  else if ( mosaic.text_selection&CURSOR_ON_SELECT_PARAMETERS )
    {
      if ( AlignAid.DisplaySet == 1 )
	{ // affine
	  AllSelected = TRUE;
	  for (j=0; j<NAFFINE; j++)
	    AllSelected &= AlignAid.Adjust[j];
	  if ( AllSelected )
	    { // Then select none.
	      for (j=0; j<NAFFINE; j++)
		AlignAid.Adjust[j] = FALSE;
	    }
	  else
	    { // Then select all.
	      for (j=0; j<NAFFINE; j++)
		AlignAid.Adjust[j] = TRUE;
	    }
	  // Any change disables all non-linear alignment.
	  AlignNL.Adjust.x = AlignNL.Adjust.y = AlignNL.Adjust.z = FALSE;
	}
      else if ( AlignAid.DisplaySet == 2 )
	{ // nonlinear
	  AllSelected = AlignNL.Adjust.x && AlignNL.Adjust.y && AlignNL.Adjust.z;
	  if ( AllSelected )
	    AlignNL.Adjust.x = AlignNL.Adjust.y = AlignNL.Adjust.z = FALSE;
	  else
	    AlignNL.Adjust.x = AlignNL.Adjust.y = AlignNL.Adjust.z = TRUE;
	  // Any change disables all affine alignment.
	  for (j=0; j<NAFFINE; j++)
	    AlignAid.Adjust[j] = FALSE;
	}
      show_alignment_info();
      draw_selectable_text_image();
    }
  else if ( mosaic.text_selection&CURSOR_ON_COST_FUNCTION )
    {
      //      AlignAid.CostFunction = !AlignAid.CostFunction;
      draw_selectable_text_image();
    }
  else if ( mosaic.text_selection&CURSOR_ON_RESET_RECALL )
    {
      if ( AlignAid.DisplaySet == 0 )
	recall_alignment_parameters();
      else
	reset_or_recall_parameters();
      calculate_orientation_rotation();
      update_alignment(1);
      cost_function(0);
      show_alignment_info();
    }
  else if ( mosaic.text_selection&CURSOR_ON_AUTO_ALIGN )
    {
      affine = FALSE;
      for (j=0; j<NAFFINE; j++)
	affine |= AlignAid.Adjust[j];
      nonlinear = AlignNL.Adjust.x || AlignNL.Adjust.y || AlignNL.Adjust.z;
      if ( affine && nonlinear )
	{
	  if ( AlignAid.DisplaySet == 2 )
	    affine = FALSE;
	  else
	    nonlinear = FALSE;
	}

      if ( affine )
	action_auto_adjust_affine(button);
      else
	action_auto_adjust_nonlinear(button);
    }
  else if ( mosaic.text_selection&CURSOR_ON_DO_EVERYTHING )
    action_do_full_alignment();
}

void action_change_display_type( int direction )
{

  if ( direction == 1 )
    {
      AlignAid.DisplayType++;
      if ( AlignAid.DisplayType > 5 )
	AlignAid.DisplayType = 0;
    }
  else if ( direction == -1 )
    {
      AlignAid.DisplayType--;
      if ( AlignAid.DisplayType < 0 )
	AlignAid.DisplayType = 5;
    }
  else
    {
      if ( AlignAid.DisplayType < 2 )
	AlignAid.DisplayType = !AlignAid.DisplayType;
      else // DisplayType is neither template nor individual; set to individual.
	AlignAid.DisplayType = 0;
    }
  point_mosaic_to_stack_data();  // set the data pointer
  build_image_display(1);
  fill_graph_vector(0);
  build_graph_display(1);
}

void action_reduce_rolloff( BOOL PromptForScale )
{
  char default_string[LEN_STR], smoothing_string[LEN_STR];
  int iDisplayType;

  if ( PromptForScale )
    {
      if (AlignAid.smooth_fov_fraction == 0 )
	sprintf(default_string,"04");
      else
	sprintf(default_string,"0");
      if ( get_X11_string(0,"Input n","(Flatten by FOV/n)",
			  default_string,2,1,smoothing_string) )
	sscanf(smoothing_string,"%d",&AlignAid.smooth_fov_fraction);
    }
  else
    {
      if (AlignAid.smooth_fov_fraction == 0 )
	AlignAid.smooth_fov_fraction = 4.;
      else
	AlignAid.smooth_fov_fraction = 0.;
    }

  // smooth the data
  filter_image_data();
  transform_coordinates(1,1);
  iDisplayType = AlignAid.DisplayType;  // save this value
  // rescale the individual
  AlignAid.DisplayType = 0;
  point_mosaic_to_stack_data();
  rescale_image(1);
  // rescale the template
  AlignAid.DisplayType = 1;
  point_mosaic_to_stack_data();
  rescale_image(1);
  // switch back to the current display
  AlignAid.DisplayType = iDisplayType;
  point_mosaic_to_stack_data();
  build_image_display(1);
}

void action_filter_overlay()
{
  int iDisplayType, j;

  AlignAid.nmask = 0;
  for (j=0; j<template_unfiltered.n3d; j++)
    {
      if ( overlay_map.weight[j] != 0. )
	{
	  AlignAid.mask[j] = 1;
	  AlignAid.nmask++;
	}
      else
	AlignAid.mask[j] = 0;
    }

  // smooth the data
  filter_image_data();
  transform_coordinates(1,1);
  iDisplayType = AlignAid.DisplayType;  // save this value
  // rescale the individual
  AlignAid.DisplayType = 0;
  point_mosaic_to_stack_data();
  rescale_image(1);
  // rescale the template
  AlignAid.DisplayType = 1;
  point_mosaic_to_stack_data();
  rescale_image(1);
  // switch back to the current display
  AlignAid.DisplayType = iDisplayType;
  point_mosaic_to_stack_data();
  // clear the overlay
  action_clear_overlay();
  build_image_display(1);
}

void reset_or_recall_parameters()
{
  BOOL NonZero, SavedValue;
  int j;

  NonZero    = FALSE;
  SavedValue = FALSE;
  if ( AlignAid.DisplaySet == 1 )
    { // affine
      for (j=0; j<6; j++)        // translations & rotations
	{
	  NonZero    |= (Alignment.value[j]      != 0.);
	  SavedValue |= (AlignAid.saved_value[j] != 0.);
	}
      for (j=6; j<9; j++)        // inflations
	{
	  NonZero    |= (Alignment.value[j]      != 1.);
	  SavedValue |= (AlignAid.saved_value[j] != 1.);
	}
      for (j=9; j<NAFFINE; j++)  // skews
	{
	  NonZero    |= (Alignment.value[j]      != 0.);
	  SavedValue |= (AlignAid.saved_value[j] != 0.);
	}
      // If values are non-zero, reset them to zero.
      if ( NonZero )
	{
	  for (j=0; j<NAFFINE; j++)
	    {
	      AlignAid.saved_value[j] = Alignment.value[j];
	      if ( j < 6 || j >= 9 )
		Alignment.value[j] = 0.;
	      else
		Alignment.value[j] = 1.;
	    }
	}
      else if ( SavedValue )
	{ // if values are saved, recall them.
	  for (j=0; j<NAFFINE; j++)
	    Alignment.value[j] = AlignAid.saved_value[j];
	}
    }
  else if ( AlignAid.DisplaySet == 2 )
    { // nonlinear
      NonZero = FALSE;
      SavedValue = FALSE;
      for (j=0; j<distort_x.n3d; j++)
	{
	  if ( distort_x.SpaceTime.fData[j] != 0. ) NonZero = TRUE;
	  if ( distort_y.SpaceTime.fData[j] != 0. ) NonZero = TRUE;
	  if ( distort_z.SpaceTime.fData[j] != 0. ) NonZero = TRUE;
	  if ( distort_x.SpaceTime.fData[j+distort_x.n3d] != 0. ) SavedValue = TRUE;
	  if ( distort_y.SpaceTime.fData[j+distort_y.n3d] != 0. ) SavedValue = TRUE;
	  if ( distort_z.SpaceTime.fData[j+distort_z.n3d] != 0. ) SavedValue = TRUE;
	}
      // If values are non-zero, reset them to zero.
      if ( NonZero )
	{
	  for (j=0; j<distort_x.n3d; j++)
	    { // Save non-zero values, then zero them.
	      // Save values by moving them to the 2nd time point
	      distort_x.SpaceTime.fData[j+distort_x.n3d] = distort_x.SpaceTime.fData[j];
	      distort_y.SpaceTime.fData[j+distort_y.n3d] = distort_y.SpaceTime.fData[j];
	      distort_z.SpaceTime.fData[j+distort_z.n3d] = distort_z.SpaceTime.fData[j];
	      // Zero values
	      distort_x.SpaceTime.fData[j]=distort_y.SpaceTime.fData[j]=distort_z.SpaceTime.fData[j]=0.;
	      AlignNL.Adjusted.x = AlignNL.Adjusted.y = AlignNL.Adjusted.z = FALSE;
	    }
	}
      else if ( SavedValue )
	{ // if values are saved, recall them.
	  for (j=0; j<distort_x.n3d; j++)
	    {
	      distort_x.SpaceTime.fData[j] = distort_x.SpaceTime.fData[j+distort_x.n3d];
	      distort_y.SpaceTime.fData[j] = distort_y.SpaceTime.fData[j+distort_y.n3d];
	      distort_z.SpaceTime.fData[j] = distort_z.SpaceTime.fData[j+distort_z.n3d];
	      if ( distort_x.SpaceTime.fData[j] != 0. ) AlignNL.Adjusted.x = TRUE;
	      if ( distort_y.SpaceTime.fData[j] != 0. ) AlignNL.Adjusted.y = TRUE;
	      if ( distort_z.SpaceTime.fData[j] != 0. ) AlignNL.Adjusted.z = TRUE;
	    }
	}
    }
}

void action_do_full_alignment()
{
  BOOL affine;
  int j;

  // Apply the rolloff filter.
  if (AlignAid.smooth_fov_fraction == 0 )
    {
      printf("\nApply the rolloff filter\n");
      action_reduce_rolloff(0);
    }
  // Apply the overlay filter if an overlay is active.
  if ( mosaic.toggle_overlays )
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
  // Perform multi-resolution affine alignment.
  affine = FALSE;
  for (j=0; j<NAFFINE; j++)
    affine |= AlignAid.Adjust[j];
  if ( affine ) // affine
    {
      printf("Perform affine alignment using all selected parameters.\n");
      action_auto_adjust_affine(2);
    }
  else
    printf("No affine parameters are selected; skip affine alignment.\n");

  // Remove the overlay filter if an overlay has been applied.
  if ( mosaic.toggle_overlays && AlignAid.nmask != 0 )
    {
      printf("Remove the overlay filter for non-linear alignment.\n");
      action_filter_overlay();
    }
  // De-select all affine parameters, and select all 3 non-linear fields.
  for (j=0; j<NAFFINE; j++)
    AlignAid.Adjust[j] = TRUE;
  // Apply multi-step non-linear alignment.
  if ( AlignNL.Adjust.x || AlignNL.Adjust.y || AlignNL.Adjust.z )
    {
      printf("Perform non-linear alignment by adjusting: ");
      if ( AlignNL.Adjust.x )
	printf("x ");
      if ( AlignNL.Adjust.y )
	printf("y ");
      if ( AlignNL.Adjust.z )
	printf("z ");
      printf("field(s).\n");
      action_auto_adjust_nonlinear(2);
      AlignNL.Adjusted.x = AlignNL.Adjusted.y = AlignNL.Adjusted.z = TRUE;
    }
}

void action_graph_toggle_text_selection_specific( int button )
{
}

int action_modify_time_model_button23()
{
  return(0);
}

void increment_activation_map()
{
}

void decrement_activation_map()
{
}
