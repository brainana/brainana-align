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
#include "srtm-prototypes.h"

struct file_list sigma2_file;
struct data_stack sigma2_stack;
BOOL Verbose;
float max_r1, max_t2, max_bp;


void initialize_SRTM_TimeSeriesModel()
{

  Verbose = FALSE;
  SRTM_input.nruns = 0;
  glm_model.options = glm_model.baseline = glm_model.baseline_terms = 0;
  glm_model.show_glm_windows = 2;
  if (glm_model.residue_string == NULL)
    strcpy(glm_model.residue_string,"");

  glm_model.iParameter = glm_model.iCondition = 0;
  glm_model.event_number_active = 1; // default event number
  glm_model.view_type = -1;  // TAC
  mosaic.activation.thresh_lin_low  = 0.; // explicitly set linear map limits for BP
  mosaic.activation.thresh_lin_high = 3.;
  // Graph text /////////////////////////////////////
  // GLM options
  new_selectable_graph_text(2, TEXTFIELD_SRTM_OPTIONS);
  // GLM settings
  new_selectable_graph_text(2, TEXTFIELD_SRTM_SETTINGS);
  // parameter values
  new_selectable_graph_text(2, TEXTFIELD_SRTM_PARVALUES);
}

int read_SRTM_TimeSeriesModel( char *file_name )
{
  if ( SRTM_read_control_file(Verbose, file_name) ) return(1);
  return(0);
}

int define_SRTM_TimeSeriesModel( int nflags, int *flags )
{
  char condition_name[LEN_STR], file_name[LEN_STR];
  int nconditions, j;
  BOOL IncludeTimeData, UseT, UseS;

  if ( nflags != 4 )
    {
      printf("Programming error 1 in function define_model\n");
      exit(1);
    }
  else
    {
      IncludeTimeData = flags[0]; // unused
      UseT            = flags[1];
      UseS            = flags[2];
      Verbose         = flags[3];
    }
  //
  for ( j=0; j<SRTM_input.nruns; j++)
    add_image_file( &image_files, SRTM_input.run_name[j], 0, 0);

  if ( SRTM_define_static_basis_functions() ) exit(1);
  if ( SRTM_define_conditions(SRTM_input.condition_string) ) exit(1);

  SRTM_describe_conditions();
  SRTM_describe_input_files();

  // Get the number of conditions.
  nconditions = SRTM_get_nconditions();
  // Add to the list of activation files, based upon the existing conditions.
  for (j=0; j<nconditions; j++)
    { // overwrite the condition string
      if ( SRTM_get_condition_name(j,condition_name) )
	return(1);
      if ( SRTM_input.nifti_output )
	sprintf(file_name,"P-%s.nii",condition_name);
      else
	sprintf(file_name,"P-%s.bfloat",condition_name);
      add_image_file( &activation_files, file_name, 1, 1);
      if ( UseT )
	{
	  if ( SRTM_input.nifti_output )
	    sprintf(file_name,"T-%s.nii",condition_name);
	  else
	    sprintf(file_name,"T-%s.bfloat",condition_name);
	  add_image_file( &activation_files, file_name, 0, 2);
	}
      if ( UseS )
	{
	  if ( SRTM_input.nifti_output )
	    sprintf(file_name,"S-%s.nii",condition_name);
	  else
	    sprintf(file_name,"S-%s.bfloat",condition_name);
	  add_image_file( &activation_files, file_name, 0, 3);
	}
      if ( SRTM_BP_condition(j) )
	{
	  if ( SRTM_input.nifti_output )
	    sprintf(file_name,"BP-%s.nii",condition_name);
	  else
	    sprintf(file_name,"BP-%s.bfloat",condition_name);
	  add_image_file( &activation_files, file_name, 0, 3);
	}
    }
  // Define parameter scales
  if ( define_parameter_scales() )
    return(1);
  else
    return(0);
}

int define_parameter_scales()
{
  int ntime, ncoeff, jtime;
  float *data_vector, *fit_vector;
  float par_r1, par_t2, par_bp, sigma2;

  ntime  = SRTM_get_time_points();
  ncoeff = SRTM_get_number_coefficients();

  data_vector = float_vector(ntime);
  fit_vector  = float_vector(ntime);

  if ( SRTM_get_target_region(ntime,data_vector) )
    return(1);
  SRTM_fit_data( ntime, data_vector, fit_vector, &sigma2 );
  // Loop over time points
  for (jtime=0, max_r1=max_t2=max_bp=-1.e5; jtime<ntime; jtime++)
    {
      par_r1 = SRTM_get_parameter_value( TYPE_R1,  jtime );
      par_t2 = SRTM_get_parameter_value( TYPE_k2,  jtime );
      par_bp = SRTM_get_parameter_value( TYPE_k2a, jtime );
      if ( par_r1 > max_r1 ) max_r1 = par_r1;
      if ( par_t2 > max_t2 ) max_t2 = par_t2;
      if ( par_bp > max_bp ) max_bp = par_bp;
    }
  
  return(0);
}

