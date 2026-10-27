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
#include "rem-prototypes.h"

struct file_list sigma2_file;
struct data_stack sigma2_stack;


void initialize_REM_TimeSeriesModel()
{
  REM_input.nfiles = 0;
  REM_input.multiple_comparisons = 0;
  REM_input.smoothed_mm = 0.;
  REM_input.smooth_ratio_mm = 1.;
  glm_model.baseline = 1;    // reuse this value from GLM
  strcpy(REM_input.condition_string,"");
}

int read_REM_TimeSeriesModel( char *file_name )
{
  if ( REM_read_control_file(file_name) ) return(1);
  return(0);
}

int define_REM_TimeSeriesModel( int nflags, int *flags )
{
  char condition_string[LEN_STR], file_name[LEN_STR];
  int nconditions, nfiles, j;
  BOOL IncludeTimeData, UseT, UseS, Verbose;

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
  for ( j=0; j<REM_input.nfiles; j++)
    add_image_file( &image_files, REM_input.signal_file_name[j], 0, 0);

  // Get the number of conditions.
  REM_define_conditions( REM_input.condition_string, &nconditions );
  printf("conditions = %s\n",REM_input.condition_string);
  // Add to the list of activation files, based upon the existing conditions.
  for (j=0; j<nconditions; j++)
    { // overwrite the condition string
      REM_get_condition_string(j,condition_string);
      if ( REM_input.nifti_output )
	sprintf(file_name,"P-%s.nii",condition_string);
      else
	sprintf(file_name,"P-%s.bfloat",condition_string);
      add_image_file( &activation_files, file_name, 1, 1);
      if ( UseT )
	{
	  if ( REM_input.nifti_output )
	    sprintf(file_name,"T-%s.nii",condition_string);
	  else
	    sprintf(file_name,"T-%s.bfloat",condition_string);
	  add_image_file( &activation_files, file_name, 0, 2);
	}
      if ( UseS )
	{
	  if ( REM_input.nifti_output )
	    sprintf(file_name,"S-%s.nii",condition_string);
	  else
	    sprintf(file_name,"S-%s.bfloat",condition_string);
	  add_image_file( &activation_files, file_name, 0, 3);
	}
    }
  // Read the sigma2 file containing fixed effects and random effects variances.
  if ( REM_input.nifti_output )
    strcpy(sigma2_file.file_name,"sigma2.nii");
  else
    strcpy(sigma2_file.file_name,"sigma2.bfloat");
  sigma2_file.next = NULL;    // only 1 file
  sigma2_file.scale_log = 0;  // linear scale
  sigma2_file.type      = 0;  // not P, T, or S
  if ( read_SpaceTime_data(Verbose, 1, &sigma2_stack, &sigma2_file, &nfiles) )
    return(1);
  else
    {
      if ( sigma2_file.nt != 3 )
	{
	  printf("Error: file sigma2.bfloat had %d time points, but 3 were expected.\n",
		 sigma2_file.nt);
	  return(1);
	}
    }
  return(0);
}

void plot_REM_TimeSeriesModel_DataPoints( int nbins, float *BinXVector, float *BinYVector, POINT2D *Scale )
{
  POINT2D Point1, Point2;
  IPOINT2D iDraw1, iDraw2, iFill1, iFill2;
  float psize;
  int ntime, j;

  ntime = REM_get_number_time_points();
  // Define the point size.
  psize = 1.5 * (float)(graph.nwidth) / (float)(nbins);
  if ( psize > 4. ) psize = 4.;
  // Display the data.
  Point1.x = BinXVector[0];  Point1.y = BinYVector[0];
  iDraw1 = Space2D_to_GraphPixels(Point1, *Scale);
  iFill1.x = iDraw1.x - psize/2;  iFill1.y = iDraw1.y - psize/2;
  iFill2.x = iDraw1.x + psize/2;  iFill2.y = iDraw1.y + psize/2;
  // Draw the data on the graph.
  DPRIMITIVE_FillRectangle(Drawable_GraphData, COLOR_YELLOW, iFill1, iFill2);
  for (j=1; j<nbins; j++)
    {
      Point2.x = BinXVector[j];  Point2.y = BinYVector[j];
      iDraw2 = Space2D_to_GraphPixels(Point2, *Scale);
      iFill1.x = iDraw2.x - psize/2;  iFill1.y = iDraw2.y - psize/2;
      iFill2.x = iDraw2.x + psize/2;  iFill2.y = iDraw2.y + psize/2;
      if ( ntime != graph.npoints/2 )  // display all points
	DPRIMITIVE_FillRectangle(Drawable_GraphData, COLOR_YELLOW, iFill1, iFill2);
      else if (j%2 == 0 )              // remove odd points (error maps) for REM time
	DPRIMITIVE_FillRectangle(Drawable_GraphData, COLOR_YELLOW, iFill1, iFill2);
      iDraw1.x = iDraw2.x;  iDraw1.y = iDraw2.y;
    }
}

