#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "util.h"
/*
** See the following include file for the definitions of the
** REM data structures.  For data analysis, there is
** no need to include rem.h into the calling program, as the
** functions in this file should handle everything without the
** calling program needing to know details.  The include file
** below has been separated from this file for purposes of simulations,
** which may want to set timing values in the REM structure directly.
*/
#include "rem.h"
//#define RAND_MAX 2147483647.
#define RAND (rand()/RAND_MAX) // a number from 0 to 1

double ibeta(double aa, double bb, double xx);

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
///////// Functions that set up the REM based upon file input ////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

int REM_read_control_file( char *file_name )
{
  FILE *file;
  char whole_line[LEN_STR], keyword[LEN_STR];
  int nline, i1, ivalue;
  int jevent, jstim, jcond;

  // Initialize the REM_input information.
  REM_input.nfiles = 0;
  REM_input.multiple_comparisons = 0;
  REM_input.smoothed_mm = 0.;
  REM_input.smooth_ratio_mm = 1.;
  strcpy(REM_input.condition_string,"");
  REM_input.global_scaling = FALSE;
  REM_input.overlapping_events = FALSE;
  REM_input.nifti_output = FALSE;

  // Initialize REM data structure
  REM.initialized = REM.ntime = REM.nevents = REM.ncoeff = 0;
  // per event
  for (jevent=0; jevent<MAX_EVENTS; jevent++)
    {
      REM.event_number[jevent] = REM.event_shape[jevent] = REM.nstims[jevent] = REM.nsamples[jevent] = 0;
      for (jstim=0; jstim<MAX_STIMS; jstim++)
	REM.event_start[jevent][jstim]  = REM.event_stop[jevent][jstim] = 0;
    }
  for (jcond=0; jcond<MAX_CONDITIONS; jcond++)
    REM.dof[jcond] = 0;

  // Open the file containing the list of runs, REM files, ...
  if ( !(file = fopen(file_name,"r")) )
    {
      printf("Error: cannot open the file %s.\n",file_name);
      return(1);
    }
  // Determine the format type of the control file.
  while ( get_line_with_comment(file,whole_line,LEN_STR) )
    {
      sscanf(whole_line,"%s",keyword);
      if ( !strcmp(keyword,"measurements") || !strcmp(keyword,"measurements:") )
	REM_input.overlapping_events = TRUE;
    }
  rewind(file);

  // Read commands up to the first occurrence of "event [event number]"
  nline = 1;
  ivalue = 0;
  while ( get_line_with_comment(file,whole_line,LEN_STR) )
    {
      if ( sscanf(whole_line,"%d",&i1) < 0 )  /* ... then this is a blank line */
	continue;
      if ( (ivalue=REMInternal_decode_keyword(whole_line)) < 0 )
	{
	  printf("Error reading line %d of file '%s':\n",nline,file_name);
	  REMInternal_describe_rem_format();
	  fclose(file);
	  return(1);
	}
      else if ( ivalue == 1 )
	break;
      nline++;
    }

  if ( !REM_input.overlapping_events )
    {
      if ( REMInternal_read_control_file_no_overlapping_events(file) )
	return(1);
    }
  else
    {
      if ( REMInternal_read_control_file_overlapping_events(file) )
	return(1);
    }
  fclose(file);

  REMInternal_define_design_matrix();

  REM_describe_input();
  REM.initialized = 1;
  return(0);
}

int REMInternal_read_control_file_no_overlapping_events(FILE *file)
{
  char whole_line[LEN_STR], keyword[LEN_STR], *argument, event_char;
  int i1;
  float value;

  // The last command was "event [event number]".
  REM.event_start[REM.nevents][0] = REM.ntime;
  REM.event_shape[REM.nevents]    = SHAPE_SQUARE;
  REM.nstims[REM.nevents]         = 1;
  REM.nsamples[REM.nevents]       = 0;
  // Beginning reading files (average, st. dev) for each sample in this event.
  while ( get_line_with_comment(file, whole_line, LEN_STR) )
    {
      if ( sscanf(whole_line,"%d",&i1) < 0 )  /* ... then this is a blank line */
	continue;
      // This line should contain either 1) signal file, or 2) event [event-number]
      sscanf(whole_line,"%s",keyword);
      argument = next_non_white( whole_line + jip_int_count((JIP_INDEX)strlen(keyword)) );
      
      // Test for the keyword "event"; otherwise, it is a file name.
      // MODIFICATION: labeling subjects/sessions as "events" encourages cheating on the DOF
      //               by using different events within subjects to boost DOF, which really is
      //               is defined as subjects (or sessions, in some cases) minus 2nd-level regressors
      //   - "events" is maintained for backward compatibility, but it is discouraged
      //   - "subjects" or "sessions" are better
      if ( !strcmp(keyword,"event") || !strcmp(keyword,"subjects") || !strcmp(keyword,"sessions") )
	{
	  REM.event_stop[REM.nevents][0] = REM.ntime-1;
	  REM.nevents++;
	  
	  if ( sscanf(argument,"%c",&event_char) != 1 )
	    {
	      printf("Error: `event' requires a character argument.\n");
	      return(1);
	    }
	  if ( !(REM.event_number[REM.nevents] = REM_EventNumberFromChar(event_char)) )
	    {
	      printf("Error: event %c is not an allowed event character.\n",event_char);
	      return(1);
	    }
	  REM.event_shape[REM.nevents]    = SHAPE_SQUARE;
	  REM.event_start[REM.nevents][0] = REM.ntime;
	  REM.nstims[REM.nevents]         = 1;
	  REM.nsamples[REM.nevents]       = 0;
	}
      else
	{
	  strcpy(REM_input.signal_file_name[REM.ntime],keyword);
	  if ( sscanf(argument,"%f",&value) == 1 )
	    {
	      REM_input.signal_magnitude[REM.ntime] = value;
	      REM_input.global_scaling = TRUE;
	    }
	  else
	    REM_input.signal_magnitude[REM.ntime] = 1.;
	  REM.ntime++;
	  REM.nsamples[REM.nevents]++;
	}
    }

  REM.event_stop[REM.nevents][0] = REM.ntime-1;
  // The event counter always follows the actual number by 1, so increment it here.
  REM.nevents++;
  REM_input.nfiles = REM.ntime;

  return(0);
}

