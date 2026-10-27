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

#define NBINS 50  // 1% per bin


void action_auto_adjust_affine( int button )
{
  static BOOL first_time=TRUE;
  BOOL AdjustSave[NAFFINE], AdjustSet, MultiResolution;
  int iCost=0;
  int j;

  if ( AlignAid.DisplaySet != 1 )
    {
      AlignAid.DisplaySet = 1;
      update_display_set();
      show_alignment_info();
    }
  MultiResolution = (button == 1);

  // Check for the rolloff filter.
  if (AlignAid.smooth_fov_fraction == 0 )
    printf("\nThe rolloff filter is not applied. Is this a mistake??\n");

  // The first time, adjust affine parameters in a hierarchical way.
  if ( first_time )
    {
      // Apply the overlay filter if an overlay is active.
      if ( mosaic.toggle_overlays && ovl_root.npts != 0 )
	{
	  printf("\nApply the overlay filter to remove extra-cerebral tissue.\n");
	  action_filter_overlay();
	}
      else
	{
	  printf("An overlay is not active.\n");
	  printf("This is OK if the template has already been skull-stripped.\n");
	  printf("Otherwise, use a brain overlay before this step.\n");
	}
      // Save the set of parameter selections.
      for (j=0; j<NAFFINE; j++)
	AdjustSave[j] = AlignAid.Adjust[j];

      // Start from scratch, using 3 parameters (shifts)
      AlignAid.iParameter = 0;
      for (j=0; j<NAFFINE; j++)
	{
	  AlignAid.Adjust[j] = 0;
	  if ( j < 3 && AdjustSave[j] )
	    AlignAid.Adjust[j] = 1;
	}
      for (j=0, AdjustSet=FALSE; j<NAFFINE; j++)
	AdjustSet |= AlignAid.Adjust[j];
      if ( AdjustSet )
	{
	  AlignAid.DisplaySet = 1;
	  if ( MultiResolution &&
	       AlignAid.SubSampleSetting.x *
	       AlignAid.SubSampleSetting.y *
	       AlignAid.SubSampleSetting.z != 1 )
	    action_change_resolution();
	  show_alignment_info();
	  auto_adjust_affine_iterations(iCost,3,3);
	}

      // Use 6 parameters (rigid body)
      AlignAid.iParameter = 0;
      show_alignment_info();
      for (j=0; j<NAFFINE; j++)
	{
	  AlignAid.Adjust[j] = 0;
	  if ( j < 6 && AdjustSave[j] )
	    AlignAid.Adjust[j] = 1;
	}
      for (j=0, AdjustSet=FALSE; j<NAFFINE; j++)
	AdjustSet |= AlignAid.Adjust[j];
      if ( AdjustSet && MultiResolution &&
	   AlignAid.SubSample.x*AlignAid.SubSample.y*AlignAid.SubSample.z != 1 )
	{ // Multi-resolution
	  auto_adjust_affine_iterations(iCost,3,2);
	  AlignAid.SubSampleSetting.x = MAX(1,AlignAid.SubSample.x/2);
	  AlignAid.SubSampleSetting.y = MAX(1,AlignAid.SubSample.y/2);
	  AlignAid.SubSampleSetting.z = MAX(1,AlignAid.SubSample.z/2);
	  action_change_resolution();  // improve resolution
	  AlignAid.iParameter = 0;
	  show_alignment_info();
	  auto_adjust_affine_iterations(iCost,2,2);
	}
      else if ( AdjustSet )
	{ // not multi-resolution, so detailed alignment of rigid-body
	  auto_adjust_affine_iterations(iCost,3,1); // detailed alignment
	}

      // Use 9 parameters
      AlignAid.iParameter = 0;
      show_alignment_info();
      for (j=0; j<NAFFINE; j++)
	{
	  AlignAid.Adjust[j] = 0;
	  if ( j < 9 && AdjustSave[j] )
	    AlignAid.Adjust[j] = 1;
	}
      for (j=0, AdjustSet=FALSE; j<NAFFINE; j++)
	AdjustSet |= AlignAid.Adjust[j];
      if ( AdjustSet )
	{
	  show_alignment_info();
	  auto_adjust_affine_iterations(iCost,2,2);
	}
      // Use all selected parameters.
      for (j=0; j<NAFFINE; j++)
	AlignAid.Adjust[j] = AdjustSave[j];
      first_time = FALSE;
      action_auto_adjust_affine(button); // recall this function
    }
  else
    {
      // Find the 1st parameter adjustment.
      AlignAid.iParameter = 0;
      while ( !AlignAid.Adjust[AlignAid.iParameter] )
	AlignAid.iParameter++;

      if ( !MultiResolution )
	{
	  show_alignment_info();
	  auto_adjust_affine_iterations(iCost,1,1);
	}
      else
	{
	  action_change_resolution();
	  show_alignment_info();
	  auto_adjust_affine_iterations(iCost,2,2);
	  if ( AlignAid.SubSample.x*AlignAid.SubSample.y*AlignAid.SubSample.z != 1 )
	    {
	      AlignAid.SubSampleSetting.x = MAX(1,AlignAid.SubSample.x/2);
	      AlignAid.SubSampleSetting.y = MAX(1,AlignAid.SubSample.y/2);
	      AlignAid.SubSampleSetting.z = MAX(1,AlignAid.SubSample.z/2);
	      action_change_resolution();
	    }
	  show_alignment_info();
	  auto_adjust_affine_iterations(iCost,1,1);
	}
      first_time = FALSE;
      print_parameter_set(2,0);
      // Final transformation: use most accurate interpolation
      update_alignment(1);
      show_on_info_line(0,0,"done","","");
      //      build_image_display(1);
      fprintf(stdout,"done\n");
      fflush(stdout);
    }
}

