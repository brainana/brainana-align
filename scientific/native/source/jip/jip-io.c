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
#include "nifti1_io.h"
#include "util.h"
#include "jip.h"

extern char error_string[LEN_STR];

/*
** Create an volume file structure.
*/
struct volume_file
{
  char name[LEN_STR];       // name
  IPOINT3D dim;             // spatial dimensions
  mat44 voxel_affine;       // complete native grid metadata
  int nt;                   // time points in file
  POINT3D resolution;       // resolution (mm per voxel) in x,y,z
  POINT3D origin;           // origin in x,y,z
  POINT3D direction;        // direction in x,y,z
  float resolution_t;       // seconds (usually)
  int data_type;            // data type of file (short, float, ...)
  int registration;         // 0 = NONE, 1 = Paxinos Rat
  int lDOF;                 // degrees of freedom for statistics
  struct volume_file *next; // pointer to the next file structure (or NULL)
};
#define VOLUME_FILE_SIZE sizeof(struct volume_file)
struct volume_file *output_file_root;

// Prototypes containing volume_file structure
int write_xdisplay_header_file( char *file_name, struct volume_file *output_file );
int write_nifti_header( int file_format, int data_type, int first_time, char *file_name,
			struct volume_file *output_file );


int read_xdisplay_file( char *file_name, char *stack_name, char *list_name, float time_step )
{
  FILE *file;
  struct XD_HEADER header;
  char slice_file_name[LEN_STR], slice_name[LEN_STR], extension[LEN_STR];
  int jset, jz;JIP_INDEX n2d, offset, iset, j;
  int nset, set[MAX_NSET];
  JIP_INDEX n3d;
  short *idata;
  unsigned short *iudata;
  unsigned char  *i8data;
  int *iidata;
  float *fdata, *data;
  LOGICAL error, swap_bytes;
  void swap_16(), swap_32();
  struct data_stack *stack;

  // Read the xdisplay header file.
  if ( read_volume_header(TRUE, file_name, &header) ) return(1);

  swap_bytes = header.byte_order != machine_byte_order();

  if ( header.data_type != 0 && header.dim.x%2 != 0 )
    {
      printf("Error: the number of readout points must be divisible by 2 for complex data.\n");
      return(1);
    }

  // Determine the set of volumes to read.
  if ( decode_subset(list_name,header.nt,&nset,set) )
    return(1);
  if ( nset < 1 )
    {
      printf("Programming error in routine read_xdisplay_file.\n");
      exit(1);
    }
  else if ( nset > MAX_NSET )
    {
      printf("Error: Too many volumes (%d) to read; increase MAX_NSET.\n",
	     nset);
      return(1);
    }
  // Make sure that the volume set is in ascending order.
  for (jset=1; jset<nset; jset++)
    {
      if ( set[jset] <= set[jset-1] )
	{
	  jip_text(2,"volumes must be read in ascending order.");
	  return(1);
	}
    }

  // Allocate memory for the stack data array.
  if ( header.data_type == 0 )
    n2d = jip_product(header.dim.x,header.dim.y);
  else
    n2d = jip_product(jip_product(2,header.dim.x),header.dim.y);
  n3d  = jip_product(n2d,header.dim.z);
  data = float_vector(nset*n3d); // This will be the stack data array.

  if ( header.file_type == BSHORT    || header.file_type == BFLOAT || header.file_type == BLONG ||
       header.file_type == USHORT    || header.file_type == UINT8 ||
       header.file_type == NIFTI_IMG || header.file_type == NIFTI_NII )
    {
      // Open the file and read it.
      if ( !(file = fopen(file_name,"rb")) )
	{
	  printf("Error: cannot open file %s.\n",file_name);
	  return(1);
	}
      // Begin reading the data.
      iset = offset = error = 0;

      // Go to the first data voxel
      error = jip_seek(file, (JIP_INDEX)(header.voxel_offset), SEEK_SET);
      if ( error )
	{
	  fprintf(stderr, "\nError doing jip_seek() to %td in data file %s\n",
		  (JIP_INDEX)(header.voxel_offset), file_name);
	  exit(1);
	}

      // Fast-forward to the first word in the set.
      if ( header.storage_type == BFLOAT || header.storage_type == BLONG )
	{ // use the following loop to avoid 4*n3d*set[0] > long integer
	  for (j=0; j<set[0]; j++)
	    error = jip_seek(file, (JIP_INDEX)(4*n3d), SEEK_CUR);
	  //  error = jip_seek(file, (JIP_INDEX)(4*n3d*set[0]), SEEK_CUR);
	}
      else if ( header.storage_type == BSHORT || header.storage_type == USHORT )
	{
	  for (j=0; j<set[0]; j++)
	    error = jip_seek(file, (JIP_INDEX)(2*n3d), SEEK_CUR);
	  // error = jip_seek(file, (JIP_INDEX)(2*n3d*set[0]), SEEK_CUR);
	}
      if ( error )
	{
	  printf("Error 1 reading file %s (format = %d).\n",file_name,header.storage_type);
	  return(1);
	}
      // Allocate space
      if ( header.storage_type == BFLOAT )
	fdata = float_vector(n3d);
      else if ( header.storage_type == BSHORT )
	idata = short_vector(n3d);
      else if ( header.storage_type == BLONG )
	iidata = int_vector(n3d);
      else if ( header.storage_type == USHORT )
	iudata = unsigned_short_vector(n3d);
      else if ( header.storage_type == UINT8 )
	i8data = unsigned_char_vector(n3d);
      // Read the array.
      for (jset=set[0]; jset<=set[nset-1]; jset++)
	{
	  if ( header.storage_type == BFLOAT )
	    error = fread( (char *)fdata, sizeof(float), n3d, file) != n3d;
	  else if ( header.storage_type == BSHORT )
	    error = fread( (char *)idata, sizeof(short), n3d, file) != n3d;
	  else if ( header.storage_type == BLONG )
	    error = fread( (char *)iidata, sizeof(int), n3d, file) != n3d;
	  else if ( header.storage_type == USHORT )
	    error = fread( (char *)iudata, sizeof(unsigned short), n3d, file) != n3d;
	  else if ( header.storage_type == UINT8 )
	    error = fread( (char *)i8data, sizeof(char), n3d, file) != n3d;
	  if ( error ) break;
	  if ( jset == set[iset] )
	    {
	      // Copy the integer array into the stack floating point array.
	      for (j=0; j<n3d; j++)
		{
		  if ( header.storage_type == BFLOAT )
		    {
		      if ( swap_bytes ) swap_32( &(fdata[j]) );
		      data[j+offset] = fdata[j];
		    }
		  else if ( header.storage_type == BSHORT )
		    {
		      if ( swap_bytes ) swap_16( &(idata[j]) );
		      data[j+offset] = idata[j];
		    }
		  else if ( header.storage_type == BLONG )
		    {
		      if ( swap_bytes ) swap_32( &(iidata[j]) );
		      data[j+offset] = iidata[j];
		    }
		  else if ( header.storage_type == USHORT )
		    {
		      if ( swap_bytes ) swap_16( &(iudata[j]) );
		      data[j+offset] = iudata[j];
		    }
		  else if ( header.storage_type == UINT8 )
		    {
		      // no need to swap, since only 1 byte+
		      data[j+offset] = i8data[j];
		    }
		  // Scale the data if necessary.
		  if ( header.scale_slope != 0. )
		    {
		      data[j+offset] *= header.scale_slope;
		      data[j+offset] += header.scale_offset;
		    }
		}
	      iset++;
	      offset += n3d;
	    }
	}
      if ( header.storage_type == BFLOAT )
	free_float_vector(fdata);
      else if ( header.storage_type == BSHORT )
	free_short_vector(idata);
      else if ( header.storage_type == BLONG )
	free_int_vector(iidata);
      else if ( header.storage_type == USHORT )
	free_unsigned_short_vector(iudata);
      else if ( header.storage_type == UINT8 )
	free_unsigned_char_vector(i8data);
      // Close the file
      fclose(file);
    }
  else if ( header.file_type == FREESURFER )
    {
      if ( header.storage_type == BSHORT )
	{
	  idata = short_vector(n2d);
	  strcpy(extension,".bshort");
	}
      else
	{
	  fdata = float_vector(n2d);
	  strcpy(extension,".bfloat");
	}
      // For each slice, open the file and read all time points.
      for (jz=0; jz<header.dim.z; jz++)
	{
	  // Create the file name.
	  if ( jz < 10 )
	    sprintf(slice_name,"00%d",jz);
	  else if ( jz < 100 )
	    sprintf(slice_name,"0%d",jz);
	  else
	    sprintf(slice_name,"%d",jz);
	  strcpy(slice_file_name,file_name);
	  strcat(slice_file_name,slice_name);
	  strcat(slice_file_name,extension);
	  // Open the binary data file.
	  if ( !(file = fopen(slice_file_name,"rb")) )
	    {
	      printf("\nError: cannot open file %s.\n",slice_file_name);
	      return(1);
	    }
	  // Now read each time point for the current slice.
	  iset = error = 0;
	  offset = jz * n2d;
	  // Go to the first time point.
	  if ( header.storage_type == BSHORT )
	    error = jip_seek(file, (JIP_INDEX)(2*n2d*set[0]), SEEK_SET);
	  else
	    error = jip_seek(file, (JIP_INDEX)(4*n2d*set[0]), SEEK_SET);
	  if ( error ) break;
	  for (jset=set[0]; jset<=set[nset-1]; jset++)
	    {
	      if ( header.storage_type == BSHORT )
		{
		  error = fread( (char *)idata, sizeof(short), n2d, file) != n2d;
		  if ( error ) break;
		  if ( jset == set[iset] )
		    {
		      // Copy the integer array into the stack floating point array.
		      for (j=0; j<n2d; j++)
			{
			  if ( swap_bytes ) swap_16( &(idata[j]) );
			  data[j+offset] = idata[j];
			}
		      iset++;
		      offset += n3d;
		    }
		}
	      else
		{
		  error = fread( (char *)fdata, sizeof(float), n2d, file) != n2d;
		  if ( error ) break;
		  if ( jset == set[iset] )
		    {
		      // Copy the integer array into the stack floating point array.
		      for (j=0; j<n2d; j++)
			{
			  if ( swap_bytes ) swap_32( &(fdata[j]) );
			  data[j+offset] = fdata[j];
			}
		      iset++;
		      offset += n3d;
		    }
		}
	    }
	  fclose(file);
	}
      if ( header.storage_type == BSHORT )
	free_short_vector(idata);
      else
	free_float_vector(fdata);
    }
  else
    {
      printf("Programming error: unknown file type in read_xdisplay_file\n");
      exit(1);
    }
  if ( error )
    {
      printf("Error 2 reading file %s.\n",file_name);
      return(1);
    }

  // The data has been read, so install the stack in memory and define things.
  if ( !strcmp("registration_distortions",stack_name) || !strcmp("histology_distortions",stack_name) )
    {
      if ( !strcmp("registration_distortions",stack_name) && nset != 3 )
	{
	  printf("Error: 3 distortion fields (x,y,z), not %d, should be read from the distortion file %s\n",
		 nset,file_name);
	  return(1);
	}
      else if ( !strcmp("histology_distortions",stack_name) && nset != 2 )
	{
	  printf("Error: 2 distortion fields (x,y), not %d, should be read from the distortion file %s\n",
		 nset,file_name);
	  return(1);
	}
      strcpy(registration_distortions.name,stack_name);
      registration_distortions.dim = header.dim;
      registration_distortions.nt  = nset;
      registration_distortions.n2d = jip_product(registration_distortions.dim.x,registration_distortions.dim.y);
      registration_distortions.n3d = jip_product(registration_distortions.n2d,registration_distortions.dim.z);
      registration_distortions.n4d = jip_product(registration_distortions.n3d,registration_distortions.nt);
      registration_distortions.npts_volume = jip_product(2,registration_distortions.n3d);
      if ( registration_distortions.SpaceTime.fcData != NULL )
	free_fcomplex_vector(registration_distortions.SpaceTime.fcData);
      registration_distortions.SpaceTime.fcData = fcomplex_vector(nset*n3d);
      registration_distortions.data_type = 0;
      stack = &registration_distortions;
    }
  else
    stack = install_stack( stack_name, 0, header.dim, nset );
  stack->status = IGNORE_BIT;
  if ( header.data_type == 0 )
    stack->status = stack->status | MAG_PHASE_BIT;
  stack->registration = header.registration;
  stack->resolution   = header.resolution;
  if ( time_step == 0. )
    stack->resolution_t = header.resolution_t;
  else
    stack->resolution_t = time_step;
  stack->origin       = header.origin;
  stack->direction    = header.direction;
  stack->fov          = header.fov;
  stack->off_diagonal = header.off_diagonal;
  stack->lDOF         = header.lDOF;
  stack->ijk_to_xyz   = header.ijk_to_xyz;
  stack->xyz_to_ijk   = header.xyz_to_ijk;

  if ( stack->off_diagonal )
    {
      printf("Warning: the stack has coordinates (x,y,z)\n");
      printf("         that are not parallel with indices (i,j,k)\n.");
      printf("         NIfTI sform output preserves this grid; legacy raw output cannot.\n");
    }

  // Copy the temporary data vector into the stack data vector.
  if ( header.data_type != 0 )
    {
      for (j=0; j<stack->n4d; j++)
	{
	  stack->SpaceTime.fcData[j].real = data[2*j];
	  stack->SpaceTime.fcData[j].imag = data[2*j+1];
	}
    }
  else
    {
      for (j=0; j<stack->n4d; j++)
	{
	  stack->SpaceTime.fcData[j].real = data[j];
	  stack->SpaceTime.fcData[j].imag = 0.;
	}
    }
  free_float_vector(data);

  // Tell the user what has been read.
  if ( stack->nt == 1 )
    printf("%d volume of size %d x %d x %d was read and stored in stack ' %s '\n",
	   stack->nt, stack->dim.x, stack->dim.y, stack->dim.z, stack->name);
  else
    {
      if ( !strcmp("registration_distortions",stack_name) )
	printf("%d volumes of size %d x %d x %d were stored as distortion maps\n",
	       stack->nt, stack->dim.x, stack->dim.y, stack->dim.z);
      else
	printf("%d volumes of size %d x %d x %d were read and stored in stack ' %s '\n",
	       stack->nt, stack->dim.x, stack->dim.y, stack->dim.z, stack->name);
    }

  return(0);
}