void plot_SRTM_TimeSeriesModel( POINT2D *Scale )
{
  POINT2D Point1, Point2;
  IPOINT2D iDraw1, iDraw2;
  int j;int nbins;
  float *BinXVector, *BinYVector, *RawVector, range, ParMax;
  int ncoeff, nruns, jbind, jrun, icutoff, istart, istop;
  int which_color;

  // Re-scale the graph unless the scale has been fixed.
  // This needs to be redone here due to the possibility that time points are ignored.
  if ( graph.ymin == graph.ymax || !graph.yrange_fixed )
    {
      // Find the min and max for the current time vector.
      graph.ymin =  1.e10;
      graph.ymax = -1.e10;
      for (j=0; j<graph.npoints; j++)
	{
	  if ( graph.yVector[j] < graph.ymin ) graph.ymin = graph.yVector[j];
	  if ( graph.yVector[j] > graph.ymax ) graph.ymax = graph.yVector[j];
	}
      // If units of percent are being used, round to an integer value.
      if ( graph.units_percent)
	{
	  graph.ymin = (int)(graph.ymin-1.);
	  graph.ymax = (int)(graph.ymax+1.);
	}
      else
	{
	  range = graph.ymax - graph.ymin;
	  graph.ymin -= range/10.;
	  graph.ymax += range/10.;
	}
    }

  // Define scales
  Scale->x = (float)(graph.nwidth) / (float)(graph.npoints);
  Scale->y = (float)(graph.nheight) / (graph.ymax-graph.ymin);

  // Display time windows excluded from the GLM
  nruns = SRTM_get_nruns();
  for (jrun=0; jrun<nruns; jrun++)
    {
      icutoff = SRTM_get_ignored_cutoff(jrun);
      SRTM_get_start_stop_times(jrun, &istart, &istop);
      icutoff += istart;  // put everything on a total-time basis
      if ( icutoff < istop )
	{
	  Point1.x = icutoff - 0.5;  Point1.y = graph.ymax;
	  Point2.x = istop   + 0.5;  Point2.y = graph.ymin;
	  iDraw1 = Space2D_to_GraphPixels(Point1, *Scale);
	  iDraw2 = Space2D_to_GraphPixels(Point2, *Scale);
	  DPRIMITIVE_FillRectangle(Drawable_GraphData, COLOR_BLUE, iDraw1, iDraw2);
	}
    }

  // Bin or smooth data
  RawVector = float_vector(graph.npoints);
  if ( graph.bin_data )
    {
      nbins = graph.npoints/graph.bin_width;
      if ( graph.npoints % graph.bin_width )
	nbins++;
      BinXVector = float_vector(nbins);
      BinYVector = float_vector(nbins);
    }
  else
    {
      nbins = graph.npoints;
      BinXVector = float_vector(nbins);
      BinYVector = float_vector(nbins);
      for (j=0; j<nbins; j++)
	BinXVector[j] = j;
    }

  // Display the fit for NS & S binding
  ncoeff = SRTM_get_number_coefficients();
  for (jbind=0; jbind<2; jbind++)
    {
      for (j=0; j<graph.npoints; j++)
	{
	  if ( SRTM_binding_fit(jbind, j, RawVector+j) )
	    return;
	}
      if ( graph.bin_data )
	bin_graph_vector(nbins,RawVector,BinXVector,BinYVector);
      else
	smooth_graph_vector(RawVector,BinYVector);
      // Set the color
      if ( jbind == 0 )
	which_color = COLOR_GREEN;
      else if ( jbind == 1 )
	which_color = COLOR_RED;
      // Plot the smoothed or binned vector.
      Point1.x = BinXVector[0];   Point1.y = BinYVector[0];
      iDraw1 = Space2D_to_GraphPixels(Point1, *Scale);
      for (j=1; j<nbins; j++)
	{
	  Point2.x = BinXVector[j];  Point2.y = BinYVector[j];
	  iDraw2 = Space2D_to_GraphPixels(Point2, *Scale);
	  DPRIMITIVE_DrawLine(Drawable_GraphData, which_color, iDraw1, iDraw2);
	  iDraw1.x = iDraw2.x;
	  iDraw1.y = iDraw2.y;
	}
    }

  // Display the complete fit to data.
  if ( graph.bin_data )
    bin_graph_vector(nbins,graph.yFit,BinXVector,BinYVector);
  else
    smooth_graph_vector(graph.yFit,BinYVector);
  Point1.x = BinXVector[0];  Point1.y = BinYVector[0];
  iDraw1 = Space2D_to_GraphPixels(Point1, *Scale);
  for (j=1; j<nbins; j++)
    {
      Point2.x = BinXVector[j];  Point2.y = BinYVector[j];
      iDraw2 = Space2D_to_GraphPixels(Point2, *Scale);
      DPRIMITIVE_DrawLine(Drawable_GraphData, COLOR_WHITE, iDraw1, iDraw2);
      iDraw1.x = iDraw2.x;
      iDraw1.y = iDraw2.y;
    }

  // Potentially display R1, tau2', or BP
  if ( glm_model.view_type >= 0 )
    {
      // Fill a vector with parameter values
      for (j=0, ParMax=0.; j<graph.npoints; j++)
	RawVector[j] = SRTM_get_parameter_value(glm_model.view_type,j);
      if ( glm_model.view_type == TYPE_R1 )
	ParMax = max_r1;
      else if ( glm_model.view_type == TYPE_k2 )
	ParMax = max_t2;
      else if ( glm_model.view_type == TYPE_k2a )
	ParMax = max_bp;
      // Test the min & max
      if ( ParMax > 0. )
	ParMax *= 2.;
      else
	ParMax /= 2.;
      if ( ParMax == 0. ) ParMax = 1.;
      // Plot parameters over the TAC curve, so re-scale the parameters setting 0 -> 0.
      for (j=0; j<graph.npoints; j++)
	RawVector[j] *= graph.ymax / ParMax;
      if ( graph.bin_data )
	bin_graph_vector(nbins,RawVector,BinXVector,BinYVector);
      else
	smooth_graph_vector(RawVector,BinYVector);
      // Plot the smoothed or binned vector.
      if ( glm_model.view_type == TYPE_R1 )       // R1
	which_color = COLOR_GREEN;
      else if ( glm_model.view_type == TYPE_k2 )  // tau2'
	which_color = COLOR_ORANGE;
      else if ( glm_model.view_type == TYPE_k2a ) // BP
	which_color = COLOR_PURPLE;
      Point1.x = BinXVector[0];   Point1.y = BinYVector[0];
      iDraw1 = Space2D_to_GraphPixels(Point1, *Scale);
      for (j=1; j<nbins; j++)
	{
	  Point2.x = BinXVector[j];  Point2.y = BinYVector[j];
	  iDraw2 = Space2D_to_GraphPixels(Point2, *Scale);
	  DPRIMITIVE_DrawLine(Drawable_GraphData, which_color, iDraw1, iDraw2);
	  iDraw1.x = iDraw2.x;
	  iDraw1.y = iDraw2.y;
	}
    }

  free_float_vector(RawVector);
  free_float_vector(BinXVector);
  free_float_vector(BinYVector);
}

