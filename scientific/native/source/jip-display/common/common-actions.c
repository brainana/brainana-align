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
#include "common-display.h"


void action_key_c()
{ // color overlay
  JIP_INDEX j;

  if ( ovl_root.npts != 0 ) // an overlay exists
    {
      ovl_root.color++;
      if ( ovl_root.color > 8 ) ovl_root.color = 1;
      for (j=0; j<mosaic.stack->n3d; j++)
	if ( overlay_map.weight[j] != 0 ) overlay_map.color[j] = ovl_root.color;
      mosaic.update_regions |= UPDATE_IMAGES;
    }
}

void action_key_C()
{ // color wire frame
  JIP_INDEX j;

  if ( wire_root.npts != 0 ) // a wire frame exists
    {
      for (j=0; j<wire_root.npts; j++)
	{
	  if ( j == 0 || wire_root.control[j-1].color == 0 )
	    {
	      wire_root.control[j].color++;
	      if ( wire_root.control[j].color > 8 ) wire_root.control[j].color = 1;
	    }
	}
      mosaic.update_regions |= UPDATE_IMAGE_TEXT_LINES;
    }
}

void action_key_control_c( int button )
{ // change crosshair display
  if ( button == 1 )
    {
      mosaic.display_crosshairs++;
      if ( mosaic.display_crosshairs > 3 ) mosaic.display_crosshairs = 0;
    }
  else
    {
      mosaic.display_crosshairs--;
      if ( mosaic.display_crosshairs < 0 ) mosaic.display_crosshairs = 3;
    }
  mosaic.update_regions |= UPDATE_IMAGES;
}

void action_key_d()
{ // delete overlay
  if ( ovl_root.npts != 0 )
    action_clear_overlay();
  mosaic.update_regions |= UPDATE_IMAGES;
  mosaic.update_regions |= UPDATE_SELECT_TEXT_IMAGE;  // show overlay buttons
}

void action_key_D()
{ // delete wire frame
  if ( wire_root.npts != 0 )
    action_clear_wires();
  mosaic.update_regions |= UPDATE_IMAGES;
  mosaic.update_regions |= UPDATE_SELECT_TEXT_IMAGE;  // show overlay buttons
}
void action_key_F()
{ // fill closed wires
  fill_closed_wires();
  mosaic.update_regions |= UPDATE_IMAGES;
}

void action_key_i()
{ // image window/level
  contrast_low = 0.0;
  contrast_high = 1.0;
  rescale_image(1);
  mosaic.update_regions |= UPDATE_IMAGES;
}

void action_key_control_i() // same as tab
{ // interpolate across zoom factor
  if ( mosaic.cursor_selection&CURSOR_ON_RIGHT_PANEL )
    action_RightPanel_tab();
  else
    {
      mosaic.interpolate = !mosaic.interpolate;
      mosaic.update_regions |= UPDATE_IMAGES;
      mosaic.update_regions |= UPDATE_NEIGHBORHOODS;
    }
}

void action_key_m()
{ // mirror overlay left-right
  action_mirror_overlay();
  update_root_overlay(1,1);
  mosaic.update_regions |= UPDATE_IMAGES;
  graph.update_regions  |= UPDATE_GRAPH;
}

void action_key_M()
{ // mirror wire frame left-right
  action_mirror_wires();
  mosaic.update_regions |= UPDATE_IMAGES;
}

void action_key_n()
{ // New overlay
  action_select_activated_point();
  update_root_overlay(1,1);
  mosaic.update_regions |= UPDATE_IMAGES;
  mosaic.update_regions |= UPDATE_SELECT_TEXT_IMAGE;  // show overlay buttons
  graph.update_regions  |= UPDATE_GRAPH;
}

void action_key_N()
{ // New wire frame
  action_draw_wires();
  mosaic.update_regions |= UPDATE_IMAGE_TEXT_LINES;
  mosaic.update_regions |= UPDATE_SELECT_TEXT_IMAGE;  // show overlay buttons
}

void action_key_r()
{ // Read overlay
  read_overlay_file(".ovl",&ovl_root);
  update_root_overlay(1,1);
  mosaic.update_regions |= UPDATE_IMAGES;
  mosaic.update_regions |= UPDATE_SELECT_TEXT_IMAGE;  // show overlay buttons
  graph.update_regions  |= UPDATE_GRAPH;
}

void action_key_R()
{ // read wire frame
  read_wire_file(".wire",&wire_root,0);
  mosaic.update_regions |= UPDATE_IMAGES;
  mosaic.update_regions |= UPDATE_SELECT_TEXT_IMAGE;  // show overlay buttons
}

void action_key_T()
{ // transparent overlay
  if ( mosaic.activation.visible )
    {
      mosaic.activation.transparent = !mosaic.activation.transparent;
      mosaic.update_regions |= UPDATE_IMAGES;
      mosaic.update_regions |= UPDATE_COLORBAR;
    }
  else if ( ovl_root.npts != 0 ) // an overlay exists
    {
      mosaic.transparent_overlay = !mosaic.transparent_overlay;
      mosaic.update_regions |= UPDATE_IMAGES;
    }
}

void action_key_s()
{ // stats output to command line (1 point)
}

void action_key_S()
{ // stats output to command line (all points)
}

void action_key_w()
{ // write overlay
  if ( ovl_root.npts != 0 )  // only write an overlay that is displayed
    write_overlay_file();
}

void action_key_W()
{ // Write wire frame
  if ( wire_root.npts != 0 ) // only write wire outlines that are displayed
    write_wire_file();
}

void action_key_z()
{ // decrease zoom factor
  mosaic.zoom--;
  if ( mosaic.zoom == 0 ) mosaic.zoom = 1;
  update_XWindow_image();
}

void action_key_Z()
{ // increase zoom factor
  mosaic.zoom++;
  update_XWindow_image();
}

void action_key_3()
{
  mosaic.display_3orthogonal = !mosaic.display_3orthogonal;
  if ( mosaic.display_3orthogonal )
    mosaic.zoom++;
  else
    {
      mosaic.zoom--;
      mosaic.zoom = MAX(1,mosaic.zoom);
    }
  action_RightPanel_cursor();
  update_XWindow_image();
}

void action_key_control_g( int button )
{ // change image grid mode
  mosaic.display_grid++;
  if ( mosaic.display_grid > 2 ) mosaic.display_grid = 0;
  mosaic.update_regions |= UPDATE_IMAGE_TEXT_LINES;
}

void action_key_pound_sign()
{ // number of slices in mosaic
  action_select_slices();
}

void action_key_return()
{
  action_image_button1(0,0,0);
}

void action_image_motion_notify()
{ // make common function
  static int last_cursor_selection = -1, last_text_selection = -1;

  mosaic.update_regions |= UPDATE_INFO_AREA_IMAGE;
  mosaic.update_regions |= UPDATE_CURSOR_IMAGE;

  if ( mosaic.cursor_selection&CURSOR_ON_DATA )
    { // mosaic or any orthogonal projection
      if ( drawing_overlay )
	action_image_draw_overlay(0);
      else if ( graph.type == 3 && ovl_root.npts == 0 ) // time graph, no overlay
	{
	  if ( !mosaic.display_crosshairs )
	    graph.update_regions |= UPDATE_GRAPH;
	}
    }

  // Update the image window if the cursor changes positions
  // (e.g., to highlight the correct window in 3-plane mode)
  if (  mosaic.cursor_selection != last_cursor_selection ||
	(mosaic.text_selection  != last_text_selection && mosaic.text_selection*last_text_selection != 0) )
    {
      if ( mosaic.cursor_selection&CURSOR_ON_SELECT_TEXT ||
	   mosaic.cursor_selection&CURSOR_ON_COLORBAR    ||
	   mosaic.cursor_selection&CURSOR_ON_RIGHT_PANEL ||
	   last_cursor_selection  &CURSOR_ON_SELECT_TEXT ||
	   last_cursor_selection  &CURSOR_ON_COLORBAR    ||
	   last_cursor_selection  &CURSOR_ON_RIGHT_PANEL )
	mosaic.update_regions |= UPDATE_SELECT_TEXT_IMAGE;
      else if ( mosaic.display_3orthogonal )
	refresh_display_image(1);  // This is included to update border highlights in 3-plane mode
      action_ColorBar_cursor();
      if ( mosaic.cursor_selection&CURSOR_ON_RIGHT_PANEL ||
	   last_cursor_selection  &CURSOR_ON_RIGHT_PANEL )
	action_RightPanel_cursor();
    }
  last_cursor_selection = mosaic.cursor_selection;
  last_text_selection   = mosaic.text_selection;
}

