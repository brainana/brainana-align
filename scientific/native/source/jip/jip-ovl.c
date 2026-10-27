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

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "util.h"
#include "jip.h"

extern char error_string[LEN_STR];


int read_ovl_file( char *file_name )
{
  FILE *ovl_file;
  char whole_line[LEN_STR];
  int ix, iy, iz, nread, found, number_overlap, number_new;
  JIP_INDEX j, j1, j2;
  float weight;
  OVERLAY overlay_temp, overlay_new;
  extern OVERLAY overlay;
  
  /*
  ** Open the ovl file.
  */
  if ( !(ovl_file = fopen(file_name,"r")) )
    {
      jip_text(2,"I was unable to open the OVL file.\n");
      return(1);
    }

  /*
  ** Determine the number of pixels in the overlay.
  */
  overlay_temp.nvoxels = 0;
 // The file format can be one of the following
  //      x y z weight
  //      x y z             (weight=1)
  // Now read the overlay and set those pixels to 1.
  while ( get_line_with_comment(ovl_file, whole_line, LEN_STR) )
    { // Try to read 4 values
      if ( sscanf(whole_line,"%d %d %d %f",&ix,&iy,&iz,&weight) != 4 )
	{ // On failure, read 3 values
	  if ( sscanf(whole_line,"%d %d %d",&ix,&iy,&iz) != 3 )
	    {
	      printf("Error: The syntax in an overlay file can be one of the following:\n");
	      printf("1)  x y z         (voxel indices), OR\n");
	      printf("2)  x y z weight  (for variable weight in range 0:1)\n");
	      return(1);
	    }
	  else
	    weight = 1.;
	}
      if ( weight < 0. || weight > 1. )
	{ // This isn't mathematically necessary, but enforce anyway
	  printf("Error: use weight values <= 1. in overlays.\n");
	  return(1);
	}
      else
	overlay_temp.nvoxels++;
    }
  jip_seek(ovl_file,0L,0);              /* Rewind the file. */

  overlay_temp.voxel  = ipoint3d_vector(overlay_temp.nvoxels);
  overlay_temp.weight = float_vector(overlay_temp.nvoxels);
  /*
  ** Read the overlay.
  */
  nread = 0;
  // The file format can be one of the following
  //      x y z weight
  //      x y z             (weight=1)
  // Now read the overlay and set those pixels to 1.
  while ( get_line_with_comment(ovl_file, whole_line, LEN_STR) )
    { // Try to read 4 values
      if ( sscanf(whole_line,"%d %d %d %f",&ix,&iy,&iz,&weight) != 4 )
	{ // On failure, read 3 values
	  if ( sscanf(whole_line,"%d %d %d",&ix,&iy,&iz) != 3 )
	    {
	      printf("Error: The syntax in an overlay file can be one of the following:\n");
	      printf("1)  x y z         (voxel indices), OR\n");
	      printf("2)  x y z weight  (for variable weight in range 0:1)\n");
	      return(1);
	    }
	  else
	    weight = 1.;
	}
      if ( weight < 0. || weight > 1. )
	{ // This isn't mathematically necessary, but enforce anyway
	  printf("Error: use weight values <= 1. in overlays.\n");
	  return(1);
	}
      else
	{
	  overlay_temp.voxel[nread].x = ix;
	  overlay_temp.voxel[nread].y = iy;
	  overlay_temp.voxel[nread].z = iz;
	  overlay_temp.weight[nread]  = weight;
	  nread++;
	}
    }
  fclose(ovl_file);

  /*
  ** The overlay file has been read.  Now do one of the following:
  ** 0) replace the old overlay with the new one.
  ** 1) OR  the two overlays (sum the weights)
  ** 2) AND the two overlays (multiply the weights)
  **
  ** NOTE THAT THESE PROCEDURES CAN BE VERY SLOW FOR LARGE OVERLAYS.
  ** These combinations do not assume knowledge of the base matrix size,
  ** so masks cannot be set to speed things up.  So, ... there are loops of n x n.
  */

  if ( overlay.io_logic==0 || overlay.nvoxels==0 )   // REPLACE
    {
      if ( overlay.nvoxels != 0 )
	{
	  free_ipoint3d_vector(overlay.voxel);
	  free_float_vector(overlay.weight);
	}
      overlay.nvoxels = overlay_temp.nvoxels;
      overlay.voxel   = ipoint3d_vector(overlay.nvoxels);
      overlay.weight  = float_vector(overlay.nvoxels);
      for (j=0; j<overlay.nvoxels; j++)
	{
	  overlay.voxel[j] = overlay_temp.voxel[j];
	  overlay.weight[j]= overlay_temp.weight[j];
	}
      free_ipoint3d_vector(overlay_temp.voxel);
      free_float_vector(overlay_temp.weight);
    }
  else if ( overlay.io_logic == 1 )      // LOGICAL OR
    {
      // Count the number of overlapping pixels in the two overlays.
      number_overlap = 0;
      for (j1=0; j1<overlay.nvoxels; j1++)
	{
	  found = 0;
	  for (j2=0; j2<overlay_temp.nvoxels; j2++)
	    if ( overlay_temp.voxel[j2].x == overlay.voxel[j1].x &&
		 overlay_temp.voxel[j2].y == overlay.voxel[j1].y &&
		 overlay_temp.voxel[j2].z == overlay.voxel[j1].z ) found = 1;
	  if ( found ) number_overlap++;
	}
      /* Allocate a new pixel array */
      overlay_new.nvoxels = overlay.nvoxels + overlay_temp.nvoxels - number_overlap;
      overlay_new.voxel   = ipoint3d_vector(overlay_new.nvoxels);
      overlay_new.weight  = float_vector(overlay_new.nvoxels);
      /* Put the pixels of the pre-existing overlay into the new array. */
      for (j=0; j<overlay.nvoxels; j++)
	{
	  overlay_new.voxel[j] = overlay.voxel[j];
	  overlay_new.weight[j]= overlay.weight[j];
	}
      // Add pixels of the new overlay into the new array IF there is not overlap with the old overlay
      number_new = 0;
      for (j2=0; j2<overlay_temp.nvoxels; j2++)
	{
	  found = 0;
	  for (j1=0; j1<overlay.nvoxels; j1++)
	    {
	      if ( overlay_temp.voxel[j2].x == overlay.voxel[j1].x &&
		   overlay_temp.voxel[j2].y == overlay.voxel[j1].y &&
		   overlay_temp.voxel[j2].z == overlay.voxel[j1].z )
		{
		  found = 1;
		  weight = overlay.weight[j1];
		}
	    }
	  if ( !found )
	    {
	      overlay_new.voxel[overlay.nvoxels+number_new]  = overlay_temp.voxel[j2];
	      overlay_new.weight[overlay.nvoxels+number_new] = overlay_temp.weight[j2];
	      number_new++;
	    }
	  else
	    overlay_new.weight[overlay.nvoxels+number_new] = MAX(weight,overlay_temp.weight[j2]);
	}
      free_ipoint3d_vector(overlay_temp.voxel);
      free_float_vector(overlay_temp.weight);
      free_ipoint3d_vector(overlay.voxel);
      free_float_vector(overlay.weight);

      overlay.nvoxels = overlay_new.nvoxels;
      overlay.voxel  = overlay_new.voxel;
      overlay.weight = overlay_new.weight;
    }
  else if ( overlay.io_logic == 2 )   // LOGICAL AND
    {
      // Count the number of overlapping pixels in the two overlays.
      number_overlap = 0;
      for (j1=0; j1<overlay.nvoxels; j1++)
	{
	  found = 0;
	  for (j2=0; j2<overlay_temp.nvoxels; j2++)
	    if ( overlay_temp.voxel[j2].x == overlay.voxel[j1].x &&
		 overlay_temp.voxel[j2].y == overlay.voxel[j1].y &&
		 overlay_temp.voxel[j2].z == overlay.voxel[j1].z ) found = 1;
	  if ( found ) number_overlap++;
	}
      /* Allocate a new pixel array */
      overlay_new.nvoxels = number_overlap;
      overlay_new.voxel   = ipoint3d_vector(overlay_new.nvoxels);
      overlay_new.weight  = float_vector(overlay_new.nvoxels);
      /* Put pixels into the new overlay IF there is overlap with the old overlay. */ 
      number_new = 0;
      for (j2=0; j2<overlay_temp.nvoxels; j2++)
	{
	  found = 0;
	  for (j1=0; j1<overlay.nvoxels; j1++)
	    if ( overlay_temp.voxel[j2].x == overlay.voxel[j1].x &&
		 overlay_temp.voxel[j2].y == overlay.voxel[j1].y &&
		 overlay_temp.voxel[j2].z == overlay.voxel[j1].z ) found = 1;
	  if ( found )
	    {
	      overlay_new.voxel[number_new]  = overlay_temp.voxel[j2];
	      overlay_new.weight[number_new] = overlay_temp.weight[j2] * overlay.weight[j1];
	      number_new++;
	    }
	}
      free_ipoint3d_vector(overlay_temp.voxel);
      free_float_vector(overlay_temp.weight);
      free_ipoint3d_vector(overlay.voxel);
      free_float_vector(overlay.weight);

      overlay.nvoxels = overlay_new.nvoxels;
      overlay.voxel   =  overlay_new.voxel;
      overlay.weight  =  overlay_new.weight;
    }
  else if ( overlay.io_logic == 3 )   // LOGICAL NOT
    {
      // Count the number of overlapping pixels in the two overlays.
      number_overlap = 0;
      for (j1=0; j1<overlay.nvoxels; j1++)
	{
	  found = 0;
	  for (j2=0; j2<overlay_temp.nvoxels; j2++)
	    if ( overlay_temp.voxel[j2].x == overlay.voxel[j1].x &&
		 overlay_temp.voxel[j2].y == overlay.voxel[j1].y &&
		 overlay_temp.voxel[j2].z == overlay.voxel[j1].z ) found = 1;
	  if ( found ) number_overlap++;
	}
      /* Allocate a new pixel array */
      overlay_new.nvoxels = overlay.nvoxels - number_overlap;
      overlay_new.voxel   = ipoint3d_vector(overlay_new.nvoxels);
      overlay_new.weight  = float_vector(overlay_new.nvoxels);
      // Copy existing pixels into the new array IF they do not overlap with new overlay.
      number_new = 0;
      for (j1=0; j1<overlay.nvoxels; j1++)
	{
	  found = 0;
	  for (j2=0; j2<overlay_temp.nvoxels; j2++)
	    if ( overlay_temp.voxel[j2].x == overlay.voxel[j1].x &&
		 overlay_temp.voxel[j2].y == overlay.voxel[j1].y &&
		 overlay_temp.voxel[j2].z == overlay.voxel[j1].z ) found = 1;
	  if ( !found )
	    {
	      overlay_new.voxel[number_new]  = overlay.voxel[j1];
	      overlay_new.weight[number_new] = overlay.weight[j1];
	      number_new++;
	    }
	}
      free_ipoint3d_vector(overlay_temp.voxel);
      free_float_vector(overlay_temp.weight);
      free_ipoint3d_vector(overlay.voxel);
      free_float_vector(overlay.weight);

      overlay.nvoxels = overlay_new.nvoxels;
      overlay.voxel   = overlay_new.voxel;
      overlay.weight  = overlay_new.weight;
    }
  else
    {
      printf("Programming error in function `read_ovl_file'.\n");
      return(1);
    }

  printf("%td overlay pixels read from file.\n",overlay.nvoxels);
  return(0);
}

