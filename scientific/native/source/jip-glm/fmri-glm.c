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
#include "nifti1.h"
#include "nifti1_io.h"
#include "util.h"
#include "glm-prototypes.h"

// Read this many voxels per time point for analysis.
// For smaller jobs, set this number to be large to avoid file i/o.
// For larger jobs,  set this number to be small to avoid memory overflow.
#define BLOCK_SIZE_BIG   50000
#define BLOCK_SIZE_SMALL 5000
int BLOCK_SIZE; // This value is used throughout the program (selected from above).

// Input parameters from the unix command line:
float S_divisor;
// resolution elements
JIP_INDEX resels;

// Output files.
#define NCONDITIONS_MAX 20
#define NEVENTS_MAX 9

float **av_sigma_map;    // [2][nvoxels]            2 maps: average, and sigma2
float **P_map, **T_map;  // [nconditions][nvoxels]  1 map per condition
float **S_map, **dS_map; // [nconditions][nvoxels]  1 map per condition
short *nAR_map;

struct input_fmri_files
{
  char fMRI_file_name[LEN_STR];     // fMRI data file name for a run
  char glm_file_name[LEN_STR];      // one glm file for each run
  FILE *fMRI_file;                  // fMRI data file input stream for the run
  IPOINT3D dim;                     // x,y,z dimensions
  int nt;                           // time dimension of fMRI run
  int bo;                           // byte order of fMRI run
  int type;                         // bshort, blong, bfloat, img, nii
  int storage;                      // bshort, blong, bfloat
  JIP_INDEX voxel_offset;                 // offset to 1st voxel in file
  float scale;                      // average value of data set for normalization
  POINT3D resolution;
  POINT3D origin;
  POINT3D direction;
  float   resolution_t;
  struct input_fmri_files *next;
};

struct input_fmri_files file_list_root;
float **auto_cor;

// Prototypes
int initialize_runs( int *ntime_points );
int analyze_AR_noise();
int read_volume(struct input_fmri_files *file_list, int ivolume, float *image);
int analyze_runs(int nconditions, int units_percent);
int read_block_vector(float *block_vector, JIP_INDEX istart, int *block_size);
int write_output_files( int nconditions);
int open_output_file(   BOOL SFile, int ntime, char *file_name_root, FILE **file );
int write_nifti_header( BOOL SFile, int ntime, char *file_name_root, FILE **file );


int main( argc, argv)
int argc;
char *argv[];
{
  FILE *file;
  char glm_control_file_name[LEN_STR];
  char *optarg;
  int option, which_arg, ntime_points;
  struct input_fmri_files *file_list, *last_file_list;
  int j, nconditions;
  BOOL units_percent=0, input_ok=FALSE;
  BOOL little_help=FALSE;

  // Default values.
  Verbose    = FALSE;
  S_divisor  = 1.;
  BLOCK_SIZE = BLOCK_SIZE_BIG;

  // Get program options on entry.
  while ( (option=my_getopt(argc,argv,"hHpsvVd:i:",&which_arg)) )
    {
      optarg = argv[which_arg];
      switch ((char)option)
	{
	case 'h':
	  little_help = TRUE;
	  break;
	case 'H':
	  help_menu(0,"glm-menu.txt");
	  exit(0);
	  break;
	case 'i':
	  // This is required: the glm control file.
	  strcpy(glm_control_file_name,optarg);
	  input_ok = TRUE;
	  break;
	case 'd':
	  if ( sscanf(optarg,"%f",&S_divisor) != 1 )
	    {
	      printf("Error reading the output scale factor.\n");
	      exit(1);
	    }
	  break;
	case 's': // small blocks size (reduce memory)
	  // Set the block size (voxels per time point) to a small value.
	  BLOCK_SIZE = BLOCK_SIZE_SMALL;
	  break;
	case 'v': // verbose: more output
	  Verbose = TRUE;
	  break;
	case 'p':
	  units_percent = TRUE;
	  break;
	default:
	  if ( option == -1 )
	    printf("Option -%s not valid.\n",optarg);
	  else
	    printf("Valid option -%c not yet programmed.\n",*optarg);
	  printf("Program exiting.\n");
	  exit(1);
	}
    }

  if ( !input_ok || little_help )
    {
      printf("Syntax: executable [GLM control file name] [optional arguments]\n");
      printf("optional arguments:\n");
      printf("-h for this help\n");
      printf("-H for more help using a menu\n");
      printf("-p to convert to percent change on output for S-n files\n");
      printf("-s for small memory allocation\n");
      printf("-v for verbose output\n");
      printf("-d [divisor]  to divide output by divisor for S-n files\n");
      printf("\n");
      if ( little_help )
	exit(0);
      else
	exit(1);
    }
  else
    printf("\n");

  // Read the analysis file
  if ( GLM_read_control_file(glm_control_file_name,Verbose) )
    {
      printf("Program terminated due to error in analysis file %s\n",optarg);
      exit(1);
    }

  // Put the image files into the list.
  for ( j=0, file_list=&file_list_root; j<GLM_input.nruns; j++)
    {
      strcpy(file_list->fMRI_file_name,GLM_input.run_name[j]);
      strcpy(file_list->glm_file_name,GLM_input.glm_name[j]);
      if ( file_list != &file_list_root )last_file_list->next = file_list;
      last_file_list = file_list;
      file_list = (struct input_fmri_files *)malloc(sizeof(struct input_fmri_files));
      file_list->next = NULL;
    }

  // Initialize each run
  if ( initialize_runs(&ntime_points) ) exit(1);

  // Don't let memory go over 256 megabytes.
  if ( (float)(ntime_points*BLOCK_SIZE*2)/1.e6 > 256. )
    {
      BLOCK_SIZE = (int)(256*1.e6/(float)ntime_points/2.);
      printf("Memory allocation reduced: BLOCK_SIZE = %d\n",BLOCK_SIZE);
    }

  // Define the basis functions.
  if ( GLM_define_basis_functions() ) exit(1);

  // Describe the basis functions.
  if ( Verbose || GLM_input.notify )
    GLM_describe_basis_functions();

  // Define the comparison condition.
  if ( GLM_define_conditions(1,GLM_input.condition_string,&nconditions) ) exit(1);
  if ( nconditions == 0 )
    {
      printf("Error: no test conditions are defined.\n");
      exit(1);
    }

  // Output the conditions to be tested.
  if ( Verbose )
    GLM_describe_conditions();

  // Describe the files.
  GLM_describe_input_files();

  // Compute AR coefficients at each voxel if necessary.
  if ( analyze_AR_noise() ) exit(1);

  // Begin reading each file and fitting each voxel.
  if ( analyze_runs(nconditions,units_percent) ) exit(1);

  //
  if ( write_output_files(nconditions) )
    {
      printf("Error writing output files.\n");
      exit(1);
    }

  // Create an initialization file for xdisplay.
  if ( !(file = fopen(".display","r")) )
    { // Then file does not exist, so proceed.
      if ( !(file = fopen(".display","w")) )
	exit(1);
      else
	{
	  fprintf(file,"jip-display -g %s\n",glm_control_file_name);
	  fclose(file);
	}
    }
  exit(0);
}

