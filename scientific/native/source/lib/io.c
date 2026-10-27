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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "nifti1_io.h"
#include "util.h"

#define MIN_NII_HEADER_SIZE 348

static char CNULL = '\0';


int read_volume_header( BOOL verbose, char *file_name, struct XD_HEADER *header )
{
  int length, itype;
  char extension[LEN_STR];
  char *dot, *slash, *end;

  strcpy(header->file_name,file_name);
  // Give default values.
  // Set dimensions to 0
  header->dim.x = header->dim.y = header->dim.z = header->nt = 0;
  header->data_type = 0;   // default is magnitude data
  // Set resolutions to 1 (e.g., 1 voxel)
  header->resolution_t = 0.;
  header->resolution.x = header->resolution.y = header->resolution.z = 1.;
  header->origin.x     = header->origin.y     = header->origin.z     = 0.;
  header->direction.x  = header->direction.y  = header->direction.z  = 1.;
  header->byte_order = machine_byte_order();  // default from current byte order
  header->registration = 0;
  header->voxel_offset = 0.;
  header->scale_slope = header->scale_offset = 0.;
  header->qfac = 0.;
  header->quaternion.x = header->quaternion.y = header->quaternion.z = 0.;
  header->off_diagonal = FALSE;
  header->lDOF = 0;

  length = jip_int_count((JIP_INDEX)strlen(file_name));

  // A dot could take these forms:
  // "file.dat"
  // "../file.dat"
  // "../file
  //
  // Determine where a slash exists (end of string if missing)
  dot   = (char *)strrchr(file_name,'.');
  slash = (char *)strrchr(file_name,'/');
  end   = file_name + jip_int_count((JIP_INDEX)strlen(file_name));

  // Determine the file type (e.g., bshort or bfloat).
  if ( file_name[length-1] == '_' )
    { // See if the file name ends with '_'
      if ( read_freesurfer_header(file_name, header) )
	return(1);
    }
  else if ( (dot<end && slash==end) || (dot<end && dot>slash) ) // The file has an extension.
    { // (dot && !slash) || (dot && dot>slash)
      if ( get_extension( file_name, &itype, extension ) )
	return(1);
      if ( itype == NIFTI_NII || itype == NIFTI_IMG )
	{
	  header->file_type = itype;
	  if ( itype == NIFTI_IMG && verbose )
	    printf("Files with .img extensions are assumed to be NIFTI, not ANALYZE\n");
	  // The data storage type and other information will be read from the file header.
	  if ( read_nifti_header(verbose, file_name, header) )
	    return(1);
	}
	else
	  {
	    // Assign the storage type based upon the file name.
	    header->storage_type = header->file_type = itype;
	    // These formats use separate headers, so the voxel offset to data is 0.
	    if ( read_xdisplay_header(file_name, header) )
	      return(1);
	  }
    }
  else // treat the file as an xdisplay file without an extension
    {  // e.g., Bruker file of type pdata/1/2dseq
      // The storage type should be contained within the header (e.g., "storage-type short")
      header->storage_type = -1;
      strcpy(extension,file_name);
      strcat(extension,".hdr");
      // These formats use separate headers, so the voxel offset to data is 0.
      if ( read_xdisplay_header(extension, header) )
	return(1);
      if ( header->storage_type <= 0 )
	{
	  printf("Error: the file type is not recognized,\n");
	  printf("so it was assumed to be an xdisplay file without an extension.\n");
	  printf("However, the storage type was not read from file %s\n",extension);
	  printf("Valid types end in .nii, .img, .bshort, .bfloat, .blong, or _'\n");
	  return(1);
	}
      else
	header->file_type = header->storage_type;  // e.g., BSHORT storage is like .bshort extension
    }
  if(header->dim.x<=0||header->dim.y<=0||header->dim.z<=0||header->nt<=0||
     !isfinite(header->voxel_offset)||header->voxel_offset<0||header->voxel_offset>=(double)PTRDIFF_MAX||
     floor(header->voxel_offset)!=header->voxel_offset) {
    fprintf(stderr,"Invalid image dimensions or nonrepresentable file offset: %s\n",file_name);
    return 1;
  }
  // Create the inverse transformation between space and coordinates.
  header->xyz_to_ijk = invert_mat44( header->ijk_to_xyz );
  // Save some additional information for convenience.
  // FOV;
  header->fov.x = header->resolution.x * header->dim.x;
  header->fov.y = header->resolution.y * header->dim.y;
  header->fov.z = header->resolution.z * header->dim.z;
  // # of voxels in 2, 3, and 4d
  header->n2d = jip_product(header->dim.x,header->dim.y);
  header->n3d = jip_product(header->n2d,header->dim.z);
  header->n4d = jip_product(header->n3d,header->nt);
  if ( verbose )
    {
      printf("file %s, ",file_name);
      if ( header->data_type == 0 )
	printf("magnitude data, ");
      else if ( header->data_type == 1 )
	printf("complex data, ");
      else if ( header->data_type == 2 )
	printf("magnitude-phase data, ");
      if ( header->storage_type == BSHORT )
	printf("short ints\n");
      else if ( header->storage_type == BLONG )
	printf("long ints\n");
      else if ( header->storage_type == BFLOAT )
	printf("floating point\n");
      else if ( header->storage_type == UINT8 )
	printf("bytes\n");
      printf("voxels (%d,%d,%d,%d), res (%g,%g,%g,%g)\n",
	     header->dim.x,header->dim.y,header->dim.z,header->nt,
	     header->resolution.x,header->resolution.y,header->resolution.z,header->resolution_t);
    }

  return(0);
}

