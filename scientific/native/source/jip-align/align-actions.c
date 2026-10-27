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
  if ( (AlignAid.SubSampleSetting.x != AlignAid.SubSample.x ||
	AlignAid.SubSampleSetting.y != AlignAid.SubSample.y ||
	AlignAid.SubSampleSetting.z != AlignAid.SubSample.z) &&
       (AlignNL.Adjusted.x || AlignNL.Adjusted.y || AlignNL.Adjusted.z) )
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
  if ( AlignAid.DisplayType == 0 )      // mimic
    { // choose a new time point from "source", and transform it into mimic
      iTime_Source++;
      if ( iTime_Source >= source_unfiltered.nt )
	iTime_Source = 0;
      select_source(iTime_Source);
    }
  else if ( AlignAid.DisplayType == 1 ) // target
    { // choose a new time point from "template", and transform it into target
      iTime_Template++;
      if ( iTime_Template >= template.nt )
	iTime_Template = 0;
      select_target(iTime_Template,target.dim);
    }

  point_mosaic_to_stack_data();         // set the data pointer
  update_alignment(1);

  mosaic.update_regions |= UPDATE_IMAGES;
  mosaic.update_regions |= UPDATE_SELECT_TEXT_IMAGE;
  mosaic.update_regions |= UPDATE_INFO_AREA_GRAPH;
  graph.update_regions  |= UPDATE_GRAPH;
}

void action_decrement_image()
{
  // decrement time
  if ( AlignAid.DisplayType == 0 )      // mimic
    { // choose a new time point from "source", and transform it into mimic
      iTime_Source--;
      if ( iTime_Source < 0 )
	iTime_Source = source_unfiltered.nt-1;
      select_source(iTime_Source);
    }
  else if ( AlignAid.DisplayType == 1 ) // template
    { // choose a new time point from "template", and transform it into target
      iTime_Template--;
      if ( iTime_Template < 0 )
	iTime_Template = template.nt-1;
      select_target(iTime_Template,target.dim);
    }

  point_mosaic_to_stack_data();  // set the data pointer
  update_alignment(1);

  mosaic.update_regions |= UPDATE_IMAGES;
  mosaic.update_regions |= UPDATE_SELECT_TEXT_IMAGE;
  mosaic.update_regions |= UPDATE_INFO_AREA_GRAPH;
  graph.update_regions  |= UPDATE_GRAPH;
}

void action_RightPanel_cursor()
{
  //  if ( AlignAid.DisplayType != 0 ) // don't change if template is shown
  //    return;
  // To speed display, an argument could be added to the following function
  // to just draw the outline (parameter field active) when the cursor hits it.
  mosaic.update_regions |= UPDATE_RIGHT_PANEL;
}

void action_RightPanel_button1()
{
  AlignAid.DisplaySet++;
  if ( AlignAid.DisplaySet > 2 )
    AlignAid.DisplaySet = 0;
  update_display_set();
  AlignAid.iParameter = AlignAid.iFirstParShown;
  show_alignment_help_text();
  mosaic.update_regions |= UPDATE_RIGHT_PANEL;
  mosaic.update_regions |= UPDATE_SELECT_TEXT_IMAGE;
}

void action_RightPanel_button23()
{
  AlignAid.DisplaySet--;
  if ( AlignAid.DisplaySet < 0 )
    AlignAid.DisplaySet = 2;
  update_display_set();
  AlignAid.iParameter = AlignAid.iFirstParShown;
  show_alignment_help_text();
  mosaic.update_regions |= UPDATE_RIGHT_PANEL;
  mosaic.update_regions |= UPDATE_SELECT_TEXT_IMAGE;
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
	  if ( AlignAid.iParameter==4 &&   // overlay filter; sometimes invisible
		    (ovl_root.npts==AlignAid.nmask||AlignAid.LowResolution) )
	    AlignAid.iParameter--;
	  else if ( AlignAid.iParameter==5 && AlignAid.LowResolution ) // 4 = rolloff
	    AlignAid.iParameter-=2;
	  else if ( AlignAid.iParameter==10 &&   // change res.; sometimes invisible
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
	  if ( AlignAid.iParameter==4 &&   // overlay filter; sometimes invisible
		    (ovl_root.npts==AlignAid.nmask||AlignAid.LowResolution) )
	    AlignAid.iParameter++;
	  else if ( AlignAid.iParameter==5 && AlignAid.LowResolution ) // 4 = rolloff
	    AlignAid.iParameter+=2;
	  else if ( AlignAid.iParameter==10 &&   // change res.; sometimes invisible
		    (AlignAid.SubSampleSetting.x == AlignAid.SubSample.x &&
		     AlignAid.SubSampleSetting.y == AlignAid.SubSample.y &&
		     AlignAid.SubSampleSetting.z == AlignAid.SubSample.z) )
	    AlignAid.iParameter++;
	}
      if ( AlignAid.iParameter > AlignAid.iLastParShown )
	AlignAid.iParameter = AlignAid.iFirstParShown;
    }
  show_alignment_help_text();
  mosaic.update_regions |= UPDATE_RIGHT_PANEL;
}

