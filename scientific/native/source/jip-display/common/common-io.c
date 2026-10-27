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
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "util.h"
#include "common-display.h"

#define MIN_HEADER_SIZE 348
#define NII_HEADER_SIZE 352


int read_SpaceTime_data( BOOL Verbose, int common_dimensions, struct data_stack *stack1,
			 struct file_list *file_info, int *nfiles )
{
  struct XD_HEADER header;
  struct file_list *file_current;
  static IPOINT3D dim;
  static POINT3D  resolution;
  static int first_time=1, data_type_save;
  JIP_INDEX lAllocate;

  // First read the dimensions for each file.
  for (file_current = file_info; file_current; file_current=file_current->next)
    {
      // Read the file type and dimensions
      if ( read_volume_header(Verbose, file_current->file_name, &header) ) return(1);
      if ( first_time )
	{
	  dim = header.dim;
	  data_type_save = header.data_type;
	  resolution   = header.resolution;
	  first_time = 0;
	}
      else
	{
	  if ( header.data_type != data_type_save && stack1 != &activation_stack )
	    {
	      printf("Don't mix real and complex data types between files.\n");
	      return(1);
	    }
	  else if ( stack1 == &activation_stack && header.data_type != 0 )
	    {
	      printf("Activation overlays cannot be complex.\n");
	      return(1);
	    }
	  else if ( common_dimensions )
	    {
	      if ( header.dim.x!=dim.x || header.dim.y!=dim.y || header.dim.z!=dim.z )
		{
		  printf("Error: dimensions for %s (%d,%d,%d)\n",
			 file_current->file_name,header.dim.x,header.dim.y,header.dim.z);
		  printf("don't match previous image dimensions (%d,%d,%d)\n",dim.x,dim.y,dim.z);
		  return(1);
		}
	      else if ( header.resolution.x != resolution.x ||
			header.resolution.y != resolution.y ||
			header.resolution.z != resolution.z )
		{
		  printf("Warning: resolution for %s (%g,%g,%g)\n",
			 file_current->file_name,
			 header.resolution.x,
			 header.resolution.y,
			 header.resolution.z);
		  printf("doesn't match previous resolution (%g,%g,%g)\n",
			 header.resolution.x,
			 header.resolution.y,
			 header.resolution.z);
		}
	    }
	}
      stack1->dim = header.dim;
      stack1->n2d = header.n2d;
      stack1->n3d = stack1->npts_volume = header.n3d;
      stack1->n4d = header.n4d;
      stack1->resolution   = header.resolution;
      if ( stack1->resolution_t == 0. )
	stack1->resolution_t = header.resolution_t;
      stack1->origin       = header.origin;
      stack1->direction    = header.direction;
      stack1->fov          = header.fov;
      stack1->ijk_to_xyz   = header.ijk_to_xyz;
      stack1->xyz_to_ijk   = header.xyz_to_ijk;
      stack1->off_diagonal = header.off_diagonal;
      stack1->registration = header.registration;
      stack1->data_type    = header.data_type;
      if ( stack1->data_type == 1 || stack1->data_type == 2 )
	stack1->npts_volume *= 2;
      else if ( stack1->data_type == 3 )
	stack1->npts_volume *= 3;
      // Fill info in the file list.
      file_current->header = header;
      if ( file_current->type == 3 ) // S file from GLM
	file_current->nt = 1;        // just take 1st image
      else
	file_current->nt = header.nt;
      file_current->start_image = stack1->nt;
      stack1->nt += file_current->nt;
      file_current->end_image   = stack1->nt-1;
    }

  // Allocate memory for the stack data array.
  stack1->n4d = jip_product(stack1->n3d,stack1->nt);
  lAllocate = stack1->npts_volume*stack1->nt;
  if ( lAllocate != 0 )
    stack1->SpaceTime.fData = float_vector_long(lAllocate);
  // Read each file, appending time points into the single stack.
  for (file_current = file_info, *nfiles=0; file_current;
       file_current=file_current->next, (*nfiles)++)
    {
      if ( file_current->header.file_type == FREESURFER )
	{
	  if ( read_freesurfer_file(stack1, file_current) )
	    return(1);
	}
      else
	{
	  if ( read_xdisplay_file(stack1, file_current) )
	    return(1);
	}
    }