int REMInternal_read_control_file_overlapping_events(FILE *file)
{
  char whole_line[LEN_STR], keyword[LEN_STR], *argument, event_char;
  int i1, iOn, iOff, nstims, *ifile_list;
  int jtime, jevent, jstim, jfile;
  float value;
  BOOL read_file_list;

  // The last command was "event [event number] shape".
  // Immediately after the "event" keyword, there should be 2 integers
  // specifying the start and stop files

  // Read the starting and ending file indices.
  REM.nsamples[REM.nevents] = nstims = 0;
  while ( get_line_with_comment(file, whole_line, LEN_STR) )
    {
      if ( sscanf(whole_line,"%d",&i1) < 0 )  /* ... then this is a blank line */
	break;
      // Read the starting and ending file indices.
      if ( sscanf(whole_line,"%d %d",&iOn,&iOff) != 2 )
	{
	  printf("The start and stop indices for files should follow event %c\n",
		 event_char);
	  return(1);
	}
      REM.event_start[REM.nevents][nstims] = iOn;
      REM.event_stop[REM.nevents][nstims]  = iOff;
      REM.nsamples[REM.nevents]           += iOff - iOn + 1;
      nstims++;
    }
  REM.nstims[REM.nevents] = nstims;
  REM.nevents++;

  // Beginning reading files (average, st. dev) for each sample in this event.
  read_file_list = FALSE;
  while ( get_line_with_comment(file, whole_line, LEN_STR) )
    {
      if ( sscanf(whole_line,"%d",&i1) < 0 )  /* ... then this is a blank line */
	continue;
      // This line should contain either 1) signal file, or 2) event [event-number]
      sscanf(whole_line,"%s",keyword);
      argument = next_non_white( whole_line + jip_int_count((JIP_INDEX)strlen(keyword)) );

      if ( read_file_list )
	{
	  strcpy(REM_input.signal_file_name[REM.ntime],keyword);
	  if ( sscanf(argument,"%f",&value) == 1 )
	    REM_input.signal_magnitude[REM.ntime] = value;
	  else
	    REM_input.signal_magnitude[REM.ntime] = 1.;
	  REM.ntime++;
	}
      else
	{
	  // Test for the keyword "event".
	  if ( !strcmp(keyword,"event")  || !strcmp(keyword,"subjects") || !strcmp(keyword,"sessions") )
	    {
	      if ( sscanf(argument,"%c",&event_char) != 1 )
		{
		  printf("Error: `event' requires a character argument.\n");
		  return(1);
		}
	      if ( !(REM.event_number[REM.nevents] = REM_EventNumberFromChar(event_char)) )
		{
		  printf("Error: event %c is not an allowed event character.\n",event_char);
		  return(1);
		}
	      // Read the event shape, if given (if not given, the shape is square)
	      sscanf(argument,"%s",keyword);  // read the event char here to find it's length for below
	      argument = next_non_white( argument + jip_int_count((JIP_INDEX)strlen(keyword)) );
	      if ( sscanf(argument,"%s",keyword) == 1 )
		{
		  if ( !strcmp(keyword,"square") )
		    REM.event_shape[REM.nevents] = SHAPE_SQUARE;
		  else if ( !strcmp(keyword,"ramp") )
		    REM.event_shape[REM.nevents] = SHAPE_RAMP;
		  else
		    {
		      printf("Error: the only defined shapes are 'square' and 'ramp'\n");
		      return(-1);
		    }
		}
	      else
		REM.event_shape[REM.nevents] = SHAPE_SQUARE;

	      // Immediately after the "event" keyword, there should be 2 integers
	      // specifying the start and stop files.
	      // These on/off indices might be repeated.
	      REM.nsamples[REM.nevents] = nstims = 0;
	      while ( get_line_with_comment(file, whole_line, LEN_STR) )
		{
		  if ( sscanf(whole_line,"%d",&i1) < 0 )  /* ... then this is a blank line */
		    break;
		  // Read the starting and ending file indices.
		  if ( sscanf(whole_line,"%d %d",&iOn,&iOff) != 2 )
		    {
		      printf("The start and stop indices for files should follow event %c\n",
			     event_char);
		      return(1);
		    }
		  REM.event_start[REM.nevents][nstims] = iOn;
		  REM.event_stop[REM.nevents][nstims]  = iOff;
		  REM.nsamples[REM.nevents]           += iOff - iOn + 1;
		  nstims++;
		}
	      REM.nstims[REM.nevents] = nstims;
	      REM.nevents++;
	    }
	  else if ( !strcmp(keyword,"measurements") || !strcmp(keyword,"measurements:") )
	    read_file_list = TRUE;
	}
    }

  // The event counter always follows the actual number by 1, so increment it here.
  REM_input.nfiles = REM.ntime;

  // Make sure each file is included in at least one event.
  ifile_list = int_vector(REM.ntime);
  for (jtime=0; jtime<REM.ntime; jtime++)
    ifile_list[jtime] = 0;
  for ( jevent=0; jevent<REM.nevents; jevent++ )
    {
      if ( REM.nstims[jevent] < 1 )
	{
	  printf("Error: event %c has no starting and ending file indicators.\n",
		 REM_EventChar(REM_get_event_number(jevent)));
	  return(1);
	}
      for ( jstim=0; jstim<REM.nstims[jevent]; jstim++ )
	{
	  for (jfile=REM.event_start[jevent][jstim]; jfile<=REM.event_stop[jevent][jstim]; jfile++)
	    ifile_list[jfile] = 1;
	}
    }
  for (jtime=0; jtime<REM.ntime; jtime++)
    {
      if ( ifile_list[jtime] == 0 )
	printf("Warning: file %d [ %s ] is not included in any events!\n",
	       jtime,REM_input.signal_file_name[jtime]);
    }
  free_int_vector(ifile_list);

  return(0);
}

int REMInternal_decode_keyword( char *line )
// return: 0 = ok, -1 = error, 1 = "event"
{
  char keyword[LEN_STR], string[LEN_STR], *argument, event_char;

  sscanf(line,"%s",keyword);
  argument = next_non_white( line + jip_int_count((JIP_INDEX)strlen(keyword)) );

  if ( !strcmp(keyword,"multiple-comparisons") )
    {
      if ( sscanf(argument,"%d",&(REM_input.multiple_comparisons)) != 1 )
	{
	  printf("Error: `multiple-comparisons' requires an integer argument.\n");
	  return(-1);
	}
      return(0);
    }
  else if ( !strcmp(keyword,"smoothing") || !strcmp(keyword,"smooth") )
    {
      if ( sscanf(argument,"%f %f",&(REM_input.smoothed_mm),&(REM_input.smooth_ratio_mm)) != 2 )
	{
	  printf("Error: `smoothing' requires 2 arguments (mm).\n");
	  printf("1) width of smoothing kernel in prior analysis,\n");
	  printf("2) width of smoothing kernel used to boost random-effects DOF\n");
	  return(-1);
	}
      return(0);
    }
  else if ( !strcmp(keyword,"conditions") )
    {
      strcpy(REM_input.condition_string,argument);
      return(0);
    }
  else if ( !strcmp(keyword,"output-format") )
    {
      sscanf(argument,"%s",string);
      if ( !strcmp(string,"nifti") || !strcmp(string,"NIFTI") )
	REM_input.nifti_output = TRUE;
      return(0);
    }
  else if ( !strcmp(keyword,"event")  || !strcmp(keyword,"subjects") || !strcmp(keyword,"sessions") )
    {
      if ( sscanf(argument,"%c",&event_char) != 1 )
	{
	  printf("Error: 'subjects' or 'sessions' or 'events' requires a character argument.\n");
	  return(-1);
	}
      if ( !(REM.event_number[REM.nevents] = REM_EventNumberFromChar(event_char)) )
	{
	  printf("Error: event %c is not an allowed event character.\n",event_char);
	  return(-1);
	}
      // Read the event shape, if given (if not given, the shape is square)
      sscanf(argument,"%s",keyword);  // read the event char here to find it's length for below
      argument = next_non_white( argument + jip_int_count((JIP_INDEX)strlen(keyword)) );
      if ( sscanf(argument,"%s",keyword) == 1 )
	{
	  if ( !strcmp(keyword,"square") )
	    REM.event_shape[REM.nevents] = SHAPE_SQUARE;
	  else if ( !strcmp(keyword,"ramp") )
	    REM.event_shape[REM.nevents] = SHAPE_RAMP;
	  else
	    {
	      printf("Error: the only defined shapes are 'square' and 'ramp'\n");
	      return(-1);
	    }
	}
      else
	REM.event_shape[REM.nevents] = SHAPE_SQUARE;
      return(1);
    }
  else // not found
    return(-1);
}