void fill_SRTM_TimeSeriesModel()
{
  float sigma2, cnr, p_raw, p_cor;

  // Define dynamic (tissue-dependent) basis functions:
  if ( SRTM_define_dynamic_basis_functions(graph.npoints, graph.yVector) ) exit(1);
  SRTM_fit_data( graph.npoints, graph.yVector, graph.yFit, &sigma2 );
  // Determine the effect size & standard deviation
  SRTM_calculate_F(graph.npoints, sigma2, 0., glm_model.iCondition, &cnr, &p_raw, &p_cor,
		  &(glm_model.effect_size), &(glm_model.effect_sdev));
}

void get_info_SRTM_TimeSeriesModel( BOOL move_cursor, char *string1, char *string2, char *string3 )
{
  float time_run, time_total, graph_value;
  int ix, ifile, itime_run;

  if ( move_cursor )
    ix = graph.icursor.x;
  else
    ix = graph.iTime;
  time_total = SRTM_time_total_minutes(ix);
  time_run   = SRTM_time_run_minutes(ix);
  split_graphX_into_file_and_location(ix, &ifile, &itime_run);
  if ( ix >= 0 )
    {
      graph_value = graph.yVector[ix];
    }
  if ( move_cursor )
    graph_value = graph.cursor.y;

  sprintf(string1," run  time   y-value");
  if ( move_cursor || ix >= 0 )
    {
      sprintf(string2," %2d %6d %9.2f",ifile,itime_run,graph_value);
      sprintf(string3,"    %6.2f",time_run);
    }
  else if ( ix < 0 )
    {
      sprintf(string2," ");
      sprintf(string3," ");
    }
}

