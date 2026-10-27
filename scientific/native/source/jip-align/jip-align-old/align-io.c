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
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include "nifti1_io.h"
#include "util.h"
#include "align.h"


void print_parameter_set( int mode, int ilevel )
     // mode: 2 = final headers and values
     // mode: 1 = print headers and values
     //       0 = print only values
{
  char prompt[LEN_STR], prompt1[LEN_STR], name[LEN_STR];
  static float parameter_save[NAFFINE];
  int j;

  if ( mode != 0 )
    {
      // Give the final parameter set.  
      sprintf(prompt,"level, MI");
      for (j=0; j<12; j++)
	{
	  sscanf(AlignAid.name[j],"%s",name); // to remove spaces
	  sprintf(prompt1,",%7s",AlignAid.name[j]);
	  strcat(prompt,prompt1);
	}
      fprintf(stdout,"%s\n",prompt);
      fflush(stdout);
      // Initialize saved parameters to unphysical values
      for (j=0; j<NAFFINE; j++)
	parameter_save[j] = 1.e10;
    }

  if ( mode == 2 )
    sprintf(prompt,"    %5.3f",AlignAid.CF_Affine);
  else if ( Alignment.value[0] == parameter_save[0] )
    sprintf(prompt,"%2d) %5s",ilevel," \"\"");
  else
    sprintf(prompt,"%2d) %5.3f",ilevel,AlignAid.CF_Affine);
  for (j=0; j<12; j++)
    {
      if ( AlignAid.increment[j] == 0. )
	sprintf(prompt1,",%7s"," **");
      else if ( Alignment.value[j] == parameter_save[j] )
	sprintf(prompt1,",%7s"," \"\"");
      else
	sprintf(prompt1,",%7.3f",Alignment.value[j]);
      strcat(prompt,prompt1);
    }
  fprintf(stdout,"%s\n",prompt);
  fflush(stdout);

  // Save parameters
  for (j=0; j<NAFFINE; j++)
    parameter_save[j] = Alignment.value[j];
}

int save_alignment_parameters()
{
  char cwd[LEN_STR], file_name[LEN_STR];
  FILE *file;
  BOOL Save_NonLinear;
  int j;

  if ( !(file = fopen("align.com","w")) )
    {
      printf("\nError: cannot open file align.com\n");
      return(1);
    }

  if ( !Alignment.RigidBody )
    fprintf(file,"set registration-type deck-of-cards\n");

  // Matrix
  fprintf(file,"set registration-matrix %d %d %d\n",
	  Alignment.dimension.x,Alignment.dimension.y,Alignment.dimension.z);
  // Resolution
  fprintf(file,"set registration-resolution %g %g %g\n",
	  Alignment.resolution.x,Alignment.resolution.y,Alignment.resolution.z);
  // Origin
  fprintf(file,"set registration-origin %g %g %g\n",
	  Alignment.origin.x,Alignment.origin.y,Alignment.origin.z);
  // Direction
  fprintf(file,"set registration-direction %g %g %g\n",
	  Alignment.direction.x,Alignment.direction.y,Alignment.direction.z);
  // Orientation
  fprintf(file,"set registration-orientation %d %d %d %d\n",
	  Alignment.OrientRotation.x,Alignment.OrientRotation.y,
	  Alignment.OrientRotation.z,Alignment.OrientSwapZ);
  // Wrapping
  if ( Alignment.WrapAround.x != 0 || Alignment.WrapAround.y != 0 || Alignment.WrapAround.z != 0 )
    fprintf(file,"set registration-wrapping %d %d %d\n",
	    Alignment.WrapAround.x,Alignment.WrapAround.y,Alignment.WrapAround.z);

  // Affine parameters
  fprintf(file,"set registration-translations %g %g %g\n",
	  Alignment.value[0],Alignment.value[1],Alignment.value[2]);
  fprintf(file,"set registration-rotations %g %g %g\n",
	  Alignment.value[3],Alignment.value[4],Alignment.value[5]);
  fprintf(file,"set registration-inflations %g %g %g\n",
	  Alignment.value[6],Alignment.value[7],Alignment.value[8]);
  fprintf(file,"set registration-skews %g %g %g\n",
	  Alignment.value[9],Alignment.value[10],Alignment.value[11]);

  Save_NonLinear = FALSE;
  for (j=0; j<distort_x.n3d; j++)
    {
      if ( distort_x.SpaceTime.fData[j] != 0. ) Save_NonLinear = TRUE;
      if ( distort_y.SpaceTime.fData[j] != 0. ) Save_NonLinear = TRUE;
      if ( distort_z.SpaceTime.fData[j] != 0. ) Save_NonLinear = TRUE;
    }

  if ( Save_NonLinear )
    {
      if ( !getcwd(cwd,LEN_STR) )
	{
	  printf("Error: non-linear alignment parameters not saved.\n");
	  fclose(file);
	  return(1);
	}
      else
	{
	  printf("cwd = %s\n",cwd);
	  strcpy(file_name,cwd);
	  strcat(file_name,"/distortions.nii");
	  if ( write_distortion_file(file_name) )
	    {
	      strcat(file_name,".nii");  // write_distortions modifies file name
	      printf("Error writing file %s\n",file_name);
	      return(1);
	    }
	  else
	    {
	      strcat(file_name,".nii");  // write_distortions modifies file name
	      fprintf(file,"set registration-distortions %s\n",file_name);
	    }
	}
    }

  fclose(file);
  printf("Alignment parameters were saved in file align.com\n");
  return(0);
}

