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
#include "lmmin.h"
#include "lm_eval.h"


void rescale_image_settings()
{
  // do nothing here
}

void rescale_activation()
{ // this function only called at beginning, or when a p-value map is created interactively
  struct file_list *this_file;
  float thresh_log_low, thresh_log_high, thresh_lin_low, thresh_lin_high, bonferoni, temp, value;
  BOOL scale_log;
  int imap, jt;JIP_INDEX index, j3d;

  thresh_log_high = thresh_lin_high = -1.e20;
  thresh_log_low  = thresh_lin_low  =  1.e20;

  scale_log = FALSE;
  for ( this_file = &activation_files, imap=0; this_file; this_file = this_file->next )
    {
      scale_log = this_file->scale_log;
      for (jt=0; jt<this_file->nt; jt++, imap++)
	{
	  for (j3d=0; j3d<mosaic.stack->npts_volume; j3d++)
	    {
	      index = imap * mosaic.stack->npts_volume + j3d;
	      value = mosaic.activation.stack->SpaceTime.fData[index];
	      if ( scale_log )
		{
		  if ( value < thresh_log_low )
		    thresh_log_low = value;
		  if ( value > thresh_log_high )
		    thresh_log_high = value;
		}
	      else
		{
		  if ( value < thresh_lin_low )
		    thresh_lin_low = value;
		  if ( value > thresh_lin_high )
		    thresh_lin_high = value;
		}
	    }
	}
    }

  // Make sure the the thresholds make sense
  if ( fabs((double)thresh_log_low) < fabs((double)thresh_log_high) )
    {
      thresh_log_low  = fabs((double)thresh_log_low);
      thresh_log_high = fabs((double)thresh_log_high);
    }
  else
    {
      temp = fabs((double)thresh_log_low);
      thresh_log_low  = fabs((double)thresh_log_high);
      thresh_log_high = temp;
    }
  if ( fabs((double)thresh_lin_low) < fabs((double)thresh_lin_high) )
    {
      thresh_lin_low  = fabs((double)thresh_lin_low);
      thresh_lin_high = fabs((double)thresh_lin_high);
    }
  else
    {
      temp = fabs((double)thresh_lin_low);
      thresh_lin_low  = fabs((double)thresh_lin_high);
      thresh_lin_high = temp;
    }

  // Set separate thresholds for linear and log scales.
  if ( mosaic.activation.thresh_lin_high == 0. )
    {
      mosaic.activation.thresh_lin_high = (int)thresh_lin_high;
      mosaic.activation.thresh_lin_low = 0.;
      if ( mosaic.activation.thresh_lin_high <= mosaic.activation.thresh_lin_low )
	mosaic.activation.thresh_lin_high = mosaic.activation.thresh_lin_low + 1.;
    }
  mosaic.activation.thresh_log_high = thresh_log_high;
  bonferoni = mosaic.stack->n3d;
  if ( GLM_input.multiple_comparisons == 0 )
    mosaic.activation.thresh_log_low = - log10( (double)(.01/bonferoni) );
  else
    mosaic.activation.thresh_log_low = 2.;
  mosaic.activation.thresh_log_low  = (int)mosaic.activation.thresh_log_low;
  mosaic.activation.thresh_log_high = (int)mosaic.activation.thresh_log_high;
  mosaic.activation.thresh_log_high = MAX(1,mosaic.activation.thresh_log_high);
  if ( mosaic.activation.thresh_log_low >= mosaic.activation.thresh_log_high )
    mosaic.activation.thresh_log_low = MAX(0,mosaic.activation.thresh_log_high-1.);
}

void convert_activation_scale( struct file_list *this_file )
{
  float conversion=0.4343; // log10 of e
  int jmap;JIP_INDEX offset_map, j;

  for (jmap=this_file->start_image; jmap<=this_file->end_image; jmap++)
    {
      offset_map = jmap * mosaic.activation.stack->n3d;
      for (j=0; j<mosaic.activation.stack->n3d; j++)
	mosaic.activation.stack->SpaceTime.fData[j+offset_map] *= conversion;
    }
}

void split_graphX_into_file_and_location(int xvalue, int *ifile, int *itime)
{
  struct file_list *this_file;

  *ifile = 0;
  for (this_file=&image_files; this_file; this_file=this_file->next)
    {
      (*ifile)++;
      if ( xvalue <= this_file->end_image )
	break;
    }
  *itime = xvalue - this_file->start_image;
}

void increment_graph_location_time()
{
  int itime;

  graph.iLastTime = graph.iTime; // save value

  // Extend the graph range down from zero to -anatomy_stack.nt
  (graph.iTime)++;
  if ( graph.iTime == mosaic.stack->nt ) graph.iTime = - anatomy_stack.nt;
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
}

void decrement_graph_location_time()
{
  int itime;

  graph.iLastTime = graph.iTime; // save value

  // Extend the graph range down from zero to -anatomy_stack.nt
  (graph.iTime)--;
  if ( graph.iTime < -anatomy_stack.nt ) graph.iTime = mosaic.stack->nt - 1;
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
}

int fit_gamma(int npoints,
	      double *xarray,
	      double *yarray,
	      float *amplitude,// amplitude of Gamma
	      float *rate)     // rate for Gamma (=1/tau)
{

  double param[2];
  lm_control_type control;
  lm_data_type data;

  lm_initialize_control(&control);

  data.user_func = func_gamma;
  data.user_x    = xarray;
  data.user_y    = yarray;
  
  param[0] = *amplitude;
  param[1] = *rate;

  // perform the fit:
  lm_minimize(npoints, 2, param, lm_evaluate_default, lm_print_default,
	      &data, &control);
  // print results:
  printf("status: %s after %d evaluations\n",
	 lm_shortmsg[control.info], control.nfev);

  *amplitude = param[0];
  *rate      = param[1];

  return(0);
}

double func_gamma(double x, double *p)
{
  double amp, rate, ex, val;
  JIP_INDEX npoints;

  amp  = p[0];
  rate = p[1];
  npoints = graph.ix_high_select - graph.ix_low_select;
  ex = exp( (double)((npoints-x)*rate));
  // The function vanishes at x = 0 and x = npoints
  val = amp * x * ( ex - 1. );
  return(val);
}