void update_SelectText_SRTM_TimeSeriesModel( struct SelectableText *SelectText )
{
  char condition_name[LEN_STR];
  int icut, irun, itime;
  float value;

  if ( SelectText->FieldID == TEXTFIELD_SRTM_OPTIONS )
    {
      SelectText->color = COLOR_CYAN;
      SelectText->visible = (graph.type == 3);
      SelectText->XPosition    = 300;
      SelectText->XSelectStart = SelectText->XPosition;
      SelectText->XSelectEnd   = SelectText->XSelectStart + 50;
      if ( glm_model.options == 0 )
	strcpy(SelectText->string,"view  :");
      else if ( glm_model.options == 1 )
	strcpy(SelectText->string,"cond. :");
      else if ( glm_model.options == 2 )
	strcpy(SelectText->string,"cutoff:");
      sprintf(SelectText->message1,"SRTM GLM options");
      sprintf(SelectText->message2,"L/R buttons cycle through options");
      sprintf(SelectText->message3," ");
    }      
  else if ( SelectText->FieldID == TEXTFIELD_SRTM_SETTINGS )
    {
      SelectText->visible = (graph.type == 3 );
      SelectText->XPosition    = 350;
      SelectText->XSelectStart = SelectText->XPosition;
      SelectText->XSelectEnd   = SelectText->XSelectStart + 100;
      if ( glm_model.options == 0 ) // view
	{
	  if ( glm_model.view_type == -1 )
	    {
	      SelectText->color = COLOR_WHITE;
	      sprintf(SelectText->string,"TAC");
	    }
	  else if ( glm_model.view_type == TYPE_R1 )
	    {
	      sprintf(SelectText->string,"  R1 ");
	      SelectText->color = COLOR_GREEN;
	    }
	  else if ( glm_model.view_type == TYPE_k2 )
	    {
	      sprintf(SelectText->string,"tau2'");
	      SelectText->color = COLOR_ORANGE;
	    }
	  else if ( glm_model.view_type == TYPE_k2a )
	    {
	      sprintf(SelectText->string,"  BP ");
	      SelectText->color = COLOR_PURPLE;
	    }
	  sprintf(SelectText->message1,"Changing graph viewing option");
	  sprintf(SelectText->message2,"TAC, R1, tau2', BP");
	  sprintf(SelectText->message3," ");
	}
      else if ( glm_model.options == 1 ) // condition
	{
	  if ( SRTM_get_condition_name(glm_model.iCondition, condition_name) )
	    {
	      printf("Programming error: condition index %d is not defined.\n",glm_model.iCondition);
	      exit(1);
	    }
	  sprintf(SelectText->string,"%s = %6.3f+-%6.3f",condition_name,glm_model.effect_size,glm_model.effect_sdev);
	  sprintf(SelectText->message1,"Condition name, effect size, and");
	  sprintf(SelectText->message2,"st. dev. from GLM fit.");
	  sprintf(SelectText->message3,"Use mouse to change conditions.");
	}
      else if ( glm_model.options == 2 ) // cutoff
	{
	  irun = SRTM_get_run_number(graph.iTime);
	  icut = SRTM_get_ignored_cutoff(irun);
	  if ( icut == -1 )
	    sprintf(SelectText->string," none ");
	  else
	    sprintf(SelectText->string," %d ",icut);
	  sprintf(SelectText->message1,"Ignore data past some time point");
	  sprintf(SelectText->message2,"Select and then respond to prompt");
	  sprintf(SelectText->message3,"Data and fit will be adjusted");
	}
    }
  else if ( SelectText->FieldID == TEXTFIELD_SRTM_PARVALUES )
    {
      SelectText->visible = ( graph.type == 3 );
      SelectText->XPosition    = 475;
      SelectText->XSelectStart = SelectText->XPosition;
      SelectText->XSelectEnd   = SelectText->XSelectStart + 100;
      value = SRTM_get_parameter_value(glm_model.iParameter,graph.iTime);
      irun  = SRTM_get_run_number(graph.iTime);
      itime = SRTM_get_itime_in_run( graph.iTime);
      sprintf(SelectText->message1,"Display parameter values");
      sprintf(SelectText->message2,"for run %d and time point %d",irun,itime);
      if ( glm_model.iParameter == TYPE_R1 )
	{
	  sprintf(SelectText->string,"    R1 = %5.2f ",value);
	  sprintf(SelectText->message3,"R1 = EF relative to ref. region");
	  SelectText->color = COLOR_GREEN;
	}
      else if ( glm_model.iParameter == TYPE_k2 )
	{
	  sprintf(SelectText->string," tau2' = %5.2f ",value);
	  sprintf(SelectText->message3,"tau2' = 1/k2a in min");
	  SelectText->color = COLOR_ORANGE;
	}
      else if ( glm_model.iParameter == TYPE_k2a )
	{
	  sprintf(SelectText->string,"    BP = %5.2f ",value);
	  sprintf(SelectText->message3,"BP = B_avail/Kd");
	  SelectText->color = COLOR_PURPLE;
	}
    }
}