int recall_alignment_parameters()
{
  struct XD_HEADER header;
  FILE *file;
  char whole_line[LEN_STR], set[LEN_STR], parameter[LEN_STR], *value;
  float rx, ry, rz;
  int nx, ny, nz, j;

  if ( !(file = fopen("align.com","r")) )
    {
      printf("\nError: cannot open file align.com\n");
      return(1);
    }
  while ( get_line_with_comment(file, whole_line, LEN_STR) )
    {
      // Lines should be of a form "set parameter value(s)"
      if ( sscanf(whole_line,"%s",set) != 1 )
	continue;
      value = next_non_white(whole_line + strlen(set));
      sscanf(value,"%s",parameter);
      value = next_non_white(value + strlen(parameter));
      // Interpret the parameter, and then read the value.
      if ( !strcmp(parameter,"registration-type") )
	{
	  sscanf(value,"%s",parameter);
	  if ( !strcmp(parameter,"deck-of-cards") )
	    Alignment.RigidBody = 0;
	  else
	    Alignment.RigidBody = 1;
	  //	  printf("rigid = %d\n",Alignment.RigidBody);
	}
      else if ( !strcmp(parameter,"registration-space") )
	{
	  sscanf(value,"%s",parameter);
	}
      else if ( !strcmp(parameter,"registration-matrix") )
	{
	  if ( sscanf(value,"%d %d %d",&nx,&ny,&nz) != 3 )
	    {
	      printf("Error reading the value of `set registration-orientation'\n");
	      return(1);
	    }
	  else if ( nx!=Alignment.dimension.x || ny!=Alignment.dimension.y || nz!=Alignment.dimension.z )
	    {
	      printf("Error: the new matrix size (%d,%d,%d) does not match the template (%d,%d,%d)\n",
		     nx,ny,nz,Alignment.dimension.x,Alignment.dimension.y,Alignment.dimension.z);
	      return(1);
	    }
	}
      else if ( !strcmp(parameter,"registration-resolution") )
	{
	  if ( sscanf(value,"%g %g %g",&rx,&ry,&rz) != 3 )
	    {
	      printf("Error reading the value of `set registration-resolution'\n");
	      return(1);
	    }
	  else if ( rx!=Alignment.resolution.x || ry!=Alignment.resolution.y || rz!=Alignment.resolution.z )
	    {
	      printf("WARNING: the new resolution (%g,%g,%g) does not match the template (%g,%g,%g)\n",
		     rx,ry,rz,Alignment.resolution.x,Alignment.resolution.y,Alignment.resolution.z);
	      printf("** I'll use the template resolution instead. **\n");
	    }
	}
      else if ( !strcmp(parameter,"registration-origin") )
	{
	  if ( sscanf(value,"%g %g %g",&rx,&ry,&rz) != 3 )
	    {
	      printf("Error reading the value of `set registration-origin'\n");
	      return(1);
	    }
	  else if ( rx!=Alignment.origin.x || ry!=Alignment.origin.y || rz!=Alignment.origin.z )
	    {
	      printf("WARNING: the new origin (%g,%g,%g) does not match the template (%g,%g,%g)\n",
		     rx,ry,rz,Alignment.origin.x,Alignment.origin.y,Alignment.origin.z);
	      printf("** I'll use the template origin instead. **\n");
	      return(1);
	    }
	}
      else if ( !strcmp(parameter,"registration-direction") )
	{
	  if ( sscanf(value,"%g %g %g",&rx,&ry,&rz) != 3 )
	    {
	      printf("Error reading the value of `set registration-direction'\n");
	      return(1);
	    }
	  else if ( rx!=Alignment.direction.x || ry!=Alignment.direction.y || rz!=Alignment.direction.z )
	    {
	      printf("Error: the new direction (%g,%g,%g) does not match the template (%g,%g,%g)\n",
		     rx,ry,rz,Alignment.direction.x,Alignment.direction.y,Alignment.direction.z);
	      return(1);
	    }
	}
      else if ( !strcmp(parameter,"registration-orientation") )
	{ // Orientation
	  if ( sscanf(value,"%d %d %d %d",
		      &(Alignment.OrientRotation.x),
		      &(Alignment.OrientRotation.y),
		      &(Alignment.OrientRotation.z),
		      &(Alignment.OrientSwapZ) ) != 4 )
	    {
	      printf("Error reading the value of `set registration-orientation'\n");
	      return(1);
	    }
	}
      else if ( !strcmp(parameter,"registration-wrapping") )
	{ // Wrapping
	  if ( sscanf(value,"%d %d %d",
		      &(Alignment.WrapAround.x),
		      &(Alignment.WrapAround.y),
		      &(Alignment.WrapAround.z)) != 3 )
	    {
	      printf("Error reading the value of `set registration-wrapping'\n");
	      return(1);
	    }
	}
      else if ( !strcmp(parameter,"registration-translations") )
	{
	  if ( sscanf(value,"%f %f %f",Alignment.value+0,Alignment.value+1,Alignment.value+2) != 3 )
	    {
	      printf("Error reading the value of `set registration-translations'\n");
	      return(1);
	    }
	}
      else if ( !strcmp(parameter,"registration-rotations") )
	{
	  if ( sscanf(value,"%f %f %f",Alignment.value+3,Alignment.value+4,Alignment.value+5) != 3 )
	    {
	      printf("Error reading the value of `set registration-rotations'\n");
	      return(1);
	    }
	}
      else if ( !strcmp(parameter,"registration-inflations") )
	{
	  if ( sscanf(value,"%f %f %f",Alignment.value+6,Alignment.value+7,Alignment.value+8) != 3 )
	    {
	      printf("Error reading the value of `set registration-inflations'\n");
	      return(1);
	    }
	}
      else if ( !strcmp(parameter,"registration-skews") )
	{
	  if ( sscanf(value,"%f %f %f",Alignment.value+9,Alignment.value+10,Alignment.value+11) != 3 )
	    {
	      printf("Error reading the value of `set registration-inflations'\n");
	      return(1);
	    }
	}
      else if ( !strcmp(parameter,"registration-distortions") )
	{
	  // The file name containing distortion maps is saved in "value"
	  if ( read_volume_header(value,&header) ) return(1);
	  if ( header.dim.x != template.dim.x ||
	       header.dim.y != template.dim.y ||
	       header.dim.z != template.dim.z )
	    {
	      printf("Error: the distortion map dimensions (%d,%d,%d) don't match the template (%d,%d,%d)\n",
		     header.dim.x,header.dim.y,header.dim.z,template.dim.x,template.dim.y,template.dim.z);
	      return(1);
	    }
	  else if ( read_distortion_file(value,header) )
	    return(1);
	  else
	    update_nonlinear_alignment_parameters();
	}
    }
  fclose(file);
  printf("Alignment parameters were recalled from file align.com\n");
  // set the adjustment parameters
  for (j=0; j<NAFFINE; j++)
    if ( Alignment.value[j] != 0. )
      AlignAid.Adjust[j] = 1;

  print_parameter_set(1,0);
  return(0);
}