int read_nifti_header( BOOL verbose, char *file_name, struct XD_HEADER *header )
{
  nifti_1_header nifti_header;
  nifti1_extender pad;
  FILE *file;
  char nifti_header_name[LEN_STR];
  size_t nread;int itype, esize, ecode;
  int j, j1, j2;
  BOOL swap_bytes;

  if ( header->file_type == NIFTI_IMG )
    {
      // Replace the extension so that file.img -> file.hdr
      if ( replace_extension(file_name, &itype, "hdr",nifti_header_name) )
	return(1);
    }
  else
    strcpy(nifti_header_name,file_name);

  if ( !(file = fopen(nifti_header_name,"r")) )
    {
      printf("Error: cannot open Nifti file %s.\n",nifti_header_name);
      return(1);
    }
  // Read the nifti header.
  nread = fread(&nifti_header, MIN_NII_HEADER_SIZE, 1, file);
  if (nread != 1)
    {
      fprintf(stderr, "\nError reading header file %s\n",nifti_header_name);
      exit(1);
    }
  // Read the pad at the end of the header.
  nread = fread(&pad, 4, 1, file);
  if ( (nread!=0) && (pad.extension[0]!=0) )
    {
      if ( (nread=fread(&esize, 4, 1, file)) != 1 )
	{
	  printf("Error: A NIFTI1 extension was indicated, but it is missing.\n");
	  return(1);
	}
      if ( fread(&ecode, 4, 1, file) != 1 )
	{
	  printf("Error: A NIFTI1 extension was indicated, but the ecode is missing.\n");
	  return(1);
	}
      if ( ecode == 20 ) // my unregistered code
	{
	  if ( fread(&(header->lDOF), 4, 1, file) != 1 )
	    {
	      printf("Error read degrees of freedom.\n");
	      return(1);
	    }
	}
    }

  fclose(file);

  // Examine dim[0] to see if it is in the range 1->7
  swap_bytes = (nifti_header.dim[0]<1 || nifti_header.dim[0]>7);
  if ( swap_bytes )
    { // byte-swapping has not been tested for NIFTI files.
      // shorts
      swap_16(&(nifti_header.session_error));
      swap_16(&(nifti_header.intent_code));
      swap_16(&(nifti_header.datatype));
      swap_16(&(nifti_header.bitpix));
      swap_16(&(nifti_header.slice_start));
      swap_16(&(nifti_header.slice_end));
      swap_16(&(nifti_header.qform_code));
      swap_16(&(nifti_header.sform_code));
      for (j=0; j<8; j++)
	swap_16(nifti_header.dim+j);
      // ints
      swap_32(&(nifti_header.sizeof_hdr));
      swap_32(&(nifti_header.extents));
      swap_32(&(nifti_header.glmax));
      swap_32(&(nifti_header.glmin));
      // floats
      swap_32(&(nifti_header.intent_p1));
      swap_32(&(nifti_header.intent_p2));
      swap_32(&(nifti_header.intent_p3));
      swap_32(&(nifti_header.vox_offset));
      swap_32(&(nifti_header.scl_slope));
      swap_32(&(nifti_header.scl_inter));
      swap_32(&(nifti_header.cal_min));
      swap_32(&(nifti_header.cal_max));
      swap_32(&(nifti_header.slice_duration));
      swap_32(&(nifti_header.toffset));
      swap_32(&(nifti_header.quatern_b));
      swap_32(&(nifti_header.quatern_c));
      swap_32(&(nifti_header.quatern_d));
      swap_32(&(nifti_header.qoffset_x));
      swap_32(&(nifti_header.qoffset_y));
      swap_32(&(nifti_header.qoffset_z));
      for (j=0; j<4; j++)
	{
	  swap_32(nifti_header.srow_x+j);
	  swap_32(nifti_header.srow_y+j);
	  swap_32(nifti_header.srow_z+j);
	}
      for (j=0; j<8; j++)
	swap_32(nifti_header.pixdim+j);
    }

  // Assign values to the x,y,z,t dimensions.
  header->dim.x    = nifti_header.dim[1];
  header->dim.y    = nifti_header.dim[2];
  header->dim.z    = nifti_header.dim[3];
  header->nt       = nifti_header.dim[4];
  if ( nifti_header.dim[0] < 4 )
    header->nt = 1;
  else if ( nifti_header.dim[0] > 4 )
    {
      printf("Warning: jip does not currently handle data of dimensionality (%d) > 4.\n",
      	     nifti_header.dim[0]);
      printf("Proceeding using the 1st 4 dimensions...\n");
    }
  // Assign resolutions.
  header->resolution.x = nifti_header.pixdim[1];
  header->resolution.y = nifti_header.pixdim[2];
  header->resolution.z = nifti_header.pixdim[3];
  header->resolution_t = nifti_header.pixdim[4];
  // The data can contain a slope and offset.
  header->scale_slope  = nifti_header.scl_slope;
  header->scale_offset = nifti_header.scl_inter;
  header->qfac         = nifti_header.pixdim[0];
  header->quaternion.x = nifti_header.quatern_b;
  header->quaternion.y = nifti_header.quatern_c;
  header->quaternion.z = nifti_header.quatern_d;
  // Define the data storage type.
  // Only a few data types are supported.
  if ( nifti_header.datatype == NIFTI_TYPE_UINT8 )
    header->storage_type = UINT8;
  else if ( nifti_header.datatype == NIFTI_TYPE_INT16 )
    header->storage_type = BSHORT;
  else if ( nifti_header.datatype == NIFTI_TYPE_UINT16 )
    header->storage_type = USHORT;
  else if ( nifti_header.datatype == NIFTI_TYPE_INT32 )
    header->storage_type = BLONG;
  else if ( nifti_header.datatype == NIFTI_TYPE_FLOAT32 )
    header->storage_type = BFLOAT;
  else if ( nifti_header.datatype == NIFTI_TYPE_COMPLEX64 )
    {
      header->storage_type = BFLOAT;
      header->data_type    = 1;
    }
  else if ( nifti_header.datatype == NIFTI_TYPE_RGB24 )
    {
      header->storage_type = UINT8;
      header->data_type    = 3;
    }
  else
    {
      printf("Error: the Nifti data storage type %d is not supported.\n",nifti_header.datatype);
      return(1);
    }

  // define the voxel offset
  header->voxel_offset = nifti_header.vox_offset;

  // Make sure that dimensions (x,y,z,t) have been read and make sense.
  if ( header->dim.x <= 0 )
    {
      printf("Error: Value for x dimension not found in header file '%s'\n",nifti_header_name);
      return(1);
    }
  else if ( header->dim.y <= 0 )
    {
      printf("Error: Value for y dimension not found in header file '%s'\n",nifti_header_name);
      return(1);
    }
  else if ( header->dim.z <= 0 )
    {
      printf("Error: Value for z dimension not found in header file '%s'\n",nifti_header_name);
      return(1);
    }
  else if ( header->nt <= 0 )
    {
      printf("Error: Value for t dimension not found in header file '%s'\n",nifti_header_name);
      return(1);
    }

  // Define the tranformation between voxel coordinates and space.
  if ( nifti_header.qform_code <= 0 && nifti_header.sform_code <= 0 ) // no valid spatial form
    { // Setting the origin to zero seems wrong, but this follows the nifti1.h documentation.
      if ( verbose )
	printf("The NIFTI header for file %s uses Method 1: the old analyze format\n",
	       file_name);
      header->origin.x = header->origin.y = header->origin.z = 0.;
      header->ijk_to_xyz = scale_and_shift_to_mat44(header->resolution, header->origin, header->direction);
    }
  else // at least one valid spatial form
    {
      if ( nifti_header.sform_code <= 0 )
	{
	  if ( verbose )
	    printf("The NIFTI header for file %s uses Method 2: quaternion rotations\n",file_name);
	  header->ijk_to_xyz = nifti_quatern_to_mat44( nifti_header.quatern_b,   // qb
						       nifti_header.quatern_c,   // qc
						       nifti_header.quatern_d,   // qd
						       nifti_header.qoffset_x,   // offset_x
						       nifti_header.qoffset_y,   // offset_y
						       nifti_header.qoffset_z,   // offset_z
						       nifti_header.pixdim[1],   // resolution_x
						       nifti_header.pixdim[2],   // resolution_y
						       nifti_header.pixdim[3],   // resolution_z
						       nifti_header.pixdim[0]);  // qfac
	}
      else
	{
	  if ( verbose )
	    printf("The NIFTI header for file %s uses Method 3: affine transformation of voxel indices\n",
		   file_name);
	  for(j1=0;j1<4;j1++) {
            header->ijk_to_xyz.m[0][j1]=nifti_header.srow_x[j1];
            header->ijk_to_xyz.m[1][j1]=nifti_header.srow_y[j1];
            header->ijk_to_xyz.m[2][j1]=nifti_header.srow_z[j1];
            header->ijk_to_xyz.m[3][j1]=j1==3?1.:0.;
          }
	}
    }
  /* sform can contain scales different from pixdim, including oblique axes. */
  header->resolution.x = sqrt(SQR(header->ijk_to_xyz.m[0][0])+SQR(header->ijk_to_xyz.m[1][0])+SQR(header->ijk_to_xyz.m[2][0]));
  header->resolution.y = sqrt(SQR(header->ijk_to_xyz.m[0][1])+SQR(header->ijk_to_xyz.m[1][1])+SQR(header->ijk_to_xyz.m[2][1]));
  header->resolution.z = sqrt(SQR(header->ijk_to_xyz.m[0][2])+SQR(header->ijk_to_xyz.m[1][2])+SQR(header->ijk_to_xyz.m[2][2]));
  for(j1=0;j1<3;j1++)for(j2=0;j2<4;j2++)if(!isfinite(header->ijk_to_xyz.m[j1][j2]))return 1;
  if(!isfinite(header->resolution.x)||!isfinite(header->resolution.y)||!isfinite(header->resolution.z)||
     header->resolution.x<=0||header->resolution.y<=0||header->resolution.z<=0)return 1;
  {
    double det=(double)header->ijk_to_xyz.m[0][0]*(header->ijk_to_xyz.m[1][1]*header->ijk_to_xyz.m[2][2]-header->ijk_to_xyz.m[1][2]*header->ijk_to_xyz.m[2][1])
      -(double)header->ijk_to_xyz.m[0][1]*(header->ijk_to_xyz.m[1][0]*header->ijk_to_xyz.m[2][2]-header->ijk_to_xyz.m[1][2]*header->ijk_to_xyz.m[2][0])
      +(double)header->ijk_to_xyz.m[0][2]*(header->ijk_to_xyz.m[1][0]*header->ijk_to_xyz.m[2][1]-header->ijk_to_xyz.m[1][1]*header->ijk_to_xyz.m[2][0]);
    if(!isfinite(det)||fabs(det)<1.e-12)return 1;
  }
  // Set the origin based upon the transformation matrix.
  header->origin.x = header->ijk_to_xyz.m[0][3];
  header->origin.y = header->ijk_to_xyz.m[1][3];
  header->origin.z = header->ijk_to_xyz.m[2][3];
  // Set the directions based upon the transformation matrix.
  if ( header->ijk_to_xyz.m[0][0] > 0.  )
    header->direction.x = 1.;
  else
    header->direction.x = -1.;
  if ( header->ijk_to_xyz.m[1][1] > 0.  )
    header->direction.y = 1.;
  else
    header->direction.y = -1.;
  if ( header->ijk_to_xyz.m[2][2] > 0.  )
    header->direction.z = 1.;
  else
    header->direction.z = -1.;
  // Issue a warning for non-zero off-diagnonal elements.
  header->off_diagonal = FALSE;
  for (j1=0; j1<3; j1++)
    {
      for (j2=0; j2<3; j2++)
	{
	  if ( j1 != j2 && header->ijk_to_xyz.m[j1][j2] != 0. )
	    header->off_diagonal = TRUE;
	}
    }

  return(0);
}