int initialize_runs( int *ntime_points )
{
  struct XD_HEADER header;
  POINT3D resolution;
  IPOINT3D dim;
  int nruns, jt;JIP_INDEX n3d, jp;
  float *volume_data, volume_average, file_average, global_average, tr;
  struct input_fmri_files *file_list;

  // Read the dimensions of each fMRI run
  for (file_list=&file_list_root; file_list; file_list = file_list->next)
    {
      // Read the file type and dimensions
      if ( read_volume_header(Verbose, file_list->fMRI_file_name, &header) )
	return(1);
      file_list->dim  = header.dim;
      file_list->nt   = header.nt;
      file_list->type = header.file_type;
      file_list->storage      = header.storage_type;
      file_list->voxel_offset = header.voxel_offset;
      file_list->bo           = header.byte_order;
      file_list->resolution   = header.resolution;
      file_list->origin       = header.origin;
      file_list->direction    = header.direction;
      file_list->resolution_t = header.resolution_t;
    }
  // Make sure the spatial dimensions (x,y,z) are the same for all fMRI runs (time points can vary).
  dim = file_list_root.dim;
  n3d = jip_product(jip_product(dim.x,dim.y),dim.z);
  resolution = file_list_root.resolution;
  *ntime_points = 0;
  for (file_list=&file_list_root, nruns=0; file_list; file_list = file_list->next, nruns++)
    {
      if ( file_list->dim.x != dim.x || file_list->dim.y != dim.y || file_list->dim.z != dim.z )
	{
	  printf("The dimensions of file %s (%d,%d,%d) are not the same as for file %s (%d,%d,%d)\n",
		 file_list->fMRI_file_name,
		 file_list->dim.x,file_list->dim.y,file_list->dim.z,
		 file_list_root.fMRI_file_name,
		 file_list_root.dim.x,file_list_root.dim.y,file_list_root.dim.z);
	  return(1);
	}
      if ( GLM_get_tr( nruns, &tr ) )
	{
	  printf("Programming error in call to GLM_get_tr using nruns = %d\n",nruns);
	  exit(1);
	}
      if ( file_list->resolution_t != tr )
	printf("Warning: file %s has a time step (%g) that differs from the GLM (%g)\n",
	       file_list->fMRI_file_name,file_list->resolution_t,tr);
      // One could test for consistent resolution, origin, direction, etc.,
      // but this voxel-wise analysis doesn't care about coordinates, so ignore these things.
      *ntime_points += file_list->nt;
    }

  printf("fMRI files: %d runs of dimension (x,y,z) = (%d,%d,%d) containing a total of %d time points.\n",
	 nruns,dim.x,dim.y,dim.z,*ntime_points);

  // Define the number of resolution elements.
  if ( GLM_input.multiple_comparisons > 0 )
    resels = GLM_input.multiple_comparisons;
  else
    resels = 0.;

  if ( GLM_input.multiple_comparisons != 0 )
    printf("Using %td resolution elements for multiple comparisons.\n",resels);
  else
    printf("Calculating raw pvalues without correction for multiple comparisons.\n");

  // Open all the fMRI data files.
  for (file_list=&file_list_root; file_list; file_list = file_list->next)
    {
      if ( !(file_list->fMRI_file = fopen(file_list->fMRI_file_name,"r")) )
	{
	  printf("Error: cannot open file %s.\n",file_list->fMRI_file_name);
	  return(1);
	}
    }

  // Find the global signal average for each file.
  if ( nruns > 1 && GLM_input.normalize_runs>0 )
    {
      if ( GLM_input.normalize_runs == 1 )
	printf("Finding BOLD normalization factors assuming independent scaling factors per run:\n");
      if ( GLM_input.normalize_runs == 2 )
	printf("Finding IRON normalization factors assuming progressive blood decay of irox oxide:\n");
      for (file_list=&file_list_root; file_list; file_list = file_list->next)
	{
	  // Allocate an volume vector
	  volume_data = float_vector(n3d);
	  file_average = 0.;
	  for (jt=0; jt<file_list->nt; jt++)
	    {
	      read_volume(file_list, jt, volume_data);
	      // For each run, find the global signal average.
	      volume_average = 0;
	      for (jp=0; jp<n3d; jp++)
		volume_average += volume_data[jp] / (float)(n3d);
	      file_average += volume_average / (float)(file_list->nt);
	    }
	  file_list->scale = file_average;
	  free_float_vector(volume_data);
	}
      
      // Re-scale the normalization factors to have an average value of 1.
      global_average = 0;
      for (file_list=&file_list_root; file_list; file_list = file_list->next)
	global_average += file_list->scale / (float)nruns;
      for (file_list=&file_list_root, nruns=0; file_list; file_list = file_list->next, nruns++)
	{
	  if ( GLM_input.normalize_runs == 1 )      // BOLD: make small baseline signal larger
	    file_list->scale = global_average / file_list->scale;
	  else if ( GLM_input.normalize_runs == 2 ) // IRON: make small baseline signal smaller
	    file_list->scale = file_list->scale / global_average; // this is TEMPORARY; not really right
	  printf("Run #%d multiplier = %f\n",nruns+1,file_list->scale);
	}
    }
  else
    {
      for (file_list=&file_list_root, nruns=0; file_list; file_list = file_list->next, nruns++)
	file_list->scale = 1.;
    }
  return(0);
}

