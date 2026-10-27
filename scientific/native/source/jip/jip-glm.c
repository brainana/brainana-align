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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "util.h"
#include "jip.h"
#include "glm-prototypes.h"

extern char error_string[LEN_STR];


int glm_remove_or_correct_baseline( int mode )
     // mode:  0 = remove
     //        1 = correct
{
  extern int flags;

  extern struct volume_list *volume_list_root;
  struct volume_list *volume;
  struct data_stack *stack, *stack_save;
  float *data, *fit, sigma2, average;
  int nstacks, jt;JIP_INDEX jv;

  // There should only be one stack.
  for ( volume=volume_list_root, nstacks=0, stack_save=0;
	volume;
	volume=volume->next )
    {
      if ( (stack=volume->stack) == stack_save ) continue;
      stack_save = stack;
      nstacks++;
    }
  if ( nstacks > 1 )
    {
      jip_text(2,"Only one volume (stack `glm') should be passed to this command.\n");
      return(1);
    }

  // Create data and fit arrays.
  data = float_vector(stack->nt);
  fit  = float_vector(stack->nt);

  // Loop over all voxels.
  for (jv=0; jv<stack->n3d; jv++)
    {
      // Assign data to a time arrays.
      for ( volume=volume_list_root, jt=0;
	    volume;
	    volume=volume->next, jt++ )
	data[jt] = volume_amp(volume,jv);
      // Fit the GLM.
      if ( GLM_fit_data(stack->nt, data, fit, &sigma2) ) return(1);
      // Adjust the data for baseline terms.
      if ( mode == 0 )
	{
	  if ( GLM_remove_baseline(0, stack->nt, data, fit) ) return(1);
	}
      else
	{
	  if ( GLM_correct_baseline(stack->nt, data, fit, &average) ) return(1);
	}
      // Replace the volume data with the adjusted data.
      for ( volume=volume_list_root, jt=0;
	    volume;
	    volume=volume->next, jt++ )
	{
	  volume->SpaceTime.fcData[jv].real = data[jt];
	  volume->SpaceTime.fcData[jv].imag = 0.;
	}
      if (!(flags&MUTE_BIT)&& jv>0&& (4*jv)%(stack->n3d)==0)
	printf("%td percent done\n",(100*jv)/stack->n3d);
    }
  // Free temporary vectors.
  free_float_vector(data);
  free_float_vector(fit);
  return(0);
}

int glm_histogram_unbiased()
     // Assumes ... 
     //    syntax: glm-histogram [stack name]
     //    volume_list contains glm
     //    overlay contains all voxel of interest
{
  FILE *file;
  char condition_string[LEN_STR];
  float *data, *fit, sigma2, threshold, praw, pcor, effect_size, effect_sdev, cnr;
  double coeff, max_coeff, max_coeff_signed;
  int hist_unbiased_pos[9], hist_unbiased_neg[9], *mask;
  int nstacks, nconditions;
  int jt, j, ix, iy, iz, isave;JIP_INDEX jv, ivoxel;
  extern int flags;
  extern struct volume_list *volume_list_root;
  extern OVERLAY overlay;
  struct volume_list *volume;
  struct data_stack *stack, *stack_save;

  // There should only be one stack in the volume list.
  for ( volume=volume_list_root, nstacks=0, stack_save=0;
	volume;
	volume=volume->next )
    {
      if ( (stack=volume->stack) == stack_save ) continue;
      stack_save = stack;
      nstacks++;
    }
  if ( nstacks > 1 )
    {
      jip_text(2,"Only one volume (stack `glm') should be passed to this command.\n");
      return(1);
    }
  else if ( GLM_get_number_events() != 9 )
    {
      printf("Error: for now, this function assumes 9 events.\n");
      return(1);
    }
  else
    {
      strcpy(condition_string,"1-2-3-4-5-6-7-8-9");
      threshold = 0.01 / (float)stack->n3d;
      for (j=0; j<9; j++)
	hist_unbiased_pos[j] = hist_unbiased_neg[j] = 0;
    }
  if ( GLM_define_conditions(1,condition_string,&nconditions) )
    return(1);
  else if ( nconditions != 1 )
    {
      printf("Programming error: nconditions = %d in glm_histogram_unbiased\n",nconditions);
      exit(1);
    }

  // Create data and fit arrays.
  data = float_vector(stack->nt);
  fit  = float_vector(stack->nt);
  mask = int_vector(stack->n3d);

  // Overlay mask
  for (jv=0; jv<stack->n3d; jv++)
    mask[jv] = 0;  // initialize to 0
  for (jv=0; jv<overlay.nvoxels; jv++)
    {
      ix = overlay.voxel[jv].x;
      iy = overlay.voxel[jv].y;
      iz = overlay.voxel[jv].z;
      ivoxel = iz * stack->n2d + ((JIP_INDEX)(iy)) * stack->dim.x + ix;
      if ( ivoxel < 0 || ivoxel >= stack->n3d )
	{
	  printf("Programming error in glm_histogram_unbiased\n");
	  exit(1);
	}
      // Set the mask to 1 for all voxels in the overlay
      mask[ivoxel] = 1;
    }

  // Loop over all voxels.
  for (jv=0; jv<stack->n3d; jv++)
    {
      // First take care of the biased histogram.
      if ( mask[jv] == 1 )
	{
	  // Assign data to a time arrays.
	  for ( volume=volume_list_root, jt=0;
		volume;
		volume=volume->next, jt++ )
	    data[jt] = volume_amp(volume,jv);
	  // Fit the GLM.
	  if ( GLM_fit_data(stack->nt, data, fit, &sigma2) ) return(1);
	  // Calculate the significance.
	  if ( GLM_calculate_F(stack->nt, sigma2, 0, 0, &cnr, &praw, &pcor, &effect_size, &effect_sdev) )
	    return(1);
	  // If the voxel p-value is significant ...
	  //	  if ( praw < threshold && fabs((double)cnr) > 5. )
	  if ( praw < threshold )
	    {
	      // Find the max of the 9 events in the fit of the residual.
	      for (j=0, max_coeff = max_coeff_signed = 0.; j < 9; j++)
		{
		  coeff = GLM_get_effect_size(j);
		  if ( fabs((double)coeff) > max_coeff )
		    {
		      max_coeff = fabs((double)coeff);
		      max_coeff_signed = coeff;
		      isave = j;
		    }
		}
	      // Update the histogram
	      if ( max_coeff_signed > 0. )
		hist_unbiased_pos[isave]++;
	      else
		hist_unbiased_neg[isave]++;
	    }
	}
      // Output percentage done.
      if (!(flags&MUTE_BIT)&& jv>0&& (4*jv)%(stack->n3d)==0)
	printf("%td percent done\n",(100*jv)/stack->n3d);
    }
  
  // Free temporary vectors.
  free_float_vector(data);
  free_float_vector(fit);
  free_int_vector(mask);

  printf("\n");
  for (j=0; j < 9; j++)
    printf("bin %d, hist = %d, pos = %d, neg = %d\n",
	   j,hist_unbiased_pos[j]+hist_unbiased_neg[j],
	   hist_unbiased_pos[j],hist_unbiased_neg[j]);

  file = fopen("hist.dat","w");
  fprintf(file,"x yall ypos yneg\n");
  for (j=0; j < 9; j++)
    fprintf(file,"%d %d %d %d\n",j,hist_unbiased_pos[j]+hist_unbiased_neg[j],
	    hist_unbiased_pos[j],-hist_unbiased_neg[j]);
  fclose(file);
  return(0);
}

