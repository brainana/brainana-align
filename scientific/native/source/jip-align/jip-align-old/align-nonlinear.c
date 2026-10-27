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

#define NBINS 50


void action_auto_adjust_nonlinear( int button )
{
  static BOOL first_time=TRUE;
  BOOL MultiStep;
  int Lambda_save;

  MultiStep = (button == 1);
  if ( !MultiStep )
    auto_adjust_nonlinear_iterations(0,1);
  else
    {
      if ( first_time )
	{ // The first time, adjust affine parameters in a hierarchical way.
	  // level 3: coarse
	  //	  if ( AlignAid.SubSample.x == 1 &&
	  //	       AlignAid.SubSample.y == 1 &&
	  //	       AlignAid.SubSample.z == 1 )
	  SetGridDown( 8 );	// Course scale
	  Lambda_save = AlignNL.Lambda_reg;
	  AlignNL.Lambda_reg = 1;
	  update_nonlinear_alignment_parameters();
	  update_alignment(0);
	  show_alignment_info();
	  auto_adjust_nonlinear_iterations(0,3);
	  // level 2: medium
	  SetGridDown( 4 ); // medium scale
	  //	  AlignNL.Lambda_reg = (1+Lambda_save)/2; // average of lowest value (1) and set value
	  AlignNL.Lambda_reg = Lambda_save;       // final adjustment at set value
	  update_nonlinear_alignment_parameters();
	  update_alignment(0);
	  show_alignment_info();
	  auto_adjust_nonlinear_iterations(0,2);
	  // level 1: fine
	  SetGridDown( 2 ); // fine scale
	  AlignNL.Lambda_reg = Lambda_save;       // final adjustment at set value
	  update_nonlinear_alignment_parameters();
	  update_alignment(0);
	  show_alignment_info();
	  auto_adjust_nonlinear_iterations(0,1);
	}
      else
	{ // Back up one level.
	  // level 2: medium
	  SetGridDown( 2*AlignNL.iGridDown );
	  update_nonlinear_alignment_parameters();
	  show_alignment_info();
	  auto_adjust_nonlinear_iterations(0,2);
	  // level 1: fine
	  SetGridDown( AlignNL.iGridDown/2 );
	  update_nonlinear_alignment_parameters();
	  show_alignment_info();
	  auto_adjust_nonlinear_iterations(0,1);
	}
    }
  first_time = FALSE;
  // Final transformation: use most accurate interpolation
  update_alignment(1);
  show_on_info_line(0,0,"done","","");
  fprintf(stdout,"done\n");
  fflush(stdout);
}

void auto_adjust_nonlinear_iterations( int icost, int ilevel )
     // icost: 0 = mutual informtation, 1 = normalized correlation
{
  int ipass, j, nbad;
  float cost_NoPenalty, cost_initial, cost_final, tolerance;
  BOOL converged;

  //  define_affine_matrix();

  //  if ( AlignAid.DisplayType != 0 ) // switch to individual
  //    action_change_display_type(0);

  if ( AlignAid.DisplaySet != 2 )
    {
      AlignAid.DisplaySet = 2;
      update_display_set();
      show_alignment_info();
    }

  for (j=0; j<3*AlignNL.NBasis; j++)
    AlignNL.Adjust3D[j] = 0;
  // Set the adjustments
  if ( AlignNL.ViewBasis )
    {
      if ( AlignNL.Adjust.x )
	AlignNL.Adjust3D[AlignNL.iBasis] = 1;
      else if ( AlignNL.Adjust.y )
	AlignNL.Adjust3D[AlignNL.iBasis+AlignNL.NBasis] = 1;
      else
	AlignNL.Adjust3D[AlignNL.iBasis+2*AlignNL.NBasis] = 1;
    }
  else
    {
      for (j=0; j<AlignNL.NBasis; j++)
	{
	  if ( AlignNL.Adjust.x )
	    AlignNL.Adjust3D[j] = 1;
	  if ( AlignNL.Adjust.y )
	    AlignNL.Adjust3D[j+AlignNL.NBasis] = 1;
	  if ( AlignNL.Adjust.z )
	    AlignNL.Adjust3D[j+2*AlignNL.NBasis] = 1;
	}
    }
  // set the adjusted flags
  AlignNL.Adjusted = AlignNL.Adjust;

  // Save the starting parameter values for this iteration.
  cost_initial = cost_function_nonlinear(icost,0); // init
  // Find the best set of parameters at this resolution level.
  ipass = 0;
  if ( ilevel == 1 )
    tolerance = 0.03;
  else if ( ilevel == 2 )
    tolerance = 0.05;
  else
    tolerance = 0.10;
  cost_NoPenalty = cost_function(AlignAid.CostFunction);

  converged  = FALSE;
  while ( !converged && ipass < 10 )
    {
      adjust_nonlinear_1pass(ilevel,ipass,icost,cost_initial);
      cost_final = cost_function_nonlinear(icost,0); // init
      converged = (cost_final-cost_initial) < tolerance * cost_NoPenalty;
      //      printf("cost_initial = %g, cost_final = %g\n",cost_initial,cost_final);
      cost_initial = cost_final;
      cost_NoPenalty = cost_function(AlignAid.CostFunction);
      ipass++;
    }
  // Check for overly large local distortions
  nbad = number_invalid_distortions(1, ilevel,-1);
  if ( nbad != 0 )
    {
      printf("There are %d bad links in x, y, or z\n",nbad);
      printf("Increase rigidity & repeat.\n");
    }
}

void adjust_nonlinear_1pass( int ilevel, int ipass, int icost, float cost_start )
     // icost:  0 = diff, 1 = mutual informtation
{
  char string1[LEN_STR], string2[LEN_STR], string3[LEN_STR];
  float cost_initial, cost_save, value_save, res, shift;
  int NCoeff, iCoeff, iMod, nbad, iScan, j;

  // Find the number of coefficients
  NCoeff = 0;
  if ( AlignNL.Adjust.x ) NCoeff += AlignNL.NBasis;
  if ( AlignNL.Adjust.y ) NCoeff += AlignNL.NBasis;
  if ( AlignNL.Adjust.z ) NCoeff += AlignNL.NBasis;
  if ( NCoeff < 1000 )
    iMod = 10;
  else if ( NCoeff < 10000 )
    iMod = 100;
  else if ( NCoeff < 100000 )
    iMod = 1000;
  else
    iMod = 5000;

  nbad = number_invalid_distortions(1, ilevel, ipass);
  if ( nbad != 0 && ilevel == 1 )
    { // adjust all basis functions if there are bad links anywhere
      for (j=0; j<AlignNL.NBasis; j++)
	{
	  if ( AlignNL.Adjust.x )
	    AlignNL.Adjust3D[j] = 1;
	  if ( AlignNL.Adjust.y )
	    AlignNL.Adjust3D[j+AlignNL.NBasis] = 1;
	  if ( AlignNL.Adjust.z )
	    AlignNL.Adjust3D[j+2*AlignNL.NBasis] = 1;
	}
    }
  if ( icost == 0 )
    sprintf(string1,"MI = %5.3f            , level #%d",AlignAid.CF_Affine,ilevel);
  else
    sprintf(string1,"normalized correlation, level %d",ilevel);

  cost_initial = cost_start;
  // Test each parameter by a 1d line scan.

  for (j=iCoeff=0; j<3*AlignNL.NBasis; j++)
    {
      AlignNL.iCoeff = AlignNL.AdjustOrder[j];
      if ( AlignNL.Adjust3D[AlignNL.iCoeff] )
	{ // an increment of zero eliminates the parameter from the search
	  if ( iCoeff%iMod == 0 )
	    {
	      sprintf(string2,"coefficients: %7d ,  pass #%d",NCoeff,ipass+1);
	      sprintf(string3,"coefficent  : %7d",iCoeff);
	      show_on_info_line(0,0,string1,string2,string3);
	    }
	  // iCoeff goes from 0 to 3*NBasis to account for (x,y,z) fields: convert to basis function
	  if ( AlignNL.iCoeff < AlignNL.NBasis )
	    {
	      AlignNL.iBasis = AlignNL.iCoeff;
	      res = template.resolution.x;
	    }
	  else if ( AlignNL.iCoeff < 2*AlignNL.NBasis )
	    {
	      AlignNL.iBasis = AlignNL.iCoeff - AlignNL.NBasis;
	      res = template.resolution.y;
	    }
	  else
	    {
	      AlignNL.iBasis = AlignNL.iCoeff - 2*AlignNL.NBasis;
	      res = template.resolution.z;
	    }
	  // Save the initial cost function.
	  value_save = AlignNL.value[AlignNL.iCoeff];
	  // find the best value for this parameter.
	  if ( (iScan=line_scan_1d_nonlinear(icost,cost_initial)) == 1 )
	    { // Then a better value was found
	      shift = fabs((double)(AlignNL.value[AlignNL.iCoeff]-value_save));
	      if ( shift < res/10. )
		AlignNL.Adjust3D[AlignNL.iCoeff] = 0;
	    }
	  else if ( iScan == 0 )
	    AlignNL.Adjust3D[AlignNL.iCoeff] = 0;
	  cost_save = cost_initial;
	  cost_initial = cost_function_nonlinear(icost,3); // set 3
	  iCoeff++;
	}
    }

  adjust_the_adjust_flags(icost);
  transform_coordinates(1,0);
  show_alignment_info();
  update_image_selectable_text();
  draw_selectable_text_image();
  build_image_display(0);

  return;
}