int read_volume(struct input_fmri_files *file_list,  // the file list structure
		int ivolume,                         // the volume to read
		float *volume_data)                  // the volume vector to fill
{
  JIP_INDEX offset, j, npts;
  int swap_bytes;
  short *idata;
  int32_t *iidata;
  float *data;

  // The # of points in volume data.
  npts = jip_product(jip_product(file_list->dim.x,file_list->dim.y),file_list->dim.z);
  swap_bytes = file_list->bo != machine_byte_order();

  // Position the file at the starting location for the specified volume.
  offset = file_list->voxel_offset;
  if ( file_list->storage == BSHORT ) // bshort
    offset += ivolume * npts * 2;
  else                        // bfloat or blong
    offset += ivolume * npts * 4;
  jip_seek(file_list->fMRI_file, offset, 0);

  // Read the data vector.
  if ( file_list->storage == BSHORT ) // .bshort
    {
      idata = short_vector(npts);
      // Read the array.
      if ( fread((char *)idata,sizeof(short),npts,file_list->fMRI_file) != npts )
	{
	  printf("Error reading the short integer data from file %s\n",file_list->fMRI_file_name);
	  return(1);
	}
      // Copy the integer array into the stack floating point array.
      for (j=0; j<npts; j++)
	{
	  if ( swap_bytes ) swap_16( &(idata[j]) );
	  volume_data[j] = idata[j];
	}
      free_short_vector(idata);
    }
  else if ( file_list->storage == BLONG ) // .blong
    {
      iidata = int_vector(npts);
      // Read the array.
      if ( fread((char *)iidata,sizeof(int32_t),npts,file_list->fMRI_file) != npts )
	{
	  printf("Error reading the long integer data from file %s\n",file_list->fMRI_file_name);
	  return(1);
	}
      // Copy the integer array into the stack floating point array.
      for (j=0; j<npts; j++)
	{
	  if ( swap_bytes ) swap_32( &(iidata[j]) );
	  volume_data[j] = iidata[j];
	}
      free_int_vector(iidata);
    }
  else // .bfloat
    {
      data = float_vector(npts);
      // Read the array.
      if ( fread((char *)data,sizeof(float),npts,file_list->fMRI_file) != npts )
	{
	  printf("Error reading the floating point data from file %s\n",file_list->fMRI_file_name);
	  return(1);
	}
      // Copy the integer array into the stack floating point array.
      for (j=0; j<npts; j++)
	{
	  if ( swap_bytes ) swap_32( &(data[j]) );
	  volume_data[j] = data[j];
	}
      free_float_vector(data);
    }
  return(0);
}

