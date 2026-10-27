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
#include "fftw3.h"
#include "util.h"
#include "align.h"
static char CNULL = '\0';


void fill_TimeSeriesModel( IPOINT3D voxel )
{
}

void plot_TimeSeriesModel( POINT2D *Scale )
{
}

void increment_graph_location_time()
{
}

void decrement_graph_location_time()
{
}

void rescale_image_settings()
{
  if ( AlignAid.DisplayType == 0 )
    {
      mimic_scale_min = image_scale_min;
      mimic_scale_max = image_scale_max;
    }
  else if ( AlignAid.DisplayType == 1 )
    {
      target_scale_min = image_scale_min;
      target_scale_max = image_scale_max;
    }
  else
    {
      distort_scale_min = image_scale_min;
      distort_scale_max = image_scale_max;
    }
}

#include "../engine/shared/initialize_alignment.inc"

void modify_alignment_parameter_names()
{
  int axes_in[3][3], axes_out[3][3];
  int j1, j2, j12;

  // Define the identity matrix for x,y,z axes.
  for (j1=0; j1<3; j1++)
    {
      for (j2=0; j2<3; j2++)
	{
	  axes_in[j1][j2] = 0;
	  if ( j1 == j2 )
	    axes_in[j1][j2] = 1;
	}
    }
  // Rotate the axes by the orientation matrix.
  for (j1=0; j1<3; j1++)
    {
      for (j2=0; j2<3; j2++)
	{
	  axes_out[j1][j2] = 0;
	  for (j12=0; j12<3; j12++)
	    axes_out[j1][j2] += Alignment.Orient[j1][j12] * axes_in[j12][j2];
	}
    }
  // Define names: x position
  if ( axes_out[0][0] == 1 || axes_out[0][0] == -1 )
    {
      sprintf(AlignAid.name[0],"  x");
      sprintf(AlignAid.name[6],"size-x");
      sprintf(AlignAid.name[4],"phi_yz");  // rotation about x
      sprintf(AlignAid.name[11],"skew_yz");
    }
  else if ( axes_out[0][1] == 1 || axes_out[0][1] == -1 )
    {
      sprintf(AlignAid.name[0],"  y");
      sprintf(AlignAid.name[6],"size-y");
      sprintf(AlignAid.name[4],"phi_xz");  // rotation about y
      sprintf(AlignAid.name[11],"skew_xz");
    }
  else if ( axes_out[0][2] == 1 || axes_out[0][2] == -1 )
    {
      sprintf(AlignAid.name[0],"  z");
      sprintf(AlignAid.name[6],"size-z");
      sprintf(AlignAid.name[4],"phi_xy");  // rotation about z
      sprintf(AlignAid.name[11],"skew_xy");
    }

  // Define names: y position
  if ( axes_out[1][0] == 1 || axes_out[1][0] == -1 )
    {
      sprintf(AlignAid.name[1],"  x");
      sprintf(AlignAid.name[7],"size-x");
      sprintf(AlignAid.name[5],"phi_yz");  // rotation about x
      sprintf(AlignAid.name[10],"skew_yz");
    }
  else if ( axes_out[1][1] == 1 || axes_out[1][1] == -1 )
    {
      sprintf(AlignAid.name[1],"  y");
      sprintf(AlignAid.name[7],"size-y");
      sprintf(AlignAid.name[5],"phi_xz");  // rotation about y
      sprintf(AlignAid.name[10],"skew_xz");
    }
  else if ( axes_out[1][2] == 1 || axes_out[1][2] == -1 )
    {
      sprintf(AlignAid.name[1],"  z");
      sprintf(AlignAid.name[7],"size-z");
      sprintf(AlignAid.name[5],"phi_xy");  // rotation about z
      sprintf(AlignAid.name[10],"skew_xy");
    }

  // Define names: z position
  if ( axes_out[2][0] == 1 || axes_out[2][0] == -1 )
    {
      sprintf(AlignAid.name[2],"  x");
      sprintf(AlignAid.name[8],"size-x");
      sprintf(AlignAid.name[3],"phi_yz");  // rotation about x
      sprintf(AlignAid.name[9],"skew_yz");
    }
  else if ( axes_out[2][1] == 1 || axes_out[2][1] == -1 )
    {
      sprintf(AlignAid.name[2],"  y");
      sprintf(AlignAid.name[8],"size-y");
      sprintf(AlignAid.name[3],"phi_xz");  // rotation about y
      sprintf(AlignAid.name[9],"skew_xz");
    }
  else if ( axes_out[2][2] == 1 || axes_out[2][2] == -1 )
    {
      sprintf(AlignAid.name[2],"  z");
      sprintf(AlignAid.name[8],"size-z");
      sprintf(AlignAid.name[3],"phi_xy");  // rotation about z
      sprintf(AlignAid.name[9],"skew_xy");
    }
}

