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
#define TargetDim (jip_active->basis_target_dim)
#define first_time (jip_active->first_basis)
#endif

void update_nonlinear_alignment_parameters()
// This function changes non-linear basis functions at CONSTANT DISTORTION FIELD RESOLUTION.
// Dimensions of distortion fields MUST EQUAL target dimensions when this function called.
{
#ifdef JIP_USE_CONTEXT
  JIP_CHECKPOINT();
#endif

  int *PeakX, *PeakY, *PeakZ, *StartX, *StartY, *StartZ, *EndX, *EndY, *EndZ;
  int jx, jy, jz;JIP_INDEX iBasis;JIP_INDEX j, index_map, jBasis;
  //  int ix, iy, iz;
  float fBasis;
  IPOINT3D Index_reg, iVoxel;
  
#ifndef JIP_USE_CONTEXT
  static IPOINT3D TargetDim;
#endif

  
#ifndef JIP_USE_CONTEXT
  static BOOL first_time=TRUE;
#endif


  if ( target.dim.x != distort_x.dim.x ||
       target.dim.y != distort_x.dim.y ||
       target.dim.z != distort_x.dim.z )
    {
      printf("Programming error: distortion field dimensions (%d,%d,%d) != target dimensions (%d,%d,%d)\n",
	     distort_x.dim.x,distort_x.dim.y,distort_x.dim.z,target.dim.x,target.dim.y,target.dim.z);
      exit(1);
    }

  if ( !first_time )
    {
      free_ipoint3d_vector(AlignNL.Index3DStart);
      free_ipoint3d_vector(AlignNL.Index3DEnd);
      free_ipoint3d_vector(AlignNL.iBasis1D);
      free(AlignNL.NeighborLow);
      free(AlignNL.NeighborHigh);
      free_point3d_vector(AlignNL.value);
      free_float_vector(AlignNL.Scale);
      free_int_vector(AlignNL.Adjust3D);
      free(AlignNL.AdjustOrder);
      free_float_matrix(AlignNL.BasisX,AlignNL.NBasis1D.x,TargetDim.x);
      free_float_matrix(AlignNL.BasisY,AlignNL.NBasis1D.y,TargetDim.y);
      free_float_matrix(AlignNL.BasisZ,AlignNL.NBasis1D.z,TargetDim.z);
    }
  TargetDim  = target.dim;
  first_time = FALSE;

  // Calculate the number of basis functions.
  AlignNL.NBasis1D.x = number_NL_basis_functions(target.dim.x,AlignNL.GridDown.x);
  AlignNL.NBasis1D.y = number_NL_basis_functions(target.dim.y,AlignNL.GridDown.y);
  AlignNL.NBasis1D.z = number_NL_basis_functions(target.dim.z,AlignNL.GridDown.z);
  AlignNL.NBasis = jip_product(jip_product(AlignNL.NBasis1D.x,AlignNL.NBasis1D.y),AlignNL.NBasis1D.z);

  // Define the basis functions
  AlignNL.BasisX       = float_matrix(AlignNL.NBasis1D.x,target.dim.x);
  AlignNL.BasisY       = float_matrix(AlignNL.NBasis1D.y,target.dim.y);
  AlignNL.BasisZ       = float_matrix(AlignNL.NBasis1D.z,target.dim.z);
  define_basis_functions(0); // x
  define_basis_functions(1); // y
  define_basis_functions(2); // z

  // Allocate space for the start and end indices.
  AlignNL.Index3DStart = ipoint3d_vector(AlignNL.NBasis);
  AlignNL.Index3DEnd   = ipoint3d_vector(AlignNL.NBasis);
  AlignNL.iBasis1D     = ipoint3d_vector(AlignNL.NBasis);
  AlignNL.NeighborLow  = index3d_vector(AlignNL.NBasis);
  AlignNL.NeighborHigh = index3d_vector(AlignNL.NBasis);
  //
  AlignNL.value        =  point3d_vector(AlignNL.NBasis);
  AlignNL.Adjust3D     = int_vector(AlignNL.NBasis);
  AlignNL.AdjustOrder  = index_vector(AlignNL.NBasis);
  AlignNL.Scale        = float_vector(AlignNL.NBasis);

  // Define the start/stop/peak locations.
  PeakX  = int_vector(AlignNL.NBasis1D.x);
  StartX = int_vector(AlignNL.NBasis1D.x);
  EndX   = int_vector(AlignNL.NBasis1D.x);
  PeakY  = int_vector(AlignNL.NBasis1D.y);
  StartY = int_vector(AlignNL.NBasis1D.y);
  EndY   = int_vector(AlignNL.NBasis1D.y);
  PeakZ  = int_vector(AlignNL.NBasis1D.z);
  StartZ = int_vector(AlignNL.NBasis1D.z);
  EndZ   = int_vector(AlignNL.NBasis1D.z);

  calc_NL_basis_function_locations(0,PeakX,StartX,EndX);
  calc_NL_basis_function_locations(1,PeakY,StartY,EndY);
  calc_NL_basis_function_locations(2,PeakZ,StartZ,EndZ);

  // Fill values
  for (jz=0; jz<AlignNL.NBasis1D.z; jz++)
    {
      for (jy=0; jy<AlignNL.NBasis1D.y; jy++)
	{
	  for (jx=0; jx<AlignNL.NBasis1D.x; jx++)
	    {
	      iBasis = ((JIP_INDEX)(jz)) * AlignNL.NBasis1D.x * AlignNL.NBasis1D.y
		+      ((JIP_INDEX)(jy)) * AlignNL.NBasis1D.x
		+      jx;

	      AlignNL.iBasis1D[iBasis].x = jx;
	      AlignNL.iBasis1D[iBasis].y = jy;
	      AlignNL.iBasis1D[iBasis].z = jz;

	      // Find basis functions corresponding to low neighbors
	      AlignNL.NeighborLow[iBasis].x
		= ((JIP_INDEX)(jz)) * AlignNL.NBasis1D.x * AlignNL.NBasis1D.y
		+ ((JIP_INDEX)(jy)) * AlignNL.NBasis1D.x
		+ MAX(jx-1,0);
	      AlignNL.NeighborLow[iBasis].y
		= ((JIP_INDEX)(jz)) * AlignNL.NBasis1D.x * AlignNL.NBasis1D.y
		+ MAX(jy-1,0) * AlignNL.NBasis1D.x
		+ jx;
	      AlignNL.NeighborLow[iBasis].z
		= MAX(jz-1,0) * AlignNL.NBasis1D.x * AlignNL.NBasis1D.y
		+ ((JIP_INDEX)(jy)) * AlignNL.NBasis1D.x
		+ jx;
	      // Find basis functions corresponding to high neighbors
	      AlignNL.NeighborHigh[iBasis].x
		= ((JIP_INDEX)(jz)) * AlignNL.NBasis1D.x * AlignNL.NBasis1D.y
		+ ((JIP_INDEX)(jy)) * AlignNL.NBasis1D.x
		+ MIN(jx+1,AlignNL.NBasis1D.x-1);
	      AlignNL.NeighborHigh[iBasis].y
		= ((JIP_INDEX)(jz)) * AlignNL.NBasis1D.x * AlignNL.NBasis1D.y
		+ MIN(jy+1,AlignNL.NBasis1D.y-1) * AlignNL.NBasis1D.x
		+ jx;
	      AlignNL.NeighborHigh[iBasis].z
		= MIN(jz+1,AlignNL.NBasis1D.z-1) * AlignNL.NBasis1D.x * AlignNL.NBasis1D.y
		+ ((JIP_INDEX)(jy)) * AlignNL.NBasis1D.x
		+ jx;

	      // Set all values ON at beginning	      
	      AlignNL.Adjust3D[iBasis]    = TRUE;
	      // Set a default adjust order to be sequential.
	      AlignNL.AdjustOrder[iBasis] = iBasis;

	      // The following arrays are indices into the basis functions
	      AlignNL.Index3DStart[iBasis].x = StartX[jx];
	      AlignNL.Index3DStart[iBasis].y = StartY[jy];
	      AlignNL.Index3DStart[iBasis].z = StartZ[jz];
	      AlignNL.Index3DEnd[iBasis].x   = EndX[jx]; // add 1 to all ends
	      AlignNL.Index3DEnd[iBasis].y   = EndY[jy]; // so that we can treat these
	      AlignNL.Index3DEnd[iBasis].z   = EndZ[jz]; // like target.dim in loops

	      // Set the scale for each basis function.
	      // This needs to be done AFTER setting start/end values
	      iVoxel.x = PeakX[jx];
	      iVoxel.y = PeakY[jy];
	      iVoxel.z = PeakZ[jz];
	      fBasis   = NL_Basis_function(iBasis, iVoxel);
	      AlignNL.Scale[iBasis] = fBasis;
	
	      // Read the values straight off the distortion maps.
	      index_map = ((JIP_INDEX)(PeakZ[jz])) * target.dim.x * target.dim.y
		+         ((JIP_INDEX)(PeakY[jy])) * target.dim.x
		+         PeakX[jx];
	      AlignNL.value[iBasis].x = distort_x.SpaceTime.fData[index_map];
	      AlignNL.value[iBasis].y = distort_y.SpaceTime.fData[index_map];
	      AlignNL.value[iBasis].z = distort_z.SpaceTime.fData[index_map];
	    }
	}
    }

  free_int_vector(PeakX);
  free_int_vector(PeakY);
  free_int_vector(PeakZ);
  free_int_vector(StartX);
  free_int_vector(StartY);
  free_int_vector(StartZ);
  free_int_vector(EndX);
  free_int_vector(EndY);
  free_int_vector(EndZ);

  // update the distortion maps
  for (j=0; j<distort_x.n3d; j++)
    distort_x.SpaceTime.fData[j] = distort_y.SpaceTime.fData[j] = distort_z.SpaceTime.fData[j] = Jac_deter.SpaceTime.fData[j] = 0.;

  for (jBasis=0; jBasis<AlignNL.NBasis; jBasis++)
    {
      for (Index_reg.z=AlignNL.Index3DStart[jBasis].z;
	   Index_reg.z<AlignNL.Index3DEnd[jBasis].z; Index_reg.z++)
	{
	  for (Index_reg.y=AlignNL.Index3DStart[jBasis].y;
	       Index_reg.y<AlignNL.Index3DEnd[jBasis].y; Index_reg.y++)
	    {
	      for (Index_reg.x=AlignNL.Index3DStart[jBasis].x;
		   Index_reg.x<AlignNL.Index3DEnd[jBasis].x; Index_reg.x++)
		{
		  index_map = index4d(Index_reg.x,Index_reg.y,Index_reg.z,0);
		  fBasis = NL_Basis_function(jBasis,Index_reg);
		  distort_x.SpaceTime.fData[index_map] += AlignNL.value[jBasis].x * fBasis;
		  distort_y.SpaceTime.fData[index_map] += AlignNL.value[jBasis].y * fBasis;
		  distort_z.SpaceTime.fData[index_map] += AlignNL.value[jBasis].z * fBasis;
		} // x
	    } // y
	} // z
    }

  if ( AlignAid.DisplayType > 1 )
    {
      mosaic.update_regions |= UPDATE_IMAGES;
      mosaic.update_regions |= UPDATE_SELECT_TEXT_IMAGE;
    }

#ifdef JIP_USE_CONTEXT
  engine_trace(3);
#endif
}

#ifdef JIP_USE_CONTEXT
#undef TargetDim
#undef first_time
#endif