int analyze_AR_noise()
{
  FILE *file;
  struct input_fmri_files *file_list;
  IPOINT3D dim;
  float *block_vector, *data_vector;
  float **partial_auto_cor, **parc; // partial auto-correlations
  float topval, botval;
  int ntime, error, block_size, idiv;JIP_INDEX n3d, ivoxel, increment;
  int jblock, jtime, jrho, jrho1, jsum;JIP_INDEX jpoint;

  // x and y dimensions
  dim = file_list_root.dim;
  n3d = jip_product(jip_product(dim.x,dim.y),dim.z);
  auto_cor = float_matrix(n3d,GLM_input.AR_order+1);
  // If no AR order is specified, simply return.
  if ( GLM_input.AR_order < 1 )
    return(0);

  // Find the number of runs and total number of time points.
  for (file_list=&file_list_root, ntime=0; file_list; file_list = file_list->next)
    ntime += file_list->nt;

  // Allocate a time vector for fitting each voxel.
  block_vector = float_vector(BLOCK_SIZE * ntime);
  data_vector  = float_vector(ntime);

  idiv = 100;
  increment = n3d / idiv;

  // Loop over each voxel.
  // Data arrays are read in pixel blocks, because if one attempts to read only 1 pixel time
  // course at a time, the file i/o takes a LONG time as one jumps around in the data files
  // (skipping over 1 image of words for each time point).
  for (jpoint=0; jpoint<n3d; jpoint+=block_size)
    {
      // Read the data vector, with the run-specific scaling factors attached.
      if ( read_block_vector(block_vector, jpoint, &block_size) ) exit(1);
      // Loop over pixels in the block.
      for (jblock=0; jblock<block_size; jblock++)
	{
	  // Fill the data vector from contents of the block vector.
	  for (jtime=0; jtime<ntime; jtime++)
	    data_vector[jtime] = block_vector[jblock+jtime*block_size];
	  ivoxel = jpoint + jblock;
	  if ( GLM_calculate_AR_coefficient(ntime,data_vector,GLM_input.AR_order,auto_cor[ivoxel]) )
	    return(1);
	  // Keep track of the percentage of the data that has been processed.
	  if ( ivoxel % increment == 0)
	    {
	      if ( GLM_input.AR_order == 1 )
		printf("\rComputing %d AR map, %td percent done",
		       GLM_input.AR_order,(100/idiv * ivoxel/increment));
	      else
		printf("\rComputing %d AR maps, %td percent done",
		       GLM_input.AR_order,(100/idiv * ivoxel/increment));
	      fflush(stdout);
	    }
	}
    }
  printf("\n\n");

  // Smooth the AR maps.
  if ( smooth_AR_maps() ) return(1);

  // Open a file that will contain the set of AR maps
  if ( open_output_file(0,GLM_input.AR_order,"AR",&file) ) return(1);
  // Write out the auto-correlation values.
  for ( jrho=1; jrho<=GLM_input.AR_order; jrho++)
    {
      for (jpoint=0; jpoint<n3d; jpoint++)
	{
	  // Write the rho value
	  error = fwrite((char *)&auto_cor[jpoint][jrho],sizeof(float),1,file) != 1;
	  if ( error )
	    {
	      printf("Error writing file `AR'\n");
	      return(1);
	    }
	}
    }
  fclose(file);

  // Calculate an array of partial auto-correlations for each voxel.
  parc = float_matrix(GLM_input.AR_order+1,GLM_input.AR_order+1);
  partial_auto_cor = float_matrix(n3d,GLM_input.AR_order+1);
  parc[0][0] = 1.;
  for (jpoint=0; jpoint<n3d; jpoint++)
    {
      partial_auto_cor[jpoint][0] = 1.;
      partial_auto_cor[jpoint][1] = parc[1][1] = auto_cor[jpoint][1];
      //      printf("\n");
      for (jrho=2; jrho<=GLM_input.AR_order; jrho++)
	{
	  topval = botval = 0.;
	  for (jsum=1; jsum<jrho; jsum++)
	    {
	      //	      printf("compute top & bot val=s from pc[%d][%d], ac[%d], ac[%d]\n",
	      //		     jrho-1,jsum,jrho-jsum,jsum);
	      topval += parc[jrho-1][jsum] * auto_cor[jpoint][jrho-jsum];
	      botval += parc[jrho-1][jsum] * auto_cor[jpoint][jsum];
	    }
	  // diagonal elements only use off-diagnonals with lower values for both indices
	  parc[jrho][jrho] = ( auto_cor[jpoint][jrho] - topval ) / ( 1. - botval );
	  // calculate off-diagonals up to jrho; do this AFTER statement above
	  for (jrho1=1; jrho1<jrho; jrho1++)
	    {
	      parc[jrho][jrho1] = parc[jrho-1][jrho1] - parc[jrho][jrho]*parc[jrho-1][jrho-jrho1];
	    }
	  partial_auto_cor[jpoint][jrho] = parc[jrho][jrho];
	}
    }

  /*
  // Open a file that will contain the set of partial AR maps
  if ( open_output_file(0,GLM_input.AR_order,"PAR",&file) ) return(1);
  // Write out the partial auto-correlation values.
  for ( jrho=1; jrho<=GLM_input.AR_order; jrho++)
  {
  for (jpoint=0; jpoint<n3d; jpoint++)
  {
  // Write the rho value
  error = fwrite((char *)&partial_auto_cor[jpoint][jrho],sizeof(float),1,file) != 1;
  if ( error )
  {
  printf("Error writing file `PAR'\n");
  return(1);
  }
  }
  }
  fclose(file);
  */

  // Free working arrays.
  free_float_vector(data_vector);
  free_float_vector(block_vector);
  free_float_matrix(parc,GLM_input.AR_order+1,GLM_input.AR_order+1);
  free_float_matrix(partial_auto_cor,n3d,GLM_input.AR_order+1);

  return(0);
}

