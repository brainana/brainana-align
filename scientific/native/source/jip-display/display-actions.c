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
#include "display.h"
#include "glm-prototypes.h"


void action_write_all_ROIs()
{
  write_ovl_list_into_roi_file(ROI_file_name);
  label_graph_window();
}

void action_write_1_ROI()
{
  if ( graph.type == 0 || graph.type == 1 || graph.type == 2 || graph.type == 4)
    write_projection(ROI_file_name);
  else
    {
      if ( ovl_root.npts != 0 )
	{
	  fill_graph_vector(1);
	  build_graph_display(1);
	  if ( !write_ovl_into_roi_file(ROI_file_name) )
	    overlay_map.number++;
	}
    }
  label_graph_window();
}

void action_key_l()
{
  // mosaic.display_grid: 0 = none, 1 = labels, 2 = ticks, 3 = both
  if ( mosaic.display_grid == 0 )
    mosaic.display_grid = 1;
  else if ( mosaic.display_grid == 1 )
    mosaic.display_grid = 0;
  else if ( mosaic.display_grid == 2)
    mosaic.display_grid = 3;
  else if ( mosaic.display_grid == 3)
    mosaic.display_grid = 2;
  mosaic.update_regions |= UPDATE_IMAGE_TEXT_LINES;
}

void action_key_L()
{
  // mosaic.display_grid: 0 = none, 1 = labels, 2 = ticks, 3 = both
  if ( mosaic.display_grid == 0 )
    mosaic.display_grid = 2;
  else if ( mosaic.display_grid == 2 )
    mosaic.display_grid = 0;
  else if ( mosaic.display_grid == 1)
    mosaic.display_grid = 3;
  else if ( mosaic.display_grid == 3)
    mosaic.display_grid = 1;
  mosaic.update_regions |= UPDATE_IMAGE_TEXT_LINES;
}

void action_display_last_time_point()
{
  int itime;

  build_graph_display(1);
  itime = graph.iTime;
  graph.iTime = graph.iLastTime;
  graph.iLastTime = itime;
  if ( graph.iTime < 0 )
    { // Start with excluded images
      itime = graph.iTime + anatomy_stack.nt;
      mosaic.fSpaceMatrix = &(anatomy_stack.SpaceTime.fData[itime*mosaic.stack->npts_volume]);
    }
  else
    { // The time point falls within the GLM
      itime = graph.iTime;
      mosaic.fSpaceMatrix = &(mosaic.stack->SpaceTime.fData[itime*mosaic.stack->npts_volume]);
    }
  mosaic.update_regions |= UPDATE_IMAGES;
  mosaic.update_regions |= UPDATE_INFO_AREA_IMAGE;
  graph.update_regions  |= UPDATE_GRAPH;
  Update_Regions();
}

void action_quit_specific()
{
}

void action_RightPanel_button1()
{
}

void action_RightPanel_cursor()
{
}

void action_RightPanel_button23()
{
}

void action_RightPanel_arrow_up_down( int up )
{
}

void action_RightPanel_arrow_left_right( int left )
{
}

void action_RightPanel_tab()
{
}

void update_right_panel()
{
}