int read_xdisplay_header( char *file_name, struct XD_HEADER *header )
{
  FILE *file;
  char xdisplay_header_name[LEN_STR], whole_line[LEN_STR];
  int nline, i1, ivalue;

  /*
  ** Replace the extension so that file.bshort -> file.hdr
  ** Open the file header and read the x & y dimensions and the # images.
  */
  if ( replace_extension(file_name, &(header->file_type), "hdr", xdisplay_header_name) ) return(1);
  if ( !(file = fopen(xdisplay_header_name,"r")) )
    {
      printf("Error: cannot open xdisplay file %s.\n",xdisplay_header_name);
      return(1);
    }
  // Read the keyword-delimited file
  nline = 1;
  // First try to read the 1st line as a keyword
  get_line_with_comment(file, whole_line, LEN_STR);
  if ( !(sscanf(whole_line,"%d",&i1)<0) )
    {
      if ( decode_xdisplay_header_keyword(whole_line,header) )
	{ // If that failed, try the xds format
	  if ( sscanf(whole_line,"%d %d %d %d",&i1,&i1,&i1,&i1) != 4 )
	    {
	      printf("Error reading the 1st line of the xds header file '%s'.\n",
		     xdisplay_header_name);
	      return(1);
	    }
	}
    }
  // Continue reading other lines.
  while ( get_line_with_comment(file,whole_line,LEN_STR) )
    {
      if ( sscanf(whole_line,"%d",&i1) < 0 )  /* ... then this is a blank line */
	continue;
      ivalue = decode_xdisplay_header_keyword(whole_line,header);
      if ( ivalue < 0 )
	break; // the keyword "info" means to ignore everything else in the file.
      else if ( ivalue > 0 )
	{ // This was an error.
	  printf("Error reading line %d of file '%s'\n",nline,xdisplay_header_name);
	  return(1);
	}
      nline++;
    }
  fclose( file );
  // Make sure that dimensions (x,y,z,t) have been read.
  if ( header->dim.x == 0 )
    {
      printf("Error: Value for x dimension not found in header file '%s'\n",xdisplay_header_name);
      return(1);
    }
  else if ( header->dim.y == 0 )
    {
      printf("Error: Value for y dimension not found in header file '%s'\n",xdisplay_header_name);
      return(1);
    }
  else if ( header->dim.z == 0 )
    {
      printf("Error: Value for z dimension not found in header file '%s'\n",xdisplay_header_name);
      return(1);
    }
  else if ( header->nt == 0 )
    {
      printf("Error: Value for t dimension not found in header file '%s'\n",xdisplay_header_name);
      return(1);
    }

  // Define the tranformation between voxel coordinates and space.
  header->ijk_to_xyz = scale_and_shift_to_mat44(header->resolution, header->origin, header->direction);

  return(0);
}