int analyze_runs(int nconditions, int units_percent)
{
  struct input_fmri_files *file_list;
  char condition_string[LEN_STR];
  IPOINT3D dim;
  float *block_vector, *data_vector, *fit_vector, *ac_vector;
  float *global_residue_vector, *weight_vector;
  float average, sigma2, cnr, p_raw, p_cor, logsig, effect_size, effect_sdev, arg, weight_sum;
  int ntime, block_size, idiv;JIP_INDEX n3d, increment;
  int jblock, jtime, jrho, jcondition;JIP_INDEX jpoint, jvoxel;

  printf("Analyzing fMRI runs using GLM ...\n\n");

  // x and y dimensions
  dim = file_list_root.dim;
  n3d = jip_product(jip_product(dim.x,dim.y),dim.z);

  // With conditions defined, allocate space for maps.
  av_sigma_map = float_matrix(3,n3d);
  P_map        = float_matrix(nconditions,n3d);
  T_map        = float_matrix(nconditions,n3d);
  S_map        = float_matrix(nconditions,n3d);
  dS_map       = float_matrix(nconditions,n3d);
  nAR_map      = short_vector(n3d);

  // Find the number of runs and total number of time points.
  for (file_list=&file_list_root, ntime=0; file_list; file_list = file_list->next)
    ntime += file_list->nt;

  // Allocate a time vector for fitting each voxel.
  block_vector   = float_vector(BLOCK_SIZE * ntime);
  data_vector    = float_vector(ntime);
  fit_vector     = float_vector(ntime);
  ac_vector      = float_vector(GLM_input.AR_order+1);

  idiv = 100;
  increment = n3d / idiv;

  // Initialize the residue vector to 0.
  global_residue_vector = float_vector(ntime);
  weight_vector = float_vector(ntime);
  for (jtime=0; jtime<ntime; jtime++)
    global_residue_vector[jtime] = 0.;

  // Loop over each voxel.
  // Data arrays are read in pixel blocks, because if one attempts to read only 1 pixel time
  // course at a time, the file i/o takes a LONG time as one jumps around in the data files
  // (skipping over 1 image of words for each time point).
  for (jpoint=0; jpoint<n3d; jpoint+=block_size)
    {
      // Read the data vector, with the run-specific scaling factors attached.
      if ( read_block_vector(block_vector, jpoint, &block_size) ) exit(1);
      // Loop over pixels in the block.
      for (jblock=0; jblock<block_size; jblock++)
	{
	  jvoxel = jpoint + jblock;
	  // Fill the data vector from contents of the block vector.
	  for (jtime=0; jtime<ntime; jtime++)
	    data_vector[jtime] = block_vector[jblock+jtime*block_size];
	  for (jrho=0; jrho<=GLM_input.AR_order; jrho++)
	    ac_vector[jrho] = auto_cor[jvoxel][jrho];
	  // Calculate the whitening filter from previously determined auto-correlations
	  if ( GLM_calculate_whitening_filter(GLM_input.AR_order+1, ac_vector, nAR_map+jvoxel) )
	    return(1);

	  // Fit the time vector.
	  GLM_fit_data( ntime, data_vector, fit_vector, &sigma2 );
	  av_sigma_map[1][jvoxel] = sqrt((double)sigma2);
	  // Calculate the cnr & p_raw for each condition.
	  for (jcondition=0; jcondition<nconditions; jcondition++)
	    {
	      if ( GLM_calculate_F(ntime, sigma2, resels,
				   jcondition, &cnr, &p_raw, &p_cor, &effect_size, &effect_sdev) )
		return(1);

	      if ( GLM_input.multiple_comparisons == 0 )
		logsig = -log10((double)p_raw);
	      else
		logsig = -log10((double)p_cor);
	      if ( cnr < 0. ) logsig *= -1;

	      P_map[jcondition][jvoxel]  = logsig;
	      T_map[jcondition][jvoxel]  = cnr;
	      S_map[jcondition][jvoxel]  = effect_size;
	      dS_map[jcondition][jvoxel] = effect_sdev;
	    }
	  // Calculation of the average must be done after the F tests,
	  // because the data and fit vectors are modified.
	  GLM_correct_baseline(ntime, data_vector, fit_vector, &average);
	  av_sigma_map[0][jvoxel] = average;
	  if ( av_sigma_map[1][jvoxel] != 0. )
	    av_sigma_map[2][jvoxel] = av_sigma_map[0][jvoxel] / av_sigma_map[1][jvoxel];
	  else
	    av_sigma_map[2][jvoxel] = 0.;
	  // Scale the effect sizes and standard deviations.
	  for (jcondition=0; jcondition<nconditions; jcondition++)
	    {
	      if ( units_percent )
		{
		  if ( average == 0. )
		    S_map[jcondition][jvoxel] = dS_map[jcondition][jvoxel] = 0.;
		  else
		    {
		      S_map[jcondition][jvoxel]  *= 100./fabs((double)average);
		      dS_map[jcondition][jvoxel] *= 100./fabs((double)average);
		    }
		}
	      S_map[jcondition][jvoxel]  /= S_divisor;
	      dS_map[jcondition][jvoxel] /= S_divisor;
	    }

	  // Keep track of the percentage of the data that has been processed.
	  if ( (jvoxel) % increment == 0)
	    {
	      printf("\r %td percent done",(100/idiv * (jvoxel)/increment));
	      fflush(stdout);
	    }
	}

      // Remove all events and baseline terms.
      GLM_create_residue(ntime, data_vector, fit_vector);
      for (jtime=0; jtime<ntime; jtime++)
	//	global_residue_vector[jtime] += data_vector[jtime]*data_vector[jtime];
	global_residue_vector[jtime] += data_vector[jtime];
    } // end loop over voxels
  // Free working arrays.
  free_float_vector(data_vector);
  free_float_vector(fit_vector);
  free_float_vector(block_vector);
  free_float_vector(ac_vector);

  for (jtime=0; jtime<ntime; jtime++)
    //    global_residue_vector[jtime] = sqrt((double)global_residue_vector[jtime]/(double)(n3d));
    global_residue_vector[jtime] = global_residue_vector[jtime]/(double)(n3d);
  // Create a weight vector from the global residue vector.
  average = 0.;
  for (jtime=0; jtime<ntime; jtime++)
    average += global_residue_vector[jtime];
  average /= (float)ntime;
  // Find the sd of this vector.  av = 0.;
  sigma2 = 0.;
  for (jtime=0; jtime<ntime; jtime++)
    sigma2 += SQR(global_residue_vector[jtime]-average);
  sigma2 /= (float)ntime;
  weight_sum = 0.;
  for (jtime=0; jtime<ntime; jtime++)
    {
      arg = SQR(global_residue_vector[jtime]-average) /2./(1.*sigma2);
      weight_vector[jtime] = exp((double)(-arg));
      if ( weight_vector[jtime] < 0.5 )
	weight_vector[jtime] = 0.;
      else
	weight_vector[jtime] = 1.;
      weight_sum += weight_vector[jtime];
    }
  /*
  // Write out the residue and weighting factors for each time point.
  if ( !(file = fopen("residue.dat","w")) )
  {
  printf("Error: cannot open the output file residue.dat\n");
  return(1);
  }
  fprintf(file,"x residue weight\n");
  // Output the global residue vector
  for (jtime=0; jtime<ntime; jtime++)
  fprintf(file,"%d %f %f\n",jtime,global_residue_vector[jtime],weight_vector[jtime]);
  fclose(file);
  */

  free_float_vector(global_residue_vector);
  free_float_vector(weight_vector);

  if ( Verbose )
    {
      printf("\n\nResults output to files.\n");
      if ( GLM_input.nifti_output )
	printf("average.nii (average + stdev)\n");
      else
	printf("average.bfloat (average + stdev)\n");
      for (jcondition=0; jcondition<nconditions; jcondition++)
	{
	  if ( GLM_get_condition_string(jcondition, condition_string) ) return(1);
	  if ( GLM_input.nifti_output )
	    printf("  P-%s.nii   T-%s.nii   S-%s.nii\n",
		   condition_string,condition_string,condition_string);
	  else
	    printf("  P-%s.bfloat   T-%s.bfloat   S-%s.bfloat\n",
		   condition_string,condition_string,condition_string);
	}
    }

  printf("\n\n");
  return(0);
}

