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

void adjust_nonlinear_1pass( int ilevel, int ipass, float cost_start )
{
#ifdef JIP_USE_CONTEXT
  JIP_CHECKPOINT();
#endif

  char string1[LEN_STR], string2[LEN_STR], string3[LEN_STR];
  float cost_initial;
  int iMod;JIP_INDEX NCoeff, nbad, jForward;JIP_INDEX jBasis;

  // Find the number of coefficients
  NCoeff = 0;
  if ( AlignNL.Adjust.axis.x ) NCoeff += AlignNL.NBasis;
  if ( AlignNL.Adjust.axis.y ) NCoeff += AlignNL.NBasis;
  if ( AlignNL.Adjust.axis.z ) NCoeff += AlignNL.NBasis;

  if ( AlignNL.NBasis < 1000 )
    iMod = 10;
  else if ( AlignNL.NBasis < 10000 )
    iMod = 100;
  else if ( AlignNL.NBasis < 100000 )
    iMod = 1000;
  else
    iMod = 5000;

  if ( AlignAid.CostFunction == 0 )
    sprintf(string1,"MI = %5.3f               , level #%d",AlignAid.CF_Affine,ilevel);
  else
    sprintf(string1,"NC = %5.3f               , level #%d",AlignAid.CF_Affine,ilevel);

  cost_initial = cost_start;
  nbad = number_invalid_distortions(1, ilevel, ipass);

  //  initialize_adjustment_order(ipass);
  determine_adjustment_order(); // This evaluates the order by deformation cost, altering histograms
  cost_function_nonlinear(0);   // necessary to initialize cost function histograms

  //  Adjust in reverse order, because the "AdjustOrder" array is low-to-high deformation cost
  for (jBasis=AlignNL.NBasis-1, jForward=0; jBasis>=0; jBasis--, jForward++)
    {
      AlignNL.iBasis = AlignNL.AdjustOrder[jBasis];
      if ( AlignNL.Adjust3D[AlignNL.iBasis] )
	{ // an increment of zero eliminates the parameter from the search
	  if ( jForward%iMod == 0 )
	    {
	      sprintf(string2,"basis functions: %7td ,  pass #%d",AlignNL.NBasis,ipass+1);
	      sprintf(string3,"adjusting basis: %7td",jForward);
	      show_on_info_line(0,0,string1,string2,string3);
	    }
	  adjust_1basis(ipass);
	}
    }

  //  adjust_the_adjust_flags();
  cost_function_nonlinear(0); // update for display
  update_alignment(0);

  
#ifdef JIP_USE_CONTEXT
  engine_trace(2);
#endif
return;

#ifdef JIP_USE_CONTEXT
  engine_trace(2);
#endif
}