int read_freesurfer_header( char *file_name, struct XD_HEADER *header )
{
  struct FS_HEADER fs_header;
  FILE *file;
  char header_file_name[LEN_STR], whole_line[LEN_STR], slice_file_name[LEN_STR], slice_name[LEN_STR];
  int length, nline, i1;
  float xr, xa, xs, yr, ya, ys;

  header->file_type = FREESURFER;

  // Replace the extension so that file.bshort -> file.hdr
  // Open the file header and read the x & y dimensions and the # images.
  strcpy(header_file_name,file_name);
  length = jip_int_count((JIP_INDEX)strlen(header_file_name));
  if ( header_file_name[length-1] != '_' )
    {
      printf("Programming error: file name should end with '_' in read_freesurfer_header\n");
      exit(1);
    }
  *(header_file_name + length - 1) = CNULL;
  strcat(header_file_name,".bhdr");

  if ( !(file = fopen(header_file_name,"r")) )
    {
      printf("Error: cannot open freesurfer bhdr file %s.\n",header_file_name);
      return(1);
    }
  // Begin reading the file
  nline = 1;
  while ( get_line_with_comment(file,whole_line,LEN_STR) )
    {
      if ( sscanf(whole_line,"%d",&i1) < 0 )  /* ... then this is a blank line */
	continue;
      if ( decode_freesurfer_header_keyword(whole_line,header,&fs_header) )
	{
	  printf("Error reading line %d of file '%s'\n",nline,header_file_name);
	  return(1);
	}
      nline++;
    }
  fclose( file );
  // Make sure that dimensions (x,y,z,t) have been read.
  if ( header->dim.x == 0 )
    {
      printf("Error: Value for x dimension not found in header file '%s'\n",header_file_name);
      return(1);
    }
  else if ( header->dim.y == 0 )
    {
      printf("Error: Value for y dimension not found in header file '%s'\n",header_file_name);
      return(1);
    }
  else if ( header->dim.z == 0 )
    {
      printf("Error: Value for z dimension not found in header file '%s'\n",header_file_name);
      return(1);
    }
  else if ( header->nt == 0 )
    {
      printf("Error: Value for t dimension not found in header file '%s'\n",header_file_name);
      return(1);
    }
  // Calculate resolutions from the free-surfer header.
  xr = (fs_header.trr - fs_header.tlr) / header->dim.x;
  xa = (fs_header.tra - fs_header.tla) / header->dim.x;
  xs = (fs_header.trs - fs_header.tls) / header->dim.x;
  header->resolution.x = sqrt((double)(xr*xr + xa*xa + xs*xs));
  yr = (fs_header.brr - fs_header.trr) / header->dim.y;
  ya = (fs_header.bra - fs_header.tra) / header->dim.y;
  ys = (fs_header.brs - fs_header.trs) / header->dim.y;
  header->resolution.y = sqrt((double)(yr*yr + ya*ya + ys*ys));

  // Finally, get the byte order. This is stored in the .hdr files, so use _000 file.
  strcpy(header_file_name,file_name);
  strcat(header_file_name,"000.hdr");
  if ( !(file = fopen(header_file_name,"r")) )
    {
      printf("Error: cannot open freesurfer file %s.\n",header_file_name);
      return(1);
    }
  fscanf(file,"%d %d %d %d",&i1,&i1,&i1,&(header->byte_order));
  fclose(file);

  // The file types could be .bshort or .bfloat
  // Attempt to open the first file to see which it is.
  strcpy(slice_file_name,file_name);
  sprintf(slice_name,"000.bshort");
  strcat(slice_file_name,slice_name);
  if ( !(file = fopen(slice_file_name,"r")) )
    {
      // Attempt to read bfloat file as an alternative.
      strcpy(slice_file_name,file_name);
      sprintf(slice_name,"000.bfloat");
      strcat(slice_file_name,slice_name);
      if ( !(file = fopen(slice_file_name,"r")) )
	{
	  printf("\nError: cannot open file %s as bshort or bfloat.\n",file_name);
	  return(1);
	}
      header->storage_type = BFLOAT;
    }
  else
    header->storage_type = BSHORT;

  // Define the tranformation between voxel coordinates and space.
  header->ijk_to_xyz = scale_and_shift_to_mat44(header->resolution, header->origin, header->direction);

  return(0);
}