void REMInternal_describe_rem_format()
{ // This is printed as the results of input error
  printf("\nExample input file:\n\n");
  printf("smoothing 0.5 1.5  # [prior smoothing (mm)] [smoothing for var. ratio]\n");
  printf("conditions p c p-c # mandatory; Statistical tests MUST be included\n");
  printf("event p # pre-treated group\n");
  printf("../group1/subject1/S-1.bfloat # any previous output file\n");
  printf("../group1/subject2/S-1.bfloat # same output file, but diffent analysis\n");
  printf("../group1/subjectn/S-1.bfloat\n");
  printf("event c # control group");
  printf("../group2/subject1/S-1.bfloat\n");
  printf("../group2/subject2/S-1.bfloat\n");
  printf("../group2/subjectn/S-1.bfloat\n");
  printf("...\n");

  printf("\nOR\n\n");

  printf("\nExample input file:\n\n");
  printf("smoothing 0.5 1.5  # [prior smoothing (mm)] [smoothing for var. ratio]\n");
  printf("conditions p c p-c # mandatory; Statistical tests MUST be included\n");
  printf("event p square # pre-treated group, DC level\n");
  printf("0 7            # start, stop files; there could be more entries below\n");
  printf("event P ramp   # pre-treated group, linear change over time");
  printf("0 7            # start, stop files\n");
  printf("event c square # pre-treated group, DC level\n");
  printf("8 15           # start, stop files\n");
  printf("event C ramp   # pre-treated group, linear change over time");
  printf("8 15           # start, stop files\n");
  printf("measurements:");
  printf("../group1/subject1/S-1.bfloat\n");
  printf("../group1/subject2/S-1.bfloat\n");
  printf("../group1/subject3/S-1.bfloat\n");
  printf("...\n");
  printf("../group1/subjectn/S-1.bfloat\n");
  printf("../group1/subjectn/S-1.bfloat\n");
  printf("../group2/subject1/S-1.bfloat\n");
  printf("../group2/subject2/S-1.bfloat\n");
  printf("../group2/subject3/S-1.bfloat\n");
  printf("...\n");
  printf("../group2/subjectn/S-1.bfloat\n");
  printf("...\n");
}
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
///////// Functions that create and use the REM //////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void REMInternal_define_design_matrix()
     // Define the basis vectors of a general linear model (REM)
     // based upon a hemodynamic response function (IRF) and
     // information about the times for the stimuli.
{
  int icoeff, nfiles;
  int jevent, jstim, jtime, jtime1, jcoeff, jcoeff1;
  float av_signal_mag, amplitude, time, duration;

  // Set the number of coefficients in the REM:
  REM.ncoeff = REM.nevents;

  // Allocate and initialize new arrays.
  REM.Beta_c      = float_vector(REM.ncoeff);
  REM.X_tc        = float_matrix(REM.ntime,REM.ncoeff);
  REM.Xw_tc       = float_matrix(REM.ntime,REM.ncoeff);

  REM.XTXw_cc     = float_matrix(REM.ncoeff,REM.ncoeff);
  REM.Variance_cc = float_matrix(REM.ncoeff,REM.ncoeff);

  REM.XTSIGm1_ct  = float_matrix(REM.ncoeff,REM.ntime);
  REM.Product_ct  = float_matrix(REM.ncoeff,REM.ntime);

  REM.XProduct_tt = float_matrix(REM.ntime,REM.ntime);
  REM.Identity_tt = float_matrix(REM.ntime,REM.ntime);
  REM.SIGm1_tt    = float_matrix(REM.ntime,REM.ntime);
  REM.Residue_tt  = float_matrix(REM.ntime,REM.ntime);
  REM.Residue2_tt = float_matrix(REM.ntime,REM.ntime);
  REM.SigFE_tt    = float_matrix(REM.ntime,REM.ntime);
  REM.SigFE2_tt   = float_matrix(REM.ntime,REM.ntime);
  REM.SigFE2R_tt  = float_matrix(REM.ntime,REM.ntime);

  for (jcoeff=0; jcoeff<REM.ncoeff; jcoeff++)
    {
      REM.Beta_c[jcoeff] = 0.;
      for (jtime=0; jtime<REM.ntime; jtime++)
	{
	  REM.X_tc[jtime][jcoeff] = REM.Xw_tc[jtime][jcoeff] = 0.;
	  REM.Product_ct[jcoeff][jtime] = REM.XTSIGm1_ct[jcoeff][jtime] = 0.;
	}
      for (jcoeff1=0; jcoeff1<REM.ncoeff; jcoeff1++)
	REM.XTXw_cc[jcoeff][jcoeff1] = REM.Variance_cc[jcoeff][jcoeff1] = 0.;
    }

  for (jtime=0; jtime<REM.ntime; jtime++)
    {
      for (jtime1=0; jtime1<REM.ntime; jtime1++)
	{
	  REM.SIGm1_tt[jtime][jtime1] = REM.Residue_tt[jtime][jtime1] = REM.Residue2_tt[jtime][jtime1] = 0.;
	  REM.SigFE_tt[jtime][jtime1] = REM.SigFE2_tt[jtime][jtime1]  = REM.SigFE2R_tt[jtime][jtime1]  = 0.;
	  REM.XProduct_tt[jtime][jtime1] = 0.;
	  if ( jtime == jtime1 )
	    REM.Identity_tt[jtime][jtime1] = 1.;
	  else
	    REM.Identity_tt[jtime][jtime1] = 0.;
	}
    }

  // Create the design matrix (basis functions).
  for (jevent=0, icoeff=0; jevent<REM.nevents; jevent++, icoeff++)
    {
      // Find the average signal magnitude in order to scale the basis functions
      // to an average value of 1.
      for (jstim=0; jstim<REM.nstims[jevent]; jstim++)
	{
	  for (jtime=REM.event_start[jevent][jstim], nfiles=0, av_signal_mag=0.;
	       jtime<=REM.event_stop[jevent][jstim];
	       jtime++)
	    {
	      av_signal_mag += REM_input.signal_magnitude[jtime];
	      nfiles++;
	    }
	  av_signal_mag = fabs((double)(av_signal_mag/(float)nfiles));
	}

      // For each event, create the column in the design vector.
      for (jtime=0; jtime<REM.ntime; jtime++)
	{
	  REM.X_tc[jtime][icoeff] = 0;
	  for (jstim=0; jstim<REM.nstims[jevent]; jstim++)
	    {
	      if ( jtime >= REM.event_start[jevent][jstim] && jtime <= REM.event_stop[jevent][jstim] )
		{
		  amplitude = REM_input.signal_magnitude[jtime] / av_signal_mag;
		  if ( REM.event_shape[jevent] == SHAPE_SQUARE )
		    REM.X_tc[jtime][icoeff] = amplitude;
		  else if ( REM.event_shape[jevent] == SHAPE_RAMP )
		    {
		      time = jtime - REM.event_start[jevent][jstim];
		      duration = REM.event_stop[jevent][jstim] - REM.event_start[jevent][jstim];
		      // REM.X_tc[jtime][icoeff] = -amplitude + time/duration * 2. * amplitude;
		      REM.X_tc[jtime][icoeff] = time/duration * amplitude;
		    }
		}
	    }
	}
    }
  return;
}