void auto_adjust_affine_iterations( int icost, int ilevel_max, int ilevel_min )
     // icost: 0 = mutual informtation, 1 = normalized correlation
{
  char string1[LEN_STR], string2[LEN_STR];
  int icount, ilevel, jpar;
  float saved_value[NAFFINE], cost_initial, cost_final, cost_last_step, tolerance;
  BOOL keep_adjusting, repeat, converged;
  static BOOL first_time = TRUE;

  if ( AlignAid.DisplayType != 0 ) // switch to individual
    action_change_display_type(0);
  if ( AlignAid.DisplaySet != 1 )
    {
      AlignAid.DisplaySet = 1;
      show_alignment_info();
    }

  cost_initial = cost_function(icost);  // update cost function here for output via print_parameter_set
  for (ilevel=ilevel_max; ilevel>=ilevel_min; ilevel--)
    {
      print_parameter_set(1,ilevel);
      if ( ilevel == 1 )
	{
	  if ( ilevel_max == 1 )
	    tolerance = 0.02;  // this is the last adjustment: fine scale
	  else
	    tolerance = 0.04;
	}
      else
	tolerance = 0.05;

      converged  = FALSE;
      while ( ! converged )
	{
	  action_set_increments(ilevel);
	  sprintf(string1,"level %d",ilevel);
	  if ( icost == 0 )
	    sprintf(string2,"mutual information");
	  else
	    sprintf(string2,"normalized correlation");
	  show_on_info_line(0,0,string1,string2,"");
	  icount = 0;
	  // Save the starting parameter values for this iteration.
	  cost_last_step = cost_initial = cost_function(icost);
	  for (jpar=0; jpar<NAFFINE; jpar++)
	    saved_value[jpar] = Alignment.value[jpar];
	  // Find the best set of parameters at this resolution level.
	  repeat = TRUE;
	  while ( repeat && icount < 20 )
	    {
	      keep_adjusting = TRUE;
	      while ( keep_adjusting )
		{
		  keep_adjusting = adjust_affine_1pass(icost,ilevel) && icount < 20;
		  cost_final = cost_function(icost);
		  keep_adjusting = keep_adjusting && (cost_final/cost_initial > 1.005);
		  cost_initial = cost_final;
		  icount++;
		}
	      cost_final = cost_function(icost);
	      repeat =(cost_final/cost_initial > 1.01);
	      cost_initial = cost_final;
	      action_set_increments(ilevel);
	      // Only accept the changes if the cost function is increased.
	      if ( cost_final < cost_last_step )
		{
		  for (jpar=0; jpar<NAFFINE; jpar++)
		    Alignment.value[jpar] = saved_value[jpar];
		}
	    }
	  converged  = (cost_final-cost_last_step)/cost_last_step < tolerance;
	  print_parameter_set(0,ilevel);
	}
    }

  first_time = FALSE;
}

BOOL adjust_affine_1pass( int icost, int ilevel )
     // icost:  0 = diff, 1 = mutual informtation
     // ilevel: specifies starting increment values (see action_set_increments)
{
  float cost_array[NAFFINE], increment[NAFFINE], max_increment[NAFFINE];
  float cost_initial, cost_final, increment_final, value_save, mag;
  int jpar;
  BOOL adjust;

  // Save the initial cost function.
  cost_initial = cost_function(icost);

  // Test each parameter by increasing and decreasing its value by the increment.
  for (jpar=0; jpar<NAFFINE; jpar++)
    {
      if ( AlignAid.Adjust[jpar] && AlignAid.increment[jpar] != 0. )
	{ // an increment of zero eliminates the parameter from the search
	  AlignAid.iParameter = jpar;
	  // Save the value of the parameter
	  value_save = Alignment.value[jpar];
	  // find the best value for this parameter.
	  line_scan_1d(icost,cost_initial, &cost_final, &increment_final);
	  Alignment.value[jpar] = value_save;
	  cost_array[jpar] = cost_final - cost_initial;
	  increment[jpar]  = increment_final;

	  refresh_display_image(0);
	  show_alignment_info();
	}
      else
	cost_array[jpar] = increment[jpar]  = 0.;
    }

  // Normalize the cost array.
  for (jpar=0, mag=0.; jpar<NAFFINE; jpar++)
    mag += SQR(cost_array[jpar]);
  mag = sqrt((double)mag);
  // If the magnitude of the cost vector is 0, then no values are lower than the
  // original value.
  if ( mag == 0. )
    return(FALSE);
  for (jpar=0; jpar<NAFFINE; jpar++)
    cost_array[jpar] /= mag;

  // Calculate the projection of the increment along the normalized cost vector.
  for (jpar=0; jpar<NAFFINE; jpar++)
    increment[jpar] = increment[jpar] * cost_array[jpar];

  get_starting_increments( ilevel, max_increment );

  adjust = FALSE;
  for (jpar=0; jpar<NAFFINE; jpar++)
    {
      if ( AlignAid.Adjust[jpar] )
	{
	  Alignment.value[jpar] += increment[jpar];
	  if ( fabs((double)increment[jpar]) >= max_increment[jpar]/8. )
	    adjust = TRUE;
	  else
	    AlignAid.increment[jpar] = 0.;
	}
    }

  // Find the 1st parameter adjustment.
  AlignAid.iParameter = 0;
  while ( !AlignAid.Adjust[AlignAid.iParameter] )
    AlignAid.iParameter++;
  transform_coordinates(1,0);
  show_alignment_info();
  build_image_display(0);

  return(adjust);
}