int write_output_file( int nifti_datatype, char *file_name )
/*
** Write an xdisplay volume file and the corresponding header file.  On the first
** time through, a new file is written.  Subsequent volumes from addition function calls
** are appended to to this file.  A header file is also written each time.
*/
{
  IPOINT3D dim;
  int nt, iwords;JIP_INDEX n3d, j;
  int file_format, data_type, update_file;JIP_INDEX error;
  float *fdata;
  short *idata;
  unsigned short *iudata;
  FILE *file;
  char full_file_name[LEN_STR];
  struct volume_file *output_file;
  struct volume_list  *volume;
  struct data_stack *stack, *stack_save;
  extern int flags;
  extern struct volume_list *volume_list_root;

  // get the file_format from the file name
  if ( (file_format=xdisplay_format(file_name)) < 0 )
    {
      printf("Error: that output file format is not supported.\n");
      printf("Use the `bfloat' extension or use `bshort' after `rescale'.\n");
      return(1);
    }
  // Determine the number of volumes in the volume list.  Make sure each
  // volume in the list has the same dimensions.
  if ( inconsistent_3d_dimensions(&dim,&nt) ) return(1);
  if ( nt == 0 ) // This should NEVER happen.
    {
      printf("Programming error: nt=0 in routine write-xdisplay !!?\n");
      exit(1);
    }

  // Make the file name `file_name.bfloat' from the root `file_name'.
  strcpy(full_file_name, file_name);
  if ( file_format == BFLOAT )
    {
      strcat(full_file_name, ".bfloat");
      data_type = BFLOAT;
    }
  else if ( file_format == BSHORT )
    {
      strcat(full_file_name, ".bshort");
      data_type = BSHORT;
    }
  else if ( file_format == USHORT )
    {
      strcat(full_file_name, ".ushort");
      data_type = USHORT;
    }
  else if ( file_format == NIFTI_IMG )
    {
      strcat(full_file_name, ".img");
      if ( nifti_datatype == BSHORT || nifti_datatype == BFLOAT )
	data_type = nifti_datatype;
      else
	data_type = BFLOAT;
    }
  else if ( file_format == NIFTI_NII )
    {
      strcat(full_file_name, ".nii");
      if ( nifti_datatype == BSHORT || nifti_datatype == BFLOAT )
	data_type = nifti_datatype;
      else
	data_type = BFLOAT;
    }

  // Install the file structure in memory or get its location.
  output_file = install_file( file_name );

  // Make sure that the dimensions agree.  If this is the 1st time the file
  // has been used, define the file dimensions.  Also, open the file.
  if ( output_file->nt == 0 ) // e.g., if the file has not been used
    {
      update_file = 0;
      output_file->dim          = dim;
      output_file->voxel_affine = volume_list_root->stack->ijk_to_xyz;
      output_file->resolution   = volume_list_root->stack->resolution;
      output_file->origin       = volume_list_root->stack->origin;
      output_file->direction    = volume_list_root->stack->direction;
      output_file->registration = volume_list_root->stack->registration;
      output_file->lDOF         = volume_list_root->stack->lDOF;
      output_file->data_type    = data_type;
      output_file->resolution_t = volume_list_root->stack->resolution_t;

      if ( file_format == NIFTI_NII )
	{
	  write_nifti_header(file_format, data_type, 1, file_name, output_file);
	  error = !(file = fopen(full_file_name,"rb+"));
	  error &= jip_seek(file, (long)(0), SEEK_END);
	}
      else
	error = !(file = fopen(full_file_name,"wb"));
      if ( error )
	{
	  printf("Error: cannot open the new file %s.\n",full_file_name);
	  return(1);
	}
    }
  else
    { // Make sure the new dimensions agree with the existing ones.
      update_file = 1;
      if ( output_file->dim.x != dim.x || output_file->dim.y != dim.y|| output_file->dim.z != dim.z )
	{
	  printf("Error: the xdisplay file `%s' has different dimensions.\n",
		 full_file_name);
	  return(1);
	}
      if ( output_file->data_type != data_type )
	{
	  printf("Error: don't mix data types in output file `%s'\n",full_file_name);
	  return(1);
	}
      if ( output_file->registration != volume_list_root->stack->registration )
	{
	  printf("Warning: o not combine registered and un-registered data in the same file.\n");
	  //	  return(1);
	}
      error = !(file = fopen(full_file_name,"ab"));
      if ( file_format == NIFTI_NII )
	error &= jip_seek(file, (long)(0), SEEK_END);
      if ( error )
	{
	  printf("Error: cannot open the existing file %s.\n",full_file_name);
	  return(1);
	}
    }
  // Update the number of time points.
  output_file->nt += nt;
  n3d = jip_product(jip_product(dim.x,dim.y),dim.z);

  if (flags&COMPLEX_OUTPUT_BIT )
    {
      iwords = 2;
      if ( (file_format==NIFTI_NII || file_format==NIFTI_IMG) && (data_type==BSHORT) )
	{
	  printf("There is no NIFTI data type for complex shorts; use complex floats.\n");
	  return(1);
	}
    }
  else
    iwords = 1;

  if ( data_type == BFLOAT )
    fdata = float_vector(iwords * n3d);
  else if ( data_type == BSHORT )
    idata = short_vector(iwords * n3d);
  else if ( data_type == USHORT )
    iudata = unsigned_short_vector(iwords * n3d);

  // Write the file
  for ( volume=volume_list_root, stack_save=0; volume; volume=volume->next )
    {
      // For each new stack, test the resolution against the resolution in the file.
      if ( (stack=volume->stack) != stack_save )
	{
	  if ( output_file->resolution.x != stack->resolution.x ||
	       output_file->resolution.y != stack->resolution.y ||
	       output_file->resolution.z != stack->resolution.z )
	    {
	      printf("Warning: the resolution of stack %s (%f %f %f)\n",
		     stack->name,stack->resolution.x,stack->resolution.y,stack->resolution.z);
	      printf("differs from the resolution assigned to the file (%f %f %f)\n",
		     output_file->resolution.x,output_file->resolution.y,output_file->resolution.z);
	    }
	  if ( output_file->resolution_t != stack->resolution_t )
	    {
	      printf("Warning: the time step of stack %s (%g)\n",stack->name,stack->resolution_t);
	      printf("differs from that assigned to the file (%g)\n",output_file->resolution_t);
	    }
	  if ( output_file->origin.x != stack->origin.x ||
	       output_file->origin.y != stack->origin.y ||
	       output_file->origin.z != stack->origin.z )
	    {
	      printf("Warning: the origin of stack %s (%f %f %f)\n",
		     stack->name,stack->origin.x,stack->origin.y,stack->origin.z);
	      printf("differs from the origin assigned to the file (%f %f %f)\n",
		     output_file->origin.x,output_file->origin.y,output_file->origin.z);
	    }
	  if ( output_file->direction.x != stack->direction.x ||
	       output_file->direction.y != stack->direction.y ||
	       output_file->direction.z != stack->direction.z )
	    {
	      printf("Warning: the direction of stack %s (%f %f %f)\n",
		     stack->name,stack->direction.x,stack->direction.y,stack->direction.z);
	      printf("differs from the direction assigned to the file (%f %f %f)\n",
		     output_file->direction.x,output_file->direction.y,output_file->direction.z);
	    }
	  stack_save = stack;
	}

      if ( data_type == BFLOAT )
	{ // floating point
	  for ( j=0; j<iwords*n3d; j+=iwords )
	    {
	      if (flags&COMPLEX_OUTPUT_BIT )
		{
		  fdata[j]   = volume->SpaceTime.fcData[j/iwords].real;
		  fdata[j+1] = volume->SpaceTime.fcData[j/iwords].imag;
		}
	      else if ( flags&PHASE_OUTPUT_BIT )
		fdata[j] = 180./PI * volume_phase(volume,j);
	      else
		fdata[j] = volume_amp(volume,j);
	    }
	}
      else if ( data_type == BSHORT )
	{ // short integers
	  for ( j=0; j<iwords*n3d; j+=iwords )
	    {
	      if (flags&COMPLEX_OUTPUT_BIT )
		{
		  idata[j]   = volume->SpaceTime.fcData[j/iwords].real;
		  idata[j+1] = volume->SpaceTime.fcData[j/iwords].imag;
		  // Catch short integer overflow.
		  if ( idata[j]   == -32768 ) idata[j]   = 32767;
		  if ( idata[j+1] == -32768 ) idata[j+1] = 32767;
		}
	      else if ( flags&PHASE_OUTPUT_BIT )
		idata[j] = 180./PI * volume_phase(volume,j);
	      else
		{
		  idata[j] = volume_amp(volume,j);
		  // Catch short integer overflow.
		  if ( idata[j] == -32768 ) idata[j] = 32767;
		}
	    }
	}
      else if ( data_type == USHORT )
	{ // short integers
	  for ( j=0; j<iwords*n3d; j+=iwords )
	    {
	      if (flags&COMPLEX_OUTPUT_BIT )
		{
		  iudata[j]   = volume->SpaceTime.fcData[j/iwords].real;
		  iudata[j+1] = volume->SpaceTime.fcData[j/iwords].imag;
		  // Catch short integer overflow.
		  //		  if ( iudata[j]   == -32768 ) iudata[j]   = 32767;
		  //		  if ( iudata[j+1] == -32768 ) iudata[j+1] = 32767;
		}
	      else if ( flags&PHASE_OUTPUT_BIT )
		iudata[j] = 180./PI * volume_phase(volume,j);
	      else
		{
		  iudata[j] = volume_amp(volume,j);
		  // Catch short integer overflow.
		  //		  if ( iudata[j] == -32768 ) iudata[j] = 32767;
		}
	    }
	}
      // Write the data.
      if ( data_type == BFLOAT )
	// floating point; either magnitude or complex
	error = fwrite((char *)fdata,sizeof(float),iwords*n3d,file) != iwords*n3d;
      else if ( data_type == BSHORT )
	// short integers
	error = fwrite((char *)idata,sizeof(short),iwords*n3d,file) != iwords*n3d;
      else if ( data_type == USHORT )
	// unsigned short integers
	error = fwrite((char *)iudata,sizeof(unsigned short),iwords*n3d,file) != iwords*n3d;
      if ( error )
	{
	  printf("Error writing the output file %s.\n",full_file_name);
	  return(1);
	}
    }
  fclose( file );
  if ( data_type == BFLOAT )
    free_float_vector(fdata);
  else if ( data_type == BSHORT )
    free_short_vector(idata);
  else if ( data_type == USHORT )
    free_unsigned_short_vector(iudata);

  // Write the header file.
  if ( file_format == BSHORT || file_format == BFLOAT || file_format == USHORT )
    write_xdisplay_header_file(file_name, output_file);
  else if ( file_format == NIFTI_IMG || file_format == NIFTI_NII )
    write_nifti_header(file_format, data_type, 0, file_name, output_file);

  printf("%d volume(s) of dimension %d x %d x %d were written to file %s.\n",
	 nt,output_file->dim.x,output_file->dim.y,output_file->dim.z,output_file->name);
  if ( update_file )   /* e.g., if the file has been used */
    printf("%d volumes now reside in file %s.\n\n",
	   output_file->nt, output_file->name);
  return(0);
}

