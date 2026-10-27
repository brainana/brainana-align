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
#ifdef JIP_USE_CONTEXT
#define NC_i2 (jip_active->nc_i2)
#define NC_t2 (jip_active->nc_t2)
#define NC_it (jip_active->nc_it)
#define av_mimic (jip_active->mean_mimic)
#define av_target (jip_active->mean_target)
#define min (jip_active->minimum)
#define max (jip_active->maximum)
#define MI_number_voxels (jip_active->mi_count)
#define MI_prob_I (jip_active->prob_i)
#define MI_prob_T (jip_active->prob_t)
#define MI_prob_IT (jip_active->prob_it)
#endif

float cost_function_nonlinear( int InitProbeSet )
// InitProbeSet:
// 0 = Init      : compute averages and cost function across full volume; initialize histograms
// 1 = InitProbe : compute histograms across current basis function and subtract from cumulative histograms
// 2 = Probe     : compute cost function across current basis; reset histograms when done
// 3 = Set       : compute cost function across current basis; do not reset histograms when done
// Calling stragegies:
// a) At any point, call with InitProbeSet = 0 to get global cost deformation
// b) To probe a single basis function but not modify sums,
//    i) InitProbeSet = 1 to remove cost of current basis
//   ii) InitProbeSet = 2 REPEATEDLY to evaluate cost in current basis
//  iii) InitProbeSet = 3 to update global sums by setting a basis scaling factor
{
#ifdef JIP_USE_CONTEXT
  double profile_start=jip_monotonic_seconds();jip_active->profile.metric_calls++;
#endif
#ifdef JIP_USE_CONTEXT
  JIP_CHECKPOINT();
#endif
 // return value is true if a change was made
  float cost;
  float value_mimic, value_target;
  float ampi, ampt, cost_def;
  double sumi2, sumt2, sumit;
  
#ifndef JIP_USE_CONTEXT
  static double NC_i2=0, NC_t2=0, NC_it=0;
#endif

  IPOINT3D Index_reg, iVoxelStart, iVoxelEnd;
  int ibin_I, ibin_T;JIP_INDEX index0;
  int jI, jT;
  BOOL non_zero, all_bases;
  JIP_INDEX MI_save_voxels;
  double MI_save_I[NBINS], MI_save_T[NBINS], MI_save_IT[NBINS][NBINS];
  
#ifndef JIP_USE_CONTEXT
  static float av_mimic, av_target, min, max;
#endif

  
#ifndef JIP_USE_CONTEXT
  static JIP_INDEX MI_number_voxels;
#endif

  
#ifndef JIP_USE_CONTEXT
  static double MI_prob_I[NBINS], MI_prob_T[NBINS], MI_prob_IT[NBINS][NBINS];
#endif


  if ( InitProbeSet == 0 )
    { // init: compute cost function across full volume
      iVoxelStart.x = iVoxelStart.y = iVoxelStart.z = 0;
      iVoxelEnd = target.dim;
      all_bases = TRUE;
    }
  else
    { // probe or set: compute cost function across current basis function, & update histograms
      iVoxelStart = AlignNL.Index3DStart[AlignNL.iBasis];
      iVoxelEnd   = AlignNL.Index3DEnd[AlignNL.iBasis];
      all_bases   = FALSE;
    }

  if ( AlignAid.CostFunction == 0 )
    { // Mutual information
      // Find the average value of non-zero voxels for the mimic and target
      if ( InitProbeSet == 0 )
	{ // recalculate averages across the volumes
	  calculate_volume_averages_nonlinear( all_bases, &av_mimic, &av_target, &min, &max);
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
		      index0 = voxel3D_to_1D(Index_reg);
#ifdef JIP_USE_CONTEXT
                  if(!metric_sample(index0))continue;
#endif
		      if ( AlignAid.matching_volume == 0 )
			non_zero = target.SpaceTime.fData[index0] != 0.;  // template
		      else
			non_zero = TRUE;                                  // everything
		      if ( non_zero && isfinite(mimic.SpaceTime.fData[index0]) && isfinite(target.SpaceTime.fData[index0]) )
			{
			  value_mimic  = mimic.SpaceTime.fData[index0]  / av_mimic;
			  value_target = target.SpaceTime.fData[index0] / av_target;
			  ibin_I = (int)fmax(0.,fmin(NBINS-1.,NBINS*((double)value_mimic-min)/((double)max-min)));
			  ibin_T = (int)fmax(0.,fmin(NBINS-1.,NBINS*((double)value_target-min)/((double)max-min)));
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
    { /* Preserve uncentered normalized correlation, using the same global
         objective for local probes as for full-volume scoring. */
      sumi2=sumt2=sumit=0.;
      for(Index_reg.z=iVoxelStart.z;Index_reg.z<iVoxelEnd.z;Index_reg.z++)
        for(Index_reg.y=iVoxelStart.y;Index_reg.y<iVoxelEnd.y;Index_reg.y++)
          for(Index_reg.x=iVoxelStart.x;Index_reg.x<iVoxelEnd.x;Index_reg.x++) {
            index0=voxel3D_to_1D(Index_reg);
#ifdef JIP_USE_CONTEXT
            if(!metric_sample(index0))continue;
#endif
            ampi=mimic.SpaceTime.fData[index0];ampt=target.SpaceTime.fData[index0];
            if((AlignAid.matching_volume==0 && ampt==0)||!isfinite(ampi)||!isfinite(ampt))continue;
            sumi2+=(double)ampi*ampi;sumt2+=(double)ampt*ampt;sumit+=(double)ampi*ampt;
          }
      if(InitProbeSet==0){NC_i2=sumi2;NC_t2=sumt2;NC_it=sumit;}
      else if(InitProbeSet!=1){NC_i2+=sumi2;NC_t2+=sumt2;NC_it+=sumit;}
      cost=(NC_i2>0&&NC_t2>0)?NC_it/sqrt(NC_i2)/sqrt(NC_t2):0.;
      if(InitProbeSet==1||InitProbeSet==2){NC_i2-=sumi2;NC_t2-=sumt2;NC_it-=sumit;}
    }
  if ( InitProbeSet == 0 )
    AlignAid.CF_Affine = cost;  // save this value to speed display functions

  // Add on the regularization term.
  if ( AlignNL.Lambda_reg != 0 )
    {
      cost_def = cost_deformation( InitProbeSet, 1 );
      cost -= (float)(AlignNL.Lambda_reg*AlignNL.Lambda_reg)/10. * cost_def;
      if ( InitProbeSet == 0 )
	AlignAid.CF_NonLinear = cost;  // save this value to speed display functions
    }
#ifdef JIP_USE_CONTEXT
  jip_active->profile.metric_seconds+=jip_monotonic_seconds()-profile_start;
#endif
  return(cost);
}

#ifdef JIP_USE_CONTEXT
#undef NC_i2
#undef NC_t2
#undef NC_it
#undef av_mimic
#undef av_target
#undef min
#undef max
#undef MI_number_voxels
#undef MI_prob_I
#undef MI_prob_T
#undef MI_prob_IT
#endif