void action_fit_psf( int itype )
// itype: 0 = Gaussian, 1 = 1/r^2
{
  double *xarray, *yarray;
  float x0, amp, sigma, yoff, arg;
  int error;JIP_INDEX j;JIP_INDEX npoints;JIP_INDEX index;

  for (j=0; j<graph.npoints; j++)
    graph.yFit[j] = 0.;
  if ( graph.ix_low_select < 0 || graph.ix_low_select >= graph.ix_high_select )
    {
      show_on_info_line(0,0,"graph range not selected","","");
      return;
    }
  npoints = graph.ix_high_select - graph.ix_low_select + 1;

  xarray  = double_vector(npoints);
  yarray  = double_vector(npoints);

  // Select the best starting point for
  // 1) amp = maximum value of array over specified range
  // 2) yoff = average value of array over specified range
  amp = yoff = 0.;
  for (j=0; j<npoints; j++)
    {
      index = graph.ix_low_select + j;
      if ( index < 0 || index > graph.npoints - 1 )
	{
	  printf("Programming error: index out of range in action_fit_gaussian().\n");
	  exit(1);
	}
      xarray[j]  = index;
      yarray[j]  = graph.yVector[index];
      if ( fabs((double)(yarray[j])) > fabs((double)amp) )
	amp = yarray[j];
      yoff += yarray[j];
    }
  yoff /= (float)npoints; // set y offset to average value
  if ( graph.type == 0 )
    x0 = mosaic.icross.x;      // current line-cursor location
  else if ( graph.type == 1 )
    x0 = mosaic.icross.y;
  else if ( graph.type == 2 )
    x0 = mosaic.icross.z;
  else if ( graph.type == 3 )
    x0 = graph.iTime;
  else if ( graph.type == 4 )
    {
      printf("Programming error: graph type = 4 in function set_graph_location().\n");
      exit(1);
    }
  sigma  = (float)(graph.ix_high_select - graph.ix_low_select)/8.;
  
  if ( itype == 0 )
    error = fit_gaussian(jip_int_count(npoints),xarray,yarray,&x0,&amp,&sigma,&yoff);
  else
    error = fit_1_over_r2(jip_int_count(npoints),xarray,yarray,&x0,&amp,&sigma,&yoff);

  if ( error )
    printf("Error fitting Gaussian\n");
  else
    {
      for (j=0; j<graph.npoints; j++)
	{
	  if ( itype == 0 )
	    {
	      arg = (j-x0) * (j-x0) / 2. / sigma / sigma;
	      //	      graph.yFit[j] = yoff + amp * exp(-(double)arg);
	      graph.yFit[j] = amp * exp(-(double)arg);
	    }
	  else
	    {
	      graph.yFit[j] = amp * SQR(sigma) / ( SQR(j-x0) + SQR(sigma) );
	    }
	}
      if ( itype == 0 )
	printf("Gaussian: x0: %g   FWHM: %g   amp: %g   y-baseline: %f\n",
	       x0,2.355*sigma,amp,yoff);
      else
	printf("1/r^2:    x0: %g   FWHM: %g   amp: %g   y-baseline: %f\n",
	       x0,2.*sigma,amp,yoff);

    }
  free_double_vector(xarray);
  free_double_vector(yarray);

  return;
}

void action_image_text_selection_specific( int button )
{
  char InputString[LEN_STR], PromptString[LEN_STR];
  float low, high;

  if ( mosaic.text_selection == TEXTFIELD_LABEL_PLANES )
    {
      if ( button == 1 )
	action_key_l();
      else
	action_key_L();
    }
  else if ( mosaic.text_selection == TEXTFIELD_COLOR_OR_GRAY )
    {
      if ( mosaic.activation.stack->nt != 0 )
	{
	  mosaic.activation.visible = !mosaic.activation.visible;
	  mosaic.update_regions |= UPDATE_IMAGES;
	  mosaic.update_regions |= UPDATE_COLORBAR;
	  mosaic.update_regions |= UPDATE_COLORBAR_LABELS;
	  mosaic.update_regions |= UPDATE_SELECT_TEXT_IMAGE;
	  graph.update_regions  |= UPDATE_GRAPH;
	}
    }
  else if ( mosaic.text_selection == TEXTFIELD_COLORBAR_RANGE )
    {
      if ( mosaic.activation.visible )
	{
	  sprintf(PromptString,"Input range (%g %g) ",
		  get_activation_threshold(-1),get_activation_threshold(1));
	  if ( get_X11_string(2,PromptString,"separate values with space",
			      "",0,1,InputString) )
	    {
	      if ( sscanf(InputString,"%g %g",&low,&high) == 2 )
		{
		  set_activation_threshold(-1,low);
		  set_activation_threshold(1,high);
		}
	    }
	}
      else
	{
	  sprintf(PromptString,"Input range (%g %g) ",image_scale_min,image_scale_max);
	  if ( get_X11_string(2,PromptString,"separate values with space",
			      "",0,1,InputString) )
	    {
	      if ( sscanf(InputString,"%g %g",&low,&high) == 2 )
		{
		  if ( low < high )
		    {
		      image_scale_min = low;
		      image_scale_max = high;
		    }
		  else if ( low > high )
		    {
		      image_scale_min = high;
		      image_scale_max = low;
		    }
		}
	    }
	}
      mosaic.update_regions |= UPDATE_IMAGES;
      mosaic.update_regions |= UPDATE_COLORBAR;
      mosaic.update_regions |= UPDATE_COLORBAR_LABELS;
    }
}