int write_xdisplay_header_file( char *file_name, struct volume_file *output_file )
{
  char full_file_name[LEN_STR];
  FILE *file;
  int iwords;
  extern int flags;

  // Make the header file name `file_name.hdr' from the root `file_name'.
  strcpy(full_file_name, file_name);
  strcat(full_file_name, ".hdr");

  // Now write the header file.
  if ( !(file = fopen(full_file_name,"w")) )
    {
      printf("Error: cannot open the header file %s.\n",full_file_name);
      return(1);
    }

  if (flags&COMPLEX_OUTPUT_BIT )
    iwords = 2;
  else
    iwords = 1;

  if (flags&COMPLEX_OUTPUT_BIT )
    fprintf(file,"data-type complex  # data are stored as (real, imag) pairs\n");
  fprintf(file,"x %d\ny %d\nz %d\nt %d\nbyte-order %d\n",
	  output_file->dim.x,
	  output_file->dim.y,
	  output_file->dim.z,
	  output_file->nt,
	  machine_byte_order() );
  if ( output_file->resolution_t != 0. )
    fprintf(file,"resolution %g %g %g %g\n",
	    output_file->resolution.x,
	    output_file->resolution.y,
	    output_file->resolution.z,
	    output_file->resolution_t);
  else
    fprintf(file,"resolution %g %g %g 1\n",
	    output_file->resolution.x,output_file->resolution.y,output_file->resolution.z);
  fprintf(file,"origin %g %g %g\n",
	  output_file->origin.x,output_file->origin.y,output_file->origin.z);
  fprintf(file,"direction %g %g %g\n",
	  output_file->direction.x,output_file->direction.y,output_file->direction.z);
  if ( output_file->registration == REG_PW_RAT )
    fprintf(file,"registration PW-rat\n");
  else if ( output_file->registration == REG_PW_RAT_OLD )
    fprintf(file,"registration paxinos-rat\n");

  // degrees of freedom
  if ( output_file->lDOF != 0 )
    fprintf(file,"degrees-of-freedom %d\n",output_file->lDOF);

  fclose(file);
  return(0);
}

