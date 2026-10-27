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
/*
** See the following include file for the definitions of the
** GLM and HRF data structures.  For data analysis, there is
** no need to include glm.h into the calling program, as the
** functions in this file should handle everything without the
** calling program needing to know details.  The include file
** below has been separated from this file for purposes of simulations,
** which may want to set timing values in the GLM structure directly.
*/
#include "glm.h"
//#define RAND_MAX 2147483647.
#define RAND (rand()/RAND_MAX) // a number from 0 to 1

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
///////// Functions that set up the GLM based upon file input ////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

int GLM_read_control_file( char *file_name, int notify )
{
  FILE *file;
  char whole_line[LEN_STR], string[LEN_STR];
  char *ptr;
  int nline, i1, ivalue;
  int jevent, jstim, jstim1, jrun, jcolumn, jevent_number, event_number, found;
  BOOL output_info;

  // Initialize the glm input information.
  GLM_input.nbaseline_terms = 3;
  GLM_input.normalize_runs = 0;
  GLM_input.AR_order = 0;
  GLM_input.AR_smooth_mm = 1.;
  GLM_input.nifti_output = FALSE;
  GLM_input.multiple_comparisons = 0;
  strcpy(GLM_input.HRF_file_name,"");
  strcpy(GLM_input.condition_string,"");

  // Initialize basic GLM information.
  GLM.nevents = GLM.nruns = GLM.ntime = GLM.ncoeff = 0;
  GLM.nbaseline = GLM.ntable_events = GLM.nexclude = 0;
  GLM.initialized = 0;
  GLM.AR_order = 0;

  // per event number
  for (jevent_number=0; jevent_number<MAX_EVENTS; jevent_number++)
    {
      GLM.convolve_table[jevent_number]      = 0; // index is event identifier
      GLM.drift_correct_table[jevent_number] = -1;
      GLM.event_scale[jevent_number] = 1.;
    }

  // per event
  for (jevent=0; jevent<MAX_EVENTS; jevent++)
    {
      GLM.nstims[jevent]     = GLM.event_number[jevent] = GLM.event_shape[jevent] = 0;
      GLM.event_tau1[jevent] = GLM.event_tau2[jevent]   = 0.;
      // drug-IRF vector
      GLM.length_drugIRF[jevent] = 0;
      if ( GLM.drugIRF[jevent]   != NULL ) free_float_vector(GLM.drugIRF[jevent]);
      if ( GLM.plasmaIRF[jevent] != NULL ) free_float_vector(GLM.plasmaIRF[jevent]);
      // per event, stim
      for (jstim=0; jstim<MAX_STIMS; jstim++)
	{
	  GLM.stim_on[jevent][jstim] = GLM.stim_off[jevent][jstim] = 0.;
	  GLM.stim_mag[jevent][jstim] = 0.;
	  GLM.table_icolumn[jevent][jstim] = 0;
	  GLM.table_icolumn[jevent][jstim] = 0;
	}
    }
  // per stimulus
  for (jstim=0; jstim<MAX_STIMS; jstim++)
    GLM.exclude_start[jstim] = GLM.exclude_end[jstim] = 0;
  // per run
  for (jrun=0; jrun<MAX_RUNS; jrun++)
    {
      GLM.table_ncolumns[jrun] = GLM.table_ntime[jrun] = 0;
      if ( GLM.table_value[jrun] != NULL )
	free_float_matrix(GLM.table_value[jrun],GLM.table_ncolumns[jrun],GLM.table_ntime[jrun]);
      else
	GLM.table_value[jrun] = NULL;
    }
  // per column
  for (jcolumn=0; jcolumn<MAX_COLUMNS; jcolumn++)
    strcpy(GLM.table_column_name[jcolumn],"undefined");

  // Open the file containing the list of runs, GLM files, ...
  if ( !(file = fopen(file_name,"r")) )
    {
      printf("Error: cannot open the file %s.\n",file_name);
      return(1);
    }

  nline = 1;
  ivalue = 0;
  while ( get_line_with_comment(file,whole_line,LEN_STR) )
    {
      if ( sscanf(whole_line,"%d",&i1) < 0 )  /* ... then this is a blank line */
	continue;
      if ( (ivalue=GLMInternal_decode_keyword(whole_line)) < 0 )
	{
	  printf("Error reading line %d of file '%s':\n",nline,file_name);
	  GLMInternal_describe_glm_format();
	  fclose(file);
	  return(1);
	}
      else if ( ivalue == 1 )
	break;
      nline++;
    }

  // Read the IRF file here.
  output_info = notify || GLM_input.notify;
  if ( GLM_read_hrf_file(output_info, GLM_input.HRF_file_name) ) return(1);

  // The last command was "runs", so begin to read the following lines as
  //   [run file name] [glm timing file name]
  GLM_input.nruns = 0;
  while ( get_line_with_comment(file, whole_line, LEN_STR) )
    {
      if ( sscanf(whole_line,"%d",&i1) < 0 )  /* ... then this is a blank line */
	continue;
      /* This line should contain a bshort and glm file name. */
      /* Read the bshort file name. */
      if ( sscanf(whole_line,"%s",string) != 1 )
	{
	  printf("Error reading the bshort file name for run %d.\n",GLM_input.nruns+1);
	  fclose(file);
	  return(1);
	}
      strcpy(GLM_input.run_name[GLM_input.nruns],string);

      // Update the argument to the next string position (GLM file name).
      ptr = next_non_white( whole_line + jip_int_count((JIP_INDEX)strlen(string)) );
      /* Read the glm file name. */
      if ( sscanf(ptr,"%s",string) != 1 )
	{
	  printf("Error reading the GLM file name for run %d.\n",GLM_input.nruns+1);
	  fclose(file);
	  return(1);
	}
      strcpy(GLM_input.glm_name[GLM_input.nruns],string);

      // Update to the next string (table file name).
      ptr = next_non_white( ptr + jip_int_count((JIP_INDEX)strlen(string)) );
      // Read the table file name.
      if ( sscanf(ptr,"%s",string) != 1 )
	strcpy(GLM_input.table_name[GLM_input.nruns],"");
      else
	strcpy(GLM_input.table_name[GLM_input.nruns],string);

      // Read the table file, if one is given.
      if ( GLM_read_table_file(GLM_input.table_name[GLM_input.nruns], GLM_input.nruns) )
	return(1);
      // Read the GLM file.
      if ( GLM_read_event_file(GLM_input.glm_name[GLM_input.nruns], GLM_input.nruns) )
	return(1);

      GLM_input.nruns++;
      GLM.nruns++;
    }
  fclose(file);

  // Before describing the stimuli, take care of a special case.
  // When the start of one stimulus exactly equals the stop of a previous stimulus,
  // decrement the stop time of the earlier stimulus by one HRF step.
  // This avoids the problem of overlapping stimuli, so the convolution works correctly.
  if ( HRF.nhrf != 0 )
    {
      for (jevent=0; jevent<GLM.nevents; jevent++)
	{
	  if ( GLM.event_shape[jevent] == SHAPE_SQUARE   ||
	       GLM.event_shape[jevent] == SHAPE_RAMPUP   ||
	       GLM.event_shape[jevent] == SHAPE_RAMPDOWN ||
	       GLM.event_shape[jevent] == SHAPE_DRUGIRF  )
	    {
	      for (jstim=0; jstim<GLM.nstims[jevent]; jstim++)
		{
		  for (jstim1=0; jstim1<GLM.nstims[jevent]; jstim1++)
		    {
		      if ( GLM.stim_on[jevent][jstim] == GLM.stim_off[jevent][jstim1] )
			GLM.stim_off[jevent][jstim1] -= HRF.time_step;
		    }
		}
	    }
	}
    }
  if ( output_info ) GLM_describe_stimuli();

  // Make sure that if a convolution flag was set for a table event, the event is actually a table.
  for (jevent_number=0; jevent_number<GLM.nevents; jevent_number++)
    {
      event_number = GLM.event_number[jevent_number];
      if ( GLM.convolve_table[event_number] == 1 )
	{ // Then make sure the event exists and it is a table.
	  for (jevent=0, found=0; jevent<GLM.nevents; jevent++)
	    {
	      if ( GLM.event_number[jevent] == event_number )
		{
		  found = 1;
		  if ( GLM.event_shape[jevent] != SHAPE_TABLE )
		    {
		      printf("Error: Convolution for table event %c was requested\n",
			     GLM_EventChar(event_number));
		      printf("using `convolve-table', but this event is not a table.\n");
		      return(1);
		    }
		}
	    }
	  if ( !found )
	    {
	      printf("Error: Convolution for table event %c was requested\n",
		     GLM_EventChar(event_number));
	      printf("using `convolve-table', but this event is not defined.\n");
	      return(1);
	    }
	}
    }

  GLM.initialized = 1;
  return(0);
}

int GLMInternal_decode_keyword( char *line )
{
  char keyword[LEN_STR], string[LEN_STR], *argument, c1;
  float scale_factor;
  int event_number, jevent_number, nterms;

  sscanf(line,"%s",keyword);
  argument = next_non_white( line + jip_int_count((JIP_INDEX)strlen(keyword)) );

  if ( !strcmp(keyword,"baseline-terms") )
    {
      if ( sscanf(argument,"%d",&(GLM_input.nbaseline_terms)) != 1 )
	{
	  printf("Error: `baseline-terms' requires an argument (1,2,3).\n");
	  return(-1);
	}
      if ( GLM_input.nbaseline_terms < 0 || GLM_input.nbaseline_terms > 4 )
	{
	  printf("Error: allowed value for `baseline-terms' are 0-4.\n");
	  return(-1);
	}
      // Attach this number of terms to all table events
      for (jevent_number=0; jevent_number<MAX_EVENTS; jevent_number++)
	{
	  if ( GLM.drift_correct_table[jevent_number] == -1 ) // -1 => undefined
	    GLM.drift_correct_table[jevent_number] = GLM_input.nbaseline_terms;
	}
      return(0);
    }
  else if ( !strcmp(keyword,"scale-event") )
    {
      if ( sscanf(argument,"%f %s",&scale_factor,string) != 2 )
	{
	  printf("Error: use `scale-event [scale] [event].\n");
	  return(-1);
	}
      c1 = string[0];
      event_number = GLM_EventNumberFromChar(c1);
      GLM.event_scale[event_number] = scale_factor;
      return(0);
    }
  else if ( !strcmp(keyword,"AR") )
    {
      if ( sscanf(argument,"%d %f",&(GLM_input.AR_order),&(GLM_input.AR_smooth_mm)) != 2 )
	{
	  GLM_input.AR_smooth_mm = 1.;
	  if ( sscanf(argument,"%d",&(GLM_input.AR_order)) != 1 )
	    {
	      printf("Error: `AR' requires an integer argument.\n");
	      return(-1);
	    }
	}
      return(0);
    }
  else if ( !strcmp(keyword,"multiple-comparisons") )
    {
      if ( sscanf(argument,"%d",&(GLM_input.multiple_comparisons)) != 1 )
	{
	  printf("Error: `multiple-comparisons' requires an integer argument.\n");
	  return(-1);
	}
      return(0);
    }
  else if ( !strcmp(keyword,"normalize-BOLD") )
    {
      GLM_input.normalize_runs = 1;
      return(0);
    }
  else if ( !strcmp(keyword,"normalize-IRON") )
    {
      GLM_input.normalize_runs = 2;
      return(0);
    }
  else if ( !strcmp(keyword,"notify") )
    {
      GLM_input.notify = 1;
      return(0);
    }
  else if ( !strcmp(keyword,"output-format") )
    {
      sscanf(argument,"%s",string);
      if ( !strcmp(string,"nifti") || !strcmp(string,"NIFTI") )
	GLM_input.nifti_output = TRUE;
      return(0);
    }
  else if ( !strcmp(keyword,"IRF") )
    {
      sscanf(argument,"%s",GLM_input.HRF_file_name);
      return(0);
    }
  else if ( !strcmp(keyword,"HRF-file") )
    {
      sscanf(argument,"%s",GLM_input.HRF_file_name);
      return(0);
    }
  else if ( !strcmp(keyword,"conditions") )
    {
      strcpy(GLM_input.condition_string,argument);
      return(0);
    }
  else if ( !strcmp(keyword,"convolve-table") )
    {
      while ( *argument )
	{
	  if ( *argument > 32 ) // not space, tab, carriage return, ...
	    {
	      event_number = GLM_EventNumberFromChar(*argument);
	      if ( event_number == 0 )
		{
		  printf("Error using convolve-table: event %c is undefined.\n",*argument);
		  return(1);
		}
	      else // set the convolution flag for the table event 
		GLM.convolve_table[event_number] = 1;
	    }
	  argument++;
	}
      return(0);
    }
  else if ( !strcmp(keyword,"drift-correct") )
    {
      if ( sscanf(argument,"%d",&nterms) != 1 )
	{
	  printf("Error: `drift-correct' requires an integer argument, followed by event IDs.\n");
	  return(-1);
	}
      argument++;  // nterms should be a single character
      while ( *argument )
	{
	  if ( *argument > 32 ) // not space, tab, carriage return, ...
	    {
	      event_number = GLM_EventNumberFromChar(*argument);
	      if ( event_number == 0 )
		{
		  printf("Error using drift-correct: event %c is undefined.\n",*argument);
		  return(1);
		}
	      else // set the drift flag for the table event 
		{
		  GLM.drift_correct_table[event_number] = nterms;
		}
	    }
	  argument++;
	}
      return(0);
    }
  else if ( !strcmp(keyword,"runs:") || !strcmp(keyword,"runs") )
    return(1);
  else // not found
    return(-1);
}

void GLMInternal_describe_glm_format()
{ // This is printed as the results of input error
  printf("\nExample glm input file:\n\n");
  printf("notify                # optional; turn on output information\n");
  printf("baseline-terms 3      # optional; allowed options are 1-4 (default = 2)\n");
  printf("normalize-BOLD        # optional; normalize raw signal for BOLD\n");
  printf("IRF BOLD.irf          # optional; use an I.R.F. for F.I.R. analyses\n");
  printf("convolve-table 8 9    # optional; convolve table events 8 & 9 with IRF\n");
  printf("conditions 1 2 12 1-2 # mandatory; Statistical tests MUST be included\n");
  printf("runs:                 # mandatory; This means that runs follow.\n");
  printf("../fMRI-run1.bshort run1.glm table1.dat # start listing runs, tables optional\n");
  printf("../fMRI-run2.bshort run2.glm            # ... and so on ...\n\n");
}

void GLMInternal_describe_event_format( int event_shape )
{ // This is printed as the results of input error
  if ( event_shape == SHAPE_SQUARE )
    {
      printf("\nExample of definition for square events:\n");
      printf("1 square    # [event identifier] [optional event shape]\n");
      printf("30 60       # [ON time] [OFF time] [optional magnitude]\n");
      printf("90 120      # [ON time] [OFF time] [optional magnitude]\n");
      printf("...         # ... and so on\n");
    }
  else if ( event_shape == SHAPE_RAMPUP )
    {
      printf("\nExample of definition for ramp events:\n");
      printf("1 ramp-up   # [event identifier] [optional event shape]\n");
      printf("30 60       # [ON time] [OFF time] [optional magnitude]\n");
      printf("90 120      # [ON time] [OFF time] [optional magnitude]\n");
      printf("...         # ... and so on\n");
    }
  else if ( event_shape == SHAPE_RAMPDOWN )
    {
      printf("\nExample of definition for ramp events:\n");
      printf("1 ramp-down # [event identifier] [optional event shape]\n");
      printf("30 60       # [ON time] [OFF time] [optional magnitude]\n");
      printf("90 120      # [ON time] [OFF time] [optional magnitude]\n");
      printf("...         # ... and so on\n");
    }
  else if ( event_shape == SHAPE_GAMMA )
    {
      printf("\nExample of definition for gamma events:\n");
      printf("1 gamma 24. # [event identifier] gamma [time constant for gamma]\n");
      printf("30 1.       # [ON time] [optional magnitude]\n");
      printf("90 2.       # [ON time] [optional magnitude]\n");
      printf("...         # ... and so on\n");
    }
  else if ( event_shape == SHAPE_SIGMOID )
    {
      printf("\nExample of definition for sigmoid events:\n");
      printf("1 sigmoid 24. # [event identifier] sigmoid [time constant for sigmoid]\n");
      printf("30 1.         # [ON time] [optional magnitude]\n");
      printf("90 2.         # [ON time] [optional magnitude]\n");
      printf("...           # ... and so on\n");
    }
  else if ( event_shape == SHAPE_DRUGIRF )
    {
      printf("\nExample of definition for drug-IRF events:\n");
      printf("1 drug-IRF 24. # [event identifier] drug-IRF [tau1] [optional tau2]\n");
      printf("30 60       # [ON time] [OFF time] [optional magnitude]\n");
      printf("90 120      # [ON time] [OFF time] [optional magnitude]\n");
      printf("...         # ... and so on\n");
    }
  else if ( event_shape == SHAPE_TABLE )
    {
      printf("\nExample of definition for table events:\n");
      printf("1 table 2  # [event identifier] table [column # in table]\n");
    }
  else if ( event_shape == SHAPE_SINE )
    {
      printf("\nExample of definition for sinusoidal events:\n");
      printf("1 sine 30  # [event identifier] sine [period]\n");
      printf("This is usually used in conjunction with a cosine event.\n");
    }
  else if ( event_shape == SHAPE_COSINE )
    {
      printf("\nExample of definition for sinusoidal events:\n");
      printf("1 cosine 30  # [event identifier] sine [period]\n");
      printf("This is usually used in conjunction with a sine event.\n");
    }
  else if ( event_shape == SHAPE_INTERACT )
    {
      printf("\nExample of definition for interaction events (product of 2 events):\n");
      printf("I interaction 1 2  # [event identifier] interaction [event1] [event2]\n");
    }
}

int GLM_read_hrf_file( int notify, char *file_name )
     // notify: 0 = no output, 1 = print output about IRF
{
  FILE *file;
  char line[LEN_STR], irf_type[LEN_STR];
  static int first_time = 1;
  int j, jexp;
  float t;

  strcpy(HRF.file_name,file_name);
  if ( !strcmp(file_name,"") || file_name == NULL )
    { // If the file name is NULL, ignore it.
      HRF.nhrf = 0;
      return(0);
    }

  if ( ! first_time )
    {
      free_float_vector(HRF.tau);
      free_float_vector(HRF.weight);
      free_float_vector(HRF.amp);
    }
  else
    first_time = 0;

  // Define the HRF to be biased.
  HRF.unbiased = 0;
  HRF.nhrf = 1;
  HRF.nexp = 3;   // fix this value: 1=delay, 2=fast, 3=slow
  HRF.tau    = float_vector(HRF.nexp);
  HRF.weight = float_vector(HRF.nexp);

  strcpy(irf_type,"exponential"); // for now, fix this to exponential

  if ( !strcmp(file_name,"BOLD-monkey") )
    { 
      HRF.time_length = 120.;
      HRF.time_step   = 0.1;
      HRF.tau[0]      = 1.5;
      HRF.tau[1]      = 4.5;
      HRF.tau[2]      = 13.5;
      HRF.weight[1]   = 0.70;
      HRF.weight[2]   = -0.30;
      /*
      HRF.weight[1]   = 3.09;
      HRF.weight[2]   = -1.20;
      */
    }
  else if ( !strcmp(file_name,"IRON-monkey") )
    {
      HRF.time_length = 120.;
      HRF.time_step   = 0.1;
      HRF.tau[0]      = 1.5;
      HRF.tau[1]      = 4.5;
      HRF.tau[2]      = 13.5;
      HRF.weight[1]   = 0.40;
      HRF.weight[2]   = 0.80;
    }
  else if ( !strcmp(file_name,"BOLD-rat") )
    { // not necessarily optimized (uses monkey time constants except for #1)
      HRF.time_length = 120.;
      HRF.time_step   = 0.1;
      HRF.tau[0]      = 0.5;
      HRF.tau[1]      = 2.5;
      HRF.tau[2]      = 13.5;
      HRF.weight[1]   = 0.7;
      HRF.weight[2]   = -0.3;
    }
  else if ( !strcmp(file_name,"IRON-rat") )
    { // not necessarily optimized (uses monkey time constants except for #1)
      HRF.time_length = 120.;
      HRF.time_step   = 0.1;
      HRF.tau[0]      = 0.5;
      HRF.tau[1]      = 2.5;
      HRF.tau[2]      = 13.5;
      HRF.weight[1]   = 0.40;
      HRF.weight[2]   = 0.80;
    }
  else
    {
      // Open the IRF file.
      if ( !(file = fopen(file_name,"r")) )
	{
	  printf("Error attempting to open the IRF file %s.\n",file_name);
	  return(1);
	}
      // Read the duration and the time step for the HRF.
      if ( !get_line_with_comment(file, line, LEN_STR) ) return(1);
      if ( sscanf(line,"%f %f %f",&(HRF.time_length),&(HRF.time_step),&(HRF.tau[0])) != 3 )
	{
	  printf("Error reading the 1st line of the IRF file %s.\n",file_name);
	  fclose(file);
	  return(1);
	}
      // Read the fast and slow time constants and weights.
      if ( !get_line_with_comment(file, line, LEN_STR) ) return(1);
      if ( sscanf(line,"%f %f",&(HRF.tau[1]),&(HRF.tau[2])) != 2 )
	{
	  printf("Error reading the 2nd line of the IRF file %s.\n",file_name);
	  fclose(file);
	  return(1);
	}
      if ( !get_line_with_comment(file, line, LEN_STR) ) return(1);
      if ( sscanf(line,"%f %f",&(HRF.weight[1]),&(HRF.weight[2])) != 2 )
	{
	  printf("Error reading the 3rd line of the IRF file %s.\n",file_name);
	  fclose(file);
	  return(1);
	}
      fclose(file);
    }
  // Compute the weight for the delay exponential by enforcing h(t=0) = 0.
  if ( HRF.tau[0] != 0. && HRF.tau[1] != 0. && HRF.tau[2] != 0. )
    HRF.weight[0] = -HRF.tau[0] * ( HRF.weight[1]/HRF.tau[1] + HRF.weight[2]/HRF.tau[2] );
  else
    HRF.weight[0] = 0.;

  // Allocate memory.
  HRF.npoints = HRF.time_length/HRF.time_step + 0.5;
  HRF.amp = float_vector(HRF.npoints);

  // Create a properly amplitude-weighted sum of exponentials.
  // Put the amplitude into HRF.amp_hrf
  for (j=0; j<HRF.npoints; j++)
    {
      t = HRF.time_step * j;
      HRF.amp[j] = 0.;
      for ( jexp=0; jexp<HRF.nexp; jexp++ )
	{
	  if ( HRF.tau[jexp] != 0. )
	    {
	      if ( !strcmp("exponential",irf_type) )
		HRF.amp[j] += HRF.time_step*HRF.weight[jexp]/HRF.tau[jexp]*exp(-(double)(t/HRF.tau[jexp]));
	      else
		HRF.amp[j] += HRF.time_step*HRF.weight[jexp]*t/HRF.tau[jexp]*exp(-(double)(t/HRF.tau[jexp]));
	    }
	}
    }

  // Output information about the HRF.
  if ( notify )
    {
      printf("\nHRF type   = %s\n",irf_type);
      printf("IRF length = %g seconds, resolution = %g seconds\n",HRF.time_length,HRF.time_step);
      for (j=0; j<HRF.nexp; j++)
	printf("Amplitude of exponential of %g seconds = %g\n",HRF.tau[j],HRF.weight[j]);
      printf("\n");
      // output a file showing the IRF
      // Open the IRF file.
      if ( !(file = fopen("HRF.dat","w")) )
	{
	  printf("Error attempting to open the hemodyamic response file (output) HRF.dat\n");
	  return(1);
	}
      else
	{
	  fprintf(file,"x y\n");
	  for (j=0; j<HRF.npoints; j++)
	    {
	      t = HRF.time_step * j;
	      fprintf(file,"%g %g\n",t,HRF.amp[j]);
	    }
	  fclose(file);
	}
    }
  else
    printf("using HRF (hemodynamic response function)\n");

  return(0);
}