int write_ovl( char *file_name_in )
{
  FILE *file;
  char file_name[LEN_STR];
  int ix, iy, iz;JIP_INDEX j;
  extern OVERLAY overlay;

  /*
  ** Pick off the file name.
  */
  if ( sscanf(file_name_in,"%s",file_name) != 1 )
    {
      jip_text(2,"A file name is a mandatory argument here.");
      return(1);
    }
  /*
  ** Open the file and write the overlays.
  */
  if ( !(file = fopen(file_name,"w")) )
    {
      jip_text(2,"I cannot open the output OVL file.");
      return(1);
    }
  /*
  ** Make sure that an overlay has been read.
  */
  if ( overlay.nvoxels == 0 )
    {
      jip_text(1,"The OVL is not active or is void, but the file was created anyway.");
      fclose(file);
      return(1);
    }
  for (j=0; j<overlay.nvoxels; j++)
    {
      ix = overlay.voxel[j].x;
      iy = overlay.voxel[j].y;
      iz = overlay.voxel[j].z;
      fprintf(file,"%d %d %d %g\n",ix,iy,iz,overlay.weight[j]);
    }
  fclose(file);
  /*
  ** User info:
  */
  printf("%td voxels were included in the overlay file.\n",overlay.nvoxels);
  return(1);
}

