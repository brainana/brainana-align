#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "nifti1.h"
#include "nifti1_io.h"
#include "fftw3.h"
#include "util.h"
#include "rem-prototypes.h"

#define MACHINE_BYTE_ORDER 1   // 0 for Mac, 1 for Linux

// resolution elements
JIP_INDEX resels;

// Output files.
#define NCONDITIONS_MAX 20
#define NEVENTS_MAX 9

float *Sigma2_FE_map;    // [nvoxels]
float *Sigma2_RE_map;    // [nvoxels]
float *Residue_series;   // [ntime*nvoxels]
float **P_map, **T_map;  // [nconditions][nvoxels]  1 map per condition
float **S_map, **dS_map; // [nconditions][nvoxels]  1 map per condition

struct input_signal_files
{
  char signal_file_name[LEN_STR];   // file name for a given subject or condition
  FILE *signal_file;                // fMRI data file input stream for the run
  IPOINT3D dim;                     // x,y,z dimensions
  int nt;                           // time dimension
  int bo;                           // byte order
  int type;                         // bshort, blong, or bfloat
  int storage;                      // bshort, blong, bfloat
  JIP_INDEX voxel_offset;                 // offset to 1st voxel in file
  float scale;                      // average value of data set for normalization
  float   resolution_t;
  POINT3D resolution;               // resolution from header
  POINT3D origin;                   // origin from header
  POINT3D direction;                // origin from header
  int lDOF;                         // degrees of freedom
  float *average;                   // average value (1st volume)
  float *stdev;                     // standard deviation (2nd volume)
  BOOL stdev_available;             // stdev is contained within the file
  struct input_signal_files *next;
};

struct input_signal_files file_list_root;

// Prototypes
int read_signal_files();
int analyze_random_effects(BOOL write_residue, int nconditions, int lDOF_effective);
int write_output_files( BOOL write_residue, int ntime, int nconditions, int lDOF_effective);
void regularize_random_effects();
void compute_effective_dof( int *lDOF_effective );
void create_simple_data_stack( struct data_stack *new_stack,
			       IPOINT3D dimension, int nt, POINT3D resolution, int data_type );
void smooth_or_sharpen_isotropic( int sharpen, float fwhm, struct data_stack *stack );
int ft_1d( int idim, int forward, struct data_stack *stack );
JIP_INDEX index4d_stack(int ix, int iy, int iz, int it, struct data_stack *stack);
int open_output_file  ( BOOL SFile, int ntime, int lDOF_effective, char *root, FILE **file );
int write_nifti_header( BOOL SFile, int ntime, int lDOF_effective, char *file_name_root, FILE **file );
int read_volume(struct input_signal_files *file_list, int ivolume, float *volume_data);


