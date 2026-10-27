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

BOOL adjust_affine_1pass( int ilevel )
     // ilevel: specifies starting increment values (see action_set_increments)
{
#ifdef JIP_USE_CONTEXT
  JIP_CHECKPOINT();
#endif

  float cost_array[NAFFINE], increment[NAFFINE], max_increment[NAFFINE];
  float cost_initial, cost_final, increment_final, value_save, mag;
  int jpar;
  BOOL adjust;

  // Save the initial cost function.
  cost_initial = cost_function();

  // Test each parameter by increasing and decreasing its value by the increment.
  for (jpar=0; jpar<NAFFINE; jpar++)
    {
      if ( AlignAid.Adjust[jpar] && AlignAid.increment[jpar] != 0. )
	{ // an increment of zero eliminates the parameter from the search
	  AlignAid.iParameter = jpar;
	  // Save the value of the parameter
	  value_save = Alignment.value[jpar];
	  // find the best value for this parameter.
	  line_scan_1d(cost_initial, &cost_final, &increment_final);
	  Alignment.value[jpar] = value_save;
	  cost_array[jpar] = cost_final - cost_initial;
	  increment[jpar]  = increment_final;

	  refresh_display_image(4);
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

#ifdef JIP_USE_CONTEXT
  engine_trace(5);
#endif
  // Find the 1st parameter adjustment.
  AlignAid.iParameter = 0;
  while ( !AlignAid.Adjust[AlignAid.iParameter] )
    AlignAid.iParameter++;
  update_alignment(0);

  return(adjust);
}
