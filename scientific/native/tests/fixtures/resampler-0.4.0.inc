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

void transform_coordinates( BOOL all_voxels, BOOL most_accurate, BOOL IgnoreOutsideTarget )
{
#ifdef JIP_USE_CONTEXT
  JIP_CHECKPOINT();
#endif

  float *data_orig, *data_reg;
  double sum_data, sum_coeff, weight;
  int MinWidth;JIP_INDEX index0, j;
  IPOINT3D Index_reg, Low_orig, High_orig, jOrig, jRel, iOrig, iWidth;
  IPOINT3D iVoxelStart, iVoxelEnd;
  IPOINT3D SmoothKernel;
  POINT3D  Space_reg, Space_orig, Index_orig, RelRes;
  float *coeff_x, *coeff_y, *coeff_z;
  float point4[4];
  BOOL InVolume, zero, ignore_overlap;
  mat44 save_ijk_to_xyz;
  mat44 save_xyz_to_ijk;

  define_affine_matrix();

  // RelRes is the resolution of the original data relative to the alignment target.
  // This value is > 1 when
  //   1) the mri is at lower resolution than the collected data, or
  //   2) the data are down-sampled during early alignment steps.
  // RelRes < 1 means interpolation into a larger matrix.
  // RelRes > 1 means smoothing into a smaller matrix.
  RelRes.x = mimic.resolution.x / source.resolution.x;
  RelRes.y = mimic.resolution.y / source.resolution.y;
  RelRes.z = mimic.resolution.z / source.resolution.z;

  if ( most_accurate )
    MinWidth = 2;
  else
    MinWidth = 1;

  iWidth.x = MAX(floor((double)(RelRes.x)),MinWidth);
  iWidth.y = MAX(floor((double)(RelRes.y)),MinWidth);
  iWidth.z = MAX(floor((double)(RelRes.z)),MinWidth);

  // Use iWidth to differentiate between interpolation and smoothing.
  SmoothKernel.x = iWidth.x > 2;
  SmoothKernel.y = iWidth.y > 2;
  SmoothKernel.z = iWidth.z > 2;

  coeff_x = float_vector(2*iWidth.x+2);   // total width is at least 2*width+1
  coeff_y = float_vector(2*iWidth.y+2);
  coeff_z = float_vector(2*iWidth.z+2);

  data_orig = source.SpaceTime.fData;
  data_reg  = mimic.SpaceTime.fData;
  if ( all_voxels )
    {
      for (j=0; j<mimic.n3d; j++)
	data_reg[j] = 0.;
    }

  save_ijk_to_xyz = mimic.ijk_to_xyz;
  save_xyz_to_ijk = source.xyz_to_ijk;

  // AlignAid.ijk_to_xyz and AlignAid.xyz_to_ijk are merely time-saving devices
  // to avoid recalculating centered-coordinate transformations on every entry.
  mimic.ijk_to_xyz  = AlignAid.ijk_to_xyz;  // target
  source.xyz_to_ijk = AlignAid.xyz_to_ijk;  // source

  if ( all_voxels )
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
  for (Index_reg.z=iVoxelStart.z; Index_reg.z<iVoxelEnd.z; Index_reg.z++)
    {
#ifdef JIP_USE_CONTEXT
      JIP_CHECKPOINT();
#endif

      for (Index_reg.y=iVoxelStart.y; Index_reg.y<iVoxelEnd.y; Index_reg.y++)
	{
	  for (Index_reg.x=iVoxelStart.x; Index_reg.x<iVoxelEnd.x; Index_reg.x++)
	    {
	      // point to the index for the current point (target or mimic)
	      index0 = voxel3D_to_1D(Index_reg);
	      data_reg[index0] = 0.; /* every local probe replaces its whole touched region */
      // See if the target value is zero for this voxel.
	      // If it is zero, it won't be used in the registration anyway, so don't register this point.
	      zero = target.SpaceTime.fData[index0] == 0.;  // target
	      if ( !AlignAid.LowResolution )
		zero |= template.SpaceTime.fData[index0] == 0.;  // no ringing, so better zero clipping
	      ignore_overlap = zero && (AlignAid.matching_volume == 0);
	      if ( ignore_overlap ) // && working_hard )
		data_reg[index0] = 0.;
	      else
		{
		  // Convert from data indices --> space in the registered system.
		  Space_reg = Convert_Voxel_to_Space( Index_reg, &mimic );
		  point4[0] = Space_reg.x;
		  point4[1] = Space_reg.y;
		  point4[2] = Space_reg.z;
		  point4[3] = 1;
		  // Rotate & translate.
		  Space_orig.x = Space_orig.y = Space_orig.z = 0.;
		  for (j=0; j<4; j++)
		    {
		      Space_orig.x += Alignment.Affine[0][j] * point4[j];
		      Space_orig.y += Alignment.Affine[1][j] * point4[j];
		      Space_orig.z += Alignment.Affine[2][j] * point4[j];
		    }

		  // non-linear distortion fields.
		  Space_orig.x += distort_x.SpaceTime.fData[index0];
		  Space_orig.y += distort_y.SpaceTime.fData[index0];
		  Space_orig.z += distort_z.SpaceTime.fData[index0];
		  // Convert from space --> indices in the source (non-registered) space.
		  Index_orig = Convert_Space_to_rVoxel( Space_orig, &source );
		  if (!isfinite(Index_orig.x)||!isfinite(Index_orig.y)||!isfinite(Index_orig.z)) continue;
      // If the nearest data value in the source system is zero
		  // then ignore this point. This prevents values from being assigned
		  // to pixels due only to neighbors (extrapolation)
		  jOrig.x = rint((double)Index_orig.x);
		  jOrig.y = rint((double)Index_orig.y);
		  jOrig.z = rint((double)Index_orig.z);
		  if ( Alignment.WrapAround.x )
		    { // Wrap x
		      if ( jOrig.x < 0 )
			jOrig.x += source.dim.x;
		      else if ( jOrig.x >= source.dim.x )
			jOrig.x -= source.dim.x;
		    }
		  if ( Alignment.WrapAround.y )
		    { // Wrap y
		      if ( jOrig.y < 0 )
			jOrig.y += source.dim.y;
		      else if ( jOrig.y >= source.dim.y )
			jOrig.y -= source.dim.y;
		    }
		  if ( Alignment.WrapAround.z )
		    { // Wrap z
		      if ( jOrig.z < 0 )
			jOrig.z += source.dim.z;
		      else if ( jOrig.z >= source.dim.z )
			jOrig.z -= source.dim.z;
		    }
		  InVolume = ( jOrig.x>=0 && jOrig.x<source.dim.x &&
			       jOrig.y>=0 && jOrig.y<source.dim.y &&
			       jOrig.z>=0 && jOrig.z<source.dim.z );
		  if ( InVolume )
		    {
		      index0 = ((JIP_INDEX)(jOrig.z))*source.dim.x*source.dim.y
			+      ((JIP_INDEX)(jOrig.y))*source.dim.x + jOrig.x;
		      if ( data_orig[index0] == 0. )
			InVolume = FALSE;
		    }

		  if ( InVolume )
		    {
		      // define limits
		      Low_orig.x  = floor((double)Index_orig.x);
		      High_orig.x = ceil((double)Index_orig.x);
		      Low_orig.y  = floor((double)Index_orig.y);
		      High_orig.y = ceil((double)Index_orig.y);
		      Low_orig.z  = floor((double)Index_orig.z);
		      High_orig.z = ceil((double)Index_orig.z);
		      // Interpolate to rectilinear pixels in source resolution space.
		      for (jOrig.x=Low_orig.x-iWidth.x, jRel.x=0;
			   jOrig.x<=High_orig.x+iWidth.x; jOrig.x++, jRel.x++)
			{
			  if ( SmoothKernel.x )
			    coeff_x[jRel.x] = Gauss((float)(Index_orig.x-jOrig.x), 0.,0.425*RelRes.x);
			  else
			    coeff_x[jRel.x] = Lanczos_filter((float)(Index_orig.x-jOrig.x),iWidth.x);
			}
		      for (jOrig.y=Low_orig.y-iWidth.y, jRel.y=0;
			   jOrig.y<=High_orig.y+iWidth.y; jOrig.y++, jRel.y++)
			{
			  if ( SmoothKernel.y )
			    coeff_y[jRel.y] = Gauss((float)(Index_orig.y-jOrig.y), 0.,0.425*RelRes.y);
			  else
			    coeff_y[jRel.y] = Lanczos_filter((float)(Index_orig.y-jOrig.y),iWidth.y);
			}
		      for (jOrig.z=Low_orig.z-iWidth.z, jRel.z=0;
			   jOrig.z<=High_orig.z+iWidth.z; jOrig.z++, jRel.z++)
			{
			  if ( SmoothKernel.z )
			    coeff_z[jRel.z] = Gauss((float)(Index_orig.z-jOrig.z), 0.,0.425*RelRes.z);
			  else
			    coeff_z[jRel.z] = Lanczos_filter((float)(Index_orig.z-jOrig.z),iWidth.z);
			}
		      sum_coeff = sum_data = 0.;
		      for (jOrig.z=Low_orig.z-iWidth.z, jRel.z=0;
			   jOrig.z<=High_orig.z+iWidth.z; jOrig.z++, jRel.z++)
			{
			  iOrig.z = jOrig.z;
			  if ( Alignment.WrapAround.z )
			    { // Wrap z
			      if ( iOrig.z < 0 )
				iOrig.z += source.dim.z;
			      else if ( iOrig.z >= source.dim.z )
				iOrig.z -= source.dim.z;
			    }
			  for (jOrig.y=Low_orig.y-iWidth.y, jRel.y=0;
			       jOrig.y<=High_orig.y+iWidth.y; jOrig.y++, jRel.y++)
			    {
			      iOrig.y = jOrig.y;
			      if ( Alignment.WrapAround.y )
				{ // Wrap y
				  if ( iOrig.y < 0 )
				    iOrig.y += source.dim.y;
				  else if ( iOrig.y >= source.dim.y )
				    iOrig.y -= source.dim.y;
				}
			      for (jOrig.x=Low_orig.x-iWidth.x, jRel.x=0;
				   jOrig.x<=High_orig.x+iWidth.x; jOrig.x++, jRel.x++)
				{
				  iOrig.x = jOrig.x;
				  if ( Alignment.WrapAround.x )
				    { // Wrap x
				      if ( iOrig.x < 0 )
					iOrig.x += source.dim.x;
				      else if ( iOrig.x >= source.dim.x )
					iOrig.x -= source.dim.x;
				    }
				  InVolume = ( iOrig.x>=0 && iOrig.x<source.dim.x &&
					       iOrig.y>=0 && iOrig.y<source.dim.y &&
					       iOrig.z>=0 && iOrig.z<source.dim.z );
				  if ( InVolume )
				    {
				      // sum over neighbors in x
				      index0 = ((JIP_INDEX)(iOrig.z))*source.dim.x*source.dim.y
					+      ((JIP_INDEX)(iOrig.y))*source.dim.x + iOrig.x;
				      weight = coeff_x[jRel.x] * coeff_y[jRel.y] * coeff_z[jRel.z];
				      sum_data  += weight * data_orig[index0];
				      sum_coeff += weight;
				    }
				}
			    }
			}
		      index0 = voxel3D_to_1D(Index_reg);
		      if ( sum_coeff == 0. )
			data_reg[index0] = 0.;
		      else
			data_reg[index0] = sum_data / sum_coeff;
		    }
		}
	    }
	}
    }
  free_float_vector(coeff_x);
  free_float_vector(coeff_y);
  free_float_vector(coeff_z);

  mimic.ijk_to_xyz = save_ijk_to_xyz;
  source.xyz_to_ijk   = save_xyz_to_ijk;
}