void action_image_button1( int press, int xbut, int ybut )
{ // make common function
  static int last_xbut, last_ybut;
  float range, step;
  BOOL moving_mouse;

  if ( press == 1 )
    { // save starting locations of x,y
      last_xbut = xbut;
      last_ybut = ybut;
      return;
    }
  moving_mouse = (xbut!=last_xbut && ybut!=last_ybut);

  if ( mosaic.cursor_selection&CURSOR_ON_COLORBAR )
    action_ColorBar_button1();
  if ( mosaic.cursor_selection&CURSOR_ON_RIGHT_PANEL )
    action_RightPanel_button1();
  else if ( mosaic.cursor_selection&CURSOR_ON_SELECT_TEXT ) // selectable text
    action_image_text_selection(1);
  else if ( (mosaic.cursor_selection&CURSOR_ON_DATA) && !moving_mouse )
    {
      if ( graph.type >=0 && graph.type <= 2 )
	action_set_projection_location(); // set the projection
      else if ( (mosaic.cursor_selection&CURSOR_ON_MOSAIC) && mosaic.display_crosshairs )
	{ // change the selection point (cross-hairs)
	  mosaic.icross = mosaic.icursor;
	  mosaic.update_regions |= UPDATE_IMAGE_CROSSHAIRS;
	  graph.update_regions  |= UPDATE_GRAPH;
	}
      else if ( mosaic.cursor_selection&CURSOR_ON_ORTHO_XY )
	{ // change the selection point (cross-hairs)
	  mosaic.icross.x = mosaic.icursor.x;
	  mosaic.icross.y = mosaic.icursor.y;
	  mosaic.update_regions |= UPDATE_IMAGES;
	  mosaic.update_regions |= UPDATE_IMAGE_CROSSHAIRS;
	  graph.update_regions  |= UPDATE_GRAPH;
	}
      else if ( mosaic.cursor_selection&CURSOR_ON_ORTHO_ZY )
	{ // change the selection point (cross-hairs)
	  mosaic.icross.y = mosaic.icursor.y;
	  mosaic.icross.z = mosaic.icursor.z;
	  mosaic.update_regions |= UPDATE_IMAGES;
	  mosaic.update_regions |= UPDATE_IMAGE_CROSSHAIRS;
	  graph.update_regions  |= UPDATE_GRAPH;
	}
      else if ( mosaic.cursor_selection&CURSOR_ON_ORTHO_XZ )
	{ // change the selection point (cross-hairs)
	  mosaic.icross.x = mosaic.icursor.x;
	  mosaic.icross.z = mosaic.icursor.z;
	  mosaic.update_regions |= UPDATE_IMAGES;
	  mosaic.update_regions |= UPDATE_IMAGE_CROSSHAIRS;
	  graph.update_regions  |= UPDATE_GRAPH;
	} // change the selection point (cross-hairs)
      else if ( mosaic.cursor_selection&CURSOR_ON_ORTHO_YZ )
	{ // change the selection point (cross-hairs)
	  mosaic.icross.y = mosaic.icursor.y;
	  mosaic.icross.z = mosaic.icursor.z;
	  mosaic.update_regions |= UPDATE_IMAGES;
	  mosaic.update_regions |= UPDATE_IMAGE_CROSSHAIRS;
	  graph.update_regions  |= UPDATE_GRAPH;
	}
    }
  else if ( moving_mouse )
    { // on release, change the contrast if the cursor moved
      // X movement
      range = (float)(xbut - last_xbut)/ (float)mosaic.ImageRegionPixels.x;
      if ( range > 0. )
	{
	  step = (1.75 - contrast_high) * range;
	  contrast_high += step/2.;
	  step = (contrast_low + 0.5) * range;
	  contrast_low  -= step/2.;
	}
      else
	{
	  step = (contrast_high - contrast_low) * (-range);
	  contrast_low  += step/2.;
	  contrast_high -= step/2.;
	}
      // Y movement
      range = (float)(ybut - last_ybut)/ (float)mosaic.ImageRegionPixels.y;
      if ( range > 0. )
	{
	  step = (1.75 - contrast_high ) * range;
	  contrast_low += step;
	  contrast_high += step;
	}
      else
	{
	  step = ( contrast_low + 0.5 ) * (-range);
	  contrast_low -= step;
	  contrast_high -= step;
	}
      rescale_image(0);
    }
}

void action_image_button23( int press )
{ // make common function
  if ( press )
    { // button press
      if ( mosaic.cursor_selection&CURSOR_ON_DATA )
	{ // mosaic or any orthogonal projection
	  action_image_draw_overlay(1);
	}
      else if ( mosaic.cursor_selection&CURSOR_ON_COLORBAR )
	action_ColorBar_button23();
      else if ( mosaic.cursor_selection&CURSOR_ON_RIGHT_PANEL )
	action_RightPanel_button23();
      else if ( mosaic.cursor_selection&CURSOR_ON_SELECT_TEXT ) // selectable text
	action_image_text_selection(2);
    }
  else
    { // button release
      if ( mosaic.cursor_selection&CURSOR_ON_DATA )
	{ // mosaic or any orthogonal projection
	    action_image_draw_overlay(-1);
	    mosaic.update_regions |= UPDATE_SELECT_TEXT_IMAGE;  // show overlay buttons
	}
    }
}

void action_image_button4()
{ // scroll wheel down
  if ( mosaic.cursor_selection&CURSOR_ON_DATA || mosaic.cursor_selection&CURSOR_ON_MOSAIC_BORDER )
    action_image_arrow_in_out(1);
  else if ( mosaic.cursor_selection&CURSOR_ON_COLORBAR )
    action_ColorBar_arrow_up_down(1);
  else if ( mosaic.cursor_selection&CURSOR_ON_RIGHT_PANEL ) // colorbar
    action_RightPanel_arrow_up_down(1);
  else
    action_image_arrow_up_down(1);
}

void action_image_button5()
{ // scroll wheel up
  if ( mosaic.cursor_selection&CURSOR_ON_DATA || mosaic.cursor_selection&CURSOR_ON_MOSAIC_BORDER )
    action_image_arrow_in_out(0);
  else if ( mosaic.cursor_selection&CURSOR_ON_COLORBAR )
    action_ColorBar_arrow_up_down(0);
  else if ( mosaic.cursor_selection&CURSOR_ON_RIGHT_PANEL )
    action_RightPanel_arrow_up_down(0);
  else
    action_image_arrow_up_down(0);
}

void action_image_arrow_up_down( int up )
{ // make common function
  float step;
  BOOL move_crosshairs, move_overlay;
  BOOL move_y, move_z;

  // Take care of the simple cases first.
  if ( mosaic.cursor_selection&CURSOR_ON_COLORBAR )
    {
      action_ColorBar_arrow_up_down( up );
      return;
    }
  else if ( mosaic.cursor_selection&CURSOR_ON_RIGHT_PANEL )
    {
      action_RightPanel_arrow_up_down( up );
      return;
    }
  else if ( mosaic.text_selection == TEXTFIELD_WHICH_OVL_WIRE )
    {
      action_step_overlay_list( !up, 0 );
      return;
    }

  // x or y projection
  move_overlay    = (mosaic.cursor_selection&CURSOR_ON_DATA) && (ovl_root.npts != 0);
  move_crosshairs = (mosaic.cursor_selection&CURSOR_ON_DATA) && 
    (mosaic.display_crosshairs || graph.type==0 || graph.type==1 || graph.type==2);

  if ( move_crosshairs )
    {
      mosaic.update_regions |= UPDATE_IMAGE_CROSSHAIRS;
      move_y = (mosaic.cursor_selection&CURSOR_ON_ORTHO_XY)
	||     (mosaic.cursor_selection&CURSOR_ON_ORTHO_ZY);
      move_z = (mosaic.cursor_selection&CURSOR_ON_ORTHO_XZ)
	||     (mosaic.cursor_selection&CURSOR_ON_ORTHO_YZ);
      if ( move_y )
	{ // x-y or z-y
	  if ( up )
	    {
	      if ( mosaic.flip_X11_y )
		{
		  mosaic.icross.y++;
		  if ( mosaic.icross.y >= mosaic.stack->dim.y )
		    mosaic.icross.y = mosaic.stack->dim.y - 1;
		  else
		    graph.update_regions |= UPDATE_GRAPH;
		}
	      else
		{
		  mosaic.icross.y--;
		  if ( mosaic.icross.y < 0 )
		    mosaic.icross.y = 0;
		  else
		    graph.update_regions |= UPDATE_GRAPH;
		}
	    }
	  else // down
	    {
	      if ( mosaic.flip_X11_y )
		{
		  mosaic.icross.y--;
		  if ( mosaic.icross.y < 0 )
		    mosaic.icross.y = 0;
		  else
		    graph.update_regions |= UPDATE_GRAPH;
		}
	      else
		{
		  mosaic.icross.y++;
		  if ( mosaic.icross.y >= mosaic.stack->dim.y )
		    mosaic.icross.y = mosaic.stack->dim.y - 1;
		  else
		    graph.update_regions |= UPDATE_GRAPH;
		}
	    }
	}
      else if ( move_z )
	{ // x-z or y-z
	  if ( up )
	    {
	      if ( mosaic.flip_X11_y )
		{
		  mosaic.icross.z++;
		  if ( mosaic.icross.z >= mosaic.stack->dim.z )
		    mosaic.icross.z = mosaic.stack->dim.z - 1;
		  else
		    graph.update_regions |= UPDATE_GRAPH;
		}
	      else
		{
		  mosaic.icross.z--;
		  if ( mosaic.icross.z < 0 )
		    mosaic.icross.z = 0;
		  else
		    graph.update_regions |= UPDATE_GRAPH;
		}
	    }
	  else // down
	    {
	      if ( mosaic.flip_X11_y )
		{
		  mosaic.icross.z--;
		  if ( mosaic.icross.z < 0 )
		    mosaic.icross.z = 0;
		  else
		    graph.update_regions |= UPDATE_GRAPH;
		}
	      else
		{
		  mosaic.icross.z++;
		  if ( mosaic.icross.z >= mosaic.stack->dim.z )
		    mosaic.icross.z = mosaic.stack->dim.z - 1;
		  else
		    graph.update_regions |= UPDATE_GRAPH;
		}
	    }
	}
    }
  else if ( move_overlay )
    {
      mosaic.update_regions |= UPDATE_IMAGES;
      move_y = (mosaic.cursor_selection&CURSOR_ON_ORTHO_XY)
	||     (mosaic.cursor_selection&CURSOR_ON_ORTHO_ZY);
      move_z = (mosaic.cursor_selection&CURSOR_ON_ORTHO_XZ)
	||     (mosaic.cursor_selection&CURSOR_ON_ORTHO_YZ);
      // Move the overlay
      if ( move_y )
	{ // x-y or z-y
	  if ( up )
	    {
		//                             y  increment
	      if ( mosaic.flip_X11_y )
		move_overlay_in_one_direction( 1,  1 );
	      else
		move_overlay_in_one_direction( 1, -1 );
	    }
	  else // down
	    {
	      if ( mosaic.flip_X11_y )
		move_overlay_in_one_direction( 1, -1 );
	      else
		move_overlay_in_one_direction( 1,  1 );
	    }
	}
      else if ( move_z )
	{ // x-z or y-z
	  if ( up )
	    {
	      //                             z  increment
	      if ( mosaic.flip_X11_y )
		move_overlay_in_one_direction( 2,  1 );
	      else
		move_overlay_in_one_direction( 2, -1 );
	    }
	  else // down
	    {
	      if ( mosaic.flip_X11_y )
		move_overlay_in_one_direction( 2, -1 );
	      else
		move_overlay_in_one_direction( 2,  1 );
	    }
	}
    }
  else if ( mosaic.cursor_selection&CURSOR_ON_SELECT_TEXT )
    action_image_text_selection(!up);
  else
    { // adjust brightness
      if ( up )
	{
	  // Move the low & high values down together.
	  // The minimum low value is -0.5, so this defines the step.
	  step = ( contrast_low + 0.5 ) / 2.;
	  if ( step > .05 ) step = 0.05;
	  contrast_low -= step;
	  contrast_high -= step;
	}
      else
	{
	  // Move the low & high values down together.
	  // The maximum high value is 1.75, so this defines the step.
	  step = ( 1.75 - contrast_high ) / 2.;
	  if ( step > .05 ) step = 0.05;
	  contrast_low += step;
	  contrast_high += step;
	}
      rescale_image(0);
    }
}

