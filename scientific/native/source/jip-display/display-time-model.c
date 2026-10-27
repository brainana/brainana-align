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
#include "display-prototypes.h"
#include "glm-prototypes.h"
#include "rem-prototypes.h"
#include "srtm-prototypes.h"

static int which_time_model = 0;


void initialize_TimeSeriesModel( int which_model )
{
  graph.time_model = which_model;
  if ( which_model == 1 )      // GLM
    initialize_GLM_TimeSeriesModel();
  else if ( which_model == 2 ) // REM
    initialize_REM_TimeSeriesModel();
  else if ( which_model == 3 ) // SRTM
    initialize_SRTM_TimeSeriesModel();
  else
    {
      printf("Error: only the GLM, REM, & SRTM-GLM models are defined.\n");
      exit(1);
    }
  which_time_model = which_model;
}

int read_TimeSeriesModel( char *file_name )
{
  if ( which_time_model == 1 )
    {
      if ( read_GLM_TimeSeriesModel(file_name) ) return(1);
    }
  else if ( which_time_model == 2 )
    {
      if ( read_REM_TimeSeriesModel(file_name) ) return(1);
    }
  else if ( which_time_model == 3 )
    {
      if ( read_SRTM_TimeSeriesModel(file_name) ) return(1);
    }
  else
    {
      printf("Error: only the GLM, REM, & SRTM-GLM models are defined.\n");
      exit(1);
    }
  return(0);
}

int TimeSeriesModel_is_initialized()
{
  if ( which_time_model == 1 )
    return GLM_is_initialized();
  else if ( which_time_model == 2 )
    return REM_is_initialized();
  else if ( which_time_model == 3 )
    return SRTM_is_initialized();
  else
    return(0);
}

int define_TimeSeriesModel( int nflags, int *flags )
{
  if ( which_time_model == 1 )
    {
      if ( define_GLM_TimeSeriesModel(nflags, flags) ) return(1);
    }
  else if ( which_time_model == 2 )
    {
      if ( define_REM_TimeSeriesModel(nflags, flags) ) return(1);
    }
  else if ( which_time_model == 3 )
    {
      if ( define_SRTM_TimeSeriesModel(nflags, flags) ) return(1);
    }
  else
    {
      printf("Error: only the GLM, REM, & SRTM-GLM models are defined.\n");
      exit(1);
    }
  return(0);
}

void plot_TimeSeriesModel_DataPoints( int nbins, float *BinXVector, float *BinYVector, POINT2D *Scale )
{
  if ( which_time_model != 2 ) // not REM; plot exactly like calling function
    plot_DataPoints( nbins, BinXVector, BinYVector, Scale );
  else
    plot_REM_TimeSeriesModel_DataPoints( nbins, BinXVector, BinYVector, Scale );
}

void plot_TimeSeriesModel( POINT2D *Scale )
{
  if ( which_time_model == 1 )
    {
      plot_GLM_TimeSeriesModel(Scale);
    }
  else if ( which_time_model == 2 )
    {
      plot_REM_TimeSeriesModel(Scale);
    }
  else if ( which_time_model == 3 )
    {
      plot_SRTM_TimeSeriesModel(Scale);
    }
  else
    {
      printf("Error: only the GLM, REM, & SRTM-GLM models are defined.\n");
      exit(1);
    }
}

void fill_TimeSeriesModel( IPOINT3D voxel )
{
  // Fill the fitting function.
  if ( TimeSeriesModel_is_initialized() ) // time
    {
      if ( which_time_model == 1 )
	{
	  fill_GLM_TimeSeriesModel();
	}
      else if ( which_time_model == 2 )
	{
	  fill_REM_TimeSeriesModel( voxel );
	}
      else if ( which_time_model == 3 )
	{
	  fill_SRTM_TimeSeriesModel();
	}
      else
	{
	  printf("Error: only the GLM, REM, & SRTM-GLM models are defined.\n");
	  exit(1);
	}
    }
}