int glm_histogram_unbiased_residue()
     // Assumes ... 
     //    syntax: glm-histogram [stack name]
     //    volume_list contains glm
     //    comparison_list contains stack named above, with previous event magnitudes
     //    overlay contains all significantly activated voxels by previous criteria
{
  extern int flags;
  extern struct volume_list *volume_list_root, *comparison_list_root;
  extern OVERLAY overlay;
  struct volume_list *volume;
  struct data_stack *stack, *stack_save, *comparison_stack;
  char condition_string[LEN_STR];
  float *data, *fit, sigma2, threshold, praw, pcor, effect_size, effect_sdev, cnr;
  double max_previous, max_residue, coeff;
  int hist_unbiased[9], *hist_biased, *mask;
  int nstacks, nconditions, nevents_previous;
  int jt, j, isave, ix, iy, iz;JIP_INDEX jv, ivoxel;

  // Find the number of images in the comparison stack.
  for ( volume=comparison_list_root, nevents_previous=0; volume; volume=volume->next )
    {
      comparison_stack = comparison_list_root->stack;
      nevents_previous++;
    }
  printf("# previous events = %d\n",nevents_previous);
  printf("This function compares previous event magnitudes to current/100.\n");

  // There should only be one stack in the volume list.
  for ( volume=volume_list_root, nstacks=0, stack_save=0;
	volume;
	volume=volume->next )
    {
      if ( (stack=volume->stack) == stack_save ) continue;
      stack_save = stack;
      nstacks++;
    }
  if ( nstacks > 1 )
    {
      jip_text(2,"Only one volume (stack `glm') should be passed to this command.\n");
      return(1);
    }
  else if ( GLM_get_number_events() != 9 )
    {
      printf("Error: for now, this function assumes 9 events.\n");
      return(1);
    }
  else
    {
      strcpy(condition_string,"1-2-3-4-5-6-7-8-9");
      threshold = 0.01 / (float)stack->n3d;
      for (j=0; j<9; j++)
	hist_unbiased[j] = 0;
    }
  if ( GLM_define_conditions(1,condition_string,&nconditions) )
    return(1);
  else if ( nconditions != 1 )
    {
      printf("Programming error: nconditions = %d in glm_histogram_unbiased\n",nconditions);
      exit(1);
    }

  // Create data and fit arrays.
  data = float_vector(stack->nt);
  fit  = float_vector(stack->nt);
  mask = int_vector(stack->n3d);
  hist_biased = int_vector(nevents_previous);

  // Initialized biased histogram.
  for (j=0; j<nevents_previous; j++)
    hist_biased[j] = 0;

  // Overlay mask
  for (jv=0; jv<stack->n3d; jv++)
    mask[jv] = 0;  // initialize to 0
  for (jv=0; jv<overlay.nvoxels; jv++)
    {
      ix = overlay.voxel[jv].x;
      iy = overlay.voxel[jv].y;
      iz = overlay.voxel[jv].z;
      ivoxel = iz * stack->n2d + ((JIP_INDEX)(iy)) * stack->dim.x + ix;
      if ( ivoxel < 0 || ivoxel >= stack->n3d )
	{
	  printf("Programming error in glm_histogram_unbiased\n");
	  exit(1);
	}
      // Set the mask to 1 for all voxels in the overlay
      mask[ivoxel] = 1;
    }

  // Loop over all voxels.
  for (jv=0; jv<stack->n3d; jv++)
    {
      // First take care of the biased histogram.
      if ( mask[jv] == 1 )
	{
	  // Find the maximum of the previous fit.
	  for (j=0, max_previous = 0.; j<nevents_previous; j++)
	    {
	      ivoxel = jv + j*comparison_stack->n3d;
	      if ( ivoxel > comparison_stack->n4d )
		{
		  printf("Programming error in glm_histogram\n");
		  exit(1);
		}
	      coeff = sqrt((double)(comparison_stack->SpaceTime.fcData[ivoxel].real
				    * comparison_stack->SpaceTime.fcData[ivoxel].real
				    + comparison_stack->SpaceTime.fcData[ivoxel].imag
				    * comparison_stack->SpaceTime.fcData[ivoxel].imag ));
	      if ( coeff > max_previous )
		{
		  max_previous = coeff;
		  isave = j;
		}
	    }
	  hist_biased[isave]++;
	}

      // Assign data to a time arrays.
      for ( volume=volume_list_root, jt=0;
	    volume;
	    volume=volume->next, jt++ )
	data[jt] = volume_amp(volume,jv);
      // Fit the GLM.
      if ( GLM_fit_data(stack->nt, data, fit, &sigma2) ) return(1);
      // Calculate the significance.
      if ( GLM_calculate_F(stack->nt, sigma2, 0, 0, &cnr, &praw, &pcor, &effect_size, &effect_sdev) )
	return(1);
      // If the voxel p-value is significant ...
      if ( praw < threshold )
	{
	  // The histogram will be incremented if
	  // 1) the voxel was not previously found to be activated
	  //    according to the pre-existing overlay, or
	  // 2) the magnitude of the largest regressor in this analysis
	  //    exceeds half the magnitude of the largest previous regressor
	  //    for this voxel, as taken from the comparison stack.

	  // Find the max of the 9 events in the fit of the residual.
	  for (j=0, max_residue = 0.; j < 9; j++)
	    {
	      coeff = fabs((double)(GLM_get_effect_size(j)));
	      if ( coeff > max_residue )
		{
		  max_residue = coeff;
		  isave = j;
		}
	    }
	  max_residue /= 100.;

	  // Update the histogram if this voxel was not previously significant.
	  if ( mask[jv] != 1 )
	    hist_unbiased[isave]++;
	  else
	    { // The voxel was previously sig, so check it's magnitude.
	      // printf("maxs: %f %f\n",max_residue,max_previous); 
	      if ( max_residue > max_previous / 2. )
		hist_unbiased[isave]++;
	    }
	}
      // Output percentage done.
      if (!(flags&MUTE_BIT)&& jv>0&& (4*jv)%(stack->n3d)==0)
	printf("%td percent done\n",(100*jv)/stack->n3d);
    }

  for (j=0; j < nevents_previous; j++)
    printf("hist_biased[%d] = %d\n",j,hist_biased[j]);
  printf("\n");
  for (j=0; j < 9; j++)
    printf("hist_unbiased[%d] = %d\n",j,hist_unbiased[j]);

  // Free temporary vectors.
  free_float_vector(data);
  free_float_vector(fit);
  free_int_vector(mask);

  return(0);
}