void delete_overlay()
{
  extern OVERLAY overlay;

  if ( overlay.nvoxels == 0 )
    return;
  else
    {
      free_ipoint3d_vector(overlay.voxel);
      free_float_vector(overlay.weight);
      overlay.nvoxels = 0;
    }
}

int create_bitmap_from_overlay( char *new_stack_name )
{
  IPOINT3D dim;
  int nt, ix, iy, iz;JIP_INDEX index, j;
  struct data_stack *new_stack;
  extern struct volume_list *volume_list_root;
  extern OVERLAY overlay;

  if ( overlay.nvoxels == 0 )
    {
      printf("Error: an overlay is not defined.\n");
      return(1);
    }

  // Get dimensions.
  if ( inconsistent_3d_dimensions(&dim,&nt) ) return(1);

  // Install the new_stack in memory.
  new_stack = install_stack( new_stack_name, volume_list_root->stack, dim, 1 );
  new_stack->data_type = 1;
  new_stack->npts_volume = new_stack->n4d;
  for (j=0; j<new_stack->n4d; j++)
    new_stack->SpaceTime.fcData[j].real = new_stack->SpaceTime.fcData[j].imag = 0.;

  for (j=0; j<overlay.nvoxels; j++)
    {
      ix = overlay.voxel[j].x;
      iy = overlay.voxel[j].y;
      iz = overlay.voxel[j].z;
      index = index4d(ix,iy,iz,0,new_stack);
      if ( index > new_stack->n3d )
	{
	  printf("(x,y,z) = (%d,%d,%d), index = %td, total = %td\n",
		 ix,iy,iz,index,new_stack->n3d);
	  jip_text(2,"The overlay falls outside the volume boundaries.");
	  return(1);
	}
      else
	//	new_stack->SpaceTime.fcData[index].real = 1.;
	new_stack->SpaceTime.fcData[index].real = overlay.weight[j];
    }
  return(0);
}