int REM_REML_iterate( int ntime,            // number of time points (dimension for vectors)
		      float *Signal_vector, // array of effects (beta values from level 1)
		      float *SigFE_vector,  // array of fixed-effects sigma values
		      int   *lDOF_vector,   // vector holding fixed effects degrees of freedom
		      float *Sigma2_FE,     // fixed-effects variance
		      float *Sigma2_RE )    // random effects variance
{
  float *RY, SigRE2, SigRE2_new, SigRE2_new_part1, SigRE2_new_part2;
  float fom, TOL=0.05;
  int lDOF_total, iRowFail;
  int jtime, jtime1;
  LOGICAL any_zero;

  // Make sure the REM has been initialized.
  if ( REM.initialized == 0 )
    {
      printf("Error in REM_fit_data: first initialize the REM\n");
      return(1);
    }
  // Make sure the REM model and the input data are consistent.
  if ( ntime != REM.ntime )
    {
      printf("Error: the number of data time points (%d) does not match the number in the REM (%d).\n",
	     ntime,REM.ntime);
      exit(1);
    }

  // If any sigma values are zero, ignore the voxel.
  for (jtime=0, any_zero=FALSE; jtime<REM.ntime; jtime++)
    if ( !(SigFE_vector[jtime]>0.) ) any_zero=TRUE;  // takes care of 'nan' too ??
  if ( any_zero )
    {
      *Sigma2_FE = *Sigma2_RE = 0.;
      return(-1);
    }

  // Calculate the fixed-effects variance.
  for (jtime=0, lDOF_total=0; jtime<REM.ntime; jtime++)
    lDOF_total += lDOF_vector[jtime];
  for (jtime=0, *Sigma2_FE=0.; jtime<REM.ntime; jtime++)
    *Sigma2_FE += (float)lDOF_vector[jtime]/(float)lDOF_total * SigFE_vector[jtime]*SigFE_vector[jtime];

  // Create the SigFE matrix.
  for (jtime=0; jtime<REM.ntime; jtime++)
    {
      for (jtime1=0; jtime1<REM.ntime; jtime1++)
	{
	  if ( jtime == jtime1 )
	    REM.SigFE_tt[jtime][jtime1] = SigFE_vector[jtime];
	  else
	    REM.SigFE_tt[jtime][jtime1] = 0.;
	}
    }
  // Calculate the square of this matrix: SigFE2
  //             mat_product      dim1       dim2        mat1       T  dim_sum      mat2        T
  Matrix_Product(REM.SigFE2_tt, REM.ntime, REM.ntime, REM.SigFE_tt, 0, REM.ntime, REM.SigFE_tt, 0);

  // As a starting value of the sigma array, use the identify matrix.
  for (jtime=0; jtime<REM.ntime; jtime++)
    {
      for (jtime1=0; jtime1<REM.ntime; jtime1++)
	REM.SIGm1_tt[jtime][jtime1] = REM.Identity_tt[jtime][jtime1];
    }
  // Invert Sigma
  rInvertSquareMatrix( REM.ntime, REM.SIGm1_tt, &iRowFail );
  REMInternal_compute_residue_matrix();

  // Allocate RY
  RY = float_vector(REM.ntime);
  // Find the initial value of the random effects variance: SigRE = Y'RY/(dof)
  for (jtime=0; jtime<REM.ntime; jtime++)
    {
      RY[jtime] = 0.;
      for (jtime1=0; jtime1<REM.ntime; jtime1++)
	RY[jtime] += REM.Residue_tt[jtime][jtime1] * Signal_vector[jtime1];
    }
  SigRE2 = 0.;
  for (jtime=0; jtime<REM.ntime; jtime++)
    SigRE2 += Signal_vector[jtime] * RY[jtime];
  SigRE2 /= (float)(REM.ntime - REM.ncoeff);
  SigRE2_new = 0.;

  // Figure of merit is relative change in SigRE:
  fom = fabs((double)(1. - SigRE2_new/SigRE2));

  while ( fom > TOL )
    {
      // Create the sigma array, a sum of fixed and random effects:
      // Sigma_jj = (SigFE^2 + SigRan^2) * I_jj
      for (jtime=0; jtime<REM.ntime; jtime++)
	{
	  for (jtime1=0; jtime1<REM.ntime; jtime1++)
	    REM.SIGm1_tt[jtime][jtime1] = REM.SigFE2_tt[jtime][jtime1]
	      + SigRE2 * REM.Identity_tt[jtime][jtime1];
	}
      // Invert Sigma
      rInvertSquareMatrix( REM.ntime, REM.SIGm1_tt, &iRowFail );
      // Compute the residue matrix.
      REMInternal_compute_residue_matrix();

      // Calculate SigFE2 * Residue
      //          mat_product          dim1       dim2         mat1       T  dim_sum       mat2         T
      Matrix_Product(REM.SigFE2R_tt, REM.ntime, REM.ntime, REM.SigFE2_tt, 0, REM.ntime, REM.Residue_tt, 0);
      SigRE2_new_part1 = SigRE2 * ( REM.ncoeff + Trace_of_Matrix(REM.ntime,REM.SigFE2R_tt) );

      // Calculate sigma^4 * S * R2 * S
      for (jtime=0; jtime<REM.ntime; jtime++)
	{
	  RY[jtime] = 0.;
	  for (jtime1=0; jtime1<REM.ntime; jtime1++)
	    RY[jtime] += REM.Residue2_tt[jtime][jtime1] * Signal_vector[jtime1];
	}
      SigRE2_new_part2 = 0.;
      for (jtime=0; jtime<REM.ntime; jtime++)
	SigRE2_new_part2 += Signal_vector[jtime] * RY[jtime];
      SigRE2_new_part2 *= SigRE2 * SigRE2;

      SigRE2_new = ( SigRE2_new_part1 + SigRE2_new_part2 ) / REM.ntime;

      // Figure of merit is relative change in SigRE:
      fom = fabs((double)(1. - SigRE2_new/SigRE2));
      SigRE2 = SigRE2_new;
    }

  // Random effects variance
  *Sigma2_RE = SigRE2_new;
  //  *Sigma2_RE = REM.Variance_cc[0][0];

  free_float_vector(RY);
  return(0);
}