int glm_create_residue( char *residue_string, char *new_stack_name )
{
  extern int flags;
  extern struct volume_list *volume_list_root;
  struct volume_list *volume;
  struct data_stack *stack, *stack_save, *new_stack;
  float *data, *fit, sigma2;
  int nstacks, jt;JIP_INDEX index, jv;

  // There should only be one stack.
  for ( volume=volume_list_root, nstacks=0, stack_save=0;
	volume;
	volume=volume->next )
    {
      if ( (stack=volume->stack) == stack_save ) continue;
      stack_save = stack;
      nstacks++;
    }
  if ( nstacks > 1 )
    {
      jip_text(2,"Only one volume (stack `glm') should be passed to this command.\n");
      return(1);
    }
  else if ( GLM_get_number_events() < 1 )
    {
      printf("Error: no events are defined in the GLM.\n");
      return(1);
    }

  // Install the new_stack in memory
  stack = volume_list_root->stack;
  new_stack = install_stack( new_stack_name, stack, stack->dim, stack->nt );
  new_stack->status = new_stack->status | MAP_BIT;
  new_stack->status = new_stack->status | MAG_PHASE_BIT;

  // Create data and fit arrays.
  data = float_vector(stack->nt);
  fit  = float_vector(stack->nt);

  // Loop over all voxels.
  for (jv=0; jv<stack->n3d; jv++)
    {
      // Assign data to a time arrays.
      for ( volume=volume_list_root, jt=0;
	    volume;
	    volume=volume->next, jt++ )
	data[jt] = volume_amp(volume,jv);
      // Fit the GLM.
      if ( GLM_fit_data(stack->nt, data, fit, &sigma2) ) return(1);
      // Remove the residue.
      if ( GLM_create_selective_residue(residue_string, stack->nt, data, fit) ) return(1);
      // Put the residue into the new stack.
      for (jt=0; jt<stack->nt; jt++)
	{
	  index = jv + new_stack->n3d * jt;
	  new_stack->SpaceTime.fcData[index].real = data[jt];
	  new_stack->SpaceTime.fcData[index].imag = 0;
	}
      // Output percentage done.
      if (!(flags&MUTE_BIT)&& jv>0&& (4*jv)%(stack->n3d)==0)
	printf("%td percent done\n",(100*jv)/stack->n3d);
    }
  // Free temporary vectors.
  free_float_vector(data);
  free_float_vector(fit);
  return(0);
}