int line_scan_1d_nonlinear( int icost, float cost_initial )
{ // return value is true if a change was made
  float cost0, cost2, cost_start, cost_new;
  float sign, value_initial, value0, value2;
  float X3Point[3], Y3Point[3], XMax, YMax;
  int icount;
  BOOL BadOriginally;

  BadOriginally = number_invalid_distortions(1,0,0);

  // Save the initial cost function.
  value_initial = AlignNL.value[AlignNL.iCoeff];
  cost_start    = cost_function_nonlinear(icost,1);  // ProbeInit 1 (disregard returned value)
  //
  // *** Must re-evaluate cost_initial when a restricted range is used.
  //
  //  Set_AlignNL_Value(AlignNL.iCoeff,value_initial);
  X3Point[1] = AlignNL.value[AlignNL.iCoeff];
  Y3Point[1] = cost_initial;
  // test the - direction
  value0 = value_initial - AlignNL.increment[AlignNL.iCoeff];
  Set_AlignNL_Value(AlignNL.iCoeff,value0);
  cost0 = cost_function_nonlinear(icost,2);  // probe 2
  X3Point[0] = AlignNL.value[AlignNL.iCoeff];
  Y3Point[0] = cost0;

  // test the + direction
  value2 = value_initial + AlignNL.increment[AlignNL.iCoeff];
  Set_AlignNL_Value(AlignNL.iCoeff,value2);
  cost2 = cost_function_nonlinear(icost,2);  // probe 2
  X3Point[2] = AlignNL.value[AlignNL.iCoeff];
  Y3Point[2] = cost2;

  if ( cost_initial > cost0 && cost_initial > cost2 )
    { // There is a maximum between points 0 and 2, so interpolate
      if ( ParabolicInterpolation(X3Point, Y3Point, &XMax, &YMax) )
	{
	  Set_AlignNL_Value(AlignNL.iCoeff,value_initial);
	  return(0);   // error in interpolation: not parabolic
	}
      Set_AlignNL_Value(AlignNL.iCoeff,XMax);
      if ( !BadOriginally && number_invalid_distortions(1,0,0) )
	{ // if the jacobian is < 0, don't accept the result - just decrease the increment
	  Set_AlignNL_Value(AlignNL.iCoeff,value_initial);
	  AlignNL.increment[AlignNL.iCoeff] /= 2.;
	  return(-1);  // don't accept - creates a bad Jacobian
	}
      else
	{ // The jacobian is ok: make sure the result is an improvement in cost function
	  cost_new = cost_function_nonlinear(icost,2);  // probe
	  if ( cost_new > cost_initial )
	    {
	      AlignNL.increment[AlignNL.iCoeff] /= 2.;
	      return(1); // accept change and reduce search range
	    }
	  else
	    {
	      Set_AlignNL_Value(AlignNL.iCoeff,value_initial);
	      return(0); // didn't reduce cost function, so restore original value
	    }
	}
    }
  else if ( cost_initial == cost0 || cost_initial == cost2 ) // not enough information
    {
      Set_AlignNL_Value(AlignNL.iCoeff,value_initial);
      if ( !BadOriginally && number_invalid_distortions(1,0,0) )
	{ // if the jacobian is < 0, don't accept the result - just decrease the increment
	  AlignNL.increment[AlignNL.iCoeff] /= 2.;
	}
      return(-1);  // bad Jacobian or no improvement in cost function
    }
  else if ( cost0 > cost_initial )
    { // - direction is greatest cost value
      cost_start = cost0;
      Set_AlignNL_Value(AlignNL.iCoeff,value0);
      if ( !BadOriginally && number_invalid_distortions(1,0,0) )
	{ // if the jacobian is < 0, don't accept the result - just decrease the increment
	  Set_AlignNL_Value(AlignNL.iCoeff,value_initial);
	  AlignNL.increment[AlignNL.iCoeff] /= 2.;
	  return(-1); // bad Jacobian
	}
      if ( (cost0-cost_initial)/cost0 < 1.e-3 )
	return(1);  // don't evaluate further if the change in cost is small
      else
	sign = -1.; // seach below in the negative direction
    }
  else if ( cost2 > cost_initial )
    { // + direction is greatest cost value
      cost_start = cost2;
      Set_AlignNL_Value(AlignNL.iCoeff,value2);
      if ( !BadOriginally && number_invalid_distortions(1,0,0) )
	{ // if the jacobian is < 0, don't accept the result - just decrease the increment
	  Set_AlignNL_Value(AlignNL.iCoeff,value_initial);
	  AlignNL.increment[AlignNL.iCoeff] /= 2.;
	  return(-1);  // bad Jacobian
	}
      if ( (cost2-cost_initial)/cost2 < 1.e-3 )
	return(1); // don't evaluate further if the change in cost is small
      else
	sign = 1.; // seach below in the positive direction
    }
  else
    {
      printf("Programming or memory error in line_scan_1d_nonlinear\n");
      printf("X3Point = (%g %g %g), Y3Point = (%g %g %g)\n",
	     X3Point[0],X3Point[1],X3Point[2],Y3Point[0],Y3Point[1],Y3Point[2]);
      exit(1);
    }
  cost_new = cost_start;

  // The maximum cost value was not bracketed by the initial search, so continue.
  icount = 0;
  while ( (cost_new >= cost_start) && icount < 10 )
    {
      cost_start = cost_new;
      Set_AlignNL_Value(AlignNL.iCoeff,
			AlignNL.value[AlignNL.iCoeff] + sign * AlignNL.increment[AlignNL.iCoeff]);
      cost_new = cost_function_nonlinear(icost,2);  // probe
      //
      if ( sign < 0 )
	{ // sign = -1; drop last point and incorporate new point
	  X3Point[2] = X3Point[1];
	  X3Point[1] = X3Point[0];
	  X3Point[0] = AlignNL.value[AlignNL.iCoeff];
	  Y3Point[2] = Y3Point[1];
	  Y3Point[1] = Y3Point[0];
	  Y3Point[0] = cost_new;
	}
      else if ( sign > 0 )
	{ // sign = +1; drop first point and incorporate new point
	  X3Point[0] = X3Point[1];
	  X3Point[1] = X3Point[2];
	  X3Point[2] = AlignNL.value[AlignNL.iCoeff];
	  Y3Point[0] = Y3Point[1];
	  Y3Point[1] = Y3Point[2];
	  Y3Point[2] = cost_new;
	}
      else
	{
	  printf("Programming error: sign == 0.\n");
	  exit(1);
	}
      icount++;
    }
  if ( ParabolicInterpolation(X3Point, Y3Point, &XMax, &YMax) )
    {
      Set_AlignNL_Value(AlignNL.iCoeff,value_initial);
      return(0);  // interpolation error: not parabolic
    }
  //  if ( fabs((double)(AlignNL.value[AlignNL.iCoeff]-XMax) < AlignNL.increment[AlignNL.iCoeff] )
  Set_AlignNL_Value(AlignNL.iCoeff,XMax);
  if ( !BadOriginally && number_invalid_distortions(1,0,0) )
    { // if the jacobian is < 0, don't accept the result - just decrease the increment
      Set_AlignNL_Value(AlignNL.iCoeff,value_initial);
      AlignNL.increment[AlignNL.iCoeff] /= 2.;
      return(-1);  // bad Jacobian
    }

  cost_new = cost_function_nonlinear(icost,2);  // probe
  if ( cost_new > cost_initial )
    return(1);  // accept change; reduced cost function
  else
    {
      Set_AlignNL_Value(AlignNL.iCoeff,value_initial);
      return(0);  // didn't reduce cost function
    }
}