#include "../engine/shared/SetGridDown.inc"

void point_mosaic_to_stack_data()
{
  // set the data pointer
  if ( AlignAid.DisplayType == 0 )      // mimic
    {
      mosaic.stack    = &mimic;
      image_scale_min = mimic_scale_min;
      image_scale_max = mimic_scale_max;
    }
  else if ( AlignAid.DisplayType == 1 ) // target
    {
      mosaic.stack    = &target;
      image_scale_min = target_scale_min;
      image_scale_max = target_scale_max;
    }
  else
    {
      if ( AlignAid.DisplayType == 2 )
	mosaic.stack    = &distort_x;
      else if ( AlignAid.DisplayType == 3 )
	mosaic.stack    = &distort_y;
      else if ( AlignAid.DisplayType == 4 )
	mosaic.stack    = &distort_z;
      else if ( AlignAid.DisplayType == 5 )
	mosaic.stack    = &Jac_deter;
      else if ( AlignAid.DisplayType == 6 )
	{
	  mosaic.stack  = &NL_bases;
	  if ( AlignNL.ViewBasis )
	    create_basis_function_image();
	}
      image_scale_min = distort_scale_min;
      image_scale_max = distort_scale_max;
    }
  mosaic.fSpaceMatrix = mosaic.stack->SpaceTime.fData;
}

#include "../engine/shared/update_alignment.inc"

void rescale_activation()
{
  float threshold_low, threshold_high, temp;
  JIP_INDEX j;

  threshold_high = -1.e20;
  threshold_low = 1.e20;
  for (j=0; j<mosaic.stack->n3d; j++)
    {
      if ( mosaic.activation.fSpaceMatrix[j] < threshold_low )
	threshold_low = mosaic.activation.fSpaceMatrix[j];
      if ( mosaic.activation.fSpaceMatrix[j] > threshold_high )
	threshold_high = mosaic.activation.fSpaceMatrix[j];
    }
  // Make sure the the thresholds make sense
  if ( fabs((double)threshold_low) < fabs((double)threshold_high) )
    {
      threshold_low  = fabs((double)threshold_low);
      threshold_high = fabs((double)threshold_high);
    }
  else
    {
      temp = fabs((double)threshold_low);
      threshold_low  = fabs((double)threshold_high);
      threshold_high = temp;
    }

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

#include "../engine/shared/update_display_set.inc"

#include "../engine/shared/smooth_or_sharpen_isotropic.inc"

#include "../engine/shared/ft_1d.inc"

#include "../engine/shared/index4d_stack.inc"

int set_adjustment_flags(char *string)
// This function interpets a string of this form: "111111000" as adjustment flags
// for 3 translations, 3 rotations, 3 inflations, and 3 skews.
// The string starts with translations, & missing flags off the end are set to zero.
{
  int lLength, j;
  char *ptr, flag;
  BOOL scanning;

  // Determine the length of the argument.
  lLength = jip_int_count((JIP_INDEX)strlen(string));
  if ( lLength > NAFFINE )
    {
      printf("Error: the length of the adjustment string (%d) is > %d\n",lLength,NAFFINE);
      return(1);
    }

  // Interpret the string.
  ptr = string;
  scanning = TRUE;
  for (j=0; j<NAFFINE; j++)
    {
      if ( scanning )
	{
	  sscanf(ptr,"%c",&flag);
	  if ( flag == '0' )
	    AlignAid.Adjust[j] = 0;
	  else
	    AlignAid.Adjust[j] = 1;
	  ptr++;
	  scanning = (*ptr != CNULL);
	}
      else
	AlignAid.Adjust[j] = 0;
    }
  return(0);
}