int glm_fit_events( char *new_stack_name )
{
  extern int flags;
  extern struct volume_list *volume_list_root;
  struct volume_list *volume;
  struct data_stack *stack, *stack_save, *new_stack;
  float *data, *fit, sigma2, threshold, cnr, praw, pcor, effect_size, effect_sdev, max;
  int hist[9]={0,0,0,0,0,0,0,0,0}, *mask, isave;
  int nevents, nstacks, nconditions, jt, jevent;JIP_INDEX index, jv;int iz;int iy;int ix;
  extern OVERLAY overlay;

  // There should only be one stack.
  for ( volume=volume_list_root, nstacks=0, stack_save=0;
	volume;
	volume=volume->next )
    {
      if ( (stack=volume->stack) == stack_save ) continue;
      stack_save = stack;
      nstacks++;
    }
  if ( nstacks > 1 )
    {
      jip_text(2,"Only one volume (stack `glm') should be passed to this command.\n");
      return(1);
    }
  else if ( (nevents=GLM_get_number_events()) < 1 )
    {
      printf("Error: no events are defined in the GLM.\n");
      return(1);
    }
  // Define conditions using the GLM input file.
  if ( GLM_define_conditions(1,GLM_input.condition_string,&nconditions) )
    return(1);

  threshold = 0.01 / (float)stack->n3d;

  // Install the new_stack in memory
  stack = volume_list_root->stack;
  new_stack = install_stack( new_stack_name, stack, stack->dim, nevents);
  new_stack->status = new_stack->status | MAP_BIT;
  new_stack->status = new_stack->status | MAG_PHASE_BIT;

  // Create data and fit arrays.
  data = float_vector(stack->nt);
  fit  = float_vector(stack->nt);
  // Overlay mask
  mask = int_vector(stack->n3d);

  // Loop over all voxels.
  for (jv=0; jv<stack->n3d; jv++)
    {
      mask[jv] = 0;
      // Assign data to a time arrays.
      for ( volume=volume_list_root, jt=0;
	    volume;
	    volume=volume->next, jt++ )
	data[jt] = volume_amp(volume,jv);
      // Fit the GLM.
      if ( GLM_fit_data(stack->nt, data, fit, &sigma2) ) return(1);
      // Calculate the significance FOR THE 1ST CONDITIONS IN THE GLM CONTROL FILE.
      if ( GLM_calculate_F(stack->nt, sigma2, 0, 0, &cnr, &praw, &pcor, &effect_size, &effect_sdev) )
	return(1);
      // get the coefficient for the specified event
      for (jevent=0, max=0; jevent<nevents; jevent++)
	{
	  effect_size = GLM_get_effect_size(jevent);
	  if ( fabs((double)effect_size) > max )
	    {
	      max = fabs((double)effect_size);
	      isave = jevent;
	    }
	  index = jv + jevent * stack->n3d;
	  new_stack->SpaceTime.fcData[index].real = effect_size;
	  new_stack->SpaceTime.fcData[index].imag = 0;
	}
      if ( praw < threshold && isave == 2 )
	mask[jv] = 1;
      // Update the histogram if necessary.
      if ( praw < threshold ) hist[isave]++;
      // Output percentage done.
      if (!(flags&MUTE_BIT)&& jv>0&& (4*jv)%(stack->n3d)==0)
	printf("%td percent done\n",(100*jv)/stack->n3d);
    }
  // Free temporary vectors.
  free_float_vector(data);
  free_float_vector(fit);
  // Output info:
  printf("stack %s contains %d volumes in the following event order:\n",
	 new_stack_name,nevents);
  for (jevent=0; jevent<nevents; jevent++)
    printf("event %d\n",GLM_get_event_number(jevent));
  // Set the overlay based upon the mask.
  if ( overlay.nvoxels != 0 )
    free_ipoint3d_vector(overlay.voxel);
  for (jv=0, overlay.nvoxels=0; jv<stack->n3d; jv++)
    {
      if ( mask[jv] == 1 ) overlay.nvoxels++;
    }
  overlay.voxel = ipoint3d_vector(overlay.nvoxels);
  for (jv=0, overlay.nvoxels=0; jv<stack->n3d; jv++)
    {
      if ( mask[jv] == 1 )
	{
	  iz = jip_int_count(jv/stack->n2d);
	  iy = jip_int_count((jv -iz*stack->n2d)/stack->dim.x);
	  ix = jip_int_count(jv - iz*stack->n2d - ((JIP_INDEX)(iy))*stack->dim.x);
	  overlay.voxel[overlay.nvoxels].x = ix;
	  overlay.voxel[overlay.nvoxels].y = iy;
	  overlay.voxel[overlay.nvoxels].z = iz;
	  overlay.nvoxels++;
	}
    }
  printf("\nAn overlay of %td voxels was defined.\n",overlay.nvoxels);
  for (jevent=0, max=0; jevent<nevents; jevent++)
    printf("hist[%d] = %d for event %d\n",jevent,hist[jevent],GLM_get_event_number(jevent));

  return(0);
}