  return(0);
}

int read_xdisplay_file( struct data_stack *stack1, struct file_list *file_info )
{
  FILE *file;
  JIP_INDEX offset_spacetime, offset_volume, j, jt;
  float *fdata;
  short *idata;
  unsigned short *iudata;
  unsigned char *i8data;
  unsigned long red, green, blue, pixel;
  int *iidata;
  int swap_bytes;JIP_INDEX output_info, error;

  swap_bytes  = machine_byte_order() != file_info->header.byte_order;
  output_info = stack1->npts_volume*file_info->nt > 1000000 && file_info->nt > 10;

 /* Open the binary data file. */
  if ( !(file = fopen(file_info->file_name,"r")) )
    {
      printf("\nError: cannot open file %s.\n",file_info->file_name);
      return(1);
    }

  // Go to the first data voxel
  error = jip_seek(file, (JIP_INDEX)(file_info->header.voxel_offset), SEEK_SET);
  if ( error )
    {
      fprintf(stderr, "\nError doing jip_seek() to %td in data file %s\n",
	      (JIP_INDEX)(file_info->header.voxel_offset), file_info->file_name);
      exit(1);
    }

  if ( file_info->header.storage_type == BFLOAT )      // bfloat
    // floating point data could be read into fSpaceTimeData directly,
    // but allocate a temporary array for consistency with other formats (below).
    fdata = float_vector(stack1->npts_volume);
  else if ( file_info->header.storage_type == BSHORT ) // bshort
    idata = short_vector(stack1->npts_volume);
  else if ( file_info->header.storage_type == BLONG )  // blong
    iidata = int_vector(stack1->npts_volume);
  else if ( file_info->header.storage_type == USHORT ) // unsigned short
    iudata = unsigned_short_vector(stack1->npts_volume);
  else if ( file_info->header.storage_type == UINT8 )  // unsigned integer with 1 byte (8 bits)
    i8data = unsigned_char_vector(stack1->npts_volume);

  offset_spacetime = file_info->start_image * stack1->npts_volume;
  // Read the array.
  for (jt=0; jt<file_info->nt; jt++)
    {
      if ( output_info && jt%(file_info->nt/10) == 0 )
	{
	  printf("\r%s: %td %c",file_info->file_name,
		 10*jt/(file_info->nt/10),37);
	  fflush(stdout);
	}
      if ( file_info->header.storage_type == BFLOAT )      // bfloat
	error = fread((char *)fdata,sizeof(float),stack1->npts_volume,file) != stack1->npts_volume;
      else if ( file_info->header.storage_type == BSHORT ) // bshort
	error = fread((char *)idata,sizeof(short),stack1->npts_volume,file) != stack1->npts_volume;
      else if ( file_info->header.storage_type == USHORT ) // unsigned short
	error = fread((char *)iudata,sizeof(unsigned short),stack1->npts_volume,file) != stack1->npts_volume;
      else if ( file_info->header.storage_type == BLONG )  // blong
	error = fread((char *)iidata,sizeof(int),stack1->npts_volume,file) != stack1->npts_volume;
      else if ( file_info->header.storage_type == UINT8 ) // unsigned char
	error = fread((char *)i8data,sizeof(char),stack1->npts_volume,file) != stack1->npts_volume;
      if ( error )
	{
	  printf("Error reading the data from file %s\n",file_info->file_name);
	  return(1);
	}
      // Copy the integer array into the stack floating point array.
      offset_volume = jt * stack1->npts_volume;
      for (j=0; j<stack1->npts_volume; j++)
	{
	  if ( file_info->header.storage_type == BFLOAT )      // bfloat
	    {
	      if ( swap_bytes ) swap_32( &(fdata[j]) );
	      stack1->SpaceTime.fData[j+offset_spacetime+offset_volume] = fdata[j];
	    }
	  else if ( file_info->header.storage_type == BSHORT ) // bshort
	    {
	      if ( swap_bytes ) swap_16( &(idata[j]) );
	      stack1->SpaceTime.fData[j+offset_spacetime+offset_volume] = idata[j];
	    }
	  else if ( file_info->header.storage_type == BLONG )  // blong
	    {
	      if ( swap_bytes ) swap_32( &(iidata[j]) );
	      stack1->SpaceTime.fData[j+offset_spacetime+offset_volume] = iidata[j];
	    }
	  else if ( file_info->header.storage_type == USHORT ) // unsigned short
	    {
	      if ( swap_bytes ) swap_16( &(iudata[j]) );
	      stack1->SpaceTime.fData[j+offset_spacetime+offset_volume] = iudata[j];
	    }
	  else if ( file_info->header.storage_type == UINT8 ) // unsigned char
	    {
	      // no need to swap, since only 1 byte
	      if ( file_info->header.data_type == 3 )
		{
		  red   = i8data[j];
		  green = i8data[j+1];
		  blue  = i8data[j+2];
		  pixel = (red<<16) + (green<<8) + blue;   // R G B
		  stack1->SpaceTime.fData[j/3+offset_spacetime+offset_volume] = pixel;
		  j += 2;  // move forward to the start of the next RGB triplet
		}
	      else
		stack1->SpaceTime.fData[j+offset_spacetime+offset_volume] = i8data[j];
	    }
	  // Scale the data if necessary.
	  if ( file_info->header.scale_slope != 0. )
	    {
	      stack1->SpaceTime.fData[j+offset_spacetime+offset_volume] *= file_info->header.scale_slope;
	      stack1->SpaceTime.fData[j+offset_spacetime+offset_volume] += file_info->header.scale_offset;
	    }
	}
      // Rotate the data if necessary: pass 1 volume.
      //      QuaternionRotation(stack1, file_info, file_info->start_image + jt);
    }

  // Done reading, so free array and close file.
  if ( file_info->header.storage_type == BFLOAT )      // bfloat
    free_float_vector(fdata);
  else if ( file_info->header.storage_type == BSHORT ) // bshort
    free_short_vector(idata);
  else if ( file_info->header.storage_type == BLONG )  // blong
    free_int_vector(iidata);
  else if ( file_info->header.storage_type == USHORT ) // unsigned short
    free_unsigned_short_vector(iudata);
  else if ( file_info->header.storage_type == UINT8 ) // unsigned char
    free_unsigned_char_vector(i8data);
  fclose(file);
  if ( output_info )
    {
      printf("\r%s: 100 %c\n",file_info->file_name,37);
      fflush(stdout);
    }
  return(0);
}