void action_graph_text_selection_specific( int button )
{
  if ( graph.text_selection == TEXTFIELD_OUTPUT_ROI )
    {
      if ( button == 1 || !ovl_root.next )
	action_write_1_ROI();
      else
	action_write_all_ROIs();
    }
  else if ( graph.text_selection == TEXTFIELD_LAST_TIME_POINT )
    {
      action_display_last_time_point();
    }
  action_graph_text_selection_TimeSeriesModel( button );
}

void action_increment_activation_map()
{
  int last_log_map;JIP_INDEX index;
  struct file_list *this_map;

  mosaic.activation.imap++;
  if ( mosaic.activation.imap >= mosaic.activation.stack->nt ) mosaic.activation.imap = 0;
  index = mosaic.stack->npts_volume * mosaic.activation.imap;
  mosaic.activation.fSpaceMatrix = &(mosaic.activation.stack->SpaceTime.fData[index]);

  last_log_map = -1;
  for ( this_map = &activation_files; this_map; this_map = this_map->next )
    {
      if ( this_map->scale_log )
	last_log_map = this_map->end_image;
      if ( mosaic.activation.imap >= this_map->start_image && mosaic.activation.imap <= this_map->end_image  )
	{
	  strcpy(mosaic.activation.map_name,this_map->file_name);
	  mosaic.activation.scale_log = this_map->scale_log;
	  if ( !mosaic.activation.scale_log && last_log_map >= 0 ) // linear scale with preceeding p-value map
	    {
	      index = mosaic.stack->npts_volume * last_log_map;
	      mosaic.activation.pFilter = &(mosaic.activation.stack->SpaceTime.fData[index]);
	    }
	  else
	    mosaic.activation.pFilter = NULL;
	}
    }
  // Change the window name. Update image and graph
  change_window_name();
  mosaic.update_regions |= UPDATE_IMAGES;
  mosaic.update_regions |= UPDATE_COLORBAR;
  mosaic.update_regions |= UPDATE_SELECT_TEXT_IMAGE; // for colorbar range
  mosaic.update_regions |= UPDATE_INFO_AREA_IMAGE;   // for displaying color value at cursor
  if ( graph.type == 3 )
    graph.update_regions |= UPDATE_GRAPH_TEXT_LINES;
  else
    graph.update_regions |= UPDATE_GRAPH;
}

void action_decrement_activation_map()
{
  int last_log_map;JIP_INDEX index;
  struct file_list *this_map;

  mosaic.activation.imap--;
  if ( mosaic.activation.imap < 0 ) mosaic.activation.imap = mosaic.activation.stack->nt - 1;
  index = mosaic.stack->npts_volume * mosaic.activation.imap;
  mosaic.activation.fSpaceMatrix = &(mosaic.activation.stack->SpaceTime.fData[index]);

  last_log_map = -1;
  for ( this_map = &activation_files; this_map; this_map = this_map->next )
    {
      if ( this_map->scale_log )
	last_log_map = this_map->end_image;
      if ( mosaic.activation.imap >= this_map->start_image && mosaic.activation.imap <= this_map->end_image  )
	{
	  strcpy(mosaic.activation.map_name,this_map->file_name);
	  mosaic.activation.scale_log = this_map->scale_log;
	  if ( !mosaic.activation.scale_log && last_log_map >= 0 ) // linear scale with preceeding p-value map
	    {
	      index = mosaic.stack->npts_volume * last_log_map;
	      mosaic.activation.pFilter = &(mosaic.activation.stack->SpaceTime.fData[index]);
	    }
	  else
	    mosaic.activation.pFilter = NULL;
	}
    }
  // Change the window name. Update image and graph
  change_window_name();
  mosaic.update_regions |= UPDATE_IMAGES;
  mosaic.update_regions |= UPDATE_COLORBAR;
  mosaic.update_regions |= UPDATE_SELECT_TEXT_IMAGE; // for colorbar range
  mosaic.update_regions |= UPDATE_INFO_AREA_IMAGE;   // for displaying color value at cursor
  if ( graph.type == 3 )
    graph.update_regions |= UPDATE_GRAPH_TEXT_LINES;
  else
    graph.update_regions |= UPDATE_GRAPH;
}