void line_scan_1d( int icost, float cost_initial, float *cost_final, float *increment_final )
{ // return value is true if a change was made
  float cost0, cost2, cost_start, cost_new;
  float sign, value_initial, value0, value2;
  float X3Point[3], Y3Point[3], XMax, YMax;
  int icount;

  value_initial = Alignment.value[AlignAid.iParameter];

  // Save the initial cost function.
  X3Point[1] = Alignment.value[AlignAid.iParameter];
  Y3Point[1] = cost_initial;
  // test the - direction
  value0 = value_initial - AlignAid.increment[AlignAid.iParameter];
  Alignment.value[AlignAid.iParameter] = value0;
  transform_coordinates(1,0);
  cost0 = cost_function(icost);
  X3Point[0] = Alignment.value[AlignAid.iParameter];
  Y3Point[0] = cost0;
  // test the + direction
  value2 = value_initial + AlignAid.increment[AlignAid.iParameter];
  Alignment.value[AlignAid.iParameter] = value2;
  transform_coordinates(1,0);
  cost2 = cost_function(icost);
  X3Point[2] = Alignment.value[AlignAid.iParameter];
  Y3Point[2] = cost2;

  if ( cost_initial > cost0 && cost_initial > cost2 )
    {
      if ( !ParabolicInterpolation(X3Point, Y3Point, &XMax, &YMax) )
	{
	  *cost_final = YMax;
	  *increment_final = XMax - value_initial;
	  AlignAid.increment[AlignAid.iParameter] /= 2.;
	}
      else
	{ // This should never happen.
	  *cost_final = cost_initial;
	  *increment_final = 0.;
	}
      return;
    }
  else if ( cost_initial == cost0 || cost_initial == cost2 ) // not enough information
    {
      *cost_final = cost_initial;
      *increment_final = 0.;
    }
  else if ( cost0 > cost_initial )
    { // - direction
      cost_start = cost0;
      Alignment.value[AlignAid.iParameter] = value0;
      sign = -1.;
    }
  else if ( cost2 > cost_initial )
    { // + direction
      cost_start = cost2;
      Alignment.value[AlignAid.iParameter] = value2;
      sign = 1.;
    }
  else
    {
      printf("Programming or memory error in line_scan_1d\n");
      printf("X3Point = (%g %g %g), Y3Point = (%g %g %g)\n",
	     X3Point[0],X3Point[1],X3Point[2],Y3Point[0],Y3Point[1],Y3Point[2]);
      exit(1);
    }
  cost_new = cost_start;

  icount = 0;
  while ( cost_new >= cost_start )
    {
      cost_start = cost_new;
      Alignment.value[AlignAid.iParameter] += sign * AlignAid.increment[AlignAid.iParameter];
      transform_coordinates(1,0);
      cost_new = cost_function(icost);
      if ( cost_new/cost_start - 1. < 1.e-5 )
      	return;
      //
      if ( sign < 0 )
	{ // sign = -1; drop last point and incorporate new point
	  X3Point[2] = X3Point[1];
	  X3Point[1] = X3Point[0];
	  X3Point[0] = Alignment.value[AlignAid.iParameter];
	  Y3Point[2] = Y3Point[1];
	  Y3Point[1] = Y3Point[0];
	  Y3Point[0] = cost_new;
	}
      else
	{ // sign = +1; drop first point and incorporate new point
	  X3Point[0] = X3Point[1];
	  X3Point[1] = X3Point[2];
	  X3Point[2] = Alignment.value[AlignAid.iParameter];
	  Y3Point[0] = Y3Point[1];
	  Y3Point[1] = Y3Point[2];
	  Y3Point[2] = cost_new;
	}
      icount++;
    }
  if ( !ParabolicInterpolation(X3Point, Y3Point, &XMax, &YMax) )
    {
      *cost_final = YMax;
      *increment_final = XMax - value_initial;
    }
  else
    {
      *cost_final = cost_initial;
      *increment_final = 0.;
    }

  return;
}