void REMInternal_compute_residue_matrix()
{
  int jtime, jtime1;

  // Calculate XTSIGm1
  //             mat_product       dim1       dim2       mat1     T  dim_sum     mat2         T
  Matrix_Product(REM.XTSIGm1_ct, REM.ncoeff, REM.ntime, REM.X_tc, 1, REM.ntime, REM.SIGm1_tt, 0);

  // Calculate the weighted average value of X.
  //             mat_product  dim1       dim2         mat1       T  dim_sum     mat2     T
  Matrix_Product(REM.Xw_tc, REM.ntime, REM.ncoeff, REM.SIGm1_tt, 0, REM.ntime, REM.X_tc, 0);

  // Calculate XTXw
  //             mat_product       dim1     dim2       mat1     T  dim_sum     mat2      T
  Matrix_Product(REM.XTXw_cc, REM.ncoeff, REM.ncoeff, REM.X_tc, 1, REM.ntime, REM.Xw_tc, 0);

  // Compute the pseudo-inverse of XTXw
  //                   mat_in         dim1       dim2       mat_out
  Matrix_PseudoInverse(REM.XTXw_cc, REM.ncoeff, REM.ncoeff, REM.Variance_cc);

  // Calculate REM.Product = Variance * XTSIGm1
  //             mat_product       dim1       dim2          mat1         T  dim_sum     mat2            T
  Matrix_Product(REM.Product_ct, REM.ncoeff, REM.ntime, REM.Variance_cc, 0, REM.ncoeff, REM.XTSIGm1_ct, 0);

  // Calculate X * Product
  //             mat_product       dim1       dim2        mat1    T  dim_sum       mat2          T
  Matrix_Product(REM.XProduct_tt, REM.ntime, REM.ntime, REM.X_tc, 0, REM.ncoeff, REM.Product_ct, 0);

  // Calculate SIG^-1 * X * Product and put result in residue matrix
  //             mat_product       dim1       dim2        mat1       T  dim_sum         mat2        T
  Matrix_Product(REM.Residue_tt, REM.ntime, REM.ntime, REM.SIGm1_tt, 0, REM.ntime, REM.XProduct_tt, 0);

  // The residue is actually REM.Residue_tt = REM.SIGm1_tt - (SIG^-1 * X * Product)_tt
  for (jtime=0; jtime<REM.ntime; jtime++)
    {
      for (jtime1=0; jtime1<REM.ntime; jtime1++)
	{
	  REM.Residue_tt[jtime][jtime1] *= -1;
	  REM.Residue_tt[jtime][jtime1] += REM.SIGm1_tt[jtime][jtime1];
	}
    }
  // Calculate the square of this matrix: Residue^2
  //             mat_product        dim1       dim2        mat1         T  dim_sum      mat2          T
  Matrix_Product(REM.Residue2_tt, REM.ntime, REM.ntime, REM.Residue_tt, 0, REM.ntime, REM.Residue_tt, 0);
}

int REM_fit_final( float Sigma2_RE,      // random effects variance
		   int ntime,            // number of time points (dimensions for vectors)
		   float *Signal_vector, // array of effects (beta values from level 1)
		   float *SigFE_vector,  // array of fixed-effects sigma values
		   float *fit_vector,    // fit vector (X*Beta)
		   float *fit_vector_av) // fit vector with global scaling removed
{
  int iRowFail;
  int jtime, jtime1, jcoeff, jcoeff1;
  LOGICAL any_zero;

  // Make sure the REM has been initialized.
  if ( REM.initialized == 0 )
    {
      printf("Error in REM_fit_data: first initialize the REM\n");
      return(1);
    }
  // Make sure the REM model and the input data are consistent.
  if ( ntime != REM.ntime )
    {
      printf("Error: the number of data time points (%d) does not match the number in the REM (%d).\n",
	     ntime,REM.ntime);
      exit(1);
    }

  // If any sigma values are zero, ignore the voxel.
  for (jtime=0, any_zero=FALSE; jtime<REM.ntime; jtime++)
    if ( SigFE_vector[jtime] == 0. ) any_zero=TRUE;
  any_zero = FALSE;
  if ( any_zero )
    {
      // Set to zero
      // 1) fit vector
      // 2) beta (equivalent to fit
      // 2) variance matrix, which is used in statistical tests.
      for (jtime=0; jtime<REM.ntime; jtime++)
	fit_vector[jtime]= fit_vector_av[jtime] = 0.;
      for (jcoeff=0; jcoeff<REM.ncoeff; jcoeff++)
	{
	  REM.Beta_c[jcoeff] = 0.;
	  for (jcoeff1=0; jcoeff1<REM.ncoeff; jcoeff1++)
	    REM.Variance_cc[jcoeff][jcoeff1] = 0.;
	}
      return(0);
    }

  // Create the SigFE matrix.
  for (jtime=0; jtime<REM.ntime; jtime++)
    {
      for (jtime1=0; jtime1<REM.ntime; jtime1++)
	{
	  if ( jtime == jtime1 )
	    REM.SigFE_tt[jtime][jtime1] = SigFE_vector[jtime];
	  else
	    REM.SigFE_tt[jtime][jtime1] = 0.;
	}
    }
  // Calculate the square of this matrix: SigFE2
  //             mat_product      dim1       dim2        mat1       T  dim_sum      mat2        T
  Matrix_Product(REM.SigFE2_tt, REM.ntime, REM.ntime, REM.SigFE_tt, 0, REM.ntime, REM.SigFE_tt, 0);

  // Create the sigma array, a sum of fixed and random effects:
  // Sigma_jj = (SigFE^2 + SigRan^2) * I_jj
  for (jtime=0; jtime<REM.ntime; jtime++)
    {
      for (jtime1=0; jtime1<REM.ntime; jtime1++)
	{
	  REM.SIGm1_tt[jtime][jtime1] = REM.SigFE2_tt[jtime][jtime1]
	    + Sigma2_RE * REM.Identity_tt[jtime][jtime1];
	}
    }
  // Invert Sigma
  rInvertSquareMatrix( REM.ntime, REM.SIGm1_tt, &iRowFail );

  // Compute the residue matrix and associated matrices.
  REMInternal_compute_residue_matrix();

  // Compute Beta
  for (jcoeff=0; jcoeff<REM.ncoeff; jcoeff++)
    {
      REM.Beta_c[jcoeff] = 0.;
      for (jtime=0; jtime<REM.ntime; jtime++)
	REM.Beta_c[jcoeff] += REM.Product_ct[jcoeff][jtime] * Signal_vector[jtime];
    }

  // Assign the fit vector.
  for (jtime=0; jtime<REM.ntime; jtime++)
    {
      fit_vector[jtime] = fit_vector_av[jtime] = 0.;
      for (jcoeff=0; jcoeff<REM.ncoeff; jcoeff++)
	{
	  fit_vector[jtime] += REM.X_tc[jtime][jcoeff] * REM.Beta_c[jcoeff] ;
	  if ( REM.X_tc[jtime][jcoeff] != 0. ) // for average, set X_tc = 0 or 1
	    fit_vector_av[jtime] += REM.Beta_c[jcoeff] ;
	}
    }

  return(0);
}