int write_distortion_file( char *file_name )
{
  FILE *file;
  int format, n3d, error;

  // get the file_format from the file name
  if ( (format=xdisplay_format(file_name)) < 0 )
    {
      printf("Error: that output file format is not supported.\n");
      printf("Use the `bfloat' extension or use `bshort' after `rescale'.\n");
      return(1);
    }

  n3d = distort_x.n3d;
  // Open the file and write the header.
  if ( open_output_file(3,format,file_name,&file) )
    return(1);

  error  = fwrite((char *)distort_x.SpaceTime.fData,sizeof(float),n3d,file) != n3d;
  error &= fwrite((char *)distort_y.SpaceTime.fData,sizeof(float),n3d,file) != n3d;
  error &= fwrite((char *)distort_z.SpaceTime.fData,sizeof(float),n3d,file) != n3d;
  if ( error )
    return(1);
  else
    fclose(file);

  return(0);
}

int open_output_file( int ntime, int format, char *file_name_root, FILE **file )
     // ntime: number of time points in file
     // file_name_root : root name of file
     // file : pointer to file output stream
{
  FILE *file_hdr;
  char file_name[LEN_STR];
  int error;

  if ( format == NIFTI_NII )
    {
      write_nifti_header(ntime,file_name_root,file);
      return(0);
    }

  // Open and write the header file.
  strcpy(file_name,file_name_root);
  strcat(file_name,".hdr");
  if ( !(file_hdr = fopen(file_name,"w")) )
    {
      printf("Error: cannot open the output file %s\n",file_name);
      return(1);
    }
  fprintf(file_hdr,"x %d\ny %d\nz %d\nt %d\nbyte-order %d\n",
	  template.dim.x,template.dim.y,template.dim.z,ntime,machine_byte_order());
  fprintf(file_hdr,"resolution %g %g %g\norigin %g %g %g\ndirection %g %g %g\n",
	  template.resolution.x,template.resolution.y,template.resolution.z,
	  template.origin.x,template.origin.y,template.origin.z,
	  template.direction.x,template.direction.y,template.direction.z);
  fprintf(file_hdr,"information:\n");
  fprintf(file_hdr,"x,y,z distortion maps from jip-align\n");
  fclose(file_hdr);

  // Open the floating point data file.
  strcpy(file_name,file_name_root);
  strcat(file_name,".bfloat");
  error = !(*file = fopen(file_name,"w"));
  if ( error )
    {
      printf("Error: cannot open the output file %s\n",file_name);
      return(1);
    }
  else
    return(0);
}