int read_freesurfer_file( struct data_stack *stack1, struct file_list *file_info )
{
  FILE *file;
  char extension[LEN_STR], slice_file_name[LEN_STR], slice_name[LEN_STR];
  int jz, jt;JIP_INDEX j;JIP_INDEX npts2d;JIP_INDEX offset_spacetime, offset_slice;
  short *idata;
  float *fdata;
  int swap_bytes, output_info;

  npts2d = ((JIP_INDEX)(stack1->dim.x)) * stack1->dim.y;  // # points per read (= one 2D slice)
  offset_spacetime = file_info->start_image * stack1->npts_volume;

  swap_bytes  = machine_byte_order() != file_info->header.byte_order;
  output_info = ((JIP_INDEX)(npts2d))*stack1->dim.z*file_info->nt > 1000000 && stack1->dim.z > 1;

  if ( file_info->header.storage_type == BFLOAT )
    {
      fdata = float_vector(npts2d);
      strcpy(extension,".bfloat");
    }
  else
    {
      idata = short_vector(npts2d);
      strcpy(extension,".bshort");
    }

  // Loop over all slices, which are saved in separate files.
  for (jz=0; jz<stack1->dim.z; jz++)
    {
      // Create the file name.
      if ( jz < 10 )
	sprintf(slice_name,"00%d",jz);
      else if ( jz < 100 )
	sprintf(slice_name,"0%d",jz);
      else
	sprintf(slice_name,"%d",jz);
      strcpy(slice_file_name,file_info->file_name);
      strcat(slice_file_name,slice_name);
      strcat(slice_file_name,extension);

      // Open the binary data file.
      if ( !(file = fopen(slice_file_name,"r")) )
	{
	  printf("\nError: cannot open file %s\n",file_info->file_name);
	  return(1);
	}

      if ( output_info )
	{
	  printf("\r%s: slice %d",file_info->file_name,jz);
	  fflush(stdout);
	}
      // Read the array.
      for (jt=0; jt<file_info->nt; jt++)
	{
	  if ( file_info->header.storage_type == BSHORT )
	    {
	      if ( fread((char *)idata,sizeof(short),npts2d,file) != npts2d )
		{
		  printf("Error reading the short data from file %s\n",file_info->file_name);
		  return(1);
		}
	      offset_slice = jt * stack1->npts_volume + ((JIP_INDEX)(jz)) * stack1->dim.x*stack1->dim.y;
	      // Copy the integer array into the stack floating point array.
	      for (j=0; j<npts2d; j++)
		{
		  if ( swap_bytes ) swap_16( &(idata[j]) );
		  stack1->SpaceTime.fData[j+offset_spacetime+offset_slice] = idata[j];
		}
	    }
	  else
	    {
	      if ( fread((char *)fdata,sizeof(float),npts2d,file) != npts2d )
		{
		  printf("Error reading the short data from file %s\n",file_info->file_name);
		  return(1);
		}
	      offset_slice = jt * stack1->npts_volume + ((JIP_INDEX)(jz)) * stack1->dim.x*stack1->dim.y;
	      // Copy the integer array into the stack floating point array.
	      for (j=0; j<npts2d; j++)
		{
		  if ( swap_bytes ) swap_32( &(fdata[j]) );
		  stack1->SpaceTime.fData[j+offset_spacetime+offset_slice] = fdata[j];
		}
	    }
	}
      // Done reading, so close file.
      fclose(file);
    }
  if ( file_info->header.storage_type == BSHORT )
    free_short_vector(idata);
  else
    free_float_vector(fdata);
  if ( output_info )
    {
      printf("\r%s: 100 %c\n",file_info->file_name,37);
      fflush(stdout);
    }
  return(0);
}