int ParabolicInterpolation(float *X3Point, float *Y3Point, float *XMax, float *YMax)
// return: 0 = ok, 1 = error (not parabolic)
{
  float XMatrix[3][3], XMatrix_T[3][3], **XTX, XTY[3], Beta[3];
  float XNew[3], YNew[3];
  float X0, Y0, amp, maxX, maxY;
  int j1, j2, jsum, iRowFail;

  //  printf("maximize in:\n");
  //  printf("X: %f %f %f\n",X3Point[0],X3Point[1],X3Point[2]);
  //  printf("Y: %f %f %f\n",Y3Point[0],Y3Point[1],Y3Point[2]);

  X0 = X3Point[1];
  Y0 = Y3Point[1];
  // Find the maximum of x, and then scale x to 1.
  for (j1=0, maxX = 0.; j1<3; j1++)
    {
      amp = fabs((double)(X3Point[j1]-X0));
      if ( amp > maxX )
	maxX = amp;
    }
  if ( maxX == 0. )
    {
      printf("Programming error: maxX = 0 in ParabolicInterpolation()\n");
      exit(1);
    }
  for (j1=0; j1<3; j1++)
    XNew[j1] = (X3Point[j1]-X0)/maxX;
  // Find the maximum of y, and then scale y to 1.
  for (j1=0, maxY = 0.; j1<3; j1++)
    {
      amp = fabs((double)(Y3Point[j1]-Y0));
      if ( amp > maxY )
	maxY = amp;
    }
  if ( maxY == 0. )
    { // not parabolic: linear with all y = 0
      *XMax = *YMax = 0.;
      return(1);
    }
  for (j1=0; j1<3; j1++)
    YNew[j1] = (Y3Point[j1]-Y0)/maxY;

  XTX = float_matrix(3,3);
  // y = a*x^2 + b*x + c
  // ... can be solved for 3 points in matrix form: Y = X * Beta

  XMatrix[0][0] = XNew[0] * XNew[0];
  XMatrix[0][1] = XNew[0];
  XMatrix[0][2] = 1.;
  XMatrix[1][0] = XNew[1] * XNew[1];
  XMatrix[1][1] = XNew[1];
  XMatrix[1][2] = 1.;
  XMatrix[2][0] = XNew[2] * XNew[2];
  XMatrix[2][1] = XNew[2];
  XMatrix[2][2] = 1.;

  // Form the transpose of this matrix
  for (j1=0; j1<3; j1++)
    {
      for (j2=0; j2<3; j2++)
	XMatrix_T[j1][j2] = XMatrix[j2][j1];
    }
  // Calculate XTX
  for (j1=0; j1<3; j1++)
    {
      for (j2=0; j2<3; j2++)
	{
	  XTX[j1][j2] = 0;
	  for (jsum=0; jsum<3; jsum++)
	    XTX[j1][j2] += XMatrix_T[j1][jsum] * XMatrix[jsum][j2];
	}
    }
  // Invert XTX
  if ( rInvertSquareMatrix(3,XTX,&iRowFail) )
    {
      printf("maximize:\n");
      printf("X: %f %f %f\n",XNew[0],XNew[1],XNew[2]);
      printf("Y: %f %f %f\n",YNew[0],YNew[1],YNew[2]);
      printf("Error: singular matrix in ParabolicInterpolation\n");
      exit(1);
    }
  // Calculate XTY
  for (j1=0; j1<3; j1++)
    {
      XTY[j1] = 0;
      for (jsum=0; jsum<3; jsum++)
	XTY[j1] += XMatrix_T[j1][jsum] * YNew[jsum];
    }
  // Finally, calculate Beta as (XTX)^1 XT*Y
  for (j1=0; j1<3; j1++)
    {
      Beta[j1] = 0;
      for (jsum=0; jsum<3; jsum++)
	Beta[j1] += XTX[j1][jsum] * XTY[jsum];
    }
  free_float_matrix(XTX,3,3);
  // y = a*x^2 + b*x + c has a maximum or minimum at x = -b/(2a), unless a = 0
  if ( Beta[0] == 0. )
    {
      *XMax = *YMax = 0.;
      return(1);
    }
  else
    {
      *XMax = -Beta[1]/2./Beta[0];
      *YMax = Beta[0]*(*XMax)*(*XMax) + Beta[1]*(*XMax) + Beta[2];
      *XMax *= maxX;
      *YMax *= maxY;
      *XMax += X0;
      *YMax += Y0;
      return(0);
    }
}