int decode_xdisplay_header_keyword( char *line, struct XD_HEADER *header)
{
  char keyword[LEN_STR], *argument, *cptr;
  int error;

  cptr = next_non_white(line);  // remove initial blanks or tabs
  sscanf(cptr,"%s",keyword);
  argument = next_non_white( cptr + jip_int_count((JIP_INDEX)strlen(keyword)) );

  error = 0;

  if ( !strcmp(keyword,"info") || !strcmp(keyword,"info:") 
       || !strcmp(keyword,"information") || !strcmp(keyword,"information:") )
    return(-1);
  else if ( !strcmp(keyword,"x") )
    error = sscanf(argument,"%d",&(header->dim.x)) != 1;
  else if ( !strcmp(keyword,"y") )
    error = sscanf(argument,"%d",&(header->dim.y)) != 1;
  else if ( !strcmp(keyword,"z") )
    error = sscanf(argument,"%d",&(header->dim.z)) != 1;
  else if ( !strcmp(keyword,"t") )
    error = sscanf(argument,"%d",&(header->nt)) != 1;
  else if ( !strcmp(keyword,"matrix") )
    {
      // try to read 4 values
      error = sscanf(argument,"%d %d %d %d",
		     &(header->dim.x),
		     &(header->dim.y),
		     &(header->dim.z),
		     &(header->nt)) != 4;
      if ( error ) // try to just get 3 values
	error = sscanf(argument,"%d %d %d",
		       &(header->dim.x),
		       &(header->dim.y),
		       &(header->dim.z)) != 3;
    }
  else if ( !strcmp(keyword,"byte-order") )
    error = sscanf(argument,"%d",&(header->byte_order)) != 1;
  else if ( !strcmp(keyword,"resolution") )
    {
      // try to read 4 values
      error = sscanf(argument,"%f %f %f %f",
		     &(header->resolution.x),
		     &(header->resolution.y),
		     &(header->resolution.z),
		     &(header->resolution_t)) != 4;
      if ( error ) // try to just get 3 values
	{
	  error = sscanf(argument,"%f %f %f",
			 &(header->resolution.x),
			 &(header->resolution.y),
			 &(header->resolution.z)) != 3;
	  header->resolution_t = 0;
	}
    }
  else if ( !strcmp(keyword,"origin") )
    error = sscanf(argument,"%f %f %f",
		   &(header->origin.x),&(header->origin.y),&(header->origin.z)) != 3;
  else if ( !strcmp(keyword,"direction") )
    {
      error = sscanf(argument,"%f %f %f",
		     &(header->direction.x),&(header->direction.y),&(header->direction.z)) != 3;
      if ( fabs((double)header->direction.x) != 1. ||
	   fabs((double)header->direction.y) != 1. ||
	   fabs((double)header->direction.z) != 1. )
	{
	  printf("Error: allowed values for direction (%g,%g,%g) are 1 and -1.\n",
		 header->direction.x,header->direction.y,header->direction.z);
	  error = 1;
	}
    }
  else if ( !strcmp(keyword,"data-type") )
    {
      sscanf(argument,"%s",keyword);
      header->data_type = 0;   // default is magnitude only
      if ( !strcmp(keyword,"magnitude") )
	header->data_type = 0; // magnitude
      else if ( !strcmp(keyword,"complex") )
	header->data_type = 1; // real-imaginary
      else if ( !strcmp(keyword,"real-imaginary") )
	header->data_type = 1; // real-imaginary
      else if ( !strcmp(keyword,"magnitude-phase") )
	header->data_type = 2; // magnitude-phase
      else
	{
	  printf("Error: arguments for data-type are\n");
	  printf("1) magnitude (this is the default),\n");
	  printf("2) complex (or real-imaginary),\n");
	  printf("3) magnitude-phase\n");
	  error = 1;
	}
    }
  else if ( !strcmp(keyword,"storage-type") )
    {
      sscanf(argument,"%s",keyword);
      if ( !strcmp(keyword,"short") )
	header->storage_type = BSHORT; // short
      else if ( !strcmp(keyword,"float") )
	header->storage_type = BFLOAT; // float
      else if ( !strcmp(keyword,"long") )
	header->storage_type = BLONG; // long
      else if ( !strcmp(keyword,"unsigned-short") )
	header->storage_type = USHORT; // unsigned-short
      else if ( !strcmp(keyword,"unsigned-char") )
	header->storage_type = UINT8; // unsigned char (8-byte unsigned int)
      else
	{
	  printf("Error: arguments for storage-type are ...\n");
	  printf("    short, float, long, unsigned-short, unsigned-char\n");
	  error = 1;
	}
    }
  else if ( !strcmp(keyword,"registration") )
    {
      sscanf(argument,"%s",keyword);
      header->data_type = 0;
      if ( !strcmp(keyword,"paxinos-rat") )
	header->registration = REG_PW_RAT_OLD;
      else if ( !strcmp(keyword,"PW-rat") )
	header->registration = REG_PW_RAT;
      else
	{
	  printf("Error: arguments for registration are\n");
	  printf("1) paxinos-rat, and\n");
	  printf("2) nothing else (paxinos-monkey later?)\n");
	  error = 1;
	}
    }
  else if ( !strcmp(keyword,"degrees-of-freedom") )
    error = sscanf(argument,"%d",&(header->lDOF)) != 1;
  else // not found
    error = 1;

  return(error);
}