void QuaternionRotation(struct data_stack *stack1,   // pass dimensions
			struct file_list *file_info, // pass rotation values
			int it )                     // time point
{ // this function is basically copied from the nifti1_io.c example function.
  float rotation[4][4];
  float qfac, a, b, c, d;
  int jx, jy, jz, ix, iy, iz;JIP_INDEX index_orig, index_new;
  float *data_temp;
  
  qfac = file_info->header.qfac;
  if ( qfac == 0. ) return;

  b    = file_info->header.quaternion.x;
  c    = file_info->header.quaternion.y;
  d    = file_info->header.quaternion.z;
  // Quaternion values of b,c,d are passed. Compute a.
  a = 1.0l - (b*b + c*c + d*d);
  if( a < 1.e-7l )
    { // special case
      a = 1.0l / sqrt(b*b+c*c+d*d);
      b *= a ; c *= a ; d *= a ;   // normalize (b,c,d) vector
      a = 0.0l;                    // a = 0 ==> 180 degree rotation
    }
  else
    a = sqrt(a) ;                  // angle = 2*arccos(a)
  // Assemble the rotation matrix.
  rotation[0][0] =        (a*a+b*b-c*c-d*d);
  rotation[0][1] = 2.0l * (b*c-a*d        );
  rotation[0][2] = 2.0l * (b*d+a*c        );
  rotation[1][0] = 2.0l * (b*c+a*d        );
  rotation[1][1] =        (a*a+c*c-b*b-d*d);
  rotation[1][2] = 2.0l * (c*d-a*b        );
  rotation[2][0] = 2.0l * (b*d-a*c        );
  rotation[2][1] = 2.0l * (c*d+a*b        );
  rotation[2][2] =        (a*a+d*d-c*c-b*b);
  // last row is always [ 0 0 0 1 ]
  rotation[3][0] = rotation[3][1] = rotation[3][2] = 0.0 ;
  rotation[3][3] = 1.0;

  // Create temporary working matrices.
  data_temp = float_vector(stack1->npts_volume);
  for (jz=0; jz<stack1->dim.z; jz++)
    {
      for (jy=0; jy<stack1->dim.y; jy++)
	{
	  for (jx=0; jx<stack1->dim.x; jx++)
	    {
	      if ( stack1->data_type == 0 )
		{ // magnitude data
		  index_orig = index4d(jx,jy,jz,it);
		  data_temp[index_orig] = stack1->SpaceTime.fData[index_orig];
		}
	      else
		{ // complex pairs
		  index_orig = index4d(2*jx,jy,jz,it);
		  data_temp[index_orig]   = stack1->SpaceTime.fData[index_orig];
		  data_temp[index_orig+1] = stack1->SpaceTime.fData[index_orig+1];
		}	
    }
	}
    }
  // Rotate the matrix.
  for (jz=0; jz<stack1->dim.z; jz++)
    {
      for (jy=0; jy<stack1->dim.y; jy++)
	{
	  for (jx=0; jx<stack1->dim.x; jx++)
	    {
	      ix = rotation[0][0]*jx + rotation[0][1]*jy +        rotation[0][2]*jz;
	      iy = rotation[1][0]*jx + rotation[1][1]*jy +        rotation[1][2]*jz;
	      iz = rotation[2][0]*jx + rotation[2][1]*jy + qfac * rotation[2][2]*jz;
	      printf("rotate (%d, %d, %d) to (%d, %d, %d)\n",jx,jy,jz,ix,iy,iz);
	      if ( stack1->data_type == 0 )
		{ // magnitude data
		  index_orig = index4d(jx,jy,jz,it);
		  index_new  = index4d(ix,iy,iz,it);
		  printf("(old,new) = (%td, %td)\n",index_orig,index_new);
		  stack1->SpaceTime.fData[index_new] = data_temp[index_orig];
		}
	      else
		{ // complex pairs
		  index_orig = index4d(2*jx,jy,jz,it);
		  index_new  = index4d(2*ix,iy,iz,it);
		  stack1->SpaceTime.fData[index_new]   = data_temp[index_orig];
		  stack1->SpaceTime.fData[index_new+1] = data_temp[index_orig+1];
		}
	    }
	}
    }
  free_float_vector(data_temp);
}