float cost_function( int icost ) // return cost function
// icost specifies cost function: 1 = variance, 0 = mutual information
{ // return value is true if a change was made
  float cost, prob_IT[NBINS][NBINS], prob_I[NBINS], prob_T[NBINS];
  float av_individual, av_template, value_individual, value_template, min, max;
  float ampi, ampt, sumi2, sumt2, sumit;
  IPOINT3D Index_reg;
  int indexT, indexI, number_voxels, ibin_I, ibin_T;
  int jI, jT;
  BOOL non_zero;

  if ( icost == 0 )
    { // Mutual information
      // Find the average value of non-zero voxels for the source and target
      calculate_volume_averages( &av_individual, &av_template);
      if ( av_individual == 0. ) av_individual = 1.;
      // Find the min and max.
      for (Index_reg.z=0, min = 1.e10, max=0.; Index_reg.z<template.dim.z; Index_reg.z++)
	{
	  for (Index_reg.y=0; Index_reg.y<template.dim.y; Index_reg.y++)
	    {
	      for (Index_reg.x=0; Index_reg.x<template.dim.x; Index_reg.x++)
		{
		  indexT = index4d_stack(Index_reg.x,Index_reg.y,Index_reg.z,iTime_Template,&template);
		  indexI = index4d_stack(Index_reg.x,Index_reg.y,Index_reg.z,graph.iTime,&individual);
		  non_zero = template.SpaceTime.fData[indexT] != 0.;
		  if ( non_zero )
		    {
		      value_individual = individual.SpaceTime.fData[indexI]/av_individual;
		      value_template   = template.SpaceTime.fData[indexT]/av_template;
		      if ( value_individual < min ) min = value_individual;
		      if ( value_template   < min ) min = value_template;
		      if ( value_individual > max ) max = value_individual;
		      if ( value_template   > max ) max = value_template;
		    }
		}
	    }
	}
      // Initialize the histograms.
      for (jI=0; jI<NBINS; jI++)
	{
	  prob_I[jI] = prob_T[jI] = 0.;
	  for (jT=0; jT<NBINS; jT++)
	    prob_IT[jI][jT] =0.;
	}
      // Fill the histograms
      for (Index_reg.z=0, number_voxels=0; Index_reg.z<template.dim.z; Index_reg.z++)
	{
	  for (Index_reg.y=0; Index_reg.y<template.dim.y; Index_reg.y++)
	    {
	      for (Index_reg.x=0; Index_reg.x<template.dim.x; Index_reg.x++)
		{
		  indexT = index4d_stack(Index_reg.x,Index_reg.y,Index_reg.z,iTime_Template,&template);
		  indexI = index4d_stack(Index_reg.x,Index_reg.y,Index_reg.z,graph.iTime,&individual);
		  non_zero = template.SpaceTime.fData[indexT] != 0.;
		  if ( non_zero )
		    {
		      value_individual = individual.SpaceTime.fData[indexI]/av_individual;
		      value_template   = template.SpaceTime.fData[indexT]/av_template;
		      ibin_I = MIN(NBINS-1,NBINS*(value_individual-min)/(max-min));
		      ibin_T = MIN(NBINS-1,NBINS*(value_template-min)/(max-min));
		      prob_I[ibin_I]++;
		      prob_T[ibin_T]++;
		      prob_IT[ibin_I][ibin_T]++;
		      number_voxels++;
		    }
		}
	    }
	}
      // Normalize the probability functions.
      for (jI=0; jI<NBINS; jI++)
	{
	  prob_I[jI] /= (float)number_voxels;
	  prob_T[jI] /= (float)number_voxels;
	  for (jT=0; jT<NBINS; jT++)
	    prob_IT[jI][jT] /= (float)number_voxels;
	}
      // Calculate the cost function.
      cost = 0.;
      for (jI=0; jI<NBINS; jI++)
	{ // sum over bins of individual
	  if ( prob_I[jI] != 0. )
	    {
	      for (jT=0; jT<NBINS; jT++)
		{ // sum over bins of template
		  if ( prob_IT[jI][jT] != 0. && prob_T[jT] != 0. )
		    cost += prob_IT[jI][jT] * log((double)(prob_IT[jI][jT]/prob_I[jI]/prob_T[jT]));
		}
	    }
	}
    }
  else
    { // Normalized correlation.
      sumi2 = sumt2 = sumit = 0.;
      for (Index_reg.z=0, number_voxels=0; Index_reg.z<template.dim.z; Index_reg.z++)
	{
	  for (Index_reg.y=0; Index_reg.y<template.dim.y; Index_reg.y++)
	    {
	      for (Index_reg.x=0; Index_reg.x<template.dim.x; Index_reg.x++)
		{
		  indexT = index4d_stack(Index_reg.x,Index_reg.y,Index_reg.z,iTime_Template,&template);
		  indexI = index4d_stack(Index_reg.x,Index_reg.y,Index_reg.z,graph.iTime,&individual);
		  non_zero = template.SpaceTime.fData[indexT] != 0.;
		  if ( non_zero )
		    {
		      ampi = individual.SpaceTime.fData[indexI];
		      ampt = template.SpaceTime.fData[indexT];
		      sumi2 += ampi*ampi;
		      sumt2 += ampt*ampt;
		      sumit += ampi*ampt;
		    }
		}
	    }
	}
      cost = sumit / sqrt((double)sumi2) / sqrt((double)sumt2);
    }
  AlignAid.CF_Affine = cost;  // save this value to speed display functions
  return(cost);
}

void calculate_volume_averages( float *av_individual, float *av_template )
{
  IPOINT3D Index_reg;
  int indexT, indexI, number_voxels;
  BOOL non_zero;

  // Find normalization factors.
  *av_individual = *av_template = 0.;
  for (Index_reg.z=0, number_voxels=0; Index_reg.z<template.dim.z; Index_reg.z++)
    {
      for (Index_reg.y=0; Index_reg.y<template.dim.y; Index_reg.y++)
	{
	  for (Index_reg.x=0; Index_reg.x<template.dim.x; Index_reg.x++)
	    {
	      indexT = index4d_stack(Index_reg.x,Index_reg.y,Index_reg.z,iTime_Template,&template);
	      indexI = index4d_stack(Index_reg.x,Index_reg.y,Index_reg.z,graph.iTime,&individual);
	      non_zero = template.SpaceTime.fData[indexT] != 0.;
	      if ( non_zero )
		{
		  *av_individual += individual.SpaceTime.fData[indexI];
		  *av_template   += template.SpaceTime.fData[indexT];
		  number_voxels ++;
		}
	    }
	}
    }
  *av_individual /= (float)number_voxels;
  *av_template   /= (float)number_voxels;
}

void action_set_increments( int level )
     // level: see get_starting_increments
{
  float increment[NAFFINE];
  int j;

  get_starting_increments(level, increment);

  for (j=0; j<NAFFINE; j++)
    AlignAid.increment[j] = increment[j];
}