void action_image_arrow_left_right( int left )
{ // make common function
  float step;
  BOOL move_crosshairs, move_overlay;
  BOOL move_x, move_y, move_z;

  if ( mosaic.cursor_selection&CURSOR_ON_COLORBAR )
    {
      action_ColorBar_arrow_left_right( left );
      return;
    }
  else if ( mosaic.cursor_selection&CURSOR_ON_RIGHT_PANEL )
    {
      action_RightPanel_arrow_left_right( left );
      return;
    }
  else if ( mosaic.text_selection == TEXTFIELD_WHICH_OVL_WIRE )
    {
      action_step_overlay_list( !left, 1 );
      return;
    }

  move_overlay    = (mosaic.cursor_selection&CURSOR_ON_DATA) && (ovl_root.npts != 0);
  move_crosshairs = (mosaic.cursor_selection&CURSOR_ON_DATA) &&
    ( mosaic.display_crosshairs || graph.type==0 || graph.type==1 || graph.type==2 );

  if ( move_crosshairs )
    {
      mosaic.update_regions |= UPDATE_IMAGE_CROSSHAIRS;
      move_x = (mosaic.cursor_selection&CURSOR_ON_ORTHO_XY)
	||     (mosaic.cursor_selection&CURSOR_ON_ORTHO_XZ);
      move_y = (mosaic.cursor_selection&CURSOR_ON_ORTHO_YZ);
      move_z = (mosaic.cursor_selection&CURSOR_ON_ORTHO_ZY);
      if ( move_x )
	{ // x-y or x-z
	  if ( left )
	    {
	      mosaic.icross.x--;
	      if ( mosaic.icross.x < 0 )
		mosaic.icross.x = 0;
	      else
		graph.update_regions |= UPDATE_GRAPH;
	    }
	  else
	    {
	      mosaic.icross.x++;
	      if ( mosaic.icross.x >= mosaic.stack->dim.x )
		mosaic.icross.x = mosaic.stack->dim.x - 1;
	      else
		graph.update_regions |= UPDATE_GRAPH;
	    }
	}
      else if ( move_y )
	{ // y-z
	  if ( left )
	    {
	      mosaic.icross.y--;
	      if ( mosaic.icross.y < 0 )
		mosaic.icross.y = 0;
	      else
		graph.update_regions |= UPDATE_GRAPH;
	    }
	  else
	    {
	      mosaic.icross.y++;
	      if ( mosaic.icross.y >= mosaic.stack->dim.y )
		mosaic.icross.y = mosaic.stack->dim.y - 1;
	      else
		graph.update_regions |= UPDATE_GRAPH;
	    }
	}
      else if ( move_z )
	{ // z-y
	  if ( left )
	    {
	      mosaic.icross.z--;
	      if ( mosaic.icross.z < 0 )
		mosaic.icross.z = 0;
	      else
		graph.update_regions |= UPDATE_GRAPH;
	    }
	  else
	    {
	      mosaic.icross.z++;
	      if ( mosaic.icross.z >= mosaic.stack->dim.z )
		mosaic.icross.z = mosaic.stack->dim.z - 1;
	      else
		graph.update_regions |= UPDATE_GRAPH;
	    }
	}
    }
  else if ( move_overlay )
    {
      mosaic.update_regions |= UPDATE_IMAGES;
      move_x = (mosaic.cursor_selection&CURSOR_ON_ORTHO_XY)
	||     (mosaic.cursor_selection&CURSOR_ON_ORTHO_XZ);
      move_y = (mosaic.cursor_selection&CURSOR_ON_ORTHO_YZ);
      move_z = (mosaic.cursor_selection&CURSOR_ON_ORTHO_ZY);
      // Move the overlay
      if ( move_x )
	{ // x-y or x-z
	  if ( left )
	    //                             x  increment
	    move_overlay_in_one_direction( 0, -1 );
	  else
	    move_overlay_in_one_direction( 0, 1 );
	}
      else if ( move_y )
	{ // y-z
	  if ( left )
	    //                             y  increment
	    move_overlay_in_one_direction( 1, -1 );
	  else
	    move_overlay_in_one_direction( 1, 1 );
	}
      else if ( move_z )
	{ // z-y
	  if ( left )
	    //                             z  increment
	    move_overlay_in_one_direction( 2, -1 );
	  else
	    move_overlay_in_one_direction( 2, 1 );
	}
    }
  else if ( mosaic.cursor_selection&CURSOR_ON_SELECT_TEXT )
    action_image_text_selection(!left);
  else
    { // adjust brightness
      if ( left )
	{
	  // Move the low and high values closer together.
	  step = (contrast_high - contrast_low)/2.;
	  if ( step > .05 ) step = 0.05;
	  contrast_low  += step/2.;
	  contrast_high -= step/2.;
	}
      else
	{
	  // Move the low and high values father apart.
	  step = (1.75 - contrast_high)/2.;
	  if ( step > .05 ) step = 0.05;
	  contrast_high += step/2.;
	  step = (contrast_low + 0.5)/10.;
	  if ( step > .05 ) step = 0.05;
	  contrast_low  -= step/2.;
	}
      rescale_image(0);
    }
}