int REM_define_conditions( char *condition_string, int *nconditions )
{ // must be called AFTER REM_read_control_file
  char *cptr, *c_event_number, cond_str[LEN_STR];
  int event_number, isign, length, found;
  int jchar, jevent, jcoeff, jcon, icondition, icontrast;
  static int ncoeff_save;

  // Make sure that REM_define_basis_functions has been called.
  if ( REM.initialized == 0 )
    {
      printf("Error: the REM basis functions should be defined before the conditions.\n");
      return(1);
    }
  // For the 1st pass, ncoeff_save will be 0, so set it.
  if ( ncoeff_save == 0 ) ncoeff_save = REM.ncoeff;

  //  Set up the conditional tests based upon "condition_string".
  //  This string can contain more than 1 condition.
  //  Example: "1 2 3 4 1234 12m34 1m3-2m4
  //  yields 7 conditions, including 6 T tests:
  //    0-3) event 1 versus baseline, 2 versus baseline, ...
  //    4) event 1+2+3+4 versus baseline
  //    5) event 1+2-3-4 versus baseline
  //  ... and 1 F test
  //    6) events 1-3 or 2-4

  // Reset all information about the conditions
  REM.nconditions = 0;
  for (jcon=0; jcon<MAX_CONDITIONS; jcon++)
    {
      // Free contrast matrix for each condition.
      if ( REM.contrast_matrix[jcon] != NULL )
	free_int_matrix(REM.contrast_matrix[jcon],REM.ncontrasts[jcon],ncoeff_save);
      // Reset contrasts to 0
      REM.ncontrasts[jcon] = 0;
    }

  // Save number of coefficients, since this info is needed to release arrays above.
  ncoeff_save = REM.ncoeff;

  // If the condition string is blank, return
  if ( sscanf(condition_string,"%d",&isign) < 0 )
    {
      *nconditions = 0;
      return(0);
    }

  // Count the number of elements in the condition string.
  cptr=condition_string;
  while ( sscanf(cptr,"%s",cond_str) == 1 )
    {
      // For each condition, count the number of ',' characters, indicating new matrix rows.
      length = jip_int_count((JIP_INDEX)strlen(cond_str));
      for (jchar=0, REM.ncontrasts[REM.nconditions]=0;
	   jchar<length; jchar++)
	{ // Each new appearance of ',' indicates a new row in the contrast matrix
	  if ( *(cptr+jchar) == ',' ) REM.ncontrasts[REM.nconditions]++;
	}
      REM.ncontrasts[REM.nconditions]++;         // There is 1 row, even if no ',' was found

      // Allocate the contrast matrix for this condition, and initialize it.
      REM.contrast_matrix[REM.nconditions] = int_matrix(REM.ncontrasts[REM.nconditions],REM.ncoeff);
      for (jcon=0; jcon<REM.ncontrasts[REM.nconditions]; jcon++)
	{
	  for (jcoeff=0; jcoeff<REM.ncoeff; jcoeff++)
	    REM.contrast_matrix[REM.nconditions][jcon][jcoeff] = 0;
	}
      // Update the number of conditions.
      REM.nconditions++;
      cptr = next_non_white(cptr+jip_int_count((JIP_INDEX)strlen(cond_str)));
    }
  *nconditions = REM.nconditions;

  // Pick apart each condition and analyze the string.
  icondition = 0;
  cptr=condition_string;
  while ( sscanf(cptr,"%s",cond_str) == 1 )
    {
      icontrast = 0;
      // printf("test string %s\n",cond_str);
      // cond_str is the individual condition to be analyzed.
      strcpy(REM.condition_string[icondition],cond_str); // save string for identification
      length = jip_int_count((JIP_INDEX)strlen(cond_str));
      isign = 1;
      for (jchar=0; jchar<length; jchar++)
	{
	  c_event_number = cond_str+jchar;
	  event_number = *c_event_number - 48;  // convert character to integer using ascii code
	  event_number = REM_EventNumberFromChar(*c_event_number);
	  if ( *c_event_number == ',' )
	    {
	      icontrast++;  // end this contrast, and make a new row in the contrast matrix
	      isign = 1;    // reset the sign for the new contrast vector
	    }
	  else if ( *c_event_number == '-' || *c_event_number == '-' )
	    isign = -1;   // change the sign of the event in the test contrast
	  else
	    {
	      // Loop over the number of stimulus events.
	      for (jevent=found=0; jevent<REM.nevents; jevent++)
		{
		  if ( REM.event_number[jevent] == event_number )
		    {
		      found = 1;
		      // printf("set contrast [%d][%d][%d] to %d\n",icondition,icontrast,jevent,isign);
		      REM.contrast_matrix[icondition][icontrast][jevent] = isign;
		    }
		}
	      if ( !found )
		{ // Make sure the specified event was found.
		  printf("Error: one of the events in the conditions (%d) is not defined.\n",
			 event_number);
		  return(1);
		}
	    }
	}
      // Update the condition counter and go to the next condition string.
      icondition++;
      cptr = next_non_white(cptr+jip_int_count((JIP_INDEX)strlen(cond_str)));
    }

  return(0);
}

int REM_calculate_T(int ntime,          // input: number of time points
		    JIP_INDEX resels,         // resolution elements (for multiple comparisons)
		    int lCondition,     // an index between 0 and REM.nconditions-1
		    int lDOF_effective, // effective DOF after regularizing RE variance
		    float *cnr,         // contrast to noise ratio
		    float *p_raw,       // output raw p-value for the relevant condition
		    float *p_cor,       // output corrected p-value for the relevant condition
		    float *effect_size, // size of effect
		    float *effect_sdev) // stardard deviation of effect
{
  float CBeta, stdev, Tstat, Fstat;
  float df1, df2, ec3;
  float too_small = exp(-100.), ec3constant=.1169;
  int jcoeff, jcoeff1;

  /*
  ** Make sure the condition is within range.
  */
  if ( lCondition < 0 || lCondition >= REM.nconditions )
    {
      printf("Error: the specified condition (%d) is out of range (0-%d)\n",
	     lCondition,REM.nconditions);
      return(1);
    }
  else if ( REM.ncontrasts[lCondition] != 1 )
    {
      printf("Error: T tests evaluate a single contrast.\n");
      return(1);
    }
  else if ( ntime != REM.ntime )
    { // Make sure that the number of time points is consistent with the number used to fit the REM.
      printf("Error: # time points (%d) != # used in REM fit (%d).\n",ntime,REM.ntime);
      return(1);
    }

  // include covariance noise
  CBeta = stdev = 0.;
  for (jcoeff=0; jcoeff<REM.ncoeff; jcoeff++)
    {
      CBeta += REM.contrast_matrix[lCondition][0][jcoeff] * REM.Beta_c[jcoeff];
      for (jcoeff1=0; jcoeff1<REM.ncoeff; jcoeff1++)
	stdev += REM.contrast_matrix[lCondition][0][jcoeff]
	  *  REM.Variance_cc[jcoeff][jcoeff1]
	  *  REM.contrast_matrix[lCondition][0][jcoeff1];
    }
  stdev = sqrt((double)stdev);

  // Set returned values
  *effect_size = CBeta;
  *effect_sdev = stdev;

  if ( stdev == 0. )
    { // This case must be constant values (e.g., zeros)
      *cnr = 0.;
      *p_raw = 1.;
      *p_cor = 1.;
    }
  else
    {
      df1 = lDOF_effective; // increased DOF after smoothing variance ratio
      df2 = 1.;             // 1 contrast

      *cnr = Tstat = *effect_size / *effect_sdev;
      Fstat = Tstat*Tstat;

      // Calculate EC3 following Worsley et al., HBM 1996
      ec3 = ec3constant * ( (df1-1.)/df1*Fstat - 1. ) / pow((double)(1.+Fstat/df1),(double)((df1-1.)/2.));
      *p_cor = (float)resels * ec3;

      *p_raw = ibeta((double)(0.5*df1),(double)(0.5*df2),(double)(df1/(df1+df2*Fstat)));
    }
  if ( *p_cor > 1. || *p_cor < 0. ) *p_cor = 1.;
  if ( *p_raw < too_small ) *p_raw = too_small;
  if ( *p_cor < too_small ) *p_cor = too_small;

  return(0);
}