int write_nifti_header( int file_format, int data_type, int first_time,
			char *file_name, struct volume_file *output_file )
{
  nifti_1_header nifti_header;
  nifti1_extender pad;
  char full_file_name[LEN_STR];
  FILE *file;
  int error, iwords, j, extension[4];
  mat44 mat_nifti;
  float qb, qc, qd, qx, qy, qz, dx, dy, dz, qfac;
  extern int flags;
  extern int nifti_method;

  mat_nifti = output_file->voxel_affine;
  nifti_mat44_to_quatern( mat_nifti,
			  &qb, &qc, &qd,
			  &qx, &qy, &qz,
			  &dx, &dy, &dz, &qfac );

  nifti_header.quatern_b = qb;
  nifti_header.quatern_c = qc;
  nifti_header.quatern_d = qd;
  nifti_header.qoffset_x = qx;
  nifti_header.qoffset_y = qy;
  nifti_header.qoffset_z = qz;

  nifti_header.sizeof_hdr = 348;
  nifti_header.dim[0] = 4;
  nifti_header.dim[1] = output_file->dim.x;
  nifti_header.dim[2] = output_file->dim.y;
  nifti_header.dim[3] = output_file->dim.z;
  nifti_header.dim[4] = output_file->nt;
  nifti_header.dim[5] = 1;
  nifti_header.dim[6] = 1;
  nifti_header.dim[7] = 1;

  // either floating real data or floating complex data
  if (flags&COMPLEX_OUTPUT_BIT )
    {
      iwords = 2;
      nifti_header.datatype = NIFTI_TYPE_COMPLEX64;
    }
  else
    {
      iwords = 1;
      if ( data_type == BFLOAT )
	nifti_header.datatype = NIFTI_TYPE_FLOAT32;
      else if ( data_type == BSHORT )
	nifti_header.datatype = NIFTI_TYPE_INT16;
    }
  nifti_header.bitpix      = 32*iwords;

  nifti_header.slice_start = nifti_header.slice_end = 0;
  nifti_header.slice_code  = 0;
  nifti_header.slice_duration = 0.;

  nifti_header.pixdim[0]   = qfac;    // "qfac": use voxel-based right-handed cooordinates
  nifti_header.pixdim[1]   = output_file->resolution.x;
  nifti_header.pixdim[2]   = output_file->resolution.y;
  nifti_header.pixdim[3]   = output_file->resolution.z;
  nifti_header.pixdim[4]   = output_file->resolution_t;
  nifti_header.pixdim[5]   = 0.;
  nifti_header.pixdim[6]   = 0.;
  nifti_header.pixdim[7]   = 0.;

  // temporary; ignore extended header info from original file
  if ( file_format == NIFTI_NII )
    {
      if ( output_file->lDOF != 0 )
	nifti_header.vox_offset  = 368;  // 352 + 16-byte extension
      else
	nifti_header.vox_offset  = 352;
    }
  else
    nifti_header.vox_offset  = 0;

  nifti_header.scl_slope = nifti_header.scl_inter = 0;
  //  nifti_header.xyzt_units  = NIFTI_UNITS_MM | NIFTI_UNITS_SEC;
  nifti_header.xyzt_units  = NIFTI_UNITS_MM;
  nifti_header.cal_min = nifti_header.cal_max = 0.;
  nifti_header.toffset = 0.;

  // Use the affine transformation (method 3).
  nifti_header.qform_code = nifti_method == 2 ? 1 : 0;
  if ( nifti_method == 2 )
    nifti_header.sform_code = 0;
  else
    nifti_header.sform_code = 1;
  for(j=0;j<4;j++) {
    nifti_header.srow_x[j]=mat_nifti.m[0][j];
    nifti_header.srow_y[j]=mat_nifti.m[1][j];
    nifti_header.srow_z[j]=mat_nifti.m[2][j];
  }
  // Unused fields
  strcpy(nifti_header.data_type,"");
  strcpy(nifti_header.db_name,"");
  nifti_header.extents = 0;
  nifti_header.session_error = 0;
  nifti_header.glmin = nifti_header.glmax = 0;
  nifti_header.regular = 'r';
  nifti_header.dim_info = 0;

  nifti_header.intent_code = 0;
  strcpy(nifti_header.intent_name,"");
  nifti_header.intent_name[0] = 0;
  nifti_header.intent_p1 = nifti_header.intent_p2 = nifti_header.intent_p3 = 0.;

  strcpy(nifti_header.descrip,"");
  strcpy(nifti_header.aux_file,"");

  // Make the header file name `file_name.hdr' from the root `file_name'.
  strcpy(full_file_name, file_name);
  if ( file_format == NIFTI_IMG )
    {
      strcat(full_file_name, ".hdr");
      strcpy(nifti_header.magic,"ni1\0");
      // Open the file
      if ( !(file = fopen(full_file_name,"wb")) )
	{
	  printf("Error: cannot open the header file %s.\n",full_file_name);
	  return(1);
	}
    }
  else
    {
      strcat(full_file_name, ".nii");
      strcpy(nifti_header.magic,"n+1\0");
      if ( first_time )
	error = !(file = fopen(full_file_name,"wb"));
      else
	error = !(file = fopen(full_file_name,"rb+"));
      // Open the file
      if ( error )
	{
	  printf("Error: cannot open the header file %s.\n",full_file_name);
	  return(1);
	}
    }

  error = fwrite((char *)(&nifti_header),sizeof(struct nifti_1_header),1,file) != 1;
  if ( error )
    {
      printf("Error writing the NIFTI header into file %s\n",full_file_name);
      return(1);
    }

  if ( file_format == NIFTI_NII )
    {
      pad.extension[0] = pad.extension[1] = pad.extension[2] = pad.extension[3] = 0;
      if ( output_file->lDOF != 0 )
	pad.extension[0] = 1;
      error = fwrite(&pad, 4, 1, file) != 1;
      if ( error )
	{
	  fprintf(stderr, "\nError writing header file extension pad %s\n",full_file_name);
	  return(1);
	}
      if ( output_file->lDOF != 0 )
	{
	  // Create a custom extension of 16 bytes as 4 integers.
	  extension[0] = 16;                // number of bytes in extension
	  extension[1] = 20;                // make up my own unregistered e-code
	  extension[2] = output_file->lDOF; // degrees of freedom saved here
	  extension[3] = 0;
	  error = fwrite(extension, sizeof(int), 4, file) != 4;
	  if ( error )
	    {
	      fprintf(stderr, "\nError writing nifti header extension for DOF %s\n",full_file_name);
	      return(1);
	    }
	}
    }

  fclose(file);
  return(0);
}