int decode_freesurfer_header_keyword( char *line,
				      struct XD_HEADER *xd_header,
				      struct FS_HEADER *fs_header)
{
  char keyword[LEN_STR], *argument;

  sscanf(line,"%s",keyword);
  argument = next_non_white( strchr(line,keyword[0]) + jip_int_count((JIP_INDEX)strlen(keyword)) );

  if ( !strcmp(keyword,"cols:") )
    {
      sscanf(argument,"%d",&(xd_header->dim.x));
      return(0);
    }
  else if ( !strcmp(keyword,"rows:") )
    {
      sscanf(argument,"%d",&(xd_header->dim.y));
      return(0);
    }
  else if ( !strcmp(keyword,"nslices:") )
    {
      sscanf(argument,"%d",&(xd_header->dim.z));
      return(0);
    }
  else if ( !strcmp(keyword,"n_time_points:") )
    {
      sscanf(argument,"%d",&(xd_header->nt));
      return(0);
    }
  else if ( !strcmp(keyword,"slice_thick:") )
    {
      sscanf(argument,"%f",&(xd_header->resolution.z));
      return(0);
    }
  else if ( !strcmp(keyword,"top_left_r:") )
    {
      sscanf(argument,"%f",&(fs_header->tlr));
      return(0);
    }
  else if ( !strcmp(keyword,"top_left_a:") )
    {
      sscanf(argument,"%f",&(fs_header->tla));
      return(0);
    }
  else if ( !strcmp(keyword,"top_left_s:") )
    {
      sscanf(argument,"%f",&(fs_header->tls));
      return(0);
    } 
  else if ( !strcmp(keyword,"top_right_r:") )
    {
      sscanf(argument,"%f",&(fs_header->trr));
      return(0);
    }
  else if ( !strcmp(keyword,"top_right_a:") )
    {
      sscanf(argument,"%f",&(fs_header->tra));
      return(0);
    }
  else if ( !strcmp(keyword,"top_right_s:") )
    {
      sscanf(argument,"%f",&(fs_header->trs));
      return(0);
    }
  else if ( !strcmp(keyword,"bottom_right_r:") )
    {
      sscanf(argument,"%f",&(fs_header->brr));
      return(0);
    }
  else if ( !strcmp(keyword,"bottom_right_a:") )
    {
      sscanf(argument,"%f",&(fs_header->bra));
      return(0);
    }
  else if ( !strcmp(keyword,"bottom_right_s:") )
    {
      sscanf(argument,"%f",&(fs_header->brs));
      return(0);
    }
  else if ( !strcmp(keyword,"normal_r:") )
    {
      sscanf(argument,"%f",&(fs_header->normal_r));
      return(0);
    }
  else if ( !strcmp(keyword,"normal_a:") )
    {
      sscanf(argument,"%f",&(fs_header->normal_a));
      return(0);
    }
  else if ( !strcmp(keyword,"normal_s:") )
    {
      sscanf(argument,"%f",&(fs_header->normal_s));
      return(0);
    }
  else if ( !strcmp(keyword,"image_te:") )
    {
      sscanf(argument,"%f",&(fs_header->te));
      return(0);
    }
  else if ( !strcmp(keyword,"image_tr:") )
    {
      sscanf(argument,"%f",&(fs_header->tr));
      xd_header->resolution_t = fs_header->tr;
      return(0);
    }
  else if ( !strcmp(keyword,"image_ti:") )
    {
      sscanf(argument,"%f",&(fs_header->ti));
      return(0);
    }
  else if ( !strcmp(keyword,"flip_angle:") )
    {
      sscanf(argument,"%f",&(fs_header->flip_angle));
      return(0);
    }
 else // not found
    return(1);
}