int create_overlay_from_bitmap()
{
  double round();
  IPOINT3D dim;
  int nt, jx, jy;JIP_INDEX nvoxels, index, j;int jz;
  float amp;
  BOOL out_of_range;
  extern struct volume_list *volume_list_root;
  extern OVERLAY overlay;

  // Get dimensions.
  if ( inconsistent_3d_dimensions(&dim,&nt) ) return(1);
  if ( nt != 1 )
    {
      printf("Error: only pass a single volume to this function.\n");
      return(1);
    }

  // Count the number of non-zero voxels.
  out_of_range = FALSE;
  for (j=0, nvoxels=0; j<volume_list_root->n3d; j++)
    {
      amp = volume_amp(volume_list_root,j);
      amp = round((double)(amp*100.))/100.;  // 1% digital resolution
      if ( amp > 1. )
	{
	  out_of_range = TRUE;
	  amp = 1.;
	}
      if ( amp < 0. )
	{
	  out_of_range = TRUE;
	  amp = 0.;
	}
      if ( amp > 0.1 ) // 10% threshold
	nvoxels++;
    }
  if ( out_of_range )
    printf("Warning: use normalized bitmaps (range:0 to 1)\n");

  // Allocate and fill the overlay.
  if ( overlay.voxel )  free_ipoint3d_vector(overlay.voxel);
  if ( overlay.weight ) free_float_vector(overlay.weight);
  overlay.voxel  = ipoint3d_vector(nvoxels);
  overlay.weight = float_vector(nvoxels);
  for (jz=0, overlay.nvoxels=0; jz<volume_list_root->dim.z; jz++)
    {
      for (jy=0; jy<volume_list_root->dim.y; jy++)
	{
	  for (jx=0; jx<volume_list_root->dim.x; jx++)
	    {
	      index = index3d(jx,jy,jz,volume_list_root);
	      amp = volume_amp(volume_list_root,index);
	      amp = round((double)(amp*100.))/100.;  // 1% digital resolution
	      if ( amp > 1. )
		amp = 1.;
	      else if ( amp < 0. )
		amp = 0.;
	      if ( amp > 0.1 ) // 10% threshold
		{
		  overlay.voxel[overlay.nvoxels].x = jx;
		  overlay.voxel[overlay.nvoxels].y = jy;
		  overlay.voxel[overlay.nvoxels].z = jz;
		  overlay.weight[overlay.nvoxels]  = amp;
		  overlay.nvoxels++;
		}
	    }
	}
    }
  return(0);
}