int GLM_read_event_file( char *file_name, int irun )
{
  FILE *file;
  int ntime, nevent, nstim, i1, iline, nread, found, icolumn, istim;
  int event_number, ievent_index, itau1, itau2;
  int jevent, j;
  char line[LEN_STR], event_shape_string[LEN_STR], event_string[LEN_STR], event_char;
  char tau1_string[LEN_STR], tau2_string[LEN_STR], tau1_char, tau2_char;
  float tr, duration, stim_on, stim_off, stim_mag, tau1, tau2, t;
  int ishape;
  BOOL found1, found2;
  
  //
  // Open the GLM file.
  //
  if ( !(file = fopen(file_name,"r")) )
    {
      printf("Error attempting to open the GLM file %s.\n",file_name);
      return(1);
    }
  //
  // The first line in the file must contain 2 floats (TR, run length).
  //
  iline = 1;
  if ( !get_line_with_comment(file, line, LEN_STR) )
    {
      printf("Error reading the 1st line of GLM file %s\n",file_name);
      return(1);
    }
  if ( sscanf(line,"%f %f",&tr,&duration) != 2 )
    {
      printf("Error reading line %d of the GLM file %s (TR, run duration).\n",
	     iline,file_name);
      return(1);
    }
  iline++;
  
  if ( irun != 0 )
    {
      if ( tr != GLM.tr[0] )
	{
	  printf("Error: TR (%f) is not the same as the first run (%f).\n",
		 tr,GLM.tr[0]);
	  exit(1);
	}
    }
  ntime = rint((double)(duration/tr));
  GLM.tr[irun] = tr;

  // Increment the run counter & duration, and store the start & stop time.
  GLM.ntime += ntime;
  // This run stops at the last time point.
  GLM.run_stop[irun] = GLM.ntime-1;
  if ( irun == 0 )
    GLM.run_start[0] = 0;
  else  // This run starts at the end of the last run, + 1 time point
    GLM.run_start[irun] = GLM.run_stop[irun-1] + 1;

  //
  // Loop until the end of file (EOF) is reached.
  // Blank lines indicate a new stimulus event.
  //
  nstim = nevent = 0;  // These are counters local to the current file (not the appended GLM).
  while ( get_line_with_comment(file,line,LEN_STR) )
    {
      if ( sscanf(line,"%d",&i1) < 0 )  // ... then this is a blank line
	{
	  // The next line must be blank or contain a stimulus event identifier.
	  while ( get_line_with_comment(file,line,LEN_STR) )
	    {
	      // The next line can contain up to 4 entries.
	      // This line must 1st specify the event identifier and shape, plus
	      // 1) for shape `gamma' or `sigmoid', tau1 for gamma/sigmoid functions
	      // 2) for shape `drug-IRF', tau1 for brain (mandatory) and tau2 for plasma (optional)
	      // 3) for shape `table', column number (mandatory) and convolve flag (optional)
	      nread = sscanf(line,"%s %s %s %s",event_string,event_shape_string,tau1_string,tau2_string);
	      sscanf(tau1_string,"%f",&tau1);
	      sscanf(tau2_string,"%f",&tau2);
	      sscanf(event_string,"%c",&event_char);
	      // Convert the ascii code to an event number
	      if ( event_char == '-' )
		event_number = -1;  // this is not a real event, but an exclusion region
	      else if ( !(event_number=GLM_EventNumberFromChar(event_char)) )
		{
		  printf("Event number (%d) for event (%c) is not allowed on line %d in file %s\n",
			 event_number,event_char,iline+1,file_name);
		  return(1);
		}
	      iline++;
	      ishape = -1;
	      if ( nread < 0 )
		continue; // This is a blank line
	      else if ( nread == 1 )
		ishape = SHAPE_SQUARE;
	      else if ( nread >= 2 ) // At least the event number and shape were read.
		{
		  if ( !strcmp("square",event_shape_string) )
		    ishape = SHAPE_SQUARE;
		  else if ( !strcmp("ramp-up",event_shape_string) )
		    ishape = SHAPE_RAMPUP;
		  else if ( !strcmp("ramp-down",event_shape_string) )
		    ishape = SHAPE_RAMPDOWN;
		  else if ( !strcmp("gamma",event_shape_string) || !strcmp("sigmoid",event_shape_string) )
		    {
		      if ( !strcmp("gamma",event_shape_string) )
			ishape = SHAPE_GAMMA;
		      else if ( !strcmp("sigmoid",event_shape_string) )
			ishape = SHAPE_SIGMOID;
		      if ( nread != 3 )
			{
			  printf("Error on line %d for event type %d in file %s\n",
				 iline,event_number,file_name);
			  printf("For gamma & sigmoid shapes, specify the time constant.\n");
			  GLMInternal_describe_event_format( ishape );
			  return(1);
			}
		      // Make sure the time constant is >= 0.
		      if ( tau1 <= 0. )
			{
			  printf("Error reading stim %d of event %c:\n",nstim,event_char);
			  printf("Gamma & sigmoid time constants (%f) must be > 0\n",tau1);
			  GLMInternal_describe_event_format( ishape );
			  return(1);
			}
		    }
		  else if ( !strcmp("sine",event_shape_string) ||
			    !strcmp("cosine",event_shape_string) )
		    {
		      if ( nread != 3 )
			{
			  printf("Error on line %d for event type %d in file %s, # words read = %d\n",
				 iline,event_number,file_name,nread);
			  printf("For sine or cosine shapes, specify the period.\n");
			  GLMInternal_describe_event_format( ishape );
			  return(1);
			}
		      if ( !strcmp("sine",event_shape_string) )
			ishape = SHAPE_SINE;
		      else
			ishape = SHAPE_COSINE;
		      // Make sure the time constant is >= 0.
		      if ( tau1 <= 0. )
			{
			  printf("Error reading stim %d of event %c:\n",nstim,event_char);
			  printf("Sine and cosine events must have a period (%g) must be > 0\n",tau1);
			  GLMInternal_describe_event_format( ishape );
			  return(1);
			}
		    }
		  else if ( !strcmp("drug-IRF",event_shape_string) )
		    {
		      if ( nread < 3 )
			{
			  printf("Error on line %d for event type %d in file %s\n",
				 iline,event_number,file_name);
			  printf("Error: for drug-IRF shape, specify the convolution time constant.\n");
			  GLMInternal_describe_event_format( SHAPE_DRUGIRF );
			  return(1);
			}
		      else if ( nread == 3 )
			tau2 = 0.;
		      if ( tau1 < 0. )
			{
			  printf("Error reading stim %d of event %c:\n",nstim,event_char);
			  printf("Time constant for shape drug-IRF (%f) must be >= 0.\n",tau1);
			  GLMInternal_describe_event_format( SHAPE_DRUGIRF );
			  return(1);
			}
		      ishape = SHAPE_DRUGIRF;
		    }
		  else if ( !strcmp("table",event_shape_string) )
		    {
		      icolumn = (int)tau1;
		      if ( nread != 3 )
			{
			  printf("Error on line %d for event type %d in file %s, # words read = %d\n",
				 iline,event_number,file_name,nread);
			  printf("For table events, give the column entry.\n");
			  GLMInternal_describe_event_format( SHAPE_TABLE );
			  return(1);
			}
		      // Make sure the column number makes sense.
		      if ( icolumn < 0 || icolumn > GLM.table_ncolumns[irun] )
			{
			  printf("Error: The TABLE event %c refers to column %d for run %d,\n",
				 event_char,icolumn,irun);
			  printf("...but allowed values are 0 to %d.\n",
				 GLM.table_ncolumns[irun]);
			  GLMInternal_describe_event_format( SHAPE_TABLE );
			  return(1);
			}
 		      ishape = SHAPE_TABLE;
		    }
		  else if ( !strcmp("interaction",event_shape_string) )
		    {
		      if ( nread != 4 )
			{
			  printf("Error on line %d for event type %d in file %s, # words read = %d\n",
				 iline,event_number,file_name,nread);
			  printf("For interaction events, give two event types that interact.\n");
			  GLMInternal_describe_event_format( SHAPE_INTERACT );
			  return(1);
			}
		      // Make sure the two interacting events were previously defined.
		      // tau1 = 1st event
		      // tau2 = 2nd event
		      sscanf(tau1_string,"%c",&tau1_char);
		      sscanf(tau2_string,"%c",&tau2_char);
		      if ( !(itau1=GLM_EventNumberFromChar(tau1_char)) ||
			   !(itau2=GLM_EventNumberFromChar(tau2_char)) )
			{
			  printf("One of the interacting events (%c, %c) is not allowed\n",
				 tau1_char,tau2_char);
			  printf("on line %d in file %s\n",iline+1,file_name);
			  return(1);
			}
		      found1 = found2 = FALSE;
		      for ( jevent=0; jevent<GLM.nevents; jevent++)
			{
			  if ( itau1 == GLM.event_number[jevent] )
			    found1 = TRUE;
			  if ( itau2 == GLM.event_number[jevent] )
			    found2 = TRUE;
			}
		      if ( !found1 || !found2 )
			{
			  printf("For event %c, interacting events must be previously defined.\n",event_char);
			  GLMInternal_describe_event_format( SHAPE_INTERACT );
			  return(1);
			}
		      else if ( itau1 == itau2 )
			{
			  printf("For event %c, the two interacting events are the same!\n",event_char);
			  GLMInternal_describe_event_format( SHAPE_INTERACT );
			  return(1);
			}
 		      ishape = SHAPE_INTERACT;
		    }
		  else
		    {
		      printf("Error: stimulus shapes are 'square' (default), 'gamma', 'sigmoid', 'drug-IRF'\n");
		      printf("or `table' (error on line %d).\n",iline);
		      return(1);
		    }
		}
	      else
		{
		  printf("Error reading line %d of the GLM file %s (event & shape for stimulus %d).\n",
			 iline,file_name,nevent);
		  return(1);
		}
	      // Make sure the stimulus event makes sense (1-9 or -1)
	      if ( event_number == 0 )
		{
		  printf("Error read line %d of the GLM file %s.\n",iline,file_name);
		  printf("Stim event 0 is reserved for the baseline condition.\n");
		  return(1);
		}
	      else if ( event_number > 0 )  // this is a stimulus, NOT an exclusion segment
		{
		  // Decide whether or not the stimulus event has been defined previously.
		  found = 0;
		  for (jevent=0; jevent<GLM.nevents; jevent++)
		    {
		      if ( event_number == GLM.event_number[jevent] )
			{ // This stimulus event already exists
			  ievent_index = jevent;
			  found = 1;
			  if ( ishape != GLM.event_shape[ievent_index] )
			    {
			      printf("Error: event %c does not have a consistent shape.\n",
				     event_char);
			      printf("The shape is %d in the 1st run and %d in run %d\n",
				     GLM.event_shape[ievent_index],ishape,irun);
			      return(1);
			    }
			  else if ( (ishape == SHAPE_GAMMA   ||
				     ishape == SHAPE_SIGMOID ||
				     ishape == SHAPE_DRUGIRF ||
				     ishape == SHAPE_DRUGIRF ||
				     ishape == SHAPE_SINE    ||
				     ishape == SHAPE_COSINE) &&
				    tau1 != GLM.event_tau1[ievent_index] )
			    {
			      printf("Error: event %c does not have a consistent time constant.\n",
				     event_char);
			      return(1);
			    }
			  else if ( ishape == SHAPE_DRUGIRF && tau2 != GLM.event_tau2[ievent_index] )
			    {
			      printf("Error: event %c does not have a consistent time constant.\n",
				     event_char);
			      return(1);
			    }
			}
		    }
		  if ( !found )
		    { // This is a new stimulus event.
		      GLM.nevents++;    // ... so increment the number of stimulation events
		      if ( GLM.nevents > MAX_EVENTS )
			{
			  printf("Error: # of stimulus events (%d) > max dimension in program (%d).\n",
				 nevent,MAX_STIMS);
			  exit(1);
			}
		      ievent_index = GLM.nevents-1;
		      GLM.event_number[ievent_index] = event_number;
		      GLM.event_shape[ievent_index]  = ishape;
		      if ( ishape == SHAPE_INTERACT )
			{
			  GLM.event_tau1[ievent_index] = itau1;
			  GLM.event_tau2[ievent_index] = itau2;
			}
		      else
			{
			  GLM.event_tau1[ievent_index] = tau1;
			  GLM.event_tau2[ievent_index] = tau2;
			}
		      if ( GLM.event_shape[ievent_index] != SHAPE_DRUGIRF )
			{
			  GLM.length_drugIRF[ievent_index] = 0.;
			  GLM.drugIRF[ievent_index]   = NULL;
			  GLM.plasmaIRF[ievent_index] = NULL;
			}
		      else
			{ // "drug-IRF" shape
			  if ( HRF.time_step == 0. ) HRF.time_step = GLM.tr[0];
			  GLM.length_drugIRF[ievent_index] = 10 * (int) (MAX(tau1,tau2)/HRF.time_step);
			  // The IRF functions will be put on the time scale of the HRF, if it exists.
			  GLM.drugIRF[ievent_index]   = float_vector(GLM.length_drugIRF[ievent_index]);
			  GLM.plasmaIRF[ievent_index] = float_vector(GLM.length_drugIRF[ievent_index]);
			  for (j=0; j<GLM.length_drugIRF[ievent_index]; j++)
			    {
			      t = HRF.time_step * j;
			      // Gamma function for drug IRF
			      GLM.drugIRF[ievent_index][j]   = t/tau1 * exp(-(double)(t/tau1));
			      // Exponential function for plasma IRF
			      if ( tau2 != 0. )
				GLM.plasmaIRF[ievent_index][j] = exp(-(double)(t/tau2));
			      else
				GLM.plasmaIRF[ievent_index][j] = 0.;
			    }
			}
		    }
		  if ( ishape == SHAPE_TABLE )
		    {
		      istim = GLM.nstims[ievent_index];
		      GLM.stim_on[ievent_index][istim]  = GLM.run_start[irun] * GLM.tr[0];
		      GLM.stim_off[ievent_index][istim] = GLM.run_stop[irun]  * GLM.tr[0];
		      if ( icolumn >= GLM.table_ncolumns[irun] )
			{
			  printf("Error: column number (%d) for event %c\n",
				 icolumn,GLM_EventChar(GLM.event_number[ievent_index]));
			  printf("       >= number of columns (%d) for run %d\n",
				 GLM.table_ncolumns[irun],irun);
			  return(1);
			}
		      else
			{ // drift-correct the table if specified.
			  //			  GLM_EventCharFromInteger(event_number,&event_char);
			  if ( GLM.drift_correct_table[event_number] > 0 )
			    {
			      if ( GLM_input.notify )
				printf("drift-correct table event %c for run %d and column %d using %d terms\n",
				       event_char,irun,icolumn,GLM.drift_correct_table[event_number]);
			      GLMInternal_drift_correct_table(irun,icolumn,GLM_input.nbaseline_terms);
			    }
			  //			  else	
			  //			    GLMInternal_drift_correct_table(irun,icolumn,1); // just remove DC term
			}
		      GLM.table_icolumn[ievent_index][istim] = icolumn;
		      GLM.nstims[ievent_index]++; // treat table as 1 long stim in the current run
		    }
		  else if ( ishape == SHAPE_INTERACT )
		    {
		      istim = GLM.nstims[ievent_index];
		      GLM.stim_on[ievent_index][istim]  = GLM.run_start[irun] * GLM.tr[0];
		      GLM.stim_off[ievent_index][istim] = GLM.run_stop[irun]  * GLM.tr[0];
		      GLM.nstims[ievent_index]++; // treat interaction as 1 long stim in the current run
		    }
		  // Update local counter.
		  nevent++;
		  nstim = 0;
		}
	      break;
	    }
	}
      else
	{ // The line was NOT blank, and it is not a stimulus identifier (event, shape, ...)
	  // The next line depends upon the shape.
	  if ( ishape==SHAPE_SQUARE   ||
	       ishape==SHAPE_RAMPUP   ||
	       ishape==SHAPE_RAMPDOWN ||
	       ishape==SHAPE_DRUGIRF  ||
	       ishape==SHAPE_SINE     ||
	       ishape==SHAPE_COSINE   ||
	       event_number < 0 )
	    { // [on] [off] [optional magnitude]
	      nread = sscanf(line,"%f %f %f",&stim_on,&stim_off,&stim_mag);
	      if ( nread < 2 )
		{
		  printf("Error reading line %d of the GLM file %s\n",iline,file_name);
		  printf("This should be on time & off time for event %c and stim %d.\n",
			 GLM_EventChar(event_number),nstim);
		  GLMInternal_describe_event_format( ishape );
		  return(1);
		}
	      else if ( nread == 2 )
		stim_mag = 1.;
	    }
	  else if ( ishape == SHAPE_GAMMA || ishape == SHAPE_SIGMOID )
	    { // [on] [optional magnitude]
	      nread = sscanf(line,"%f %f",&stim_on,&stim_mag);
	      if ( nread == 0 )
		{
		  printf("Error reading line %d of the GLM file %s\n",iline,file_name);
		  printf("This line should at least give the ON time for event %c and stim %d.\n",
			 GLM_EventChar(event_number),nstim);
		  GLMInternal_describe_event_format( ishape );
		  return(1);
		}
	      else if ( nread == 1 )
		stim_mag = 1.;
	      stim_off = (GLM.run_stop[irun] - GLM.run_start[irun]) * GLM.tr[irun];
	    }
	  else if ( ishape == SHAPE_TABLE || ishape == SHAPE_INTERACT )
	    {
	      printf("Error interpretting line %d of the GLM file %s\n",iline,file_name);
	      printf("Only a blank line should follow a table or interaction event.\n");
	      GLMInternal_describe_event_format( ishape );
	      return(1);
	    }
	  else
	    {
	      printf("The shape (%d) for event %c is not defined.\n",ishape,GLM_EventChar(event_number));
	      return(1);
	    }
	  //
	  // Handle error conditions:
	  //
	  // 1) Each ON time must be greater than 0.
	  if ( stim_on < 0. )
	    {
	      printf("Error: ON time must be > 0 for stim %d of event %c.\n",
		     nstim,GLM_EventChar(event_number));
	      return(1);
	    }
	  // 2) The OFF time must be later than the ON time for stim events > 0
	  if ( event_number > 0 && stim_off <= stim_on )
	    {
	      printf("Error: OFF time must be > ON time for stim %d of event %c.\n",
		     nstim,GLM_EventChar(event_number));
	      return(1);
	    }
	  // 3) The OFF time must be equal to or later than the ON time for stim events < 0
	  else if ( event_number < 0 && stim_off < stim_on )
	    {
	      printf("Error: OFF time must be >= ON time for exclusion segments (event %c).\n",
		     GLM_EventChar(event_number));
	      return(1);
	    }
	  // Set GLM values using tr and the run number.
	  if ( event_number > 0 )
	    { // This is an event, not an exclusion range
	      GLM.stim_on[ievent_index][GLM.nstims[ievent_index]]   = stim_on   + GLM.run_start[irun]*tr;
	      GLM.stim_off[ievent_index][GLM.nstims[ievent_index]]  = stim_off  + GLM.run_start[irun]*tr;
	      GLM.stim_mag[ievent_index][GLM.nstims[ievent_index]]  = stim_mag;
	      GLM.nstims[ievent_index]++;
	      nstim++;
	      if ( GLM.nstims[ievent_index] > MAX_STIMS )
		{
		  printf("Error: the number of stimuli (%d) for event %c > max dimension in program (%d).\n",
			 GLM.nstims[ievent_index],GLM_EventChar(nevent),MAX_STIMS);
		}
	    }
	  else
	    { // Exclusion range
	      GLM.exclude_start[GLM.nexclude] = stim_on  + GLM.run_start[irun];
	      GLM.exclude_end[GLM.nexclude]   = stim_off + GLM.run_start[irun];
	      GLM.nexclude++;
	      if ( GLM.nexclude > MAX_STIMS )
		{
		  printf("Error: the number of segments to exclude (%d) > max dimension in program (%d).\n",
			 GLM.nexclude,MAX_STIMS);
		}
	    }
	}
      iline++;
    }
  fclose( file );

  // Further brief error checking:
  for (jevent=0; jevent<GLM.nevents; jevent++)
    {
      event_number = GLM.event_number[jevent];
      if ( GLM_EventCharFromInteger(event_number,&event_char) )
	{
	  printf("Programming error: event number (%d) apparenty not defined.?!\n",event_number);
	  exit(1);
	}
      else if ( GLM.nstims[jevent] < 1 )
	{
	  printf("Error checking problem: # stims < 1 for event %c.\n",event_char);
	  printf("Check format of event description file.\n");
	  exit(1);
	}
      if ( GLM.event_shape[jevent] != SHAPE_SQUARE   &&
	   GLM.event_shape[jevent] != SHAPE_GAMMA    &&
	   GLM.event_shape[jevent] != SHAPE_SIGMOID  &&
	   GLM.event_shape[jevent] != SHAPE_TABLE    &&
	   GLM.event_shape[jevent] != SHAPE_DRUGIRF  &&
	   GLM.event_shape[jevent] != SHAPE_SINE     &&
	   GLM.event_shape[jevent] != SHAPE_COSINE   &&
	   GLM.event_shape[jevent] != SHAPE_RAMPUP   &&
	   GLM.event_shape[jevent] != SHAPE_RAMPDOWN &&
	   GLM.event_shape[jevent] != SHAPE_INTERACT )
	{
	  printf("Programming error: the event shape (%d) is unknown.\n",GLM.event_shape[jevent]);
	  exit(1);
	}
    }

  return(0);
}

