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

void action_change_resolution()
{
  IPOINT3D TargetDim;
  float SubSampleMax_old, SubSampleMax_new;

  if ( AlignAid.SubSampleSetting.x == AlignAid.SubSample.x &&
       AlignAid.SubSampleSetting.y == AlignAid.SubSample.y &&
       AlignAid.SubSampleSetting.z == AlignAid.SubSample.z )
    return;  // no change necessary

  AlignAid.LowResolution =
    ( AlignAid.SubSampleSetting.x != 1 ) ||
    ( AlignAid.SubSampleSetting.y != 1 ) ||
    ( AlignAid.SubSampleSetting.z != 1 );

  // Zero the overlay when not at full resolution.
  if ( AlignAid.LowResolution )
    action_key_d(); // remove overlay

  SubSampleMax_old = MAX(AlignAid.SubSample.z,MAX(AlignAid.SubSample.x,AlignAid.SubSample.y));
  SubSampleMax_new = MAX(AlignAid.SubSampleSetting.z,
			 MAX(AlignAid.SubSampleSetting.x,AlignAid.SubSampleSetting.y));
  mosaic.zoom *= SubSampleMax_new/SubSampleMax_old;
  mosaic.zoom = MAX(1,mosaic.zoom);

  // reset the matrix, but keep the fov constant
  TargetDim.x = template.dim.x / AlignAid.SubSampleSetting.x;
  TargetDim.y = template.dim.y / AlignAid.SubSampleSetting.y;
  TargetDim.z = template.dim.z / AlignAid.SubSampleSetting.z;

  // Change the resolution of the target and all corresponding stacks.
  select_target(iTime_Template,TargetDim);
  AlignAid.SubSample  = AlignAid.SubSampleSetting;
  mosaic.stack = &target;
  mosaic.fSpaceMatrix = mosaic.stack->SpaceTime.fData;
  point_mosaic_to_stack_data();  // reset the data pointer

  // Reset the crosshair location.
  mosaic.icross.x = mosaic.stack->dim.x/2;
  mosaic.icross.y = mosaic.stack->dim.y/2;
  mosaic.icross.z = mosaic.stack->dim.z/2;

  // Fill the mimic volume.
  transform_coordinates(1,1,1);

  // re-create the window display
  mosaic.DimPanels.z = 0; // all panels will be displayed
  update_XWindow_image();

  // At full resolution, potentially apply the overlay.
  if ( !AlignAid.LowResolution )
    {
      if ( AlignAid.nmask != 0 )
	action_key_d(); // remove overlay
    }

  cost_function_nonlinear(0);
  update_graph_data(1);

  // xxx is this needed?
  mosaic.update_regions |= UPDATE_IMAGES;
  mosaic.update_regions |= UPDATE_RIGHT_PANEL;
}