void plot_REM_TimeSeriesModel( POINT2D *Scale )
{
  POINT2D Point1, Point2;
  IPOINT2D iDraw1, iDraw2, limits;
  int j, jevent, nevents, ntime;

  ntime = REM_get_number_time_points();

  // Display shaded regions for event windows in the REM
  nevents = REM_get_number_events();
  for (jevent=0; jevent<nevents; jevent++)
    {
      Point1.y = graph.ymax;
      Point2.y = graph.ymin;
      limits = REM_GetEventLimits( REM_get_event_number(jevent) );
      if ( ntime == graph.npoints / 2 )
	{
	  Point1.x = 2*limits.x-1.;
	  Point2.x = 2*limits.y+2.;
	}
      else
	{
	  Point1.x = limits.x-0.5;
	  Point2.x = limits.y+0.5;
	}
      iDraw1 = Space2D_to_GraphPixels(Point1, *Scale);
      iDraw2 = Space2D_to_GraphPixels(Point2, *Scale);
      if ( jevent%2 )
	DPRIMITIVE_FillRectangle(Drawable_GraphData, COLOR_CYAN, iDraw1, iDraw2);
      else
	DPRIMITIVE_FillRectangle(Drawable_GraphData, COLOR_GRAY, iDraw1, iDraw2);
    }

  // Put error bars on the even points.
  if ( ntime == graph.npoints / 2 )
    {
      for (j=0; j<graph.npoints; j+=2)
	{
	  Point1.x = Point2.x = graph.xVector[j] / mosaic.stack->resolution_t;
	  Point1.y = graph.yVector[j] - graph.yVector[j+1];
	  Point2.y = graph.yVector[j] + graph.yVector[j+1];
	  iDraw1 = Space2D_to_GraphPixels(Point1, *Scale);
	  iDraw2 = Space2D_to_GraphPixels(Point2, *Scale);
	  DPRIMITIVE_DrawLine(Drawable_GraphData, COLOR_YELLOW, iDraw1, iDraw2);
	}
    }

  if ( REM_input.global_scaling && glm_model.baseline && ntime==graph.npoints/2 )
    {
      // Display the average fit to data excluding global scaling factors (odd points).
      Point1.x = (graph.xVector[1]-0.5) / mosaic.stack->resolution_t;  Point1.y = graph.yFit[1];
      iDraw1 = Space2D_to_GraphPixels(Point1, *Scale);
      for (j=3; j<graph.npoints; j+=2)
	{
	  Point2.x = (graph.xVector[j]-0.5) / mosaic.stack->resolution_t;
	  Point2.y = graph.yFit[j];
	  iDraw2 = Space2D_to_GraphPixels(Point2, *Scale);
	  DPRIMITIVE_DrawLine(Drawable_GraphData, COLOR_RED, iDraw1, iDraw2);
	  iDraw1.x = iDraw2.x;
	  iDraw1.y = iDraw2.y;
	}
    }

  /*
  float *fit;
  if ( REM_input.overlapping_events )
    { // more than 1 regressor accounts for magnitude of each point
      fit = float_vector(ntime);
      for (jevent=0; jevent<nevents; jevent++)
	{
	  if ( REM_event_fit(jevent, ntime, fit) )
	    return;
	  // Display the actual fit to data including global scaling factors (event points)
	  Point1.x = graph.xVector[0] / mosaic.stack->resolution_t;
	  Point1.y = fit[0];
	  iDraw1 = Space2D_to_GraphPixels(Point1, *Scale);
	  for (j=1; j<graph.npoints; j++)
	    {
	      if ( ntime == graph.npoints / 2 )
		Point2.x = graph.xVector[2*j] / mosaic.stack->resolution_t;
	      else
		Point2.x = graph.xVector[j] / mosaic.stack->resolution_t;
	      Point2.y = fit[j];
	      iDraw2 = Space2D_to_GraphPixels(Point2, *Scale);
	      DPRIMITIVE_DrawLine(Drawable_GraphData, COLOR_RED, iDraw1, iDraw2);
	      iDraw1.x = iDraw2.x;
	      iDraw1.y = iDraw2.y;
	    }
	}
      free_float_vector(fit);
    }
  */

  // Display the actual fit to data including global scaling factors (event points)
  Point1.x = graph.xVector[0] / mosaic.stack->resolution_t;  Point1.y = graph.yFit[0];
  iDraw1 = Space2D_to_GraphPixels(Point1, *Scale);
  if ( ntime == graph.npoints / 2 )
    {
      for (j=2; j<graph.npoints; j+=2)
	{
	  Point2.x = graph.xVector[j] / mosaic.stack->resolution_t;
	  Point2.y = graph.yFit[j];
	  iDraw2 = Space2D_to_GraphPixels(Point2, *Scale);
	  DPRIMITIVE_DrawLine(Drawable_GraphData, COLOR_WHITE, iDraw1, iDraw2);
	  iDraw1.x = iDraw2.x;
	  iDraw1.y = iDraw2.y;
	}
    }
  else
    {
      for (j=1; j<ntime; j++)
	{
	  Point2.x = graph.xVector[j] / mosaic.stack->resolution_t;
	  Point2.y = graph.yFit[j];
	  iDraw2 = Space2D_to_GraphPixels(Point2, *Scale);
	  DPRIMITIVE_DrawLine(Drawable_GraphData, COLOR_WHITE, iDraw1, iDraw2);
	  iDraw1.x = iDraw2.x;
	  iDraw1.y = iDraw2.y;
	}
    }
}