int GLM_read_table_file( char *file_name, int irun )
{
  FILE *file;
  char line[LEN_STR], string[LEN_STR], *ptr;
  int jcolumn, j, nread;
  BOOL error;

  // If the file name is NULL, ignore it.
  if ( !strcmp(file_name,"") || file_name == NULL )
    return(0);

  // Open the GLM file.
  if ( !(file = fopen(file_name,"r")) )
    {
      printf("Error attempting to open the TABLE file %s.\n",file_name);
      return(1);
    }

  // Read the 1st line of the table.
  if ( !get_line_with_comment(file, line, LEN_STR) ) return(1);
  // Count the number of strings on the 1st line.
  ptr = line;
  nread = 0;
  while ( sscanf(ptr,"%s",string)==1 )
    {
      ptr = next_non_white( ptr + jip_int_count((JIP_INDEX)strlen(string)) );
      // Assign the column names based upon the 1st run.
      if ( irun == 0 )
	strcpy(GLM.table_column_name[nread],string);
      else
	{ // For other runs, make sure the column names are consistent.
	  if ( strcmp(GLM.table_column_name[nread],string) )
	    printf("Warning: runs 1 & %d have mismatched names (%s & %s) in column %d.\n",
		   irun+1,GLM.table_column_name[nread],string,nread);
	}
      nread++;
    }
  GLM.table_ncolumns[irun] = nread;  // # of columns in this table
  if ( irun != 0 && GLM.table_ncolumns[irun] != GLM.table_ncolumns[0] )
    printf("Warning: the tables for runs %d/%d have different numbers of columns (%d/%d)\n",
	   irun,0,GLM.table_ncolumns[irun],GLM.table_ncolumns[0]);

  // Now determine the number of time points in the table.
  GLM.table_ntime[irun] = 0;
  while ( get_line_with_comment(file,line,LEN_STR) )
    (GLM.table_ntime[irun])++;

  rewind(file);
  get_line_with_comment(file,line,LEN_STR); // read 1st line again

  // Allocate the table memory.
  GLM.table_value[irun] = float_matrix(GLM.table_ncolumns[irun],GLM.table_ntime[irun]);

  // Read the table values for each column.
  for (j=0; j<GLM.table_ntime[irun]; j++)
    {
      get_line_with_comment(file,line,LEN_STR); // read each line again.
      for (jcolumn=0, ptr=line; jcolumn<GLM.table_ncolumns[irun]; jcolumn++)
	{
	  error =  sscanf(ptr,"%s",string) != 1;
	  error &= sscanf(ptr,"%f",&(GLM.table_value[irun][jcolumn][j])) != 1;
	  if ( error )
	    {
	      printf("Error reading line %d of TABLE file %s\n",j+2,file_name);
	      return(1);
	    }
	  else if ( isnan(GLM.table_value[irun][jcolumn][j]) )
	    {
	      printf("Error: table entry %d in column %d of run %d is not a number.\n",
		     j,jcolumn,irun);
	      exit(1);
	    }
	  ptr = next_non_white( ptr + jip_int_count((JIP_INDEX)strlen(string)) );
	}
    }
  fclose(file);
  return(0);
}

int GLM_write_glm_files( char *dat_file_name, char *stim_file_root )
{
  FILE *file;
  char stim_file_name[LEN_STR], string[LEN_STR], event_char;
  int event_number;
  int jrun, jevent, jstim, jcond;
  float stim_on, stim_off, stim_mag;
  BOOL include_event, include_stim, exclude_region, first_region;

  // Check that things have been defined.
  if ( !GLM.initialized || GLM.ntime <= 0 || GLM.nevents == 0 || GLM.ncoeff <= 0 )
    {
      printf("Error: GLM not defined in GLM_write_glm_files.\n");
      return(1);
    }

  // Open the file for writing.
  if ( !(file = fopen(dat_file_name,"w")) )
    {
      printf("Error attempting to open the data file %s.\n",dat_file_name);
      return(1);
   }
  fprintf(file,"#notify             # output GLM information\n");
  // baseline terms
  fprintf(file,"baseline-terms %d   # poly. order of baseline (0-3)\n",
	  GLM_input.nbaseline_terms);
  // Normalization flag
  if ( GLM_input.normalize_runs == 1 )
    fprintf(file,"normalize-BOLD    # adjust each run for av. signal before fit\n");
  else if ( GLM_input.normalize_runs == 2 )
    fprintf(file,"normalize-IRON    # adjust each run for av. signal before fit\n");
  // NIFTI output flag
  if ( GLM_input.nifti_output )
    fprintf(file,"output-format nifti # use .nii output format for P-x, T-x, S-x files\n");
  // IRF file
  if ( strcmp(GLM_input.HRF_file_name,"") )
    fprintf(file,"IRF %s       # FIR analysis\n",GLM_input.HRF_file_name);

  // Condition string (e.g., 1 2 12 1-2)
  sprintf(string,"conditions %s",GLM.condition_string[0]);
  for (jcond=1; jcond<GLM.nconditions; jcond++)
    {
      strcat(string," ");
      strcat(string,GLM.condition_string[jcond]);
    }
  fprintf(file,"%s\n",string);

  fprintf(file,"runs:\n");
  // Write list of runs. Each run has a corresponding stimulus (.glm) file.
  for (jrun=0; jrun<GLM.nruns; jrun++)
    {
      strcpy(stim_file_name,stim_file_root);
      if ( GLM.nruns == 1 )
	sprintf(string,".glm");
      else
	sprintf(string,"%d.glm",jrun+1);
      strcat(stim_file_name,string);
      fprintf(file,"%s %s\n",
	      GLM_input.run_name[jrun],stim_file_name);
    }
  // done with .dat file
  fclose(file);

  // Write each stimulus file.
  for (jrun=0; jrun<GLM.nruns; jrun++)
    {
      strcpy(stim_file_name,stim_file_root);
      if ( GLM.nruns == 1 )
	sprintf(string,".glm");
      else
	sprintf(string,"%d.glm",jrun+1);
      strcat(stim_file_name,string);
      // Open the file for writing.
      if ( !(file = fopen(stim_file_name,"w")) )
	{
	  printf("Error attempting to open the data file %s.\n",dat_file_name);
	  return(1);
	}
      // TR (time per volume) and length of run.
      fprintf(file,"%g %g\n",GLM.tr[0],
	      (float)(GLM.run_stop[jrun]-GLM.run_start[jrun]+1)*GLM.tr[0]);
      // Loop over events.
      for (jevent=0; jevent<GLM.nevents; jevent++)
	{
	  for (jstim=0, include_event = FALSE; jstim<GLM.nstims[jevent]; jstim++)
	    {
	      // Include this event if either the beginning or end of a stimulus
	      // falls within the run window.
	      include_stim =
		(GLM.stim_on[jevent][jstim]    >= GLM.run_start[jrun] * GLM.tr[0]
		 &&GLM.stim_on[jevent][jstim]  <= GLM.run_stop[jrun]  * GLM.tr[0]) ||
		(GLM.stim_off[jevent][jstim]   >= GLM.run_start[jrun] * GLM.tr[0]
		 &&GLM.stim_off[jevent][jstim] <= GLM.run_stop[jrun]  * GLM.tr[0]);
	      include_event = include_event || include_stim;
	    }
	  if ( include_event )
	    {
	      event_number = GLM.event_number[jevent];
	      if ( GLM_EventCharFromInteger(event_number,&event_char) )
		{
		  printf("Programming error: event number (%d) apparently not defined.?!\n",event_number);
		  exit(1);
		}
	      if ( GLM.event_shape[jevent] == SHAPE_SQUARE )
		sprintf(string,"%c square",event_char);
	      else if ( GLM.event_shape[jevent] == SHAPE_RAMPUP )
		sprintf(string,"%c ramp-up",event_char);
	      else if ( GLM.event_shape[jevent] == SHAPE_RAMPDOWN )
		sprintf(string,"%c ramp-down",event_char);
	      else if ( GLM.event_shape[jevent] == SHAPE_GAMMA )
		sprintf(string,"%c gamma %f",event_char,GLM.event_tau1[jevent]);
	      else if ( GLM.event_shape[jevent] == SHAPE_SIGMOID )
		sprintf(string,"%c sigmoid %f",event_char,GLM.event_tau1[jevent]);
	      else if ( GLM.event_shape[jevent] == SHAPE_SINE )
		sprintf(string,"%c sine %f",event_char,GLM.event_tau1[jevent]);
	      else if ( GLM.event_shape[jevent] == SHAPE_COSINE )
		sprintf(string,"%c cosine %f",event_char,GLM.event_tau1[jevent]);
	      else if ( GLM.event_shape[jevent] == SHAPE_DRUGIRF )
		{
		  if ( GLM.event_tau2[jevent] != 0. )
		    sprintf(string,"%c drug-IRF %f %f",event_char,
			    GLM.event_tau1[jevent],GLM.event_tau2[jevent]);
		  else
		    sprintf(string,"%c drug-IRF %f",event_char,GLM.event_tau1[jevent]);
		}
	      else if ( GLM.event_shape[jevent] == SHAPE_TABLE )
		sprintf(string,"%c table %d",event_char,GLM.table_icolumn[jevent][0]);

	      fprintf(file,"\n%s\n",string);
	      for (jstim=0, include_stim = FALSE;
		   jstim<GLM.nstims[jevent]; jstim++)
		{
		  // Include a stimulus if it falls inside the run window.
		  include_stim =
		    (GLM.stim_on[jevent][jstim]    >= GLM.run_start[jrun] * GLM.tr[0]
		     &&GLM.stim_on[jevent][jstim]  <= GLM.run_stop[jrun]  * GLM.tr[0]) ||
		    (GLM.stim_off[jevent][jstim]   >= GLM.run_start[jrun] * GLM.tr[0]
		     &&GLM.stim_off[jevent][jstim] <= GLM.run_stop[jrun]  * GLM.tr[0]);
		  if ( include_stim )
		    {
		      // Set the limits for the stimulus within the run.
		      stim_on  = MAX(GLM.stim_on[jevent][jstim],GLM.run_start[jrun]*GLM.tr[0]);
		      stim_off = MIN(GLM.stim_off[jevent][jstim],GLM.run_stop[jrun]*GLM.tr[0]);
		      stim_mag = GLM.stim_mag[jevent][jstim];
		      // Define stimuli relative to the beginning of the run.
		      stim_on  -= GLM.run_start[jrun] * GLM.tr[0];
		      stim_off -= GLM.run_start[jrun] * GLM.tr[0];
		      // Use a different format for square and gamma stimuli.
		      if ( GLM.event_shape[jevent] == SHAPE_SQUARE   ||
			   GLM.event_shape[jevent] == SHAPE_RAMPUP   ||
			   GLM.event_shape[jevent] == SHAPE_RAMPDOWN ||
			   GLM.event_shape[jevent] == SHAPE_SINE     ||
			   GLM.event_shape[jevent] == SHAPE_COSINE )
			fprintf(file,"%g %g\n",stim_on,stim_off);
		      else if ( GLM.event_shape[jevent] == SHAPE_DRUGIRF )
			fprintf(file,"%g %g %g\n",stim_on,stim_off,stim_mag);
		      else if ( GLM.event_shape[jevent] == SHAPE_GAMMA || GLM.event_shape[jevent] == SHAPE_SIGMOID )
			fprintf(file,"%g %g\n",stim_on,stim_mag);
		    }
		}
	    }
	}
      // Now exclude regions
      for (jstim=0, first_region=TRUE, exclude_region = FALSE;
	   jstim<GLM.nexclude; jstim++)
	{
	  // Exclude a region if it falls inside the run window.
	  exclude_region =
	    (GLM.exclude_start[jstim]   >= GLM.run_start[jrun]
	     &&GLM.exclude_start[jstim] <= GLM.run_stop[jrun]) ||
	    (GLM.exclude_end[jstim]     >= GLM.run_start[jrun]
	     &&GLM.exclude_end[jstim]   <= GLM.run_stop[jrun]);
	  if ( exclude_region )
	    {
	      // Set the limits for the region within the run.
	      stim_on  = MAX(GLM.exclude_start[jstim],GLM.run_start[jrun]);
	      stim_off = MIN(GLM.exclude_end[jstim],GLM.run_stop[jrun]);
	      // Define region relative to the beginning of the run.
	      stim_on  -= GLM.run_start[jrun];
	      stim_off -= GLM.run_start[jrun];
	      if ( first_region )
		fprintf(file,"\n-1\n");
	      fprintf(file,"%d %d\n",(int)stim_on,(int)stim_off);
	      first_region = FALSE;
	    }
	}
      fclose(file);
    }
  return(0);
}
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
///////// Functions that create and use the GLM //////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

int GLM_define_basis_functions()
     // Define the basis vectors of a general linear model (GLM)
     // based upon a hemodynamic response function (IRF) and
     // information about the times for the stimuli.
{
  int icoeff, nsteps_per_tr, itime, event_number;
  int jevent, jstim, jtime, jtime1, jcoeff, jcoeff1, jrun;
  char event_char;
  static int first_time = 1;
  float *paradigm, scale_factor, duration, points_included, rel_magnitude;

  if ( HRF.nhrf == 1 )
    {
      if ( (nsteps_per_tr=(int)(GLM.tr[0]/HRF.time_step+0.5)) < 1 )
	{
	  printf("Error: the IRF time step (%f) must be <= TR (%f)\n",
		 HRF.time_step,GLM.tr[0]);
	  return(1);
	}
    }
  else
    nsteps_per_tr = 1;
  
  // Free existing arrays.
  if ( ! first_time )
    {
      free_float_matrix(GLM.Xorig_tc,GLM.ntime,GLM.ncoeff);
      free_float_matrix(GLM.X_tc,GLM.ntime,GLM.ncoeff);
      free_float_matrix(GLM.XTXm1_cc,GLM.ncoeff,GLM.ncoeff);
      free_float_matrix(GLM.XTXm1XT_ct,GLM.ncoeff,GLM.ntime);
      free_float_vector(GLM.Beta_c);
      if ( GLM.AR_order != 0 )
	free_float_matrix(GLM.WhiteFilter_tt,GLM.ntime,GLM.ntime);
    }
  else
    {
      first_time = 0;
      GLM.WhiteFilter_tt = NULL;
      GLM.ResMat_tt      = NULL;
      GLM.ARMixm1_rr     = NULL;
    }
  
  // Set the number of coefficients in the GLM:
  GLM.nbaseline = GLM_input.nbaseline_terms * GLM.nruns;
  GLM.ncoeff    = GLM.nevents + GLM.nbaseline;
  GLM.ntable_events = 0; // set to 0 here, and then add to this number below
  
  // Allocate and initialize new arrays.
  GLM.Xorig_tc       = float_matrix(GLM.ntime,GLM.ncoeff);
  GLM.X_tc           = float_matrix(GLM.ntime,GLM.ncoeff);
  GLM.XTXm1_cc       = float_matrix(GLM.ncoeff,GLM.ncoeff);
  GLM.XTXm1XT_ct     = float_matrix(GLM.ncoeff,GLM.ntime);
  GLM.Beta_c         = float_vector(GLM.ncoeff);
  for (jcoeff=0; jcoeff<GLM.ncoeff; jcoeff++)
    {
      GLM.Beta_c[jcoeff] = 0.;
      for (jcoeff1=0; jcoeff1<GLM.ncoeff; jcoeff1++)
	GLM.XTXm1_cc[jcoeff][jcoeff1] = 0.;
      for (jtime=0; jtime<GLM.ntime; jtime++)
	GLM.X_tc[jtime][jcoeff] = GLM.XTXm1XT_ct[jcoeff][jtime] = 0.;
    }
  if ( GLM_input.AR_order != 0 ) // Auto-regressive correlation model
    {
      GLM.WhiteFilter_tt = float_matrix(GLM.ntime,GLM.ntime);
      // Set the initial whitening filter equal to the identity matrix.
      for (jtime=0; jtime<GLM.ntime; jtime++)        // row
	{
	  for (jtime1=0; jtime1<GLM.ntime; jtime1++) // column
	    {
	      GLM.WhiteFilter_tt[jtime][jtime1] = 0.;
	      if ( jtime1 == jtime )
		GLM.WhiteFilter_tt[jtime][jtime1] = 1.;
	    }
	}
    }

  // Define the exclusion vector
  if ( GLM.include_time )
    free_int_vector(GLM.include_time);
  GLM.include_time = int_vector(GLM.ntime);
  // Initialize the vector with 1's.
  for (jtime=0; jtime<GLM.ntime; jtime++)
    GLM.include_time[jtime] = 1;
  // Loop over time, setting values to zero.
  for (jtime=0; jtime<GLM.ntime; jtime++)
    {
      for (jstim=0; jstim<GLM.nexclude; jstim++)
	{
	  if ( jtime >= GLM.exclude_start[jstim] && jtime <= GLM.exclude_end[jstim] )
	    GLM.include_time[jtime] = 0;
	}
    }
  /*
  ////////////////////////////////////////////////////////
  ///// read data from the file "residue1.dat"
  // Open the file containing the list of runs, GLM files, ...
  if ( (file = fopen("residue1.dat","r")) )
  {
  fscanf(file,"%s %s %s",string,string,string);
  for (jtime=0; jtime<GLM.ntime; jtime++)
  {
  fscanf(file,"%s %s %f",string,string,GLM.include_time+jtime);
  printf("include[%d] = %f\n",jtime,GLM.include_time[jtime]);
  }
  }

  GLM.nexclude = 0;
  for (jtime=0; jtime<GLM.ntime; jtime++)
  {
  if ( GLM.include_time[jtime] == 0 )
  {
  GLM.exclude_start[GLM.nexclude] = GLM.exclude_end[GLM.nexclude] = jtime;
  GLM.nexclude++;
  }
  }
  printf("in glm, exclude %d\n",GLM.nexclude);
  */
  ////////////////////////////////////////////////////////

  // Count the number of included time points.
  for (jtime=0, points_included=0; jtime<GLM.ntime; jtime++)
    points_included += GLM.include_time[jtime];
  // Define the degrees of freedom
  GLM.dof = points_included - GLM.ncoeff;

  /*
  ** Create the basis functions. The number & order of functions is:
  **  1 amplitude for each stimulus event
  **          OR
  **  1 amplitude for parameter assigned to each stimulus events
  **    ... looped over the number of stimulus events ...
  ** then:
  **        baseline parameters (offset, slope, quad)
  */

  // Allocate a paradigm vector; also used for SCM, if IRF exists.
  paradigm = float_vector(GLM.ntime * nsteps_per_tr);

  // Create the stimulus-convolved matrix.
  // Loop over the number of stimulus events.
  for (jevent=0, icoeff=0; jevent<GLM.nevents; jevent++, icoeff++)
    {
      if ( GLM.event_shape[jevent] == SHAPE_TABLE )
	{
	  if ( GLMInternal_create_table(jevent, nsteps_per_tr, paradigm) )
	    return(1);
	  GLM.ntable_events++;
	}
      else if ( GLM.event_shape[jevent] == SHAPE_INTERACT )
	GLMInternal_create_interaction(jevent,paradigm);
      else
	{
	  // Create the paradigm vector.
	  if ( GLMInternal_create_stimulus_paradigm(jevent, nsteps_per_tr, paradigm) )
	    return(1);
	  // Convolve the paradigm with the IRF function, if it exists.
	  // For a drug-IRF model, the IRF should describe the blood plasma time course,
	  // usually as a single exponential.
	  if ( GLM.event_shape[jevent] == SHAPE_DRUGIRF ) // shape = `drug-IRF'
	    {
	      // First convolve with plasma function, if it is defined.
	      if ( GLMInternal_convolve_paradigm_IRF(1, nsteps_per_tr, paradigm, jevent) )
		return(1);
	      // Then convolve with drug response.
	      if ( GLMInternal_convolve_paradigm_IRF(2, nsteps_per_tr, paradigm, jevent) )
		return(1);
	    }
	}

      event_number = GLM.event_number[jevent];
      if ( !(GLM.event_shape[jevent] == SHAPE_INTERACT) &&
	   !(GLM.event_shape[jevent] == SHAPE_TABLE && !GLM.convolve_table[event_number]) )
	{ // For any shape but table, or even for tables that have the flag set,
	  // convolve the paradigm with the hemodynamic IRF.
	  if ( GLMInternal_convolve_paradigm_IRF(0, nsteps_per_tr, paradigm, 0) )
	    return(1);
	}

      // Make sure the stimulus paradigm is normalized to the event scale (generally 1).
      scale_factor = GLM.event_scale[event_number];
      if ( GLM.event_shape[jevent] == SHAPE_INTERACT )
	rel_magnitude = GLMInternal_normalize_paradigm(GLM.ntime, paradigm, scale_factor);
      else
	rel_magnitude = GLMInternal_normalize_paradigm(GLM.ntime*nsteps_per_tr, paradigm, scale_factor);
      if ( GLM_EventCharFromInteger(event_number,&event_char) )
	{
	  printf("Programming error: event number (%d) apparenty not defined.?!\n",event_number);
	  exit(1);
	}
      // printf("relative scaling for REM for event %c = %g\n",event_char,rel_magnitude);
      
      // On the regular (TR) time raster, set the GLM stimulus basis function as the
      // corresponding value on the fine scale raster.
      for (jtime=0; jtime<GLM.ntime; jtime++)
	{
	  if ( GLM.event_shape[jevent] == SHAPE_INTERACT )
	    GLM.X_tc[jtime][icoeff] = paradigm[jtime];
	  else
	    GLM.X_tc[jtime][icoeff] = paradigm[jtime*nsteps_per_tr];
	}
    }
  free_float_vector(paradigm);

  // The constant offset values for each run ...
  if ( GLM_input.nbaseline_terms >= 1 )
    {
      for ( jrun=0; jrun<GLM.nruns; jrun++)
	{
	  for ( jtime=0; jtime<GLM.ntime; jtime++ )
	    {
	      if ( jtime >= GLM.run_start[jrun] && jtime <= GLM.run_stop[jrun] )
		GLM.X_tc[jtime][icoeff] = 1.;
	      else
		GLM.X_tc[jtime][icoeff] = 0.;
	    }
	  icoeff++;
	}
    }
  // The drift corrections for each run ...
  // Give each drift correction an average value of zero.
  // 1st order: set range 0->1, then subtract 1/2 to make zero mean.
  if ( GLM_input.nbaseline_terms >= 2 )
    {
      for ( jrun=0; jrun<GLM.nruns; jrun++)
	{
	  duration = GLM.run_stop[jrun] - GLM.run_start[jrun];
	  for ( jtime=0; jtime<GLM.ntime; jtime++ )
	    {
	      itime = jtime - GLM.run_start[jrun];
	      if ( jtime >= GLM.run_start[jrun] && jtime <= GLM.run_stop[jrun] )
		GLM.X_tc[jtime][icoeff] = (float)(itime) / duration - 1./2.;
	      else
		GLM.X_tc[jtime][icoeff] = 0.;
	    }
	  icoeff++;
	}
    }
  // 2nd order drift, if included ...
  // Set range 0->1, then subtract 1/3 to make zero mean.
  if ( GLM_input.nbaseline_terms >= 3 )
    {
      for ( jrun=0; jrun<GLM.nruns; jrun++)
	{
	  duration = GLM.run_stop[jrun] - GLM.run_start[jrun];
	  for ( jtime=0; jtime<GLM.ntime; jtime++ )
	    {
	      itime = jtime - GLM.run_start[jrun];
	      if ( jtime >= GLM.run_start[jrun] && jtime <= GLM.run_stop[jrun] )
		GLM.X_tc[jtime][icoeff] = (float)itime/duration * (float)itime/duration - 1./3.;
	      else
		GLM.X_tc[jtime][icoeff] = 0.;
	    }
	  icoeff++;
	}
    }
  // 3rd order drift, if included ...
  // Set range 0->1, then subtract 1/4 to make zero mean.
  if ( GLM_input.nbaseline_terms >= 4 )
    {
      for ( jrun=0; jrun<GLM.nruns; jrun++)
	{
	  duration = GLM.run_stop[jrun] - GLM.run_start[jrun];
	  for ( jtime=0; jtime<GLM.ntime; jtime++ )
	    {
	      itime = jtime - GLM.run_start[jrun];
	      if ( jtime >= GLM.run_start[jrun] && jtime <= GLM.run_stop[jrun] )
		GLM.X_tc[jtime][icoeff] = (float)itime/duration * (float)itime/duration * (float)itime/duration * - 1./4.;
	      else
		GLM.X_tc[jtime][icoeff] = 0.;
	    }
	  icoeff++;
	}
    }

  // The basis functions now are defined, except for further steps, including whitening.

  // Multiply the non-whitened design matrix by the exclusion matrix.
  for (jcoeff=0; jcoeff<GLM.ncoeff; jcoeff++)
    {
      for (jtime=0; jtime<GLM.ntime; jtime++)
	{
	  GLM.Xorig_tc[jtime][jcoeff] = GLM.X_tc[jtime][jcoeff]; // copy basis functions
	  GLM.X_tc[jtime][jcoeff]    *= GLM.include_time[jtime]; // apply inclusion matrix
	}
    }

  //////////////////////////////////////////////////////////
  // The non-whitened design matrix (GLM.X_tc) is defined.
  //////////////////////////////////////////////////////////

  // The following function call is required to define the model,
  // even if "whitening" only uses the identity matrix.
  GLMInternal_whiten_model(1);

  if ( GLM_input.AR_order != 0 ) // Auto-regressive correlation model
    {
      if ( GLM.ResMat_tt != NULL )
	free_float_matrix(GLM.ResMat_tt,GLM.ntime,GLM.ntime);
      GLM.ResMat_tt  = float_matrix(GLM.ntime,GLM.ntime);
      GLM.ARMixm1_rr = float_matrix(GLM_input.AR_order+1,GLM_input.AR_order+1);
      // Find the residue matrix: R = I - X (X'X)^-1 X'
      //                          R_tt' = I_tt' - X_tc Z_ct'
      for (jtime=0; jtime<GLM.ntime; jtime++)
	{
	  for (jtime1=0; jtime1<GLM.ntime; jtime1++)
	    {
	      // Start with the identity matrix
	      if ( jtime == jtime1 )
		// GLM.ResMat_tt[jtime][jtime1] = 1.;
		GLM.ResMat_tt[jtime][jtime1] = GLM.include_time[jtime];
	      else
		GLM.ResMat_tt[jtime][jtime1] = 0;
	      for (jcoeff=0; jcoeff<GLM.ncoeff; jcoeff++)
		GLM.ResMat_tt[jtime][jtime1] -= GLM.include_time[jtime] * GLM.include_time[jtime1] *
		  GLM.X_tc[jtime][jcoeff] * GLM.XTXm1XT_ct[jcoeff][jtime1];
	    }
	}
      // Calculate the AR mixing matrix
      GLMInternal_calculate_AR_mixing_matrix();
    }

  GLM.initialized = 2;
  return(0);
}

