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
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include "util.h"
#include "display.h"
#include "display-X11.h"
#include "glm-prototypes.h"
#include "rem-prototypes.h"

#define IRANGE 65536
#define MAX_ARGS 100


int main(int argc, char *argv[])
{
  char arguments[MAX_ARGS][LEN_STR], *myargv[MAX_ARGS];
  struct file_list *this_run, *this_file;
  char *optarg, ovl_name[LEN_STR], wire_name[LEN_STR];
  int option, which_arg, *model_flags, min_nt;
  int myargc, nruns, nfiles, number_panels, nPanelsX, j;
  BOOL UseT=FALSE, UseS=FALSE, IncludeTimeData=TRUE, Verbose=FALSE;
  float start, stop;

  myargc = argc;
  for (j=0; j<MAX_ARGS; j++)
    {
      if ( j < argc ) strcpy(arguments[j],argv[j]);
      myargv[j] = arguments[j];
    }

  strcpy(program_name,"display");
  if ( myargc < 2 || (myargc == 2 && (!strcmp("-y",myargv[1]) || !strcmp("-Y",myargv[1]))) )
    {
      // Read the file .xdisplay if it exists.
      read_program_init_file(".xdisplay",myargv,&myargc);
      if ( myargc < 2 )
	{
	  read_program_init_file(".display",myargv,&myargc);
	  if ( myargc < 2 )
	    {
	      printf("Error: this program should be invoked as\n");
	      printf("   display <image file> [optional arguments]\n\n");
	      action_help(0,1);
	      exit(1);
	    }
	}
    }

  anatomy_stack.nt = image_stack.nt = activation_stack.nt = 0;
  graph.time_model = graph.freq_domain = 0;
  strcpy(ovl_name,"");
  strcpy(wire_name,"");
  strcpy(ROI_file_name,".roi");
  strcpy(roi_file_root.name,"ignore the root\n");
  nPanelsX = 0;
  number_panels = 0;
  mosaic.zoom = 0;
  mosaic.flip_X11_y       = TRUE;
  mosaic.Attach_ColorBar  = TRUE;
  mosaic.display_ortho_ZY = TRUE;
  mosaic.mosaic_orientation = ORTHO_XY;
  mosaic.activation.thresh_lin_low  = mosaic.activation.thresh_lin_high = 0.;

  // Get Options.
  while ( (option=my_getopt(myargc,myargv,
			    "A:a:z:Z:x:X:p:P:e:m:M:o:O:w:W:3hHtTSvVyYi:G:g:r:",
			    &which_arg)) )
    {
      optarg = myargv[which_arg];
      switch ((char)option)
	{
	case 'a':
	  add_image_file( &activation_files, optarg, 0, 0);
	  break;
	case 'A':
	  add_image_file( &activation_files, optarg, 2, 0);
	  break;
	case 'G':
	  initialize_TimeSeriesModel(3);
	  if ( read_TimeSeriesModel(optarg) ) exit(1);
	  break;
	case 'g':
	  initialize_TimeSeriesModel(1);
	  if ( read_TimeSeriesModel(optarg) ) exit(1);
	  break;
	case 'e':
	  add_image_file( &anatomy_files, optarg, 0, 0);
	  break;
	case 'h':
	  action_help(0,1);
	  exit(0);
	  break;
	case 'H':
	  help_menu(0,"display-menu.txt");
	  exit(0);
	  break;
	case 'i':
	  add_image_file( &image_files, optarg, 0, 0);
	  break;
	case 'm': // mosaic: set number of panels
	case 'M':
	  sscanf(optarg,"%d",&number_panels);
	  break;
	case 'o':
	case 'O':
	  strcpy(ovl_name,optarg);
	  break;
	case 'p':
	case 'P':
	  add_image_file( &activation_files, optarg, 1, 0);
	  break;
	case 'r':
	  initialize_TimeSeriesModel(2);
	  if ( read_TimeSeriesModel(optarg) ) exit(1);
	  break;
	case 'S':
	  UseS = TRUE;
	  break;
	case 't':
	  IncludeTimeData = FALSE;
	  break;
	case 'T':
	  UseT = TRUE;
	  break;
	case 'v':
	case 'V':
	  Verbose = TRUE;
	  break;
	case 'w':
	case 'W':
	  strcpy(wire_name,optarg);
	  break;
	case 'x':
	case 'X':
	  sscanf(optarg,"%d",&(nPanelsX));
	  if ( nPanelsX < 1 ) nPanelsX = 1;
	  break;
	case 'y':
	  mosaic.flip_X11_y = !mosaic.flip_X11_y;
	  break;
	case 'Y':
	  mosaic.display_ortho_ZY = !mosaic.display_ortho_ZY;
	  break;
	case 'z':
	case 'Z':
	  sscanf(optarg,"%d",&(mosaic.zoom));
	  if ( mosaic.zoom < 1 ) mosaic.zoom = 0;
	  break;
	case '3': // orthogonal planes
	  number_panels = -1;
	  break;
	default:
	  if ( option == -1 )
	    printf("Option %s not valid.\n",argv[which_arg]);
	  else
	    printf("Valid option -%c not yet programmed.\n",*optarg);
	  printf("Program exiting.\n");
	  exit(1);
	}
    }

  // Define the time series model
  if ( TimeSeriesModel_is_initialized() )
    {
      model_flags = int_vector(4);
      model_flags[0] = IncludeTimeData;
      model_flags[1] = UseT;
      model_flags[2] = UseS;
      model_flags[3] = Verbose;
      define_TimeSeriesModel(4, model_flags);
    }

  // Read run files.
  if ( !strcmp(image_files.file_name,"") )
    {
      // Read the file .xdisplay if it exists.
      printf("Error: no image files and no .xdisplay file\n");
      exit(1);
    }

  // Read the image files that are excluded from the time series model.
  printf("\n");
  if ( strcmp(anatomy_files.file_name,"") )
    {
      if ( read_SpaceTime_data(Verbose, 1, &anatomy_stack, &anatomy_files, &nfiles) )
	exit(1);
    }
  // Read the activation files.
  if ( strcmp(activation_files.file_name,"") )
    {
      if ( !read_SpaceTime_data(Verbose, 1, &activation_stack, &activation_files, &nfiles) )
	{ // go on if an error occurs reading the activation map overlay.
	  for ( this_file = &activation_files; this_file; this_file = this_file->next )
	    {
	      if ( this_file->scale_log == 2 )
		convert_activation_scale(this_file);
	    }
	}
      else
	{
	  //	  printf("Error reading activation files. Continuing without activation overlays ...\n");
	  //	  activation_stack.nt = 0;
	  exit(1);
	}
    }
  // Read the main data files.
  if ( read_SpaceTime_data(Verbose, 1, &image_stack, &image_files, &nruns) )
    exit(1);
  // Find the LEAST number of time points per file.
  for (this_file=&image_files, min_nt=100000; this_file; this_file=this_file->next)
    if (this_file->nt < min_nt ) min_nt = this_file->nt;

  if ( image_stack.off_diagonal )
    {
      printf("Warning: the data has voxel indices             (i,j,k)\n");
      printf("         that are not parallel with coordinates (x,y,z).\n");
      printf("         The coordinates will display correctly, but\n");
      printf("         wire frames won't work.\n");
    }

  if ( !TimeSeriesModel_is_initialized() )
    {
      if ( min_nt >= 10 )
	{ // Define a polynomial GLM if a GLM was not previously defined.
	  initialize_TimeSeriesModel(1);
	  GLM_initialize( image_stack.nt, nruns );
	  for ( this_run = &image_files, j=0; this_run; 
		this_run = this_run->next, j++ )
	    {
	      start = (float)(this_run->start_image);
	      stop  = (float)(this_run->end_image);
	      GLM_initialize_run(j,start,stop);
	    }
	  glm_model.shape = SHAPE_GAMMA; // make a gamma function the default
	  GLM_define_basis_functions();
	  if ( GLM_input.notify ) GLM_describe_basis_functions();
	}
      else
	graph.time_model = 0; // don't use polynomial GLM model for very few time points
    }

  // Initialize the mosaic and graph structures.
  initialize_mosaic_ovl( number_panels );
  mosaic.DimPanels.x = nPanelsX;

  // Read the overlay and wire lists;
  // this must be done after the mosaic has been initialized.
  if ( strcmp(wire_name,"") )
    {
      if ( read_wire_list(wire_name) ) exit(1);
      mosaic.toggle_overlays = 2;
    }
  if ( strcmp(ovl_name,"") )
    {
      if ( read_overlay_list(ovl_name) ) exit(1);
      mosaic.toggle_overlays = 1;
    }

  initialize_graph();
  graph.iTime = -anatomy_stack.nt;
  graph.iLastTime = graph.iTime;

  // set the time resolution to 1 if it's set to 0.
  if ( graph.time_model == 2 )
    mosaic.stack->resolution_t = 0.5;
  if ( mosaic.stack->resolution_t == 0. )
    mosaic.stack->resolution_t = 1.0;

  // Bring up the display.
  initialize_XWindows();
  rescale_image(1);
  update_XWindow_image();
  update_graph_data(1);

  // not sure why the following update is necessary
  mosaic.update_regions  = UPDATE_IMAGES;
  Update_Regions();

  // Interactive events:
  loop_X_events();
  // The above never returns, so we never reach the exit.
  exit(1);
}