void fill_REM_TimeSeriesModel( IPOINT3D voxel )
{
  int ntime;JIP_INDEX index4d;
  int jtime;JIP_INDEX jset;
  float Sigma2_RE, value, weight_sum;
  float *Signal_vector, *SigFE_vector, *fit_vector, *fit_vector_av;

  if ( ovl_root.npts != 0 )
    { // Average Sigma2_RE across the overlay.
      for (jset=0, Sigma2_RE=0., weight_sum=0.;
	   jset<ovl_root.npts; jset++)
	{
	  index4d = 2 * mosaic.stack->n3d        // 3rd volume in file sigma2
	    +       ovl_root.voxel[jset].z * mosaic.stack->n2d
	    +       ((JIP_INDEX)(ovl_root.voxel[jset].y)) * mosaic.stack->dim.x
	    +       ovl_root.voxel[jset].x;
	  value = sigma2_stack.SpaceTime.fData[index4d];
	  Sigma2_RE  += ovl_root.weight[jset] * value;
	  weight_sum += ovl_root.weight[jset];
	}
      Sigma2_RE /= weight_sum;
    }
  else
    { // take value of Sigma2_RE at a single voxel
      index4d = 2 * mosaic.stack->n3d        // 3rd volume in file sigma2
	+       voxel.z * mosaic.stack->n2d
	+       ((JIP_INDEX)(voxel.y)) * mosaic.stack->dim.x
	+       voxel.x;
      Sigma2_RE = sigma2_stack.SpaceTime.fData[index4d];
    }

  // The number of time points is halved (value, error, value, error, ...)
  ntime = REM_get_number_time_points();
  Signal_vector = float_vector(ntime);
  SigFE_vector  = float_vector(ntime);
  fit_vector    = float_vector(ntime);
  fit_vector_av = float_vector(ntime);

  for (jtime=0; jtime<ntime; jtime++)
    {
      if ( ntime == graph.npoints / 2 )
	{
	  Signal_vector[jtime] = graph.yVector[2*jtime];
	  SigFE_vector[jtime]  = graph.yVector[2*jtime+1];
	}
      else
	{
	  Signal_vector[jtime] = graph.yVector[jtime];
	  SigFE_vector[jtime]  = 0.;
	}
    }

  // Create a fit vector for the value at this voxel or overlay.
  REM_fit_final(Sigma2_RE, ntime, Signal_vector, SigFE_vector, fit_vector, fit_vector_av);

  if ( ntime == graph.npoints / 2 )
    {
      // The time series is (value,error,value,error,...)
      // Put the fit with global scaling into even points, and the average fit value into odd points.
      for (jtime=0; jtime<ntime; jtime++)
	{
	  graph.yFit[2*jtime]   = fit_vector[jtime];
	  graph.yFit[2*jtime+1] = fit_vector_av[jtime];
	}
    }
  else
    {
      for (jtime=0; jtime<ntime; jtime++)
	graph.yFit[jtime] = fit_vector[jtime];
    }

  // remove baseline if requested
  if ( !glm_model.baseline && ntime == graph.npoints/2 )
    {
      for (jtime=0; jtime<ntime; jtime++)
	graph.yVector[2*jtime] -= ( graph.yFit[2*jtime] - graph.yFit[2*jtime+1] );
    }

  free_float_vector(Signal_vector);
  free_float_vector(SigFE_vector);
  free_float_vector(fit_vector);
  free_float_vector(fit_vector_av);
}