int get_extension( char *file_name, int *itype, char *extension )
{
  char *cptr;
  
  // Determine the file type (e.g., bshort or bfloat) if possible.
  // if ( (dot && !slash) || (dot && dot later than slash)
  if ( (cptr=(char *)strrchr(file_name,'.')) )
    {
      extension = cptr+1;
      if ( extension[jip_int_count((JIP_INDEX)strlen(extension))-1] == 10 )  // line feed can sneak into file_name
	*(extension + jip_int_count((JIP_INDEX)strlen(extension))-1) = CNULL;// when this string comes from file input
      if ( !strcmp(extension,"bshort") )
	*itype = BSHORT;
      else if ( !strcmp(extension,"blong") )
	*itype = BLONG;
      else if ( !strcmp(extension,"bfloat") )
	*itype = BFLOAT;
      else if ( !strcmp(extension,"ushort") )
	*itype = USHORT;
      else if ( !strcmp(extension,"nii") )
	*itype = NIFTI_NII;
      else if ( !strcmp(extension,"img") )
	*itype = NIFTI_IMG;
      else
	*itype = -1;  // unknown
    }
  else
    {
      printf("The file name has no extension!\n");
      return(1);
    }
  return(0);
}

int replace_extension( char *file_name, int *itype, char *extension, char *new_file_name )
{
  char original_extension[LEN_STR], *cptr;
  
  if ( get_extension(file_name, itype, original_extension) )
    return(1);

  // Replace the original extension with the new one.
  strcpy(new_file_name,file_name);
  if ( (cptr=(char *)strrchr(new_file_name,(int)'.')) )
    strcpy(cptr+1,extension);
  else
    {
      printf("Programming error in replace_extension.");
      exit(1);
    }

  return(0);
}