int REM_calculate_F(int ntime,          // input: number of time points
		    JIP_INDEX resels,         // resolution elements (for multiple comparisons)
		    int lCondition,     // an index between 0 and REM.nconditions-1
		    int lDOF_effective, // effective DOF after regularizing RE variance
		    float *cnr,         // output cnr for the relevant condition
		    float *p_raw,       // output raw p-value for the relevant condition
		    float *p_cor,       // output corrected p-value for the relevant condition
		    float *effect_size, // size of effect
		    float *effect_sdev) // stardard deviation of effect
{
  float Fstat, df1, df2, numerator, ec3;
  float CBetaSum, CBetaMax, beta;
  float *CBeta, **FCovarInv;
  float too_small = exp(-100.), ec3constant=.1169;
  int lContrastMax, iRowFail;
  int jcoeff, jcoeff1, jc1, jc2;

  // Make sure the condition is within range.
  if ( lCondition < 0 || lCondition >= REM.nconditions )
    {
      printf("Error: the specified condition (%d) is out of range (0-%d)\n",
	     lCondition,REM.nconditions);
      return(1);
    }
  else if ( ntime != REM.ntime )
    { // Make sure that the number of time points is consistent with the number used to fit the REM.
      printf("Error: # time points (%d) != # used in REM fit (%d).\n",ntime,REM.ntime);
      return(1);
    }

  CBeta = float_vector(REM.ncontrasts[lCondition]);
  // Calculate the product CBeta
  for (jc1=0, lContrastMax=0, CBetaSum=CBetaMax=0.;
       jc1<REM.ncontrasts[lCondition];
       jc1++)
    {
      CBeta[jc1] = 0;
      for (jcoeff=0; jcoeff<REM.ncoeff; jcoeff++)
	CBeta[jc1] += REM.contrast_matrix[lCondition][jc1][jcoeff] * REM.Beta_c[jcoeff];
      CBetaSum += CBeta[jc1];
      if ( fabs((double)CBeta[jc1]) > fabs((double)CBetaMax) )
	{
	  CBetaMax = CBeta[jc1];
	  lContrastMax = jc1;
	}
    }
  // Determine the effect size.
  *effect_size = CBetaMax;

  FCovarInv = float_matrix(REM.ncontrasts[lCondition],REM.ncontrasts[lCondition]);
  for ( jc1=0; jc1<REM.ncontrasts[lCondition]; jc1++)
    {
      for ( jc2=0; jc2<REM.ncontrasts[lCondition]; jc2++)
	{
	  FCovarInv[jc1][jc2] = 0.;
	  for (jcoeff=0; jcoeff<REM.ncoeff; jcoeff++)
	    {
	      for (jcoeff1=0; jcoeff1<REM.ncoeff; jcoeff1++)
		{
		  FCovarInv[jc1][jc2] += REM.contrast_matrix[lCondition][jc1][jcoeff]
		    * REM.Variance_cc[jcoeff][jcoeff1] * REM.contrast_matrix[lCondition][jc2][jcoeff1];
		}
	    }
	}
    }
  // Now invert the matrix.
  rInvertSquareMatrix( REM.ncontrasts[lCondition], FCovarInv, &iRowFail );

  // Compute the numerator for F
  numerator = 0.;
  for (jc1=0; jc1<REM.ncontrasts[lCondition]; jc1++)
    {
      for (jc2=0; jc2<REM.ncontrasts[lCondition]; jc2++)
	numerator += CBeta[jc1] * FCovarInv[jc1][jc2] * CBeta[jc2];
    }

  // Free temporary vectors.
  free_float_vector(CBeta);
  free_float_matrix(FCovarInv,REM.ncontrasts[lCondition],REM.ncontrasts[lCondition]);

  // Degrees of freedom
  df1 = lDOF_effective;
  df2 = REM.ncontrasts[lCondition];

  Fstat = numerator / df2;
  *cnr = sqrt((double)Fstat);
  *effect_sdev = CBetaMax / *cnr;

  // Calculate EC3 following Worsley et al., HBM 1996
  ec3 = ec3constant * ( (df1-1.)/df1*Fstat - 1. ) / pow((double)(1.+Fstat/df1),(double)((df1-1.)/2.));
  *p_cor = (float)resels * ec3;

  if ( CBetaMax < 0. ) *cnr *= -1.;

  if ( CBetaMax == 0. )
    { // This case must be constant values (e.g., zeros)
      *cnr = 0.;
      *p_raw = 1.;
      *p_cor = 1.;
    }
  else
    {
      beta = ibeta((double)(0.5*df1),(double)(0.5*df2),(double)(df1/(df1+df2*Fstat)));
      if ( df2 == 1. )
	*p_raw = beta;
      else
	{
	  *p_raw = 2.*beta;
	  if ( *p_raw == 2. ) *p_raw = 1.;
	  if ( *p_raw > 1.0 ) *p_raw = 2. - *p_raw;
	}
    }
  if ( *p_cor > 1. || *p_cor < 0. ) *p_cor = 1.;
  if ( *p_raw < too_small ) *p_raw = too_small;
  if ( *p_cor < too_small ) *p_cor = too_small;
  
  return(0);
}
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
///////////// Functions that describe the REM ////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void REM_describe_input()
{
  int jevent, jstim, jfile;

  if ( !REM_input.overlapping_events )
    {
      for (jevent=0; jevent<REM.nevents; jevent++)
	{
	  if ( REM.event_shape[jevent] == SHAPE_SQUARE )
	    printf("\nSQUARE event %c has %d samples:\n",
		   REM_EventChar(REM.event_number[jevent]),REM.nsamples[jevent]);
	  else if ( REM.event_shape[jevent] == SHAPE_RAMP )
	    printf("\nRAMP event %c has %d samples:\n",
		   REM_EventChar(REM.event_number[jevent]),REM.nsamples[jevent]);
	  for (jfile=REM.event_start[jevent][0]; jfile<=REM.event_stop[jevent][0]; jfile++)
	    printf("file[%d] = %s , magnitude = %g\n",jfile-REM.event_start[jevent][0],
		   REM_input.signal_file_name[jfile],
		   REM_input.signal_magnitude[jfile]);
	}
    }
  else
    {
      for (jevent=0; jevent<REM.nevents; jevent++)
	{
	  if ( REM.event_shape[jevent] == SHAPE_SQUARE )
	    printf("\nSQUARE event %c has %d samples:\n",
		   REM_EventChar(REM.event_number[jevent]),REM.nsamples[jevent]);
	  else if ( REM.event_shape[jevent] == SHAPE_RAMP )
	    printf("\nRAMP event %c has %d samples:\n",
		   REM_EventChar(REM.event_number[jevent]),REM.nsamples[jevent]);
	  for (jstim=0; jstim<REM.nstims[jevent]; jstim++)
	    printf("%d %d   (ON, OFF)\n",REM.event_start[jevent][jstim],REM.event_stop[jevent][jstim]);
	}
      printf("measurement files:\n");
      for (jfile=0; jfile<REM.ntime; jfile++)
	printf("file[%d] = %s , magnitude = %g\n",
	       jfile,
	       REM_input.signal_file_name[jfile],
	       REM_input.signal_magnitude[jfile]);

    }
}

void REM_describe_basis_functions()
{
  FILE *plot_file;
  int jevent, jcoeff, jtime;

  /*
  ** print the basis functions
  */
  if ( !(plot_file = fopen("basis.dat","w")) )
    {
      printf("Cannot open basis function output file ...\n");
      return;
    }
  fprintf(plot_file,"x ");
  for (jevent=0; jevent<REM.nevents; jevent++)
    fprintf(plot_file,"event%c ",REM_EventChar(REM.event_number[jevent]));
  fprintf(plot_file,"\n");
  for (jtime=0; jtime<REM.ntime; jtime++)
    {
      fprintf(plot_file,"%d ",jtime);
      for (jcoeff=0; jcoeff<REM.ncoeff; jcoeff++)
	{
	  fprintf(plot_file,"%f ",REM.X_tc[jtime][jcoeff]);
	}
      fprintf(plot_file,"\n");
    }
  fclose(plot_file);
}