int image_event_handler_specific(XEvent *event)
{
  char which_key;
  BOOL current_window;

  current_window = (event->xbutton.window == XWindow_image);
  if ( !current_window && event->type != KeyPress ) return(0);

  switch (event->type)
    {
    case KeyPress:
      {
	if ( !get_key_press(&(event->xkey), &which_key) ) return(1);
	switch (which_key)
	  {
	    ///////////////////////////////////////////////
	    //////////////// non-alphabetic ///////////////
	    ///////////////////////////////////////////////
	  case '+':
	  case '=':
	    if ( mosaic.cursor_selection&CURSOR_ON_COLORBAR &&
		 mosaic.activation.nmaps != 0 )
	      action_increment_activation_map();
	    else
	      {
		increment_graph_location(1);
		mosaic.update_regions |= UPDATE_IMAGES;
		mosaic.update_regions |= UPDATE_INFO_AREA_IMAGE;
		graph.update_regions  |= UPDATE_INFO_AREA_GRAPH;
		if ( graph.type == 3 )
		  {
		    graph.update_regions |= UPDATE_GRAPH_TEXT_LINES;
		    graph.update_regions |= UPDATE_SELECT_TEXT_GRAPH;
		  }
		else
		  graph.update_regions |= UPDATE_GRAPH;
	      }
	    break;
	  case '-':
	  case '_':
	    if ( mosaic.cursor_selection&CURSOR_ON_COLORBAR &&
		 mosaic.activation.nmaps != 0 )
	      action_decrement_activation_map();
	    else
	      {
		decrement_graph_location(1);
		mosaic.update_regions |= UPDATE_IMAGES;
		mosaic.update_regions |= UPDATE_INFO_AREA_IMAGE;
		graph.update_regions |= UPDATE_INFO_AREA_GRAPH;
		if ( graph.type == 3 )
		  {
		    graph.update_regions |= UPDATE_GRAPH_TEXT_LINES;
		    graph.update_regions |= UPDATE_SELECT_TEXT_GRAPH;
		  }
		else
		  graph.update_regions |= UPDATE_GRAPH;
	      }
	    break;
	    ///////////////////////////////////////////////
	    //////////////// Begin alphabetic /////////////
	    ///////////////////////////////////////////////
	  case 'A':
	    action_animate();
	    show_on_info_line(0,0,"animation stopped","","");
	    break;
	  case 'v': // visible
	  case ' ':
	    if ( mosaic.activation.stack->nt != 0 )
	      {
		mosaic.activation.visible = !mosaic.activation.visible;
		mosaic.update_regions |= UPDATE_IMAGES;
		mosaic.update_regions |= UPDATE_COLORBAR;
		mosaic.update_regions |= UPDATE_COLORBAR_LABELS;
		mosaic.update_regions |= UPDATE_SELECT_TEXT_IMAGE;
		graph.update_regions  |= UPDATE_GRAPH;
	      }
	    break;
	  case 'P':
	    if ( graph.type != 3 )
	      {
		graph.type = 3;
		initialize_graph();
	      }
	    if ( !action_compute_p_value() )
	      {
		rescale_activation();
		update_XWindow_image();
		graph.update_regions |= UPDATE_GRAPH;
	      }
	    break;
	  case 'r': //
	    action_write_1_ROI();
	    break;
	  case 'R': // 
	    action_write_all_ROIs();
	    break;
	  case 's':
	    fill_graph_vector(1);
	    write_roi_stats(0);
	    break;
	  case 'S':
	    fill_graph_vector(1);
	    write_roi_stats(1);
	    break;
	  case 'g':   // graph
	  case 'G':   // graph
	    action_bring_up_graph();
	    break;
	  case 'l':
	    action_key_l();
	    break;
	  case 'L':
	    action_key_L();
	    break;
	  default:
	    // Pass the event to the graph handler if it came from the image window.
	    if ( current_window )
	      graph_event_handler_general(event); // 0 indicates event NOT from graph window
	    break;
	  }
	break; /* END OF case KeyPress: */
      }
    default:
      break;
    } /* END OF switch(event->type) */

  return(0);
}

