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

void initialize_alignment()
{
  int itest, ipower, j;
  float resolution;

  graph.iTime = iTime_Template = 0; // time indices for source & template

  AlignAid.DisplayType    = 0;      // 0 = mimic, 1 = target
  AlignAid.DisplaySet     = 0;      // 0=orientation, 1=affine, 2=2nd-order NL, 3=3rd-order NL
  AlignAid.iParameter     = 0;      // index in alignment array
  AlignAid.iFirstParShown = 0;
  AlignAid.iLastParShown  = 10;      // 9 total: 4 orient, 4 downsample, 1 rolloff, 1 matching
  AlignAid.ChangeValue    = TRUE;   // for interaction: 0 = change increment, 1 = change value of parameters
  AlignAid.ChangeValue    = TRUE;   // for interaction: 0 = change increment, 1 = change value of parameters
  Alignment.RigidBody     = TRUE;   // fix this; use "deck-of-cards" only for data sets with 1 or 2 slices
  Alignment.OrientRotation.x = Alignment.OrientRotation.y = Alignment.OrientRotation.z = 0;
  Alignment.OrientSwapZ   = 0;      // swap Z; changes parity of coordinate system (RHC<-->LHC)
  AlignAid.SubSampleSetting.x = AlignAid.SubSampleSetting.y = AlignAid.SubSampleSetting.z = 1;
  AlignAid.SubSample.x        = AlignAid.SubSample.y        = AlignAid.SubSample.z        = 1;
  AlignAid.LowResolution  = FALSE;
  AlignAid.CostFunction   = 0;

  // Alignment names: Affine (12 total)
  sprintf(AlignAid.name[0],"  x");
  sprintf(AlignAid.name[1],"  y");
  sprintf(AlignAid.name[2],"  z");
  sprintf(AlignAid.name[3],"phi_xy");  // rotation about z
  sprintf(AlignAid.name[4],"phi_yz");  // rotation about x
  sprintf(AlignAid.name[5],"phi_xz");  // rotation about y
  sprintf(AlignAid.name[6],"size-x");
  sprintf(AlignAid.name[7],"size-y");
  sprintf(AlignAid.name[8],"size-z");
  sprintf(AlignAid.name[9],"skew_xy");
  sprintf(AlignAid.name[10],"skew_xz");
  sprintf(AlignAid.name[11],"skew_yz");

  for (j=0; j<NAFFINE; j++)
    {
      if ( j>=6 && j<9 ) // inflations
	Alignment.value[j] = 1.;
      else // set all other values to zero
	Alignment.value[j] = 0.;
    }
  for (j=0; j<NAFFINE; j++)
    AlignAid.saved_value[j] = Alignment.value[j];

  // Increments:
  action_set_increments(2);

  // Sub-sampling: assume that x (readout) has the best "anatomical resolution",
  // or number of pixels, in the source data.
  // dimension dimension/32  2^n subsample=2*n
  //    32        1           0    1
  //    64        2           1    2
  //   128        4           2    4
  //   256        8           3    6
  //   512       16           4    8
  itest  = template.dim.x;
  itest /= 32;
  ipower = 0;
  while ( itest > 1 )
    {
      itest/=2;
      ipower++;
    }
  if ( ipower == 0 )
    AlignAid.SubSampleSetting.x = 1;
  else
    AlignAid.SubSampleSetting.x = MIN(8,2*ipower);
  resolution = template.resolution.x * AlignAid.SubSampleSetting.x;
  AlignAid.SubSampleSetting.y = rint((double)(resolution / template.resolution.y));
  AlignAid.SubSampleSetting.z = rint((double)(resolution / template.resolution.z));
  AlignAid.SubSampleSetting.y = MAX(1,AlignAid.SubSampleSetting.y);
  AlignAid.SubSampleSetting.z = MAX(1,AlignAid.SubSampleSetting.z);

  /////////////////////////// Non-linear alignment /////////////////////////
  //
  AlignNL.Adjust.axis.x= AlignNL.Adjust.axis.y = AlignNL.Adjust.axis.z = TRUE;
  AlignNL.Adjusted.x   = AlignNL.Adjusted.y    = AlignNL.Adjusted.z    = FALSE;
  AlignNL.ViewBasis    = FALSE;
  AlignNL.Lambda_reg   = 5;
  AlignNL.Jacob_limits = 2.;
  AlignNL.stiffness    = 0.;
  // Set the basis function down-sampling based upon a single down-sampling factor.
  SetGridDown( 8 );
}