void GLMInternal_drift_correct_table( int irun, int icolumn, int nterms )
{
  float *fit;
  int *include, j;


  if ( nterms == 0 )
    return;

  // Temporary working vectors
  fit     = float_vector(GLM.table_ntime[irun]);
  include = int_vector(GLM.table_ntime[irun]);
  
  // Define a polynomial GLM; include only non-zero points.
  // This method ignores all zero values in a table.
  for (j=0; j<GLM.table_ntime[irun]; j++)
    {
      if ( GLM.table_value[irun][icolumn][j] == 0. )
	include[j] = 0;
      else
	include[j] = 1;
    }
  if ( PolynomialDefineGLM(  nterms, GLM.table_ntime[irun], include) )  exit(1);
  if ( PolynomialFitGLM(     GLM.table_ntime[irun], GLM.table_value[irun][icolumn], fit) ) exit(1);
  if ( PolynomialRemoveGLM(0,GLM.table_ntime[irun], GLM.table_value[irun][icolumn], fit) ) exit(1);
  // Set ignored values back to zero.
  /*
  for (j=0; j<GLM.table_ntime[irun]; j++)
    {
      if ( include[j] == 0 )
	GLM.table_value[irun][icolumn][j] = 0.;
    }
  */
  free_float_vector(fit);
  free_int_vector(include);
}

void GLMInternal_whiten_model( int first_time )
     // On the first pass (first_time == 1),
     //    i) copy the X_tc matrix for later usage, and
     //   ii) calculate the fitting matrix XTXm1XT_ct for subsequent analysis
     // On later calls (first_time == 0),
     //    - recalculate XTXm1XT_ct after whitening based upon voxel-specific auto-correlations,
     // OR - just return and do not modify the previously calculated XTXm1XT_ct
{
  int jtime, jtime1, jcoeff, jcoeff1, jcon, jc1, jc2, iRowFail;
  static float **Xnw_tc; // no-whitened design matrix

  if ( first_time )
    { // Make a copy of the unwhitened design matrix.
      Xnw_tc = float_matrix(GLM.ntime,GLM.ncoeff); // temp vector for X_tc
      for (jtime=0; jtime<GLM.ntime; jtime++)      // column
	{
	  for (jcoeff=0; jcoeff<GLM.ncoeff; jcoeff++)
	    Xnw_tc[jtime][jcoeff] = GLM.X_tc[jtime][jcoeff];
	}
    }
  if ( !first_time && GLM.AR_order == 0 ) return;  // do nothing
  // else go on if
  // 1) first_time == 1 or
  // 2) GLM.AR_order != 0

  // Note that if AR_order is 0, then WhiteFilter_tt has never been allocated.
  // This saves memory, as WhiteFilter_tt is an array of size ntime^2.
  if ( GLM.AR_order != 0 )
    {
      // Modify the design matrix by multiplication with the whitening filter.
      for (jtime=0; jtime<GLM.ntime; jtime++)        // column
	{
	  for (jcoeff=0; jcoeff<GLM.ncoeff; jcoeff++)
	    {
	      GLM.X_tc[jtime][jcoeff] = 0.; // initialize the pre-whitened design matrix
	      for (jtime1=MAX(0,jtime-GLM.AR_order); jtime1<=jtime; jtime1++)
		GLM.X_tc[jtime][jcoeff] += GLM.WhiteFilter_tt[jtime][jtime1] * Xnw_tc[jtime1][jcoeff];
	    }
	}
    }

  // Calculate (X_transpose * X) by summing over the time index.
  for (jcoeff=0; jcoeff<GLM.ncoeff; jcoeff++)
    {
      for (jcoeff1=0; jcoeff1<GLM.ncoeff; jcoeff1++)
	{
	  GLM.XTXm1_cc[jcoeff][jcoeff1] = 0.;
	  for (jtime=0; jtime<GLM.ntime; jtime++)
	    GLM.XTXm1_cc[jcoeff][jcoeff1] += GLM.X_tc[jtime][jcoeff] * GLM.X_tc[jtime][jcoeff1];
	}
    }

  // Invert to get (X_transpose * X)^-1
  rInvertSquareMatrix(GLM.ncoeff, GLM.XTXm1_cc, &iRowFail);

  // Calculate (X_transpose * X)^-1 * X_transpose
  // by summing over the shared coefficient index.
  for (jcoeff=0; jcoeff<GLM.ncoeff; jcoeff++)
    {
      for (jtime=0; jtime<GLM.ntime; jtime++)
	{
	  GLM.XTXm1XT_ct[jcoeff][jtime] = 0.;
	  for (jcoeff1=0; jcoeff1<GLM.ncoeff; jcoeff1++)
	    GLM.XTXm1XT_ct[jcoeff][jtime] += GLM.XTXm1_cc[jcoeff][jcoeff1] * GLM.X_tc[jtime][jcoeff1];
	}
    }

  // Update the covariance matrix used in the statistical tests.
  for (jcon=0; jcon<GLM.nconditions; jcon++)
    {
      // Define the F convariance matrix for each condition.
      for (jc1=0; jc1<GLM.ncontrasts[jcon]; jc1++)
	{
	  for (jc2=0; jc2<GLM.ncontrasts[jcon]; jc2++)
	    {
	      GLM.FCovar[jcon][jc1][jc2] = 0.;
	      for (jcoeff=0; jcoeff<GLM.ncoeff; jcoeff++)
		{
		  for (jcoeff1=0; jcoeff1<GLM.ncoeff; jcoeff1++)
		    {
		      GLM.FCovar[jcon][jc1][jc2]  +=   GLM.contrast_matrix[jcon][jc1][jcoeff]
			* GLM.XTXm1_cc[jcoeff][jcoeff1] * GLM.contrast_matrix[jcon][jc2][jcoeff1];
		    }
		}
	    }
	}
      // Make a copy for the inverse
      for (jc1=0; jc1<GLM.ncontrasts[jcon]; jc1++)
	{
	  for (jc2=0; jc2<GLM.ncontrasts[jcon]; jc2++)
	    GLM.FCovarInv[jcon][jc1][jc2] = GLM.FCovar[jcon][jc1][jc2];
	}
      // Now invert the matrix.
      rInvertSquareMatrix( GLM.ncontrasts[jcon], GLM.FCovarInv[jcon], &iRowFail );
    }
}

void GLMInternal_calculate_AR_mixing_matrix()
{
  int jrho0, jrho1, jtime, jtime1, jtime_sum, iRowFail;
  float **Matrix0_tt, **Matrix1_tt, **Matrix01_tt;

  Matrix0_tt   = float_matrix(GLM.ntime,GLM.ntime);
  Matrix1_tt   = float_matrix(GLM.ntime,GLM.ntime);
  Matrix01_tt  = float_matrix(GLM.ntime,GLM.ntime);

  for (jrho0=0; jrho0<=GLM_input.AR_order; jrho0++)
    {
      if ( GLM_input.AR_order > 1 )
	{
	  printf("\rCalculating AR bias reduction matrix for row %d ...",jrho0);
	  fflush(stdout);
	}
      // Calculate 1 matrix that forms, by trace, part of the M matrix of Worsley
      for (jtime=0; jtime<GLM.ntime; jtime++)
	{ // matrix0_tt' = R_tj * D_jt', j is time, D is an upper diagonal matrix
	  for (jtime1=0; jtime1<GLM.ntime; jtime1++)
	    { 
	      Matrix0_tt[jtime][jtime1] = 0.;
	      for (jtime_sum=0; jtime_sum<GLM.ntime; jtime_sum++)
		if ( jtime1 == jtime_sum + jrho0 ) // This applies upper diagonal D_jt'
		  Matrix0_tt[jtime][jtime1] += GLM.ResMat_tt[jtime][jtime_sum];
	    }
	}
      // Calculate 1 matrix that forms, by trace, part of the M matrix of Worsley
      for (jrho1=0; jrho1<=GLM_input.AR_order; jrho1++)
	{
	  for (jtime=0; jtime<GLM.ntime; jtime++)
	    {
	      for (jtime1=0; jtime1<GLM.ntime; jtime1++)
		{
		  if ( jrho1 == 0 )
		    { // Matrix1 is the identity matrix
		      if ( jtime == jtime1 )
			Matrix1_tt[jtime][jtime1] = GLM.include_time[jtime1];
		      //			Matrix1_tt[jtime][jtime1] = 1.;
		      else
			Matrix1_tt[jtime][jtime1] = 0.;
		    }
		  else
		    { // matrix1_tt' = R_tj * ( D_jt' + D_t'j), j is time
		      Matrix1_tt[jtime][jtime1] = 0.;
		      for (jtime_sum=0; jtime_sum<GLM.ntime; jtime_sum++)
			if ( jtime1==jtime_sum+jrho1 || jtime_sum==jtime1+jrho1 ) // upper diagonal here
			  Matrix1_tt[jtime][jtime1] += GLM.ResMat_tt[jtime][jtime_sum];
		    }
		}
	    }
	  // Calculate Matrix01 as the product.
	  // We only need the trace, so only the diagnonal elements are calculated.
	  for (jtime=0; jtime<GLM.ntime; jtime++)
	    {
	      Matrix01_tt[jtime][jtime] = 0.;
	      for (jtime_sum=0; jtime_sum<GLM.ntime; jtime_sum++)
		Matrix01_tt[jtime][jtime] +=
		  Matrix0_tt[jtime][jtime_sum] * Matrix1_tt[jtime_sum][jtime];
	    }
	  // Finally set the matrix element for the desired array.
	  GLM.ARMixm1_rr[jrho0][jrho1] = Trace_of_Matrix(GLM.ntime,Matrix01_tt);
	}
    }
  free_float_matrix(Matrix0_tt,GLM.ntime,GLM.ntime);
  free_float_matrix(Matrix1_tt,GLM.ntime,GLM.ntime);
  free_float_matrix(Matrix01_tt,GLM.ntime,GLM.ntime);
  // Invert the matrix to form M^-1, in Worsley's notation
  rInvertSquareMatrix(GLM_input.AR_order+1, GLM.ARMixm1_rr, &iRowFail);
  printf("\n");
}

int GLM_calculate_AR_coefficient(int ntime, float *data_vector, int norder, float *auto_cor)
     // INPUT :  data_vector[ntime]
     // OUTPUT:  auto_cor[norder+1]
{
  float *residue, *ac, y_prime;
  int jtime, jtime1, jlag, jrho;

  if ( ntime != GLM.ntime )
    {
      printf("Error calculating AR coefficient: nt (%d,%d) is inconsistent.\n",
	     ntime,GLM.ntime);
      return(1);
    }
  else if ( norder != GLM_input.AR_order )
    {
      printf("Error calculating AR coefficient: order (%d,%d) is inconsistent.\n",
	     norder,GLM_input.AR_order);
      return(1);
    }
  else if ( norder == 0 )
    return(0);  // do nothing


  residue = float_vector(ntime);
  ac      = float_vector(GLM_input.AR_order+1);
  // Calculate the residue.
  for (jtime=0; jtime<GLM.ntime; jtime++)
    {
      // r_t = R_tt' * Y_t'
      residue[jtime] = 0.;
      for (jtime1=0; jtime1<GLM.ntime; jtime1++)
	{
	  y_prime = GLM.include_time[jtime1] * data_vector[jtime1];
	  //	  y_prime = data_vector[jtime1];
	  residue[jtime] += GLM.ResMat_tt[jtime][jtime1] * y_prime;
	}
    }

  // From the residue, compute each autocorrelation
  for (jlag=0; jlag<=GLM_input.AR_order; jlag++)
    {
      ac[jlag] = 0.;
      for (jtime=jlag; jtime<GLM.ntime; jtime++)
	ac[jlag] += residue[jtime] * residue[jtime-jlag];
    }

  /*
  // temporary
  for (jlag=0; jlag<=GLM_input.AR_order; jlag++)
    {
      auto_cor[jlag] = ac[jlag];
      if ( jlag == 0 )
	{
	  if ( auto_cor[0] == 0. )
	    auto_cor[0] = 1.;
	}
      else
	auto_cor[jlag] /= auto_cor[0];
      //      if ( data_vector[0] != 0. )
      //	printf("auto_cor[%d] = %g\n",jlag,auto_cor[jlag]);
    }
  // temporary
  */

  // return the normalized autocorrelation of the residue
  for (jlag=0; jlag<=GLM_input.AR_order; jlag++)
    {
      auto_cor[jlag] = 0;
      for (jrho=0; jrho<=GLM_input.AR_order; jrho++)
	auto_cor[jlag] += GLM.ARMixm1_rr[jlag][jrho] * ac[jrho];
      // If the 0th order correlation is 0 (blank time course), set it to 1.
      if ( jlag == 0 )
	{
	  if ( auto_cor[0] == 0. )
	    auto_cor[0] = 1.;
	}
      else
	auto_cor[jlag] /= auto_cor[0];
    }
  // Normalize the 0th value.
  auto_cor[0] = 1.;
  free_float_vector(residue);
  free_float_vector(ac);

  return(0);
}