void get_starting_increments( int level, float *increment )
     // Increments are specified by the level.
     // Levels are set according to the resolution.
     // For translations, the scale ("resolution" below) is the coarsest resolution in x,y,z.
     // For rotations & inflations, the scale is set by largest translation across the FOV.
     // level:
     // 3) 2 * resolution for translations, rotations, inflations
     // 2) resolution for all parameters
     // 1) resolution/2 for all parameters
     // 0) resolution/4 for all parameters
{
  int j;
  float distance, fov_min;

  // Choosing a distance scale is a tradeoff between
  // 1) making steps big enough that they can capture big differences in beginning positions,
  // 2) making steps small enough that images are unlikely to be experience zero overlap during
  //    the first few iterations
  distance = MAX(MAX(template_unfiltered.resolution.x,template_unfiltered.resolution.y),
		 template_unfiltered.resolution.z);
  fov_min  = MIN(MIN(template_unfiltered.fov.x,template_unfiltered.fov.y),template_unfiltered.fov.z);
  distance = MIN(distance,fov_min/16.);

  if ( level == 3 )
    { // coarse scale, no skew
      for (j=0; j<NAFFINE; j++)
	{
	  if ( j < 3 )      // translations
	    increment[j] = 4. * distance;
	  else if ( j < 6 ) // rotations in degrees
	    increment[j] = 15.;
	  else if ( j < 9 ) // inflations
	    increment[j] = 0.1;
	  else if ( j < 12 )// skews
	    increment[j] = 0.1;
	}
    }
  else if ( level == 2 )
    { // medium scale
      for (j=0; j<NAFFINE; j++)
	{
	  if ( j < 3 )      // translations
	    increment[j] = 2*distance;
	  else if ( j < 6 ) // rotations in degrees
	    increment[j] = 10.;
	  else if ( j < 9 ) // inflations
	    increment[j] = 0.05;
	  else if ( j < 12 )// skews
	    increment[j] = 0.05;
	}
    }
  else if ( level == 1 )
    { // fine scale
      for (j=0; j<NAFFINE; j++)
	{
	  if ( j < 3 )      // translations
	    increment[j] = distance;
	  else if ( j < 6 ) // rotations in degrees
	    increment[j] = 5.;
	  else if ( j < 9 ) // inflations
	    increment[j] = 0.02;
	  else if ( j < 12 )// skews
	    increment[j] = 0.02;
	}
    }
  else if ( level == 0 )
    { // ultra-fine scale
      for (j=0; j<NAFFINE; j++)
	{
	  if ( j < 3 )      // translations
	    increment[j] = distance/4.;
	  else if ( j < 6 ) // rotations in degrees
	    increment[j] = 2.;
	  else if ( j < 9 ) // inflations
	    increment[j] = 0.01;
	  else if ( j < 12 )// skews
	    increment[j] = 0.01;
	}
    }
}