int read_overlay_list( char *list_file_name )
{
  FILE *file;
  char whole_line[LEN_STR], ovl_name[LEN_STR], file_name[LEN_STR], color[LEN_STR];
  char *ptr;
  int i1, novl, icolor;JIP_INDEX j;
  struct ovl_list *ovl_current, *ovl_last;

  // Open the file containing the list of runs, GLM files, ...
  if ( !(file = fopen(list_file_name,"r")) )
    {
      printf("Error: cannot open the file %s.\n",list_file_name);
      return(1);
    }

  // Beginning read lines of the overlay list file.
  // Lines should be:
  // overlay_name overlay_file [optional overlay color]
  novl = 0;
  while ( get_line_with_comment(file, whole_line, LEN_STR) )
    {
      if ( sscanf(whole_line,"%d",&i1) < 0 )  // ... then this is a blank line
	continue;
      // This line should contain an overlay name and a file name.
      if ( sscanf(whole_line,"%s",ovl_name) != 1 )
	{
	  printf("Error reading the overlay name for overay %d.\n",novl+1);
	  return(1);
	}
      // Update the argument to the next string position.
      ptr = next_non_white( whole_line + jip_int_count((JIP_INDEX)strlen(ovl_name)) );
      // Read the overlay file name.
      if ( sscanf(ptr,"%s",file_name) != 1 )
	{
	  printf("Error reading the file name for overlay %d.\n",novl+1);
	  return(1);
	}
      // Update the argument to the next string position.
      ptr = next_non_white( ptr + jip_int_count((JIP_INDEX)strlen(file_name)) );
      // Try to read a color for the overlay.
      icolor = 1;
      if ( jip_int_count((JIP_INDEX)strlen(ptr)) > 1 )
	{
	  if ( sscanf(ptr,"%s",color) == 1 )
	    {
	      if ( !strcmp(color,"cyan") )
		icolor = COLOR_CYAN;
	      else if ( !strcmp(color,"red") )
		icolor = COLOR_RED;
	      else if ( !strcmp(color,"green") )
		icolor = COLOR_GREEN;
	      else if ( !strcmp(color,"blue") )
		icolor = COLOR_BLUE;
	      else if ( !strcmp(color,"white") )
		icolor = COLOR_WHITE;
	      else if ( !strcmp(color,"purple") )
		icolor = COLOR_PURPLE;
	      else if ( !strcmp(color,"yellow") )
		icolor = COLOR_YELLOW;
	      else if ( !strcmp(color,"orange") )
		icolor = COLOR_ORANGE;
	    }
	}
      // Find the last entry in the ovl list.
      ovl_current = &ovl_root;
      while ( ovl_current )
	{
	  ovl_last    = ovl_current;
	  ovl_current = ovl_current->next;
	}
      // Allocate a new member of the list;
      if ( !(ovl_current = (struct ovl_list *)malloc(sizeof(struct ovl_list))) )
	{
	  printf("Error: memory problem creating a new ovl structure in the list.\n");
	  exit(1);
	}
      ovl_last->next = ovl_current;
      ovl_current->next = NULL;

      // Initialize the new list member.
      strcpy(ovl_current->name,ovl_name);
      ovl_current->color  = icolor;
      ovl_current->npts = 0;
      ovl_current->active = FALSE;
      ovl_current->next = NULL;
      // "over-allocate" these arrays for simplicity (npts dimensions < n3d)
      ovl_current->voxel  = ipoint3d_vector(mosaic.stack->n3d);
      ovl_current->weight = float_vector(mosaic.stack->n3d);
      for (j=0; j<mosaic.stack->n3d; j++)
	{
	  ovl_current->weight[j] = 0.;
	  ovl_current->voxel[j].x = ovl_current->voxel[j].y = ovl_current->voxel[j].z = 0;
	}
      read_overlay_file( file_name, ovl_current );
      novl++;
    }
  return(0);
}