int GLM_calculate_whitening_filter(int ndim, float *auto_cor, short *norder)
     // INPUT :  ndim
     // INPUT :  auto_cor[ndim]
{
  float **toeplitz, **cholesky, *ac_vector, arg;
  int jtime, jtime1, jrho, jrho1, jrel, iRowFail;
  int debug = 0, positive_definite;

  *norder = GLM_input.AR_order;  // default value; may be changed later in function

  if ( ndim != GLM_input.AR_order+1 )
    {
      printf("Error calculating whitening filter: AR dimension (%d)\n",ndim);
      printf("is inconsistent with AR order (%d)\n",GLM_input.AR_order);
      return(1);
    }

  if ( GLM_input.AR_order < 1 )
    {
      GLM.AR_order = 0;
      return(0);  // do nothing
    }
  else
    {
      GLM.AR_order = GLM_input.AR_order;
      /*
	for (jrho=0; jrho<=GLM_input.AR_order; jrho++)
	if ( auto_cor[jrho] < 0.01 )
	{
	GLM.AR_order = jrho-1;
	break;
	}
      */
    }

  if ( debug && auto_cor[1] > 0.2 )
    {
      printf("auto_cor\n");
      for (jrho=0; jrho<=GLM.AR_order; jrho++)
	printf(" %f",auto_cor[jrho]);
      printf("\n");
    }

  // Follow Worsley et al., NeuroImage 2002, "A general statistical analysis for fMRI data."
  // Create a Toeplitz matrix from the auto-correlations.
  toeplitz = float_matrix(GLM_input.AR_order+1,GLM_input.AR_order+1);
  cholesky = float_matrix(GLM_input.AR_order+1,GLM_input.AR_order+1);
  for (jrho=0; jrho<=GLM.AR_order; jrho++)
    {
      for (jrho1=0; jrho1<=GLM.AR_order; jrho1++)
	toeplitz[jrho][jrho1] = cholesky[jrho][jrho1] = 0.;
    }
  // Upper diagonal
  for (jrho=0; jrho<=GLM.AR_order; jrho++)
    {
      for (jrho1=jrho; jrho1<=GLM.AR_order; jrho1++)
	toeplitz[jrho][jrho1] = auto_cor[jrho1-jrho];
    }
  // Lower diagonal by symmetry;
  for (jrho=0; jrho<=GLM.AR_order; jrho++)
    {
      for (jrho1=0; jrho1<jrho; jrho1++)
	toeplitz[jrho][jrho1] = toeplitz[jrho1][jrho];
    }
  /*
  // temporary: print matrix
  printf("toeplitz\n");
  for (jrho=0; jrho<=GLM.AR_order; jrho++)
  {
  for (jrho1=0; jrho1<=GLM.AR_order; jrho1++)
  printf(" %f",toeplitz[jrho][jrho1]);
  printf("\n");
  }
  */
  ////////////////////////////////////////////////////////////////////////////
  // Cholesky factorization of matrix above.
  positive_definite = 0;
  while ( !positive_definite )
    {
      positive_definite = 1;
      for (jrho1=0; jrho1<=GLM.AR_order && positive_definite; jrho1++)
	{
	  for (jrho=jrho1; jrho<=GLM.AR_order && positive_definite; jrho++)
	    {
	      arg = toeplitz[jrho][jrho1]; // lower triangle
	      for (jrel=0; jrel<=jrho1-1; jrel++)
		arg -= toeplitz[jrho][jrel] * toeplitz[jrho1][jrel];
	      if ( jrho == jrho1 )
		{
		  if ( arg < 0. )
		    {
		      GLM.AR_order--;
		      positive_definite = 0;
		      if ( debug )
			printf("*** arg = %f , order = %d***\n",arg,GLM.AR_order);
		    }
		  cholesky[jrho][jrho1] = sqrt(fabs((double)arg));
		}
	      else
		cholesky[jrho][jrho1] = arg / cholesky[jrho1][jrho1];
	    }
	}
    }
  /*
    printf("cholesky\n");
    for (jrho=0; jrho<=GLM.AR_order; jrho++)
    {
    for (jrho1=0; jrho1<=GLM.AR_order; jrho1++)
    printf(" %f",cholesky[jrho][jrho1]);
    printf("\n");
    }
  */
  // invert the matrix
  rInvertSquareMatrix(GLM.AR_order+1, cholesky, &iRowFail);
  /*
    printf("inverse cholesky\n");
    for (jrho=0; jrho<=GLM.AR_order; jrho++)
    {
    for (jrho1=0; jrho1<=GLM.AR_order; jrho1++)
    printf(" %f",cholesky[jrho][jrho1]);
    printf("\n");
    }
  */
  // reformat the last row
  ac_vector = float_vector(GLM.AR_order+1);
  for (jrho=0; jrho<=GLM.AR_order; jrho++)
    ac_vector[jrho] = cholesky[GLM.AR_order][GLM.AR_order-jrho];
  // Fill Whitening Filter
  if ( GLM.AR_order != GLM_input.AR_order )
    {
      // Reset the filter matrix out to length GLM_input.AR_order
      for (jtime=0; jtime<GLM.ntime; jtime++)
	{
	  for (jrho=0; jrho<=GLM_input.AR_order; jrho++)
	    {
	      if ( jtime+jrho < GLM.ntime )
		GLM.WhiteFilter_tt[jtime+jrho][jtime] = 0.;
	    }
	}
    }
  // Fill Whitening Filter
  for (jtime=0; jtime<GLM.ntime; jtime++)
    {
      for (jrho=0; jrho<=GLM.AR_order; jrho++)
	{
	  if ( jtime+jrho < GLM.ntime )
	    GLM.WhiteFilter_tt[jtime+jrho][jtime] = ac_vector[jrho];
	}
    }
  // Overwrite the 1st block of the whitening matrix with the inverse cholesky.
  for (jrho=0; jrho<=GLM.AR_order; jrho++)
    {
      for (jrho1=0; jrho1<=GLM.AR_order; jrho1++)
	GLM.WhiteFilter_tt[jrho][jrho1] = cholesky[jrho][jrho1];
    }

  if ( debug && auto_cor[1] > 0.2 )
    {
      printf("whitening matrix:\n");
      for (jtime=0; jtime<5; jtime++)
	{
	  for (jtime1=0; jtime1<5; jtime1++)
	    printf(" %f",GLM.WhiteFilter_tt[jtime][jtime1]);
	  printf("\n");
	}
    }

  *norder = GLM.AR_order;

  free_float_vector(ac_vector);
  free_float_matrix(toeplitz,GLM_input.AR_order+1,GLM_input.AR_order+1);
  free_float_matrix(cholesky,GLM_input.AR_order+1,GLM_input.AR_order+1);

  return(0);
}

int GLMInternal_create_table(int index_event,   // the index into arrays of length MAX_EVENT
			     int nsteps_per_tr, // defines a finer scale than tr
			     float *paradigm)   // length of nsteps_per_tr * GLM.ntime
{
  int icolumn, ntime_run, ntime_fine, ilow, ihigh;
  int jtime, lTime, jstim, irun;
  float time, time_in_run, fIndex, fmix;

  ntime_fine = GLM.ntime * nsteps_per_tr;
  // Initialize the paradigm, using the fine time scale.
  for (jtime=0; jtime<ntime_fine; jtime++)
    paradigm[jtime] = 0.;
  // Create the paradigm based upon the event shape.
  for (jtime=0, irun=0; jtime<ntime_fine; jtime++)
    {
      // Find the run that contains this time.
      lTime = jtime/nsteps_per_tr;
      if ( lTime > GLM.run_stop[irun] ) irun++;

      time = (float)jtime * (GLM.tr[0]/(double)nsteps_per_tr);
      time_in_run = time - GLM.run_start[irun] * GLM.tr[0];
      ntime_run = GLM.run_stop[irun] - GLM.run_start[irun] + 1;
      for (jstim=0; jstim<GLM.nstims[index_event]; jstim++)
	{
	  icolumn = GLM.table_icolumn[index_event][jstim];
	  if ( time >= GLM.stim_on[index_event][jstim] &&
	       time <= GLM.stim_off[index_event][jstim] )
	    {
	      if ( ntime_run != GLM.table_ntime[irun] )
		{
		  printf("Programming error #1 in GLMInternal_create_table.\n");
		  printf("# points in table (%d) != # points (%d) in run %d\n",
			 GLM.table_ntime[irun],ntime_run,irun);
		  exit(1);
		}
	      else if ( icolumn < 0 || icolumn > GLM.table_ncolumns[irun] )
		{
		  printf("Programming error #2 in GLMInternal_create_table.\n");
		  exit(1);
		}
	      // Change the time index to a floating point image index.
	      fIndex = time_in_run / GLM.tr[0];
	      if ( fIndex <= 0 )                 // Can't interpolate at the low end
		paradigm[jtime] = GLM.table_value[irun][icolumn][0];
	      else if ( fIndex >= ntime_run-1 ) // Can't interpolate at the high end
		paradigm[jtime] = GLM.table_value[irun][icolumn][ntime_run-1];
	      else
		{ // Interpolate the table.
		  ilow = (int)fIndex;
		  ihigh = ilow + 1;
		  fmix  = (ihigh - fIndex);
		  paradigm[jtime] = fmix  * GLM.table_value[irun][icolumn][ilow]
		    +        ( 1. - fmix) * GLM.table_value[irun][icolumn][ihigh];
		}
	    }
	}
    }
  return(0);
}

void GLMInternal_create_interaction(int index_event, float *paradigm)
{
  int interact1, interact2, icoeff1, icoeff2;
  int jevent, jtime;

  interact1 = (int)GLM.event_tau1[index_event];
  interact2 = (int)GLM.event_tau2[index_event];

  for (jevent=0, icoeff1=icoeff2=-1; jevent<GLM.nevents; jevent++)
    {
      if ( interact1 == GLM.event_number[jevent] )
	icoeff1 = jevent;
      if ( interact2 == GLM.event_number[jevent] )
	icoeff2 = jevent;
    }
  if ( icoeff1 == icoeff2 || icoeff1 < 0 || icoeff2 < 0 )
    {
      printf("Programming error in function GLMInternal_create_interaction.\n");
      printf("icoeff1 = %d, icoeff2 = %d\n",icoeff1,icoeff2);
      exit(1);
    }

  // Initialize the paradigm, using the fine time scale.
  for (jtime=0; jtime<GLM.ntime; jtime++)
    paradigm[jtime] = GLM.X_tc[jtime][icoeff1] * GLM.X_tc[jtime][icoeff2];
}

int GLMInternal_create_stimulus_paradigm(int index_event,   // the index into arrays of length MAX_EVENT
					 int nsteps_per_tr, // defines a finer scale than tr
					 float *paradigm)   // length of nsteps_per_tr * GLM.ntime
     // 1) For a sensory/motor/cognitive stimulus, the paradigm (neural function) is square
     //
     // There are 2 ways to analyze a drug:
     // 2) Define the neural function to be a gamma or sigmoid function.
     // 3) Define the paradigm to be a square function equal to the infusion time (shape=drug-irf).
{
  float time, t, exponential, sigmoid, omega, arg;
  int ntime_fine, jtime, jstim;

  ntime_fine = GLM.ntime * nsteps_per_tr;
  // Initialize the paradigm, using the fine time scale.
  for (jtime=0; jtime<ntime_fine; jtime++)
    paradigm[jtime] = 0.;
  // Create the paradigm based upon the event shape.
  for (jtime=0; jtime<ntime_fine; jtime++)
    {
      time = (float)jtime * (GLM.tr[0] / (float)nsteps_per_tr);
      // Find the run that contains this time.
      for (jstim=0; jstim<GLM.nstims[index_event]; jstim++)
	{
	  if ( time >= GLM.stim_on[index_event][jstim] && time <= GLM.stim_off[index_event][jstim] )
	    {
	      if ( GLM.event_shape[index_event] != SHAPE_GAMMA && GLM.event_shape[index_event] != SHAPE_SIGMOID && paradigm[jtime] != 0. )
		{ // Don't allow overlap for square stimuli
		  printf("Error: overlapping stimuli for event %c and stimulus %d\n",
			 GLM_EventChar(GLM.event_number[index_event]),jstim);
		  return(1);
		}
	      else if ( GLM.event_shape[index_event] == SHAPE_SQUARE ||
			GLM.event_shape[index_event] == SHAPE_DRUGIRF )
		paradigm[jtime] += GLM.stim_mag[index_event][jstim];  // square neuronal part
	      else if ( GLM.event_shape[index_event] == SHAPE_RAMPUP )
		{ // linear increase from 0 to 1 over stimulus range
		  t = time - GLM.stim_on[index_event][jstim];
		  paradigm[jtime] += GLM.stim_mag[index_event][jstim] * t /
		    (GLM.stim_off[index_event][jstim] - GLM.stim_on[index_event][jstim]);
		}
	      else if ( GLM.event_shape[index_event] == SHAPE_RAMPDOWN )
		{ // linear decrease from 1 to 0 over stimulus range
		  t = time - GLM.stim_on[index_event][jstim];
		  paradigm[jtime] += GLM.stim_mag[index_event][jstim] *
		    (1-t/(GLM.stim_off[index_event][jstim] - GLM.stim_on[index_event][jstim]));
		}
	      else if ( GLM.event_shape[index_event] == SHAPE_SINE )
		{
		  t = time - GLM.stim_on[index_event][jstim];
		  omega = 2. * PI / GLM.event_tau1[index_event];
		  paradigm[jtime] += GLM.stim_mag[index_event][jstim] * sin((double)(omega*t));
		}
	      else if ( GLM.event_shape[index_event] == SHAPE_COSINE )
		{
		  t = time - GLM.stim_on[index_event][jstim];
		  omega = 2. * PI / GLM.event_tau1[index_event];
		  paradigm[jtime] += GLM.stim_mag[index_event][jstim] * cos((double)(omega*t));
		}
	      else if ( GLM.event_shape[index_event] == SHAPE_GAMMA )
		{
		  t = time - GLM.stim_on[index_event][jstim];
		  arg = t / GLM.event_tau1[index_event];
		  exponential = arg * exp(-(double)arg) / exp(-1.);
		  paradigm[jtime] += GLM.stim_mag[index_event][jstim] * exponential;
		}
	      else if ( GLM.event_shape[index_event] == SHAPE_SIGMOID )
		{
		  t = time - GLM.stim_on[index_event][jstim];
		  arg = t / GLM.event_tau1[index_event];
		  sigmoid = arg / sqrt((double)(1+SQR(arg)));
		  paradigm[jtime] += GLM.stim_mag[index_event][jstim] * sigmoid;
		}
	    }
	}
    }
  return(0);
}

float GLMInternal_normalize_paradigm( int ntime, float *paradigm, float max_value)
{ // Scale the paradigm to max_value; max_value > 0 preserves the sign of the paradigm
  int jtime;
  float max_shape;
  float scale_factor;

  // Find the maximum value of the paradigm.
  for (jtime=0, max_shape=0.; jtime<ntime; jtime++)
    {
      if ( fabs((double)paradigm[jtime]) > fabs((double)max_shape) )
	max_shape = paradigm[jtime];
    }
  if ( max_shape != 0. )
    {
      scale_factor = max_value/fabs((double)max_shape);
      for (jtime=0; jtime<ntime; jtime++)
      	paradigm[jtime] *= scale_factor;
    }
  return(max_shape * max_value);
}

int GLMInternal_convolve_paradigm_IRF( int convolution_flag, // 0 = HRF, 1 = plasma, 2 = drug effect
				       int nsteps_per_tr, // defines a finer scale than tr
				       float *paradigm,   // length of nsteps_per_tr * GLM.ntime
				       int index_event )  // into the event arrays
{
  int ntime_fine, npoints_irf, jtime, jsum;
  float *paradigm_convolve, *irf;

  // Pick the convolution function.
  if ( convolution_flag == 0 )
    {
      if ( HRF.nhrf == 0 )
	return(0);  // do nothing for no HRF
      irf         = HRF.amp;
      npoints_irf = HRF.npoints;
    }
  else if ( convolution_flag == 1 )
    {
      if ( GLM.event_tau2[index_event] == 0. )
      	return(0);  // do nothing for no plasma function
      else if ( GLM.length_drugIRF[index_event] == 0 )
	{
	  printf("Programming error in function GLMInternal_convolve_paradigm_IRF.\n");
	  exit(1);
	}
      irf         = GLM.plasmaIRF[index_event];
      npoints_irf = GLM.length_drugIRF[index_event];
    }
  else if ( convolution_flag == 2 )
    {
      if ( GLM.event_tau1[index_event] == 0. )
	return(0);  // do nothing if time constant = 0
      else if ( GLM.length_drugIRF[index_event] == 0 )
	{
	  printf("Programming error in function GLMInternal_convolve_paradigm_IRF.\n");
	  exit(1);
	}
      irf         = GLM.drugIRF[index_event];
      npoints_irf = GLM.length_drugIRF[index_event];
    }

  // Allocate space for a new paradigm vector.
  ntime_fine = GLM.ntime * nsteps_per_tr;
  paradigm_convolve = float_vector(GLM.ntime * nsteps_per_tr);
  // Initialize the new vector
  for (jtime=0; jtime<ntime_fine; jtime++)
    paradigm_convolve[jtime] = 0.;

  // Convolve the IRF with the paradigm file to get the basis function.
  for (jtime=0; jtime<ntime_fine; jtime++)
    {
      for (jsum=0; jsum<npoints_irf; jsum++)
	if ( jtime-jsum >= 0 )
	  paradigm_convolve[jtime] += irf[jsum] * paradigm[jtime-jsum];
    }
  // Copy the new vector into the original vector.
  for (jtime=0; jtime<ntime_fine; jtime++)
    paradigm[jtime] = paradigm_convolve[jtime];
  free_float_vector(paradigm_convolve);
  return(0);
}

int GLM_fit_data( int ntime, float *data_vector, float *fit_vector, float *sigma2  )
     // input:
     //        ntime      : # points in time series; dimension for data_vector/fit_vector
     //        data_vector: Y in the equation Y = X * Beta
     // output: 
     //        fit_vector : fit to data (X * Beta);       [this is just for display]
     //        sigma2     : sum of (data-fit)^2 / DOF     [IMPORTANT: this is used in statistics]
{
  float y_prime, fit;
  int jtime, jcoeff;

  // Make sure the GLM has been initialized.
  if ( GLM.initialized != 2 )
    {
      printf("Error in GLM_fit_data: first initialize the GLM and define stimuli\n");
      return(1);
    }
  // Make sure the GLM model and the input data are consistent.
  if ( ntime != GLM.ntime )
    {
      printf("Error: the number of data time points (%d) does not match the number in the GLM (%d).\n",
	     ntime,GLM.ntime);
      exit(1);
    }

  // Whiten the data and model when autocorrelations are being taken into account.
  GLMInternal_whiten_data(data_vector);
  GLMInternal_whiten_model(0);

  // Calculate the coefficients of the hemodynamic function Beta_c.
  for (jcoeff=0; jcoeff<GLM.ncoeff; jcoeff++)
    {
      GLM.Beta_c[jcoeff] = 0.;
      // Calculate X_transpose * Y, summing over time
      for (jtime=0; jtime<GLM.ntime; jtime++)
	{
	  y_prime = GLM.include_time[jtime] * data_vector[jtime];
	  GLM.Beta_c[jcoeff] += GLM.XTXm1XT_ct[jcoeff][jtime] * y_prime;
	  //	  data_vector[jtime] = y_prime;
	}
    }
  // Compute the fit vector and the variance between the data and fit.
  *sigma2 = 0.;
  for (jtime=0; jtime<GLM.ntime; jtime++)
    {
      fit = 0.;                // this must done correctly for stats
      fit_vector[jtime] = 0.;  // this is just for display
      for (jcoeff=0; jcoeff<GLM.ncoeff; jcoeff++)
	{
	  fit               += GLM.X_tc[jtime][jcoeff]     * GLM.Beta_c[jcoeff];
	  fit_vector[jtime] += GLM.Xorig_tc[jtime][jcoeff] * GLM.Beta_c[jcoeff];
	}
      y_prime = GLM.include_time[jtime] * data_vector[jtime];
      *sigma2 += SQR(y_prime - fit);
    }
  // The variance is divided the degrees of freedom.
  *sigma2 /= GLM.dof;

  return(0);
}

int GLM_remove_baseline( int option, int ntime, float *data_vector, float *fit_vector )
     // always remove baseline: option 0 = nothing else, 1 = convert to % change
{
  int jtime, jcoeff, icoeff, jrun;

  if ( ntime != GLM.ntime )
    {
      printf("Error: # time points (%d) does not match GLM (%d) in GLM_remove_baseline()\n",
	     ntime,GLM.ntime);
      return(1);
    }

  // Remove all effects of no interest (e.g., baseline terms).
  for ( jcoeff=GLM.ncoeff-GLM.nbaseline; jcoeff<GLM.ncoeff; jcoeff ++ )
    {
      for (jtime=0; jtime<ntime; jtime++)
	{
	  fit_vector[jtime] -= GLM.Xorig_tc[jtime][jcoeff] * GLM.Beta_c[jcoeff];
	  data_vector[jtime]-= GLM.Xorig_tc[jtime][jcoeff] * GLM.Beta_c[jcoeff];
	}
    }
  if ( option != 0 )
    {
      // To convert from Delta signal to % signal, divide by the baseline of each run
      // and multiply by 100.
      // The baseline coefficient for the 1st run is ...
      icoeff = GLM.ncoeff - GLM.nbaseline;
      for (jrun=0; jrun<GLM.nruns; jrun++, icoeff++)
	{
	  for (jtime=GLM.run_start[jrun]; jtime<=GLM.run_stop[jrun]; jtime++)
	    {
	      if ( GLM.Beta_c[icoeff] != 0. )
		data_vector[jtime] *= 100./GLM.Beta_c[icoeff];
	      else
		data_vector[jtime] = 0.;
	    }	    
	}
    }
  return(0);
}

int GLM_correct_baseline( int ntime, float *data_vector, float *fit_vector, float *average )
     // remove drift terms only; e.g., all baseline terms but the 0th order one
     // data_vector: vector of data points that is MODIFIED by this function (in/out)
     // fit_vector:  vector of fit values  that is MODIFIED by this function (in/out)
     // average:     the average value of all points in the modified data vector
{
  int jdrift, jtime, icoeff0, icoeff, jrun;
  float fNaverage;

  if ( ntime != GLM.ntime )
    {
      printf("Error: # time points (%d) does not match the GLM (%d) in GLM_remove_baseline()\n",
	     ntime,GLM.ntime);
      return(1);
    }

  // Loop over drift terms, removing each from all the runs.
  for (jdrift=1; jdrift<GLM_input.nbaseline_terms; jdrift++)
    {
      for (jrun=0; jrun<GLM.nruns; jrun++, icoeff++)
	{
	  // The drift coefficients for the run
	  icoeff0 = GLM.ncoeff - GLM.nbaseline + jrun * GLM_input.nbaseline_terms;
	  icoeff  = icoeff0 + jdrift;
	  for (jtime=GLM.run_start[jrun]; jtime<=GLM.run_stop[jrun]; jtime++)
	    {
	      fit_vector[jtime]  -= GLM.Xorig_tc[jtime][icoeff] * GLM.Beta_c[icoeff];
	      data_vector[jtime] -= GLM.Xorig_tc[jtime][icoeff] * GLM.Beta_c[icoeff];
	    }	    
	}
    }
  // Compute the average of the modified data vector across all included time points.
  *average = 0.;
  fNaverage = 0;
  for (jtime=0; jtime<GLM.ntime; jtime++)
    {
      *average += GLM.include_time[jtime] * data_vector[jtime];
      fNaverage += GLM.include_time[jtime]; // 0 or 1 usually
    }
  *average /= (float)fNaverage;

  return(0);
}