void transform_coordinates( BOOL all_voxels, BOOL most_accurate )
{
  float *data_orig, *data_reg, template_data;
  double sum_data, sum_coeff, weight;
  int MinWidth, indexT, index0, j;
  IPOINT3D Index_reg, Low_orig, High_orig, jOrig, jRel, iOrig, iWidth;
  IPOINT3D iVoxelStart, iVoxelEnd;
  POINT3D  Space_reg, Space_orig, Index_orig, RelRes;
  float *coeff_x, *coeff_y, *coeff_z;
  float point4[4];
  BOOL InVolume, ignore_overlap;
  mat44 save_ijk_to_xyz;
  mat44 save_xyz_to_ijk;

  define_affine_matrix();

  // RelRes is the resolution of the original data relative to the alignment target.
  // This value is > 1 when
  //   1) the template is at lower resolution than the collected data, or
  //   2) the data are down-sampled during early alignment steps.
  RelRes.x = Alignment.resolution.x / original.resolution.x;
  RelRes.y = Alignment.resolution.y / original.resolution.y;
  RelRes.z = Alignment.resolution.z / original.resolution.z;

  if ( most_accurate )
    MinWidth = 2;
  else
    MinWidth = 1;

  iWidth.x = MAX(floor((double)(RelRes.x)),MinWidth);
  iWidth.y = MAX(floor((double)(RelRes.y)),MinWidth);
  iWidth.z = MAX(floor((double)(RelRes.z)),MinWidth);

  coeff_x = float_vector(2*iWidth.x+2);   // total width is at least 2*width+1
  coeff_y = float_vector(2*iWidth.y+2);
  coeff_z = float_vector(2*iWidth.z+2);

  data_orig = &(original.SpaceTime.fData[graph.iTime*original.n3d]);
  data_reg  = &(individual.SpaceTime.fData[graph.iTime*individual.n3d]);
  if ( all_voxels )
    {
      for (j=0; j<individual.n3d; j++)
	data_reg[j] = 0.;
    }

  save_ijk_to_xyz = individual.ijk_to_xyz;
  save_xyz_to_ijk = original.xyz_to_ijk;

  // AlignAid.ijk_to_xyz and AlignAid.xyz_to_ijk are merely time-saving devices
  // to avoid recalculating centered-coordinate transformations on every entry.
  individual.ijk_to_xyz = AlignAid.ijk_to_xyz;  // target
  original.xyz_to_ijk   = AlignAid.xyz_to_ijk;  // source

  if ( all_voxels )
    {
      iVoxelStart.x = iVoxelStart.y = iVoxelStart.z = 0;
      iVoxelEnd = template.dim;
    }
  else
    {
      iVoxelStart = AlignNL.Index3DStart[AlignNL.iBasis];
      iVoxelEnd   = AlignNL.Index3DEnd[AlignNL.iBasis];
    }

  // Loop over voxels in the standard resolution space.
  for (Index_reg.z=iVoxelStart.z; Index_reg.z<iVoxelEnd.z; Index_reg.z++)
    {
      for (Index_reg.y=iVoxelStart.y; Index_reg.y<iVoxelEnd.y; Index_reg.y++)
	{
	  for (Index_reg.x=iVoxelStart.x; Index_reg.x<iVoxelEnd.x; Index_reg.x++)
	    {
	      // point to the index for the current point (template or individual)
	      index0 = index4d(Index_reg.x,Index_reg.y,Index_reg.z,0);
	      indexT = index4d_stack(Index_reg.x,Index_reg.y,Index_reg.z,iTime_Template,&template);
	      // See if the template value is zero for this voxel.
	      // If it is zero, it won't be used in the registration anyway, so don't register this point.
	      template_data = template.SpaceTime.fData[indexT];
	      ignore_overlap = (template_data == 0.);
	      ignore_overlap = FALSE;
	      if ( ignore_overlap ) // && working_hard )
		{
		  index0 = Index_reg.z*Alignment.dimension.x*Alignment.dimension.y
		    + Index_reg.y*Alignment.dimension.x + Index_reg.x;
		  data_reg[index0] = 0.;
		}
	      else
		{
		  // Convert from data indices --> space in the registered system.
		  Space_reg = Convert_Voxel_to_Space( Index_reg, &individual );
		  point4[0] = Space_reg.x;
		  point4[1] = Space_reg.y;
		  point4[2] = Space_reg.z;
		  point4[3] = 1;
		  // Rotate & translate.
		  Space_orig.x = Space_orig.y = Space_orig.z = 0.;
		  for (j=0; j<4; j++)
		    {
		      Space_orig.x += Alignment.Affine[0][j] * point4[j];
		      Space_orig.y += Alignment.Affine[1][j] * point4[j];
		      Space_orig.z += Alignment.Affine[2][j] * point4[j];
		    }

		  // non-linear distortion fields.
		  Space_orig.x += distort_x.SpaceTime.fData[index0];
		  Space_orig.y += distort_y.SpaceTime.fData[index0];
		  Space_orig.z += distort_z.SpaceTime.fData[index0];
		  // Convert from space --> indices in the original (non-registered) space.
		  Index_orig = Convert_Space_to_rVoxel( Space_orig, &original );
		  // If the nearest data value in the original system is zero
		  // then ignore this point. This prevents values from being assigned
		  // to pixels due only to neighbors (extrapolation)
		  jOrig.x = rint((double)Index_orig.x);
		  jOrig.y = rint((double)Index_orig.y);
		  jOrig.z = rint((double)Index_orig.z);
		  if ( Alignment.WrapAround.x )
		    { // Wrap x
		      if ( jOrig.x < 0 )
			jOrig.x += original.dim.x;
		      else if ( jOrig.x >= original.dim.x )
			jOrig.x -= original.dim.x;
		    }
		  if ( Alignment.WrapAround.y )
		    { // Wrap y
		      if ( jOrig.y < 0 )
			jOrig.y += original.dim.y;
		      else if ( jOrig.y >= original.dim.y )
			jOrig.y -= original.dim.y;
		    }
		  if ( Alignment.WrapAround.z )
		    { // Wrap z
		      if ( jOrig.z < 0 )
			jOrig.z += original.dim.z;
		      else if ( jOrig.z >= original.dim.z )
			jOrig.z -= original.dim.z;
		    }
		  InVolume = ( jOrig.x>=0 && jOrig.x<original.dim.x &&
			       jOrig.y>=0 && jOrig.y<original.dim.y &&
			       jOrig.z>=0 && jOrig.z<original.dim.z );
		  if ( InVolume )
		    {
		      index0 = jOrig.z*original.dim.x*original.dim.y
			+      jOrig.y*original.dim.x + jOrig.x;
		      if ( data_orig[index0] == 0. )
			InVolume = FALSE;
		    }

		  if ( InVolume )
		    {
		      // define limits
		      Low_orig.x  = floor((double)Index_orig.x);
		      High_orig.x = ceil((double)Index_orig.x);
		      Low_orig.y  = floor((double)Index_orig.y);
		      High_orig.y = ceil((double)Index_orig.y);
		      Low_orig.z  = floor((double)Index_orig.z);
		      High_orig.z = ceil((double)Index_orig.z);
		      // Interpolate to rectilinear pixels in original resolution space.
		      for (jOrig.x=Low_orig.x-iWidth.x, jRel.x=0;
			   jOrig.x<=High_orig.x+iWidth.x; jOrig.x++, jRel.x++)
			{
			  if ( most_accurate )
			    coeff_x[jRel.x] = Lanczos_filter((float)(Index_orig.x-jOrig.x),iWidth.x);
			  else
			    coeff_x[jRel.x] = Gauss((float)(Index_orig.x-jOrig.x), 0.,0.425*RelRes.x);
			}
		      for (jOrig.y=Low_orig.y-iWidth.y, jRel.y=0;
			   jOrig.y<=High_orig.y+iWidth.y; jOrig.y++, jRel.y++)
			{
			  if ( most_accurate )
			    coeff_y[jRel.y] = Lanczos_filter((float)(Index_orig.y-jOrig.y),iWidth.y);
			  else
			    coeff_y[jRel.y] = Gauss((float)(Index_orig.y-jOrig.y), 0.,0.425*RelRes.y);
			}
		      for (jOrig.z=Low_orig.z-iWidth.z, jRel.z=0;
			   jOrig.z<=High_orig.z+iWidth.z; jOrig.z++, jRel.z++)
			{
			  if ( most_accurate )
			    coeff_z[jRel.z] = Lanczos_filter((float)(Index_orig.z-jOrig.z),iWidth.z);
			  else
			    coeff_z[jRel.z] = Gauss((float)(Index_orig.z-jOrig.z), 0.,0.425*RelRes.z);
			}
		      sum_coeff = sum_data = 0.;
		      for (jOrig.z=Low_orig.z-iWidth.z, jRel.z=0;
			   jOrig.z<=High_orig.z+iWidth.z; jOrig.z++, jRel.z++)
			{
			  iOrig.z = jOrig.z;
			  if ( Alignment.WrapAround.z )
			    { // Wrap z
			      if ( iOrig.z < 0 )
				iOrig.z += original.dim.z;
			      else if ( iOrig.z >= original.dim.z )
				iOrig.z -= original.dim.z;
			    }
			  for (jOrig.y=Low_orig.y-iWidth.y, jRel.y=0;
			       jOrig.y<=High_orig.y+iWidth.y; jOrig.y++, jRel.y++)
			    {
			      iOrig.y = jOrig.y;
			      if ( Alignment.WrapAround.y )
				{ // Wrap y
				  if ( iOrig.y < 0 )
				    iOrig.y += original.dim.y;
				  else if ( iOrig.y >= original.dim.y )
				    iOrig.y -= original.dim.y;
				}
			      for (jOrig.x=Low_orig.x-iWidth.x, jRel.x=0;
				   jOrig.x<=High_orig.x+iWidth.x; jOrig.x++, jRel.x++)
				{
				  iOrig.x = jOrig.x;
				  if ( Alignment.WrapAround.x )
				    { // Wrap x
				      if ( iOrig.x < 0 )
					iOrig.x += original.dim.x;
				      else if ( iOrig.x >= original.dim.x )
					iOrig.x -= original.dim.x;
				    }
				  InVolume = ( iOrig.x>=0 && iOrig.x<original.dim.x &&
					       iOrig.y>=0 && iOrig.y<original.dim.y &&
					       iOrig.z>=0 && iOrig.z<original.dim.z );
				  if ( InVolume )
				    {
				      // sum over neighbors in x
				      index0 = iOrig.z*original.dim.x*original.dim.y
					+      iOrig.y*original.dim.x + iOrig.x;
				      weight = coeff_x[jRel.x] * coeff_y[jRel.y] * coeff_z[jRel.z];
				      sum_data  += weight * data_orig[index0];
				      sum_coeff += weight;
				    }
				}
			    }
			}
		      index0 = Index_reg.z*Alignment.dimension.x*Alignment.dimension.y
			+ Index_reg.y*Alignment.dimension.x + Index_reg.x;
		      if ( sum_coeff == 0. )
			data_reg[index0] = 0.;
		      else
			data_reg[index0] = sum_data / sum_coeff;
		    }
		}
	    }
	}
    }
  free_float_vector(coeff_x);
  free_float_vector(coeff_y);
  free_float_vector(coeff_z);

  individual.ijk_to_xyz = save_ijk_to_xyz;
  original.xyz_to_ijk   = save_xyz_to_ijk;
}