int glm_maximal_effect( char *condition_string, char *new_stack_name )
{
  extern int flags;
  extern struct volume_list *volume_list_root;
  struct volume_list *volume;
  struct data_stack *stack, *stack_save, *new_stack;
  float *data, *fit, sigma2, effect_size;
  int nevents, nstacks, nconditions, jt;JIP_INDEX jv;

  // There should only be one stack.
  for ( volume=volume_list_root, nstacks=0, stack_save=0;
	volume;
	volume=volume->next )
    {
      if ( (stack=volume->stack) == stack_save ) continue;
      stack_save = stack;
      nstacks++;
    }
  if ( nstacks > 1 )
    {
      jip_text(2,"Only one volume (stack `glm') should be passed to this command.\n");
      return(1);
    }
  else if ( (nevents=GLM_get_number_events()) < 1 )
    {
      printf("Error: no events are defined in the GLM.\n");
      return(1);
    }
  // Define the given condition.
  if ( GLM_define_conditions(1,condition_string,&nconditions) )
    return(1);

  // Install the new_stack in memory
  stack = volume_list_root->stack;
  new_stack = install_stack( new_stack_name, stack, stack->dim, 1);
  new_stack->status = new_stack->status | MAP_BIT;
  new_stack->status = new_stack->status | MAG_PHASE_BIT;

  // Create data and fit arrays.
  data = float_vector(stack->nt);
  fit  = float_vector(stack->nt);

  // Loop over all voxels.
  for (jv=0; jv<stack->n3d; jv++)
    {
      // Assign data to a time arrays.
      for ( volume=volume_list_root, jt=0;
	    volume;
	    volume=volume->next, jt++ )
	data[jt] = volume_amp(volume,jv);
      // Fit the GLM.
      if ( GLM_fit_data(stack->nt, data, fit, &sigma2) ) return(1);
      // Calculate the significance FOR THE 1ST CONDITIONS IN THE GLM CONTROL FILE.
      effect_size = GLM_get_maximal_effect_size(0);
      new_stack->SpaceTime.fcData[jv].real = effect_size;
      new_stack->SpaceTime.fcData[jv].imag = 0;
      // Output percentage done.
      if (!(flags&MUTE_BIT)&& jv>0&& (4*jv)%(stack->n3d)==0)
	printf("%td percent done\n",(100*jv)/stack->n3d);
    }
  // Free temporary vectors.
  free_float_vector(data);
  free_float_vector(fit);
  // Output info:
  printf("stack %s contains 1 volume\n",new_stack_name);

  return(0);
}

int glm_create_residual_variance( char *new_stack_name )
{
  extern int flags;
  extern struct volume_list *volume_list_root;
  struct volume_list *volume;
  struct data_stack *stack, *stack_save, *new_stack;
  float *data, *fit, sigma2, sum_squares;
  int nstacks, dof, jt;JIP_INDEX jv;

  // There should only be one stack.
  for ( volume=volume_list_root, nstacks=0, stack_save=0;
	volume;
	volume=volume->next )
    {
      if ( (stack=volume->stack) == stack_save ) continue;
      stack_save = stack;
      nstacks++;
    }
  if ( nstacks > 1 )
    {
      jip_text(2,"Only one volume (stack `glm') should be passed to this command.\n");
      return(1);
    }
  else if ( GLM_get_number_events() < 1 )
    {
      printf("Error: no events are defined in the GLM.\n");
      return(1);
    }

  // Install the new_stack in memory
  stack = volume_list_root->stack;
  new_stack = install_stack( new_stack_name, stack, stack->dim, 1 );
  new_stack->status = new_stack->status | MAP_BIT;
  new_stack->status = new_stack->status | MAG_PHASE_BIT;

  // Create data and fit arrays.
  data = float_vector(stack->nt);
  fit  = float_vector(stack->nt);

  dof = GLM_get_DOF();
  // Loop over all voxels.
  for (jv=0; jv<stack->n3d; jv++)
    {
      // Assign data to a time arrays.
      for ( volume=volume_list_root, jt=0;
	    volume;
	    volume=volume->next, jt++ )
	data[jt] = volume_amp(volume,jv);
      // Fit the GLM.
      if ( GLM_fit_data(stack->nt, data, fit, &sigma2) ) return(1);
      // Calculate the sum of squares.
      sum_squares = 0.;
      for (jt=0; jt<stack->nt; jt++)
	sum_squares += SQR(data[jt]-fit[jt]);
      // Normalize the sum of squares by the degrees of freedom.
      new_stack->SpaceTime.fcData[jv].real = sum_squares / (float)dof;
      new_stack->SpaceTime.fcData[jv].imag = 0.;
      // Output percentage done.
      if (!(flags&MUTE_BIT)&& jv>0&& (4*jv)%(stack->n3d)==0)
	printf("%td percent done\n",(100*jv)/stack->n3d);
    }
  // Free temporary vectors.
  free_float_vector(data);
  free_float_vector(fit);
  return(0);
}

