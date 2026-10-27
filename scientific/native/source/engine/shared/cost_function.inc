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


/* Shared numerical source: legacy X11 frontend and context-based engine. */

float cost_function() // return cost function
{
#ifdef JIP_USE_CONTEXT
  double profile_start=jip_monotonic_seconds();jip_active->profile.metric_calls++;
#endif // return value is true if a change was made
#ifdef JIP_USE_CONTEXT
  JIP_CHECKPOINT();
#endif

  float cost;double prob_IT[NBINS][NBINS], prob_I[NBINS], prob_T[NBINS];
  float av_mimic, av_target, value_mimic, value_target, min, max;
  float H_I, H_T, H_IT;
  IPOINT3D Index_reg;
  int ibin_I, ibin_T;JIP_INDEX index0, number_voxels;
  int jI, jT;
  BOOL non_zero;

  if ( AlignAid.CostFunction == 0 )
    { // Mutual information
      // Find the average value of non-zero voxels for the source and target
      calculate_volume_averages( &av_mimic, &av_target);
#ifdef JIP_USE_CONTEXT
      if(av_target==0.||!isfinite(av_target)||!isfinite(av_mimic))
        jip_runtime_raise(JIP_INVALID,"affine metric has undefined intensity normalization",0);
#endif
      if ( av_mimic == 0. ) av_mimic = 1.;
      // Find the min and max.
      for (Index_reg.z=0, min = 1.e10, max=0.; Index_reg.z<target.dim.z; Index_reg.z++)
	{
	  for (Index_reg.y=0; Index_reg.y<target.dim.y; Index_reg.y++)
	    {
	      for (Index_reg.x=0; Index_reg.x<target.dim.x; Index_reg.x++)
		{
		  index0 = voxel3D_to_1D(Index_reg);
#ifdef JIP_USE_CONTEXT
                  if(!metric_sample(index0))continue;
#endif
		  if ( AlignAid.matching_volume == 1 )
		    non_zero = TRUE;                                  // everything
		  else
		    non_zero = target.SpaceTime.fData[index0] != 0.;  // template
		  if ( non_zero )
		    {
		      value_mimic  = mimic.SpaceTime.fData[index0]  / av_mimic;
		      value_target = target.SpaceTime.fData[index0] / av_target;
		      if ( value_mimic  < min ) min = value_mimic;
		      if ( value_target < min ) min = value_target;
		      if ( value_mimic  > max ) max = value_mimic;
		      if ( value_target > max ) max = value_target;
		    }
		}
	    }
	}
#ifdef JIP_USE_CONTEXT
      if(!(max>min)||!isfinite(min)||!isfinite(max))
        jip_runtime_raise(JIP_INVALID,"affine metric has no finite intensity range",0);
#endif
      // Initialize the histograms.
      for (jI=0; jI<NBINS; jI++)
	{
	  prob_I[jI] = prob_T[jI] = 0.;
	  for (jT=0; jT<NBINS; jT++)
	    prob_IT[jI][jT] =0.;
	}
      // Fill the histograms
      for (Index_reg.z=0, number_voxels=0; Index_reg.z<target.dim.z; Index_reg.z++)
	{
	  for (Index_reg.y=0; Index_reg.y<target.dim.y; Index_reg.y++)
	    {
	      for (Index_reg.x=0; Index_reg.x<target.dim.x; Index_reg.x++)
		{
		  index0 = voxel3D_to_1D(Index_reg);
#ifdef JIP_USE_CONTEXT
                  if(!metric_sample(index0))continue;
#endif
		  if ( AlignAid.matching_volume == 1 )
		    non_zero = TRUE;                                  // everything
		  else
		    non_zero = target.SpaceTime.fData[index0] != 0.;  // template
		  if ( non_zero )
		    {
		      value_mimic  = mimic.SpaceTime.fData[index0]  / av_mimic;
		      value_target = target.SpaceTime.fData[index0] / av_target;
		      ibin_I = MIN(NBINS-1,NBINS*(value_mimic-min)/(max-min));
		      ibin_T = MIN(NBINS-1,NBINS*(value_target-min)/(max-min));
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
	  prob_I[jI] /= (double)number_voxels;
	  prob_T[jI] /= (double)number_voxels;
	  for (jT=0; jT<NBINS; jT++)
	    prob_IT[jI][jT] /= (double)number_voxels;
	}
      // Calculate the cost function.
      cost = 0.;
      for (jI=0; jI<NBINS; jI++)
	{ // sum over bins of mimic
	  if ( prob_I[jI] != 0. )
	    {
	      for (jT=0; jT<NBINS; jT++)
		{ // sum over bins of target
		  if ( prob_IT[jI][jT] != 0. && prob_T[jT] != 0. )
		    cost += prob_IT[jI][jT] * log((double)(prob_IT[jI][jT]/prob_I[jI]/prob_T[jT]));
		}
	    }
	}
    }
  else
    { // Normalized mutual information
      // Find the average value of non-zero voxels for the source and target
      calculate_volume_averages( &av_mimic, &av_target);
#ifdef JIP_USE_CONTEXT
      if(av_target==0.||!isfinite(av_target)||!isfinite(av_mimic))
        jip_runtime_raise(JIP_INVALID,"affine metric has undefined intensity normalization",0);
#endif
      if ( av_mimic == 0. ) av_mimic = 1.;
      // Find the min and max.
      for (Index_reg.z=0, min = 1.e10, max=0.; Index_reg.z<target.dim.z; Index_reg.z++)
	{
	  for (Index_reg.y=0; Index_reg.y<target.dim.y; Index_reg.y++)
	    {
	      for (Index_reg.x=0; Index_reg.x<target.dim.x; Index_reg.x++)
		{
		  index0 = voxel3D_to_1D(Index_reg);
#ifdef JIP_USE_CONTEXT
                  if(!metric_sample(index0))continue;
#endif
		  if ( AlignAid.matching_volume == 1 )
		    non_zero = TRUE;                                  // everything
		  else
		    non_zero = target.SpaceTime.fData[index0] != 0.;  // template
		  if ( non_zero )
		    {
		      value_mimic  = mimic.SpaceTime.fData[index0]  / av_mimic;
		      value_target = target.SpaceTime.fData[index0] / av_target;
		      if ( value_mimic  < min ) min = value_mimic;
		      if ( value_target < min ) min = value_target;
		      if ( value_mimic  > max ) max = value_mimic;
		      if ( value_target > max ) max = value_target;
		    }
		}
	    }
	}
#ifdef JIP_USE_CONTEXT
      if(!(max>min)||!isfinite(min)||!isfinite(max))
        jip_runtime_raise(JIP_INVALID,"affine metric has no finite intensity range",0);
#endif
      // Initialize the histograms.
      for (jI=0; jI<NBINS; jI++)
	{
	  prob_I[jI] = prob_T[jI] = 0.;
	  for (jT=0; jT<NBINS; jT++)
	    prob_IT[jI][jT] =0.;
	}
      // Fill the histograms
      for (Index_reg.z=0, number_voxels=0; Index_reg.z<target.dim.z; Index_reg.z++)
	{
	  for (Index_reg.y=0; Index_reg.y<target.dim.y; Index_reg.y++)
	    {
	      for (Index_reg.x=0; Index_reg.x<target.dim.x; Index_reg.x++)
		{
		  index0 = voxel3D_to_1D(Index_reg);
#ifdef JIP_USE_CONTEXT
                  if(!metric_sample(index0))continue;
#endif
		  if ( AlignAid.matching_volume == 1 )
		    non_zero = TRUE;                                  // everything
		  else
		    non_zero = target.SpaceTime.fData[index0] != 0.;  // template
		  if ( non_zero )
		    {
		      value_mimic  = mimic.SpaceTime.fData[index0]  / av_mimic;
		      value_target = target.SpaceTime.fData[index0] / av_target;
		      ibin_I = MIN(NBINS-1,NBINS*(value_mimic-min)/(max-min));
		      ibin_T = MIN(NBINS-1,NBINS*(value_target-min)/(max-min));
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
	  prob_I[jI] /= (double)number_voxels;
	  prob_T[jI] /= (double)number_voxels;
	  for (jT=0; jT<NBINS; jT++)
	    prob_IT[jI][jT] /= (double)number_voxels;
	}
      for (jI=0, H_I=0.; jI<NBINS; jI++)
	{ // sum over bins of mimic
	  if ( prob_I[jI] != 0. )
	    H_I -= prob_T[jI] * log((double)(prob_I[jI]));
	}
      for (jT=0, H_T=0.; jT<NBINS; jT++)
	{ // sum over bins of mimic
	  if ( prob_T[jT] != 0. )
	    H_T -= prob_T[jT] * log((double)(prob_T[jT]));
	}
      for (jI=0, H_IT=0.; jI<NBINS; jI++)
	{
	  for (jT=0; jT<NBINS; jT++)
	    { // sum over bins of target
	      if ( prob_IT[jI][jT] != 0. )
		H_IT -= prob_IT[jI][jT] * log((double)(prob_IT[jI][jT]));
	    }
	}
      // Calculate the cost function.
      if ( H_IT != 0. )
	cost = (H_I + H_T) / H_IT;
      else
	cost = 0.;
    }
  AlignAid.CF_Affine = cost;  // save this value to speed display functions
#ifdef JIP_USE_CONTEXT
  jip_active->profile.metric_seconds+=jip_monotonic_seconds()-profile_start;
#endif
  return(cost);
}