void align_individual_template_centroids()
//  Adjust x,y,z affine translations to align centroids for the individual and template.
{
  IPOINT3D Index_reg;
  POINT3D Centroid_individual, Centroid_template, Space;
  float Mag_template, Mag_individual, MagSum_template, MagSum_individual;
  int indexI, indexT;

  // Compute the centroids
  Centroid_template.x   = Centroid_template.y   = Centroid_template.z   = 0.;
  Centroid_individual.x = Centroid_individual.y = Centroid_individual.z = 0.;
  // Loop over voxels in the standard resolution space.
  for (Index_reg.z=0; Index_reg.z<template.dim.z; Index_reg.z++)
    {
      for (Index_reg.y=0; Index_reg.y<template.dim.y; Index_reg.y++)
	{
	  for (Index_reg.x=0; Index_reg.x<template.dim.x; Index_reg.x++)
	    {
	      Space  = Convert_Voxel_to_Space( Index_reg, &template );
	      // point to the index for the current point (template or individual)
	      indexT = index4d_stack(Index_reg.x,Index_reg.y,Index_reg.z,iTime_Template,&template);
	      indexI = index4d_stack(Index_reg.x,Index_reg.y,Index_reg.z,graph.iTime,&individual);
	      Mag_template           = template.SpaceTime.fData[indexT];
	      Mag_individual         = individual.SpaceTime.fData[indexI];
	      MagSum_template       += Mag_template;
	      MagSum_individual     += Mag_individual;
	      Centroid_template.x   += Mag_template   * Space.x;
	      Centroid_template.y   += Mag_template   * Space.y;
	      Centroid_template.z   += Mag_template   * Space.z;
	      Centroid_individual.x += Mag_individual * Space.x;
	      Centroid_individual.y += Mag_individual * Space.y;
	      Centroid_individual.z += Mag_individual * Space.z;
	    }
	}
    }
  Centroid_template.x   /= MagSum_template;
  Centroid_template.y   /= MagSum_template;
  Centroid_template.z   /= MagSum_template;
  Centroid_individual.x /= MagSum_individual;
  Centroid_individual.y /= MagSum_individual;
  Centroid_individual.z /= MagSum_individual;

  Alignment.value[0] = (Centroid_individual.x - Centroid_template.x);
  Alignment.value[1] = (Centroid_individual.y - Centroid_template.y);
  Alignment.value[2] = (Centroid_individual.z - Centroid_template.z);
}