void action_image_arrow_in_out( int in )
{ // make common function
  BOOL move_crosshairs, move_overlay, update;
  int j;

  // overlay
  move_overlay    = ovl_root.npts != 0;
  move_crosshairs = mosaic.display_crosshairs;

  if ( (mosaic.cursor_selection&CURSOR_ON_MOSAIC) && (mosaic.DimPanels.z!=mosaic.DisplayVolumeDim.z) )
    { // shift the slices in a restricted mosaic view
      update = FALSE;
      if ( in )
	{
	  if ( mosaic.display_slice[0] != 1 )
	    {
	      if ( mosaic.mosaic_orientation == ORTHO_XY )
		{
		  mosaic.icross.z--;
		  if ( mosaic.icross.z < 0 )
		    mosaic.icross.z = 0;
		  else
		    update = TRUE;
		}
	      else if ( mosaic.mosaic_orientation == ORTHO_XZ )
		{
		  mosaic.icross.y--;
		  if ( mosaic.icross.y < 0 )
		    mosaic.icross.y = 0;
		  else
		    update = TRUE;
		}
	      else if ( mosaic.mosaic_orientation == ORTHO_YZ || mosaic.mosaic_orientation == ORTHO_ZY )
		{
		  mosaic.icross.x--;
		  if ( mosaic.icross.x < 0 )
		    mosaic.icross.x = 0;
		  else
		    update = TRUE;
		}
	      for (j=0; j<mosaic.DisplayVolumeDim.z-1; j++)
		mosaic.display_slice[j] = mosaic.display_slice[j+1];
	      mosaic.display_slice[mosaic.DisplayVolumeDim.z-1] = 0;
	    }
	}
      else
	{
	  if ( mosaic.display_slice[mosaic.DisplayVolumeDim.z-1] != 1 )
	    {
	      if ( mosaic.mosaic_orientation == ORTHO_XY )
		{
		  mosaic.icross.z++;
		  if ( mosaic.icross.z >= mosaic.stack->dim.z )
		    mosaic.icross.z = mosaic.stack->dim.z-1;
		  else
		    update = TRUE;
		}
	      else if ( mosaic.mosaic_orientation == ORTHO_XZ )
		{
		  mosaic.icross.y++;
		  if ( mosaic.icross.y >= mosaic.stack->dim.y )
		    mosaic.icross.y = mosaic.stack->dim.y-1;
		  else
		    update = TRUE;
		}
	      else if ( mosaic.mosaic_orientation == ORTHO_YZ || mosaic.mosaic_orientation == ORTHO_ZY )
		{
		  mosaic.icross.x++;
		  if ( mosaic.icross.x >= mosaic.stack->dim.x )
		    mosaic.icross.z = mosaic.stack->dim.x-1;
		  else
		    update = TRUE;
		}
	      for (j=mosaic.DisplayVolumeDim.z-1; j>0; j--)
		mosaic.display_slice[j] = mosaic.display_slice[j-1];
	      mosaic.display_slice[0] = 0;
	    }
	}
      if ( update )
	{
	  mosaic.update_regions |= UPDATE_IMAGE_CROSSHAIRS;
	  mosaic.update_regions |= UPDATE_IMAGES;
	  graph.update_regions  |= UPDATE_GRAPH;
	}
    }
  else if ( move_crosshairs )
    {
      mosaic.update_regions |= UPDATE_IMAGE_CROSSHAIRS;
      mosaic.update_regions |= UPDATE_IMAGES;
      if ( mosaic.cursor_selection&CURSOR_ON_ORTHO_XY )
	{ // x-y, so move z
	  if ( in )
	    {
	      mosaic.icross.z--;
	      if ( mosaic.icross.z < 0 )
		mosaic.icross.z = 0;
	      else
		graph.update_regions |= UPDATE_GRAPH;
	    }
	  else
	    {
	      mosaic.icross.z++;
	      if ( mosaic.icross.z >= mosaic.stack->dim.z )
		mosaic.icross.z = mosaic.stack->dim.z-1;
	      else
		graph.update_regions |= UPDATE_GRAPH;
	    }
	}
      else if ( mosaic.cursor_selection&CURSOR_ON_DATA &&
		((mosaic.cursor_selection&CURSOR_ON_ORTHO_ZY) ||
		 (mosaic.cursor_selection&CURSOR_ON_ORTHO_YZ)) )
	{ // z-y or y-z, so move x
	  if ( in )
	    {
	      mosaic.icross.x--;
	      if ( mosaic.icross.x < 0 )
		mosaic.icross.x = 0;
	      else
		graph.update_regions |= UPDATE_GRAPH;
	    }
	  else
	    {
	      mosaic.icross.x++;
	      if ( mosaic.icross.x >= mosaic.stack->dim.x )
		mosaic.icross.x = mosaic.stack->dim.x-1;
	      else
		graph.update_regions |= UPDATE_GRAPH;
	    }
	}
      else if ( mosaic.cursor_selection&CURSOR_ON_ORTHO_XZ )
	{ // x-z, so move y
	  if ( in )
	    {
	      mosaic.icross.y--;
	      if ( mosaic.icross.y < 0 )
		mosaic.icross.y = 0;
	      else
		graph.update_regions |= UPDATE_GRAPH;
	    }
	  else
	    {
	      mosaic.icross.y++;
	      if ( mosaic.icross.y >= mosaic.stack->dim.y )
		mosaic.icross.y = mosaic.stack->dim.y-1;
	      else
		graph.update_regions |= UPDATE_GRAPH;
	    }
	}
    }
  else if ( move_overlay )
    {
      mosaic.update_regions |= UPDATE_IMAGE_CROSSHAIRS;
      mosaic.update_regions |= UPDATE_IMAGES;
      // Move the overlay
      if ( mosaic.cursor_selection&CURSOR_ON_ORTHO_XY )
	{ // x-y, so move in z
	  if ( in )
	    //                             z  increment
	    move_overlay_in_one_direction( 2, -1 );
	  else
	    move_overlay_in_one_direction( 2, 1 );
	}
      else if ( (mosaic.cursor_selection&CURSOR_ON_ORTHO_ZY) || (mosaic.cursor_selection&CURSOR_ON_ORTHO_YZ) )
	{ // z-y or y-z, so move in x
	  if ( in )
	    //                             x  increment
	    move_overlay_in_one_direction( 0, -1 );
	  else
	    move_overlay_in_one_direction( 0, 1 );
	}
      else if ( mosaic.cursor_selection&CURSOR_ON_ORTHO_XZ )
	{ // x-z, so move in y
	  if ( in )
	    //                             y  increment
	    move_overlay_in_one_direction( 1, -1 );
	  else
	    move_overlay_in_one_direction( 1, 1 );
	}
    }
}

void move_overlay_in_one_direction( int direction, int increment )
     // direction: 0 = x, 1 = y, 2 = z
     // increment: 1 or -1
{
  float *weight; // temporary array for moving overlay
  short *color;  // temporary array for moving overlay
  IPOINT3D voxel, voxel_new;
  JIP_INDEX j, index, index_new;
  int upper;

  if ( ovl_root.npts == 0 ) return;

  if ( direction == 0 )      // x
    upper = mosaic.stack->dim.x;
  else if ( direction == 1 ) // y
    upper = mosaic.stack->dim.y;
  else
    upper = mosaic.stack->dim.z;        // z

  // Make a temporary copy of the overlay bit array.
  weight = float_vector(mosaic.stack->n3d);
  color  = short_vector(mosaic.stack->n3d);
  for (j=0; j<mosaic.stack->n3d; j++)
    {
      weight[j] = overlay_map.weight[j];
      color[j]  = overlay_map.color[j];
    }

  for (voxel.z=0; voxel.z<mosaic.stack->dim.z; voxel.z++)
    {
      if ( direction == 2 ) // z
	{
	  voxel_new.z = voxel.z + increment;
	  if ( voxel_new.z < 0 ) voxel_new.z = upper-1;
	  if ( voxel_new.z == upper ) voxel_new.z = 0;
	}
      else
	voxel_new.z = voxel.z;
      for (voxel.y=0; voxel.y<mosaic.stack->dim.y; voxel.y++)
	{
	  if ( direction == 1 ) // y
	    {
	      voxel_new.y = voxel.y + increment;
	      if ( voxel_new.y < 0 ) voxel_new.y = upper-1;
	      if ( voxel_new.y == upper ) voxel_new.y = 0;
	    }
	  else
	    voxel_new.y = voxel.y;
	  for (voxel.x=0; voxel.x<mosaic.stack->dim.x; voxel.x++)
	    {
	      if ( direction == 0 ) // x
		{
		  voxel_new.x = voxel.x + increment;
		  if ( voxel_new.x < 0 ) voxel_new.x = upper-1;
		  if ( voxel_new.x == upper ) voxel_new.x = 0;
		}
	      else
		voxel_new.x = voxel.x;
	      index     = voxel3D_to_1D(voxel);
	      index_new = voxel3D_to_1D(voxel_new);
	      overlay_map.weight[index_new] = weight[index];
	      overlay_map.color[index_new]  = color[index];
	    }
	}
    }
  free_float_vector(weight);
  free_short_vector(color);
  update_root_overlay(1,1);
  graph.update_regions |= UPDATE_GRAPH;
}

void action_step_overlay_list( int next, int combine )
{
  struct ovl_list *ovl_current, *ovl_previous, *ovl_last, *ovl_selected;
  struct wire_list *wire_current, *wire_previous, *wire_last, *wire_selected;
  CONTROL_POINT *control_save;
  JIP_INDEX j, index;

  if ( mosaic.toggle_overlays == 0 )
    return; // do nothing; no overlays or wire frames are stored
  else if ( mosaic.toggle_overlays == 1 )
    { // use overlay list, not wire list
      // Find the last overlay in the pre-defined list.
      for (ovl_current=ovl_root.next; ovl_current; ovl_current=ovl_current->next)
	ovl_previous = ovl_current;
      // Find the currently active pre-defined overlay, and the one before it.
      ovl_selected = &ovl_root;
      for (ovl_current=ovl_root.next; ovl_current; ovl_current=ovl_current->next)
	{
	  if ( ovl_current->active )
	    {
	      ovl_selected = ovl_current;
	      ovl_previous = ovl_last;
	    }
	  ovl_last = ovl_current;
	}
      if ( next ) // up is like moving up in the file of overlay names
	{ // Make the selected ovl the next one in the list.
	  if ( ovl_selected->next )
	    ovl_selected = ovl_selected->next; // next overlay in the list
	  else
	    ovl_selected = ovl_root.next;      // first overlay in the list
	}
      else
	{ // Make the selected overlay the previous one in the list.
	  if ( ovl_selected != ovl_root.next ) // in this case, the root and current are the same
	    ovl_selected = ovl_previous;
	  else
	    ovl_selected = ovl_last;
	}
      for (ovl_current=ovl_root.next; ovl_current; ovl_current=ovl_current->next)
	ovl_current->active = FALSE;
      ovl_selected->active = TRUE;

      if ( !combine )
	{
	  // Copy data from the selected overlay into the root.
	  for (j=0; j<mosaic.stack->n3d; j++)
	    {
	      overlay_map.weight[j] = 0.;
	      overlay_map.color[j] = 0;
	    }
	  for (j=0; j<ovl_selected->npts; j++)
	    {
	      index = voxel3D_to_1D(ovl_selected->voxel[j]);
	      overlay_map.weight[index] = ovl_selected->weight[j];
	      overlay_map.color[index]  = ovl_selected->color;
	    }
	  update_root_overlay(0,1);
	  strcpy(ovl_root.name,ovl_selected->name);
	}
      else
	{ // Add to overlay.
	  for (j=0; j<ovl_selected->npts; j++)
	    {
	      index = voxel3D_to_1D(ovl_selected->voxel[j]);
	      // The greatest weight carries the color.
	      if ( ovl_selected->weight[j] > overlay_map.weight[index] )
		overlay_map.color[index] = ovl_selected->color;
	      // Add weights.
	      overlay_map.weight[index] += ovl_selected->weight[j];
	      overlay_map.weight[index] = MIN(1.,overlay_map.weight[index]);
	    }
	  update_root_overlay(0,1);
	  sprintf(ovl_root.name,"ovl%d",overlay_map.number);
	}
      // Update the x and y array.
      graph.update_regions |= UPDATE_GRAPH;
      show_on_info_line(0,0,ovl_selected->name,"","");
    }
  else if ( mosaic.toggle_overlays == 2 )
    { // use wire list, not overlay list
      // Find the last defined member of the list.
      for (wire_current=&wire_root; wire_current; wire_current=wire_current->next)
	wire_last = wire_current;
      // Now find the currently active member of the list.
      wire_previous = wire_last;  // give this a value, in case the root is active
      wire_current = &wire_root;
      while ( !(wire_current->active) )
	{
	  wire_previous = wire_current;
	  wire_current = wire_current->next;
	}
      if ( next ) // up is like moving up in the file of wire names
	{ // Make the selected wire the next one in the list.
	  if ( wire_current->next )
	    wire_selected = wire_current->next; // next wire in the list
	  else
	    wire_selected = wire_root.next;     // first wire in the list
	}
      else
	{ // Make the selected wire the previous one in the list.
	  if ( wire_current != wire_root.next ) // in this case, the root and current are the same
	    wire_selected = wire_previous;
	  else
	    wire_selected = wire_last;
	}
      wire_current->active = FALSE;
      wire_selected->active = TRUE;

      if ( combine )
	{ // Add wire frame to existing root.
	  control_save = control_vector(wire_root.npts + wire_selected->npts);
	  for (j=0; j<wire_root.npts; j++)
	    control_save[j] = wire_root.control[j];
	  for (j=0; j<wire_selected->npts; j++)
	    control_save[j+wire_root.npts] = wire_selected->control[j];
	  if ( wire_root.control ) free_control_vector(wire_root.control);
	  wire_root.npts    = wire_root.npts + wire_selected->npts;
	  wire_root.control = control_save;
	}
      else
	{ // Copy wire frame into root.
	  // Allocate space in the root structure.
	  if ( wire_root.control ) free_control_vector(wire_root.control);
	  wire_root.npts = wire_selected->npts;
	  wire_root.control = control_vector(wire_root.npts);
	  // Copy data from the selected wire into the root.
	  for (j=0; j<wire_selected->npts; j++)
	    wire_root.control[j] = wire_selected->control[j];
	  // Update the x and y array.
	  strcpy(wire_root.name,wire_selected->name);
	}
      show_on_info_line(0,0,wire_selected->name,"","");
    }
  mosaic.update_regions |= UPDATE_IMAGES;
}