int GLM_define_conditions( int DefineContrasts, char *condition_string, int *nconditions )
     // DefineContrasts: 0 = just get strings, 1 = define contrast matrix
{ // must be called AFTER GLM_define_basis_functions()
  char *cptr, *c_event_number, cond_str[LEN_STR], event_char;
  int event_number, isign, length, found, iRowFail;
  int jchar, jevent, jcoeff, jcoeff1, jcon, jc1, jc2, icondition, icontrast;
  static int ncoeff_save=0;

  // For the 1st pass, ncoeff_save will be 0, so set it.
  if ( ncoeff_save == 0 ) ncoeff_save = GLM.ncoeff;

  //  Set up the conditional tests based upon "condition_string".
  //  This string can contain more than 1 condition.
  //  Example: "1 2 3 4 1234 12-34 12-33 1-3,2-4
  //  yields 7 conditions, including 7 T tests:
  //    1-4) event 1 versus baseline, 2 versus baseline, ...
  //    5) event 1+2+3+4 versus baseline
  //    6) event 1+2-3-4 versus baseline
  //    7) event 1+2-3-3 versus baseline (e.g., [1 1 -2 ..] in contrast vector)
  //  ... and 1 F test
  //    1) events 1-3 or 2-4

  // Reset all information about the conditions
  GLM.nconditions = 0;
  for (jcon=0; jcon<MAX_CONDITIONS; jcon++)
    {
      // Free contrast matrix for each condition.
      if ( GLM.contrast_matrix[jcon] != NULL )
	free_int_matrix(GLM.contrast_matrix[jcon],GLM.ncontrasts[jcon],ncoeff_save);
      // Free F inverse-covariance matrix for each condition.
      if ( GLM.FCovar[jcon] != NULL )
	free_float_matrix(GLM.FCovar[jcon], GLM.ncontrasts[jcon], GLM.ncontrasts[jcon]);
      if ( GLM.FCovarInv[jcon] != NULL )
	free_float_matrix(GLM.FCovarInv[jcon], GLM.ncontrasts[jcon], GLM.ncontrasts[jcon]);
      // C*Beta vector for each condition
      // Reset contrasts to 0
      GLM.ncontrasts[jcon] = 0;
    }

  // Save number of coefficients, since this info is needed to release arrays above.
  ncoeff_save = GLM.ncoeff;

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
      for (jchar=0, GLM.ncontrasts[GLM.nconditions]=0;
	   jchar<length; jchar++)
	{ // Each new appearance of ',' indicates a new row in the contrast matrix
	  if ( *(cptr+jchar) == ',' ) GLM.ncontrasts[GLM.nconditions]++;
	}
      GLM.ncontrasts[GLM.nconditions]++;         // There is 1 row, even if no ',' was found

      if ( DefineContrasts )
	{
	  // Allocate the contrast matrix for this condition, and initialize it.
	  GLM.contrast_matrix[GLM.nconditions] = int_matrix(GLM.ncontrasts[GLM.nconditions],GLM.ncoeff);
	  for (jcon=0; jcon<GLM.ncontrasts[GLM.nconditions]; jcon++)
	    {
	      for (jcoeff=0; jcoeff<GLM.ncoeff; jcoeff++)
		GLM.contrast_matrix[GLM.nconditions][jcon][jcoeff] = 0;
	    }
	}
      // Update the number of conditions.
      GLM.nconditions++;
      cptr = next_non_white(cptr+jip_int_count((JIP_INDEX)strlen(cond_str)));
    }
  *nconditions = GLM.nconditions;

  // Pick apart each condition and analyze the string.
  icondition = 0;
  cptr=condition_string;
  while ( sscanf(cptr,"%s",cond_str) == 1 )
    {
      icontrast = 0;
      // printf("test string %s\n",cond_str);
      // cond_str is the individual condition to be analyzed.
      strcpy(GLM.condition_string[icondition],cond_str); // save string for identification
      length = jip_int_count((JIP_INDEX)strlen(cond_str));
      if ( DefineContrasts )
	{
	  isign = 1;
	  for (jchar=0; jchar<length; jchar++)
	    {
	      c_event_number = cond_str+jchar;
	      event_number = GLM_EventNumberFromChar(*c_event_number);
	      if ( *c_event_number == ',' )
		{
		  icontrast++;  // end this contrast, and make a new row in the contrast matrix
		  isign = 1;    // reset the sign for the new contrast vector
		}
	      else if ( *c_event_number == '-' )
		isign = -1;   // change the sign of the event in the test contrast
	      else
		{
		  // Loop over the number of stimulus events.
		  for (jevent=found=0; jevent<GLM.nevents; jevent++)
		    {
		      if ( GLM.event_number[jevent] == event_number )
			{
			  found = 1;
			  // printf("set contrast [%d][%d][%d] to %d\n",icondition,icontrast,jevent,isign);
			  // If an event occurs n times in a condition, the value will be n.
			  GLM.contrast_matrix[icondition][icontrast][jevent] += isign;
			}
		    }
		  if ( !found )
		    { // Make sure the specified event was found.
		      if ( GLM_EventCharFromInteger(event_number, &event_char) )
			{
			  printf("Error: event type %c is not allowed.\n",event_char);
			  return(1);
			}      
		      printf("Error: one of the events in the conditions (%c) is not defined.\n",
			     event_char);
		      return(1);
		    }
		}
	    }
	}
      // Update the condition counter and go to the next condition string.
      icondition++;
      cptr = next_non_white(cptr+jip_int_count((JIP_INDEX)strlen(cond_str)));
    }

  if ( DefineContrasts && GLM.initialized == 2 ) // then the basis functions have been defined
    {
      // The conditions have been defined. Now calculate the covariance matrix.
      for (jcon=0; jcon<GLM.nconditions; jcon++)
	{
	  // Allocate the F covariance matrix for each condition.
	  GLM.FCovar[jcon]    = float_matrix(GLM.ncontrasts[jcon],GLM.ncontrasts[jcon]);
	  GLM.FCovarInv[jcon] = float_matrix(GLM.ncontrasts[jcon],GLM.ncontrasts[jcon]);
	  // Define the F convariance matrix for each condition.
	  for (jc1=0; jc1<GLM.ncontrasts[jcon]; jc1++)
	    {
	      for (jc2=0; jc2<GLM.ncontrasts[jcon]; jc2++)
		{
		  GLM.FCovar[jcon][jc1][jc2] = 0.;
		  for (jcoeff=0; jcoeff<GLM.ncoeff; jcoeff++)
		    {
		      for (jcoeff1=0; jcoeff1<GLM.ncoeff; jcoeff1++)
			{
			  GLM.FCovar[jcon][jc1][jc2]  +=   GLM.contrast_matrix[jcon][jc1][jcoeff]
			    * GLM.XTXm1_cc[jcoeff][jcoeff1] * GLM.contrast_matrix[jcon][jc2][jcoeff1];
			}
		    }
		}
	    }
	  // Make a copy for the inverse
	  for (jc1=0; jc1<GLM.ncontrasts[jcon]; jc1++)
	    {
	      for (jc2=0; jc2<GLM.ncontrasts[jcon]; jc2++)
		GLM.FCovarInv[jcon][jc1][jc2] = GLM.FCovar[jcon][jc1][jc2];
	    }
	  // Now invert the matrix.
	  rInvertSquareMatrix( GLM.ncontrasts[jcon], GLM.FCovarInv[jcon], &iRowFail );
	}
    }

  return(0);
}