int read_wire_list( char *list_file_name )
{
  FILE *file;
  char whole_line[LEN_STR], wire_name[LEN_STR], file_name[LEN_STR], color[LEN_STR];
  char *ptr;
  int i1, nwire, icolor;
  struct wire_list *wire_current, *wire_last;

  // Open the file containing the list of runs, GLM files, ...
  if ( !(file = fopen(list_file_name,"r")) )
    {
      printf("Error: cannot open the file %s.\n",list_file_name);
      return(1);
    }

  // Beginning read lines of the wire list file.
  // Lines should be:
  // wire_name wire_file [optional wire color]
  nwire = 0;
  while ( get_line_with_comment(file, whole_line, LEN_STR) )
    {
      if ( sscanf(whole_line,"%d",&i1) < 0 )  // ... then this is a blank line
	continue;
      // This line should contain an wire name and a file name.
      if ( sscanf(whole_line,"%s",wire_name) != 1 )
	{
	  printf("Error reading the wire name for overay %d.\n",nwire+1);
	  return(1);
	}
      // Update the argument to the next string position.
      ptr = next_non_white( whole_line + jip_int_count((JIP_INDEX)strlen(wire_name)) );
      // Read the wire file name.
      if ( sscanf(ptr,"%s",file_name) != 1 )
	{
	  printf("Error reading the file name for wire %d.\n",nwire+1);
	  return(1);
	}
      // Update the argument to the next string position.
      ptr = next_non_white( ptr + jip_int_count((JIP_INDEX)strlen(file_name)) );
      // Try to read a color for the wire.
      icolor = 1;
      if ( jip_int_count((JIP_INDEX)strlen(ptr)) > 1 )
	{
	  if ( sscanf(ptr,"%s",color) == 1 )
	    {
	      if ( !strcmp(color,"cyan") )
		icolor = COLOR_CYAN;
	      else if ( !strcmp(color,"red") )
		icolor = COLOR_RED;
	      else if ( !strcmp(color,"green") )
		icolor = COLOR_GREEN;
	      else if ( !strcmp(color,"blue") )
		icolor = COLOR_BLUE;
	      else if ( !strcmp(color,"white") )
		icolor = COLOR_WHITE;
	      else if ( !strcmp(color,"purple") )
		icolor = COLOR_PURPLE;
	      else if ( !strcmp(color,"yellow") )
		icolor = COLOR_YELLOW;
	      else if ( !strcmp(color,"orange") )
		icolor = COLOR_ORANGE;
	    }
	}
      // Find the last entry in the wire list.
      wire_current = &wire_root;
      while ( wire_current )
	{
	  wire_last    = wire_current;
	  wire_current = wire_current->next;
	}
      // Allocate a new member of the list;
      if ( !(wire_current = (struct wire_list *)malloc(sizeof(struct wire_list))) )
	{
	  printf("Error: memory problem creating a new wire structure in the list.\n");
	  exit(1);
	}
      wire_last->next = wire_current;
      wire_current->next = NULL;
      // Initialize the new list member.
      strcpy(wire_current->name,wire_name);
      wire_current->npts = 0;
      read_wire_file( file_name, wire_current, icolor );
      nwire++;
    }
  return(0);
}