int read_block_vector(float *block_vector, JIP_INDEX istart, int *block_size)
{
  JIP_INDEX lOffset;
  IPOINT3D dim;
  int lRunOffset, npts, jt, jp;
  int swap_bytes;JIP_INDEX n3d;
  short *iblock;
  int32_t *iiblock;
  float *block;
  struct input_fmri_files *file_list;

  dim = file_list_root.dim;
  n3d = jip_product(jip_product(dim.x,dim.y),dim.z);

  if ( istart + BLOCK_SIZE <= n3d )
    *block_size = BLOCK_SIZE;
  else
    *block_size = jip_int_count(n3d - istart);

  // Position each file at the specified starting point.
  for (file_list=&file_list_root; file_list; file_list = file_list->next)
    {
      lOffset = file_list->voxel_offset;
      if ( file_list->storage == BSHORT ) // bshort
	lOffset += istart * 2;
      else                        // bfloat or blong
	lOffset += istart * 4;
      jip_seek(file_list->fMRI_file, lOffset, SEEK_SET); // offset from beginning of file
    }

  // Loop over the files.
  for (file_list=&file_list_root, lRunOffset=0;
       file_list;
       lRunOffset+=file_list->nt*(*block_size), file_list = file_list->next)
    {
      swap_bytes = file_list->bo != machine_byte_order();
      npts = file_list->nt * (*block_size);
      // Read the block vector.
      if ( file_list->storage == BSHORT ) // .bshort
	{
	  iblock = short_vector( npts );
	  // Read the array.
	  for (jt=0; jt<file_list->nt; jt++)
	    {
	      lOffset = jt * (*block_size);
	      if ( fread((char *)(iblock+lOffset),sizeof(short),*block_size,file_list->fMRI_file) != *block_size )
		{
		  printf("Error reading the short integer data from file %s\n",file_list->fMRI_file_name);
		  return(1);
		}
	      // Update the file to the next word location.
	      lOffset = (n3d-*block_size) * 2; // offset in bytes between consequtive time points
	      if ( jt != file_list->nt - 1 ) jip_seek(file_list->fMRI_file, lOffset, SEEK_CUR);
	    }
	  // Copy the integer array into the stack floating point array.
	  for (jp=0; jp<npts; jp++)
	    {
	      if ( swap_bytes ) swap_16( &(iblock[jp]) );
	      // Fill the time vector, and attach the proper scale factor for run normalization.
	      block_vector[jp+lRunOffset] = iblock[jp] * file_list->scale;
	    }
	  free_short_vector(iblock);
	}
      else if ( file_list->storage == BLONG ) // .blong
	{
	  iiblock = int_vector( npts );
	  // Read the array.
	  for (jt=0; jt<file_list->nt; jt++)
	    {
	      lOffset = jt * (*block_size);
	      if ( fread((char *)(iiblock+lOffset),sizeof(int32_t),*block_size,file_list->fMRI_file) != *block_size )
		{
		  printf("Error reading the long integer data from file %s\n",file_list->fMRI_file_name);
		  return(1);
		}
	      // Update the file to the next word location.
	      lOffset = (n3d-*block_size) * 4; // offset in bytes between consequtive time points
	      if ( jt != file_list->nt - 1 ) jip_seek(file_list->fMRI_file, lOffset, SEEK_CUR);
	    }
	  // Copy the integer array into the stack floating point array.
	  for (jp=0; jp<npts; jp++)
	    {
	      if ( swap_bytes ) swap_32( &(iiblock[jp]) );
	      // Fill the time vector, and attach the proper scale factor for run normalization.
	      block_vector[jp+lRunOffset] = iiblock[jp] * file_list->scale;
	    }
	  free_int_vector(iiblock);
	}
      else // .bfloat
	{
	  block = float_vector( npts );
	  // Read the array.
	  for (jt=0; jt<file_list->nt; jt++)
	    {
	      lOffset = jt * (*block_size);
	      if ( fread((char *)(block+lOffset),sizeof(float),*block_size,file_list->fMRI_file) != *block_size )
		{
		  printf("Error reading the floating point data from file %s\n",file_list->fMRI_file_name);
		  return(1);
		}
	      // Update the file to the next word location.
	      lOffset = (n3d-*block_size) * 4; // offset in bytes between consequtive time points
	      if ( jt != file_list->nt - 1 ) jip_seek(file_list->fMRI_file, lOffset, SEEK_CUR);
	    }
	  // Copy the array into the stack floating point array.
	  for (jp=0; jp<npts; jp++)
	    {
	      if ( swap_bytes ) swap_32( &(block[jp]) );
	      // Fill the time vector, and attach the proper scale factor for run normalization.
	      block_vector[jp+lRunOffset] = block[jp] * file_list->scale;
	    }
	  free_float_vector(block);
	}
    }
  return(0);
}