int glm_create_AR_maps( char *new_stack_name )
{
  extern int flags;
  extern struct volume_list *volume_list_root;
  struct volume_list *volume;
  struct data_stack *stack, *stack_save, *new_stack;
  float *data, *fit, sigma2, autocor, normalize;
  int nstacks, nmaps, jt, jl;JIP_INDEX jv, loffset;

  // There should only be one stack.
  for ( volume=volume_list_root, nstacks=0, stack_save=0;
	volume;
	volume=volume->next )
    {
      if ( (stack=volume->stack) == stack_save ) continue;
      stack_save = stack;
      nstacks++;
    }
  if ( nstacks > 1 )
    {
      jip_text(2,"Only one volume (stack `glm') should be passed to this command.\n");
      return(1);
    }
  else if ( GLM_get_number_events() < 1 )
    {
      printf("Error: no events are defined in the GLM.\n");
      return(1);
    }

  nmaps = MIN(40,stack->nt-1);

  // Install the new_stack in memory
  stack = volume_list_root->stack;
  new_stack = install_stack( new_stack_name, stack, stack->dim, nmaps );
  new_stack->status = new_stack->status | MAP_BIT;
  new_stack->status = new_stack->status | MAG_PHASE_BIT;

  // Create data and fit arrays.
  data = float_vector(stack->nt);
  fit  = float_vector(stack->nt);

  // Loop over all voxels.
  for (jv=0; jv<stack->n3d; jv++)
    {
      // Assign data to a time arrays.
      for ( volume=volume_list_root, jt=0;
	    volume;
	    volume=volume->next, jt++ )
	data[jt] = volume_amp(volume,jv);
      // Fit the GLM.
      if ( GLM_fit_data(stack->nt, data, fit, &sigma2) ) return(1);
      // Remove the fit
      for (jt=0; jt<stack->nt; jt++)
	data[jt] -= fit[jt];
      // Calculate the autocorrelation for lags from 0 to nmaps
      for (jl=0; jl<=nmaps; jl++)
	{
	  autocor = 0;
	  for (jt=jl; jt<stack->nt; jt++)
	    autocor += data[jt]*data[jt-jl];
	  if ( jl == 0 )
	    normalize = autocor;
	  else
	    {
	      autocor /= normalize;
	      // Fill the stack.
	      loffset = new_stack->n3d * (jl-1);
	      new_stack->SpaceTime.fcData[jv+loffset].real = autocor;
	      new_stack->SpaceTime.fcData[jv+loffset].imag = 0.;
	    }
	}
      // Output percentage done.
      if (!(flags&MUTE_BIT)&& jv>0&& (4*jv)%(stack->n3d)==0)
	printf("%td percent done\n",(100*jv)/stack->n3d);
    }
  // Free temporary vectors.
  free_float_vector(data);
  free_float_vector(fit);
  return(0);
}

int glm_assess_global_F_variance( JIP_INDEX n3d, int test_F_old, int event_number,
				  FCOMPLEX *fcData_F_new, FCOMPLEX *fcData_F_old )
{
  char condition_string[LEN_STR], event_string[LEN_STR];
  float *data, *fit, sigma2, cnr, praw, pcor, effect_size, effect_sdev, sum_squares, var;
  float av_variance, av_cnr, norm;
  int nstacks, nevents, nconditions, dof;JIP_INDEX nvoxels;
  int jx, jy, jz, jt, jevent, ix, iy, iz;JIP_INDEX jv, index;
  BOOL include_voxel;
  short *mask_ovl;
  struct volume_list *volume, *volume_start;
  struct data_stack *stack, *stack_save;
  extern int flags;
  extern struct volume_list *volume_list_root;
  extern OVERLAY overlay;

  // There should only be one stack.
  for ( volume=volume_list_root, nstacks=0, stack_save=0;
	volume;
	volume=volume->next )
    {
      if ( (stack=volume->stack) == stack_save ) continue;
      stack_save = stack;
      if ( !strcmp(stack->name,"glm") )
	volume_start = volume;
      nstacks++;
    }
  if ( nstacks > 2 )
    {
      jip_text(2,"One or two stacks should be passed to this command.\n");
      return(1);
    }
  else if ( strcmp(stack->name,"glm") )
    {
      printf("The stack name (%s) in glm_assess_global_F_variance should be `glm'\n",stack->name);
      return(1);
    }
  else if ( (nevents=GLM_get_number_events()) < 1 )
    {
      printf("Error: no events are defined in the GLM.\n");
      return(1);
    }
  else if ( n3d != volume_list_root->n3d )
    {
      printf("Error: Inconsistent number of voxels (%td, %td) in glm_assess_global_F_variance.\n",
	     n3d,volume_list_root->n3d);
      return(1);
    }

  // Create data and fit arrays.
  data = float_vector(stack->nt);
  fit  = float_vector(stack->nt);

  // Allocate a mask volume to allow masking by an volume overlay.
  mask_ovl = short_vector(stack->n3d);
  // Set a default mask.
  for (jv=0; jv<stack->n3d; jv++)
    mask_ovl[jv] = 0;
  // Now set a mask from an overlay if one is active.
  nvoxels = 0;
  if ( overlay.nvoxels != 0 )
    {
      for (jv=0; jv<overlay.nvoxels; jv++)
	{
	  ix = overlay.voxel[jv].x;
	  iy = overlay.voxel[jv].y;
	  iz = overlay.voxel[jv].z;
	  index = index3d(ix,iy,iz,volume_list_root);
	  if ( index > stack->n3d )
	    {
	      printf("(x,y,z) = (%d,%d,%d), index = %td, total = %td\n",
		     ix,iy,iz,index,stack->n3d);
	      jip_text(2,"The overlay falls outside the volume boundaries.");
	      return(1);
	    }
	  else
	    {
	      mask_ovl[index] = 1;
	      nvoxels++;
	    }
	}
    }

  dof = GLM_get_DOF();
  // Define condition as a joint F test across all events.
  if ( event_number == 0 )
    {
      sprintf(condition_string,"%d",GLM_get_event_number(0));
      for (jevent=1; jevent<nevents; jevent++)
	{
	  //      sprintf(event_string,"-%d",GLM_get_event_number(jevent));
	  sprintf(event_string,"-%d",GLM_get_event_number(jevent));
	  strcat(condition_string,event_string);
	}
    }
  else
    sprintf(condition_string,"%d",event_number);

  printf("T test for condition %s\n",condition_string);
  if ( GLM_define_conditions(1,condition_string,&nconditions) )
    return(1);
  
  // Loop over all voxels.
  nvoxels = 0;
  av_variance = av_cnr = 0.;
  for (jz=0; jz<stack->dim.z; jz++)
    {
      for (jy=0; jy<stack->dim.y; jy++)
	{
	  for (jx=0; jx<stack->dim.x; jx++)
	    {
	      jv = index4d(jx,jy,jz,0,stack);
	      fcData_F_new[jv].real = fcData_F_new[jv].imag = 0.;
	      if ( mask_ovl[jv] == 1 )
		{
		  // Assign data to a time arrays.
		  for ( volume=volume_start, jt=0;
			volume;
			volume=volume->next, jt++ )
		    data[jt] = volume_amp(volume,jv);
		  // Fit the GLM.
		  if ( GLM_fit_data(stack->nt, data, fit, &sigma2) ) return(1);
		  // if ( GLM_remove_baseline(0, stack->nt, data, fit) ) return(1);
		  if ( GLM_calculate_F(stack->nt, sigma2, 0, 0, &cnr, &praw, &pcor,
				       &effect_size, &effect_sdev) ) return(1);
		  for (jevent=0, norm=0.; jevent<nevents; jevent++)
		    {
		      norm += SQR(GLM_get_effect_size(jevent));
		    }
		  norm = sqrt((double)norm);
		  fcData_F_new[jv].real = cnr*cnr/norm;
		  //		  
		  if ( test_F_old )
		    include_voxel = (fcData_F_new[jv].real >= fcData_F_old[jv].real);
		  else
		    include_voxel = TRUE;
		  include_voxel = TRUE;
		  //		  include_voxel &= fabs((double)praw) < 1.e-8;
		  if ( include_voxel )
		    {
		      norm = 1.;
		      av_cnr += fabs((double)(cnr/norm));
		      // Calculate the sum of squares.
		      sum_squares = 0.;
		      for (jt=0; jt<stack->nt; jt++)
			sum_squares += SQR(data[jt]/10.-fit[jt]/10.);
		      var = sum_squares / (float)dof;
		      av_variance += var/norm;
		      nvoxels++;
		    }
		}
	    }
	}
    }
  // Output percentage done.
  if (!(flags&MUTE_BIT)&& jv>0&& (4*jv)%(stack->n3d)==0)
    printf("%td percent done\n",(100*jv)/stack->n3d);
  // Normalize to global averages across the overlay region.
  if ( nvoxels != 0 )
    {
      av_variance /= (float)nvoxels;
      av_cnr        /= (float)nvoxels;
    }
  else
    av_variance = av_cnr = 0.;

  printf("# nvoxels = %td, av variance = %f, av cnr = %f\n",
	 nvoxels,av_variance,av_cnr);

  // Free temporary vectors.
  free_float_vector(data);
  free_float_vector(fit);
  free_short_vector(mask_ovl);
  return(0);
}