int write_nifti_header( int ntime, char *file_name_root, FILE **file )
{
  nifti_1_header nifti_header;
  nifti1_extender pad;
  char full_file_name[LEN_STR];
  int error, j;
  mat44 mat;
  float qb, qc, qd, qx, qy, qz, dx, dy, dz, qfac;

  mat = scale_and_shift_to_mat44(template.resolution, template.origin, template.direction);
  nifti_mat44_to_quatern( mat,
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
  nifti_header.dim[1] = template.dim.x;
  nifti_header.dim[2] = template.dim.y;
  nifti_header.dim[3] = template.dim.z;
  nifti_header.dim[4] = ntime;
  nifti_header.dim[5] = 1;
  nifti_header.dim[6] = 1;
  nifti_header.dim[7] = 1;

  nifti_header.datatype = NIFTI_TYPE_FLOAT32;
  nifti_header.bitpix   = 32;

  nifti_header.slice_start = nifti_header.slice_end = 0;
  nifti_header.slice_code  = 0;
  nifti_header.slice_duration = 0.;

  nifti_header.pixdim[0]   = qfac;    // specifies parity of coordinate system
  nifti_header.pixdim[1]   = template.resolution.x;
  nifti_header.pixdim[2]   = template.resolution.y;
  nifti_header.pixdim[3]   = template.resolution.z;
  nifti_header.pixdim[4]   = 0.;
  nifti_header.pixdim[5]   = 0.;
  nifti_header.pixdim[6]   = 0.;
  nifti_header.pixdim[7]   = 0.;

  // temporary; ignore extended header info from original file
  nifti_header.vox_offset  = 352;

  nifti_header.scl_slope   = nifti_header.scl_inter = 0;
  nifti_header.xyzt_units  = NIFTI_UNITS_MM | NIFTI_UNITS_SEC;
  nifti_header.xyzt_units  = NIFTI_UNITS_MM;
  nifti_header.cal_min = nifti_header.cal_max = 0.;
  nifti_header.toffset = 0.;

  // Use the affine transformation (method 3).
  nifti_header.qform_code = 1;
  nifti_header.sform_code = 0;
  // Initialize all elements to 0.
  for (j=0; j<4; j++)
    nifti_header.srow_x[j] = nifti_header.srow_y[j] = nifti_header.srow_z[j] = 0.;
  // Diagonal elements are resolution * direction.
  nifti_header.srow_x[0] = template.resolution.x * template.direction.x;
  nifti_header.srow_y[1] = template.resolution.y * template.direction.y;
  nifti_header.srow_z[2] = template.resolution.z * template.direction.z;
  // The last row is the origin.
  nifti_header.srow_x[3] = template.origin.x;
  nifti_header.srow_y[3] = template.origin.y;
  nifti_header.srow_z[3] = template.origin.z;

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

  // Make the header file name `file_name.hdr' from the root `file_name_root'.
  strcpy(full_file_name, file_name_root);
  strcat(full_file_name, ".nii");
  strcpy(nifti_header.magic,"n+1\0");
  error = !(*file = fopen(full_file_name,"wb"));
  if ( error )
    {
      printf("Error: cannot open the header file %s.\n",full_file_name);
      return(1);
    }

  error = fwrite((char *)(&nifti_header),sizeof(struct nifti_1_header),1,*file) != 1;
  if ( error )
    {
      printf("Error writing the NIFTI header into file %s\n",full_file_name);
      return(1);
    }

  pad.extension[0] = pad.extension[1] = pad.extension[2] = pad.extension[3] = 0;
  error = fwrite(&pad, 4, 1, *file) != 1;
  if ( error )
    {
      fprintf(stderr, "\nError writing header file extension pad %s\n",full_file_name);
      return(1);
    }
  return(0);
}

int read_distortion_file( char *file_name, struct XD_HEADER header )
{
  FILE *file;
  int npts, j;
  int swap_bytes, error;

  /* Open the binary data file. */
  if ( !(file = fopen(file_name,"r")) )
    {
      printf("\nError: cannot open file %s.\n",file_name);
      return(1);
    }
  if ( header.storage_type != BFLOAT )      // bfloat
    {
      printf("\nError: the storage type for distortion file %s should be binary floats\n",file_name);
      return(1);
    }

  // Go to the first data voxel
  error = fseek(file, (long)(header.voxel_offset), SEEK_SET);
  if ( error )
    {
      fprintf(stderr, "\nError doing fseek() to %ld in data file %s\n",
	      (long)(header.voxel_offset), file_name);
      exit(1);
    }

  npts = template.npts_volume;
  // Read the arrays.
  error  = fread((char *)distort_x.SpaceTime.fData,sizeof(float),npts,file) != npts;
  error &= fread((char *)distort_y.SpaceTime.fData,sizeof(float),npts,file) != npts;
  error &= fread((char *)distort_z.SpaceTime.fData,sizeof(float),npts,file) != npts;
  fclose(file);

  if ( error )
    {
      printf("Error reading the distortion map data from file %s\n",file_name);
      return(1);
    }

  // Swap bytes if necessary 
  swap_bytes  = machine_byte_order() != header.byte_order;
  if ( swap_bytes )
    {
      for (j=0; j<template.npts_volume; j++)
	{
	  swap_32( &(distort_x.SpaceTime.fData[j]) );
	  swap_32( &(distort_y.SpaceTime.fData[j]) );
	  swap_32( &(distort_z.SpaceTime.fData[j]) );
	}
    }
  // Set adjustment flags.
  AlignNL.Adjusted.x = AlignNL.Adjusted.y = AlignNL.Adjusted.z = FALSE;
  for (j=0; j<template.npts_volume; j++)
    {
      if ( distort_x.SpaceTime.fData[j] != 0. ) AlignNL.Adjusted.x = TRUE;
      if ( distort_y.SpaceTime.fData[j] != 0. ) AlignNL.Adjusted.y = TRUE;
      if ( distort_z.SpaceTime.fData[j] != 0. ) AlignNL.Adjusted.z = TRUE;
    }
  return(0);
}