int graph_event_handler_specific(XEvent *event)
{
  char which_key;
  BOOL current_window;
  int command_found;

  current_window = (event->xbutton.window == XWindow_graph);
  if ( !current_window && event->type != KeyPress ) return(0);

  switch (event->type)
    {
    case KeyPress:
      {
	if ( !get_key_press(&(event->xkey), &which_key) ) return(1);
	switch (which_key)
	  {
	  case 'f':
	    if ( graph.type == 0 || graph.type == 1 || graph.type == 2 )
	      action_fit_psf(0);
	    build_graph_display(1);
	    break;
	  case 'F':
	    if ( graph.type == 0 || graph.type == 1 || graph.type == 2 )
	      action_fit_psf(1);
	    build_graph_display(1);
	    break;
	  case 'i': // ignore out of range
	    action_ignore_out_of_range();
	    build_graph_display(1);
	    break;
	  case 'l': // ignore out of range
	    action_display_last_time_point();
	    break;
	  case 'r':
	    action_write_1_ROI();
	    break;
	  case 'R':
	    if ( !ovl_root.next )
	      action_write_1_ROI();
	    else
	      action_write_all_ROIs();
	    break;
	  case 'u': // units
	    action_change_graph_units();
	    break;
	  default:
	    command_found = graph_event_handler_TimeSeriesModel(which_key);
	    if ( !command_found )
	      {
		// Pass the event to the image handler if it came from the graph window.
		if ( current_window )
		  image_event_handler_general(event); // 0 indicates event NOT from graph window
	      }
	    break;
	  }
	break; /* END OF case KeyPress on graph: */
      }
    default:
      break;
    } /* END OF switch(event->type) */
  return(0);
}