void action_clear_overlay()
{
  JIP_INDEX j;

  for (j=0; j<mosaic.stack->n3d; j++)
    {
      overlay_map.weight[j] = 0.;
      overlay_map.color[j]  = 0;
    }
  ovl_root.npts = 0;
  update_root_overlay(1,1);
  if ( mosaic.cursor_selection <= CURSOR_ON_ORTHO_YZ )
    graph.update_regions |= UPDATE_GRAPH;
}

void action_clear_wires()
{
  wire_root.npts = 0;
  free_control_vector(wire_root.control);
  wire_root.control = NULL;
  sprintf(wire_root.name,"none");
}

void action_mirror_overlay()
{
  struct ovl_map overlay_mirror;
  IPOINT3D voxel, voxel_mirror;
  JIP_INDEX n3d, index, j;
  float weight;

  if ( ovl_root.npts == 0 )
    return;

  // Create a temporary overlay map.
  // This provides a simple way to keep track of selected voxels to aid reflection.
  n3d = jip_product(jip_product(mosaic.stack->dim.x,mosaic.stack->dim.y),mosaic.stack->dim.z);
  overlay_mirror.weight = float_vector(n3d);
  overlay_mirror.color  = short_vector(n3d);
  for (j=0; j<n3d; j++)
    {
      overlay_mirror.weight[j] = 0.;
      overlay_mirror.color[j]  = 0;
    }

  for (voxel.z=0; voxel.z<mosaic.stack->dim.z; voxel.z++)
    {
      for (voxel.y=0; voxel.y<mosaic.stack->dim.y; voxel.y++)
	{
	  for (voxel.x=0; voxel.x<mosaic.stack->dim.x; voxel.x++)
	    {
	      index  = voxel3D_to_1D(voxel);
	      weight = overlay_map.weight[index];
	      if ( weight != 0. )
		{
		  voxel_mirror   = voxel;
		  voxel_mirror.x = (mosaic.stack->dim.x-1) - voxel.x;
		  index = voxel3D_to_1D(voxel_mirror);
		  overlay_mirror.weight[index] = weight;
		  overlay_mirror.color[index]  = ovl_root.color;
		}
	    }
	}
    }

  // Copy the reflected overlay into the main overlay.
  for (j=0; j<n3d; j++)
    {
      overlay_map.weight[j] = overlay_mirror.weight[j];
      overlay_map.color[j]  = overlay_mirror.color[j];
    }
  free_float_vector(overlay_mirror.weight);
  free_short_vector(overlay_mirror.color);
}

void action_mirror_wires()
{
  POINT3D point;
  float x1, x2, xmin, xmax;
  JIP_INDEX jpt;

  if ( wire_root.npts == 0 )
    return;

  x1 = mosaic.stack->origin.x - mosaic.stack->resolution.x/2.; // origin is CENTER of voxel
  x2 = x1 + mosaic.stack->direction.x * mosaic.stack->fov.x;
  xmin = MIN(x1,x2);
  xmax = MAX(x1,x2);

  for ( jpt=0; jpt<wire_root.npts; jpt++ )
    {
      point = wire_root.control[jpt].point;
      point.x = xmax - (point.x - xmin);
      wire_root.control[jpt].point = point;
    }
}

void action_help(int mode, int button)
// mode:   0 = startup,    1 = interactive
// button: 1 = short help, 2 = longer help
{
  FILE *file;
  char *jip_home;
  char file_name[LEN_STR], string[LEN_STR];

  jip_home = getenv("JIP_HOME");
  if ( !jip_home )
    {
      printf("For help, quit and define the environment variable JIP_HOME, and then re-start.\n");
      return;
    }
  strcpy(file_name,jip_home);
  strcat(file_name,"/documentation/");
  strcat(file_name,program_name);
  if ( mode == 0 )
    strcat(file_name,"-startup.txt");
  else
    {
      if ( button == 1 )
	strcat(file_name,"-interactive-help1.txt");
      else
	strcat(file_name,"-interactive-help2.txt");
    }

  // Open the file containing the list of runs, GLM files, ...
  if ( !(file = fopen(file_name,"r")) )
    {
      printf("Error: cannot open the file %s.\n",file_name);
      printf("The environment variable JIP_HOME is set to %s\n",jip_home);
      printf("If this is incorrect, quit, define this environment variable, and re-start.\n");
      return;
    }

  printf("\n");
  while ( get_line_with_comment(file, string, LEN_STR) )
    {
      printf("%s",string);
    }
  fclose(file);
  return;
}

void action_set_projection_location()
{

  mosaic.icross.x = mosaic.icursor.x;
  mosaic.icross.y = mosaic.icursor.y;
  mosaic.icross.z = mosaic.icursor.z;
  graph.update_regions  |= UPDATE_GRAPH;
  mosaic.update_regions |= UPDATE_IMAGE_CROSSHAIRS;
}