int glm_vary_time_constant( int event_number, char *new_stack_name )
{
  float tau, tau_max, tau_max_F, F1, F_max, step_size;
  int event_index, nstacks, nevents, nmaps, imap;JIP_INDEX iOffset, jv;
  struct volume_list *volume;
  struct data_stack *stack, *stack_save, *new_stack;
  extern int flags;
  extern struct volume_list *volume_list_root;

  // There should only be one stack.
  for ( volume=volume_list_root, nstacks=0, stack_save=0;
	volume;
	volume=volume->next )
    {
      if ( (stack=volume->stack) == stack_save ) continue;
      stack_save = stack;
      nstacks++;
    }
  if ( nstacks > 1 )
    {
      jip_text(2,"Only one volume (stack `glm') should be passed to this command.\n");
      return(1);
    }
  else if ( (nevents=GLM_get_number_events()) == 0 )
    {
      printf("Error: no events are defined in the GLM\n");
      return(1);
    }
  else if ( (event_index=GLM_get_event_index(event_number)) < 0 ||
	    event_index >= nevents )
    {
      printf("Error: the event index (%d) must be >= 0 and < the number of events (%d)\n",
	     event_index,nevents);
      return(1);
    }

  // Find the maximum time constant.
  tau_max = 2. * GLM_get_time_constant(event_index);
  printf("tau_max = %f\n",tau_max);

  // Create a new stack with one volume of F per time constant,
  // plus one more map of the time constant corresponding to maximum F.
  nmaps = (int)tau_max + 2;
  stack = volume_list_root->stack;
  new_stack = install_stack( new_stack_name, stack, stack->dim, nmaps );
  new_stack->status = new_stack->status | MAP_BIT;
  new_stack->status = new_stack->status | MAG_PHASE_BIT;

  step_size = 1;
  for ( tau=1; tau<=tau_max; tau += step_size )
    {
      GLM_set_time_constant(event_index,tau);
      printf("tau = %g\n",tau);
      imap = tau - 1;
      iOffset = imap * new_stack->n3d;
      glm_assess_global_F_variance( new_stack->n3d, 0, event_number,
				    new_stack->SpaceTime.fcData+iOffset, new_stack->SpaceTime.fcData+iOffset);
    }
  // For each voxel, cycle through the list of maps and find the maximum tau.
  for ( jv=0; jv<new_stack->n3d; jv++ )
    {
      for ( tau=1, F_max=tau_max_F=0.; tau<=tau_max; tau += step_size )
	{
	  imap = tau - 1;
	  iOffset = imap * new_stack->n3d;
	  F1 = new_stack->SpaceTime.fcData[jv+iOffset].real;
	  if ( F1 > F_max )
	    {
	      F_max = F1;
	      tau_max_F = tau;
	    }
	}
      iOffset = (new_stack->nt-2) * new_stack->n3d;
      new_stack->SpaceTime.fcData[jv+iOffset].real = F_max;
      iOffset = (new_stack->nt-1) * new_stack->n3d;
      new_stack->SpaceTime.fcData[jv+iOffset].real = tau_max_F;
    }
  return(0);
}

