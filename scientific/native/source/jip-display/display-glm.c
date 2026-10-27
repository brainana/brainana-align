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
#include "fftw3.h"
#include "util.h"
#include "display.h"
#include "display-X11.h"
#include "glm-prototypes.h"


void initialize_GLM_TimeSeriesModel()
{
  glm_model.options = 0;
  glm_model.baseline = 1;
  glm_model.show_glm_windows = 2;
  glm_model.iCondition = 0.;
  if (glm_model.residue_string == NULL)
    strcpy(glm_model.residue_string,"");

  GLM_input.nruns = 0;
  GLM_input.nbaseline_terms = 3;
  glm_model.event_number_active = 1; // default event number
  strcpy(GLM_input.condition_string,"");
  // Graph text /////////////////////////////////////
  // GLM options
  new_selectable_graph_text(2, TEXTFIELD_GLM_OPTIONS);
  // GLM settings
  new_selectable_graph_text(2, TEXTFIELD_GLM_SETTINGS);
  // baseline
  new_selectable_graph_text(2, TEXTFIELD_GLM_BASELINE);
  // chi2
  new_selectable_graph_text(4, TEXTFIELD_CHI2);
}

int read_GLM_TimeSeriesModel( char *file_name )
{
  if ( GLM_read_control_file(file_name,0) ) return(1);
  // Don't go throught the auto-correlation procedure for xdisplay
  GLM_input.AR_order = 0;
  return(0);
}

int define_GLM_TimeSeriesModel( int nflags, int *flags )
{
  char condition_string[LEN_STR], file_name[LEN_STR];
  int nconditions, j;
  BOOL IncludeTimeData, UseT, UseS, Verbose;

  if ( nflags != 4 )
    {
      printf("Programming error 1 in function define_model\n");
      exit(1);
    }
  else
    {
      IncludeTimeData = flags[0];
      UseT            = flags[1];
      UseS            = flags[2];
      Verbose         = flags[3];
    }
  if ( IncludeTimeData )
    { // Include all time data from all runs.
      for ( j=0; j<GLM_input.nruns; j++)
	add_image_file( &image_files, GLM_input.run_name[j], 0, 0);
      GLM_define_basis_functions();
      if ( GLM_input.notify ) GLM_describe_basis_functions();
    }
  else if ( GLM_input.nruns > 0 )
    { // Sometimes the time data exceeds memory limits or the user doesn't want to load it.
      // Just use the average file when requested.
      if ( GLM_input.nifti_output )
	add_image_file( &image_files, "average.nii", 0, 0);
      else
	add_image_file( &image_files, "average.bfloat", 0, 0);
    }
  glm_model.shape = GLM_get_paradigm_shape(-1);
  if ( glm_model.shape != SHAPE_SQUARE || glm_model.shape != SHAPE_GAMMA )
    glm_model.shape = SHAPE_GAMMA;
  // Get the number of conditions.
  GLM_define_conditions( IncludeTimeData, GLM_input.condition_string, &nconditions );
  // Add to the list of activation files, based upon the existing conditions.
  for (j=0; j<nconditions; j++)
    { // overwrite the condition string
      GLM_get_condition_string(j,condition_string);
      if ( GLM_input.nifti_output )
	sprintf(file_name,"P-%s.nii",condition_string);
      else
	sprintf(file_name,"P-%s.bfloat",condition_string);
      add_image_file( &activation_files, file_name, 1, 1);
      if ( UseT )
	{
	  if ( GLM_input.nifti_output )
	    sprintf(file_name,"T-%s.nii",condition_string);
	  else
	    sprintf(file_name,"T-%s.bfloat",condition_string);
	  add_image_file( &activation_files, file_name, 0, 2);
	}
      if ( UseS )
	{
	  if ( GLM_input.nifti_output )
	    sprintf(file_name,"S-%s.nii",condition_string);
	  else
	    sprintf(file_name,"S-%s.bfloat",condition_string);
	  add_image_file( &activation_files, file_name, 0, 3);
	}
    }
  
  glm_model.shape = GLM_get_paradigm_shape(-1);
  if ( glm_model.shape != SHAPE_SQUARE || glm_model.shape != SHAPE_GAMMA )
    glm_model.shape = SHAPE_GAMMA;
  // Start off with column events in the residue.
  GLM_define_table_residue(glm_model.residue_string);

  return(0);
}