int write_slices( char *file_name )
/*
** Write an xdisplay volume file and the corresponding header file.  On the first
** time through, a new file is written.  Subsequent volumes are appended to to this file.
** A header file is also written each time.
*/
{
  IPOINT3D dim;
  int nt, iwords, jz;JIP_INDEX n2d, ioffset, j;
  int format, update_file;JIP_INDEX error;
  float *fdata;
  short *idata;
  char full_file_name[LEN_STR], end_file_name[LEN_STR];
  FILE *file;
  struct volume_file *output_file;
  struct volume_list  *volume;
  struct data_stack *stack, *stack_save;
  extern int flags;
  extern struct volume_list *volume_list_root;

  // First determine the file format.
  if ( (format=xdisplay_format(file_name)) < 0 )
    {
      printf("Error: that output file format is not supported.\n");
      printf("Use the `bfloat' extension or use `bshort' after `rescale'.\n");
      return(1);
    }
  // Determine the number of volumes in the volume list.  Make sure each
  // volume in the list has the same dimensions.
  if ( inconsistent_3d_dimensions(&dim,&nt) ) return(1);
  if ( nt == 0 ) // This should NEVER happen.
    {
      printf("Programming error: nt=0 in routine write-xdisplay !!?\n");
      exit(1);
    }

  n2d = jip_product(jip_product(dim.x,dim.y),1);
  if (flags&COMPLEX_OUTPUT_BIT )
    iwords = 2;
  else
    iwords = 1;
  if ( format == BFLOAT )
    fdata = float_vector(iwords * n2d);
  else
    idata = short_vector(iwords * n2d);

  for ( jz=0; jz<dim.z; jz++)
    {
      // Make the file name `file_name.bfloat' from the root `file_name'.
      strcpy(full_file_name, file_name);
      if ( format == BFLOAT )
	{
	  if ( jz < 10 )
	    sprintf(end_file_name,"_00%d.bfloat",jz);
	  else
	    sprintf(end_file_name,"_0%d.bfloat",jz);
	}
      else
	{
	  if ( jz < 10 )
	    sprintf(end_file_name,"_00%d.bshort",jz);
	  else
	    sprintf(end_file_name,"_0%d.bshort",jz);
	}
      strcat(full_file_name,end_file_name);
      // Install the file structure in memory or get its location.
      output_file = install_file( full_file_name );

      // Make sure that the dimensions agree.  If this is the 1st time the file
      // has been used, define the file dimensions.  Also, open the file.
      if ( output_file->nt == 0 ) // e.g., if the file has not been used
	{
	  update_file = 0;
	  output_file->dim   = dim;
	  output_file->dim.z = 1;  // single slice output
	  output_file->resolution = volume_list_root->stack->resolution;
	  output_file->origin     = volume_list_root->stack->origin;
	  output_file->direction  = volume_list_root->stack->direction;
	  if ( !(file = fopen(full_file_name,"w")) )
	    {
	      printf("Error: cannot open the new file %s.\n",full_file_name);
	      return(1);
	    }
	}
      else
	{ // Make sure the new dimensions agree with the existing ones.
	  update_file = 1;
	  if ( output_file->dim.x != dim.x || output_file->dim.y != dim.y || output_file->dim.z != 1 )
	    {
	      printf("Error: the xdisplay file `%s' has different dimensions.\n",
		     file_name);
	      return(1);
	    }
	  if ( !(file = fopen(full_file_name,"a")) )
	    {
	      printf("Error: cannot open the existing file %s.\n",full_file_name);
	      return(1);
	    }
	}
      /* Update the number of time points. */
      output_file->nt += nt;

      // Write the file
      for ( volume=volume_list_root, stack_save=0; volume; volume=volume->next )
	{
	  // For each new stack, test the resolution against the resolution in the file.
	  if ( (stack=volume->stack) != stack_save )
	    {
	      if ( output_file->resolution.x != stack->resolution.x ||
		   output_file->resolution.y != stack->resolution.y ||
		   output_file->resolution.z != stack->resolution.z )
		{
		  printf("Warning: the resolution of stack %s (%g %g %g)\n",
			 stack->name,stack->resolution.x,stack->resolution.y,stack->resolution.z);
		  printf("differs from the resolution assigned to the file (%g %g %g)\n",
			 output_file->resolution.x,output_file->resolution.y,output_file->resolution.z);
		}
	      if ( output_file->origin.x != stack->origin.x ||
		   output_file->origin.y != stack->origin.y ||
		   output_file->origin.z != stack->origin.z )
		{
		  printf("Warning: the origin of stack %s (%g %g %g)\n",
			 stack->name,stack->origin.x,stack->origin.y,stack->origin.z);
		  printf("differs from the origin assigned to the file (%g %g %g)\n",
			 output_file->origin.x,output_file->origin.y,output_file->origin.z);
		}
	      if ( output_file->direction.x != stack->direction.x ||
		   output_file->direction.y != stack->direction.y ||
		   output_file->direction.z != stack->direction.z )
		{
		  printf("Warning: the direction of stack %s (%g %g %g)\n",
			 stack->name,stack->direction.x,stack->direction.y,stack->direction.z);
		  printf("differs from the direction assigned to the file (%g %g %g)\n",
			 output_file->direction.x,output_file->direction.y,output_file->direction.z);
		}
	      stack_save = stack;
	    }
	  // offset to slice
	  ioffset = jz * n2d;
	  
	  if ( format == BFLOAT )
	    { // floating point
	      for ( j=0; j<iwords*n2d; j+=iwords )
		{
		  if (flags&COMPLEX_OUTPUT_BIT )
		    {
		      fdata[j]   = volume->SpaceTime.fcData[(j+ioffset)/iwords].real;
		      fdata[j+1] = volume->SpaceTime.fcData[(j+ioffset)/iwords].imag;
		    }
		  else
		    fdata[j] = volume_amp(volume,j+ioffset);
		}
	    }
	  else
	    { // short integers
	      for ( j=0; j<iwords*n2d; j+=iwords )
		{
		  if (flags&COMPLEX_OUTPUT_BIT )
		    {
		      idata[j]   = volume->SpaceTime.fcData[(j+ioffset)/iwords].real;
		      idata[j+1] = volume->SpaceTime.fcData[(j+ioffset)/iwords].imag;
		      // Catch short integer overflow.
		      if ( idata[j]   == -32768 ) idata[j]   = 32767;
		      if ( idata[j+1] == -32768 ) idata[j+1] = 32767;
		    }
		  else
		    {
		      idata[j] = volume_amp(volume,j+ioffset);
		      // Catch short integer overflow.
		      if ( idata[j] == -32768 ) idata[j] = 32767;
		    }
		}
	    }
	  // Write the data.
	  if ( format == BFLOAT ) // floating point; either magnitude or complex
	    error = fwrite((char *)fdata,sizeof(float),iwords*n2d,file) != iwords*n2d;
	  else
	    error = fwrite((char *)idata,sizeof(short),iwords*n2d,file) != iwords*n2d;
	  if ( error )
	    {
	      printf("Error writing the output file %s.\n",full_file_name);
	      return(1);
	    }
	}
      fclose( file );

      /* Make the header file name `file_name.hdr' from the root `file_name'. */
      strcpy(full_file_name, file_name);
      if ( jz < 10 )
	sprintf(end_file_name,"_00%d.hdr",jz);
      else
	sprintf(end_file_name,"_0%d.hdr",jz);
      strcat(full_file_name,end_file_name);

      /* Now write the header file. */
      if ( !(file = fopen(full_file_name,"w")) )
	{
	  printf("Error: cannot open the header file %s.\n",full_file_name);
	  return(1);
	}
      if (flags&COMPLEX_OUTPUT_BIT )
	fprintf(file,"data-type complex  # data are stored as (real, imag) pairs\n");
      fprintf(file,"x %d\ny %d\nz %d\nt %d\nbyte-order %d\n",
	      output_file->dim.x,
	      output_file->dim.y,
	      output_file->dim.z,
	      output_file->nt,
	      machine_byte_order() );
      fprintf(file,"resolution %g %g %g\n",
	      output_file->resolution.x,output_file->resolution.y,output_file->resolution.z);
      fprintf(file,"origin %g %g %g\n",
	      output_file->origin.x,output_file->origin.y,output_file->origin.z);
      fprintf(file,"direction %g %g %g\n",
	      output_file->direction.x,output_file->direction.y,output_file->direction.z);
      fclose(file);
    }
  if ( format == BFLOAT )
    free_float_vector(fdata);
  else
    free_short_vector(idata);

  printf("%d slices of dimension %d x %d x %d were written to separate files.\n",
	 dim.z,output_file->dim.x,output_file->dim.y,output_file->nt);
  if ( update_file )   /* e.g., if the file has been used */
    printf("%d volumes now reside in files of type %s.\n\n",
	   output_file->nt, output_file->name);
  return(0);
}

struct volume_file *install_file( char *output_file_name )
{
  struct volume_file *output_file, *last_output_file;
  int nfile;
  
  /*
  ** Compare all existing file names against the input name.
  */
  nfile = 1;
  output_file = output_file_root;
  while ( output_file )       /* while output_file pointer != NULL */
    {
      if ( !(strcmp(output_file_name,output_file->name)) )
	return( output_file );
      last_output_file = output_file;
      output_file = output_file->next;  /* Go to the next file in the list. */
      nfile++;
    }
  /*
  ** If the name was not found (and thus this is a new file to add to the
  ** linked list of files), then allocate room for a file structure.
  */
  if ( (output_file=(struct volume_file *)malloc(VOLUME_FILE_SIZE)) )
    {
      strcpy( output_file->name, output_file_name );
      output_file->dim.x = output_file->dim.y = output_file->dim.z = output_file->nt = 0;
      output_file->registration = 0;
      output_file->next = NULL;
      if ( nfile == 1 )
	output_file_root = output_file;
      else
	last_output_file->next = output_file;
    }
  else
    {
      printf("Error: out of memory in routine `install_file'.\n");
      exit(1);
    }
  return( output_file );
}