void action_graph_text_selection_SRTM_TimeSeriesModel( int button )
{
  int nevents, nconditions, irun, icutoff;
  char input_string[LEN_STR], prompt_string[LEN_STR];

  if ( graph.text_selection == TEXTFIELD_SRTM_OPTIONS )
    {
      if ( button == 1 )
	glm_model.options--;
      else
	glm_model.options++;
      if ( glm_model.options > 2 )
	glm_model.options = 0;
      else if ( glm_model.options < 0 )
	glm_model.options = 2;
      graph.update_regions |= UPDATE_SELECT_TEXT_GRAPH;
    }
  else if ( graph.text_selection == TEXTFIELD_SRTM_SETTINGS )
    {
      if ( glm_model.options == 0 ) // view type
	{
	  if ( button == 1 )
	    glm_model.view_type++;
	  else
	    glm_model.view_type--;
	  if ( glm_model.view_type > 2 )
	    glm_model.view_type = -1;
	  else if ( glm_model.view_type < -1 )
	    glm_model.view_type = 2;
	  if ( glm_model.view_type >= 0 ) // not TAC
	    glm_model.iParameter = glm_model.view_type;
	  graph.update_regions |= UPDATE_GRAPH;
	  graph.update_regions |= UPDATE_SELECT_TEXT_GRAPH;
	}
      else if ( glm_model.options == 1 ) // condition
	{
	  if ( button == 1 )
	    glm_model.iCondition++;
	  else
	    glm_model.iCondition--;
	  nconditions = SRTM_get_nconditions();
	  if ( glm_model.iCondition == nconditions )
	    glm_model.iCondition = 0;
	  else if ( glm_model.iCondition < 0 )
	    glm_model.iCondition = nconditions-1;
	  graph.update_regions |= UPDATE_SELECT_TEXT_GRAPH;
	}
      else if ( glm_model.options == 2 ) // cutoff
	{
	  irun = SRTM_get_run_number(graph.iTime);
	  sprintf(prompt_string,"Input a time point for run %d",irun);
	  if ( !get_X11_string(1,"ignore data after:",prompt_string," ",2,1,input_string) )
	    {
	      show_on_info_line(1,0,"bad input","inclusion cutoff not modified","");
	      return;
	    }
	  sscanf(input_string,"%d",&icutoff);
	  if ( SRTM_set_inclusion_cutoff(irun,icutoff) )
	    {
	      show_on_info_line(1,0,"bad input","inclusion cutoff not modified","");
	      return;
	    }
	  if ( SRTM_define_dynamic_basis_functions(graph.npoints, graph.yVector) )
	    return;
	  update_graph_data(1);
	  return;
	}
      nevents = SRTM_get_number_events_inside_residue(glm_model.residue_string);
      if ( nevents == 0 )
	strcpy(glm_model.residue_string,"");
      update_graph_data(1);
      label_graph_window();
    }
  else if ( graph.text_selection == TEXTFIELD_SRTM_PARVALUES )
    {
      if ( button == 1 )
	glm_model.iParameter++;
      else
	glm_model.iParameter--;
      if ( glm_model.iParameter > 2 )
	glm_model.iParameter = 0;
      else if ( glm_model.iParameter < 0 )
	glm_model.iParameter = 2;
      graph.update_regions |= UPDATE_SELECT_TEXT_GRAPH;
    }
}

void action_graph_button23_SRTM_TimeSeriesModel()
{
}

int graph_event_handler_SRTM_TimeSeriesModel( char key )
{
  return(0);
}

void label_graph_time_window_SRTM_TimeSeriesModel(char *FileName )
{
}