void initialize_specific_selectable_text()
{
  ////////////////////////////////////////////////////////////////////
  // Initialize the "standard" set of text fields for the image window.
  // new_selectable_image_text(Y-loc, ID)
  new_selectable_image_text(0,   TEXTFIELD_LABEL_PLANES);   // activation visibility
  new_selectable_image_text(0,   TEXTFIELD_COLOR_OR_GRAY);  // colorbar or grayscale selection
  new_selectable_image_text(2,   TEXTFIELD_COLORBAR_RANGE); // colorbar or grayscale range

  ////////////////////////////////////////////////////////////////////
  // Initialize the "standard" set of text fields for the graph window.
  // new_selectable_graph_text(Y-loc, ID)
  // write ROI output
  new_selectable_graph_text(4, TEXTFIELD_OUTPUT_ROI);
  // last time point
  new_selectable_graph_text(4, TEXTFIELD_LAST_TIME_POINT);
}

void update_specific_image_selectable_text()
{ // This function is called whenever mosaic.ImageRegionPixels.x changes.
  struct SelectableText *SelectText;
  float range;
  int istep, iColorBar;

  istep = MAX(15,mosaic.ImageRegionPixels.x/25);
  // The following should be consistent with common-display.c
  iColorBar = mosaic.ImageRegionPixels.x - lXBottomMenu;

  for (SelectText = &(mosaic.SelectText); SelectText; SelectText=SelectText->next)
    {
      // All text fields already have initial values (e.g., visible = 1)
      if ( SelectText->FieldID == TEXTFIELD_LABEL_PLANES ) // label planes with positions/grids
	{
	  SelectText->XPosition    = 20*istep;
	  SelectText->XSelectStart = SelectText->XPosition;
	  SelectText->XSelectEnd   = SelectText->XSelectStart + istep-1;
	  SelectText->color        = COLOR_GRAY;
	  sprintf(SelectText->string," l ");  // text field
	  sprintf(SelectText->message1,"Keyboard: l/L");
	  sprintf(SelectText->message2,"Label position (l) or grids (L) on aligned data");
	  sprintf(SelectText->message3,"L button           or R button");
	  if ( mosaic.ImageRegionPixels.x < 350 )
	    SelectText->visible = 0;
	}
      else if ( SelectText->FieldID == TEXTFIELD_COLOR_OR_GRAY ) // label planes with positions/grids
	{
	  SelectText->XPosition    = mosaic.ImageRegionPixels.x + mosaic.nx_ColorBar/4;
	  SelectText->XSelectStart = mosaic.ImageRegionPixels.x;
	  SelectText->XSelectEnd   = mosaic.ImageRegionPixels.x + mosaic.nx_ColorBar;
	  if ( mosaic.activation.visible )
	    {
	      SelectText->color = COLOR_YELLOW;
	      if ( mosaic.text_selection == TEXTFIELD_COLOR_OR_GRAY )
		sprintf(SelectText->string,"->gray ");  // text field
	      else
		sprintf(SelectText->string," color ");  // text field
	    }
	  else
	    {
	      SelectText->color = COLOR_GRAY;
	      if ( mosaic.text_selection == TEXTFIELD_COLOR_OR_GRAY )
		sprintf(SelectText->string,"->color");  // text field
	      else
		sprintf(SelectText->string," gray  ");  // text field
	    }
	  sprintf(SelectText->message1,"Keyboard: spacebar");
	  sprintf(SelectText->message2,"Toggle between activation colorbar");
	  sprintf(SelectText->message3,"and grayscale colorbar");
	  if ( mosaic.nx_ColorBar <= 0 || mosaic.activation.nmaps == 0 )
	    SelectText->visible = 0;
	}
      else if ( SelectText->FieldID == TEXTFIELD_COLORBAR_RANGE ) // activation visibility
	{
	  //	  if (activation_stack.nt == 0 )   // no activation
	  //	    SelectText->visible = 0;
	  SelectText->XPosition    = iColorBar;
	  SelectText->XSelectStart = SelectText->XPosition + lXBottomMenu;
	  SelectText->XSelectEnd   = SelectText->XSelectStart + mosaic.nx_ColorBar;
	  if ( mosaic.activation.visible && mosaic.activation.scale_log )
	    // Put the activation range below the colorbar
	    sprintf(SelectText->string,"%d to %d",
		    (int)(get_activation_threshold(-1)),(int)(get_activation_threshold(1)));
	  else if ( mosaic.activation.visible )
	    {
	      // Put the activation range below the colorbar
	      range = get_activation_threshold(1) - get_activation_threshold(-1);
	      if ( range > 5. )
		sprintf(SelectText->string,"%d to %d",
			(int)(get_activation_threshold(-1)),(int)(get_activation_threshold(1)));
	      else
		sprintf(SelectText->string,"%g to %g",
			get_activation_threshold(-1),get_activation_threshold(1));
	    }
	  else
	    {
	      range = image_scale_max - image_scale_min;
	      if ( range > 5. )
		sprintf(SelectText->string,"%d to %d",(int)image_scale_min,(int)image_scale_max);
	      else
		sprintf(SelectText->string,"%g to %g",image_scale_min,image_scale_max);
	    }
	  sprintf(SelectText->message1,"Select to input range.");
	  sprintf(SelectText->message2,"Use spacebar to toggle activation on/off.");
	  sprintf(SelectText->message3," ");
	}
    }
}