float cost_function_nonlinear( int icost, int InitProbeSet )
// InitProbeSet:
// 0 = Init      : compute averages and cost function across full volume; initialize histograms
// 1 = InitProbe : compute histograms across current basis function and subtract from cumulative histograms
// 2 = Probe     : compute cost function across current basis; reset histograms when done
// 3 = Set       : compute cost function across current basis; do not reset histograms when done
// icost specifies cost function: 0 = variance, 1 = mutual information
{ // return value is true if a change was made
  float cost;
  float value_individual, value_template;
  float ampi, ampt, sumi2, sumt2, sumit, cost_def;
  IPOINT3D Index_reg, iVoxelStart, iVoxelEnd;
  int indexT, indexI, ibin_I, ibin_T;
  int jI, jT;
  BOOL non_zero, all_bases;
  int   MI_save_voxels;
  float MI_save_I[NBINS], MI_save_T[NBINS], MI_save_IT[NBINS][NBINS];
  static float av_individual, av_template, min, max;
  static int   MI_number_voxels;
  static float MI_prob_I[NBINS], MI_prob_T[NBINS], MI_prob_IT[NBINS][NBINS];

  if ( InitProbeSet == 0 )
    { // init: compute cost function across full volume
      iVoxelStart.x = iVoxelStart.y = iVoxelStart.z = 0;
      iVoxelEnd = template.dim;
      all_bases = TRUE;
    }
  else
    { // probe or set: compute cost function across current basis function, & update histograms
      iVoxelStart = AlignNL.Index3DStart[AlignNL.iBasis];
      iVoxelEnd   = AlignNL.Index3DEnd[AlignNL.iBasis];
      all_bases   = FALSE;
    }

  if ( icost == 0 )
    { // Mutual information
      // Find the average value of non-zero voxels for the source and target
      if ( InitProbeSet == 0 )
	{ // recalculate averages across the volumes
	  calculate_volume_averages_nonlinear( all_bases, &av_individual, &av_template, &min, &max);
	}
      if ( max-min <= 0. )
	cost = 0.;
      else
	{
	  if ( InitProbeSet == 0 ) // init
	    { // initialize all cumulative values
	      MI_number_voxels = 0;
	      // Initialize the histograms.
	      for (jI=0; jI<NBINS; jI++)
		{
		  MI_prob_I[jI] = MI_prob_T[jI] = 0.;
		  for (jT=0; jT<NBINS; jT++)
		    MI_prob_IT[jI][jT] = 0.;
		}
	    }
	  MI_save_voxels = 0.;
	  for (jI=0; jI<NBINS; jI++)
	    {
	      MI_save_I[jI] = MI_save_T[jI] = 0.;
	      for (jT=0; jT<NBINS; jT++)
		MI_save_IT[jI][jT] = 0.;
	    }
	  // Fill the histograms
	  for (Index_reg.z=iVoxelStart.z; Index_reg.z<iVoxelEnd.z; Index_reg.z++)
	    {
	      for (Index_reg.y=iVoxelStart.y; Index_reg.y<iVoxelEnd.y; Index_reg.y++)
		{
		  for (Index_reg.x=iVoxelStart.x; Index_reg.x<iVoxelEnd.x; Index_reg.x++)
		    {
		      indexT = index4d_stack(Index_reg.x,Index_reg.y,Index_reg.z,iTime_Template,&template);
		      indexI = index4d_stack(Index_reg.x,Index_reg.y,Index_reg.z,graph.iTime,&individual);
		      non_zero = template.SpaceTime.fData[indexT] != 0.;
		      if ( non_zero )
			{
			  value_individual = individual.SpaceTime.fData[indexI]/av_individual;
			  value_template   = template.SpaceTime.fData[indexT]/av_template;
			  ibin_I = MAX(0,MIN(NBINS-1,NBINS*(value_individual-min)/(max-min)));
			  ibin_T = MAX(0,MIN(NBINS-1,NBINS*(value_template-min)/(max-min)));
			  if ( InitProbeSet != 1 ) // ProbeInit
			    { // update cumulative histograms used for MI EXCEPT for "Probe Init"
			      MI_prob_I[ibin_I]++;
			      MI_prob_T[ibin_T]++;
			      MI_prob_IT[ibin_I][ibin_T]++;
			      MI_number_voxels++;
			    }
			  if ( InitProbeSet == 1 || InitProbeSet == 2 ) // ProbeInit or Probe
			    { // These are temporary histograms
			      MI_save_I[ibin_I]++;
			      MI_save_T[ibin_T]++;
			      MI_save_IT[ibin_I][ibin_T]++;
			      MI_save_voxels++;
			    }
			}
		    }
		}
	    }
	  // Normalize the probability functions.
	  if ( MI_number_voxels == 0. )
	    cost = 0.;
	  else
	    cost = mutual_information(MI_number_voxels, &(MI_prob_I[0]), &(MI_prob_T[0]), &(MI_prob_IT[0][0]));

	  if ( InitProbeSet == 1 || InitProbeSet == 2 ) // ProbeInit or Probe
	    { // Reset the voxel count and histograms, as this is only a temporary computation of MI
	      MI_number_voxels -= MI_save_voxels;
	      for (jI=0; jI<NBINS; jI++)
		{
		  MI_prob_I[jI] = MAX(0,MI_prob_I[jI] - MI_save_I[jI]);
		  MI_prob_T[jI] = MAX(0,MI_prob_T[jI] - MI_save_T[jI]);
		  for (jT=0; jT<NBINS; jT++)
		    MI_prob_IT[jI][jT] = MAX(0,MI_prob_IT[jI][jT]-MI_save_IT[jI][jT]);
		}
	    }
	}
    }
  else
    { // Normalized correlation.
      sumi2 = sumt2 = sumit = 0.;
      for (Index_reg.z=iVoxelStart.z; Index_reg.z<iVoxelEnd.z; Index_reg.z++)
	{
	  for (Index_reg.y=iVoxelStart.y; Index_reg.y<iVoxelEnd.y; Index_reg.y++)
	    {
	      for (Index_reg.x=iVoxelStart.x; Index_reg.x<iVoxelEnd.x; Index_reg.x++)
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
      if ( (sumi2 == 0. ) || (sumt2==0.) )
	cost=0.;
      else
	cost = sumit / sqrt((double)sumi2) / sqrt((double)sumt2);
    }
  AlignAid.CF_Affine = cost;  // save this value to speed display functions

  // Add on the regularization term.
  if ( AlignNL.Lambda_reg != 0 )
    {
      cost_def = cost_deformation( InitProbeSet, 1 );
      cost -= (float)(AlignNL.Lambda_reg*AlignNL.Lambda_reg)/10. * cost_def;
      AlignAid.CF_NonLinear = cost;  // save this value to speed display functions
    }
  return(cost);
}

float mutual_information(int number_voxels, float *prob_I, float *prob_T, float *prob_IT)
{
  float cost, rVoxels;
  int jI, jT, jIT;

  rVoxels = number_voxels;
  // Calculate the cost function.
  cost = 0.;
  for (jI=0; jI<NBINS; jI++)
    { // sum over bins of individual
      if ( prob_I[jI] != 0. )
	{
	  for (jT=0; jT<NBINS; jT++)
	    { // sum over bins of template
	      jIT = jI*NBINS + jT;
	      {
		if ( rVoxels > 0 && prob_IT[jIT] > 0. && prob_I[jI] > 0. && prob_T[jT] > 0. )
		  cost += prob_IT[jIT]/rVoxels
		    * log((double)(rVoxels*prob_IT[jIT]/prob_I[jI]/prob_T[jT]));
	      }
	    }
	}
    }
  return(cost);
}

float cost_deformation( int InitProbeSet, BOOL normalize )
// InitProbeSet:
// 0 = Init      : compute cost function across full volume; initialize total
// 1 = InitProbe : compute cost function across current basis function and subtract from total
// 2 = Probe     : compute cost function across current basis; reset total when done
// 3 = Set       : compute cost function across current basis; do not reset total when done
// 4 = Local     : compute cost function across current basis only
{
  IPOINT3D Index_reg, iVoxelStart, iVoxelEnd;
  POINT3D Delta2;
  int DEF_save_nvoxels;
  float DEF_save_sum2;
  float cost, fAdd, d0, d1, d2, delta_plus, delta_minus;
  int index0, index1, index2, iX1, iX2, iY1, iY2, iZ1, iZ2;
  BOOL valid_plus, valid_minus;
  static int DEF_nvoxels;
  static float DEF_sum2;

  if ( InitProbeSet == 0 )
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
  if ( InitProbeSet == 0 || InitProbeSet == 4 ) // init
    {
      DEF_sum2    = 0.;
      DEF_nvoxels = 0;
    }
  DEF_save_sum2    = 0.;
  DEF_save_nvoxels = 0;

  Delta2.x = SQR(template.resolution.x);
  Delta2.y = SQR(template.resolution.y);
  Delta2.z = SQR(template.resolution.z);
  delta_plus = delta_minus = 0.;  // initialize to prevent passing nan to bending_penalty()

  for (Index_reg.z=iVoxelStart.z; Index_reg.z<iVoxelEnd.z; Index_reg.z++)
    {
      for (Index_reg.y=iVoxelStart.y; Index_reg.y<iVoxelEnd.y; Index_reg.y++)
	{
	  for (Index_reg.x=iVoxelStart.x; Index_reg.x<iVoxelEnd.x; Index_reg.x++)
	    {
	      // point to the index for the current point
	      index0 = index4d(Index_reg.x,Index_reg.y,Index_reg.z,0);
	      fAdd = 0.;

	      if ( AlignNL.Adjust.x )
		{
		  ///////////////////// X distortion field ////////////////////////////////
		  d0 = distort_x.SpaceTime.fData[index0];
		  // Compression/expansion: 2 points in same direction as distortion field
		  iX1 = Index_reg.x + 1;  // point +
		  iX2 = Index_reg.x - 1;  // point -
		  valid_plus  = (iX1 >=0 && iX1<template.dim.x);
		  valid_minus = (iX2 >=0 && iX2<template.dim.x);
		  if ( valid_plus )
		    { // penalize + direction
		      index1 = index4d(iX1,Index_reg.y,Index_reg.z,0);
		      d1 = distort_x.SpaceTime.fData[index1];
		      delta_plus = d1-d0;
		    }
		  if ( valid_minus )
		    { // penalize - direction
		      index2 = index4d(iX2,Index_reg.y,Index_reg.z,0);
		      d2 = distort_x.SpaceTime.fData[index2];
		      delta_minus = d0-d2;
		    }
		  fAdd += bending_penalty( valid_plus, valid_minus, delta_plus, delta_minus, Delta2.x );
		  // Shear: 4 points in directions orthogonal to the distortion field
		  iY1 = Index_reg.y + 1;  // point +
		  iY2 = Index_reg.y - 1;  // point -
		  valid_plus  = (iY1 >=0 && iY1<template.dim.y);
		  valid_minus = (iY2 >=0 && iY2<template.dim.y);
		  if ( valid_plus )
		    { // penalize + direction
		      index1 = index4d(Index_reg.x,iY1,Index_reg.z,0);
		      d1 = distort_x.SpaceTime.fData[index1];
		      delta_plus = d1-d0;
		    }
		  if ( valid_minus )
		    { // penalize - direction
		      index2 = index4d(Index_reg.x,iY2,Index_reg.z,0);
		      d2 = distort_x.SpaceTime.fData[index2];
		      delta_minus = d0-d2;
		    }
		  fAdd += bending_penalty( valid_plus, valid_minus, delta_plus, delta_minus, Delta2.y );
		  iZ1 = Index_reg.z + 1;  // point +
		  iZ2 = Index_reg.z - 1;  // point -
		  valid_plus  = (iZ1 >=0 && iZ1<template.dim.z);
		  valid_minus = (iZ2 >=0 && iZ2<template.dim.z);
		  if ( valid_plus )
		    { // penalize + direction
		      index1 = index4d(Index_reg.x,Index_reg.y,iZ1,0);
		      d1 = distort_x.SpaceTime.fData[index1];
		      delta_plus = d1-d0;
		    }
		  if ( valid_minus )
		    { // penalize - direction
		      index2 = index4d(Index_reg.x,Index_reg.y,iZ2,0);
		      d2 = distort_x.SpaceTime.fData[index2];
		      delta_minus = d0-d2;
		    }
		  fAdd += bending_penalty( valid_plus, valid_minus, delta_plus, delta_minus, Delta2.z );
		}

	      if ( AlignNL.Adjust.y )
		{
		  ///////////////////// Y distortion field ////////////////////////////////
		  d0 = distort_y.SpaceTime.fData[index0];
		  // Compression/expansion: 2 points in same direction as distortion field
		  iY1 = Index_reg.y + 1;  // point +
		  iY2 = Index_reg.y - 1;  // point -
		  valid_plus  = (iY1 >=0 && iY1<template.dim.y);
		  valid_minus = (iY2 >=0 && iY2<template.dim.y);
		  if ( valid_plus )
		    { // penalize + direction
		      index1 = index4d(Index_reg.x,iY1,Index_reg.z,0);
		      d1 = distort_y.SpaceTime.fData[index1];
		      delta_plus = d1-d0;
		    }
		  if ( valid_minus )
		    { // penalize - direction
		      index2 = index4d(Index_reg.x,iY2,Index_reg.z,0);
		      d2 = distort_y.SpaceTime.fData[index2];
		      delta_minus = d0-d2;
		    }
		  fAdd += bending_penalty( valid_plus, valid_minus, delta_plus, delta_minus, Delta2.y );
		  // Shear: 4 points in directions orthogonal to the distortion field
		  iX1 = Index_reg.x + 1;  // point +
		  iX2 = Index_reg.x - 1;  // point -
		  valid_plus  = (iX1 >=0 && iX1<template.dim.x);
		  valid_minus = (iX2 >=0 && iX2<template.dim.x);
		  if ( valid_plus )
		    { // penalize + direction
		      index1 = index4d(iX1,Index_reg.y,Index_reg.z,0);
		      d1 = distort_y.SpaceTime.fData[index1];
		      delta_plus = d1-d0;
		    }
		  if ( valid_minus )
		    { // penalize - direction
		      index2 = index4d(iX2,Index_reg.y,Index_reg.z,0);
		      d2 = distort_y.SpaceTime.fData[index2];
		      delta_minus = d0-d2;
		    }
		  fAdd += bending_penalty( valid_plus, valid_minus, delta_plus, delta_minus, Delta2.x );
		  iZ1 = Index_reg.z + 1;  // point +
		  iZ2 = Index_reg.z - 1;  // point -
		  valid_plus  = (iZ1 >=0 && iZ1<template.dim.z);
		  valid_minus = (iZ2 >=0 && iZ2<template.dim.z);
		  if ( valid_plus )
		    { // penalize + direction
		      index1 = index4d(Index_reg.x,Index_reg.y,iZ1,0);
		      d1 = distort_y.SpaceTime.fData[index1];
		      delta_plus = d1-d0;
		    }
		  if ( valid_minus )
		    { // penalize - direction
		      index2 = index4d(Index_reg.x,Index_reg.y,iZ2,0);
		      d2 = distort_y.SpaceTime.fData[index2];
		      delta_minus = d0-d2;
		    }
		  fAdd += bending_penalty( valid_plus, valid_minus, delta_plus, delta_minus, Delta2.z );
		}

	      if ( AlignNL.Adjust.z )
		{
		  ///////////////////// Z distortion field ////////////////////////////////
		  d0 = distort_z.SpaceTime.fData[index0];
		  // Compression/expansion: 2 points in same direction as distortion field
		  iZ1 = Index_reg.z + 1;  // point +
		  iZ2 = Index_reg.z - 1;  // point -
		  valid_plus  = (iZ1 >=0 && iZ1<template.dim.z);
		  valid_minus  = (iZ2 >=0 && iZ2<template.dim.z);
		  if ( valid_plus )
		    { // penalize + direction
		      index1 = index4d(Index_reg.x,Index_reg.y,iZ1,0);
		      d1 = distort_z.SpaceTime.fData[index1];
		      delta_plus = d1-d0;
		    }
		  if ( valid_minus )
		    { // penalize - direction
		      index2 = index4d(Index_reg.x,Index_reg.y,iZ2,0);
		      d2 = distort_z.SpaceTime.fData[index2];
		      delta_minus = d0-d2;
		    }
		  fAdd += bending_penalty( valid_plus, valid_minus, delta_plus, delta_minus, Delta2.z );
		  // Shear: 4 points in directions orthogonal to the distortion field
		  iX1 = Index_reg.x + 1;  // point +
		  iX2 = Index_reg.x - 1;  // point -
		  valid_plus  = (iX1 >=0 && iX1<template.dim.x);
		  valid_minus = (iX2 >=0 && iX2<template.dim.x);
		  if ( valid_plus )
		    { // penalize + direction
		      index1 = index4d(iX1,Index_reg.y,Index_reg.z,0);
		      d1 = distort_z.SpaceTime.fData[index1];
		      delta_plus = d1-d0;
		    }
		  if ( valid_minus )
		    { // penalize - direction
		      index2 = index4d(iX2,Index_reg.y,Index_reg.z,0);
		      d2 = distort_z.SpaceTime.fData[index2];
		      delta_plus = d0-d2;
		    }
		  fAdd += bending_penalty( valid_plus, valid_minus, delta_plus, delta_minus, Delta2.x );
		  iY1 = Index_reg.y + 1;  // point +
		  iY2 = Index_reg.y - 1;  // point -
		  valid_plus  = (iY1 >=0 && iY1<template.dim.y);
		  valid_minus = (iY2 >=0 && iY2<template.dim.y);
		  if ( valid_plus )
		    { // penalize + direction
		      index1 = index4d(Index_reg.x,iY1,Index_reg.z,0);
		      d1 = distort_z.SpaceTime.fData[index1];
		      delta_plus = d1-d0;
		    }
		  if ( valid_minus )
		    { // penalize - direction
		      index2 = index4d(Index_reg.x,iY2,Index_reg.z,0);
		      d2 = distort_z.SpaceTime.fData[index2];
		      delta_minus = d1-d0;
		    }
		  fAdd += bending_penalty( valid_plus, valid_minus, delta_plus, delta_minus, Delta2.y );
		}

	      if ( InitProbeSet != 1 ) // ProbeInit
		{ // update total EXCEPT for "Probe Init"
		  DEF_sum2 += fAdd;
		  DEF_nvoxels++;
		}
	      if ( InitProbeSet == 1 || InitProbeSet == 2 ) // ProbeInit or Probe
		{ // These are temporary values
		  DEF_save_sum2 += fAdd;
		  DEF_save_nvoxels++;
		}
	    }
	}
    }
  cost = DEF_sum2;
  if ( normalize )
    {
      if ( DEF_nvoxels != 0 )
	cost = DEF_sum2/(float)DEF_nvoxels;
      else
	cost = 1.e10;
    }

  if ( InitProbeSet == 1 || InitProbeSet == 2 ) // ProbeInit or Probe
    { // Reset the voxel count and sum2, as this is only a temporary computation of MI
      DEF_sum2    -= DEF_save_sum2;
      DEF_nvoxels -= DEF_save_nvoxels;
    }

  return(cost);
}

float bending_penalty( BOOL valid_plus, BOOL valid_minus, float delta_plus, float delta_minus, float res2 )
{
  float deriv1_2, deriv2_2, denom, penalty;
  float fraction_smooth, fraction_stiff;

  fraction_stiff  = AlignNL.stiffness;
  fraction_smooth = 1. - fraction_stiff;

  penalty = 0.;
  if ( valid_plus )  // Add a term that depends upon the 1st derivative with the plus neighbor
    penalty += fraction_stiff/2. * SQR( SQR(delta_plus)/res2 );  // 4th power in delta/resolution
  if ( valid_minus ) // Add a term that depends upon the 1st derivative with the minus neighbor
    penalty += fraction_stiff/2. * SQR( SQR(delta_minus)/res2 );
  if ( valid_plus && valid_minus )
    { // then there is a 2nd derivative
      deriv1_2 = SQR( (delta_plus+delta_minus)/2. );
      deriv2_2 = SQR( delta_plus-delta_minus );
      denom  = deriv1_2 + res2;
      denom *= SQR(denom);  // now cubed, or 6th power in derivative and resolution
      // The following is basically the bending energy:
      // BE = SQR( curvature ) = SQR( deriv2 * res / [res^2 + deriv1^2]^3/2 )
      penalty += fraction_smooth * deriv2_2 * SQR(res2) / denom;  // dimensionless BE
    }
  return(penalty);
}

int number_invalid_distortions( int all_voxels, int ilevel, int ipass )
{
  POINT3D d0, d1, res;
  IPOINT3D Index0, iVoxelStart, iVoxelEnd;
  float jacob[3][3], determ, min_determ, max_determ;
  int indexT, index0, index1;
  int nbad;
  BOOL valid;
  static BOOL first_time=TRUE;

  res  = template.resolution;
  nbad = 0;
  min_determ =  1.e10;
  max_determ = -1.e10;

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

  for (Index0.z=iVoxelStart.z; Index0.z<iVoxelEnd.z; Index0.z++)
    {
      for (Index0.y=iVoxelStart.y; Index0.y<iVoxelEnd.y; Index0.y++)
	{
	  for (Index0.x=iVoxelStart.x; Index0.x<iVoxelEnd.x; Index0.x++)
	    {
	      // point to the index for the current point (template or individual)
	      indexT = index4d_stack(Index0.x,Index0.y,Index0.z,iTime_Template,&template);
	      index0 = index4d(Index0.x,Index0.y,Index0.z,0);
	      if ( template.SpaceTime.fData[indexT] != 0. )
		{ // only check points with non-zero template data
		  // d0 is the 3D distortion field for the point of interest
		  d0.x = distort_x.SpaceTime.fData[index0];
		  d0.y = distort_y.SpaceTime.fData[index0];
		  d0.z = distort_z.SpaceTime.fData[index0];
		  // Form derivatives using the point at (i,j,k)=(i0+1,j0+1,k0+1)
		  valid = Index0.x+1 < template.dim.x
		    &&    Index0.y+1 < template.dim.y 
		    &&    Index0.z+1 < template.dim.z ;
		  if ( valid )
		    {
		      // x
		      index1 = index4d(Index0.x+1,Index0.y,Index0.z,0); // dx
		      d1.x = distort_x.SpaceTime.fData[index1] + res.x;
		      d1.y = distort_y.SpaceTime.fData[index1];
		      d1.z = distort_z.SpaceTime.fData[index1];
		      jacob[0][0] = (d1.x-d0.x)/res.x; // dx1/dx
		      jacob[1][0] = (d1.y-d0.y)/res.x; // dy1/dx
		      jacob[2][0] = (d1.z-d0.z)/res.x; // dz1/dx
		      // y
		      index1 = index4d(Index0.x,Index0.y+1,Index0.z,0); // dy
		      d1.x = distort_x.SpaceTime.fData[index1];
		      d1.y = distort_y.SpaceTime.fData[index1] + res.y;
		      d1.z = distort_z.SpaceTime.fData[index1];
		      jacob[0][1] = (d1.x-d0.x)/res.y; // dx1/dy
		      jacob[1][1] = (d1.y-d0.y)/res.y; // dy1/dy
		      jacob[2][1] = (d1.z-d0.z)/res.y; // dz1/dy
		      // z
		      index1 = index4d(Index0.x,Index0.y,Index0.z+1,0); // dz
		      d1.x = distort_x.SpaceTime.fData[index1];
		      d1.y = distort_y.SpaceTime.fData[index1];
		      d1.z = distort_z.SpaceTime.fData[index1] + res.z;
		      jacob[0][2] = (d1.x-d0.x)/res.z; // dx1/dz
		      jacob[1][2] = (d1.y-d0.y)/res.z; // dy1/dz
		      jacob[2][2] = (d1.z-d0.z)/res.z; // dz1/dz
		      // calculate the determinant
		      determ = jacob[0][0] * jacob[1][1] * jacob[2][2]
			+      jacob[0][1] * jacob[1][2] * jacob[2][0]
			+      jacob[0][2] * jacob[1][0] * jacob[2][1]
			-      jacob[2][0] * jacob[1][1] * jacob[0][2]
			-      jacob[2][1] * jacob[1][2] * jacob[0][0]
			-      jacob[2][2] * jacob[1][0] * jacob[0][1];
		      if ( determ < min_determ ) min_determ = determ;
		      if ( determ > max_determ ) max_determ = determ;
		      if ( determ < 1./(float)AlignNL.Jacob_limits ||  // maximum compression
			   determ >    (float)AlignNL.Jacob_limits )   // maximum expansion
			nbad++;
		    }
		}
	    }
	}
    }

  if ( first_time )
    printf("\nresolution: %g %g %g\n",template.resolution.x,template.resolution.y,template.resolution.z);

  if ( first_time || ipass != 0 )
    {
      printf("min & max determinant : %.2f %.2f    (# bad = %d)\n",min_determ,max_determ,nbad);
      if ( ipass >= 0 )
	printf("MI = %.4f / %.4f for level %d and pass %d\n",
	       cost_function(AlignAid.CostFunction),
	       cost_function_nonlinear(AlignAid.CostFunction,0),ilevel,ipass);
      else
	printf("MI = %.4f / %.4f on final pass for level %d\n",
	       cost_function(AlignAid.CostFunction),
	       cost_function_nonlinear(AlignAid.CostFunction,0),ilevel);
      printf("\n");
    }
  first_time = FALSE;
  return(nbad);
}

float jacobian_determinant(POINT3D d0, POINT3D d1, POINT3D res)
{
  float jacob[3][3], determ;

  // x
  jacob[0][0] = (d1.x-d0.x)/res.x; // dx/dx
  jacob[1][0] = (d1.y-d0.y)/res.x; // dy/dx
  jacob[2][0] = (d1.z-d0.z)/res.x; // dz/dx
  // y
  jacob[0][1] = (d1.x-d0.x/res.y); // dx/dy
  jacob[1][1] = (d1.y-d0.y/res.y); // dy/dy
  jacob[2][1] = (d1.z-d0.z/res.y); // dz/dy
  // z
  jacob[0][2] = (d1.x-d0.x)/res.z; // dx/dz
  jacob[1][2] = (d1.y-d0.y)/res.z; // dy/dz
  jacob[2][2] = (d1.z-d0.z)/res.z; // dz/dz
  // add 1 to all jacobian terms to account for resolution step.
  jacob[0][0] += 1.;
  jacob[1][1] += 1.;
  jacob[2][2] += 1.;
  // calculate the determinant
  determ = jacob[0][0] * jacob[1][1] * jacob[2][2]
    +      jacob[0][1] * jacob[1][2] * jacob[2][0]
    +      jacob[0][2] * jacob[1][0] * jacob[2][1]
    -      jacob[2][0] * jacob[1][1] * jacob[0][2]
    -      jacob[2][1] * jacob[1][2] * jacob[0][0]
    -      jacob[2][2] * jacob[1][0] * jacob[0][1];
  return(determ);
}
 
void calculate_volume_averages_nonlinear( int all_bases, float *av_individual, float *av_template,
					  float *min, float *max)
{
  float value_individual, value_template;
  int jx, jy, jz, indexT, indexI, number_voxels;
  IPOINT3D Index_reg, iVoxelStart, iVoxelEnd;
  BOOL non_zero;

  if ( all_bases )
    {
      iVoxelStart.x = iVoxelStart.y = iVoxelStart.z = 0;
      iVoxelEnd = template.dim;
    }
  else
    {
      iVoxelStart = AlignNL.Index3DStart[AlignNL.iBasis];
      iVoxelEnd   = AlignNL.Index3DEnd[AlignNL.iBasis];
    }

  // Find normalization factors.
  number_voxels = 0;
  *av_individual = *av_template = 0.;
  for (jz=iVoxelStart.z; jz<iVoxelEnd.z; jz++)
    {
      for (jy=iVoxelStart.y; jy<iVoxelEnd.y; jy++)
	{
	  for (jx=iVoxelStart.x; jx<iVoxelEnd.x; jx++)
	    {
	      indexT = index4d_stack(jx,jy,jz,iTime_Template,&template);
	      indexI = index4d_stack(jx,jy,jz,graph.iTime,&individual);
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
  
  if ( *av_individual == 0. ) *av_individual = 1.;
  // Find the min and max.
  // Loop over voxels in the standard resolution space.
  *min = 1.e10;  *max = 0.;
  for (Index_reg.z=iVoxelStart.z; Index_reg.z<iVoxelEnd.z; Index_reg.z++)
    {
      for (Index_reg.y=iVoxelStart.y; Index_reg.y<iVoxelEnd.y; Index_reg.y++)
	{
	  for (Index_reg.x=iVoxelStart.x; Index_reg.x<iVoxelEnd.x; Index_reg.x++)
	    {
	      indexT = index4d_stack(Index_reg.x,Index_reg.y,Index_reg.z,iTime_Template,&template);
	      indexI = index4d_stack(Index_reg.x,Index_reg.y,Index_reg.z,graph.iTime,&individual);
	      non_zero = template.SpaceTime.fData[indexT] != 0.;
	      if ( non_zero )
		{
		  value_individual = individual.SpaceTime.fData[indexI]/(*av_individual);
		  value_template   = template.SpaceTime.fData[indexT]/(*av_template);
		  if ( value_individual < *min ) *min = value_individual;
		  if ( value_template   < *min ) *min = value_template;
		  if ( value_individual > *max ) *max = value_individual;
		  if ( value_template   > *max ) *max = value_template;
		}
	    }
	}
    }
}

void adjust_the_adjust_flags( int icost )
{
  float cost_def, cost_total, average_cost, percent_ignored, percent_reinstated;
  int lIgnored, lReinstated, lTotal, lDefFactor;
  int j;

  lDefFactor = 0;
  if ( AlignNL.Adjust.x ) lDefFactor++;
  if ( AlignNL.Adjust.y ) lDefFactor++;
  if ( AlignNL.Adjust.z ) lDefFactor++;

  cost_total   = cost_deformation(0,0);  // total deformation cost
  // For 3D adjustment, the average deformation cost should be about 8 * total,
  // due to 2-fold overlap in each dim.
  average_cost = cost_total/(float)(AlignNL.NBasis)*pow(2.,(double)lDefFactor);

  lIgnored = lReinstated = 0;
  for (j=0; j<AlignNL.NBasis; j++) // loop over x coefficients
    { // j is both basis and coefficient counter
      if ( (AlignNL.Adjust.x && !AlignNL.Adjust3D[j]) ||
	   (AlignNL.Adjust.y && !AlignNL.Adjust3D[j+AlignNL.NBasis]) ||
	   (AlignNL.Adjust.z && !AlignNL.Adjust3D[j+2*AlignNL.NBasis]) )
	{
	  if ( AlignNL.Adjust.x && !AlignNL.Adjust3D[j] )
	    lIgnored++;
	  if ( AlignNL.Adjust.y && !AlignNL.Adjust3D[j+AlignNL.NBasis] )
	    lIgnored++;
	  if ( AlignNL.Adjust.z && !AlignNL.Adjust3D[j+2*AlignNL.NBasis])
	    lIgnored++;
	  // Create an inclusion criteria based upon the deformation energy in this basis function.
	  AlignNL.iBasis = j;
	  cost_def = cost_deformation(4,0); // cost in current basis function
	  if ( cost_def > 4. * average_cost || number_invalid_distortions(0,0,0) )
	    {
	      if ( AlignNL.Adjust.x )
		{
		  AlignNL.Adjust3D[j] = TRUE;
		  lReinstated++;
		}
	      if ( AlignNL.Adjust.y )
		{
		  AlignNL.Adjust3D[j+AlignNL.NBasis] = TRUE;
		  lReinstated++;
		}
	      if ( AlignNL.Adjust.z )
		{
		  AlignNL.Adjust3D[j+2*AlignNL.NBasis] = TRUE;
		  lReinstated++;
		}
	    }
	}
    }
  lTotal = 0;
  if ( AlignNL.Adjust.x )
    lTotal += AlignNL.NBasis;
  if ( AlignNL.Adjust.y )
    lTotal += AlignNL.NBasis;
  if ( AlignNL.Adjust.z )
    lTotal += AlignNL.NBasis;
  percent_ignored    = 100. * (float)lIgnored/(float)lTotal;
  percent_reinstated = 100. * (float)lReinstated/(float)lTotal;
  printf("Ignored    due to small change        : %.1f %c\n",percent_ignored,37);
  printf("Reinstated due to deformation tension : %.1f %c\n",percent_reinstated,37);
}

void Set_AlignNL_Value(int iCoeff, float value)
{
  float *data, basis;
  int iBasis, index0;
  IPOINT3D Index_reg;

  if ( isnan(value) )
    {
      printf("Programming error: setting nan for coefficient %d\n",iCoeff);
      exit(1);
    }

  iBasis = AlignNL.iBasis;
  // Select the subfield (x,y,z)
  if ( AlignNL.iCoeff < AlignNL.NBasis )
    data = distort_x.SpaceTime.fData;
  else if ( AlignNL.iCoeff < 2*AlignNL.NBasis )
    data = distort_y.SpaceTime.fData;
  else
    data = distort_z.SpaceTime.fData;

  // First subtract the old sub-field from the relevant (x,y,z) distortion field
  for (Index_reg.z=AlignNL.Index3DStart[iBasis].z;
       Index_reg.z<AlignNL.Index3DEnd[iBasis].z; Index_reg.z++)
    {
      for (Index_reg.y=AlignNL.Index3DStart[iBasis].y;
	   Index_reg.y<AlignNL.Index3DEnd[iBasis].y; Index_reg.y++)
	{
	  for (Index_reg.x=AlignNL.Index3DStart[iBasis].x;
	       Index_reg.x<AlignNL.Index3DEnd[iBasis].x; Index_reg.x++)
	    {
	      index0 = index4d(Index_reg.x,Index_reg.y,Index_reg.z,0);
	      basis = NL_Basis_function(iBasis,Index_reg);
	      data[index0] -= AlignNL.value[iCoeff] * basis;
	      data[index0] += value * basis;
	    } // x
	} // y
    } // z
  // Set the value.
  AlignNL.value[iCoeff] = value;
  transform_coordinates(0,0);
}

void update_nonlinear_alignment_parameters()
// This function changes non-linear basis functions at CONSTANT DISTORTION FIELD RESOLUTION.
// Dimensions of distortion fields MUST EQUAL template dimensions when this function called.
{
  int *PeakX, *PeakY, *PeakZ, *StartX, *StartY, *StartZ, *EndX, *EndY, *EndZ;
  int jx, jy, jz, j, iBasis, index_map, jBasis;
  //  int ix, iy, iz;
  float basis;
  IPOINT3D Index_reg;
  static IPOINT3D TemplateDim;
  static BOOL first_time=TRUE;

  if ( template.dim.x != distort_x.dim.x ||
       template.dim.y != distort_x.dim.y ||
       template.dim.z != distort_x.dim.z )
    {
      printf("Programming error: distortion field dimensions != template dimensions\n");
      exit(1);
    }

  if ( !first_time )
    {
      free_ipoint3d_vector(AlignNL.Index3DStart);
      free_ipoint3d_vector(AlignNL.Index3DEnd);
      free_ipoint3d_vector(AlignNL.iBasis1D);
      free_float_vector(AlignNL.value);
      free_float_vector(AlignNL.increment);
      free_int_vector(AlignNL.Adjust3D);
      free_int_vector(AlignNL.AdjustOrder);
      free_float_matrix(AlignNL.BasisX,AlignNL.NBasis1D.x,TemplateDim.x);
      free_float_matrix(AlignNL.BasisY,AlignNL.NBasis1D.y,TemplateDim.y);
      free_float_matrix(AlignNL.BasisZ,AlignNL.NBasis1D.z,TemplateDim.z);
    }
  TemplateDim = template.dim;
  first_time  = FALSE;

  // Calculate the number of basis functions.
  AlignNL.NBasis1D.x = number_NL_basis_functions(template.dim.x,AlignNL.GridDown.x);
  AlignNL.NBasis1D.y = number_NL_basis_functions(template.dim.y,AlignNL.GridDown.y);
  AlignNL.NBasis1D.z = number_NL_basis_functions(template.dim.z,AlignNL.GridDown.z);
  AlignNL.NBasis     = AlignNL.NBasis1D.x * AlignNL.NBasis1D.y * AlignNL.NBasis1D.z;

  // Allocate space for the start and end indices.
  AlignNL.Index3DStart = ipoint3d_vector(AlignNL.NBasis);
  AlignNL.Index3DEnd   = ipoint3d_vector(AlignNL.NBasis);
  AlignNL.iBasis1D     = ipoint3d_vector(AlignNL.NBasis);
  AlignNL.value        =  float_vector(3*AlignNL.NBasis);
  AlignNL.increment    =  float_vector(3*AlignNL.NBasis);
  AlignNL.Adjust3D     =    int_vector(3*AlignNL.NBasis);
  AlignNL.AdjustOrder  =    int_vector(3*AlignNL.NBasis);
  AlignNL.BasisX       = float_matrix(AlignNL.NBasis1D.x,template.dim.x);
  AlignNL.BasisY       = float_matrix(AlignNL.NBasis1D.y,template.dim.y);
  AlignNL.BasisZ       = float_matrix(AlignNL.NBasis1D.z,template.dim.z);
  
  // Define the start/stop/peak locations.
  PeakX  = int_vector(AlignNL.NBasis1D.x);
  StartX = int_vector(AlignNL.NBasis1D.x);
  EndX   = int_vector(AlignNL.NBasis1D.x);
  PeakY  = int_vector(AlignNL.NBasis1D.y);
  StartY = int_vector(AlignNL.NBasis1D.y);
  EndY   = int_vector(AlignNL.NBasis1D.y);
  PeakZ  = int_vector(AlignNL.NBasis1D.z);
  StartZ = int_vector(AlignNL.NBasis1D.z);
  EndZ   = int_vector(AlignNL.NBasis1D.z);

  calc_NL_basis_function_locations(AlignNL.NBasis1D.x,template.dim.x,AlignNL.GridDown.x,
				   PeakX,StartX,EndX);
  calc_NL_basis_function_locations(AlignNL.NBasis1D.y,template.dim.y,AlignNL.GridDown.y,
				   PeakY,StartY,EndY);
  calc_NL_basis_function_locations(AlignNL.NBasis1D.z,template.dim.z,AlignNL.GridDown.z,
				   PeakZ,StartZ,EndZ);

  // Define the basis functions
  define_basis_functions(0); // x
  define_basis_functions(1); // y
  define_basis_functions(2); // z
  
  // Fill values
  for (jz=0; jz<AlignNL.NBasis1D.z; jz++)
    {
      for (jy=0; jy<AlignNL.NBasis1D.y; jy++)
	{
	  for (jx=0; jx<AlignNL.NBasis1D.x; jx++)
	    {
	      iBasis = jz * AlignNL.NBasis1D.x * AlignNL.NBasis1D.y
		+      jy * AlignNL.NBasis1D.x
		+      jx;

	      AlignNL.iBasis1D[iBasis].x = jx;
	      AlignNL.iBasis1D[iBasis].y = jy;
	      AlignNL.iBasis1D[iBasis].z = jz;
	
	      // FWHM = AlignNL.GridDown * template.resolution
	      AlignNL.increment[iBasis]                  = AlignNL.GridDown.x*template.resolution.x/2.;
	      AlignNL.increment[iBasis+AlignNL.NBasis]   = AlignNL.GridDown.y*template.resolution.y/2.;
	      AlignNL.increment[iBasis+AlignNL.NBasis*2] = AlignNL.GridDown.z*template.resolution.z/2.;
	      // Set all values ON at beginning	      
	      AlignNL.Adjust3D[iBasis]                  = 1;
	      AlignNL.Adjust3D[iBasis+AlignNL.NBasis]   = 1;
	      AlignNL.Adjust3D[iBasis+AlignNL.NBasis*2] = 1;
	      // Set a default adjust order to be sequential.
	      AlignNL.AdjustOrder[iBasis]                  = iBasis;
	      AlignNL.AdjustOrder[iBasis+AlignNL.NBasis]   = iBasis+AlignNL.NBasis;
	      AlignNL.AdjustOrder[iBasis+AlignNL.NBasis*2] = iBasis+AlignNL.NBasis*2;

	      // The following arrays are indices into the basis functions
	      AlignNL.Index3DStart[iBasis].x = StartX[jx];
	      AlignNL.Index3DStart[iBasis].y = StartY[jy];
	      AlignNL.Index3DStart[iBasis].z = StartZ[jz];
	      AlignNL.Index3DEnd[iBasis].x   = EndX[jx]+1; // add 1 to all ends
	      AlignNL.Index3DEnd[iBasis].y   = EndY[jy]+1; // so that we can treat these
	      AlignNL.Index3DEnd[iBasis].z   = EndZ[jz]+1; // like template.dim in loops
	      // Read the values straight off the distortion maps.
	      index_map = PeakZ[jz] * template.dim.x * template.dim.y
		+         PeakY[jy] * template.dim.x
		+         PeakX[jx];
	      AlignNL.value[iBasis]                  = distort_x.SpaceTime.fData[index_map];
	      AlignNL.value[iBasis+AlignNL.NBasis]   = distort_y.SpaceTime.fData[index_map];
	      AlignNL.value[iBasis+AlignNL.NBasis*2] = distort_z.SpaceTime.fData[index_map];
	    }
	}
    }
  // The adjustment order doesn't seem to matter much.
  // Centric adjustment order
  /*
  for (jz=0; jz<AlignNL.NBasis1D.z; jz++)
    {
      for (jy=0; jy<AlignNL.NBasis1D.y; jy++)
	{
	  for (jx=0; jx<AlignNL.NBasis1D.x; jx++)
	    {
	      jBasis = jz * AlignNL.NBasis1D.x * AlignNL.NBasis1D.y
		+      jy * AlignNL.NBasis1D.x
		+      jx;

	      ix = jx + AlignNL.NBasis1D.x/2;
	      if ( ix >= AlignNL.NBasis1D.x )
		ix = AlignNL.NBasis1D.x/2 - (ix - AlignNL.NBasis1D.x + 1);

	      iy = jy + AlignNL.NBasis1D.y/2;
	      if ( iy >= AlignNL.NBasis1D.y )
		iy = AlignNL.NBasis1D.y/2 - (iy - AlignNL.NBasis1D.y + 1);

	      iz = jz + AlignNL.NBasis1D.z/2;
	      if ( iz >= AlignNL.NBasis1D.z )
		iz = AlignNL.NBasis1D.z/2 - (iz - AlignNL.NBasis1D.z + 1);

	      iBasis = iz * AlignNL.NBasis1D.x * AlignNL.NBasis1D.y
		+      iy * AlignNL.NBasis1D.x
		+      ix;

	      AlignNL.AdjustOrder[jBasis]                  = iBasis;
	      AlignNL.AdjustOrder[jBasis+AlignNL.NBasis]   = iBasis+AlignNL.NBasis;
	      AlignNL.AdjustOrder[jBasis+AlignNL.NBasis*2] = iBasis+AlignNL.NBasis*2;
	      //	      printf("(%d,%d,%d) = %d --> (%d,%d,%d) = %d\n",jx,jy,jz,jBasis,ix,iy,iz,iBasis);
	    }
	}
    }
  */

  free_int_vector(PeakX);
  free_int_vector(PeakY);
  free_int_vector(PeakZ);
  free_int_vector(StartX);
  free_int_vector(StartY);
  free_int_vector(StartZ);
  free_int_vector(EndX);
  free_int_vector(EndY);
  free_int_vector(EndZ);

  // update the distortion maps
  for (j=0; j<distort_x.n3d; j++)
    distort_x.SpaceTime.fData[j] = distort_y.SpaceTime.fData[j] = distort_z.SpaceTime.fData[j] = 0.;

  for (jBasis=0; jBasis<AlignNL.NBasis; jBasis++)
    {
      for (Index_reg.z=AlignNL.Index3DStart[jBasis].z;
	   Index_reg.z<AlignNL.Index3DEnd[jBasis].z; Index_reg.z++)
	{
	  for (Index_reg.y=AlignNL.Index3DStart[jBasis].y;
	       Index_reg.y<AlignNL.Index3DEnd[jBasis].y; Index_reg.y++)
	    {
	      for (Index_reg.x=AlignNL.Index3DStart[jBasis].x;
		   Index_reg.x<AlignNL.Index3DEnd[jBasis].x; Index_reg.x++)
		{
		  index_map = index4d_stack(Index_reg.x,Index_reg.y,Index_reg.z,0,&distort_x);
		  basis = NL_Basis_function(jBasis,Index_reg);
		  distort_x.SpaceTime.fData[index_map] += AlignNL.value[jBasis] * basis;
		  distort_y.SpaceTime.fData[index_map] += AlignNL.value[jBasis+AlignNL.NBasis]   * basis;
		  distort_z.SpaceTime.fData[index_map] += AlignNL.value[jBasis+AlignNL.NBasis*2] * basis;
		} // x
	    } // y
	} // z
    }

  if ( AlignAid.DisplayType > 1 )
    update_image_and_graph(1);
}

int number_NL_basis_functions( int nvoxels, int iDown )
// nvoxels = dimension of template in x, y, or z
// iDown   = down-sampling factor in the respective dimension
{
  int NBasis;

  NBasis = nvoxels;
  if ( iDown > 1 )
    NBasis = NBasis/2 + 1;
  if ( iDown > 2 )
    NBasis = NBasis/2 + 1;
  if ( iDown > 4 )
    NBasis = NBasis/2 + 1;
  if ( iDown > 8 )
    NBasis = NBasis/2 + 1;

  return(NBasis);
}

void calc_NL_basis_function_locations( int nBasis, int nvoxels, int iDown,
				       int *Peak, int *Start, int *End)
{
  int iBasis, iStart;

  // Even Basis functions
  iBasis = 0;
  Start[iBasis] = Peak[iBasis] = 0;
  End[iBasis]   = Peak[iBasis] + iDown;
  iStart = iDown;
  iBasis = 2;
  while ( iStart < nvoxels )
    {
      if ( iBasis < nBasis )
	{
	  Start[iBasis] = iStart + 1; // iStart value = 0
	  Peak[iBasis]  = MIN(nvoxels-1,iStart+iDown);
	  End[iBasis]   = MIN(nvoxels-1,iStart+iDown*2);
	}
      iBasis += 2;
      iStart += 2*iDown;
    }

  // Odd Basis functions
  iBasis = 1;
  iStart = 0;
  while ( iStart < nvoxels )
    {
      if ( iBasis < nBasis )
	{
	  Start[iBasis] = iStart + 1;  // iStart value = 0
	  Peak[iBasis]  = MIN(nvoxels-1,iStart+iDown);
	  End[iBasis]   = MIN(nvoxels-1,iStart+iDown*2);
	}
      iBasis += 2;
      iStart += 2*iDown;
    }
}

void create_basis_function_image()
{
  int index, iBasis;
  IPOINT3D Index_reg;
  BOOL included;

  iBasis = AlignNL.iBasis;
  // Fill in the basis function
  for (Index_reg.z=0; Index_reg.z<template.dim.z; Index_reg.z++)
    {
      for (Index_reg.y=0; Index_reg.y<template.dim.y; Index_reg.y++)
	{
	  for (Index_reg.x=0; Index_reg.x<template.dim.x; Index_reg.x++)
	    {
	      included  = (Index_reg.z>=AlignNL.Index3DStart[iBasis].z &&
			   Index_reg.z< AlignNL.Index3DEnd[iBasis].z);
	      included &= (Index_reg.y>=AlignNL.Index3DStart[iBasis].y &&
			   Index_reg.y< AlignNL.Index3DEnd[iBasis].y);
	      included &= (Index_reg.x>=AlignNL.Index3DStart[iBasis].x &&
			   Index_reg.x< AlignNL.Index3DEnd[iBasis].x);
	      index = index4d_stack(Index_reg.x,Index_reg.y,Index_reg.z,0,&NL_bases);
	      if ( included )
		NL_bases.SpaceTime.fData[index] = NL_Basis_function(iBasis,Index_reg);
	      else
		NL_bases.SpaceTime.fData[index] = 0.;
	    }
	}
    }
}

float NL_Basis_function(int iBasis, IPOINT3D iVoxel)
{
  POINT3D Basis1D;
  float Basis;

  if ( iVoxel.x < AlignNL.Index3DStart[iBasis].x || iVoxel.x >= AlignNL.Index3DEnd[iBasis].x ||
       iVoxel.y < AlignNL.Index3DStart[iBasis].y || iVoxel.y >= AlignNL.Index3DEnd[iBasis].y ||
       iVoxel.z < AlignNL.Index3DStart[iBasis].z || iVoxel.z >= AlignNL.Index3DEnd[iBasis].z )
    return(0.);

  Basis1D.x = AlignNL.BasisX[AlignNL.iBasis1D[iBasis].x][iVoxel.x];
  Basis1D.y = AlignNL.BasisY[AlignNL.iBasis1D[iBasis].y][iVoxel.y];
  Basis1D.z = AlignNL.BasisZ[AlignNL.iBasis1D[iBasis].z][iVoxel.z];
  Basis     = Basis1D.x * Basis1D.y * Basis1D.z;

  return(Basis);
}

void define_basis_functions( int which )
// which: 0 = x, 1 = y, 2 = z
{
  int nBasis, nSpace, iDown, nBasis1, nBasis2, nBasis4, nBasis8;
  int jBasis, jSpace;
  float **mat, **mat1_bs, **mat2_bs, **mat4_bs, **mat8_bs, **mix_lh;

  if ( which == 0 )
    {
      mat     = AlignNL.BasisX;
      nBasis  = AlignNL.NBasis1D.x;
      nSpace  = template.dim.x;
      iDown   = AlignNL.GridDown.x;
    }
  else if ( which == 1 )
    {
      mat    = AlignNL.BasisY;
      nBasis = AlignNL.NBasis1D.y;
      nSpace = template.dim.y;
      iDown  = AlignNL.GridDown.y;
    }
  else if ( which == 2 )
    {
      mat    = AlignNL.BasisZ;
      nBasis = AlignNL.NBasis1D.z;
      nSpace = template.dim.z;
      iDown  = AlignNL.GridDown.z;
    }

  ////////////////// Level-1 Basis Functions /////////////////////
  nBasis1 = number_NL_basis_functions(nSpace,1);
  // Define the level-1 matrix with no down-sampling.
  mat1_bs = float_matrix(nBasis1,nSpace);
  for (jBasis=0; jBasis<nBasis1; jBasis++)
    {
      for (jSpace=0; jSpace<nSpace; jSpace++)
	{
	  if ( jBasis == jSpace )
	    mat1_bs[jBasis][jSpace] = 1.;
	  else
	    mat1_bs[jBasis][jSpace] = 0.;
	}
    }
  if ( iDown == 1 )
    {
      if ( nBasis != nBasis1 )
	{
	  printf("Programming error in define_basis_functions():\n");
	  printf("# of basis functions (%d) != # of level-1 basis functions (%d)\n",nBasis,nBasis1);
	  exit(1);
	}
      for (jBasis=0; jBasis<nBasis; jBasis++)
	{
	  for (jSpace=0; jSpace<nSpace; jSpace++)
	    mat[jBasis][jSpace] = mat1_bs[jBasis][jSpace];
	}
      free_float_matrix(mat1_bs,nBasis,nSpace);
      return;
    }

  ////////////////// Level-2 Basis Functions /////////////////////
  nBasis2 = number_NL_basis_functions(nSpace,2);
  // Define the mixing matrix from level 1 to level 2.
  mix_lh = float_matrix(nBasis2,nBasis1);   // mix_lh = mix_lowDim_highDim
  compute_mixing_matrix(nBasis2,nBasis1,mix_lh);
  // Compute the level-2 matrix (2x down-sampling): mat2_bs = mix_lh * mat1_bs
  mat2_bs = float_matrix(nBasis2,nSpace);
  //               X1*X2    L1       L2       X1    t   L12      X2      t
  Matrix_Product( mat2_bs, nBasis2, nSpace, mix_lh, 0, nBasis1, mat1_bs, 0);
  free_float_matrix(mat1_bs,nBasis1,nSpace);
  free_float_matrix(mix_lh,nBasis2,nBasis1);
  if ( iDown == 2 )
    {
      if ( nBasis != nBasis2 )
	{
	  printf("Programming error in define_basis_functions():\n");
	  printf("# of basis functions (%d) != # of level-1 basis functions (%d)\n",nBasis,nBasis2);
	  exit(1);
	}
      for (jBasis=0; jBasis<nBasis; jBasis++)
	{
	  for (jSpace=0; jSpace<nSpace; jSpace++)
	    mat[jBasis][jSpace] = mat2_bs[jBasis][jSpace];
	}
      free_float_matrix(mat2_bs,nBasis,nSpace);
      return;
    }

  ////////////////// Level-4 Basis Functions /////////////////////
  nBasis4 = number_NL_basis_functions(nSpace,4);
  // Define the mixing matrix from level 2 to level 4.
  mix_lh = float_matrix(nBasis4,nBasis2);
  compute_mixing_matrix(nBasis4,nBasis2,mix_lh);
  // Compute the level-2 matrix (2x down-sampling): mat4_bs = mix_lh * mat2_bs
  mat4_bs = float_matrix(nBasis4,nSpace);
  //               X1*X2    L1       L2       X1    t   L12      X2      t
  Matrix_Product( mat4_bs, nBasis4, nSpace, mix_lh, 0, nBasis2, mat2_bs, 0);
  free_float_matrix(mix_lh,nBasis4,nBasis2);
  free_float_matrix(mat2_bs,nBasis2,nSpace);
  if ( iDown == 4 )
    {
      if ( nBasis != nBasis4 )
	{
	  printf("Programming error in define_basis_functions():\n");
	  printf("# of basis functions (%d) != # of level-1 basis functions (%d)\n",nBasis,nBasis4);
	  exit(1);
	}
      for (jBasis=0; jBasis<nBasis; jBasis++)
	{
	  for (jSpace=0; jSpace<nSpace; jSpace++)
	    mat[jBasis][jSpace] = mat4_bs[jBasis][jSpace];
	}
      free_float_matrix(mat4_bs,nBasis,nSpace);
      return;
    }

  ////////////////// Level-8 Basis Functions /////////////////////
  nBasis8 = number_NL_basis_functions(nSpace,8);
  // Define the mixing matrix from level 2 to level 4.
  mix_lh = float_matrix(nBasis8,nBasis4);
  compute_mixing_matrix(nBasis8,nBasis4,mix_lh);
  // Compute the level-2 matrix (2x down-sampling): mat8_bs = mix_lh * mat4_bs
  mat8_bs = float_matrix(nBasis8,nSpace);
  //               X1*X2    L1       L2       X1    t   L12      X2      t
  Matrix_Product( mat8_bs, nBasis8, nSpace, mix_lh, 0, nBasis4, mat4_bs, 0);
  free_float_matrix(mix_lh,nBasis8,nBasis4);
  free_float_matrix(mat4_bs,nBasis4,nSpace);
  if ( iDown == 8 )
    {
      if ( nBasis != nBasis8 )
	{
	  printf("Programming error in define_basis_functions():\n");
	  printf("# of basis functions (%d) != # of level-1 basis functions (%d)\n",nBasis,nBasis8);
	  exit(1);
	}
      for (jBasis=0; jBasis<nBasis; jBasis++)
	{
	  for (jSpace=0; jSpace<nSpace; jSpace++)
	    mat[jBasis][jSpace] = mat8_bs[jBasis][jSpace];
	}
      free_float_matrix(mat8_bs,nBasis,nSpace);
      return;
    }
  else
    {
      printf("Programming error in define_basis_functions():\n");
      printf("Down-sampling for dimension %d == %d, but 8 is the largest implementation.\n",which,iDown);
      exit(1);
    }
}

void compute_mixing_matrix( int nBasis_l, int nBasis_h, float **mix_lh )
// nBasis_l = low  dimension
// nBasis_h = high dimension
{
  int jl, jh;

  for (jl=0; jl<nBasis_l; jl++)
    {
      for (jh=0; jh<nBasis_h; jh++)
	{
	  if ( jh == 2*jl )
	    mix_lh[jl][jh] = 1.;
	  else if ( (jh == 2*jl-1) || (jh == 2*jl+1) )
	    mix_lh[jl][jh] = 0.5;
	  else
	    mix_lh[jl][jh] = 0.;
	}
    }
}
