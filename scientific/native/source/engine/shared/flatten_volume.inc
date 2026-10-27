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

void flatten_volume( int which_volume, struct data_stack *InputStack )
// Flatten time point iTime of stack InputStack and put the result in flatData.
// "InputStack" can have multiple time points.
// "flatData" should be a vector with the same dimensions but 1 time point.
{
  struct data_stack temporary;
  POINT3D smooth_mm;
  JIP_INDEX j;

  if ( AlignAid.smooth_fov_fraction == 0 )
    return;
  else
    { // Flatten the stack dividing by a low-version of itself
      // Make a temporary copy of the InputStack.
      temporary = *InputStack;
      temporary.nt = 1;
      temporary.n4d = jip_product(temporary.n3d,temporary.nt);
      temporary.SpaceTime.fData = float_vector(temporary.n4d);
      for (j=0; j<temporary.n3d; j++)
	temporary.SpaceTime.fData[j] = InputStack->SpaceTime.fData[j];
      // Smooth the copy based upon the FOV
      smooth_mm.x = InputStack->fov.x / (float)AlignAid.smooth_fov_fraction;
      smooth_mm.y = InputStack->fov.y / (float)AlignAid.smooth_fov_fraction;
      smooth_mm.z = InputStack->fov.z / (float)AlignAid.smooth_fov_fraction;
      if ( which_volume == 0 )
	printf("Flatten the template by %g x %g x %g mm.\n",
	       smooth_mm.x,smooth_mm.y,smooth_mm.z);
      else
	printf("Flatten the source by %g x %g x %g mm.\n",
	       smooth_mm.x,smooth_mm.y,smooth_mm.z);
      smooth_or_sharpen_isotropic(0,smooth_mm, &temporary);
      // divide the smoothed version into the unfiltered version
      for (j=0; j<InputStack->n3d; j++)
	{
	  if ( temporary.SpaceTime.fData[j] != 0. )
	    InputStack->SpaceTime.fData[j] = InputStack->SpaceTime.fData[j] / temporary.SpaceTime.fData[j];
	  else
	    InputStack->SpaceTime.fData[j] = 0.;
	}
      free_float_vector(temporary.SpaceTime.fData);
    }
}