int glm_vary_time_constants( int ntime_constants, char *new_stack_name )
{
  float tau1, tau2, tau3, tau, tau_max, tau_max_F, F1, F_max, step_size;
  int nstacks, nevents, nmaps, imap, jevent;JIP_INDEX iOffset, jv;
  struct volume_list *volume, *F_volume;
  struct data_stack *stack, *stack_save, *new_stack;
  extern int flags;
  extern struct volume_list *volume_list_root;

  printf("# time constants = %d\n",ntime_constants);

  // There should only be one stack.
  for ( volume=volume_list_root, nstacks=0, stack_save=0;
	volume;
	volume=volume->next )
    {
      if ( (stack=volume->stack) == stack_save ) continue;
      stack_save = stack;
      nstacks++;
    }
  if ( ntime_constants == 1 && nstacks > 1 )
    {
      jip_text(2,"Only one stack (`glm') should be passed to this command with 1 time constant.\n");
      return(1);
    }
  else if ( ntime_constants > 1 && nstacks != 2 )
    {
      jip_text(2,"Two volumes (F and `glm') should be passed to this command.\n");
      return(1);
    }
  else if ( (nevents=GLM_get_number_events()) < 1 )
    {
      printf("Error: no events are defined in the GLM.\n");
      return(1);
    }
  else if ( ntime_constants > nevents )
    {
      printf("Error: # events (%d) > # time constants (%d)\n",nevents,ntime_constants);
      return(1);
    }
  else if ( ntime_constants > 3 )
    {
      printf("Error: # of time constants (%d) should be >0 and <3\n",ntime_constants);
      return(1);
    }

  F_volume = volume_list_root;

  // Find the maximum time constant.
  tau_max = 0.;
  for (jevent=0; jevent<ntime_constants; jevent++)
    {
      tau = GLM_get_time_constant(jevent);
      if ( tau > tau_max ) tau_max = tau;
    }
  tau_max *= 2.;
  printf("tau_max = %f\n",tau_max);

  // Create a new stack with one volume of F per time constant,
  // plus one more map of the time constant corresponding to maximum F.
  nmaps = (int)tau_max + 2;
  stack = volume_list_root->stack;
  new_stack = install_stack( new_stack_name, stack, stack->dim, nmaps );
  new_stack->status = new_stack->status | MAP_BIT;
  new_stack->status = new_stack->status | MAG_PHASE_BIT;

  step_size = 1;
  if ( ntime_constants == 1 )
    {
      for ( tau1=1; tau1<=tau_max; tau1 += step_size )
	{
	  GLM_set_time_constant(0,tau1);
	  printf("tau1 = %g\n",tau1);
	  imap = tau1 - 1;
	  iOffset = imap * new_stack->n3d;
	  glm_assess_global_F_variance( new_stack->n3d, 0, 0,
					new_stack->SpaceTime.fcData+iOffset,new_stack->SpaceTime.fcData+iOffset);
	}
      // For each voxel, cycle through the list of maps and find the maximum tau.
      for ( jv=0; jv<new_stack->n3d; jv++ )
	{
	  for ( tau1=1, F_max=tau_max_F=0.; tau1<=tau_max; tau1 += step_size )
	    {
	      imap = tau1 - 1;
	      iOffset = imap * new_stack->n3d;
	      F1 = new_stack->SpaceTime.fcData[jv+iOffset].real;
	      if ( F1 > F_max )
		{
		  F_max = F1;
		  tau_max_F = tau1;
		}
	    }
	  iOffset = (new_stack->nt-2) * new_stack->n3d;
	  new_stack->SpaceTime.fcData[jv+iOffset].real = F_max;
	  iOffset = (new_stack->nt-1) * new_stack->n3d;
	  new_stack->SpaceTime.fcData[jv+iOffset].real = tau_max_F;
	}
    }
  else if ( ntime_constants == 2 )
    {
      for ( tau1=1; tau1<=tau_max; tau1 += step_size )
	{
	  GLM_set_time_constant(0,tau1);
	  imap = tau1 - 1;
	  iOffset = imap * new_stack->n3d;
	  for ( tau2=tau1+1; tau2<=tau_max; tau2 += step_size )
	    {
	      GLM_set_time_constant(1,tau2);
	      printf("tau1 = %g, tau2 = %g\n",tau1,tau2);
	      glm_assess_global_F_variance( new_stack->n3d, 1, 0,
					    new_stack->SpaceTime.fcData+iOffset,F_volume->SpaceTime.fcData);
	    }
	}
    }
  else if ( ntime_constants == 3 )
    {
      for ( tau1=1; tau1<=tau_max; tau1 += step_size )
	{
	  GLM_set_time_constant(0,tau1);
	  imap = tau1 - 1;
	  iOffset = imap * new_stack->n3d;
	  for ( tau2=tau1+1; tau2<=tau_max; tau2 += step_size )
	    {
	      GLM_set_time_constant(1,tau2);
	      for ( tau3=tau2+1; tau3<=tau_max; tau3 += step_size )
		{
		  GLM_set_time_constant(2,tau3);
		  printf("tau1 = %g, tau2 = %g, tau3 = %g\n",tau1,tau2,tau3);
		  glm_assess_global_F_variance( new_stack->n3d, 1, 0,
						new_stack->SpaceTime.fcData+iOffset,F_volume->SpaceTime.fcData);
		}
	    }
	}
    }
  return(0);
}