void update_specific_graph_selectable_text()
{
  struct SelectableText *SelectText;

  for (SelectText = &(graph.SelectText); SelectText; SelectText=SelectText->next)
    {
      if ( SelectText->FieldID == TEXTFIELD_OUTPUT_ROI )        // ROI    ovl
	{
	  SelectText->XPosition    = 320;
	  SelectText->XSelectStart = SelectText->XPosition;
	  SelectText->XSelectEnd   = SelectText->XSelectStart + 40;
	  SelectText->color        = COLOR_CYAN;  // group overlay/wire by color
	  if ( ! ovl_root.next )
	    {
	      sprintf(SelectText->string," R ");  // text field
	      sprintf(SelectText->message1,"KeyBoard: r");
	      sprintf(SelectText->message2,"Write ROI for current overlay");
	      sprintf(SelectText->message3,"L button");
	    }
	  else
	    {
	      sprintf(SelectText->string," R ");  // text field
	      sprintf(SelectText->message1,"KeyBoard: r/R");
	      sprintf(SelectText->message2,"ROI for 1 (r) or all (R) overlays");
	      sprintf(SelectText->message3,"L button      or R button");
	    }
	  // This button is visible for all type but time, and also for time with an overlay
	  SelectText->visible = (graph.type != 3) || (ovl_root.npts != 0 || ovl_root.next);
	}
      else if ( SelectText->FieldID == TEXTFIELD_LAST_TIME_POINT ) // toggle back to last time point
	{
	  SelectText->XPosition    = 220;
	  SelectText->XSelectStart = SelectText->XPosition;
	  SelectText->XSelectEnd   = SelectText->XSelectStart + 50;
	  SelectText->color        = COLOR_CYAN;  // group overlay/wire by color
	  sprintf(SelectText->string," l ");  // text field
	  sprintf(SelectText->message1,"KeyBoard: l");
	  sprintf(SelectText->message2,"Display last time point shown.");
	  sprintf(SelectText->message3,"left mouse button");
	  // This button is visible only for time graphs
	  SelectText->visible = (graph.type == 3);
	}
      else
	update_SelectText_TimeSeriesModel( SelectText );
    }
}