void action_image_text_selection( int button )
{ // for selectable text
  struct wire_list *wire_list0;
  struct ovl_list  *ovl_list0;
  BOOL wire_enabled;

  if ( mosaic.text_selection == TEXTFIELD_NEW_OVL_WIRE )
    {
      wire_enabled = (mosaic.display_3orthogonal || mosaic.mosaic_orientation==ORTHO_XY);
      if ( wire_enabled && mosaic.activation.visible )
	{
	  if ( button == 1 && mosaic.activation.visible )
	    action_key_n();  // overlay based upon activation
	  else
	    action_key_N();  // new wire frame
	}
      else if ( wire_enabled )
	action_key_N();  // new wire frame
      else if ( mosaic.activation.visible )
	action_key_n();  // overlay based upon activation
    }
  else if ( mosaic.text_selection == TEXTFIELD_READ_OVL_WIRE )
    {
      if ( button == 1 )
	action_key_r();
      else
	action_key_R();
    }
  else if ( mosaic.text_selection == TEXTFIELD_WRITE_OVL_WIRE )
    {
      if ( button == 1 )
	action_key_w();
      else
	action_key_W();
    }
  else if ( mosaic.text_selection == TEXTFIELD_DEL_OVL_WIRE )
    {
      if ( button == 1 )
	action_key_d();
      else
	action_key_D();
    }
  else if ( mosaic.text_selection == TEXTFIELD_COLOR_OVL_WIRE )
    {
      if ( button == 1 )
	action_key_c();
      else
	action_key_C();
    }
  else if ( mosaic.text_selection == TEXTFIELD_MIRROR_OVL_WIRE )
    {
      if ( button == 1 )
	action_key_m();
      else
	action_key_M();
    }
  else if ( mosaic.text_selection == TEXTFIELD_FILL_WIRE )
    action_key_F();
  else if ( mosaic.text_selection == TEXTFIELD_OVL_STATS )
    {
      if ( button == 1 )
	{ // 's'
	  fill_graph_vector(1);
	  write_roi_stats(0);
	}
      else
	{ // 'S'
	  if ( ovl_root.npts != 0 )
	    {
	      fill_graph_vector(1);
	      write_roi_stats(1);
	    }
	}
    }
  else if ( mosaic.text_selection == TEXTFIELD_TRANSPARENT )
    action_key_T();
  else if ( mosaic.text_selection == TEXTFIELD_IMAGE_LEVEL )
    action_key_i();
  else if ( mosaic.text_selection == TEXTFIELD_IMAGE_ZOOM )
    {
      if ( button == 1 )
	action_key_z();
      else
	action_key_Z();
    }
  else if ( mosaic.text_selection == TEXTFIELD_IMAGE_CROSS )
    action_key_control_c(button);
  else if ( mosaic.text_selection == TEXTFIELD_IMAGE_INTERP )
    action_key_control_i();
  else if ( mosaic.text_selection == TEXTFIELD_IMAGE_GRAPH )
    action_bring_up_graph();
  else if ( mosaic.text_selection == TEXTFIELD_HELP )
    action_help(1, button);
  else if ( mosaic.text_selection == TEXTFIELD_QUIT )
    action_quit();
  else if ( mosaic.text_selection == TEXTFIELD_QUESTION )
    action_key_question_mark();
  else if ( mosaic.text_selection == TEXTFIELD_POUND_SIGN )
    action_key_pound_sign();
  else if ( mosaic.text_selection == TEXTFIELD_DATA_TYPE )
    {
      if ( mosaic.stack->data_type != 0  )
	{
	  if ( button == 1 )
	    {
	      mosaic.display_data_type++;
	      if ( mosaic.display_data_type > 3 ) mosaic.display_data_type = 0;
	    }
	  else
	    {
	      mosaic.display_data_type--;
	      if ( mosaic.display_data_type < 0 ) mosaic.display_data_type = 3;
	    }
	  contrast_low = 0.0;
	  contrast_high = 1.0;
	  rescale_image(1);
	  graph.update_regions |= UPDATE_GRAPH;
	}
    }
  else if ( mosaic.text_selection == TEXTFIELD_OVL_OR_WIRE )
    {
      ovl_list0  = ovl_root.next;   // 1st overlay in list
      wire_list0 = wire_root.next;  // 1st wire frame in list
      if ( mosaic.toggle_overlays == 1 && wire_list0 != NULL )
	mosaic.toggle_overlays = 2; // go to wire frames
      else if ( mosaic.toggle_overlays == 2 && ovl_list0 != NULL )
	mosaic.toggle_overlays = 1; // go to overlays
      mosaic.update_regions |= UPDATE_SELECT_TEXT_IMAGE;
    }
  else if ( mosaic.text_selection == TEXTFIELD_WHICH_OVL_WIRE )
    {
      if ( button == 1 )
	action_step_overlay_list(1, 0);
      else
	action_step_overlay_list(0, 0);
      mosaic.update_regions |= UPDATE_SELECT_TEXT_IMAGE;
    }
  else if ( mosaic.text_selection == TEXTFIELD_MOSAIC_3PLANE )
    {
      mosaic.display_3orthogonal = !mosaic.display_3orthogonal;
      if ( mosaic.display_3orthogonal )
	{
	  mosaic.zoom++;
	}
      else
	{
	  mosaic.zoom--;
	  mosaic.zoom = MAX(1,mosaic.zoom);
	}
      update_XWindow_image();
    }
  else if ( mosaic.text_selection == TEXTFIELD_MOSAIC_ORIENT )
    {
      if ( button == 1 )
	{
	  if ( mosaic.mosaic_orientation == ORTHO_XY )
	    mosaic.mosaic_orientation = ORTHO_XZ;
	  else if ( mosaic.mosaic_orientation == ORTHO_XZ )
	    {
	      if ( mosaic.display_ortho_ZY )
		mosaic.mosaic_orientation = ORTHO_ZY;
	      else
		mosaic.mosaic_orientation = ORTHO_YZ;
	    }
	  else // y-z or z-y
	    mosaic.mosaic_orientation = ORTHO_XY;
	}
      else
	{
	  if ( mosaic.mosaic_orientation == ORTHO_XY )
	    {
	      if ( mosaic.display_ortho_ZY )
		mosaic.mosaic_orientation = ORTHO_ZY;
	      else
		mosaic.mosaic_orientation = ORTHO_YZ;
	    }
	  else if ( mosaic.mosaic_orientation == ORTHO_XZ )
	    mosaic.mosaic_orientation = ORTHO_XY;
	  else // y-z or z-y
	    mosaic.mosaic_orientation = ORTHO_XZ;
	}
      update_XWindow_image();
    }
  else if ( mosaic.text_selection == TEXTFIELD_XPANELS )
    {
      if ( mosaic.display_3orthogonal )
	{
	  mosaic.display_ortho_ZY = !mosaic.display_ortho_ZY;
	  if ( mosaic.mosaic_orientation == ORTHO_YZ )
	    mosaic.mosaic_orientation = ORTHO_ZY;
	  else if ( mosaic.mosaic_orientation == ORTHO_ZY )
	    mosaic.mosaic_orientation = ORTHO_YZ;
	}
      else
	{
	  if ( button == 1 )
	    {
	      mosaic.DimPanels.x++;
	      while ( ((JIP_INDEX)(mosaic.DimPanels.x)) * (mosaic.DimPanels.y-1) >= mosaic.DimPanels.z )
		mosaic.DimPanels.y--;
	    }
	  else
	    {
	      mosaic.DimPanels.x = MAX(1,mosaic.DimPanels.x-1);
	      while ( ((JIP_INDEX)(mosaic.DimPanels.x)) * mosaic.DimPanels.y < mosaic.DimPanels.z )
		mosaic.DimPanels.y++;
	    }
	  mosaic.DimPanels.x = MIN(mosaic.DimPanels.x,mosaic.DimPanels.z);
	  mosaic.DimPanels.y = MIN(mosaic.DimPanels.y,mosaic.DimPanels.z);
	}
      update_XWindow_image();
    }
  else if ( mosaic.text_selection == TEXTFIELD_COLORBAR_TAILP )
    {
      if ( button == 1 )
	{
	  if ( mosaic.activation.color_scheme != 0 ) // 1 tail showing; go back to 2 tails
	    mosaic.activation.color_scheme = 0;
	  else                                      // both tails showing; select + tail
	    mosaic.activation.color_scheme = 1;
	}
      else
	{
	  if ( mosaic.activation.color_scheme == 0 )
	    mosaic.activation.color_sign *= -1;   // switch signs
	  else
	    mosaic.activation.color_scheme *= -1; // 1->-1,  2->-2
	}
      mosaic.update_regions |= UPDATE_COLORBAR;
      mosaic.update_regions |= UPDATE_IMAGES;
      mosaic.update_regions |= UPDATE_SELECT_TEXT_IMAGE;
    }
  else if ( mosaic.text_selection == TEXTFIELD_COLORBAR_TAILN )
    {
      if ( button == 1 )
	{
	  if ( mosaic.activation.color_scheme != 0 ) // 1 tail showing; go back to 2 tails
	    mosaic.activation.color_scheme = 0;
	  else                                       // both tails showing; select - tail
	    mosaic.activation.color_scheme = -1;
	}
      else
	{
	  if ( mosaic.activation.color_scheme == 0 )
	    mosaic.activation.color_sign *= -1;   // switch signs
	  else
	    mosaic.activation.color_scheme *= -1; // -1->1, -2->2
	}
      mosaic.update_regions |= UPDATE_COLORBAR;
      mosaic.update_regions |= UPDATE_IMAGES;
      mosaic.update_regions |= UPDATE_SELECT_TEXT_IMAGE;
    }
  else if ( mosaic.cursor_selection&CURSOR_ON_SELECT_TEXT )
    action_image_text_selection_specific( button );
}

float get_activation_threshold( int low_or_high )
{
  if ( low_or_high == -1 )
    { // low threshold
      if ( mosaic.activation.scale_log )
	return(mosaic.activation.thresh_log_low);
      else
	return(mosaic.activation.thresh_lin_low);
    }
  else
    { // high threshold
      if ( mosaic.activation.scale_log )
	return(mosaic.activation.thresh_log_high);
      else
	return(mosaic.activation.thresh_lin_high);
    }
}

void set_activation_threshold( int low_or_high, float value )
{
  if ( low_or_high == -1 )
    { // low threshold
      if ( mosaic.activation.scale_log )
	{
	  mosaic.activation.thresh_log_low = value;
	  if ( mosaic.activation.thresh_log_low < 0. )
	    mosaic.activation.thresh_log_low = 0.;
	}
      else
	{
	  mosaic.activation.thresh_lin_low = value;
	  if ( mosaic.activation.color_scheme == 0 && mosaic.activation.thresh_lin_low < 0. )
	    mosaic.activation.thresh_lin_low = 0.;
	}
    }
  else
    { // high threshold
      if ( mosaic.activation.scale_log )
	mosaic.activation.thresh_log_high = value;
      else
	mosaic.activation.thresh_lin_high = value;
    }
  mosaic.update_regions |= UPDATE_IMAGES;
  mosaic.update_regions |= UPDATE_COLORBAR_LABELS;
  mosaic.update_regions |= UPDATE_SELECT_TEXT_IMAGE;
}

void change_activation_threshold( int low_or_high, float change )
{
  if ( low_or_high == -1 )
    { // low threshold
      if ( mosaic.activation.scale_log )
	{
	  mosaic.activation.thresh_log_low += change;
	  if ( change >= 1. )
	    mosaic.activation.thresh_log_low = (int)mosaic.activation.thresh_log_low;
	  if ( mosaic.activation.thresh_log_low >= mosaic.activation.thresh_log_high )
	    mosaic.activation.thresh_log_low -= 1.;
	  if ( mosaic.activation.thresh_log_low < 0. )
	    mosaic.activation.thresh_log_low = 0.;
	}
      else
	{
	  mosaic.activation.thresh_lin_low += change;
	  if ( change >= 1. )
	    mosaic.activation.thresh_lin_low = (int)mosaic.activation.thresh_lin_low;
	  if ( mosaic.activation.thresh_lin_low >= mosaic.activation.thresh_lin_high )
	    mosaic.activation.thresh_lin_low -= 1.;
	  if ( mosaic.activation.color_scheme == 0 && mosaic.activation.thresh_lin_low < 0. )
	    mosaic.activation.thresh_lin_low = 0.;
	}
    }
  else
    { // high threshold
      if ( mosaic.activation.scale_log )
	{
	  mosaic.activation.thresh_log_high += change;
	  if ( change >= 1. )
	    mosaic.activation.thresh_log_high = (int)mosaic.activation.thresh_log_high;
	  if ( mosaic.activation.thresh_log_high <= mosaic.activation.thresh_log_low )
	    mosaic.activation.thresh_log_high += 1.;
	}
      else
	{
	  mosaic.activation.thresh_lin_high += change;
	  if ( change >= 1. )
	    mosaic.activation.thresh_lin_high = (int)mosaic.activation.thresh_lin_high;
	  if ( mosaic.activation.thresh_lin_high <= mosaic.activation.thresh_lin_low )
	    mosaic.activation.thresh_lin_high += 1.;
	}
    }
  mosaic.update_regions |= UPDATE_IMAGES;
  mosaic.update_regions |= UPDATE_COLORBAR_LABELS;
  mosaic.update_regions |= UPDATE_SELECT_TEXT_IMAGE;
}