int GLM_calculate_F(int ntime,          // input: number of time points
		    float sigma2,       // input: variance between data and fit
		    JIP_INDEX resels,         // resolution elements (for multiple comparisons)
		    int lCondition,     // an index between 0 and GLM.nconditions-1
		    float *cnr,         // output cnr    for the relevant condition
		    float *p_raw,       // output raw p-value for the relevant condition
		    float *p_cor,       // output corrected p-value for the relevant condition
		    float *effect_size, // size of effect
		    float *effect_sdev) // stardard deviation of effect
{
  float Fstat, df1, df2, numerator, ec3, x;
  float *CBeta, CBetaMax, beta;
  float too_small = exp(-100.), ec3constant=.1169;
  int lContrastMax;
  int jcoeff, jc1, jc2;

  // 
  if ( sigma2 == 0. )
    {
      *cnr = *effect_size = *effect_sdev = 0.;
      *p_raw = *p_cor = 1.;
      return(0);
    }

  // Make sure the condition is within range.
  if ( lCondition < 0 || lCondition >= GLM.nconditions )
    {
      printf("Error: the specified condition (%d) is out of range (0-%d)\n",
	     lCondition,GLM.nconditions);
      return(1);
    }
  else if ( ntime != GLM.ntime )
    { // Make sure that the number of time points is consistent with the number used to fit the GLM.
      printf("Error: # time points != # used in GLM fit.\n");
      return(1);
    }

  CBeta = float_vector(GLM.ncontrasts[lCondition]);
  // Calculate the product CBeta
  for (jc1=0, lContrastMax=0, CBetaMax=0.;
       jc1<GLM.ncontrasts[lCondition];
       jc1++)
    {
      CBeta[jc1] = 0;
      for (jcoeff=0; jcoeff<GLM.ncoeff; jcoeff++)
	CBeta[jc1] += GLM.contrast_matrix[lCondition][jc1][jcoeff] * GLM.Beta_c[jcoeff];
      if ( fabs((double)CBeta[jc1]) > fabs((double)CBetaMax) )
	{
	  CBetaMax = CBeta[jc1];
	  lContrastMax = jc1;
	}
    }
  // Determine the effect size and standard deviation.
  *effect_size = CBetaMax;
  *effect_sdev = sqrt((double)(GLM.FCovar[lCondition][lContrastMax][lContrastMax] * sigma2));

  // Compute the numerator for F
  numerator = 0.;
  for (jc1=0; jc1<GLM.ncontrasts[lCondition]; jc1++)
    {
      for (jc2=0; jc2<GLM.ncontrasts[lCondition]; jc2++)
	numerator += CBeta[jc1]
	  * GLM.FCovarInv[lCondition][jc1][jc2] * CBeta[jc2];
    }
  free_float_vector(CBeta);

  // Degrees of freedom
  df1 = GLM.dof;
  df2 = GLM.ncontrasts[lCondition];

  Fstat = numerator / sigma2 / df2;
  *cnr = sqrt((double)Fstat);

  // Calculate EC3 following Worsley et al., HBM 1996
  ec3 = ec3constant * ( (df1-1.)/df1*Fstat - 1. ) / pow((double)(1.+Fstat/df1),(double)((df1-1.)/2.));
  *p_cor = (float)resels * ec3;

  if ( CBetaMax < 0. ) *cnr *= -1.;

  if ( sigma2 == 0. )
    { // This case must be constant values (e.g., zeros)
      *cnr = 0.;
      *p_raw = 1.;
      *p_cor = 1.;
    }
  else
    {
      x = df1/(df1+df2*Fstat);
      if (x > 1.0) x = 1.;
      //      beta = betai(0.5*df1,0.5*df2,x);                     // numerical recipes
      beta = ibeta((double)(0.5*df1),(double)(0.5*df2),(double)x); // cephes
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
///////////// Functions that describe the GLM ////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void GLM_describe_input_files()
{
  int jrun;

  printf("\nFiles: FMRI (mandatory), GLM (mandatory), TABLE (optional)\n");
  printf("---------------------------------------------------------\n");
  for (jrun=0; jrun<GLM_input.nruns; jrun++)
    printf("%d %s %s %s\n",jrun+1,
	   GLM_input.run_name[jrun],GLM_input.glm_name[jrun],GLM_input.table_name[jrun]);
  printf("\n");
}

void GLM_describe_stimuli()
{
  char event_char, e1, e2;
  int jevent, jstim, event_number, icolumn;

  //
  // Output information about the GLM file.
  //
  if ( GLM.nruns == 1 )
    printf("GLM description: %d time points in %d run with TR = %4.1f\n\n  stimuli:\n",
	   GLM.ntime,GLM.nruns,GLM.tr[0]);
  else
    printf("GLM description: %d time points in %d runs with TR = %4.1f\n\n  stimuli:\n",
	   GLM.ntime,GLM.nruns,GLM.tr[0]);
  /* Stimuli */
  printf("\n# events = %d\n\n",GLM.nevents);
  printf(" ON    OFF  STRENGTH\n");
  printf("----- ----- --------\n");
  for (jevent=0; jevent<GLM.nevents; jevent++)
    {
      event_number = GLM.event_number[jevent];
      if ( GLM_EventCharFromInteger(event_number,&event_char) )
	{
	  printf("Programming error: event number (%d) apparenty not defined.?!\n",event_number);
	  exit(1);
	}
      else if ( GLM.nstims[jevent] < 1 )
	{
	  printf("Error checking problem: # stims < 1 for event %c.\n",event_char);
	  printf("Check format of event description file.\n");
	  exit(1);
	}
      for (jstim=0; jstim<GLM.nstims[jevent]; jstim++)
	{
	  if ( GLM.event_shape[jevent] == SHAPE_SQUARE )
	    {
	      if ( jstim == 0 )
		printf("%5.1f %5.1f %8.2f (square event %c)\n",
		       GLM.stim_on[jevent][jstim],GLM.stim_off[jevent][jstim],GLM.stim_mag[jevent][jstim],
		       event_char);
	      else
		printf("%5.1f %5.1f %8.2f\n",
		       GLM.stim_on[jevent][jstim],GLM.stim_off[jevent][jstim],GLM.stim_mag[jevent][jstim]);
	    }
	  else if ( GLM.event_shape[jevent] == SHAPE_RAMPUP )
	    {
	      if ( jstim == 0 )
		printf("%5.1f %5.1f %8.2f (ramp-up event %c)\n",
		       GLM.stim_on[jevent][jstim],GLM.stim_off[jevent][jstim],GLM.stim_mag[jevent][jstim],
		       event_char);
	      else
		printf("%5.1f %5.1f %8.2f\n",
		       GLM.stim_on[jevent][jstim],GLM.stim_off[jevent][jstim],GLM.stim_mag[jevent][jstim]);
	    }
	  else if ( GLM.event_shape[jevent] == SHAPE_RAMPDOWN )
	    {
	      if ( jstim == 0 )
		printf("%5.1f %5.1f %8.2f (ramp-down event %c)\n",
		       GLM.stim_on[jevent][jstim],GLM.stim_off[jevent][jstim],GLM.stim_mag[jevent][jstim],
		       event_char);
	      else
		printf("%5.1f %5.1f %8.2f\n",
		       GLM.stim_on[jevent][jstim],GLM.stim_off[jevent][jstim],GLM.stim_mag[jevent][jstim]);
	    }
	  else if ( GLM.event_shape[jevent] == SHAPE_SINE )
	    {
	      if ( jstim == 0 )
		printf("%5.1f %5.1f %8.2f (sine event %c, period = %g)\n",
		       GLM.stim_on[jevent][jstim],GLM.stim_off[jevent][jstim],GLM.stim_mag[jevent][jstim],
		       event_char,GLM.event_tau1[jevent]);
	      else
		printf("%5.1f %5.1f %8.2f\n",
		       GLM.stim_on[jevent][jstim],GLM.stim_off[jevent][jstim],GLM.stim_mag[jevent][jstim]);
	    }
	  else if ( GLM.event_shape[jevent] == SHAPE_COSINE )
	    {
	      if ( jstim == 0 )
		printf("%5.1f %5.1f %8.2f (cosine event %c, period = %g)\n",
		       GLM.stim_on[jevent][jstim],GLM.stim_off[jevent][jstim],GLM.stim_mag[jevent][jstim],
		       event_char,GLM.event_tau1[jevent]);
	      else
		printf("%5.1f %5.1f %8.2f\n",
		       GLM.stim_on[jevent][jstim],GLM.stim_off[jevent][jstim],GLM.stim_mag[jevent][jstim]);
	    }
	  else if ( GLM.event_shape[jevent] == SHAPE_GAMMA )
	    {
	      if ( jstim == 0 )
		printf("%5.1f %5.1f %8.2f (gamma event %c, tau = %5.1f)\n",
		       GLM.stim_on[jevent][jstim],GLM.stim_off[jevent][jstim],GLM.stim_mag[jevent][jstim],
		       event_char,GLM.event_tau1[jevent]);
	      else
		printf("%5.1f %5.1f %8.2f\n",
		       GLM.stim_on[jevent][jstim],GLM.stim_off[jevent][jstim],GLM.stim_mag[jevent][jstim]);
	    }
	  else if ( GLM.event_shape[jevent] == SHAPE_SIGMOID )
	    {
	      if ( jstim == 0 )
		printf("%5.1f %5.1f %8.2f (sigmoid event %c, tau = %5.1f)\n",
		       GLM.stim_on[jevent][jstim],GLM.stim_off[jevent][jstim],GLM.stim_mag[jevent][jstim],
		       event_char,GLM.event_tau1[jevent]);
	      else
		printf("%5.1f %5.1f %8.2f\n",
		       GLM.stim_on[jevent][jstim],GLM.stim_off[jevent][jstim],GLM.stim_mag[jevent][jstim]);
	    }
	  else if ( GLM.event_shape[jevent] == SHAPE_DRUGIRF )
	    {
	      if ( jstim == 0 )
		printf("%5.1f %5.1f %8.2f (drug-IRF event %c, tau = %g %g)\n",
		       GLM.stim_on[jevent][jstim],GLM.stim_off[jevent][jstim],
		       GLM.stim_mag[jevent][jstim],event_char,
		       GLM.event_tau1[jevent],GLM.event_tau2[jevent]);
	      else
		printf("%5.1f %5.1f %8.2f\n",
		       GLM.stim_on[jevent][jstim],GLM.stim_off[jevent][jstim],
		       GLM.stim_mag[jevent][jstim]);
	    }
	  else if ( GLM.event_shape[jevent] == SHAPE_TABLE )
	    {
	      icolumn = GLM.table_icolumn[jevent][jstim];
	      printf("%5.1f %5.1f          (table event %c, column %d -> %s)\n",
		     GLM.stim_on[jevent][jstim],GLM.stim_off[jevent][jstim],
		     event_char,icolumn,GLM.table_column_name[icolumn]);
	    }
	  else if ( GLM.event_shape[jevent] == SHAPE_INTERACT )
	    {
	      GLM_EventCharFromInteger((int)GLM.event_tau1[jevent],&e1);
	      GLM_EventCharFromInteger((int)GLM.event_tau2[jevent],&e2);
	      printf("%5.1f %5.1f          (interaction event %c, event %c times event %c)\n",
		     GLM.stim_on[jevent][jstim],GLM.stim_off[jevent][jstim],
		     event_char,e1,e2);
	    }
	  else
	    {
	      printf("Programming error: the event shape (%d) is unknown.\n",GLM.event_shape[jevent]);
	      exit(1);
	    }
	}
    }
  /* Exclusion segments: */
  for (jstim=0; jstim<GLM.nexclude; jstim++)
    {
      if ( jstim == 0 )
	printf("%5d %5d (excluded segments, units = time points)\n",
	       GLM.exclude_start[jstim],GLM.exclude_end[jstim]);
      else
	printf("%5d %5d\n",GLM.exclude_start[jstim],GLM.exclude_end[jstim]);
    }
}

void GLM_describe_basis_functions()
{
  FILE *plot_file;
  char event_char;
  int jcoeff, jtime, jrun;

  /*
  ** print the basis functions
  */
  if ( !(plot_file = fopen("basis.dat","w")) )
    {
      printf("Cannot open basis function output file ...\n");
      return;
    }
  fprintf(plot_file,"x ");
  for (jcoeff=0; jcoeff<GLM.nevents; jcoeff++)
    {
      GLM_EventCharFromInteger(GLM_get_event_number(jcoeff),&event_char);
      fprintf(plot_file,"event_%c ",event_char);
    }
  if ( GLM_input.nbaseline_terms > 0 )
    {
      for (jrun=0; jrun<GLM.nruns; jrun++)
	fprintf(plot_file,"offset%d ",jrun);
    }
  if ( GLM_input.nbaseline_terms > 1 )
    {
      for (jrun=0; jrun<GLM.nruns; jrun++)
	fprintf(plot_file,"slope%d ",jrun);
    }
  if ( GLM_input.nbaseline_terms > 2 )
    {
      for (jrun=0; jrun<GLM.nruns; jrun++)
	fprintf(plot_file,"quad%d ",jrun);
    }
  if ( GLM_input.nbaseline_terms > 3 )
    {
      for (jrun=0; jrun<GLM.nruns; jrun++)
	fprintf(plot_file,"triad%d ",jrun);
    }
  fprintf(plot_file,"\n");
  for (jtime=0; jtime<GLM.ntime; jtime++)
    {
      fprintf(plot_file,"%d ",jtime);
      for (jcoeff=0; jcoeff<GLM.ncoeff; jcoeff++)
	{
	  fprintf(plot_file,"%f ",GLM.Xorig_tc[jtime][jcoeff]);
	}
      fprintf(plot_file,"\n");
    }
  fclose(plot_file);
}

void GLM_describe_conditions()
{
  int jcondition, jcontrast, jcoeff;

  // Echo the condition strings:
  printf("Conditions to be tested: ");
  for (jcondition=0; jcondition<GLM.nconditions; jcondition++)
    printf("%s ",GLM.condition_string[jcondition]);
  printf("\n");

  // Describe each decoded condition
  for (jcondition=0; jcondition<GLM.nconditions; jcondition++)
    {
      printf("condition %s (%d contrasts):\n",
	     GLM.condition_string[jcondition],GLM.ncontrasts[jcondition]);
      for (jcontrast=0; jcontrast<GLM.ncontrasts[jcondition]; jcontrast++)
	{
	  for (jcoeff=0; jcoeff<GLM.ncoeff; jcoeff++)
	    printf("%d ",GLM.contrast_matrix[jcondition][jcontrast][jcoeff]);
	  printf("\n");
	}
    }
}

void GLM_describe_fit()
{
  int jcoeff;

  for (jcoeff=0; jcoeff<GLM.ncoeff; jcoeff++)
    printf("amplitude of basis function %d = %f\n",jcoeff,GLM.Beta_c[jcoeff]);
}

int GLM_get_condition_string( int ncondition, char *condition_string )
{
  if ( ncondition < 0 || ncondition >= GLM.nconditions )
    return(1);
  else
    strcpy(condition_string,GLM.condition_string[ncondition]);
  return(0);
}

int GLM_is_initialized()
{
  return(GLM.initialized);
}

int GLM_get_nruns()
{
  return(GLM.nruns);
}

int GLM_get_nbaseline_terms()
{
  return(GLM_input.nbaseline_terms);
}

int GLM_get_nconditions()
{
  return(GLM.nconditions);
}

int GLM_get_number_events()
{
  return(GLM.nevents);
}

int GLM_get_event_number(int index_event)
{
  if ( index_event < GLM.nevents )
    return(GLM.event_number[index_event]);
  else
    return(0);
}

int GLM_get_event_index(int event_number)
{
  int jevent, found, index_event;

  // Check to see if the event exists.
  for ( jevent=0, found=0; jevent<GLM.nevents; jevent++ )
    {
      if ( GLM.event_number[jevent] == event_number )
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

void GLM_set_time_constant( int index_event, float tau )
{
  int j;
  float t, tau1;

  if ( GLM.event_shape[index_event] == SHAPE_SQUARE   ||
       GLM.event_shape[index_event] == SHAPE_RAMPUP   ||
       GLM.event_shape[index_event] == SHAPE_RAMPDOWN ||
       GLM.event_shape[index_event] == SHAPE_TABLE )
    {
      printf("Warning: event %c (index %d) does have a time constant.\n",
	     GLM_EventChar(GLM_get_event_number(index_event)),index_event);
    }
  GLM.event_tau1[index_event] = tau;
  if ( GLM.event_shape[index_event] == SHAPE_DRUGIRF )
    {
      free_float_vector(GLM.drugIRF[index_event]);
      free_float_vector(GLM.plasmaIRF[index_event]);

      if ( HRF.time_step == 0. ) HRF.time_step = 1.;
      tau1 = GLM.event_tau2[index_event];
      GLM.length_drugIRF[index_event] = 10 * (int) (MAX(tau,tau1)/HRF.time_step);
      // The IRF functions will be put on the time scale of the HRF, if it exists.
      GLM.drugIRF[index_event]   = float_vector(GLM.length_drugIRF[index_event]);
      GLM.plasmaIRF[index_event] = float_vector(GLM.length_drugIRF[index_event]);
      for (j=0; j<GLM.length_drugIRF[index_event]; j++)
	{
	  t = HRF.time_step * j;
	  // Gamma function for drug IRF
	  GLM.drugIRF[index_event][j] = t/tau * exp(-(double)(t/tau));
	  // Exponential function for plasma IRF
	  GLM.plasmaIRF[index_event][j] = exp(-(double)(t/tau1));
	}
    }
  if ( GLM_define_basis_functions() )
    {
      printf("Error re-defining basis functions\n");
      exit(1);
    }
  GLM_describe_basis_functions();
  GLM_describe_stimuli();
}

float GLM_get_time_constant(int index_event )
{
  if ( GLM.event_shape[index_event] == SHAPE_SQUARE   ||
       GLM.event_shape[index_event] == SHAPE_RAMPUP   ||
       GLM.event_shape[index_event] == SHAPE_RAMPDOWN ||
       GLM.event_shape[index_event] == SHAPE_TABLE )
    {
      printf("Warning: event %c (index %d) does have a time constant.\n",
	     GLM_EventChar(GLM_get_event_number(index_event)),index_event);
    }
  return GLM.event_tau1[index_event];
}

float GLM_get_effect_size(int index_event )
{
  return(GLM.Beta_c[index_event]);
}

int GLM_get_tr( int irun, float *tr )
{
  if ( irun < 0 || irun >= GLM.nruns )
    return(1);
  else
    *tr = GLM.tr[irun];
  return(0);
}

float GLM_get_maximal_effect_size(int lCondition )
{ // index_coeff should be the same as index_event
  int jtime, jc, jcoeff;
  float *CBeta, y, max;

  CBeta = float_vector(GLM.ncontrasts[lCondition]);
  // Loop over time points, and find the maximal value for given condition.
  for (jtime=0, max=0.; jtime<GLM.ntime; jtime++)
    {
      // Calculate the product CBeta for the given condition.
      for (jc=0; jc<GLM.ncontrasts[lCondition]; jc++)
	{
	  // Find C*Beta
	  CBeta[jc] = 0;
	  for (jcoeff=0; jcoeff<GLM.ncoeff; jcoeff++)
	    CBeta[jc] += GLM.contrast_matrix[lCondition][jc][jcoeff] * GLM.Beta_c[jcoeff];
	}
      // Now find the maximum value of y = X * C*Beta
      for (jc=0; jc<GLM.ncontrasts[lCondition]; jc++)
	{
	  y = GLM.X_tc[jtime][jc] * CBeta[jc];
	  if ( fabs((double)y) > fabs((double)max) ) max = y;
	}
    }
  free_float_vector(CBeta);
  return(max);
}

void GLMInternal_get_start_stop_times( int iRun, float *start, float *stop)
{
  int jrun;
  float time_total, time_run;

  if ( iRun >= GLM.nruns )
    {
      printf("Programming error: run %d does not exist\n",iRun);
      exit(1);
    }
  time_total = 0.;
  for ( jrun=0; jrun<=iRun; jrun++ )
    {
      time_run = ( GLM.run_stop[jrun]  - GLM.run_start[jrun] ) * GLM.tr[jrun];
      time_total += time_run;
    }
  *stop  = time_total;
  *start = time_total - time_run;
}

int GLM_get_number_excluded_regions()
{
  return(GLM.nexclude);
}

int GLM_get_excluded_region( int index_exclude, int *exclude_start, int *exclude_end)
{
  *exclude_start = GLM.exclude_start[index_exclude];
  *exclude_end   = GLM.exclude_end[index_exclude];
  return(0);
}

int GLM_get_DOF()
{
  int DOF, jtime;

  DOF = 0;
  for (jtime=0; jtime<GLM.ntime; jtime++)
    DOF += GLM.include_time[jtime];
  DOF -= GLM.ncoeff;
  return(DOF);
}

int GLM_is_excluded( int index_time )
{
  int excluded;
  if ( !GLM.initialized )
    return(0);
  excluded = !GLM.include_time[index_time];
  return(excluded);
}

int GLM_get_stimulus(int index_event, float *stim_on, float *stim_off)
     // stim_on, stim_off in units of time points
     // *** Note: stim_off = stim_on + 1 for gamma function
     // return 1 if true (successfully got a stimulus) and 0 if false
{
  static JIP_INDEX index_stim;

  if ( index_event == -1 )
    {
      index_stim = -1;
      return(0);
    }

  index_stim++;
  if ( index_stim < GLM.nstims[index_event] )
    {
      *stim_on  = GLM.stim_on [index_event][index_stim] / GLM.tr[0];
      *stim_off = GLM.stim_off[index_event][index_stim] / GLM.tr[0];
      if ( GLM.event_shape[index_event] == SHAPE_GAMMA   ||
	   GLM.event_shape[index_event] == SHAPE_SIGMOID ||
	   GLM.event_shape[index_event] == SHAPE_SINE    ||
	   GLM.event_shape[index_event] == SHAPE_COSINE )
	*stim_off = *stim_on + 1; // this is for display purposes
      return(1);
    }
  else
    {
      index_stim = -1;
      return(0);
    }
}

int GLM_event_is_defined( int event_number )
{
  int jevent, found;

  for (jevent=0, found=0; jevent<GLM.nevents; jevent++)
    {
      if ( GLM.event_number[jevent] == event_number )
	found = 1;
    }
  return(found);
}
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
///////////// Functions used in interactive mode /////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

int GLM_set_nbaseline_terms( int nbaseline )
{
  if ( nbaseline < 0 || nbaseline > 4 )
    {
      printf("Error: # of baseline terms (%d) should be >=0 and <=4.\n",nbaseline);
      return(1);
    }
  else
    {
      GLM_input.nbaseline_terms = nbaseline;
      return(0);
    }
}

int GLM_initialize( int ntime, int nruns )
{ // This function is useful when the GLM is being defined interactively.
  int jtime, jrun;

  if ( ntime == 0 || nruns == 0 )
    {
      printf("Error: zero value passed to GLM_initialized\n");
      return(1);
    }

  GLM.ntime = ntime;
  GLM.nruns = nruns;
  for (jrun=0; jrun<nruns; jrun++)
    GLM.tr[jrun] = 1.;

  if ( GLM.include_time )
    free_int_vector(GLM.include_time);
  GLM.include_time = int_vector(GLM.ntime);
  // Initialize the vector with 1's.
  for (jtime=0; jtime<GLM.ntime; jtime++)
    GLM.include_time[jtime] = 1;

  GLM.nevents = GLM.nexclude = GLM.nconditions = GLM.ncoeff = 0;

  GLM.initialized = 1;
  return(0);
}

void GLM_initialize_run( int index_run, float start, float stop )
{ // This function is useful when the GLM is being defined interactively.
  GLM.run_start[index_run] = start;
  GLM.run_stop[index_run]  = stop;
}

int GLM_define_table_event_interactive(int event_number, float *table_data)
{
  int index_event, found, icolumn;
  int jevent, jrun, jtime;
  static BOOL first_time = TRUE;
  
  if ( GLM.initialized == 0 )
    {
      printf("Error in GLM_define_1_stimulus: first initialize the GLM\n");
      return(1);
    }
  // Set the event number. If the event does not exist, add it.
  index_event = found = 0;
  for ( jevent=0; jevent<GLM.nevents; jevent++)
    {
      if ( event_number == GLM.event_number[jevent] )
	{
	  found++;
	  index_event = jevent;
	}
    }
  if ( !found )
    {
      index_event = GLM.nevents;
      GLM.nevents++;
    }
  else if ( found > 1 )
    {
      printf("Programming error in GLM_define_1_stimulus\n");
      exit(1);
    }
  if ( first_time )
    {
      for (jrun=0; jrun<GLM.nruns; jrun++)
	{
	  GLM.table_ntime[jrun] = GLM.run_stop[jrun] - GLM.run_start[jrun] + 1; // time points in "table"
	  GLM.table_value[jrun] = float_matrix(MAX_COLUMNS,GLM.table_ntime[jrun]);
	}
      first_time = FALSE;
    }

  // Set the stimulus event and shape (square, gamma, ...).
  GLM.event_number[index_event] = event_number;
  GLM.event_shape[index_event]  = SHAPE_TABLE;
  GLM.nstims[index_event]       = GLM.nruns; // set stims = runs (1 table per run)
  GLM.event_tau1[index_event]   = 0.;
  GLM.event_tau2[index_event]   = 0.;

  GLM.convolve_table[event_number]  = 0;
  GLM.event_scale[event_number]     = 1.;
  for (jrun=0; jrun<GLM.nruns; jrun++)
    {
      GLM.stim_on [index_event][jrun] = GLM.run_start[jrun]; // start run
      GLM.stim_off[index_event][jrun] = GLM.run_stop [jrun]; // end run
      GLM.table_ncolumns[jrun]++;                            // add another "column" for each call
      icolumn = GLM.table_ncolumns[jrun] - 1;               // last column
      GLM.table_icolumn[index_event][jrun] = icolumn;
      sprintf(GLM.table_column_name[icolumn],"event%d",event_number);
      for (jtime=0; jtime<GLM.table_ntime[jrun]; jtime++)
	GLM.table_value[jrun][icolumn][jtime] = table_data[GLM.run_start[jrun]+jtime];
    }

  return(0);
}

int GLM_define_1_stimulus(int event_number, int ishape,
			  float TimePoint_on, float TimePoint_off, float TimePoint_tau)
{
  int nevents_save, index_event, irun, found;JIP_INDEX index_stim;
  int jevent, jstim, jrun;
  float stim_on, stim_off, stim_off_select, event_tau;
  
  stim_on   = TimePoint_on  * GLM.tr[0];
  stim_off  = TimePoint_off * GLM.tr[0];
  event_tau = TimePoint_tau * GLM.tr[0];

  if ( GLM.initialized == 0 )
    {
      printf("Error in GLM_define_1_stimulus: first initialize the GLM\n");
      return(1);
    }

  nevents_save = GLM.nevents;

  // Set the event number. If the event does not exist, add it.
  index_event = found = 0;
  for ( jevent=0; jevent<GLM.nevents; jevent++)
    {
      if ( event_number == GLM.event_number[jevent] )
	{
	  found++;
	  index_event = jevent;
	}
    }
  if ( !found )
    {
      index_event = GLM.nevents;
      GLM.nevents++;
    }
  else if ( found > 1 )
    {
      printf("Programming error in GLM_define_1_stimulus\n");
      exit(1);
    }
  // Set the stimulus event and shape (square, gamma, ...).
  GLM.event_number[index_event] = event_number;
  GLM.event_shape[index_event]  = ishape;
  GLM.event_scale[event_number] = 1.;

  if ( event_tau == 0. ) // Don't let tau be zero.
    GLM.event_tau1[index_event] = (stim_off - stim_on)/2.;
  else
    GLM.event_tau1[index_event] = event_tau;
  GLM.event_tau2[index_event] = 0.;

  // Set or increment the number of stimuli for this event.
  if ( !found )
    {
      index_stim = 0;
      GLM.nstims[index_event] = 1;
    }
  else
    {
      index_stim = found = 0;
      for (jstim=0; jstim<GLM.nstims[index_event]; jstim++)
	{
	  if ( GLM.event_shape[index_event] == SHAPE_GAMMA ||
	       GLM.event_shape[index_event] == SHAPE_SIGMOID ) // extend the apparent length of GAMMA/SIGMOID
	    stim_off_select = GLM.stim_on[index_event][jstim] + GLM.event_tau1[index_event];
	  else
	    stim_off_select = GLM.stim_off[index_event][jstim];
	  if ( (stim_off >=GLM.stim_on[index_event][jstim] && stim_off<=stim_off_select)
	       ||(stim_on>=GLM.stim_on[index_event][jstim] && stim_on <=stim_off_select)
	       ||(stim_on<=GLM.stim_on[index_event][jstim] && stim_off>=stim_off_select) )
	    {
	      found++;
	      index_stim = jstim;
	    }
	}
      if ( !found )
	{
	  index_stim = GLM.nstims[index_event];
	  GLM.nstims[index_event]++;
	}
      else if ( found > 1 )
	{
	  // For a given event (known), stimuli cannot overlap in time.
	  printf("Error: stimuli of the same event (%d) cannot overlap.\n",
		 GLM.event_number[index_event]);
	  GLM.nevents = nevents_save;
	  return(1);
	}
    }

  // For all stimulus types, set the new "on time". Set the magnitude to 1.
  GLM.stim_on[index_event][index_stim]  = stim_on;
  GLM.stim_mag[index_event][index_stim] = 1.;
  if ( !found )
    {
      if ( ishape != SHAPE_GAMMA && ishape != SHAPE_SIGMOID )
	GLM.stim_off[index_event][index_stim] = stim_off;
      else // don't change off time for gamma/sigmoid function (end of run),
	{  // but set the off time if this is a new event.
	  // Find the run that contains stim_on
	  for ( jrun=0; jrun<GLM.nruns; jrun++)
	    {
	      if ( stim_on >= GLM.run_start[jrun] * GLM.tr[0] &&
		   stim_on <= GLM.run_stop[jrun]  * GLM.tr[0] )
		irun = jrun;
	    }      
	  GLM.stim_off[index_event][index_stim] = GLM.run_stop[irun] * GLM.tr[0];
	}
   }
  else if ( ishape == SHAPE_GAMMA || ishape == SHAPE_SIGMOID )
    {
      for (jrun=0; jrun<GLM.nruns; jrun++)
	{
	  if ( TimePoint_off < GLM.run_stop[jrun] )
	    {
	      stim_off = GLM.run_stop[jrun] * GLM.tr[0];
	      break;
	    }
	}
      GLM.stim_off[index_event][index_stim] = stim_off;
      printf("set stim off to %f\n",GLM.stim_off[index_event][index_stim]);
    }
  else
    GLM.stim_off[index_event][index_stim] = stim_off;

  return(0);
}

int GLM_delete_1_stimulus(int event_number, float location_TimePoint)
{
  float location_time, stim_off;
  int found;JIP_INDEX index_event, index_stim, index_this_event, index_this_stim;
  JIP_INDEX jevent, jstim, jtime;

  location_time = location_TimePoint * GLM.tr[0];

    if ( GLM.initialized == 0 )
      {
	printf("Error in GLM_define_1_stimulus: first initialize the GLM\n");
	return(1);
      }

  // Loop over events and stimuli to find the location.
  found = 0;
  index_event = index_this_event = index_stim = -1;
  for (jevent=0; jevent<GLM.nevents; jevent++)
    {
      for (jstim=0; jstim<GLM.nstims[jevent]; jstim++)
	{
	  if ( GLM.event_shape[jevent] != SHAPE_GAMMA && GLM.event_shape[jevent] != SHAPE_SIGMOID )
	    stim_off = GLM.stim_off[jevent][jstim];
	  else
	    stim_off = GLM.stim_on[jevent][jstim] + GLM.event_tau1[jevent];
	  if ( location_time >= GLM.stim_on[jevent][jstim] && location_time <= stim_off )
	    { // Save values corresponding to any stimulus at this location.
	      index_event = jevent;
	      index_stim = jstim;
	      found++;
	      if ( GLM.event_number[jevent] == event_number )
		{ // Save values that correspond to the event passed to this function.
		  index_this_event = jevent;
		  index_this_stim = jstim;
		}
	    }
	}
    }
  // How many were found?
  if ( !found )
    return(0);  // Ignore locations that don't correspond to stimuli.
  else if ( found > 1 )
    { // This must represent different events, since stimuli cannot overlap for a single event.
      if ( index_this_event >= 0 && index_this_stim >= 0 )
	{ // then the situation is no longer ambiguous; use this event.
	  index_event = index_this_event;
	  index_stim = index_this_stim;
	}
      else
	{
	  printf("Ambiguous: more than 1 event (!= %d) was found at this location.\n",
		 event_number);
	  return(1);
	}
    }
  // The location specifies a unique (event,stim):
  if ( GLM.event_shape[jevent] == SHAPE_TABLE  ||
       GLM.event_shape[jevent] == SHAPE_SINE   || 
       GLM.event_shape[jevent] == SHAPE_COSINE || 
       GLM.event_shape[jevent] == SHAPE_DRUGIRF )
   return(0); // Only allow square or gamma types to be deleted.

  // Remove the selected stimulus.
  printf("delete stimulus of event %c at location %f\n",
	 GLM_EventChar(GLM.event_number[index_event]),location_time);
  for (jstim=index_stim; jstim<GLM.nstims[index_event]; jstim++)
    { // copy other stims down in the arrays
      GLM.stim_on[index_event][jstim]   = GLM.stim_on[index_event][jstim+1];
      GLM.stim_off[index_event][jstim]  = GLM.stim_off[index_event][jstim+1];
      GLM.stim_mag[index_event][jstim]  = GLM.stim_mag[index_event][jstim+1];
    }
  GLM.nstims[index_event]--;
  // If this was the only stimulus for the event, remove the event.
  if ( GLM.nstims[index_event] == 0 )
    {
      for (jevent=index_event; jevent<GLM.nevents; jevent++)
	{ // copy other events down in the arrays
	  GLM.event_number[jevent] = GLM.event_number[jevent+1];
	  GLM.event_shape[jevent]  = GLM.event_shape[jevent+1];
	  GLM.event_tau1[jevent]   = GLM.event_tau1[jevent+1];
	  GLM.event_tau2[jevent]   = GLM.event_tau2[jevent+1];
	  GLM.nstims[jevent]       = GLM.nstims[jevent+1];
	  for (jstim=0; jstim<GLM.nstims[jevent]; jstim++)
	    {
	      GLM.stim_on  [jevent][jstim] = GLM.stim_on  [jevent+1][jstim];
	      GLM.stim_off [jevent][jstim] = GLM.stim_off [jevent+1][jstim];
	      GLM.stim_mag [jevent][jstim] = GLM.stim_mag [jevent+1][jstim];
	      GLM.table_icolumn[jevent][jstim] = GLM.table_icolumn[jevent+1][jstim];
	    }
	  // drugIRF vector
	  GLM.length_drugIRF[jevent] = GLM.length_drugIRF[jevent+1];
	  if ( GLM.drugIRF[jevent]   != NULL ) free_float_vector(GLM.drugIRF[jevent]);
	  if ( GLM.plasmaIRF[jevent] != NULL ) free_float_vector(GLM.plasmaIRF[jevent]);
	  if ( GLM.length_drugIRF[jevent] != 0 )
	    {
	      GLM.drugIRF[jevent]   = float_vector(GLM.length_drugIRF[jevent]);
	      GLM.plasmaIRF[jevent] = float_vector(GLM.length_drugIRF[jevent]);
	      for (jtime=0; jtime<GLM.length_drugIRF[jevent]; jtime++)
		{
		  GLM.drugIRF[jevent][jtime]   = GLM.drugIRF[jevent+1][jstim];
		  GLM.plasmaIRF[jevent][jtime] = GLM.plasmaIRF[jevent+1][jstim];
		}
	    }
	}
      GLM.nevents--;
    }

  return(0);
}

int GLM_identify_time_point(int iTime,            // location of timepoint
			    char *residue_string, // list of events that have been removed
			    char *output_string)  // output string describing time point
{
  // do not include table events, which span all time points
  // do not include events that have been excluded from the GLM
  float location_time, stim_off;
  int found;JIP_INDEX index_event, index_stim;
  int jevent, jstim;

  if ( GLM.initialized == 0 )
    {
      printf("Error in GLM_identify_stimulus: first initialize the GLM\n");
      return(1);
    }
  else if ( iTime < 0 || iTime >= GLM.ntime )
    {
      printf("Error: time point (%d) is outside the GLM (0 to %d)\n",iTime,GLM.ntime);
      return(1);
    }
  else
    location_time = iTime * GLM.tr[0];

  // Check to see if it is excluded from the GLM.
  if ( !GLM.include_time[iTime] )
    {
      //      sprintf(output_string,"GLM: ignored time point");
      sprintf(output_string,"ignored");
      return(0);
    }

  // Loop over events and stimuli to find the location.
  found = 0;
  index_event = index_stim = -1;
  for (jevent=0; jevent<GLM.nevents; jevent++)
    {
      if ( GLM.event_shape[jevent] != SHAPE_TABLE && !GLM_event_is_in_string(residue_string,jevent) )
	{
	  for (jstim=0; jstim<GLM.nstims[jevent]; jstim++)
	    {
	      if ( GLM.event_shape[jevent] != SHAPE_GAMMA || GLM.event_shape[jevent] != SHAPE_SIGMOID )
		stim_off = GLM.stim_off[jevent][jstim];
	      else
		stim_off = GLM.stim_on[jevent][jstim] + GLM.event_tau1[jevent];
	      if ( location_time >= GLM.stim_on[jevent][jstim] &&
		   location_time <= stim_off )
		{ // Save values corresponding to any stimulus at this location.
		  index_event = jevent;
		  index_stim = jstim;
		  found++;
		}
	    }
	}
    }
  // How many were found?
  if ( !found )
    {
      //      sprintf(output_string,"GLM: %d runs, %d events",GLM.nruns,GLM.nevents);
      sprintf(output_string," ");
      return(0);
    }
  else if ( found > 1 )
    { // This must represent different events, since stimuli cannot overlap for a single event.
      sprintf(output_string,"%d stimuli",found);
      return(0);
    }

  /*
    if ( GLM.event_shape[index_event] == SHAPE_SQUARE )
    sprintf(output_string,"GLM: square event %c (%g-%g)",
    GLM_EventChar(GLM.event_number[index_event]),
    GLM.stim_on[index_event][index_stim],GLM.stim_off[index_event][index_stim]);
    else if ( GLM.event_shape[index_event] == SHAPE_RAMPUP )
    sprintf(output_string,"GLM: ramp-up event %c (%g-%g)",
    GLM_EventChar(GLM.event_number[index_event]),
    GLM.stim_on[index_event][index_stim],GLM.stim_off[index_event][index_stim]);
    else if ( GLM.event_shape[index_event] == SHAPE_RAMPDOWN )
    sprintf(output_string,"GLM: ramp-down event %c (%g-%g)",
    GLM_EventChar(GLM.event_number[index_event]),
    GLM.stim_on[index_event][index_stim],GLM.stim_off[index_event][index_stim]);
    else if ( GLM.event_shape[index_event] == SHAPE_GAMMA )
    sprintf(output_string,"GLM: gamma event %c @ %g, tau = %g",
    GLM_EventChar(GLM.event_number[index_event]),GLM.stim_on[index_event][index_stim],
    GLM.event_tau1[index_event]);
    else if ( GLM.event_shape[index_event] == SHAPE_DRUGIRF )
    sprintf(output_string,"GLM: drug-IRF event %c (%g-%g)",
    GLM_EventChar(GLM.event_number[index_event]),
    GLM.stim_on[index_event][index_stim],GLM.stim_off[index_event][index_stim]);
  */

  if ( GLM.event_shape[index_event] == SHAPE_SQUARE )
    sprintf(output_string,"%c",
	    GLM_EventChar(GLM.event_number[index_event]));
  else if ( GLM.event_shape[index_event] == SHAPE_RAMPUP )
    sprintf(output_string,"%d",
	    GLM_EventChar(GLM.event_number[index_event]));
  else if ( GLM.event_shape[index_event] == SHAPE_RAMPDOWN )
    sprintf(output_string,"%c",
	    GLM_EventChar(GLM.event_number[index_event]));
  else if ( GLM.event_shape[index_event] == SHAPE_GAMMA )
    sprintf(output_string,"%c",
	    GLM_EventChar(GLM.event_number[index_event]));
  else if ( GLM.event_shape[index_event] == SHAPE_SIGMOID )
    sprintf(output_string,"%c",
	    GLM_EventChar(GLM.event_number[index_event]));
  else if ( GLM.event_shape[index_event] == SHAPE_DRUGIRF )
    sprintf(output_string,"%c",
	    GLM_EventChar(GLM.event_number[index_event]));

  return(0);
}

int GLM_increment_event_number( int ievent )
{
  // This function is designed for an interactive program
  // that steps events based upon the existing events.
  int jevent, event_number, found, event_number_new, index_event;

  if ( GLM.nevents == 0 )
    { // Toggle between 1 and -1
      if ( ievent == 0 || ievent == -1 )
	event_number_new = 1;
      else
	event_number_new = -1;
    }
  else
    {
      // If ievent = -1, return the first event.
      if ( ievent == -1 )
	event_number_new = GLM.event_number[0];
      else
	{
	  // Check to see if the event exists.
	  for ( jevent=0, found=0; jevent<GLM.nevents; jevent++ )
	    {
	      if ( ievent == GLM.event_number[jevent] )
		{
		  found = 1;
		  index_event = jevent;
		}
	    }
	  if ( !found ) // then ievent represents the next event ...
	    { // ... so go back to -1 (ignore)
	      event_number_new = -1;
	    }
	  else // ievent is an existing event ...
	    {
	      // If this is not the last event in the list, go to the next one.
	      if ( index_event < GLM.nevents-1 )
		event_number_new = GLM.event_number[index_event+1];
	      else
		{ // return an used event (lowest possible) for the next event
		  for (event_number=1; event_number<MAX_EVENTS; event_number++)
		    {
		      for ( jevent=0, found=0; jevent<GLM.nevents; jevent++ )
			{
			  if ( event_number == GLM.event_number[jevent] )
			    found = 1;
			}
		      if ( !found )
			{
			  event_number_new = event_number;
			  break;
			}
		    }
		}
	    }
	}
    }
  return(event_number_new);
}

int GLM_event_fit( int baseline, char *residue_string, int index_event, int index_time, float *fit )
{
  int jcoeff, include_event;

  if ( index_time >= GLM.ntime )
    {
      printf("Error: time index (%d) >= GLM time points (%d) in GLM_fit_residue()\n",
	     index_time,GLM.ntime);
      return(1);
    }
  else if ( GLM.ncoeff <= 0 )
    {
      printf("Error: no coefficients assigned yet in GLM_fit_residue.\n");
      return(1);
    }

  *fit = 0.;
  // Add the terms of no interest.
  if ( baseline )
    {
      for ( jcoeff=GLM.ncoeff-GLM.nbaseline; jcoeff < GLM.ncoeff; jcoeff ++ )
	*fit += GLM.Xorig_tc[index_time][jcoeff] * GLM.Beta_c[jcoeff];
    }

  // Possibly add the event, depending upon the residue value.
  if ( jip_int_count((JIP_INDEX)strlen(residue_string)) == 0 )
    include_event = 1;
  else
    include_event = ! GLM_event_is_in_string(residue_string, index_event);

  if ( include_event )
    *fit += GLM.Xorig_tc[index_time][index_event] * GLM.Beta_c[index_event];

  return(0);
}

int GLM_create_all_conditions( char *condition_string )
{ // Create a condition string to feed into GLM_define_conditions
  char condition1[LEN_STR];
  int jevent, jevent1;

  if ( GLM.nevents == 0 )
    {
      printf("Error in GLM_create_all_conditions: no defined stimuli\n");
      return(1);
    }
  else
    sprintf(condition_string,"%d ",GLM.event_number[0]);
  // Make all single conditions
  for ( jevent=1; jevent<GLM.nevents; jevent++ )
    {
      sprintf(condition1,"%d ",GLM.event_number[jevent]);
      strcat(condition_string,condition1);
    }
  // Make all pair-wise summed combinations.
  for ( jevent=0; jevent<GLM.nevents; jevent++ )
    {
      for ( jevent1=jevent+1; jevent1<GLM.nevents; jevent1++ )
	{
	  sprintf(condition1,"%d%d ",GLM.event_number[jevent],GLM.event_number[jevent1]);
	  strcat(condition_string,condition1);
	}
    }
  // Make all pair-wise subtractions.
  for ( jevent=0; jevent<GLM.nevents; jevent++ )
    {
      for ( jevent1=jevent+1; jevent1<GLM.nevents; jevent1++ )
	{
	  sprintf(condition1,"%d-%d ",GLM.event_number[jevent],GLM.event_number[jevent1]);
	  strcat(condition_string,condition1);
	}
    }
  return(0);
}

int GLM_EventNumberFromChar(char event_char)
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

int GLM_EventCharFromInteger(int event_number, char *event_char)
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

char GLM_EventChar(int event_number)
{
  char event_char;

  if ( GLM_EventCharFromInteger(event_number, &event_char) )
    {
      printf("Programming error: event number %d apparently is not defined!\n",event_number);
      exit(1);
    }
  return event_char;
}

int GLM_EventStringFromInteger(int event_number, char *event_string)
{
  char event_char, event_string_temp[LEN_STR];

  if ( GLM_EventCharFromInteger(event_number,&event_char) )
    {
      printf("Error: event %d is not defined.\n",event_number);
      return(1);
    }
  strcpy(event_string_temp," ");
  event_string_temp[0] = event_char;
  strcpy(event_string,event_string_temp);
  return(0);
}

void GLM_define_table_residue(char *residue_string)
{ // Put all defined table events into a residue integer.
  char event_string[LEN_STR];
  int jevent, event_number;

  strcpy(residue_string,"");
  for (jevent=0; jevent<GLM.nevents; jevent++)
    { // Loop over all events.
      if ( GLM.event_shape[jevent] == SHAPE_TABLE )
	{
	  event_number = GLM.event_number[jevent];
	  GLM_EventStringFromInteger(event_number,event_string);
	  strcat(residue_string,event_string);
	}
    }
  return;
}

void GLM_define_residue_from_inclusion(char *include_string, char *residue_string)
{ // Put all defined table events into a residue integer.
  char event_string[LEN_STR];
  int jevent, event_number;

  strcpy(residue_string,"");
  for (jevent=0; jevent<GLM.nevents; jevent++)
    { // Loop over all events.
      if ( !GLM_event_is_in_string(include_string, jevent) )
	{
	  event_number = GLM.event_number[jevent];
	  GLM_EventStringFromInteger(event_number,event_string);
	  strcat(residue_string,event_string);
	}
    }
}

int GLM_invalid_residue(char *residue_string)
     // return the invalid event number if one exists in residue_string
{
  int event_number;JIP_INDEX j;
  char event_char;

  if ( jip_int_count((JIP_INDEX)strlen(residue_string)) == 0 ) return(0);
  for (j=0; j<jip_int_count((JIP_INDEX)strlen(residue_string)); j++)
    {
      event_char   = residue_string[j];
      event_number = GLM_EventNumberFromChar(event_char);
      // Make sure the event is valid.
      if ( event_number != 0 && !GLM_event_is_defined(event_number) < 0 )
	return(event_number);
    }
  return(0);
}

int GLM_event_is_in_string(char *string, int index_event)
{
  int event_number;JIP_INDEX j;
  char event_char;

  if ( GLM_invalid_residue(string) )
    {
      printf("Programming error: test for residue validity first.\n");
      exit(1);
    }

  // Find the event corresponding to the given coefficient.
  if ( jip_int_count((JIP_INDEX)strlen(string)) == 0 ) return(0);
  for (j=0; j<jip_int_count((JIP_INDEX)strlen(string)); j++)
    {
      event_char   = string[j];
      event_number = GLM_EventNumberFromChar(event_char);
      if ( event_number == GLM.event_number[index_event] ) return(1);
    }
  return(0);
}

int GLM_create_residue( int ntime, float *data_vector, float *fit_vector )
{
  int jcoeff, jtime;

  if ( ntime != GLM.ntime )
    {
      printf("Error: # time points (%d) does not match GLM (%d) in GLM_adjust_event()\n",
	     ntime,GLM.ntime);
      return(1);
    }
  else if ( GLM.ncoeff < 1 )
    {
      printf("Error: no coefficients assigned yet in GLM_adjust.\n");
      return(1);
    }

  // Loop over drift terms, removing each from all the runs.
  for (jtime=0; jtime<GLM.ntime; jtime++)
    {
      for ( jcoeff=0; jcoeff<GLM.ncoeff; jcoeff ++ )
	{
	  fit_vector[jtime]  -= GLM.X_tc[jtime][jcoeff] * GLM.Beta_c[jcoeff];
	  data_vector[jtime] -= GLM.X_tc[jtime][jcoeff] * GLM.Beta_c[jcoeff];
	}
    }

  return(0);
}

int GLM_create_selective_residue( char *residue_string, int ntime, float *data_vector, float *fit_vector )
     // residue_string: "" = remove no events, "1" = remove event 1, "12" = remove events 1 & 2, ...
{
  int event_number, remove_coefficient;
  int jcoeff, jtime;

  // ignore event numbers below zero.
  if ( jip_int_count((JIP_INDEX)strlen(residue_string)) == 0 )
    return(0);
  else
    {
      if ( (event_number=GLM_invalid_residue(residue_string)) )
	{
	  printf("Error: event %c is not valid in the residue (GLM_create_selective_residue)\n",
		 GLM_EventChar(event_number));
	  return(1);
	}
    }

  if ( ntime != GLM.ntime )
    {
      printf("Error: # time points (%d) does not match GLM (%d) in GLM_adjust_event()\n",
	     ntime,GLM.ntime);
      return(1);
    }
  else if ( GLM.ncoeff <= 0 )
    {
      printf("Error: no coefficients assigned yet in GLM_adjust.\n");
      return(1);
    }

  // Remove all other stimuli.
  for ( jcoeff=0; jcoeff < GLM.ncoeff-GLM.nbaseline; jcoeff ++ )
    {
      // remove this coefficient from both the data and the fit if it's in the residue
      remove_coefficient = GLM_event_is_in_string(residue_string, jcoeff);
      if ( remove_coefficient )
	{
	  for (jtime=0; jtime<GLM.ntime; jtime++)
	    {
	      data_vector[jtime] -= GLM.Xorig_tc[jtime][jcoeff] * GLM.Beta_c[jcoeff];
	      fit_vector[jtime]  -= GLM.Xorig_tc[jtime][jcoeff] * GLM.Beta_c[jcoeff];
	    }
	}
    }
  return(0);
}

int GLM_get_number_events_inside_residue( char *residue_string )
{
  int found_event[MAX_EVENTS];
  int jevent, event_number, nevents;

  if ( GLM_invalid_residue(residue_string) )
    {
      printf("Programming error: test for residue validity first.\n");
      exit(1);
    }

  for (jevent=0; jevent<GLM.nevents; jevent++)
    found_event[jevent] = 0;

  // Set values in the found_event array.
  for (jevent=0, nevents=0; jevent<GLM.nevents; jevent++)
    {
      if ( GLM_event_is_in_string(residue_string, jevent) )
	{
	  event_number = GLM.event_number[jevent];
	  found_event[event_number] = 1;
	}
    }
  // Count the number of values in the found_event array.
  for (jevent=0, nevents=0; jevent<MAX_EVENTS; jevent++)
    if ( found_event[jevent] ) nevents++;
  return(nevents);
}

int GLM_set_paradigm_shape( int event_number_input, int event_shape )
{
  int event_number, jevent;

  // Set the event shape for all events with the given event.
  for ( jevent=0; jevent<GLM.nevents; jevent++ )
    {
      event_number = GLM.event_number[jevent];
      if ( event_number == event_number_input )
	{
	  if ( event_shape == SHAPE_SQUARE ||  event_shape == SHAPE_GAMMA )
	    GLM.event_shape[jevent] = event_shape;
	  else
	    {
	      printf("Error: allowed shapes are 0 and 1.\n");
	      return(1);
	    }
	}
    }
  return(0);
}

int GLM_get_paradigm_shape( int event_number )
{
  int found, index_event, jevent;

  if ( event_number > MAX_EVENTS )
    {
      printf("Error: the event number (%d) is out of range in GLM_get_paradigm_shape().\n",
	     event_number);
      exit(1);
    }

  // return the lowest unused event
  for ( jevent=0, found=0; jevent<GLM.nevents; jevent++ )
    {
      if ( GLM.event_number[jevent] == event_number )
	{
	  found = 1;
	  index_event = jevent;
	}
    }
  if ( event_number <= 0 && GLM.nevents > 0 )
    return(GLM.event_shape[0]); // return the 1st shape
  else if ( !found )
    return(0);  // default shape: square
  else // if found
    return(GLM.event_shape[index_event]);
}

void GLM_modify_excluded_region( int mode, int start, int end )
{ // mode: 0 = toggle excluded points, 1 = add excluded points
  int jtime, ibinary;

  // Within the defined range passed to the function,
  // turn on to off and off to on.
  for (jtime=MAX(0,start); jtime<=MIN(end,GLM.ntime-1); jtime++)
    {
      if ( mode == 1 )
	GLM.include_time[jtime] = 0;
      else
	GLM.include_time[jtime] = !GLM.include_time[jtime];
    }
  // Now convert the vector back into a series of excluded ranges.
  for (jtime=0, GLM.nexclude=0, ibinary=1; jtime<GLM.ntime; jtime++)
    {
      if ( GLM.include_time[jtime] != ibinary )
	{ // then this is a switch point from 0 to 1 or 1 to 0
	  ibinary = !ibinary;
	  if ( ibinary == 0 ) // starting excluded region
	    GLM.exclude_start[GLM.nexclude] = jtime;
	  else // starting included region, so use jtime-1
	    {
	      GLM.exclude_end[GLM.nexclude] = MIN(jtime-1,GLM.ntime);
	      GLM.nexclude++;
	    }
	}
    }
  // Take care of last point.
  if ( GLM.include_time[GLM.ntime-1] == 0 )
    {
      GLM.exclude_end[GLM.nexclude] = GLM.ntime-1;
      GLM.nexclude++;
    }

  return;
}
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
/////////////Functions for simulations ///////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

int GLM_generate_synthetic_data1( int ibasis,
				  float contrast_scale,
				  float noise_max,
				  int ntime,
				  float *simdata )
{
  float noise;
  int jtime;

  /*
  ** Make sure the requested basis function is < # basis functions.
  */
  if ( ibasis >= GLM.ncoeff )
    {
      printf("Error: the requested basis function (%d) must be less than the total # of functions (%d).\n",
	     ibasis,GLM.ncoeff);
      return(1);
    }
  /*
  ** Make sure the GLM model and the input # points are consistent.
  */
  if ( ntime != GLM.ntime )
    {
      printf("Error: the number of data time points (%d) does not match the number in the GLM (%d).\n",
	     ntime,GLM.ntime);
      return(1);
    }

  for (jtime=0; jtime<GLM.ntime; jtime++)
    {
      noise = noise_max * (1. - 2.*RAND);
      simdata[jtime] = contrast_scale * GLM.X_tc[jtime][ibasis] + noise;
    }
  return(0);
}

int GLM_generate_synthetic_data2( int ibasis1, int ibasis2,
				  float contrast_scale1, float contrast_scale2,
				  float noise_max, int ntime, float *simdata )
{
  float noise;
  int jtime;

  /*
  ** Make sure the requested basis function is < # basis functions.
  */
  if ( ibasis1 >= GLM.ncoeff || ibasis2 >= GLM.ncoeff)
    {
      printf("Error: the 2 requested basis function (%d, %d) must be less than the total # of functions (%d).\n",
	     ibasis1,ibasis2,GLM.ncoeff);
      return(1);
    }
  /*
  ** Make sure the GLM model and the input # points are consistent.
  */
  if ( ntime != GLM.ntime )
    {
      printf("Error: the number of data time points (%d) does not match the number in the GLM (%d).\n",
	     ntime,GLM.ntime);
      return(1);
    }

  for (jtime=0; jtime<GLM.ntime; jtime++)
    {
      noise = noise_max * (1. - 2.*RAND);
      simdata[jtime] = noise
	+ contrast_scale1 * GLM.X_tc[jtime][ibasis1]
	+ contrast_scale2 * GLM.X_tc[jtime][ibasis2];
    }
  return(0);
}

int GLM_calculate_relative_CNR( int lCondition, float *cnr )
{
  float df, contrast, noise;
  int jcoeff, jcoeff1;

  /*
  ** Make sure the condition is within range.
  */
  if ( lCondition < 0 || lCondition >= GLM.nconditions )
    {
      printf("Error: the specified condition (%d) is out of range (0-%d)\n",
	     lCondition,GLM.nconditions);
      return(1);
    }
  if ( GLM.contrast_matrix[lCondition] == NULL )
    {
      printf("The conditions (contrast matrix) must first be set.\n");
      return(1);
    }

  df = GLM.ntime - GLM.ncoeff;  /* degrees of freedom */
  contrast = noise = 0;
  for (jcoeff=0; jcoeff<GLM.ncoeff; jcoeff++)
    {
      contrast += GLM.contrast_matrix[lCondition][0][jcoeff] * GLM.Beta_c[jcoeff];
      for (jcoeff1=0; jcoeff1<GLM.ncoeff; jcoeff1++)
	noise += GLM.contrast_matrix[lCondition][0][jcoeff] * GLM.XTXm1_cc[jcoeff][jcoeff1]
	  *      GLM.contrast_matrix[lCondition][0][jcoeff1];
    }
  noise = sqrt((double)noise/df);
  *cnr = contrast / noise;
  return(0);
}

int GLM_calculate_unbiased_efficiency( float *eff )
{
  float denominator;
  int jcoeff;

  denominator = 0.;
  for (jcoeff=0; jcoeff<GLM.ncoeff; jcoeff++)
    denominator += GLM.XTXm1_cc[jcoeff][jcoeff];
  
  *eff = 1. / denominator;
  return(0);
}

void GLMInternal_whiten_data(float *data_vector)
{
  float *data_temp;
  int jtime, jtime1;

  if ( GLM.AR_order < 1 ) return;  // do nothing

  // Allocate space for whitened data.
  data_temp = float_vector(GLM.ntime);
  // Copy the data to the temporary vector.
  for (jtime=0; jtime<GLM.ntime; jtime++)
    data_temp[jtime] = data_vector[jtime];

  // Whiten the data.
  for (jtime=0; jtime<GLM.ntime; jtime++)
    {
      data_vector[jtime] = 0.;
      for (jtime1=MAX(0,jtime-GLM.AR_order); jtime1<=jtime; jtime1++)
	{
	  data_vector[jtime] += GLM.WhiteFilter_tt[jtime][jtime1] * data_temp[jtime1];
	  /*
	  if ( jtime==2 ) printf("orig = %f, add %f x %f = %f\n",data_temp[jtime],
				 GLM.WhiteFilter_tt[jtime][jtime1],data_temp[jtime1],
				 GLM.WhiteFilter_tt[jtime][jtime1] * data_temp[jtime1]);
	  */
	}
    }
  free_float_vector(data_temp);
  return;
}
