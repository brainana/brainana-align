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
#define first_time (jip_active->first_affine_iteration)
#endif

void auto_adjust_affine_iterations( int ilevel_max, int ilevel_min )
{
#ifdef JIP_USE_CONTEXT
  JIP_CHECKPOINT();
#endif

  char string1[LEN_STR], string2[LEN_STR];
  int icount, ilevel, jpar;
  float saved_value[NAFFINE], cost_initial, cost_final, cost_last_step, tolerance;
  BOOL keep_adjusting, repeat, converged;
  
#ifndef JIP_USE_CONTEXT
  static BOOL first_time = TRUE;
#endif


  if ( AlignAid.DisplayType != 0 ) // switch to mimic
    action_change_display_type(0);
  if ( AlignAid.DisplaySet != 1 )
    {
      AlignAid.DisplaySet = 1;
      update_alignment(0);
    }
  Update_Regions();

  cost_initial = cost_function();  // update cost function here for output via print_parameter_set
#ifdef JIP_USE_CONTEXT
  if(!(cost_initial>0.)||!isfinite(cost_initial))
    jip_runtime_raise(JIP_INVALID,"affine initialization has no usable image information; improve initial overlap",0);
#endif
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
	  if ( AlignAid.CostFunction == 0 )
	    sprintf(string2,"mutual information");
	  else
	    sprintf(string2,"normalized correlation");
	  show_on_info_line(0,0,string1,string2,"");
	  icount = 0;
	  // Save the starting parameter values for this iteration.
	  cost_last_step = cost_initial = cost_function();
#ifdef JIP_USE_CONTEXT
          if(!(cost_initial>0.)||!isfinite(cost_initial))
            jip_runtime_raise(JIP_INVALID,"affine iteration has no usable image information",0);
#endif
	  for (jpar=0; jpar<NAFFINE; jpar++)
	    saved_value[jpar] = Alignment.value[jpar];
	  // Find the best set of parameters at this resolution level.
	  repeat = TRUE;
	  while ( repeat && icount < MAXIT )
	    {
	      keep_adjusting = TRUE;
	      while ( keep_adjusting )
		{
		  keep_adjusting = adjust_affine_1pass(ilevel) && icount < 20;
		  cost_final = cost_function();
		  keep_adjusting = keep_adjusting && (cost_final/cost_initial > 1.005);
		  cost_initial = cost_final;
		  icount++;
		}
	      cost_final = cost_function();
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
	  if ( icount == MAXIT )
	    printf("Reached maximum interations (%d)!\n",MAXIT);
	}
    }

  first_time = FALSE;
}

#ifdef JIP_USE_CONTEXT
#undef first_time
#endif
