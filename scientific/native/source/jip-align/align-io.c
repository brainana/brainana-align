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
#include <stdint.h>
#include <errno.h>
#include <limits.h>
#include "nifti1_io.h"
#include "util.h"
#include "align.h"

#define DEBUG 0


void print_parameter_set( int mode, int ilevel )
     // mode: 2 = final headers and values
     // mode: 1 = print headers and values
     //       0 = print only values
{
  char prompt[LEN_STR], prompt1[LEN_STR], name[LEN_STR];
  static float parameter_save[NAFFINE];
  int j;

  if ( !DEBUG ) return;

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
  int j1, j2;JIP_INDEX j;

  if ( !(file = fopen("align.com","w")) )
    {
      printf("\nError: cannot open file align.com\n");
      return(1);
    }

  fprintf(file,"# JIP pull map: centered target mm -> centered source mm, A*x + u(x).\n");
  fprintf(file,"# Native voxel affines below are grid metadata, not a second registration transform.\n");
  fprintf(file,"# align.mat contains the inverse affine only; nonlinear fields are stored separately.\n");

  ////////////////////// Target ////////////////////
  // Matrix
  fprintf(file,"set target-matrix %d %d %d\n",
	  template.dim.x,template.dim.y,template.dim.z);
  // Resolution
  fprintf(file,"set target-resolution %.9g %.9g %.9g\n",
	  template.resolution.x,template.resolution.y,template.resolution.z);
  // Origin
  fprintf(file,"set target-origin %.9g %.9g %.9g\n",
	  template.origin.x,template.origin.y,template.origin.z);
  // Direction
  fprintf(file,"set target-direction %g %g %g\n",
	  template.direction.x,template.direction.y,template.direction.z);
  ////////////////////// source ////////////////////
  // Matrix
  fprintf(file,"set source-matrix %d %d %d\n",
	  source.dim.x,source.dim.y,source.dim.z);
  // Resolution
  fprintf(file,"set source-resolution %.9g %.9g %.9g\n",
	  source.resolution.x,source.resolution.y,source.resolution.z);
  // Origin
  fprintf(file,"set source-origin %.9g %.9g %.9g\n",
	  source.origin.x,source.origin.y,source.origin.z);
  // Direction
  fprintf(file,"set source-direction %g %g %g\n",
	  source.direction.x,source.direction.y,source.direction.z);
  /* Preserve full native grid geometry separately from centered registration. */
  fprintf(file,"set target-voxel-affine");
  for(j1=0;j1<4;j1++)for(j2=0;j2<4;j2++)fprintf(file," %.9g",template.ijk_to_xyz.m[j1][j2]);
  fprintf(file,"\nset source-voxel-affine");
  for(j1=0;j1<4;j1++)for(j2=0;j2<4;j2++)fprintf(file," %.9g",source.ijk_to_xyz.m[j1][j2]);
  fprintf(file,"\n");
  // rotation type
  if ( !Alignment.RigidBody )
    fprintf(file,"set registration-type deck-of-cards\n");
  // Orientation
  fprintf(file,"set registration-orientation %d %d %d %d\n",
	  Alignment.OrientRotation.x,Alignment.OrientRotation.y,
	  Alignment.OrientRotation.z,Alignment.OrientSwapZ);
  // Wrapping
  if ( Alignment.WrapAround.x != 0 || Alignment.WrapAround.y != 0 || Alignment.WrapAround.z != 0 )
    fprintf(file,"set registration-wrapping %d %d %d\n",
	    Alignment.WrapAround.x,Alignment.WrapAround.y,Alignment.WrapAround.z);
  // Affine parameters
  fprintf(file,"set registration-translations %.9g %.9g %.9g\n",
	  Alignment.value[0],Alignment.value[1],Alignment.value[2]);
  fprintf(file,"set registration-rotations %.9g %.9g %.9g\n",
	  Alignment.value[3],Alignment.value[4],Alignment.value[5]);
  fprintf(file,"set registration-inflations %.9g %.9g %.9g\n",
	  Alignment.value[6],Alignment.value[7],Alignment.value[8]);
  fprintf(file,"set registration-skews %.9g %.9g %.9g\n",
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

  // Write the affine matrix.
  if ( !(file = fopen("align.mat","w")) )
    {
      printf("\nError: cannot open file align.mat\n");
      return(1);
    }
  for (j1=0; j1<4; j1++)
    {
      for (j2=0; j2<4; j2++)
	fprintf(file," %.9g",Alignment.AffineI[j1][j2]);
      fprintf(file,"\n");
    }
  fclose(file);

  printf("Alignment parameters were saved in file align.com and align.mat\n");
  printf("Use align.com with `jip' to register files.\n");
  printf("align.mat is the inverse centered-axis affine only; nonlinear fields are stored separately.\n");
  return(0);
}

int recall_alignment_parameters()
{
  struct XD_HEADER header;
  FILE *file;
  char whole_line[LEN_STR], set[LEN_STR], parameter[LEN_STR], *value;
  float rx, ry, rz;
  int nx, ny, nz;

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
      value = next_non_white(whole_line + jip_int_count((JIP_INDEX)strlen(set)));
      sscanf(value,"%s",parameter);
      value = next_non_white(value + jip_int_count((JIP_INDEX)strlen(parameter)));
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
      else if ( !strcmp(parameter,"registration-matrix") ||
		!strcmp(parameter,"target-matrix") )
	{
	  if ( sscanf(value,"%d %d %d",&nx,&ny,&nz) != 3 )
	    {
	      printf("Error reading the value of `set registration-orientation'\n");
	      return(1);
	    }
	  else if ( nx!=template.dim.x || ny!=template.dim.y || nz!=template.dim.z )
	    {
	      printf("Error: the new matrix size (%d,%d,%d) does not match the template (%d,%d,%d)\n",
		     nx,ny,nz,template.dim.x,template.dim.y,template.dim.z);
	      printf("** I'll use the template matrix instead.\n");
	    }
	}
      else if ( !strcmp(parameter,"registration-resolution") ||
		!strcmp(parameter,"target-resolution") )
	{
	  if ( sscanf(value,"%g %g %g",&rx,&ry,&rz) != 3 )
	    {
	      printf("Error reading the value of `set registration-resolution'\n");
	      return(1);
	    }
	  else if ( rx!=template.resolution.x || ry!=template.resolution.y || rz!=template.resolution.z )
	    {
	      printf("WARNING: the new resolution (%g,%g,%g) does not match the template (%g,%g,%g)\n",
		     rx,ry,rz,template.resolution.x,template.resolution.y,template.resolution.z);
	      printf("** I'll use the template resolution instead. **\n");
	    }
	}
      else if ( !strcmp(parameter,"registration-origin") ||
		!strcmp(parameter,"target-origin") )
	{
	  if ( sscanf(value,"%g %g %g",&rx,&ry,&rz) != 3 )
	    {
	      printf("Error reading the value of `set registration-origin'\n");
	      return(1);
	    }
	  else if ( rx!=template.origin.x || ry!=template.origin.y || rz!=template.origin.z )
	    {
	      printf("WARNING: the new origin (%g,%g,%g) does not match the template (%g,%g,%g)\n",
		     rx,ry,rz,template.origin.x,template.origin.y,template.origin.z);
	      printf("** I'll use the template origin instead. **\n");
	    }
	}
      else if ( !strcmp(parameter,"registration-direction") ||
		!strcmp(parameter,"target-direction") )
	{
	  if ( sscanf(value,"%g %g %g",&rx,&ry,&rz) != 3 )
	    {
	      printf("Error reading the value of `set registration-direction'\n");
	      return(1);
	    }
	  else if ( rx!=template.direction.x || ry!=template.direction.y || rz!=template.direction.z )
	    {
	      printf("Error: the new direction (%g,%g,%g) does not match the template (%g,%g,%g)\n",
		     rx,ry,rz,template.direction.x,template.direction.y,template.direction.z);
	      printf("** I'll use the template directions instead.\n");
	    }
	}
      else if (!strcmp(parameter,"target-voxel-affine")||!strcmp(parameter,"source-voxel-affine")) {
        float a[16];int r, c;mat44 expected=!strcmp(parameter,"target-voxel-affine")?template.ijk_to_xyz:source.ijk_to_xyz;
        if(sscanf(value,"%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f",a,a+1,a+2,a+3,a+4,a+5,a+6,a+7,a+8,a+9,a+10,a+11,a+12,a+13,a+14,a+15)!=16){fclose(file);return 1;}
        for(r=0;r<4;r++)for(c=0;c<4;c++)if(!isfinite(a[4*r+c])||fabs(a[4*r+c]-expected.m[r][c])>1.e-5*MAX(1.,fabs(expected.m[r][c]))) {
          fprintf(stderr,"Saved native geometry does not match loaded image.\n");fclose(file);return 1;
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
	  if ( read_volume_header(Verbose,value,&header) ) return(1);
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
	}
    }
  fclose(file);
  define_affine_matrix();
  /* Basis construction clears the Jacobian display. Refresh it from the
     restored dense map, using the restored affine rather than stale state. */
  number_invalid_distortions(1,-1,0);
  printf("Alignment parameters were recalled from file align.com\n");
  print_parameter_set(1,0);

  return(0);
}

extern int write_nifti_header_stream(int ntime,FILE *file);

int write_distortion_file(char *file_name)
{
  FILE *file=NULL;int format, error=1, fd=-1, c;size_t n,i;
  char temporary[LEN_STR],destination[LEN_STR],root[LEN_STR];
  float *fields[3];temporary[0]=0;
  if(jip_int_count((JIP_INDEX)strlen(file_name))>=sizeof(root))return 1;
  strcpy(root,file_name);format=xdisplay_format(root);
  if(format!=NIFTI_NII&&format!=BFLOAT){fprintf(stderr,"Displacement output must be .nii or .bfloat\n");return 1;}
  n=(size_t)distort_x.n3d;
  if(!n||n!=(size_t)template.npts_volume||distort_y.n3d!=distort_x.n3d||distort_z.n3d!=distort_x.n3d)return 1;
  fields[0]=distort_x.SpaceTime.fData;fields[1]=distort_y.SpaceTime.fData;fields[2]=distort_z.SpaceTime.fData;
  for(c=0;c<3;c++)for(i=0;i<n;i++)if(!isfinite(fields[c][i]))return 1;
  if(format==BFLOAT) {
    /* Keep the legacy two-file format; propagate EVERY failure and close it. */
    if(open_output_file(3,format,root,&file))return 1;
  } else {
    if(snprintf(destination,sizeof(destination),"%s.nii",root)>=(int)sizeof(destination)||
       snprintf(temporary,sizeof(temporary),"%s.tmp-XXXXXX.nii",root)>=(int)sizeof(temporary))return 1;
    fd=mkstemps(temporary,4);if(fd<0)return 1;
    file=fdopen(fd,"wb");if(!file){close(fd);unlink(temporary);return 1;}
    if(write_nifti_header_stream(3,file))goto done;
  }
  error=0;
  for(c=0;c<3;c++)if(fwrite(fields[c],sizeof(float),n,file)!=n){error=1;break;}
  if(fflush(file)!=0||ferror(file))error=1;
  if(fclose(file)!=0)error=1;file=NULL;
  if(format==NIFTI_NII) {
    if(!error&&rename(temporary,destination)!=0)error=1;
    if(error)unlink(temporary);
  }
  if(!error)strcpy(file_name,root); /* preserve the historical stem-only result */
done:
  if(file)fclose(file);
  if(error&&temporary[0])unlink(temporary);
  if(error)fprintf(stderr,"Failed to save complete displacement field: %s\n",file_name);
  return error;
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
      return write_nifti_header(ntime,file_name_root,file);
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
  error=ferror(file_hdr);
  if(fclose(file_hdr)!=0)error=1;
  if(error)return 1;

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

int write_nifti_header_stream(int ntime,FILE *file)
{
  nifti_1_header nifti_header;
  nifti1_extender pad;
  int error, j;
  mat44 mat;
  float qb, qc, qd, qx, qy, qz, dx, dy, dz, qfac;

  memset(&nifti_header,0,sizeof(nifti_header));
  mat = template.ijk_to_xyz;
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
  nifti_header.qform_code = 0;
  nifti_header.sform_code = 1;
  for(j=0;j<4;j++) {
    nifti_header.srow_x[j]=mat.m[0][j];
    nifti_header.srow_y[j]=mat.m[1][j];
    nifti_header.srow_z[j]=mat.m[2][j];
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

  strcpy(nifti_header.descrip,"JIP pull u: centered target to centered source axes, mm");
  strcpy(nifti_header.aux_file,"");

  strcpy(nifti_header.magic,"n+1");
  memset(&pad,0,sizeof(pad));
  if(fwrite(&nifti_header,sizeof(nifti_header),1,file)!=1)return 1;
  return fwrite(&pad,4,1,file)!=1;
}

int write_nifti_header(int ntime,char *root,FILE **file)
{
  char name[LEN_STR];*file=NULL;
  if(snprintf(name,sizeof(name),"%s.nii",root)>=(int)sizeof(name))return 1;
  *file=fopen(name,"wb");if(!*file)return 1;
  if(write_nifti_header_stream(ntime,*file)){fclose(*file);*file=NULL;return 1;}
  return 0;
}

int read_distortion_file(char *file_name,struct XD_HEADER header)
{
  FILE *file=NULL;float *buffer=NULL;size_t n,count,i;int c, r, error=1;
  double expected,actual,tolerance;
  if(header.storage_type!=BFLOAT||header.nt!=3||header.dim.x!=template.dim.x||
     header.dim.y!=template.dim.y||header.dim.z!=template.dim.z||
     !isfinite(header.voxel_offset)||header.voxel_offset<0||(double)header.voxel_offset>=(double)LONG_MAX||floor(header.voxel_offset)!=header.voxel_offset||
     (header.scale_slope!=0&&header.scale_slope!=1)||header.scale_offset!=0)goto done;
  /* Dimensions alone cannot establish that a displacement grid matches. */
  for(r=0;r<3;r++)for(c=0;c<4;c++) {
    expected=template.ijk_to_xyz.m[r][c];actual=header.ijk_to_xyz.m[r][c];
    tolerance=1.e-5*MAX(1.,fabs(expected));
    if(!isfinite(actual)||fabs(expected-actual)>tolerance)goto done;
  }
  n=(size_t)template.npts_volume;
  if(!n||n>SIZE_MAX/(3*sizeof(float)))goto done;
  count=3*n;buffer=(float*)malloc((size_t)jip_product(count,sizeof(float)));if(!buffer)goto done;
  file=fopen(file_name,"rb");if(!file)goto done;
  if(jip_seek(file,(JIP_INDEX)header.voxel_offset,SEEK_SET)!=0)goto done;
  if(fread(buffer,sizeof(float),count,file)!=count||ferror(file))goto done;
  if(fclose(file)!=0){file=NULL;goto done;}file=NULL;
  if(machine_byte_order()!=header.byte_order)for(i=0;i<count;i++)swap_32(buffer+i);
  for(i=0;i<count;i++)if(!isfinite(buffer[i]))goto done;
  /* Commit only after all components, geometry and finite values validate. */
  memcpy(distort_x.SpaceTime.fData,buffer,n*sizeof(float));
  memcpy(distort_y.SpaceTime.fData,buffer+n,n*sizeof(float));
  memcpy(distort_z.SpaceTime.fData,buffer+2*n,n*sizeof(float));
  /* Building editable basis coefficients reconstructs a field at the current
     (often coarser) basis grid. Merely opening/saving an imported dense map
     must not replace it with that approximation. Reuse the validated read
     buffer to restore the exact map after basis initialization; explicit
     basis-grid changes or a new optimization retain their original behavior. */
  update_nonlinear_alignment_parameters();
  memcpy(distort_x.SpaceTime.fData,buffer,n*sizeof(float));
  memcpy(distort_y.SpaceTime.fData,buffer+n,n*sizeof(float));
  memcpy(distort_z.SpaceTime.fData,buffer+2*n,n*sizeof(float));
  AlignNL.Adjusted.x=AlignNL.Adjusted.y=AlignNL.Adjusted.z=FALSE;
  for(i=0;i<n;i++) {
    if(buffer[i]!=0)AlignNL.Adjusted.x=TRUE;
    if(buffer[n+i]!=0)AlignNL.Adjusted.y=TRUE;
    if(buffer[2*n+i]!=0)AlignNL.Adjusted.z=TRUE;
  }
  error=0;
done:
  if(file)fclose(file);free(buffer);
  if(error)fprintf(stderr,"Cannot restore complete finite displacement field on the current template grid: %s\n",file_name);
  return error;
}