void REM_describe_fit()
{
  int jcoeff;

  for (jcoeff=0; jcoeff<REM.ncoeff; jcoeff++)
    printf("amplitude of basis function %d = %f\n",jcoeff,REM.Beta_c[jcoeff]);
}

int REM_get_condition_string( int ncondition, char *condition_string )
{
  if ( ncondition < 0 || ncondition >= REM.nconditions )
    return(1);
  else
    strcpy(condition_string,REM.condition_string[ncondition]);
  return(0);
}

int REM_is_initialized()
{
  return(REM.initialized);
}

int REM_get_nconditions()
{
  return(REM.nconditions);
}

int REM_get_number_events()
{
  return(REM.nevents);
}

int REM_get_event_number(int index_event)
{
  if ( index_event < REM.nevents )
    return(REM.event_number[index_event]);
  else
    return(0);
}

int REM_get_event_index(int event_number)
{
  int jevent, found, index_event;

  // Check to see if the event exists.
  for ( jevent=0, found=0; jevent<REM.nevents; jevent++ )
    {
      if ( REM.event_number[jevent] == event_number )
	{
	  found = 1;
	  index_event = jevent;
	}
    }
  if ( found )
    return(index_event);
  else
    return(-1);
}

int REM_event_fit(int index_event, int npoints, float *fit)
{
  int jtime;

  if ( index_event < 0 || index_event >= REM.nevents )
    {
      printf("Error: event index %d is out of bounds in function REM_event_fit\n",index_event);
      return(1);
    }
  else if ( npoints != REM.ntime )
    {
      printf("Error: # points (%d) does not match REM (%d) in function REM_event_fit\n",
	     npoints,REM.ntime);
      return(1);
    }
  // Fill the fit vector
  for (jtime=0; jtime<REM.ntime; jtime++)
    fit[jtime] = REM.X_tc[jtime][index_event] * REM.Beta_c[index_event] ;

  return(0);
}

float REM_get_maximal_effect_size(int lCondition )
{ // index_coeff should be the same as index_event
  int jtime, jc, jcoeff;
  float *CBeta, y, max;

  CBeta = float_vector(REM.ncontrasts[lCondition]);
  // Loop over time points, and find the maximal value for given condition.
  for (jtime=0, max=0.; jtime<REM.ntime; jtime++)
    {
      // Calculate the product CBeta for the given condition.
      for (jc=0; jc<REM.ncontrasts[lCondition]; jc++)
	{
	  // Find C*Beta
	  CBeta[jc] = 0;
	  for (jcoeff=0; jcoeff<REM.ncoeff; jcoeff++)
	    CBeta[jc] += REM.contrast_matrix[lCondition][jc][jcoeff] * REM.Beta_c[jcoeff];
	}
      // Now find the maximum value of y = X * C*Beta
      for (jc=0; jc<REM.ncontrasts[lCondition]; jc++)
	{
	  y = REM.X_tc[jtime][jc] * CBeta[jc];
	  if ( fabs((double)y) > fabs((double)max) ) max = y;
	}
    }
  free_float_vector(CBeta);
  return(max);
}

int REM_get_number_time_points()
{
  return(REM.ntime);
}

int REM_get_DOF()
{
  int DOF, jtime;

  DOF = 0;
  for (jtime=0; jtime<REM.ntime; jtime++)
    DOF += 1;
  DOF -= REM.ncoeff;
  return(DOF);
}

int REM_EventNumberFromChar(char event_char)
     // character (in)  event integer (out)  ascii
     // -------------   -------------------  -----
     //      1-9              1-9            49-57
     //      A-Z             36-61           65-90
     //      a-z             10-35           97-122
     //
     // error returns 0
{
  int event_number;

  event_number = event_char;  // convert character to integer ascii code
  if ( (event_number<49 && event_number!=44) ||
       (event_number>57 && event_number<65)  ||
       (event_number>90 && event_number<97)  ||
       (event_number>122) )
    return(0);

  if ( event_number == 44 )   // ',' is special case
    event_number = -1;
  else if ( event_number >= 49 && event_number <= 57 )      // 1-9 characters
    event_number -= 48;  // numbers 1-9   (subtract 49, add 1)
  else if ( event_number >= 97 && event_number <= 122 ) // a-z characters
    event_number -= 87;  // numbers 10-35 (subtract 97, add 10)
  else if ( event_number >= 65 && event_number <= 90 )  // A-Z characters
    event_number -= 29;  // numbers 10-35 (subtract 65, add 36)
  return(event_number);
}

int REM_EventCharFromInteger(int event_number, char *event_char)
{
  int ascii_code;

  if ( event_number>=1 && event_number<=9 )
    ascii_code = event_number + 48;  // add 49-1
  else if ( event_number>=10 && event_number<=35 )
    ascii_code = event_number + 87;  // add 97-10
  else if ( event_number>=36 && event_number<=61 )
    ascii_code = event_number + 29;  // add 65-36
  else
    return(1);
  *event_char = ascii_code;
  return(0);
}

char REM_EventChar(int event_number)
{
  char event_char;

  if ( REM_EventCharFromInteger(event_number, &event_char) )
    {
      printf("Programming error: event number %d apparently is not defined!\n",event_number);
      exit(1);
    }
  return event_char;
}

int REM_EventStringFromInteger(int event_number, char *event_string)
{
  char event_char, event_string_temp[LEN_STR];

  if ( REM_EventCharFromInteger(event_number,&event_char) )
    {
      printf("Error: event %d is not defined.\n",event_number);
      return(1);
    }
  strcpy(event_string_temp," ");
  event_string_temp[0] = event_char;
  strcpy(event_string,event_string_temp);
  return(0);
}

IPOINT2D REM_GetEventLimits( int event_number )
{
  IPOINT2D limits;
  int ievent_index;

  ievent_index = REM_get_event_index(event_number);
  limits.x = REM.event_start[ievent_index][0];
  limits.y = REM.event_stop[ievent_index][REM.nstims[ievent_index]-1];
  return(limits);
}

int REM_IdentifyEvent( int iEntry, char *event_char, int *nSample )
// return: number of found events at this location
{
  int found;
  int jevent, jstim, ievent_index;

  if ( iEntry < 0 || iEntry >= REM.ntime )
    {
      printf("Error: entry %d is invalid in function REM_IdentifyEvent\n",iEntry);
      return(1);
    }

  for (jevent=found=0; jevent<REM.nevents; jevent++)
    {
      for (jstim=0; jstim<REM.nstims[jevent]; jstim++)
	{
	  if ( iEntry >= REM.event_start[jevent][jstim] && iEntry <= REM.event_stop[jevent][jstim] )
	    {
	      found++;
	      *event_char = REM_EventChar(REM.event_number[jevent]);
	      ievent_index = jevent;
	    }
	}
    }
  if ( found >= 1 )
    {
      *nSample = iEntry - REM.event_start[ievent_index][0];
      if ( found > 1 )
	*event_char = '?';
    }
  else
    {
      *nSample = -1;
      *event_char = '?';
    }
  return(found);
}