void action_RightPanel_arrow_left_right( int left )
{
  int iGrid;
  float step, resolution;

  if ( AlignAid.DisplaySet == 0 ) // orientation
    {
      if ( AlignAid.iParameter >=0 && AlignAid.iParameter <= 3 )
	{
	  if ( AlignAid.DisplayType != 0 )  // not mimic
	    action_change_display_type(0);  // ... so display mimic
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
	{ // overlay filter
	  action_filter_overlay();
	}
      else if ( AlignAid.iParameter == 5 )
	{ // rolloff filter width
	  action_reduce_rolloff(1);
	}
      else if ( AlignAid.iParameter == 6 )
	{ // matching criteria
	  AlignAid.matching_volume = !AlignAid.matching_volume;
	}
      else if ( AlignAid.iParameter == 7 )
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
	  resolution = template.resolution.x * AlignAid.SubSampleSetting.x;
	  AlignAid.SubSampleSetting.y = rint((double)(resolution /template.resolution.y));
	  AlignAid.SubSampleSetting.z = rint((double)(resolution /template.resolution.z));
	  AlignAid.SubSampleSetting.y = MAX(1,AlignAid.SubSampleSetting.y);
	  AlignAid.SubSampleSetting.z = MAX(1,AlignAid.SubSampleSetting.z);
	}
      else if ( AlignAid.iParameter == 8 )
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
      else if ( AlignAid.iParameter == 9 )
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
      else if ( AlignAid.iParameter == 10 )
	{
	  action_change_resolution();
	}
      if ( AlignAid.iParameter < 4 )
	{
	  calculate_orientation_rotation();
	  // The orientation will change (x,y,z) vector directions, so change alignment parameter names
	  modify_alignment_parameter_names();
	}
    }

  else if ( AlignAid.DisplaySet == 1 ) // affine
    {
      if ( AlignAid.ChangeValue )
	{
	  if ( AlignAid.DisplayType != 0 ) // not mimic
	    action_change_display_type(0); // ... so display mimic
	  if ( left )
	    Alignment.value[AlignAid.iParameter] -= AlignAid.increment[AlignAid.iParameter];
	  else
	    Alignment.value[AlignAid.iParameter] += AlignAid.increment[AlignAid.iParameter];
	  AlignAid.Adjust[AlignAid.iParameter] = TRUE;
	  cost_function();
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
	    step = 0.005;
	  else if ( AlignAid.iParameter < 12 )   // skews
	    step = 0.01;
	  if ( left )
	    AlignAid.increment[AlignAid.iParameter] -= step;
	  else
	    AlignAid.increment[AlignAid.iParameter] += step;
	  if ( AlignAid.increment[AlignAid.iParameter] <= 0. )
	    AlignAid.increment[AlignAid.iParameter] = step;
	  mosaic.update_regions |= UPDATE_RIGHT_PANEL;
	  return;
	}
    }

  else if ( AlignAid.DisplaySet == 2 ) // non-linear
    {
      if ( AlignAid.iParameter == 0 )
	AlignNL.Adjust.axis.x = !AlignNL.Adjust.axis.x;
      else if ( AlignAid.iParameter == 1 )
	AlignNL.Adjust.axis.y = !AlignNL.Adjust.axis.y;
      else if ( AlignAid.iParameter == 2 )
	AlignNL.Adjust.axis.z = !AlignNL.Adjust.axis.z;
      else if ( AlignAid.iParameter == 3 )
	{ // x
	  iGrid = AlignNL.iGridDown;
	  if ( left )
	    iGrid /= 2;
	  else
	    iGrid *= 2;
	  iGrid = MIN(MAX(1,iGrid),16);
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
	  if ( AlignNL.ViewBasis && AlignAid.DisplayType == 6 )
	    {
	      create_basis_function_image();
	      mosaic.update_regions |= UPDATE_IMAGES;
	      graph.update_regions  |= UPDATE_GRAPH;
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
	      if ( AlignNL.Jacob_limits == 10. )
		AlignNL.Jacob_limits = 5.;
	      else if ( AlignNL.Jacob_limits == 5. )
		AlignNL.Jacob_limits = 2.;
	      else if ( AlignNL.Jacob_limits == 2. )
		AlignNL.Jacob_limits = 1.5;
	      else if ( AlignNL.Jacob_limits == 1.5 )
		AlignNL.Jacob_limits = 1.25;
	      AlignNL.Jacob_limits  = MAX(1.25,AlignNL.Jacob_limits);
	    }
	  else
	    {
	      if ( AlignNL.Jacob_limits == 1.25 )
		AlignNL.Jacob_limits = 1.5;
	      else if ( AlignNL.Jacob_limits == 1.5 )
		AlignNL.Jacob_limits = 2.;
	      else if ( AlignNL.Jacob_limits == 2. )
		AlignNL.Jacob_limits = 5.;
	      else if ( AlignNL.Jacob_limits == 5. )
		AlignNL.Jacob_limits = 10.;
	      AlignNL.Jacob_limits  = MIN(10.,AlignNL.Jacob_limits);
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
  show_alignment_help_text();
  if ( (AlignAid.DisplaySet==0 && AlignAid.iParameter<4)  ||
       (AlignAid.DisplaySet==1 && AlignAid.ChangeValue) )
    update_alignment(1);
  mosaic.update_regions |= UPDATE_RIGHT_PANEL;
  mosaic.update_regions |= UPDATE_SELECT_TEXT_IMAGE;
}

void action_RightPanel_tab()
{
  if ( AlignAid.DisplaySet == 2 && AlignAid.iParameter == 4 ) // non-linear
    { // non-linear basis functions
      AlignNL.iBasis += AlignNL.NBasis / 100;
      if ( AlignNL.iBasis >= AlignNL.NBasis )
	AlignNL.iBasis = 0;
      if ( AlignNL.ViewBasis && AlignAid.DisplayType == 6 )
	{
	  create_basis_function_image();
	  mosaic.update_regions |= UPDATE_IMAGES;
	  graph.update_regions  |= UPDATE_GRAPH;
	}
      mosaic.update_regions |= UPDATE_RIGHT_PANEL;
    }
  else
    {
      AlignAid.ChangeValue = !AlignAid.ChangeValue;
      mosaic.update_regions |= UPDATE_RIGHT_PANEL;
    }
}

void action_image_text_selection_specific( int button )
{
  BOOL AllSelected, affine, nonlinear;
  int j;

  if ( mosaic.text_selection == TEXTFIELD_MIMIC_TARGET )
    {
      if ( button == 1 )
	action_change_display_type(1);
      else
	action_change_display_type(-1);
    }
  else if ( mosaic.text_selection == TEXTFIELD_SELECT_PARAMETERS )
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
	  AlignNL.Adjust.axis.x = AlignNL.Adjust.axis.y = AlignNL.Adjust.axis.z = FALSE;
	}
      else if ( AlignAid.DisplaySet == 2 )
	{ // nonlinear
	  AllSelected = AlignNL.Adjust.axis.x && AlignNL.Adjust.axis.y && AlignNL.Adjust.axis.z;
	  if ( AllSelected )
	    AlignNL.Adjust.axis.x = AlignNL.Adjust.axis.y = AlignNL.Adjust.axis.z = FALSE;
	  else
	    AlignNL.Adjust.axis.x = AlignNL.Adjust.axis.y = AlignNL.Adjust.axis.z = TRUE;
	  // Any change disables all affine alignment.
	  for (j=0; j<NAFFINE; j++)
	    AlignAid.Adjust[j] = FALSE;
	}
      mosaic.update_regions |= UPDATE_RIGHT_PANEL;
      mosaic.update_regions |= UPDATE_SELECT_TEXT_IMAGE;
    }
  else if ( mosaic.text_selection == TEXTFIELD_COST_FUNCTION )
    {
      mosaic.update_regions |= UPDATE_SELECT_TEXT_IMAGE;
    }
  else if ( mosaic.text_selection == TEXTFIELD_RESET_RECALL )
    {
      if ( AlignAid.DisplaySet == 0 )
	recall_alignment_parameters();
      else
	reset_or_recall_parameters();
      calculate_orientation_rotation();
      update_alignment(1);
      cost_function(0);
      mosaic.update_regions |= UPDATE_IMAGES;
      mosaic.update_regions |= UPDATE_RIGHT_PANEL;
    }
  else if ( mosaic.text_selection == TEXTFIELD_AUTO_ALIGN )
    {
      affine = FALSE;
      for (j=0; j<NAFFINE; j++)
	affine |= AlignAid.Adjust[j];
      nonlinear = AlignNL.Adjust.axis.x || AlignNL.Adjust.axis.y || AlignNL.Adjust.axis.z;
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
  else if ( mosaic.text_selection == TEXTFIELD_DO_EVERYTHING )
    action_do_full_alignment();
}

#include "../engine/shared/action_change_display_type.inc"

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

  // Modify the target and source
  select_target(iTime_Template, target.dim);
  select_source(iTime_Source);

  transform_coordinates(1,1,1);
  iDisplayType = AlignAid.DisplayType;  // save this value
  // rescale the mimic
  AlignAid.DisplayType = 0;
  point_mosaic_to_stack_data();
  rescale_image(1);
  // rescale the target
  AlignAid.DisplayType = 1;
  point_mosaic_to_stack_data();
  rescale_image(1);
  // switch back to the current display
  AlignAid.DisplayType = iDisplayType;
  point_mosaic_to_stack_data();
  mosaic.update_regions |= UPDATE_IMAGES;
  mosaic.update_regions |= UPDATE_SELECT_TEXT_IMAGE;
}

void action_filter_overlay()
{
  int iDisplayType;
  JIP_INDEX j;

  AlignAid.nmask = 0;
  for (j=0; j<template.n3d; j++)
    {
      if ( overlay_map.weight[j] != 0. )
	{
	  AlignAid.mask[j] = 1;
	  AlignAid.nmask++;
	}
      else
	AlignAid.mask[j] = 0;
    }
  select_target( iTime_Template, target.dim );
  // Fill the mimic volume.
  transform_coordinates(1,1,1);

  iDisplayType = AlignAid.DisplayType;  // save this value
  // rescale the mimic
  AlignAid.DisplayType = 0;
  point_mosaic_to_stack_data();
  rescale_image(1);
  // rescale the target
  AlignAid.DisplayType = 1;
  point_mosaic_to_stack_data();
  rescale_image(1);
  // switch back to the current display
  AlignAid.DisplayType = iDisplayType;
  point_mosaic_to_stack_data();
  // clear the overlay
  action_clear_overlay();
  mosaic.update_regions |= UPDATE_IMAGES;
  mosaic.update_regions |= UPDATE_SELECT_TEXT_IMAGE;
}

void reset_or_recall_parameters()
{
  BOOL NonZero, SavedValue;
  JIP_INDEX j;

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
	      Jac_deter.SpaceTime.fData[j+Jac_deter.n3d] = Jac_deter.SpaceTime.fData[j];
	      // Zero values
	      distort_x.SpaceTime.fData[j]=distort_y.SpaceTime.fData[j]=distort_z.SpaceTime.fData[j]=Jac_deter.SpaceTime.fData[j]=0.;
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
	      Jac_deter.SpaceTime.fData[j] = Jac_deter.SpaceTime.fData[j+Jac_deter.n3d];
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
  // Perform multi-resolution affine alignment.
  affine = FALSE;
  for (j=0; j<NAFFINE; j++)
    affine |= AlignAid.Adjust[j];
  if ( affine ) // affine
    {
      printf("Perform affine alignment using all selected parameters.\n");
      action_auto_adjust_affine(1);
    }
  else
    printf("No affine parameters are selected; skip affine alignment.\n");

  // De-select affine parameters; retain the selected non-linear fields.
  for (j=0; j<NAFFINE; j++)
    AlignAid.Adjust[j] = FALSE;
  // Apply multi-step non-linear alignment.
  if ( AlignNL.Adjust.axis.x || AlignNL.Adjust.axis.y || AlignNL.Adjust.axis.z )
    {
      printf("Perform non-linear alignment by adjusting: ");
      if ( AlignNL.Adjust.axis.x )
	printf("x ");
      if ( AlignNL.Adjust.axis.y )
	printf("y ");
      if ( AlignNL.Adjust.axis.z )
	printf("z ");
      printf("field(s).\n");
      action_auto_adjust_nonlinear(1);
      AlignNL.Adjusted.x = AlignNL.Adjusted.y = AlignNL.Adjusted.z = TRUE;
    }
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

void action_graph_text_selection_specific( int button )
{
}

void action_graph_button23_TimeSeriesModel()
{
}