int machine_byte_order()  // find byte order for cpu
{
  union
  {
    unsigned char bb[2];
    short         ishort;
  } test;

  int byte_order;

  test.bb[0] = 1;
  test.bb[1] = 0;

  if ( test.ishort == 1 )
    byte_order = 1;
  else
    byte_order = 0;

  return (byte_order);
}

void swap_16( void *ptr )
{
  unsigned char uc, *ucptr;

  ucptr = (unsigned char *)ptr;
  uc = *ucptr; *ucptr = *(ucptr+1); *(ucptr+1) = uc;

  return;
}

void swap_32( void *ptr )
{
  unsigned char uc, *ucptr;

  ucptr= (unsigned char*)ptr;
  uc= *ucptr;     *ucptr= *(ucptr+3); *(ucptr+3)= uc;
  uc= *(ucptr+1); *(ucptr+1) = *(ucptr+2); *(ucptr+2)= uc;

  return;
}

int xdisplay_format( char *file_name )
{
  char *cptr, extension[LEN_STR];
  int format;

  extension[0] = CNULL;
  cptr = strrchr(file_name,'.');
  if ( cptr == 0 ) return(-1);
  sscanf(cptr+1,"%s",extension);
  *cptr = CNULL;

  if ( !strcmp(extension,"bfloat") )
    format = BFLOAT;
  else if ( !strcmp(extension,"bshort") )
    format = BSHORT;
  else if ( !strcmp(extension,"ushort") )
    format = USHORT;
  else if ( !strcmp(extension,"img") )
    format = NIFTI_IMG;
  else if ( !strcmp(extension,"nii") )
    format = NIFTI_NII;
  else
    format = -1;

  return(format);
}
