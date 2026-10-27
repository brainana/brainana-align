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
#define DEF_nvoxels (jip_active->def_count)
#define DEF_sum2 (jip_active->def_sum2)
#endif

float cost_deformation( int InitProbeSet, BOOL normalize )
// InitProbeSet:
// 0 = Init      : compute cost function across full volume; initialize total
// 1 = InitProbe : compute cost function across current basis function and subtract from total
// 2 = Probe     : compute cost function across current basis; reset total when done
// 3 = Set       : compute cost function across current basis; do not reset total when done
// 4 = Local     : compute cost function across current basis only
// Calling stragegies:
// a) At any point, call with InitProbeSet = 0 to get global cost deformation
// b) At any point, call with InitProbeSet = 4 to get local  cost deformation
// c) To probe a single basis function but not modify sums,
//    i) InitProbeSet = 1 to remove deformation cost of current basis
//   ii) InitProbeSet = 2 REPEATEDLY to evaluate deformation in current basis
//  iii) InitProbeSet = 3 to put deformation into global sums
{
#ifdef JIP_USE_CONTEXT
  JIP_CHECKPOINT();
#endif

  IPOINT3D Index_reg, iVoxelStart, iVoxelEnd;
  POINT3D Delta2;
  JIP_INDEX DEF_save_nvoxels;
  double DEF_save_sum2;
  float cost, fAdd, d0, d1, d2, delta_plus, delta_minus;
  int iX1, iX2, iY1, iY2, iZ1, iZ2;JIP_INDEX index0, index1, index2;
  BOOL valid_plus, valid_minus;
  
#ifndef JIP_USE_CONTEXT
  static JIP_INDEX DEF_nvoxels;
#endif

  
#ifndef JIP_USE_CONTEXT
  static double DEF_sum2;
#endif


  if ( InitProbeSet == 0 )
    {
      iVoxelStart.x = iVoxelStart.y = iVoxelStart.z = 0;
      iVoxelEnd = target.dim;
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

  Delta2.x = SQR(target.resolution.x);
  Delta2.y = SQR(target.resolution.y);
  Delta2.z = SQR(target.resolution.z);
  delta_plus = delta_minus = 0.;  // initialize to prevent passing nan to bending_penalty()

  for (Index_reg.z=iVoxelStart.z; Index_reg.z<iVoxelEnd.z; Index_reg.z++)
    {
#ifdef JIP_USE_CONTEXT
      JIP_CHECKPOINT();
#endif

      for (Index_reg.y=iVoxelStart.y; Index_reg.y<iVoxelEnd.y; Index_reg.y++)
	{
	  for (Index_reg.x=iVoxelStart.x; Index_reg.x<iVoxelEnd.x; Index_reg.x++)
	    {
	      // point to the index for the current point
	      index0 = voxel3D_to_1D(Index_reg);
	      fAdd   = 0.;

	      if ( AlignNL.Adjust.axis.x )
		{
		  ///////////////////// X distortion field ////////////////////////////////
		  d0 = distort_x.SpaceTime.fData[index0];
		  // Compression/expansion: 2 points in same direction as distortion field
		  iX1 = Index_reg.x + 1;  // point +
		  iX2 = Index_reg.x - 1;  // point -
		  valid_plus  = (iX1 >=0 && iX1<target.dim.x);
		  valid_minus = (iX2 >=0 && iX2<target.dim.x);
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
		  valid_plus  = (iY1 >=0 && iY1<target.dim.y);
		  valid_minus = (iY2 >=0 && iY2<target.dim.y);
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
		  valid_plus  = (iZ1 >=0 && iZ1<target.dim.z);
		  valid_minus = (iZ2 >=0 && iZ2<target.dim.z);
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

	      if ( AlignNL.Adjust.axis.y )
		{
		  ///////////////////// Y distortion field ////////////////////////////////
		  d0 = distort_y.SpaceTime.fData[index0];
		  // Compression/expansion: 2 points in same direction as distortion field
		  iY1 = Index_reg.y + 1;  // point +
		  iY2 = Index_reg.y - 1;  // point -
		  valid_plus  = (iY1 >=0 && iY1<target.dim.y);
		  valid_minus = (iY2 >=0 && iY2<target.dim.y);
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
		  valid_plus  = (iX1 >=0 && iX1<target.dim.x);
		  valid_minus = (iX2 >=0 && iX2<target.dim.x);
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
		  valid_plus  = (iZ1 >=0 && iZ1<target.dim.z);
		  valid_minus = (iZ2 >=0 && iZ2<target.dim.z);
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

	      if ( AlignNL.Adjust.axis.z )
		{
		  ///////////////////// Z distortion field ////////////////////////////////
		  d0 = distort_z.SpaceTime.fData[index0];
		  // Compression/expansion: 2 points in same direction as distortion field
		  iZ1 = Index_reg.z + 1;  // point +
		  iZ2 = Index_reg.z - 1;  // point -
		  valid_plus  = (iZ1 >=0 && iZ1<target.dim.z);
		  valid_minus  = (iZ2 >=0 && iZ2<target.dim.z);
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
		  valid_plus  = (iX1 >=0 && iX1<target.dim.x);
		  valid_minus = (iX2 >=0 && iX2<target.dim.x);
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
		      delta_minus = d0-d2;
		    }
		  fAdd += bending_penalty( valid_plus, valid_minus, delta_plus, delta_minus, Delta2.x );
		  iY1 = Index_reg.y + 1;  // point +
		  iY2 = Index_reg.y - 1;  // point -
		  valid_plus  = (iY1 >=0 && iY1<target.dim.y);
		  valid_minus = (iY2 >=0 && iY2<target.dim.y);
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
		      delta_minus = d0-d2;
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
	cost = 100.;  // big value
    }

  if ( InitProbeSet == 1 || InitProbeSet == 2 ) // ProbeInit or Probe
    { // Reset the voxel count and sum2, as this is only a temporary computation of MI
      DEF_sum2    -= DEF_save_sum2;
      DEF_nvoxels -= DEF_save_nvoxels;
    }

  return(cost);
}

#ifdef JIP_USE_CONTEXT
#undef DEF_nvoxels
#undef DEF_sum2
#endif