int smooth_AR_maps()
{
  float *temp_data;
  POINT3D resolution, weight3d;
  IPOINT3D dim;
  int jx, jy, jz, jt, jx_rel, jy_rel, jz_rel, jx_neighbor, jy_neighbor, jz_neighbor;JIP_INDEX jv;
  int ix_rel, iy_rel, iz_rel;JIP_INDEX index_pixel, index_neighbor;
  float fwhm, sigma2, dist2, dist_scale, weight, data_sum, weight_sum;

  fwhm = GLM_input.AR_smooth_mm;

  if ( fwhm == 0. )
    {
      printf("No spatial filtering of AR maps - width = 0\n");
      return(0);
    }
  else
    printf("Smooth AR maps using gaussian kernel FWHM of %5.2f mm\n",fwhm);

  dim = file_list_root.dim;
  resolution = file_list_root.resolution;

  sigma2  = 1.44 * fwhm/2 * fwhm/2;
  // Average over a distance such that the relative contribution of
  // voxels at extremes is at least e^-3
  dist_scale = sqrt((double)(3.*sigma2));

  // Define the limits in x, y, and z
  ix_rel = ceil((double)(dist_scale/resolution.x));
  iy_rel = ceil((double)(dist_scale/resolution.y));
  if ( dim.z == 1 )
    iz_rel = 0;
  else
    iz_rel = ceil((double)(dist_scale/resolution.z));

  printf("(dim.x,dim.y,dim.z,nt) = (%d, %d, %d, %d), voxel range: (+- %d, +- %d, +- %d)\n",
	 dim.x,dim.y,dim.z,GLM_input.AR_order,ix_rel,iy_rel,iz_rel);
  // Allocate space for the new (smoothed) data matrix.
  temp_data = float_vector(((JIP_INDEX)(dim.x))*dim.y*dim.z);
  for (jt=1; jt<=GLM_input.AR_order; jt++)
    {
      printf("\rsmoothing map AR%d",jt);
      fflush(stdout);
      for (jz=0; jz<dim.z; jz++)
	{
	  for (jy=0; jy<dim.y; jy++)
	    {
	      for (jx=0; jx<dim.x; jx++)
		{
		  data_sum = weight_sum = 0.;
		  for (jz_rel=-iz_rel; jz_rel<=iz_rel; jz_rel++)
		    {
		      jz_neighbor = jz + jz_rel;
		      dist2 = jz_rel*jz_rel*resolution.z*resolution.z;
		      weight3d.z = exp(-(double)(dist2/sigma2));
		      for (jy_rel=-iy_rel; jy_rel<=iy_rel; jy_rel++)
			{
			  jy_neighbor = jy + jy_rel;
			  dist2 = jy_rel*jy_rel*resolution.y*resolution.y;
			  weight3d.y = exp(-(double)(dist2/sigma2));
			  for (jx_rel=-ix_rel; jx_rel<=ix_rel; jx_rel++)
			    {
			      jx_neighbor = jx + jx_rel;
			      dist2 = jx_rel*jx_rel*resolution.x*resolution.x;
			      weight3d.x = exp(-(double)(dist2/sigma2));
			      if ( jx_neighbor >= 0 && jx_neighbor < dim.x &&
				   jy_neighbor >= 0 && jy_neighbor < dim.y &&
				   jz_neighbor >= 0 && jz_neighbor < dim.z )
				{
				  index_neighbor = ((JIP_INDEX)(jz_neighbor))*dim.x*dim.y + ((JIP_INDEX)(jy_neighbor))*dim.x + jx_neighbor;
				  weight = weight3d.x * weight3d.y * weight3d.z;
				  data_sum   += weight * auto_cor[index_neighbor][jt];
				  weight_sum += weight;
				}
			    } // jx_rel
			} // jy_rel
		    } // jz_rel
		  index_pixel = ((JIP_INDEX)(jz))*dim.x*dim.y + ((JIP_INDEX)(jy))*dim.x + jx;
		  temp_data[index_pixel] = data_sum / weight_sum;
		} // jx
	    } // jy
	} // jz
      // Copy the temporary array back into the stack array and deallocate
      // the temporary array.
      for (jv=0; jv<((JIP_INDEX)(dim.x))*dim.y*dim.z; jv++)
	auto_cor[jv][jt] = temp_data[jv];
    } // jt
  free_float_vector(temp_data);
  printf("\n");
  return(0);
}