void plot_GLM_TimeSeriesModel( POINT2D *Scale )
{
  int which_color;
  char string[LEN_STR];
  POINT2D Point1, Point2;
  IPOINT2D iDraw1, iDraw2;
  int j, ishape;int nbins;int index_gc;
  float stim_on, stim_off, yvalue, range;
  float *BinXVector, *BinYVector, *RawVector;
  int nevents, nexclude, jevent, event_number, istart, iend;
  unsigned long red;

  // Re-scale the graph unless the scale has been fixed.
  // This needs to be redone here due to the possibility that time points are ignored.
  if ( graph.ymin == graph.ymax || !graph.yrange_fixed )
    {
      // Find the min and max for the current time vector.
      graph.ymin =  1.e10;
      graph.ymax = -1.e10;
      for (j=0; j<graph.npoints; j++)
	{
	  if ( !GLM_is_excluded(j) ) // Ignore time point in the GLM
	    {
	      if ( graph.yVector[j] < graph.ymin ) graph.ymin = graph.yVector[j];
	      if ( graph.yVector[j] > graph.ymax ) graph.ymax = graph.yVector[j];
	    }
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

  if ( glm_model.show_glm_windows ) // time graph
    {
      // Display shaded regions for event windows in the GLM
      nevents = GLM_get_number_events();
      for (jevent=0; jevent<nevents; jevent++)
	{
	  event_number = GLM_get_event_number(jevent);
	  index_gc = event_number - 1;
	  if ( index_gc > 8 )
	    index_gc = 8;
	  ishape = GLM_get_paradigm_shape(event_number);
	  // Set the color for the shaded area based upon the stimulus type.
	  red = WHITE_PIXEL * ( 0.25 + 0.75*(float)((nevents-jevent-1)/(float)nevents) );
	  //	  red = (red<<8) + red;
	  red = (red<<16);
	  // Get the stimuli
	  GLM_get_stimulus(-1,&stim_on,&stim_off);
	  while ( GLM_get_stimulus(jevent,&stim_on,&stim_off) )
	    {
	      if ( ishape != SHAPE_TABLE )
		{
		  Point1.x = stim_on;       Point1.y = graph.ymax;
		  Point2.x = stim_off;      Point2.y = graph.ymin;
		  iDraw1 = Space2D_to_GraphPixels(Point1, *Scale);
		  iDraw2 = Space2D_to_GraphPixels(Point2, *Scale);
		  iDraw1.x -= Scale->x/2.;  iDraw2.x += Scale->x/2.;
		  if ( !GLM_event_is_in_string(glm_model.residue_string,jevent) )
		    DPRIMITIVE_FillRectangle(Drawable_GraphData, COLOR_GRAY, iDraw1, iDraw2);
		}
	    }
	}
      if ( glm_model.show_glm_windows == 2 )
	{
	  // Display time windows excluded from the GLM
	  nexclude = GLM_get_number_excluded_regions();
	  for (jevent=0; jevent<nexclude; jevent++)
	    {
	      GLM_get_excluded_region(jevent, &istart, &iend);
	      Point1.x = istart - 0.5;  Point1.y = graph.ymax;
	      Point2.x = iend   + 0.5;  Point2.y = graph.ymin;
	      iDraw1 = Space2D_to_GraphPixels(Point1, *Scale);
	      iDraw2 = Space2D_to_GraphPixels(Point2, *Scale);
	      DPRIMITIVE_FillRectangle(Drawable_GraphData, COLOR_BLUE, iDraw1, iDraw2);
	    }
	}
      // label the stimuli by type
      nevents = GLM_get_number_events();
      for (jevent=0; jevent<nevents; jevent++)
	{
	  // Get the stimuli
	  event_number = GLM_get_event_number(jevent);
	  if ( GLM_EventStringFromInteger(event_number,string) )
	    {
	      printf("Programming error: creating an event label for event %d\n",event_number);
	      exit(1);
	    }
	  index_gc = jevent - 1;
	  if ( index_gc > 8 )
	    index_gc = 8;
	  if ( !GLM_event_is_in_string(glm_model.residue_string,jevent) )
	    which_color = COLOR_WHITE;
	  else
	    which_color = COLOR_BLUE;
	  GLM_get_stimulus(-1,&stim_on,&stim_off);
	  while ( GLM_get_stimulus(jevent,&stim_on,&stim_off) )
	    {
	      iDraw1.x = graph_x( Scale->x, stim_on-0.5 );
	      yvalue = (graph.ymax+graph.ymin)/2.;
	      iDraw1.y = graph_y( Scale->y, yvalue ) + INFO_HEIGHT*(index_gc-nevents/4);
	      DPRIMITIVE_FillText(Drawable_GraphData, which_color, iDraw1, string);
	    }
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

  // Display the fit for each stimulus type
  nevents = GLM_get_number_events();
  for (jevent=0; jevent<nevents; jevent++)
    {
      for (j=0; j<graph.npoints; j++)
	{
	  if ( GLM_event_fit(glm_model.baseline, glm_model.residue_string, jevent, j, RawVector+j) )
	    return;
	}
      if ( graph.bin_data )
	bin_graph_vector(nbins,RawVector,BinXVector,BinYVector);
      else
	smooth_graph_vector(RawVector,BinYVector);
      // Plot the smoothed vector.
      Point1.x = BinXVector[0];   Point1.y = BinYVector[0];
      iDraw1 = Space2D_to_GraphPixels(Point1, *Scale);
      for (j=1; j<nbins; j++)
	{
	  Point2.x = BinXVector[j];  Point2.y = BinYVector[j];
	  iDraw2 = Space2D_to_GraphPixels(Point2, *Scale);
	  DPRIMITIVE_DrawLine(Drawable_GraphData, COLOR_RED, iDraw1, iDraw2);
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
  free_float_vector(RawVector);
  free_float_vector(BinXVector);
  free_float_vector(BinYVector);
}

void fill_GLM_TimeSeriesModel()
{
  float cnr, p_raw, p_cor;
  int nconditions, iFilter;

  // Fit the data using the GLM
  GLM_fit_data( graph.npoints, graph.yVector, graph.yFit, &glm_model.chi2_dof );

  nconditions = GLM_get_nconditions();
  if ( nconditions != 0 )
    GLM_calculate_F( graph.npoints, glm_model.chi2_dof, 0., glm_model.iCondition, &cnr, &p_raw, &p_cor,
		     &(glm_model.effect_size), &(glm_model.effect_sdev) );
  // Remove the baseline if requested.
  if ( !glm_model.baseline )
    if ( GLM_remove_baseline(0, graph.npoints, graph.yVector, graph.yFit) )
      return;
  // Make a residue from the data, if requested.
  if ( GLM_create_selective_residue(glm_model.residue_string,
				    graph.npoints, graph.yVector, graph.yFit) )
    return;

  if ( graph.freq_domain )
    {
      iFilter = 1;
      FT_graph_data(iFilter);
    }
}

void FT_graph_data( int iFilter )
{
  static fftw_plan plan_ft1d;
  static fftw_complex *Data_out;
  static double *Data_in;
  int j, nInput, nOutput, icomplex;
  float dist, filter;
  unsigned flags=0;
  //  static BOOL first_time=TRUE;

  //  if ( first_time )
  //    {
      nInput  = graph.npoints;
      nOutput = graph.npoints/2 + 1;
      // Allocate arrays.
      printf("allocate %d and %d points\n",nInput,nOutput);
      Data_in  = double_vector(nInput);
      Data_out = (fftw_complex *)fftw_malloc((size_t)jip_product(sizeof(fftw_complex),nOutput));
      //      first_time = FALSE;
      //   }
  printf("plan r2c\n");
  // real to complex FT
  plan_ft1d = fftw_plan_dft_r2c_1d(nInput, Data_in, Data_out, flags);

  for (j=0; j<graph.npoints; j++)
    Data_in[j] = graph.yVector[j];

  printf("fftw\n");
  // Transform the data.
  fftw_execute(plan_ft1d);
  if ( !iFilter )
    {
      // Put the transformed data back into the original vector
      printf("put back\n");
      for (j=0; j<graph.npoints; j++)
	{
	  icomplex = j/2;
	  printf("j = %d, icomplex = %d\n",j,icomplex);
	  graph.yVector[j] = sqrt((double)(SQR(Data_out[icomplex][0]) + SQR(Data_out[icomplex][1])));
	}
    }
  else
    {
      for (j=0; j<graph.npoints/2; j++)
	{
	  // Filter here
	  dist = (float)j / (float)(graph.npoints/2-1);  // 0 to 1
	  dist = j;
	  filter = 1. / (1. + pow( (double)dist/20.,6.) );
	  //	  filter = cos((double)(PI/2.*dist));
	  printf("j=%d, filter = %g\n",j,filter);
	  Data_out[j][0] *= filter;
	  Data_out[j][1] *= filter;
	}
      //
      plan_ft1d = fftw_plan_dft_c2r_1d(nInput, Data_out, Data_in, flags);
      // Transform the data.
      fftw_execute(plan_ft1d);
      for (j=0; j<graph.npoints; j++)
	graph.yVector[j] = Data_in[j] / (float)graph.npoints;
    }
}

void get_info_GLM_TimeSeriesModel( BOOL move_cursor, char *string1, char *string2, char *string3 )
{
  char GLM_description[LEN_STR];
  float time_total, time_run, graph_value, tr;
  int ix, ifile, itime_run;

  if ( move_cursor )
    ix = graph.icursor.x;
  else
    ix = graph.iTime;
  sprintf(string1," run  time       graph event");
  split_graphX_into_file_and_location(ix, &ifile, &itime_run);
  if ( GLM_get_tr(ifile-1,&tr) )
    {
      printf("Error: the GLM tr is not defined in file display-glm.c\n");
      exit(1);
    }
  //  time_run = itime_run * mosaic.stack->resolution_t;
  time_run = itime_run * tr;
  if ( ix >= 0 )
    {
      if ( GLM_identify_time_point(ix, glm_model.residue_string, GLM_description) )
	{
	  printf("Programming error in function get_info_GLM_TimeSeriesModel.\n");
	  printf("Error trying to interpret point %d from GLM.\n",ix);
	  exit(1);
	}
      time_total  = graph.xVector[ix];
      graph_value = graph.yVector[ix];
    }
  if ( move_cursor )
    {
      time_total  = graph.cursor.x;
      graph_value = graph.cursor.y;
    }
  if ( move_cursor || ix >= 0 )
    {
      sprintf(string2," %2d %5d %9.2f  %s",ifile,itime_run,graph_value,GLM_description);
      sprintf(string3," %8g",time_run);
    }
  else if ( ix < 0 )
    {
      sprintf(string2," ");
      sprintf(string3," ");
    }

}

void update_SelectText_GLM_TimeSeriesModel( struct SelectableText *SelectText )
{
  char event_string[LEN_STR], condition_name[LEN_STR];
  int nconditions;

  if ( SelectText->FieldID == TEXTFIELD_GLM_OPTIONS )
    {
      SelectText->color = COLOR_CYAN;
      SelectText->visible = (graph.type == 3);
      SelectText->XPosition    = 300;
      SelectText->XSelectStart = SelectText->XPosition;
      SelectText->XSelectEnd   = SelectText->XSelectStart + 50;
      if ( glm_model.options == 0 )
	strcpy(SelectText->string," event :");
      else if ( glm_model.options == 1 )
	strcpy(SelectText->string," shape :");
      else if ( glm_model.options == 2 )
	strcpy(SelectText->string,"exclude:");
      else if ( glm_model.options == 3 )
	strcpy(SelectText->string,"include:");
      else if ( glm_model.options == 4 )
	strcpy(SelectText->string,"baseline");
      else if ( glm_model.options == 5 )
	strcpy(SelectText->string,"condition:");
      sprintf(SelectText->message1,"Define events, shapes, ...");
      sprintf(SelectText->message2,"L/R buttons cycle through options");
      sprintf(SelectText->message3," ");
    }      
  else if ( SelectText->FieldID == TEXTFIELD_GLM_SETTINGS )
    {
      SelectText->color = COLOR_CYAN;
      SelectText->visible = (graph.type == 3 );
      SelectText->XPosition    = 370;
      SelectText->XSelectStart = SelectText->XPosition;
      SelectText->XSelectEnd   = SelectText->XSelectStart + 100;
      if ( glm_model.options == 0 )      // event
	{
	  if ( glm_model.event_number_active > 0 )
	    sprintf(SelectText->string,"  %c  ",GLM_EventChar(glm_model.event_number_active));
	  else
	    strcpy(SelectText->string,"ignore");
	  sprintf(SelectText->message1,"Set the event number");
	  sprintf(SelectText->message2,"L/R buttons cycle through options");
	  sprintf(SelectText->message3,"Options = ignore or event IDs ");
	}
      else if ( glm_model.options == 1 ) // shape
	{
	  if ( glm_model.shape == SHAPE_SQUARE )
	    strcpy(SelectText->string,"square");
	  else if ( glm_model.shape == SHAPE_GAMMA )
	    strcpy(SelectText->string,"gamma");
	  else if ( glm_model.shape == SHAPE_TABLE )
	    strcpy(SelectText->string,"table");
	  sprintf(SelectText->message1,"Set the event shape");
	  sprintf(SelectText->message2,"L/R buttons cycle through options");
	  sprintf(SelectText->message3,"('table' is like correlation)");
	}
      else if ( glm_model.options == 2 ) // exclude
	{
	  strcpy(event_string,glm_model.residue_string);
	  if ( jip_int_count((JIP_INDEX)strlen(event_string)) == 0 )
	    sprintf(SelectText->string," none ");
	  else
	    sprintf(SelectText->string," %s ",event_string);
	  sprintf(SelectText->message1,"Exclude a list of events");
	  sprintf(SelectText->message2,"Select and then respond to prompt");
	  sprintf(SelectText->message3,"Data and fit will be adjusted");
	}
      else if ( glm_model.options == 3 ) // include
	{
	  GLM_define_residue_from_inclusion(glm_model.residue_string,event_string);
	  if ( jip_int_count((JIP_INDEX)strlen(event_string)) == 0 )
	    sprintf(SelectText->string," none ");
	  else
	    sprintf(SelectText->string," %s ",event_string);
	  sprintf(SelectText->message1,"Include a list of events");
	  sprintf(SelectText->message2,"Select and then respond to prompt");
	  sprintf(SelectText->message3,"Data and fit will be adjusted");
	}
      else if ( glm_model.options == 4 ) // baseline-terms
	{
	  sprintf(SelectText->string," %d ",GLM_get_nbaseline_terms());
	  sprintf(SelectText->message1,"Define polynomial for baseline.");
	  sprintf(SelectText->message2,"3 terms for DC + quadratic drift.");
	  sprintf(SelectText->message3,"Select and then respond to prompt");
	}
      else if ( glm_model.options == 5 ) // condition
	{
	  nconditions = GLM_get_nconditions();
	  if ( nconditions == 0 )
	    sprintf(SelectText->string," none");
	  else if ( GLM_get_condition_string(glm_model.iCondition, condition_name) )
	    {
	      printf("Programming error: condition index %d is not defined.\n",glm_model.iCondition);
	      exit(1);
	    }
	  else
	    sprintf(SelectText->string," %s = %6.3f+-%6.3f",condition_name,glm_model.effect_size,glm_model.effect_sdev);
	  sprintf(SelectText->message1,"Condition name, effect size, and");
	  sprintf(SelectText->message2,"st. dev. from GLM fit.");
	  sprintf(SelectText->message3,"Use mouse to change conditions.");
	}
    }
  else if ( SelectText->FieldID == TEXTFIELD_GLM_BASELINE )
    {
      SelectText->color = COLOR_CYAN;
      SelectText->visible = ( graph.type == 3 );
      SelectText->XPosition    = 500;
      SelectText->XSelectStart = SelectText->XPosition;
      SelectText->XSelectEnd   = SelectText->XSelectStart + 40;
      strcpy(SelectText->string," b ");
      sprintf(SelectText->message1,"Remove/reinstate GLM baseline");
      sprintf(SelectText->message2,"Keyboard equivalent: b");
      sprintf(SelectText->message3," ");
    }      
  else if ( SelectText->FieldID == TEXTFIELD_CHI2 )
    {
      SelectText->color = COLOR_GRAY;
      SelectText->visible = ( graph.type == 3 );
      SelectText->XPosition    = 500;
      SelectText->XSelectStart = SelectText->XPosition;
      SelectText->XSelectEnd   = SelectText->XSelectStart + 40;
      sprintf(SelectText->string,"%5.2e",glm_model.chi2_dof);
      sprintf(SelectText->message1,"Chi-squared per degree of freedom");
      sprintf(SelectText->message2,"from GLM fit to data");
      sprintf(SelectText->message3," ");
    }      
}

void action_graph_text_selection_GLM_TimeSeriesModel( int button )
{
  int nevents, nbaseline, nconditions;
  char input_string[LEN_STR];

  if ( graph.text_selection == TEXTFIELD_GLM_OPTIONS )
    {
      if ( button == 1 )
	glm_model.options++;
      else
	glm_model.options--;
      if ( glm_model.options > 5 )
	glm_model.options = 0;
      else if ( glm_model.options < 0 )
	glm_model.options = 5;
      graph.update_regions |= UPDATE_SELECT_TEXT_GRAPH;
    }
  else if ( graph.text_selection == TEXTFIELD_GLM_SETTINGS )
    {
      if ( glm_model.options == 0 )      // event number
	{
	  glm_model.event_number_active = GLM_increment_event_number(glm_model.event_number_active);
	  graph.update_regions |= UPDATE_SELECT_TEXT_GRAPH;
	}
      else if ( glm_model.options == 1 ) // shape
	{
	  glm_model.shape++;
	  if ( glm_model.shape > SHAPE_TABLE )
	    glm_model.shape = SHAPE_SQUARE;
	  graph.update_regions |= UPDATE_SELECT_TEXT_GRAPH;
	}
      else if ( glm_model.options == 2 ) // exclusion
	{
	  if ( !get_X11_string(1,"Exclude events","(event numbers, DEL, RET, ESC)"," ",
			       3,1,input_string) )
	    {
	      show_on_info_line(1,0,"no events","","");
	      return;
	    }
	  strcpy(glm_model.residue_string,input_string);
	  nevents = GLM_get_number_events_inside_residue(glm_model.residue_string);
	  if ( nevents == 0 )
	    strcpy(glm_model.residue_string,"");
	  update_graph_data(1);
	  label_graph_window();
	}
      else if ( glm_model.options == 3 ) // inclusion
	{
	  if ( !get_X11_string(1,"Include events","(event numbers, DEL, RET, ESC)"," ",
			       3,1,input_string) )
	    {
	      show_on_info_line(1,0,"no events","","");
	      return;
	    }
	  GLM_define_residue_from_inclusion(input_string,glm_model.residue_string);
	}
      else if ( glm_model.options == 4 ) // baseline terms
	{
	  if ( !get_X11_string(1,"# baseline terms","0,1,2,3,4"," ",
			       2,1,input_string) )
	    {
	      show_on_info_line(1,0,"bad input","baseline terms not modified","");
	      return;
	    }
	  sscanf(input_string,"%d",&nbaseline);
	  if ( GLM_set_nbaseline_terms(nbaseline) )
	    {
	      show_on_info_line(1,0,"bad input","baseline terms not modified","");
	      return;
	    }
	  GLM_define_basis_functions();
	  update_graph_data(1);
	  return;
	}
      else if ( glm_model.options == 5 ) // condition
	{
	  if ( button == 1 )
	    glm_model.iCondition++;
	  else
	    glm_model.iCondition--;
	  nconditions = GLM_get_nconditions();
	  if ( glm_model.iCondition >= nconditions )
	    glm_model.iCondition = 0;
	  else if ( glm_model.iCondition < 0 )
	    glm_model.iCondition = nconditions-1;
	  graph.update_regions |= UPDATE_SELECT_TEXT_GRAPH;
	}
      nevents = GLM_get_number_events_inside_residue(glm_model.residue_string);
      if ( nevents == 0 )
	strcpy(glm_model.residue_string,"");
      update_graph_data(1);
      label_graph_window();
    }
  else if ( graph.text_selection == TEXTFIELD_GLM_BASELINE )
    {
      glm_model.baseline = !glm_model.baseline;
      update_graph_data(1);
    }
  else if ( graph.text_selection == TEXTFIELD_GRAPH_MODEL )
    {
      graph.freq_domain = !graph.freq_domain;
      update_graph_data(1);
    }
}

void action_graph_button23_GLM_TimeSeriesModel()
{
  char string[LEN_STR];

  if ( glm_model.event_number_active == -1 )
    {
      // then the selected range should be ignored.
      GLM_modify_excluded_region( 0, graph.ix_low_select, graph.ix_high_select );
      GLM_define_basis_functions();
    }
  else
    {
      if ( graph.ix_low_select == graph.ix_high_select )
	graph.ix_low_select = -1;  // set low value to -1, save high value location
      if ( glm_model.shape == SHAPE_SQUARE )
	action_define_square_function(glm_model.event_number_active);
      else if ( glm_model.shape == SHAPE_GAMMA )
	action_fit_gamma(glm_model.event_number_active);
      else if ( glm_model.shape == SHAPE_TABLE )
	action_define_table_event(glm_model.event_number_active);
    }
  if ( graph.ix_low_select < 1 )
    sprintf(string,"deleted selection at location %d",graph.ix_high_select);
  else
    sprintf(string,"selected %d to %d",graph.ix_low_select,graph.ix_high_select);
  show_on_info_line(0,0,string,"","");

  fill_graph_vector(0);
}

int graph_event_handler_GLM_TimeSeriesModel( char key )
// return value: 1 = found, 0 = not found
{
  int found;

  found = 1;
  switch (key)
    {
    case 'G'-'@': // write glm files
      action_write_glm_files();
      break;
    case 'b':
      glm_model.baseline = !glm_model.baseline;
      graph.update_regions |= UPDATE_GRAPH;
      break;
    case ' ':
      glm_model.show_glm_windows++;
      if ( glm_model.show_glm_windows > 2 )
	glm_model.show_glm_windows = 0;
      build_graph_display(1);
      break;
    case 'e':
      glm_model.event_number_active = GLM_increment_event_number(glm_model.event_number_active);
      show_information_graph(0);
      break;
    default:
      found = 0;
      break;
    }
  return(found);
}

void label_graph_time_window_GLM_TimeSeriesModel( char *FileName )
{
  char string[LEN_STR];
  int j;

  if ( jip_int_count((JIP_INDEX)strlen(glm_model.residue_string)) == 0 )
    sprintf(FileName,".roi");
  else
    {
      sprintf(FileName,".roi-");
      for (j=0; j<GLM_get_number_events(); j++)
	{
	  if ( !GLM_event_is_in_string(glm_model.residue_string,j) )
	    {
	      GLM_EventStringFromInteger(GLM_get_event_number(j),string);
	      strcat(FileName,string);
	    }
	}
    }
}

int action_define_table_event( int event_number )
{
  if ( event_number < 0 )
    return(0); // ignore
  else if ( graph.ix_low_select < 0 )
    GLM_delete_1_stimulus(event_number, (float)graph.ix_high_select);
  else
    // Define this stimulus
    GLM_define_table_event_interactive(event_number, graph.yVector);
  GLM_define_basis_functions();
  GLM_describe_basis_functions();
  GLM_describe_stimuli();
  return(0);
}

int action_define_square_function( int event_number )
{
  if ( event_number < 0 )
    return(0); // ignore
  else if ( graph.ix_low_select < 0 )
    GLM_delete_1_stimulus(event_number, (float)graph.ix_high_select);
  else
    // Define this stimulus
    GLM_define_1_stimulus(event_number, SHAPE_SQUARE,
			  (float)graph.ix_low_select,(float)graph.ix_high_select,0.);
  GLM_define_basis_functions();
  return(0);
}

int action_fit_gamma( int event_number )
{
  float slope, offset, amp, rate, tau;
  double *xarray, *yarray;
  int iGraph, error;JIP_INDEX j;JIP_INDEX npoints;JIP_INDEX index;

  if ( graph.type == 0 )
    iGraph = mosaic.icross.x;
  else if ( graph.type == 1 )
    iGraph = mosaic.icross.y;
  else if ( graph.type == 2 )
    iGraph = mosaic.icross.z;
  else if ( graph.type == 3 )
    iGraph = graph.iTime;
  else if ( graph.type == 4 )
    {
      printf("Programming error: graph type = 4 in function action_fit_gamma().\n");
      exit(1);
    }

  printf("range: %d %d\n",graph.ix_low_select,graph.ix_high_select);

  if ( event_number < 0 )
    return(0); // ignore
  else if ( graph.ix_low_select < 0 )
    GLM_delete_1_stimulus(event_number, (float)graph.ix_high_select);
  else if ( iGraph > graph.ix_low_select && iGraph < graph.ix_high_select )
    {
      tau = iGraph - graph.ix_low_select;
      GLM_define_1_stimulus(event_number, SHAPE_GAMMA,
			    (float)graph.ix_low_select,
			    (float)graph.ix_high_select,
			    tau);
    }
  else
    {
      // Copy the data vector into the fit vector.
      for (j=0; j<graph.npoints; j++)
	graph.yFit[j] = graph.yVector[j];
      if ( graph.ix_low_select  < 0 || graph.ix_low_select  > graph.npoints - 1 ||
	   graph.ix_high_select < 0 || graph.ix_high_select > graph.npoints - 1 )
	{
	  printf("Programming error: graph.ix_low_select (or high_select) out of range.\n");
	  exit(1);
	}
      // Detread the fit vector
      // 1) set endpoints to 0
      // 2) set all values outside the selected range to 0
      slope = (graph.yFit[graph.ix_high_select] - graph.yFit[graph.ix_low_select])
	/ (float)(graph.ix_high_select - graph.ix_low_select);
      offset = graph.yFit[graph.ix_high_select] - slope * graph.ix_high_select;
      for (j=0; j<graph.npoints; j++)
	{
	  if ( j >= graph.ix_low_select && j <=graph.ix_high_select )
	    graph.yFit[j] -= slope * j + offset;
	  else
	    graph.yFit[j] = 0.;
	}
      // Allocate array for fitting
      npoints = graph.ix_high_select - graph.ix_low_select + 1;
      xarray  = double_vector(npoints);
      yarray  = double_vector(npoints);
      // Give reasonable starting values by setting
      // 1) amp = maximum value of array over specified range,
      // 2) rate = 1/4 the range
      amp = 0.;
      for (j=0; j<npoints; j++)
	{
	  index = graph.ix_low_select + j;
	  if ( index < 0 || index > graph.npoints - 1 )
	    {
	      printf("Programming error: index out of range in action_fit_gamma().\n");
	      exit(1);
	    }
	  xarray[j]  = j;
	  yarray[j]  = graph.yFit[index];
	  // Set starting amplitude to signed maximum value within the range.
	  if ( fabs((double)(yarray[j])) > fabs((double)amp) )
	    amp = yarray[j];
	}
      rate = 20./(float)npoints; // starting time constant ~ npoints/2
      
      error = fit_gamma(jip_int_count(npoints),xarray,yarray,&amp,&rate);
      free_double_vector(xarray);
      free_double_vector(yarray);
      tau = 1./rate;
      if ( error )
	{
	  printf("Error fitting Gamma function\n");
	  return(1);
	}

      GLM_define_1_stimulus(event_number, SHAPE_GAMMA,
			    (float)graph.ix_low_select,
			    (float)graph.ix_high_select,
			    tau);
   }
  GLM_define_basis_functions();

  return(0);
}

int action_compute_p_value()
{ // xdisplay only
  char condition_string[LEN_STR], info[LEN_STR];
  struct file_list *this_map, *free_me;
  IPOINT3D voxel;
  int nconditions;JIP_INDEX npts, index, index_activation, offset_map;
  int jt, jcondition, jmap;
  float cnr, p_raw, p_cor, logsig, size, sdev, sigma2;

  // Make sure the basis functions have been defined.
  if ( GLM_get_number_events() == 0 )
    {
      printf("This function requires a defined GLM model.\n");
      return(1);
    }
  else
    {
      GLM_describe_stimuli();
      GLM_describe_basis_functions();
    }
  // Reset the activation stack if necessary.
  if ( mosaic.activation.stack->nt != 0 )
    free_float_vector(mosaic.activation.stack->SpaceTime.fData);
  // Create all conditions.
  if ( GLM_create_all_conditions(condition_string) )
    {
      mosaic.activation.visible = 0;
      return(1);
    }
  if ( GLM_define_conditions(1, condition_string, &nconditions) )
    {
      mosaic.activation.visible = 0;
      return(1);
    }
  mosaic.activation.stack->dim = mosaic.stack->dim;
  mosaic.activation.stack->n3d = mosaic.stack->n3d;
  mosaic.activation.stack->nt  = nconditions;
  npts = mosaic.activation.stack->n3d;
  mosaic.activation.stack->SpaceTime.fData = float_vector(npts*mosaic.activation.stack->nt);
  mosaic.activation.nmaps = nconditions;

  // Loop over x, y, and z
  for (voxel.z=0; voxel.z<mosaic.stack->dim.z; voxel.z++)
    {
      for (voxel.y=0; voxel.y<mosaic.stack->dim.y; voxel.y++)
	{
	  for (voxel.x=0; voxel.x<mosaic.stack->dim.x; voxel.x++)
	    {
	      index_activation = voxel3D_to_1D(voxel);
	      if ( index_activation % (npts/20) == 0)
		{
		  sprintf(info,"calculating p: %td %c",5*index_activation/(npts/20),37);
		  show_on_info_line(0,0,info,"","");
		  refresh_display_image(0);
		  update_graph_data(0);
		}
	      fflush(stdout);
	      // .. and time for each (x,y) pair
	      for (jt=0; jt<mosaic.stack->nt; jt++)
		{
		  index = index4d(voxel.x,voxel.y,voxel.z,jt);
		  graph.yVector[jt] = mosaic.stack->SpaceTime.fData[index];
		}
	      // 
	      if ( GLM_fit_data(mosaic.stack->nt, graph.yVector, graph.yFit, &sigma2) )
		return(1);
	      for (jcondition=0; jcondition<nconditions; jcondition++)
		{
		  if ( GLM_calculate_F(mosaic.stack->nt, sigma2, npts,
				       jcondition, &cnr, &p_raw, &p_cor, &size, &sdev) )
		    return(1);
		  logsig = -log10((double)p_raw);
		  if ( cnr < 0. ) logsig *= -1;
		  offset_map = jcondition * mosaic.activation.stack->n3d;
		  mosaic.activation.stack->SpaceTime.fData[offset_map + index_activation] = logsig;
		}
	    }
	}
    }
  // Delete the existing map list.
  this_map = activation_files.next;
  while ( this_map )
    {
      free_me = this_map;
      this_map = this_map->next;
      free((char *)free_me);
    }
  // Initialize the activation root name.
  strcpy(activation_files.file_name,"");
  // Create the new list of activation maps.
  for ( jmap=0; jmap<nconditions; jmap++ )
    {
      GLM_get_condition_string(jmap, condition_string);
      add_image_file( &activation_files, condition_string, 1, 1 );
    }
  // Add necessary information to each list member.
  for ( this_map = &activation_files, jmap=0;
	this_map;
	this_map = this_map->next, jmap++ )
    {
      this_map->nt = 1;
      this_map->header.storage_type = 3;
      this_map->header.byte_order = 1;
      this_map->start_image = this_map->end_image = jmap;
    }
  mosaic.activation.imap = 0;
  mosaic.activation.fSpaceMatrix = &(mosaic.activation.stack->SpaceTime.fData[0]);
  mosaic.activation.scale_log = 1;
  mosaic.activation.visible = 1;
  strcpy(mosaic.activation.map_name,activation_files.file_name);

  return(0);
}

int action_write_glm_files()
{ // xdisplay only
  struct file_list *this_file;
  char condition_string[LEN_STR];
  int nruns, nconditions;

  // Make sure the basis functions have been defined.
  if ( GLM_get_number_events() == 0 )
    {
      printf("This function requires a defined GLM model.\n");
      return(1);
    }
  else
    {
      GLM_describe_stimuli();
      GLM_describe_basis_functions();
    }
  // See if conditions are already defined.
  if ( GLM_get_condition_string(0,condition_string) )
    { // If not defined, create conditions
      // Create all conditions.
      if ( GLM_create_all_conditions(condition_string) )
	{
	  printf("Error creating conditions.\n");
	  return(1);
	}
      if ( GLM_define_conditions(1, condition_string, &nconditions) )
	{
	  printf("Error defining conditions.\n");
	  return(1);
	}
    }
  // Put the list of runs into the GLM input structure.
  for (this_file=&image_files, nruns=0; this_file; this_file=this_file->next, nruns++)
    strcpy(GLM_input.run_name[nruns],this_file->file_name);
  GLM_input.nruns = nruns;

  // Write the glm control files and the timing files.
  if ( GLM_write_glm_files("glm.dat","run") ) return(1);

  return(0);
}

void action_ignore_out_of_range()
{ // xdisplay only
  int j;

  if ( !GLM_is_initialized() ) return;

  // Automatically exclude any regions where the data is out of range on the graph.
  for (j=0; j<mosaic.stack->nt; j++)
    {
      if ( graph.yVector[j] < graph.ymin || graph.yVector[j] > graph.ymax )
	GLM_modify_excluded_region(1, j, j);
    }
  GLM_define_basis_functions();
  return;
}