int main( argc, argv)
int argc;
char *argv[];
{
  FILE *file;
  char *optarg;
  int option, which_arg;
  struct input_signal_files *file_list, *last_file_list;
  char condition_string[LEN_STR], rem_control_file[LEN_STR];
  int jcondition, nconditions, ntime;
  int lDOF_effective;
  BOOL input_ok=FALSE;
  BOOL little_help=FALSE;
  BOOL write_residue=FALSE;

  Verbose = FALSE;

  // Get program options on entry.
  while ( (option=my_getopt(argc,argv,"hHrvVi:",&which_arg)) )
    {
      optarg = argv[which_arg];
      switch ((char)option)
	{
	case 'h':
	  little_help = TRUE;
	  break;
	case 'H':
	  help_menu(0,"rem-menu.txt");
	  exit(0);
	  break;
	case 'i':
	  // Read the analysis file
	  if ( REM_read_control_file(optarg) )
	    {
	      printf("Program terminated due to error in analysis file %s\n",optarg);
	      exit(1);
	    }
	  else
	    {
	      strcpy(rem_control_file,optarg);
	      input_ok = TRUE;
	    }
	  break;
	case 'r':
	  write_residue = TRUE;
	  break;
	case 'v':
	case 'V':
	  Verbose = TRUE;
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
      printf("Syntax: executable [file name] [optional arguments]\n");
      printf("optional arguments:\n");
      printf("-h for this help\n");
      printf("-H for more help using a menu\n");
      printf("-r to write out a residue time series\n");
      printf("\n");
      if ( little_help )
	exit(0);
      else
	exit(1);
    }
  else
    printf("\n");

  // Put the image files into the list.
  for ( ntime=0, file_list=&file_list_root; ntime<REM_input.nfiles; ntime++)
    {
      strcpy(file_list->signal_file_name,REM_input.signal_file_name[ntime]);
      if ( file_list != &file_list_root )last_file_list->next = file_list;
      last_file_list = file_list;
      file_list = (struct input_signal_files *)malloc(sizeof(struct input_signal_files));
      file_list->next = NULL;
    }

  // Read the set of signal files with (beta, stdev) in each file.
  if ( read_signal_files() ) exit(1);

  /* Describe the basis functions. */
  if ( Verbose )
    REM_describe_basis_functions();
  /* Define the comparison condition. */
  if ( REM_define_conditions(REM_input.condition_string,&nconditions) ) exit(1);
  if ( nconditions == 0 )
    {
      printf("Error: no test conditions are defined.\n");
      exit(1);
    }

  // Output the conditions to be tested.
  printf("Conditions to be tested: ");
  for (jcondition=0; jcondition<nconditions; jcondition++)
    {
      if ( REM_get_condition_string(jcondition, condition_string) ) return(1);
      printf("%s ",condition_string);
    }
  printf("\n");

  // Compute the degrees of freedom.
  compute_effective_dof( &lDOF_effective );

  /* Begin reading each file and fitting each voxel. */
  if ( analyze_random_effects(write_residue, nconditions,lDOF_effective) ) exit(1);

  // Write output files (e.g., S-1.bfloat, T-1.bfloat, P-1.bfloat, ...)
  if ( write_output_files(write_residue, ntime, nconditions, lDOF_effective) )
    {
      printf("Error writing output files.\n");
      exit(1);
    }

  // Create an initialization file for xdisplay.
  if ( !(file = fopen(".xdisplay","r")) )
    { // Then file does not exist, so proceed.
      if ( !(file = fopen(".xdisplay","w")) )
	return(1);
      else
	{
	  fprintf(file,"xd -r %s\n",rem_control_file);
	  fclose(file);
	}
    }

  exit(0);
}

int read_signal_files()
{
  struct XD_HEADER header;
  IPOINT3D dim;
  POINT3D resolution, origin, direction;
  int ntime, nfiles;JIP_INDEX j;
  struct input_signal_files *file_list;

  // Read the dimensions of each signal file
  for (file_list=&file_list_root; file_list; file_list = file_list->next)
    {
      // Read the file type and dimensions
      if ( read_volume_header(Verbose, file_list->signal_file_name, &header) )
	return(1);
      file_list->dim  = header.dim;
      file_list->nt   = header.nt;
      file_list->type = header.file_type;
      file_list->storage      = header.storage_type;
      file_list->voxel_offset = header.voxel_offset;
      file_list->bo   = header.byte_order;
      if ( header.lDOF == 0 )
	file_list->lDOF = 1;
      else
	file_list->lDOF = header.lDOF;
      file_list->resolution = header.resolution;
      file_list->origin     = header.origin;
      file_list->direction  = header.direction;
      file_list->resolution_t = header.resolution_t;
      file_list->stdev_available = TRUE;
    }

  dim = file_list_root.dim;
  resolution = file_list_root.resolution;
  origin     = file_list_root.origin;
  direction  = file_list_root.direction;
  ntime = 0;
  for (file_list=&file_list_root, nfiles=0; file_list; file_list = file_list->next, nfiles++)
    {
      if ( file_list->dim.x != dim.x || file_list->dim.y != dim.y || file_list->dim.z != dim.z )
	{ // Make sure the spatial dimensions (x,y,z) are the same for all files
	  printf("The dimensions of file %s (%d,%d,%d) are not the same as for file %s (%d,%d,%d)\n",
		 file_list->signal_file_name,file_list->dim.x,file_list->dim.y,file_list->dim.z,
		 file_list_root.signal_file_name,
		 file_list_root.dim.x,file_list_root.dim.y,file_list_root.dim.z);
	  return(1);
	}
      else if ( file_list->nt != 2 )
	{ // The number of time points should be 2.
	  file_list->stdev_available = FALSE;
	}
      if ( file_list->resolution.x != resolution.x ||
	   file_list->resolution.y != resolution.y ||
	   file_list->resolution.z != resolution.z )
	{
	  printf("WARNING: The resolution of file %s (%g,%g,%g) differs from file %s (%g,%g,%g)\n",
		 file_list->signal_file_name,
		 file_list->origin.x,file_list->origin.y,file_list->origin.z,
		 file_list_root.signal_file_name,
		 file_list_root.origin.x,file_list_root.origin.y,file_list_root.origin.z);
	}
      ntime++;
    }

  printf("Input files: %d samples of dimension (x,y,z) = (%d,%d,%d).\n",
	 nfiles,dim.x,dim.y,dim.z);
  // Define the number of resolution elements.
  if ( REM_input.multiple_comparisons == 0 )
    resels = 1;
  else
    resels = REM_input.multiple_comparisons;

  // Open all the signal data files.
  for (file_list=&file_list_root; file_list; file_list = file_list->next)
    {
      if ( !(file_list->signal_file = fopen(file_list->signal_file_name,"r")) )
	{
	  printf("file name 1 = %s\n",file_list->signal_file_name);
	  printf("Error: cannot open file %s.\n",file_list->signal_file_name);
	  return(1);
	}
      file_list->average = float_vector(((JIP_INDEX)(dim.x))*dim.y*dim.z);
      file_list->stdev   = float_vector(((JIP_INDEX)(dim.x))*dim.y*dim.z);
      read_volume(file_list, 0, file_list->average);
      if ( file_list->stdev_available )
	read_volume(file_list, 1, file_list->stdev);
      else
	{
	  for (j=0; j<((JIP_INDEX)(dim.x))*dim.y*dim.z; j++)
	    file_list->stdev[j] = 0.01;
	}
    }

  return(0);
}

int analyze_random_effects(BOOL write_residue, int nconditions, int lDOF_effective)
{
  FILE *file;
  struct input_signal_files *file_list;
  char condition_string[LEN_STR];
  float *data_vector, *stdev_vector, *fit_vector, *fit_vector_av;
  int *lDOF_vector;
  float Sigma2_FE, Sigma2_RE;
  float cnr, p_raw, p_cor, logsig, effect_size, effect_sdev;
  int ntime, idiv, lReturn, lIgnored;JIP_INDEX nvoxels, increment, error;
  int jtime, jcondition;JIP_INDEX jvoxel;
  BOOL IgnoreVoxel;

  printf("\nAnalyzing random effects ...\n");

  // number of voxels
  nvoxels = ((JIP_INDEX)(file_list_root.dim.x)) * file_list_root.dim.y * file_list_root.dim.z;

  // Find the number of files = total number of time points.
  for (file_list=&file_list_root, ntime=0; file_list; file_list = file_list->next)
    ntime++;

  // With conditions defined, allocate space for maps.
  Sigma2_FE_map  = float_vector(nvoxels);
  Sigma2_RE_map  = float_vector(nvoxels);
  if ( write_residue )
    Residue_series = float_vector(nvoxels*ntime);
  P_map          = float_matrix(nconditions,nvoxels);
  T_map          = float_matrix(nconditions,nvoxels);
  S_map          = float_matrix(nconditions,nvoxels);
  dS_map         = float_matrix(nconditions,nvoxels);

  // Allocate a time vector for fitting each voxel.
  data_vector  = float_vector(ntime);
  stdev_vector = float_vector(ntime);
  lDOF_vector  = int_vector(ntime);
  fit_vector   = float_vector(ntime);
  fit_vector_av= float_vector(ntime);

  // Create a vector of DOF values.
  for (file_list=&file_list_root, jtime=0;
       file_list;
       file_list = file_list->next, jtime++)
    lDOF_vector[jtime] = file_list->lDOF;

  idiv = 100;
  increment = nvoxels / idiv;

  lIgnored = 0;
  // Loop over each voxel.
  for (jvoxel=0; jvoxel<nvoxels; jvoxel++)
    {
      // Read the dimensions of each signal file
      for (file_list=&file_list_root, jtime=0;
	   file_list;
	   file_list = file_list->next, jtime++)
	{
	  data_vector[jtime]  = file_list->average[jvoxel];
	  stdev_vector[jtime] = fabs((double)(file_list->stdev[jvoxel])); // must be >= 0.
	}
      // Fit the time vector.
      lReturn = REM_REML_iterate( ntime, data_vector, stdev_vector, lDOF_vector,
				  &Sigma2_FE, &Sigma2_RE );
      if ( lReturn == 1 )
	exit(1);
      else if ( lReturn < 0 )
	lIgnored++;

      Sigma2_FE_map[jvoxel] = Sigma2_FE;
      Sigma2_RE_map[jvoxel] = Sigma2_RE;

      // Keep track of the percentage of the data that has been processed.
      if ( (jvoxel) % increment == 0)
	{
	  printf("\r %td percent done computing RE variance",(100/idiv * (jvoxel)/increment));
	  fflush(stdout);
	}
    }
  printf("\r 100 percent done computing RE variance\n");
  // Average and standard deviation
  if ( open_output_file(0,3,lDOF_effective,"sigma2",&file) ) return(1);
  error  = fwrite((char *)Sigma2_FE_map,sizeof(float),nvoxels,file) != nvoxels;
  error  = fwrite((char *)Sigma2_RE_map,sizeof(float),nvoxels,file) != nvoxels;
  if ( error )
    return(1);

  if ( REM_input.smooth_ratio_mm != 0. )
    {
      // "Regularize" the random-effects variance by smoothing.
      regularize_random_effects();
      error  = fwrite((char *)Sigma2_RE_map,sizeof(float),nvoxels,file) != nvoxels;
      if ( error )
	return(1);
      else
	fclose(file);
    }

  if ( REM_input.multiple_comparisons != 0 )
    printf("\nCalculate p values corrected for multiple comparisons (%td).\n",resels);
  else
    printf("\nCalculating raw pvalues without correction for multiple comparisons.\n");
  // Re-run the loop, now computing statistics.
  for (jvoxel=0; jvoxel<nvoxels; jvoxel++)
    {
      // Read the dimensions of each signal file
      for (file_list=&file_list_root, jtime=0;
	   file_list;
	   file_list = file_list->next, jtime++)
	{
	  data_vector[jtime]  = file_list->average[jvoxel];
	  stdev_vector[jtime] = file_list->stdev[jvoxel];
	}
      // Fit the time vector.
      Sigma2_RE = Sigma2_RE_map[jvoxel];
      if ( !Sigma2_RE > 0.)
	{
	  IgnoreVoxel = TRUE;
	  logsig = cnr = effect_size = effect_sdev = 0.;
	  if ( write_residue )
	    { // Put zeroes into the 4D residue
	      for (jtime=0; jtime<ntime; jtime++)
		Residue_series[jtime*nvoxels + jvoxel] = 0.;
	    }
	}
      else
	{
	  IgnoreVoxel = FALSE;
	  REM_fit_final( Sigma2_RE, ntime, data_vector, stdev_vector, fit_vector, fit_vector_av );
	  if ( write_residue )
	    { // Put data-fit into the 4D residue
	      for (jtime=0; jtime<ntime; jtime++)
		Residue_series[jtime*nvoxels + jvoxel] = data_vector[jtime] - fit_vector[jtime];
	    }
	}

      // Calculate the cnr & p_raw for each condition.
      for (jcondition=0; jcondition<nconditions; jcondition++)
	{
	  if ( !IgnoreVoxel )
	    {
	      if ( REM_calculate_F(ntime, resels, jcondition, lDOF_effective, &cnr,
				   &p_raw, &p_cor, &effect_size, &effect_sdev) )
	      //	      if ( REM_calculate_T(ntime, resels, jcondition, lDOF_effective, &cnr,
	      //				   &p_raw, &p_cor, &effect_size, &effect_sdev) )
		return(1);
	      if ( REM_input.multiple_comparisons == 0 )
		logsig = -log10((double)p_raw);
	      else
		logsig = -log10((double)p_cor);
	      if ( cnr < 0. ) logsig *= -1;
	    }
	  P_map[jcondition][jvoxel]  = logsig;
	  T_map[jcondition][jvoxel]  = cnr;
	  S_map[jcondition][jvoxel]  = effect_size;
	  dS_map[jcondition][jvoxel] = effect_sdev;
	}
      // Keep track of the percentage of the data that has been processed.
      if ( (jvoxel) % increment == 0)
	{
	  printf("\r %td percent done computing statistics",(100/idiv * (jvoxel)/increment));
	  fflush(stdout);
	}
    }
  printf("\r 100 percent done computing RE variance\n");
  printf("\nResults output to files\n  average.bfloat (average + stdev)\n");
  for (jcondition=0; jcondition<nconditions; jcondition++)
    {
      if ( REM_get_condition_string(jcondition, condition_string) ) return(1);
      printf("  P-%s.bfloat, T-%s.bfloat, & S-%s.bfloat\n",
	     condition_string,condition_string,condition_string);
    }

  // Free working arrays.
  free_float_vector(data_vector);
  free_float_vector(stdev_vector);
  free_int_vector(lDOF_vector);
  free_float_vector(fit_vector);
  free_float_vector(fit_vector_av);

  return(0);
}

int write_output_files( BOOL write_residue, int ntime, int nconditions, int lDOF_effective )
{
  FILE *file;
  char file_name[LEN_STR], condition_string[LEN_STR];
  int jcondition;
  JIP_INDEX nvoxels, error;

  // number of voxels
  nvoxels = ((JIP_INDEX)(file_list_root.dim.x)) * file_list_root.dim.y * file_list_root.dim.z;

  // For each condition, open P and T output files.
  for (jcondition=0; jcondition<nconditions; jcondition++)
    {
      if ( REM_get_condition_string(jcondition, condition_string) ) return(1);

      // Open a file that will contain P values.
      sprintf(file_name,"P-%s",condition_string);
      if ( open_output_file(0,1,lDOF_effective,file_name,&file) ) return(1);
      error = fwrite((char *)&(P_map[jcondition][0]),sizeof(float),nvoxels,file) != nvoxels;
      if ( error )
	return(1);
      else
	fclose(file);

      // Open a file that will contain T values.
      sprintf(file_name,"T-%s",condition_string);
      if ( open_output_file(0,1,lDOF_effective,file_name,&file) ) return(1);
      error = fwrite((char *)&(T_map[jcondition][0]),sizeof(float),nvoxels,file) != nvoxels;
      if ( error )
	return(1);
      else
	fclose(file);

      // Open a file that will contain S values.
      sprintf(file_name,"S-%s",condition_string);
      if ( open_output_file(1,2,lDOF_effective,file_name,&file) ) return(1);
      error  = fwrite((char *)&(S_map[jcondition][0]),sizeof(float),nvoxels,file) != nvoxels;
      error &= fwrite((char *)&(dS_map[jcondition][0]),sizeof(float),nvoxels,file) != nvoxels;
      if ( error )
	return(1);
      else
	fclose(file);
    }

  if ( write_residue )
    {
      // Open a file that will contain residues.
      sprintf(file_name,"residue");
      if ( open_output_file(1,ntime,lDOF_effective,file_name,&file) ) return(1);
      error  = fwrite((char *)Residue_series,sizeof(float),ntime*nvoxels,file) != ntime*nvoxels;
      if ( error )
	return(1);
      else
	fclose(file);
    }

  return(0);
}

void regularize_random_effects()
{
  struct data_stack tau2;
  JIP_INDEX jvoxel;
  IPOINT3D dimension;
  float fwhm;

  // number of voxels
  dimension = file_list_root.dim;

  // make a temporary stack for smoothing     nt                            data_type
  create_simple_data_stack( &tau2, dimension, 1, file_list_root.resolution, 0 );

  // Loop over each voxel.
  for (jvoxel=0; jvoxel<tau2.n3d; jvoxel++)
    {
      if ( Sigma2_FE_map[jvoxel] == 0. )
	tau2.SpaceTime.fData[jvoxel] = 0.;
      else
	tau2.SpaceTime.fData[jvoxel] = 1. + Sigma2_RE_map[jvoxel] / Sigma2_FE_map[jvoxel];
    }

  printf("\nsmoothing variance ratio\n");
  fwhm = REM_input.smooth_ratio_mm;
  smooth_or_sharpen_isotropic( 0, fwhm, &tau2 );

  // Convert tau2 to a regularized random effects variance.
  for (jvoxel=0; jvoxel<tau2.n3d; jvoxel++)
    {
      if ( tau2.SpaceTime.fData[jvoxel] != 0. )
	Sigma2_RE_map[jvoxel] = fabs((double)(tau2.SpaceTime.fData[jvoxel] * Sigma2_FE_map[jvoxel]
					      - Sigma2_FE_map[jvoxel]));
    }
  return;
}

void compute_effective_dof( int *lDOF_effective )
{
  struct input_signal_files *file_list;
  POINT3D resolution;
  float fwhm_signal, fwhm_noise, DOF_ratio;
  int lDOF_fixed, lDOF_random;

  resolution = file_list_root.resolution;
  fwhm_signal = REM_input.smoothed_mm;

  // Increase the effective resolution of the input images by the declared amount of prior smoothing.
  resolution.x = sqrt((double)(fwhm_signal*fwhm_signal + resolution.x*resolution.x));
  resolution.y = sqrt((double)(fwhm_signal*fwhm_signal + resolution.y*resolution.y));
  resolution.z = sqrt((double)(fwhm_signal*fwhm_signal + resolution.z*resolution.z));

  // Set the image resolution as the cubic root of the volume.
  fwhm_signal = resolution.x * resolution.y * resolution.z;
  fwhm_signal = pow((double)fwhm_signal,0.3333);

  fwhm_noise = REM_input.smooth_ratio_mm;

  // Fixed-effects DOF is the sum from the individual files.
  for (file_list=&file_list_root, lDOF_fixed=0;
       file_list;
       file_list = file_list->next)
    lDOF_fixed += file_list->lDOF;
  // Random-effects DOF.
  lDOF_random = REM_get_DOF();

  DOF_ratio = 2.*fwhm_noise*fwhm_noise/fwhm_signal/fwhm_signal + 1.;
  DOF_ratio = pow((double)DOF_ratio,1.5);
  DOF_ratio *= lDOF_random;

  *lDOF_effective = rint((double)( 1./(1./DOF_ratio + 1/(float)lDOF_fixed) ));

  // Output information:
  printf("Effective isotropic resolution    = %4.2f mm\n",fwhm_signal);
  printf("Smoothing FWHM for variance ratio = %g mm\n",fwhm_noise);
  printf("Degrees of freedom: fixed = %d, random = %d, smoothed = %d\n",
	 lDOF_fixed, lDOF_random, *lDOF_effective);

  return;
}

void create_simple_data_stack( struct data_stack *new_stack,
			       IPOINT3D dimension, int nt, POINT3D resolution, int data_type )
{
  new_stack->dim          = dimension;
  new_stack->resolution   = resolution;
  new_stack->nt           = nt;
  new_stack->resolution_t = 1.;
  new_stack->n2d          = jip_product(new_stack->dim.x,new_stack->dim.y);
  new_stack->n3d          = jip_product(new_stack->dim.z,new_stack->n2d);
  new_stack->n4d          = jip_product(new_stack->n3d,new_stack->nt);

  if ( data_type == 0 )
    new_stack->npts_volume = new_stack->n3d;     // magnitude
  else
    new_stack->npts_volume = jip_product(2,new_stack->n3d); // complex
  new_stack->direction.x  = new_stack->direction.y = new_stack->direction.z = 1;
  new_stack->origin.x     = new_stack->origin.y    = new_stack->origin.z    = 0.;
  new_stack->fov.x        = new_stack->dim.x * new_stack->resolution.x;
  new_stack->fov.y        = new_stack->dim.y * new_stack->resolution.y;
  new_stack->fov.z        = new_stack->dim.z * new_stack->resolution.z;
  new_stack->resolution_t = 1.; // time point units
  new_stack->registration = new_stack->status = new_stack->data_type = new_stack->off_diagonal = 0;
  // allocate data
  if ( data_type == 0 )
    new_stack->SpaceTime.fData  = float_vector(new_stack->npts_volume);
  else
    new_stack->SpaceTime.fcData = fcomplex_vector(new_stack->npts_volume);
  new_stack->next = NULL;
}

void smooth_or_sharpen_isotropic( int sharpen, float fwhm, struct data_stack *stack )
     // fwhm is in units of mm (or whatever spatial units are used)
{
  POINT3D resolution, fov2, sigma2;
  float sigma2_k, x, y, z, scale, amp, phase;
  int jx, jy, jz, jt;JIP_INDEX jp, npts, index;
  struct data_stack stack_temp;

  stack_temp.dim = stack->dim;
  stack_temp.n2d = stack->n2d;
  stack_temp.n3d = stack->n3d;
  stack_temp.n4d = stack->n4d;
  stack_temp.nt  = stack->nt;
  stack_temp.resolution = stack->resolution;
  npts = stack->n4d;
  stack_temp.SpaceTime.fcData = fcomplex_vector(npts);
  for (jp=0; jp<npts; jp++)
    {
      stack_temp.SpaceTime.fcData[jp].real = stack->SpaceTime.fData[jp];
      stack_temp.SpaceTime.fcData[jp].imag = 0.;
    }

  // inverse FT in x, y, and z
  ft_1d( 1, -1, &stack_temp ); // x
  ft_1d( 2, -1, &stack_temp ); // y
  ft_1d( 3, -1, &stack_temp ); // z

  // Determine the 3D sigma_k
  // Multiply data by a k-space filter
  sigma2_k = 2.355*2.355/4./PI/PI /fwhm/fwhm;

  // Smooth data by multiplication in k-space
  for ( jt=0; jt<stack->nt; jt++ )
    {
      // Work on 1 volume at a time
      resolution = stack->resolution;
      fov2.x = resolution.x*resolution.x * stack->dim.x*stack->dim.x;
      fov2.y = resolution.y*resolution.y * stack->dim.y*stack->dim.y;
      fov2.z = resolution.z*resolution.z * stack->dim.z*stack->dim.z;
      sigma2.x = sigma2_k * fov2.x; // use normalized fov in each dimension
      sigma2.y = sigma2_k * fov2.y;
      sigma2.z = sigma2_k * fov2.z;
      for ( jz=0; jz<stack->dim.z; jz++ ) // these are steps in k-space
	{
	  z = jz - stack->dim.z/2 + 0.5;
	  for ( jy=0; jy<stack->dim.y; jy++ )
	    {
	      y = jy - stack->dim.y/2 + 0.5;
	      for ( jx=0; jx<stack->dim.x; jx++ )
		{
		  x = jx - stack->dim.x/2 + 0.5;
		  scale
		    = exp(-(double)(x*x/2/sigma2.x))
		    * exp(-(double)(y*y/2/sigma2.y))
		    * exp(-(double)(z*z/2/sigma2.z));
		  if ( sharpen )
		    scale = 1. - scale; // switch low-pass to high-pass
		  index = index4d_stack(jx,jy,jz,jt,&stack_temp);
		  stack_temp.SpaceTime.fcData[index].real *= scale;
		  stack_temp.SpaceTime.fcData[index].imag *= scale;
		}
	    }
	}
    }

  // forward FT in x, y, and z
  ft_1d( 1, 1, &stack_temp ); // x
  ft_1d( 2, 1, &stack_temp ); // y
  ft_1d( 3, 1, &stack_temp ); // z

  // The data should be real, but make sure here.
  for ( jp=0; jp<npts; jp++ )
    {
      amp   = compute_amplitude(stack_temp.SpaceTime.fcData[jp]);
      if ( amp < 0. ) printf("amp[%td] = %g\n",jp,amp);
      phase = compute_phase(stack_temp.SpaceTime.fcData[jp]);
      //      if ( phase < 0. ) amp = -amp;
      //      stack->SpaceTime.fData[jp] = amp;
      stack->SpaceTime.fData[jp] = amp;
    }
  free_fcomplex_vector(stack_temp.SpaceTime.fcData);
}

int ft_1d( int idim, int forward, struct data_stack *stack )
{
  // idim: 1 -> x,
  //       2 -> y,
  //       3 -> z
  // forward: 1 -> forward  (k-space to image space)
  //         -1 -> backward (image space to k-space)
  fftw_plan plan_ft1d;
  fftw_complex *fftwData;
  FCOMPLEX *Data1d;
  float div;
  int lInner, lOuter1, lOuter2, direction;
  int j1, j2, jIn, jt;JIP_INDEX index;

  if ( forward == 1 )
    direction = FFTW_FORWARD;
  else
    direction = FFTW_BACKWARD;
  
  // Set the dimensions based upon the FT direction.
  if ( idim == 1 )
    {
      lInner  = stack->dim.x;
      lOuter1 = stack->dim.y;
      lOuter2 = stack->dim.z;
    }
  else if ( idim == 2 )
    {
      lOuter1 = stack->dim.x;
      lInner  = stack->dim.y;
      lOuter2 = stack->dim.z;
    }
  else if ( idim == 3 )
    {
      lOuter1 = stack->dim.x;
      lOuter2 = stack->dim.y;
      lInner  = stack->dim.z;
    }
  if ( forward == 1 )
    div = (float)lInner;
  else
    div = 1.;

  // Allocate the 1d arrays
  Data1d   = fcomplex_vector(lInner); // floats
  fftwData = (fftw_complex *)fftw_malloc((size_t)jip_product(sizeof(fftw_complex),lInner)); // doubles
  // Loop over time + the 3 spatial dimensions
  for (jt=0; jt<stack->nt; jt++)
    {
      for (j1=0; j1<lOuter1; j1++)
	{
	  for (j2=0; j2<lOuter2; j2++)
	    {
	      for (jIn=0; jIn<lInner; jIn++)
		{
		  if ( idim == 1 )
		    index = index4d_stack(jIn,j1,j2,jt,stack);
		  else if ( idim == 2 )
		    index = index4d_stack(j1,jIn,j2,jt,stack);
		  else if ( idim == 3 )
		    index = index4d_stack(j1,j2,jIn,jt,stack);
		  Data1d[jIn] = stack->SpaceTime.fcData[index];
		}
	      // On the 1st line of data, define the FT "plan".
	      if ( jt == 0 && j1==0 && j2==0 )
		plan_ft1d = fftw_plan_dft_1d(lInner,fftwData,fftwData,direction,FFTW_MEASURE);
	      
	      // swap on input
	      swap_x( Data1d, lInner, 0 );
	      // Assign the fftw directly (rather than use pointers) in case
	      // the byte size differs for the 2 arrays.
	      for (jIn=0; jIn<lInner; jIn++)
		{
		  fftwData[jIn][0] = Data1d[jIn].real;
		  fftwData[jIn][1] = Data1d[jIn].imag;
		}
	      // Transform the data.
	      fftw_execute(plan_ft1d);
	      // Use the original array.
	      for ( jIn=0; jIn<lInner; jIn++ )
		{
		  Data1d[jIn].real = fftwData[jIn][0];
		  Data1d[jIn].imag = fftwData[jIn][1];
		}
	      // The data comes out of the fft routine in wrap-around order.
	      swap_x( Data1d, lInner, 1 );
	      // Put the transformed data back into the original matrix.
	      for (jIn=0; jIn<lInner; jIn++)
		{
		  if ( idim == 1 )
		    index = index4d_stack(jIn,j1,j2,jt,stack);
		  else if ( idim == 2 )
		    index = index4d_stack(j1,jIn,j2,jt,stack);
		  else if ( idim == 3 )
		    index = index4d_stack(j1,j2,jIn,jt,stack);
		  stack->SpaceTime.fcData[index].real = Data1d[jIn].real/div;
		  stack->SpaceTime.fcData[index].imag = Data1d[jIn].imag/div;
		}
	    }
	}
    }

  // Free data and plan allocation
  free_fcomplex_vector(Data1d);
  fftw_destroy_plan(plan_ft1d);
  fftw_free(fftwData);

  return(0);
}

JIP_INDEX index4d_stack(int ix, int iy, int iz, int it, struct data_stack *stack)
{
  if ( ix < 0 || ix >= stack->dim.x ||
       iy < 0 || iy >= stack->dim.y || 
       iz < 0 || iz >= stack->dim.z ||
       it < 0 || it >= stack->nt )
    {
      printf("Programming error: index out of range in index4d_stack.\n");
      printf("(x,y,z,t) = (%d,%d,%d,%d)\n",ix,iy,iz,it);
      exit(1);
    }
  return((JIP_INDEX)it * stack->n3d + iz*stack->n2d + ((JIP_INDEX)(iy))*stack->dim.x + ix);
}

int open_output_file( BOOL SFile, int ntime, int lDOF_effective, char *file_name_root, FILE **file )
     // ntime: number of time points in file
     // file_name_root : root name of file
     // file : pointer to file output stream
{
  FILE *file_hdr;
  char file_name[LEN_STR];
  int error;

  if ( REM_input.nifti_output )
    {
      write_nifti_header(SFile,ntime,lDOF_effective,file_name_root,file);
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
	  file_list_root.dim.x,file_list_root.dim.y,file_list_root.dim.z,ntime,MACHINE_BYTE_ORDER);
  fprintf(file_hdr,"resolution %g %g %g\n",
	  file_list_root.resolution.x,file_list_root.resolution.y,file_list_root.resolution.z);
  fprintf(file_hdr,"origin %g %g %g\n",
	  file_list_root.origin.x,file_list_root.origin.y,file_list_root.origin.z);
  fprintf(file_hdr,"direction %g %g %g\n",
	  file_list_root.direction.x,file_list_root.direction.y,file_list_root.direction.z);
  fprintf(file_hdr,"degrees-of-freedom %d\n",lDOF_effective);
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

int write_nifti_header( BOOL SFile, int ntime, int lDOF_effective, char *file_name_root, FILE **file )
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
      extension[0] = 16;             // number of bytes in extension
      extension[1] = 20;             // make up my own unregistered e-code
      extension[2] = lDOF_effective; // degrees of freedom saved here
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

int read_volume(struct input_signal_files *file_list,  // the file list structure
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
  jip_seek(file_list->signal_file, offset, 0);

  // Read the data vector.
  if ( file_list->storage == BSHORT ) // .bshort
    {
      idata = short_vector(npts);
      // Read the array.
      if ( fread((char *)idata,sizeof(short),npts,file_list->signal_file) != npts )
	{
	  printf("Error reading the short integer data from file %s\n",file_list->signal_file_name);
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
      if ( fread((char *)iidata,sizeof(int32_t),npts,file_list->signal_file) != npts )
	{
	  printf("Error reading the long integer data from file %s\n",file_list->signal_file_name);
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
      if ( fread((char *)data,sizeof(float),npts,file_list->signal_file) != npts )
	{
	  printf("Error reading the floating point data from file %s\n",file_list->signal_file_name);
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