void initialize_mosaic_ovl( int number_panels )
{
  IPOINT2D RoughDisplaySize;
  JIP_INDEX j, npts;

  // Generic global variables
  contrast_low  = 0.;
  contrast_high = 1.;
  drawing_overlay = 0;

  mosaic.display_3orthogonal = 0;
  mosaic.nx_RightPanel = 0;

  mosaic.stack = &(image_stack);
  mosaic.activation.stack = &(activation_stack);

  if ( number_panels < 0 )
    mosaic.display_3orthogonal = 1;
  else if ( number_panels > mosaic.stack->dim.z )
    number_panels = mosaic.stack->dim.z;

  mosaic.interpolate = 0;
  mosaic.clipping_mode = 0;
  mosaic.display_crosshairs  = 3;  // 0 = none, 1 = pixelated, 2 = line, 3 = cross
  mosaic.display_grid = 0;
  mosaic.display_data_type = 0;
  mosaic.transparent_overlay = 0;

  // Compute display size
  if ( number_panels > 0 )
    mosaic.DimPanels.z = number_panels;
  else if ( number_panels < 0 )
    mosaic.DimPanels.z = 4;
  else if ( number_panels == 0 )
    mosaic.DimPanels.z = mosaic.stack->dim.z;
  compute_display_size(&RoughDisplaySize,1.);
  //  mosaic.DimPanels.z = 0; // reset to 0 so that all panels are displayed in mosaic

  // Orthogonal display values.
  mosaic.icross.x = mosaic.stack->dim.x/2;
  mosaic.icross.y = mosaic.stack->dim.y/2;
  mosaic.icross.z = mosaic.stack->dim.z/2;

  // Set the pointer for the display data to the individual data array.
  // Set the pointer for the activation
  if ( anatomy_stack.nt != 0 )
    mosaic.fSpaceMatrix  = &(anatomy_stack.SpaceTime.fData[0]);
  else
    mosaic.fSpaceMatrix = &(mosaic.stack->SpaceTime.fData[0]);
  // Set the complex pointer equal to the floating pointer.
  mosaic.activation.visible = 0;
  mosaic.activation.transparent = FALSE;
  mosaic.activation.nmaps = mosaic.activation.stack->nt;
  mosaic.activation.imap = 0;

  if ( mosaic.activation.stack->nt != 0 )
    {
      mosaic.activation.visible = 1;
      mosaic.activation.fSpaceMatrix = &(mosaic.activation.stack->SpaceTime.fData[0]);
      strcpy(mosaic.activation.map_name,activation_files.file_name);
      mosaic.activation.scale_log = activation_files.scale_log;
      rescale_activation();
    }
  mosaic.activation.color_scheme = 0;
  mosaic.activation.color_sign   = 1;
  mosaic.toggle_overlays  = 0;

  // Initialize the overlay map.
  overlay_map.weight = float_vector(mosaic.stack->n3d);
  overlay_map.color  = short_vector(mosaic.stack->n3d);
  for (j=0; j<mosaic.stack->n3d; j++)
    {
      overlay_map.weight[j] = 0.;
      overlay_map.color[j] = 0;
    }
  overlay_map.number = 1;

  // Initialize the root overlay.
  sprintf(ovl_root.name,"none");
  ovl_root.color = 6;
  ovl_root.npts = 0;
  ovl_root.active = TRUE; // This is always true
  ovl_root.next = NULL;
  // "over-allocate" these arrays for simplicity (npts dimensions < n3d)
  
  npts = mosaic.stack->n3d;
  ovl_root.voxel  = ipoint3d_vector(npts);
  ovl_root.weight = float_vector(npts);
  for (j=0; j<npts; j++)
    {
      ovl_root.weight[j] = 0.;
      ovl_root.voxel[j].x = ovl_root.voxel[j].y = ovl_root.voxel[j].z = 0;
    }

  // Initialize the root wire frame
  sprintf(wire_root.name,"none");
  wire_root.active = TRUE;
  wire_root.npts   = 0;
  wire_root.control= NULL;
  wire_root.next   = NULL;

  // Initialize selectable text.
  initialize_selectable_text();
}