int write_output_files( int nconditions)
{
  FILE *file;
  IPOINT3D dim;
  char file_name[LEN_STR], condition_string[LEN_STR];
  float *AR_map;
  int jcondition;
  JIP_INDEX n3d, error, j;

  dim  = file_list_root.dim;
  n3d = jip_product(jip_product(dim.x,dim.y),dim.z);

  // make a floating point copy of the nAR map for output
  AR_map = float_vector(n3d);
  for (j=0; j<n3d; j++)
    AR_map[j] = nAR_map[j];

  // Average and standard deviation
  if ( open_output_file(0,3,"average",&file) ) return(1);
  error  = fwrite((char *)&(av_sigma_map[0][0]),sizeof(float),n3d,file) != n3d;
  error &= fwrite((char *)&(av_sigma_map[1][0]),sizeof(float),n3d,file) != n3d;
  error &= fwrite((char *)&(av_sigma_map[2][0]),sizeof(float),n3d,file) != n3d;
  if ( error )
    return(1);
  else
    fclose(file);

  /*
  if ( GLM_input.AR_order > 1 )
  {
  // order of AR map
  if ( open_output_file(0,1,"nAR",&file) ) return(1);
  error = fwrite((char *)AR_map,sizeof(float),n3d,file) != n3d;
  if ( error )
  return(1);
  else
  fclose(file);
  }
  */

  // For each condition, open P and T output files.
  for (jcondition=0; jcondition<nconditions; jcondition++)
    {
      if ( GLM_get_condition_string(jcondition, condition_string) ) return(1);

      // Open a file that will contain P values.
      sprintf(file_name,"P-%s",condition_string);
      if ( open_output_file(0,1,file_name,&file) ) return(1);
      error = fwrite((char *)&(P_map[jcondition][0]),sizeof(float),n3d,file) != n3d;
      if ( error )
	return(1);
      else
	fclose(file);

      // Open a file that will contain T values.
      sprintf(file_name,"T-%s",condition_string);
      if ( open_output_file(0,1,file_name,&file) ) return(1);
      error = fwrite((char *)&(T_map[jcondition][0]),sizeof(float),n3d,file) != n3d;
      if ( error )
	return(1);
      else
	fclose(file);

      // Open a file that will contain S values.
      sprintf(file_name,"S-%s",condition_string);
      if ( open_output_file(1,2,file_name,&file) ) return(1);
      error  = fwrite((char *)&(S_map[jcondition][0]),sizeof(float),n3d,file) != n3d;
      error &= fwrite((char *)&(dS_map[jcondition][0]),sizeof(float),n3d,file) != n3d;
      if ( error )
	return(1);
      else
	fclose(file);
    }
  return(0);
}

int open_output_file( BOOL SFile, int ntime, char *file_name_root, FILE **file )
     // ntime: number of time points in file
     // file_name_root : root name of file
     // file : pointer to file output stream
{
  FILE *file_hdr;
  char file_name[LEN_STR];
  int error;

  if ( GLM_input.nifti_output )
    {
      write_nifti_header(SFile, ntime,file_name_root,file);
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
	  file_list_root.dim.x,file_list_root.dim.y,file_list_root.dim.z,ntime,machine_byte_order());
  fprintf(file_hdr,"resolution %g %g %g\norigin %g %g %g\ndirection %g %g %g\n",
	  file_list_root.resolution.x,file_list_root.resolution.y,file_list_root.resolution.z,
	  file_list_root.origin.x,file_list_root.origin.y,file_list_root.origin.z,
	  file_list_root.direction.x,file_list_root.direction.y,file_list_root.direction.z);
  fprintf(file_hdr,"degrees-of-freedom %d\n",GLM_get_DOF());
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

int write_nifti_header( BOOL SFile, int ntime, char *file_name_root, FILE **file )
{
  nifti_1_header nifti_header;
  nifti1_extender pad;
  char full_file_name[LEN_STR];
  int error, j, extension[4];
  mat44 mat;
  float qb, qc, qd, qx, qy, qz, dx, dy, dz, qfac;

  mat = scale_and_shift_to_mat44(file_list_root.resolution, file_list_root.origin, file_list_root.direction);
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
  nifti_header.dim[1] = file_list_root.dim.x;
  nifti_header.dim[2] = file_list_root.dim.y;
  nifti_header.dim[3] = file_list_root.dim.z;
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
  nifti_header.pixdim[1]   = file_list_root.resolution.x;
  nifti_header.pixdim[2]   = file_list_root.resolution.y;
  nifti_header.pixdim[3]   = file_list_root.resolution.z;
  nifti_header.pixdim[4]   = file_list_root.resolution_t;
  nifti_header.pixdim[5]   = 0.;
  nifti_header.pixdim[6]   = 0.;
  nifti_header.pixdim[7]   = 0.;

  // temporary; ignore extended header info from original file
  if ( SFile )
    nifti_header.vox_offset  = 368;  // 352 + 16-byte extension
  else
    nifti_header.vox_offset  = 352;  // 352 + 16-byte extension

  nifti_header.scl_slope   = nifti_header.scl_inter = 0;
  //  nifti_header.xyzt_units  = NIFTI_UNITS_MM | NIFTI_UNITS_SEC;
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
  nifti_header.srow_x[0] = file_list_root.resolution.x * file_list_root.direction.x;
  nifti_header.srow_y[1] = file_list_root.resolution.y * file_list_root.direction.y;
  nifti_header.srow_z[2] = file_list_root.resolution.z * file_list_root.direction.z;
  // The last row is the origin.
  nifti_header.srow_x[3] = file_list_root.origin.x;
  nifti_header.srow_y[3] = file_list_root.origin.y;
  nifti_header.srow_z[3] = file_list_root.origin.z;

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
  if ( SFile )
    pad.extension[0] = 1;  // indicates extension is present
  error = fwrite(&pad, 4, 1, *file) != 1;
  if ( error )
    {
      fprintf(stderr, "\nError writing header file extension pad %s\n",full_file_name);
      return(1);
    }
  if ( SFile )
    {
      // Create a custom extension of 16 bytes as 4 integers.
      extension[0] = 16;            // number of bytes in extension
      extension[1] = 20;            // make up my own unregistered e-code
      extension[2] = GLM_get_DOF(); // degrees of freedom saved here
      extension[3] = 0;
      error = fwrite(extension, sizeof(int), 4, *file) != 4;
      if ( error )
	{
	  fprintf(stderr, "\nError writing nifti header extension for DOF %s\n",full_file_name);
	  return(1);
	}
    }
  return(0);
}