void change_grayscale_range( int low_or_high, float change )
{
  if ( low_or_high == -1 )
    { // low threshold
      image_scale_min += change;
      if ( image_scale_min >= image_scale_max )
	image_scale_min = image_scale_max;;
    }
  else
    { // high threshold
      image_scale_max += change;
      if ( image_scale_max <= image_scale_min )
	image_scale_max = image_scale_min;;
    }
  mosaic.update_regions |= UPDATE_IMAGES;
  mosaic.update_regions |= UPDATE_COLORBAR_LABELS;
  mosaic.update_regions |= UPDATE_SELECT_TEXT_IMAGE;
}

void action_rescale_image_from_graph()
{
  float xscale;

  xscale = (float)(graph.npoints-1) / (graph.xmax-graph.xmin);
  image_scale_min = graph.xmin + (float)graph.ix_low_select / xscale;
  image_scale_max = graph.xmin + (float)graph.ix_high_select/ xscale;
  rescale_image_settings();
}

void action_graph_motion_notify()
{ // make common function
  static int last_cursor_selection = -1, last_text_selection = -1;

  if ( graph.cursor_selection&CURSOR_ON_DATA )
    graph.update_regions |= UPDATE_CURSOR_GRAPH;

  // Update the image window if the cursor changes positions
  if ( graph.cursor_selection != last_cursor_selection ||
       graph.text_selection   != last_text_selection )
    graph.update_regions |= UPDATE_SELECT_TEXT_GRAPH;

  last_cursor_selection = graph.cursor_selection;
  last_text_selection   = graph.text_selection;
}

void action_new_graph_type( int button )
{

  if ( button == 1 )
    {
      graph.type++;
      if ( graph.type > 4 )
	graph.type = 0;
    }
  else if ( button == 2 )
    {
      graph.type--;
      if ( graph.type < 0 )
	graph.type = 4;
      //      update_graph_selectable_text();
    }
  else
    make_XWindow_graph();

  initialize_graph(); // type 3 = time graph
  mosaic.update_regions |= UPDATE_IMAGES;
  graph.update_regions  |= UPDATE_GRAPH;
}

void action_graph_text_selection( int button )
{ // for selectable text
  int increment, j;
  float fLow, fHigh;
  char PromptString[LEN_STR], InputString[LEN_STR];

  if ( graph.text_selection == TEXTFIELD_GRAPH_TYPE )   // x, y, z, time, ...
    action_new_graph_type(button);
  else if ( graph.text_selection == TEXTFIELD_GRAPH_UNITS )   // like 'u'
    action_change_graph_units();
  else if ( graph.text_selection == TEXTFIELD_GRAPH_HIST_CLIP )
    {
      mosaic.clipping_mode = !mosaic.clipping_mode;
    }
  else if ( graph.text_selection == TEXTFIELD_GRAPH_AUTORANGE )   // like 'r'
    {
      graph.yrange_fixed = !graph.yrange_fixed;
      build_graph_display(1);
    }
  else if ( graph.text_selection == TEXTFIELD_GRAPH_YLOW ||
	    graph.text_selection == TEXTFIELD_GRAPH_YHIGH )
    {
      sprintf(PromptString,"Input low & high range");
      if ( get_X11_string(2,PromptString,"separate values with space",
			  "",0,1,InputString) )
	{
	  if ( sscanf(InputString,"%g %g",&fLow,&fHigh) == 2 )
	    {
	      if ( fLow < fHigh )
		{
		  graph.ymin = fLow;
		  graph.ymax = fHigh;
		}
	      else
		{
		  graph.ymin = fHigh;
		  graph.ymax = fLow;
		}
	      graph.yrange_fixed = TRUE;
	      build_graph_display(1);
	    }
	}
    }
  else if ( graph.text_selection == TEXTFIELD_GRAPH_BIN )
    {
      graph.bin_data = !graph.bin_data;
      if ( !graph.bin_data && !(graph.bin_width%2) ) // smooth by odd numbers
	graph.bin_width++;
    }
  else if ( graph.text_selection == TEXTFIELD_GRAPH_BIN_VALUE )
    {
      if ( graph.type == 3 )
	{ // time graph
	  if ( graph.bin_data )
	    increment = 1;
	  else
	    increment = 2;
	  if ( button == 1 )
	    graph.bin_width += increment;
	  else
	    graph.bin_width -= increment;
	  if ( graph.bin_width < 1 ) graph.bin_width = 1;
	  if ( !graph.bin_data && !(graph.bin_width%2) ) // smooth by odd numbers
	    graph.bin_width++;
	}
      else if ( graph.type == 4 )
	{ // histogram graph
	  if ( button == 1 )
	    graph.histogram_bins -= 10;
	  else
	    graph.histogram_bins += 10;
	  graph.histogram_bins = MAX(10,graph.histogram_bins);
	  // Allocate &n intialize arrays
	  graph.npoints = graph.histogram_bins;
	  graph.xVector   = float_vector(graph.npoints);
	  graph.yVector   = float_vector(graph.npoints);
	  graph.yVector1  = float_vector(graph.npoints);
	  graph.yFit      = float_vector(graph.npoints);
	  for (j=0; j<graph.npoints; j++)
	    graph.xVector[j] = graph.yVector[j] = graph.yFit[j] = graph.yVector1[j] = 0.;
	}
    }
  else
    action_graph_text_selection_specific( button );
  graph.update_regions |= UPDATE_GRAPH;
  graph.update_regions |= UPDATE_SELECT_TEXT_GRAPH;
}

void action_graph_button1( int ixvalue )
{
  if ( graph.cursor_selection&CURSOR_ON_DATA )
    {
      set_graph_location(ixvalue);
      mosaic.update_regions |= UPDATE_IMAGES;
      mosaic.update_regions |= UPDATE_INFO_AREA_IMAGE;
      build_graph_display(0);
    }
  else if ( graph.cursor_selection&CURSOR_ON_SELECT_TEXT )
    action_graph_text_selection(1);
}

void action_graph_button23( int press, int ixvalue )
{
  int iswitch;

  if ( press )
    { // button press
      if ( graph.cursor_selection&CURSOR_ON_DATA )
	graph.ix_low_select = ixvalue;
      else if ( graph.cursor_selection&CURSOR_ON_SELECT_TEXT )
	action_graph_text_selection(2);
    }
  else if ( graph.cursor_selection&CURSOR_ON_DATA )
    { // button release
      graph.ix_high_select = ixvalue;
      if ( graph.ix_high_select < graph.ix_low_select)
	{ // switch high and low
	  iswitch = graph.ix_high_select;
	  graph.ix_high_select = graph.ix_low_select;
	  graph.ix_low_select = iswitch;
	}
      if ( graph.type < 3 )
	graph.update_regions |= UPDATE_GRAPH;
      else if ( graph.type == 3 )
	{
	  action_graph_button23_TimeSeriesModel();
	  graph.update_regions |= UPDATE_GRAPH;
	}
      else if ( graph.type == 4 )
	{
	  action_rescale_image_from_graph();
	  graph.update_regions  |= UPDATE_GRAPH;
	  mosaic.update_regions |= UPDATE_IMAGES;
	  mosaic.update_regions |= UPDATE_COLORBAR_LABELS;
	}
    }
}

void action_graph_arrow_left_right( int left )
{

  if ( graph.cursor_selection&CURSOR_ON_DATA )
    {
      if ( left )
	decrement_graph_location(0);
      else
	increment_graph_location(0);
      build_graph_display(0);
    }
  else if ( graph.cursor_selection&CURSOR_ON_SELECT_TEXT )
    {
      if ( left )
	action_graph_text_selection(1);
      else
	action_graph_text_selection(2);
    }
  mosaic.update_regions |= UPDATE_IMAGES;
  mosaic.update_regions |= UPDATE_INFO_AREA_IMAGE;
  graph.update_regions  |= UPDATE_GRAPH;
}

void action_graph_arrow_up_down( int up, float yvalue )
{
  float range, div;
  
  if ( up )
    {
      if ( graph.units_percent )
	{
	  if ( yvalue > (graph.ymin + graph.ymax)/2. )
	    {
	      if ( fabs((double)graph.ymax) < 10. )
		graph.ymax += 1.;
	      else
		{
		  graph.ymax += 5.;
		  graph.ymax = 5. * (int)(graph.ymax/5.);
		}
	    }
	  else
	    {
	      if ( fabs((double)graph.ymin) < 10. )
		graph.ymin += 1.;
	      else
		{
		  graph.ymin += 5.;
		  graph.ymin = 5. * (int)(graph.ymin/5.);
		}
	    }
	}
      else
	{
	  range = graph.ymax - graph.ymin;
	  if ( yvalue > (graph.ymin + graph.ymax)/2. )
	    {
	      graph.ymax += range*0.10;
	      div = (int)(log10(fabs((double)graph.ymin))-2.);
	      div = pow(10.,(double)div);
	      graph.ymax = div * (int)(graph.ymax/div);
	    }
	  else
	    {
	      graph.ymin += range*0.10;
	      div = (int)(log10(fabs((double)graph.ymin))-2.);
	      div = pow(10.,(double)div);
	      graph.ymin = div * (int)(graph.ymin/div);
	    }
	}
    }
  else
    {
      if ( graph.units_percent )
	{
	  if ( yvalue > (graph.ymin + graph.ymax)/2. )
	    {
	      if ( fabs((double)graph.ymax) < 10. )
		graph.ymax -= 1.;
	      else
		{
		  graph.ymax -= 5.;
		  graph.ymax = 5. * (int)(graph.ymax/5.);
		}
	    }
	  else
	    {
	      if ( fabs((double)graph.ymin) < 10. )
		graph.ymin -= 1.;
	      else
		{
		  graph.ymin -= 5.;
		  graph.ymin = 5. * (int)(graph.ymin/5.);
		}
	    }
	}
      else
	{
	  range = graph.ymax - graph.ymin;
	  if ( yvalue > (graph.ymin + graph.ymax)/2. )
	    {
	      graph.ymax -= range*0.10;
	      div = (int)(log10(fabs((double)graph.ymin))-2.);
	      div = pow(10.,(double)div);
	      graph.ymax = div * (int)(graph.ymax/div);
	    }
	  else
	    {
	      graph.ymin -= range*0.10;
	      div = (int)(log10(fabs((double)graph.ymin))-2.);
	      div = pow(10.,(double)div);
	      graph.ymin = div * (int)(graph.ymin/div);
	    }
	}
    }
  graph.yrange_fixed = 1;
  build_graph_display(1);
}