int read_overlay_file( char *file_name, struct ovl_list *ovl )
{
  FILE *file;
  char whole_line[LEN_STR];
  IPOINT3D voxel;
  JIP_INDEX index;
  float weight;
  
  // Open the ovl file.
  if ( !(file = fopen(file_name,"r")) )
    {
      printf("Error opening file %s\n",file_name);
      return(1);
    }
  // The file format can be one of the following
  //      x y z weight
  //      x y z             (weight=1)
  // Now read the overlay and set those pixels to 1.
  while ( get_line_with_comment(file, whole_line, LEN_STR) )
    { // Try to read 4 values
      if ( sscanf(whole_line,"%d %d %d %f",&(voxel.x),&(voxel.y),&(voxel.z),&weight) != 4 )
	{ // On failure, read 3 values
	  if ( sscanf(whole_line,"%d %d %d",&(voxel.x),&(voxel.y),&(voxel.z)) != 3 )
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
	{ // < 1 isn't mathematically necessary, but enforce anyway
	  printf("Error: use weight values <= 1. in overlays.\n");
	  return(1);
	}
      else if ( voxel.x < 0 || voxel.x >= mosaic.stack->dim.x ||
		voxel.y < 0 || voxel.y >= mosaic.stack->dim.y ||
		voxel.z < 0 || voxel.z >= mosaic.stack->dim.z )
	{
	  printf("Error: the overlay in file .ovl does not agree with the image dimensions.\n");
	  return(1);
	}
      else
	{
	  if ( ovl != &ovl_root )
	    { // add a new voxel
	      ovl->voxel[ovl->npts] = voxel;
	      ovl->weight[ovl->npts]  = weight;
	      ovl->npts++;
	    }
	  else
	    { // modify the root overlay.
	      index = voxel3D_to_1D(voxel);
	      overlay_map.weight[index] = weight;
	      overlay_map.color[index]  = ovl_root.color;
	    }
	}
    }
      fclose(file);

  return(0);
}

int write_overlay_file()
{
  FILE *file;
  IPOINT3D voxel;
  JIP_INDEX index;
  int novl_pixels;
  float weight_sum;
  
  /*
  ** Open the ovl files.
  */
  if ( !(file = fopen(".ovl","w")) )
    {
      printf("Error opening file (.ovl).\n");
      return(1);
    }

  weight_sum = 0.;
  novl_pixels = 0;
  for (voxel.z=0; voxel.z<mosaic.stack->dim.z; voxel.z++)
    {
      for (voxel.y=0; voxel.y<mosaic.stack->dim.y; voxel.y++)
	{
	  for (voxel.x=0; voxel.x<mosaic.stack->dim.x; voxel.x++)
	    {
	      index = voxel3D_to_1D(voxel);
	      if ( overlay_map.weight[index] == 1. )
		fprintf(file,"%d %d %d\n",voxel.x,voxel.y,voxel.z);
	      else if ( overlay_map.weight[index] != 0. )
		fprintf(file,"%d %d %d %g\n",voxel.x,voxel.y,voxel.z,overlay_map.weight[index]);
	      if ( overlay_map.weight[index] != 0. )
		{
		  novl_pixels++;
		  weight_sum += overlay_map.weight[index];
		}
	    }
	}
    }
  fclose(file);
  printf("%d pixels (total weight = %g) written to file .ovl\n",novl_pixels,weight_sum);
  if ( novl_pixels != ovl_root.npts )
    {
      printf("Programming error: # overlay pixels not consistent (%d versus %td)\n",
	     novl_pixels,ovl_root.npts);
      return(1);
    }

  return(0);
}

int read_wire_file( char *file_name, struct wire_list *wire_list, int new_color )
{
  FILE *file;
  CONTROL_POINT *wire_save;
  POINT3D point;
  int color, npts_in_file;JIP_INDEX jpt;
  
  // Open the wire file.
  if ( !(file = fopen(file_name,"r")) )
    {
      printf("Error opening file %s\n",file_name);
      return(1);
    }
  // Find the number of points in the file.
  npts_in_file = 0;
  while ( fscanf(file,"%f %f %f %d",&(point.x),&(point.y),&(point.z),&color) != EOF )
    npts_in_file++;
  rewind(file);
  if ( npts_in_file == 0 ) return(0);

  // Save the current wire points.
  if ( wire_list->npts != 0 )
    {
      wire_save = control_vector(wire_list->npts);
      for (jpt=0; jpt<wire_list->npts; jpt++)
	wire_save[jpt] = wire_list->control[jpt];
      free_control_vector(wire_list->control);
    }

  // Allocate the wire points, and copy the saved points back into the array.
  wire_list->control = control_vector(wire_list->npts + npts_in_file);
  if ( wire_list->npts != 0 )
    {
      for (jpt=0; jpt<wire_list->npts; jpt++)
	wire_list->control[jpt] = wire_save[jpt];
      free_control_vector(wire_save);
    }      

  // Read the file, putting points into the extended wire array.
  while ( fscanf(file,"%f %f %f %d",&(point.x),&(point.y),&(point.z),&color) != EOF )
    {
      wire_list->control[wire_list->npts].point = point;
      if ( color == 1 && new_color != 0 ) color = new_color;
      wire_list->control[wire_list->npts].color = color;
      wire_list->npts++;
    }
  fclose(file);

  //  printf("%d control points for a wireframe were read from %s\n",npts_in_file,file_name);
  return(0);
}

int write_wire_file()
{
  FILE *file;
  JIP_INDEX jpt;
  
  /*
  ** Open the wire file.
  */
  if ( !(file = fopen(".wire","w")) )
    {
      printf("Error opening file (.wire).\n");
      return(1);
    }

  for (jpt=0; jpt<wire_root.npts; jpt++)
    fprintf(file,"%f %f %f  %d\n",
	    wire_root.control[jpt].point.x,
	    wire_root.control[jpt].point.y,
	    wire_root.control[jpt].point.z,
	    wire_root.control[jpt].color);
  fclose(file);
  printf("%td control points for a wireframe were written to file .wire\n",
	 wire_root.npts);

  return(0);
}

int write_roi_stats( int all_points )
{
  int j;JIP_INDEX npoints;

  if ( ovl_root.npts != 0 )
    fprintf(stdout,"Overlay name: %s\n",ovl_root.name);
  if ( graph.type == 0 )
    fprintf(stdout,"X\tNpts\tAverage\n");
  else if ( graph.type == 1 )
    fprintf(stdout,"Y\tNpts\tAverage\n");
  else if ( graph.type == 2 )
    fprintf(stdout,"Z\tNpts\tAverage\n");
  else
    fprintf(stdout,"Image\tNpts\tAverage\tStDev\n");

  npoints = MAX(1,ovl_root.npts);

  if ( all_points )
    {
      // Loop over the time points.
      for (j=0; j<graph.npoints; j++)
	{
	  if ( graph.type == 3 || graph.type < 0 )
	    fprintf(stdout,"%d\t%td\t%g\t%g\n",j,npoints,graph.yVector[j],graph.yVector1[j]);
	  else
	    fprintf(stdout,"%d\t%td\t%g\n",j,npoints,graph.yVector[j]);
	}
    }
  else
    { // Current time point only
      if ( graph.type == 3 || graph.type < 0 )
	fprintf(stdout,"%d\t%td\t%g\t%g\n",graph.iTime,npoints,graph.yVector[graph.iTime],
		graph.yVector1[graph.iTime]);
      else
	fprintf(stdout,"%d\t%td\t%g\n",graph.iTime,npoints,graph.yVector[graph.iTime]);
    }
  return(0);
}