void get_info_TimeSeriesModel( BOOL move_cursor, char *string1, char *string2, char *string3 )
{

  if ( graph.type != 3 || graph.time_model == 0 )
    {
      printf("Programming error in function display_info_TimeSeriesModel.\n");
      exit(1);
    }
  if ( which_time_model == 1 )      // GLM
    get_info_GLM_TimeSeriesModel( move_cursor, string1, string2, string3 );
  else if ( which_time_model == 2 ) // REM
    get_info_REM_TimeSeriesModel( move_cursor, string1, string2, string3 );
  else if ( which_time_model == 3 ) // SRTM
    get_info_SRTM_TimeSeriesModel( move_cursor, string1, string2, string3 );
  else
    {
      printf("Error: only the GLM, REM, & SRTM-GLM models are defined.\n");
      exit(1);
    }

}

void update_SelectText_TimeSeriesModel( struct SelectableText *SelectText )
{
  if ( which_time_model == 1 )      // GLM
    update_SelectText_GLM_TimeSeriesModel( SelectText );
  else if ( which_time_model == 2 ) // REM
    update_SelectText_REM_TimeSeriesModel( SelectText );
  else if ( which_time_model == 3 ) // SRTM
    update_SelectText_SRTM_TimeSeriesModel( SelectText );
  else if ( which_time_model != 0 )
    {
      printf("Error: only the GLM, REM, & SRTM-GLM models are defined.\n");
      exit(1);
    }
}

void action_graph_text_selection_TimeSeriesModel( int button )
{
  if ( which_time_model == 1 )      // GLM
    action_graph_text_selection_GLM_TimeSeriesModel( button );
  else if ( which_time_model == 2 ) // REM
    action_graph_text_selection_REM_TimeSeriesModel( button );
  else if ( which_time_model == 3 ) // SRTM
    action_graph_text_selection_SRTM_TimeSeriesModel( button );
  else if ( which_time_model != 0 )
    {
      printf("Error: only the GLM, REM, & SRTM-GLM models are defined.\n");
      exit(1);
    }
}

void action_graph_button23_TimeSeriesModel()
{
  if ( which_time_model == 1 )      // GLM
    action_graph_button23_GLM_TimeSeriesModel();
  else if ( which_time_model == 2 ) // REM
    action_graph_button23_REM_TimeSeriesModel();
  else if ( which_time_model == 3 ) // SRTM
    action_graph_button23_SRTM_TimeSeriesModel();
  else if ( which_time_model != 0 )
    {
      printf("Error: only the GLM, REM, & SRTM-GLM models are defined.\n");
      exit(1);
    }
}

int graph_event_handler_TimeSeriesModel( char key )
{
  int command_found=0;

  if ( which_time_model == 1 )      // GLM
    command_found = graph_event_handler_GLM_TimeSeriesModel(key);
  else if ( which_time_model == 2 ) // REM
    command_found = graph_event_handler_REM_TimeSeriesModel(key);
  else if ( which_time_model == 3 ) // SRTM
    command_found = graph_event_handler_SRTM_TimeSeriesModel(key);
  else if ( which_time_model != 0 )
    {
      printf("Error: only the GLM, REM, & SRTM-GLM models are defined.\n");
      exit(1);
    }
  return(command_found);
}

void label_graph_time_window_TimeSeriesModel( char *FileName )
{
  if ( which_time_model == 1 )      // GLM
    label_graph_time_window_GLM_TimeSeriesModel(FileName);
  else if ( which_time_model == 2 ) // REM
    label_graph_time_window_REM_TimeSeriesModel(FileName);
  else if ( which_time_model == 3 ) // SRTM
    label_graph_time_window_SRTM_TimeSeriesModel(FileName);
  else if ( which_time_model != 0 )
    {
      printf("Error: only the GLM, REM, & SRTM-GLM models are defined.\n");
      exit(1);
    }
}