void action_change_graph_units()
{
  if ( graph.type == 3 )
    {
      graph.units_percent = !graph.units_percent;
      graph.yrange_fixed = 0;
      graph.update_regions |= UPDATE_GRAPH;
    }
}

void action_ColorBar_button1()
{ // xdisplay only
  float low_value, high_value, range, yvalue;

  if ( mosaic.activation.visible )
    {
      low_value  = get_activation_threshold(-1);
      high_value = get_activation_threshold(1);
    }
  else
    {
      low_value  = image_scale_min;
      high_value = image_scale_max;
    }
  range  = high_value - low_value;
  yvalue = high_value - range * mosaic.cursor.y/(float)(mosaic.ImageRegionPixels.y);

  if ( mosaic.cursor_selection&CURSOR_ON_COLORBAR_HIGH )
    {
      if ( mosaic.activation.visible)
	set_activation_threshold(1,yvalue);
      else
	image_scale_max = yvalue;
    }
  else
    {
      if ( mosaic.activation.visible)
	set_activation_threshold(-1,yvalue);
      else
	image_scale_min = yvalue;
    }

  mosaic.update_regions |= UPDATE_IMAGES;
  mosaic.update_regions |= UPDATE_COLORBAR_LABELS;
  mosaic.update_regions |= UPDATE_SELECT_TEXT_IMAGE;
}

void action_ColorBar_cursor()
{
  // show_information_colorbar();
}

void action_ColorBar_button23()
{ // xdisplay only
  float low_value, high_value, range, change;

  if ( mosaic.activation.visible )
    {
      low_value  = get_activation_threshold(-1);
      high_value = get_activation_threshold(1);
    }
  else
    {
      low_value  = image_scale_min;
      high_value = image_scale_max;
    }
  range  = high_value - low_value;
  change = range/10.;
  if ( change < 1. ) change = 1.;

  if ( mosaic.cursor_selection&CURSOR_ON_COLORBAR_HIGH )
    {
      if ( mosaic.activation.visible )
	change_activation_threshold( 1,change );
      else
	change_grayscale_range( 1,change );
    }
  else
    {
      if ( mosaic.activation.visible )
	change_activation_threshold( -1,-change );
      else
	change_grayscale_range( -1,-change );
    }
}

void action_ColorBar_arrow_up_down( int up )
{ // xdisplay only
  float low_value, high_value, range, step;
  int istep;

  if ( mosaic.activation.visible )
    {
      low_value  = get_activation_threshold(-1);
      high_value = get_activation_threshold(1);
    }
  else
    {
      low_value  = image_scale_min;
      high_value = image_scale_max;
    }
  range = high_value - low_value;

  if ( range == 1. )
    step = 0.1;
  else if ( range > 1. )
    {
      istep = (int)(log10((double)range)-1.);
      istep = (int)pow(10.,(double)istep);
      if ( istep <= 1 ) istep = 1;
      step = istep;
    }
  else
    {
      step  = log10((double)range)-1.;
      istep = step;
      step = pow(10.,(double)istep);
      step = fabs((double)step);
    }

  if ( up )
    {
      if ( mosaic.cursor_selection&CURSOR_ON_COLORBAR_HIGH )
	{
	  if ( mosaic.activation.visible )
	    change_activation_threshold( 1, step );
	  else
	    change_grayscale_range( 1, step );
	}
      else
	{
	  if ( mosaic.activation.visible )
	    change_activation_threshold( -1, step );
	  else
	    change_grayscale_range( -1, step );
	}
    }
  else
    {
      if ( mosaic.cursor_selection&CURSOR_ON_COLORBAR_HIGH )
	{
	  if ( mosaic.activation.visible )
	    change_activation_threshold( 1, -step );
	  else
	    change_grayscale_range( 1, -step );
	}
      else
	{
	  if ( mosaic.activation.visible )
	    change_activation_threshold( -1, -step );
	  else
	    change_grayscale_range( -1, -step );
	}
    }
}

void action_ColorBar_arrow_left_right( int left )
{ // xdisplay only
  if ( left )
    {
      mosaic.activation.color_scheme--;
      if ( mosaic.activation.color_scheme < -2 ) mosaic.activation.color_scheme = 2;
    }
  else
    {
      mosaic.activation.color_scheme++;
      if ( mosaic.activation.color_scheme > 2 ) mosaic.activation.color_scheme = -2;
    }
  set_activation_threshold( -1,get_activation_threshold(-1) ); // to enforce 0 limit on linear maps
  mosaic.update_regions |= UPDATE_COLORBAR;
}

void action_select_activated_region()
{
  BOOL activated, ignore;
  IPOINT3D jRel, voxel;
  int expand;JIP_INDEX index1d, nvoxels, nvoxels_save, j;
  float value, sign;

  index1d = voxel3D_to_1D(mosaic.icross);
  value = mosaic.activation.fSpaceMatrix[index1d];
  if ( value < 0 )
    sign = -1.;
  else
    sign = 1.;
  if ( sign*value < get_activation_threshold(-1) )
    {
      fprintf(stdout,"Center voxel not activated; function canceled.\n");
      fflush(stdout);
      return;
    }

  // Initialize the overlay map
  for (j=0; j<mosaic.stack->n3d; j++)
    {
      overlay_map.weight[j] = 0.;
      overlay_map.color[j] = 0;
    }
  overlay_map.weight[index1d] = 1.;

  nvoxels = 1;
  nvoxels_save = expand = 0;
  while ( nvoxels > nvoxels_save )
    {
      nvoxels_save = nvoxels;
      // Increment the expansion range.
      expand++;
      for (jRel.z=-expand; jRel.z<=expand; jRel.z++)
	{
	  voxel.z = mosaic.icross.z + jRel.z;
	  for (jRel.y=-expand; jRel.y<=expand; jRel.y++)
	    {
	      voxel.y = mosaic.icross.y + jRel.y;
	      for (jRel.x=-expand; jRel.x<=expand; jRel.x++)
		{
		  voxel.x = mosaic.icross.x + jRel.x;
		  // Ignore indices that are out of bounds.
		  ignore = (voxel.x < 0 || voxel.x >= mosaic.stack->dim.x )
		    ||     (voxel.y < 0 || voxel.y >= mosaic.stack->dim.y )
		    ||     (voxel.z < 0 || voxel.z >= mosaic.stack->dim.z );
		  // Ignore center voxel.
		  ignore |= (jRel.x==0 && jRel.y==0 && jRel.z==0);
		  if ( !ignore )
		    {
		      index1d = voxel3D_to_1D(voxel);
		      // Ignore voxels that have already been identified.
		      ignore = overlay_map.weight[index1d] != 0.;
		      if ( !ignore )
			{
			  // Ignore voxels that do not touch other activated voxels.
			  ignore = !voxel_touches_activated_neighbor(voxel);
			  if ( !ignore )
			    {
			      value = mosaic.activation.fSpaceMatrix[index1d];
			      activated = sign*value > get_activation_threshold(-1);
			      if ( activated )
				{
				  overlay_map.weight[index1d] = 1.;
				  overlay_map.color[index1d]  = ovl_root.color;
				  nvoxels++;
				}
			    }
			}
		    }
		}
	    }
	}
    }
  update_root_overlay(1,1);

  return;
}

BOOL voxel_touches_activated_neighbor(IPOINT3D voxel)
{
  IPOINT3D neighbor, jRel;
  BOOL touch, ignore;
  JIP_INDEX index1d;

  touch = FALSE;
  for (jRel.z=-1; jRel.z<=1; jRel.z++)
    {
      neighbor.z = voxel.z + jRel.z;
      for (jRel.y=-1; jRel.y<=1; jRel.y++)
	{
	  neighbor.y = voxel.y + jRel.y;
	  for (jRel.x=-1; jRel.x<=1; jRel.x++)
	    {
	      neighbor.x = voxel.x + jRel.x;
	      // Ignore indices that are out of bounds.
	      ignore = (neighbor.x < 0 || neighbor.x >= mosaic.stack->dim.x )
		||     (neighbor.y < 0 || neighbor.y >= mosaic.stack->dim.y )
		||     (neighbor.z < 0 || neighbor.z >= mosaic.stack->dim.z );
	      // Ignore center voxel.
	      ignore |= (jRel.x==0 && jRel.y==0 && jRel.z==0);
	      // Require a shared border, not just a point connection.
	      ignore |= (jRel.x * jRel.y * jRel.z != 0);
	      if ( !ignore )
		{
		  // Does the current voxel touch any activated neighbor?
		  index1d = voxel3D_to_1D(neighbor);
		  touch |= overlay_map.weight[index1d] != 0.;
		}
	    }
	}
    }
  return(touch);
}