int mirror_overlay_in_x( int iDimensionX )
{
  JIP_INDEX j;
  OVERLAY overlay_temp;
  extern OVERLAY overlay;
  
  printf("in mirror function\n");

  if ( overlay.nvoxels == 0 )
    {
      printf("Error: no overlay exists in memory.\n");
      return(1);
    }

  // Create a temporary overlay with 2ce as many points.
  overlay_temp.nvoxels = 2 * overlay.nvoxels;
  overlay_temp.voxel   = ipoint3d_vector(overlay_temp.nvoxels);
  overlay_temp.weight  = float_vector(overlay_temp.nvoxels);

  // Copy the overlay. Reverse y indices for the new voxels.
  for (j=0; j<overlay_temp.nvoxels; j++)
    {
      if ( j<overlay.nvoxels )
	{
	  overlay_temp.voxel[j]  = overlay.voxel[j];
	  overlay_temp.weight[j] = overlay.weight[j];
	}
      else
	{
	  overlay_temp.voxel[j]  = overlay.voxel[j-overlay.nvoxels];
	  overlay_temp.weight[j] = overlay.weight[j-overlay.nvoxels];
	  // Mirror the voxel in y
	  overlay_temp.voxel[j].x = iDimensionX - overlay_temp.voxel[j].x - 1;
	  if ( overlay_temp.voxel[j].x < 0 )
	    {
	      printf("Error: voxel index (%d) < 0 during mirror; check x dimension.\n",
		     overlay_temp.voxel[j].x);
	      free_ipoint3d_vector(overlay_temp.voxel);
	      free_float_vector(overlay_temp.weight);
	      return(1);
	    }
	}
    }

  // Release arrays from the standard overlay.
  if ( overlay.nvoxels != 0 )
    {
      free_ipoint3d_vector(overlay.voxel);
      free_float_vector(overlay.weight);
    }
  // Copy the temporary overlay into the standard one.
  overlay.nvoxels = overlay_temp.nvoxels;
  overlay.voxel   = ipoint3d_vector(overlay.nvoxels);
  overlay.weight  = float_vector(overlay.nvoxels);
  for (j=0; j<overlay.nvoxels; j++)
    {
      overlay.voxel[j] = overlay_temp.voxel[j];
      overlay.weight[j]= overlay_temp.weight[j];
    }
  // Release the temporary arrays.
  free_ipoint3d_vector(overlay_temp.voxel);
  free_float_vector(overlay_temp.weight);

  printf("%td overlay pixels now in overlay.\n",overlay.nvoxels);
  return(0);
}