void get_info_REM_TimeSeriesModel( BOOL move_cursor, char *string1, char *string2, char *string3 )
{
  char event_char, map[LEN_STR];
  float graph_value;
  int ix, isample, nfound, ntime;

  if ( move_cursor )
    ix = graph.icursor.x;
  else
    ix = graph.iTime;
  sprintf(string1," event instance graph map");
  if ( ix >= 0 )
    {
      ntime = REM_get_number_time_points();
      if ( ntime == graph.npoints / 2 )
	nfound = REM_IdentifyEvent(ix/2,&event_char,&isample);
      else
	nfound = REM_IdentifyEvent(ix,&event_char,&isample);
      graph_value = graph.yVector[ix];
    }
  if ( move_cursor )
    graph_value = graph.cursor.y;
  if ( ix%2 )
    strcpy(map,"error");
  else
    strcpy(map,"value");

  if ( move_cursor || ix >= 0 )
    {
      sprintf(string2,"   %c   %3d %9.2f  %s",event_char,isample,graph_value,map);
      if ( nfound > 1 )
	sprintf(string3," %d events found here",nfound);
      else
	sprintf(string3," ");
    }
  else if ( ix < 0 )
    {
      sprintf(string2," ");
      sprintf(string3," ");
    }

}

void update_SelectText_REM_TimeSeriesModel( struct SelectableText *SelectText )
{

  if ( SelectText->FieldID == TEXTFIELD_GLM_BASELINE )
    {
      SelectText->color = COLOR_CYAN;
      SelectText->visible = ( graph.type == 3 && REM_input.global_scaling );
      SelectText->XPosition    = 450;
      SelectText->XSelectStart = SelectText->XPosition;
      SelectText->XSelectEnd   = SelectText->XSelectStart + 40;
      strcpy(SelectText->string," b ");
      sprintf(SelectText->message1,"Remove/reinstate GLM baseline");
      sprintf(SelectText->message2,"Keyboard equivalent: b");
      sprintf(SelectText->message3," ");
    }      
}

void action_graph_text_selection_REM_TimeSeriesModel( int button )
{
}

void action_graph_button23_REM_TimeSeriesModel()
{
}

int graph_event_handler_REM_TimeSeriesModel( char key )
{
  return(0);
}

void label_graph_time_window_REM_TimeSeriesModel(char *FileName )
{
}