void action_animate()
{ // xdisplay only
  XEvent event;

  show_on_info_line(0,0,"animation in progress, any key will quit","","");
  XSync(XDisplay,True);
  do
    {
      increment_graph_location(1);
      mosaic.update_regions |= UPDATE_IMAGES;
      mosaic.update_regions |= UPDATE_INFO_AREA_IMAGE;
      mosaic.update_regions |= UPDATE_INFO_AREA_GRAPH;
      Update_Regions();
      build_graph_display(0);
      XFlush(XDisplay);
    }
  while
    ( ! ( get_next_event(&event) &&
	  (event.type == KeyPress      ||
	   event.type == ButtonPress   ||
	   event.type == ButtonRelease ||
	   event.type == MotionNotify))
      );
}

void label_graph_window()
{
  char graph_type[LEN_STR], string[LEN_STR];
  int number_rois;

  if ( !XWindow_graph )
    return;

  if ( graph.type == 0 )      // x
    sprintf(graph_type,"x projection");
  else if ( graph.type == 1 ) // y
    sprintf(graph_type,"y projection");
  else if ( graph.type == 2 ) // z
    sprintf(graph_type,"z projection");
  else if ( graph.type == 3 ) // t
    sprintf(graph_type,"time points");
  else if ( graph.type == 4 ) // t
    sprintf(graph_type,"histogram");

  // Define the ROI file name based upon the residue setting.
  if ( graph.type == 0 )         // x
    sprintf(ROI_file_name,".px");
  else if ( graph.type == 1 )    // y
    sprintf(ROI_file_name,".py");
  else if ( graph.type == 2 )    // z
    sprintf(ROI_file_name,".pz");
  else if ( graph.type == 4 )    // hist
    sprintf(ROI_file_name,".hist");
  else                           // time
    label_graph_time_window_TimeSeriesModel( ROI_file_name ); 
  number_rois = get_number_rois_in_file(ROI_file_name);
  sprintf(string,"%s  ----  output to file %s (%d)",graph_type,ROI_file_name,number_rois);
  // Give the window a name.
  XStoreName(XDisplay, XWindow_graph, string);
}
