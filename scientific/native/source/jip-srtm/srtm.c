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
** GLM structures.  For data analysis, there is
** no need to include glm.h into the calling program, as the
** functions in this file should handle everything without the
** calling program needing to know details.  The include file
** below has been separated from this file for purposes of simulations,
** which may want to set timing values in the GLM structure directly.
*/
#include "srtm.h"

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
///////// Functions that set up the GLM based upon file input ////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

int SRTM_read_control_file( BOOL verbose, char *file_name )
{
  FILE *file;
  char whole_line[LEN_STR], string[LEN_STR];
  char *ptr;
  float dt, deriv;
  int nline, i1, ivalue, itime;
  int jrun, jt, jcolumn, jcoeff, jperiod;

  // Initialize the glm input information.
  SRTM_input.multiple_comparisons = 0;
  SRTM_input.nifti_output = FALSE;
  SRTM_input.nruns = 0;
  strcpy(SRTM_input.reference_region_file,"none");
  strcpy(SRTM_input.target_region_file,"none");

  // Initialize basic GLM information.
  SRTM.initialized = 0;
  SRTM.ncoeff = SRTM.nruns = SRTM.ntime = 0;

  // per coefficient
  for (jcoeff=0; jcoeff<MAX_COEFF; jcoeff++)
    {
      SRTM.event_id[jcoeff] = SRTM.event_type[jcoeff] = SRTM.event_shape[jcoeff] = SRTM.nperiods[jcoeff] = SRTM.tau[jcoeff] = 0;
      for (jperiod=0; jperiod<MAX_PERIODS; jperiod++)
	SRTM.period_start[jcoeff][jperiod] = SRTM.period_stop[jcoeff][jperiod] = 0.;
    }

  // per run
  for (jrun=0; jrun<MAX_RUNS; jrun++)
    {
      if ( SRTM.table_value[jrun] != NULL )
	free_float_matrix(SRTM.table_value[jrun],SRTM.table_ncolumns[jrun],SRTM.nt[jrun]);
      else
	SRTM.table_value[jrun] = NULL;
      SRTM.nt[jrun] = SRTM.run_istart[jrun] = SRTM.run_istop[jrun] = 0;
      SRTM.table_ncolumns[jrun] = 0;
      SRTM.inclusion_cutoff[jrun] = -1;
      SRTM.k2prime[jrun] = 0.;
    }
  // per column
  for (jcolumn=0; jcolumn<MAX_COLUMNS; jcolumn++)
    strcpy(SRTM.table_column_name[jcolumn],"undefined");

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
      if ( (ivalue=SRTMInternal_decode_keyword(whole_line)) < 0 )
	{
	  printf("Error reading line %d of file '%s':\n",nline,file_name);
	  SRTMInternal_describe_glm_format();
	  fclose(file);
	  return(1);
	}
      else if ( ivalue == 1 )
	break;
      nline++;
    }

  // The last command was "runs", so begin to read the following lines as
  //   [run file name] [glm timing file name]
  SRTM.nruns = 0;
  while ( get_line_with_comment(file, whole_line, LEN_STR) )
    {
      if ( sscanf(whole_line,"%d",&i1) < 0 )  /* ... then this is a blank line */
	continue;
      /* This line should contain a bshort and glm file name. */
      /* Read the bshort file name. */
      if ( sscanf(whole_line,"%s",string) != 1 )
	{
	  printf("Error reading the bshort file name for run %d.\n",SRTM.nruns+1);
	  fclose(file);
	  return(1);
	}
      strcpy(SRTM_input.run_name[SRTM.nruns],string);

      // Update the argument to the next string position (table file name).
      ptr = next_non_white( whole_line + jip_int_count((JIP_INDEX)strlen(string)) );
      /* Read the table file name. */
      if ( sscanf(ptr,"%s",string) != 1 )
	{
	  printf("Error reading the GLM file name for run %d.\n",SRTM.nruns+1);
	  fclose(file);
	  return(1);
	}
      strcpy(SRTM_input.glm_name[SRTM.nruns],string);

      // Update the argument to the next string position (table file name).
      ptr = next_non_white( ptr + jip_int_count((JIP_INDEX)strlen(string)) );
      // append or new
      if ( sscanf(ptr,"%s",string) != 1 )
	{
	  printf("Three arguments are required for run %d: [data file] [table file] ['new' or 'append'].\n",SRTM.nruns+1);
	  fclose(file);
	  return(1);
	}
      strcpy(SRTM_input.table_name[SRTM.nruns],string);

      // Read the table file, if one is given.
      if ( SRTM_read_table_file(SRTM_input.table_name[SRTM.nruns], SRTM.nruns) )
	return(1);
      // Read the GLM file after the table file.
      if ( SRTM_read_GLM_file(SRTM_input.glm_name[SRTM.nruns], SRTM.nruns) )
	return(1);

      SRTM.nruns++;
    }
  fclose(file);
  SRTM_input.nruns = SRTM.nruns;

  // A reference region overlay must have been supplied.
  if ( !strcmp(SRTM_input.reference_region_file,"none") )
    {
      printf("Error: the name of a reference region overlay file must be supplied.\n");
      return(1);
    }
  else if ( !strcmp(SRTM_input.target_region_file,"none") )
    {
      printf("Error: the name of a target region overlay file must be supplied.\n");
      return(1);
    }

  printf("Read reference region data using overlay %s\n",SRTM_input.reference_region_file);
  printf("Read reference region data using overlay %s\n",SRTM_input.target_region_file);
  // Read the reference & target region data for each run.
  for (jrun=0; jrun<SRTM.nruns; jrun++)
    {
      if ( read_ROIs_from_file(verbose, jrun, SRTM_input.run_name[jrun]) )
	return(1);
      // data file name
      // Compute the reference region derivative.
      for (jt=0; jt<SRTM.table_nt[jrun]; jt++)
	{
	  dt    = SRTM.table_value[jrun][0][jt];
	  if ( jt != 0 )
	    deriv = (SRTM.reference_region[jrun][jt]-SRTM.reference_region[jrun][jt-1])/dt;
	  else
	    deriv = SRTM.reference_region[jrun][jt]/dt;
	  SRTM.ref_region_deriv[jrun][jt] = deriv;
	}
    }

  // Define run start and stop indices.
  for (jrun=0, itime=0; jrun<SRTM.nruns; jrun++)
    {
      SRTM.run_istart[jrun] = itime;
      SRTM.run_istop[jrun]  = SRTM.run_istart[jrun] + SRTM.nt[jrun] - 1;
      itime += SRTM.nt[jrun];
    }

  // Allocate matrices.
  // Allocate and initialize new arrays.
  SRTM.Weight_t   = float_vector(SRTM.ntime);
  SRTM.Xorig_tc   = float_matrix(SRTM.ntime,SRTM.ncoeff);
  SRTM.Xmix_tc    = float_matrix(SRTM.ntime,SRTM.ncoeff);
  SRTM.X_tc       = float_matrix(SRTM.ntime,SRTM.ncoeff);
  SRTM.XTXm1_cc   = float_matrix(SRTM.ncoeff,SRTM.ncoeff);
  SRTM.XTXm1XT_ct = float_matrix(SRTM.ncoeff,SRTM.ntime);
  SRTM.Beta_c     = float_vector(SRTM.ncoeff);

  SRTM.initialized = 1;
  return(0);
}

int SRTMInternal_decode_keyword( char *line )
{
  char keyword[LEN_STR], string[LEN_STR], *argument;

  sscanf(line,"%s",keyword);
  argument = next_non_white( line + jip_int_count((JIP_INDEX)strlen(keyword)) );

  if ( !strcmp(keyword,"multiple-comparisons") )
    {
      if ( sscanf(argument,"%d",&(SRTM_input.multiple_comparisons)) != 1 )
	{
	  printf("Error: `multiple-comparisons' requires an integer argument.\n");
	  return(-1);
	}
      return(0);
    }
  else if ( !strcmp(keyword,"output-format") )
    {
      sscanf(argument,"%s",string);
      if ( !strcmp(string,"nifti") || !strcmp(string,"NIFTI") )
	SRTM_input.nifti_output = TRUE;
      return(0);
    }
  else if ( !strcmp(keyword,"reference-region") )
    {
      sscanf(argument,"%s",SRTM_input.reference_region_file);
      return(0);
    }
  else if ( !strcmp(keyword,"target-region") )
    {
      sscanf(argument,"%s",SRTM_input.target_region_file);
      return(0);
    }
  else if ( !strcmp(keyword,"conditions") )
    {
      strcpy(SRTM_input.condition_string,argument);
      return(0);
    }
  //  else if ( !strcmp(keyword,"condition") )
  //    {
  //      sscanf(argument,"%s",SRTM.condition_name[SRTM.nconditions]);
  //      ptr = next_non_white( argument + jip_int_count((JIP_INDEX)strlen(SRTM.condition_name[SRTM.nconditions])) );
  //      strcpy(SRTM.condition_string[SRTM.nconditions],ptr);
  //      SRTM.nconditions++;
  //      return(0);
  //    }
  else if ( !strcmp(keyword,"runs:") || !strcmp(keyword,"runs") )
    return(1);
  else // not found
    return(-1);
}

int SRTM_read_GLM_file( char *file_name, int irun )
{
  FILE *file;
  char line[LEN_STR], arg1[LEN_STR], arg2[LEN_STR], arg3[LEN_STR];
  char event_char, event_type[LEN_STR], event_shape[LEN_STR];
  float start, stop, tau, Value, ValueMax, run_start, run_stop;
  int iline, ncoeff, nperiod, nread, itype, ishape, i1, found, icolumn, istart, itime;
  int event_id;
  int jcoeff, icoeff, jrun, jt;

  //
  // Open the GLM file.
  //
  if ( !(file = fopen(file_name,"r")) )
    {
      printf("Error attempting to open the GLM file %s.\n",file_name);
      return(1);
    }
  //
  // The first line in the file must contain 2 values: [# time points] [k2']
  //
  iline = 1;
  if ( !get_line_with_comment(file, line, LEN_STR) )
     {
      printf("Error reading the 1st line of GLM file %s\n",file_name);
      return(1);
    }
  if ( sscanf(line,"%d %f",&(SRTM.nt[irun]),&tau) != 2 )
    {
      printf("Error reading line %d of the GLM file %s (# points, tau2').\n",
	     iline,file_name);
      return(1);
    }
  else if ( SRTM.nt[irun] != SRTM.table_nt[irun] )
    {
      printf("***************************************************************\n");
      printf("The GLM file and table have different numbers of time points (%d, %d)\n",SRTM.nt[irun],SRTM.table_nt[irun]);
      printf("Data after point %d will be ignored.\n",SRTM.nt[irun]);
      printf("***************************************************************\n");
      // Define the cutoff time to be the value entered in the GLM file.
      SRTM.inclusion_cutoff[irun] = SRTM.nt[irun];
      // Reset the number of time points to the number in the table.
      SRTM.nt[irun] = SRTM.table_nt[irun];
    }
  else
    SRTM.inclusion_cutoff[irun] = SRTM.nt[irun];
  iline++;

  if ( tau != 0. )
    SRTM.k2prime[irun] = 1./tau;

  // Increment the run counter & duration, and store the start & stop time.
  SRTM.ntime += SRTM.nt[SRTM.nruns];

  // This run stops at the last time point.
  SRTM.run_istop[irun] = SRTM.ntime-1;
  if ( irun == 0 )
    SRTM.run_istart[0] = 0;
  else  // This run starts at the end of the last run, + 1 time point
    SRTM.run_istart[irun] = SRTM.run_istop[irun-1] + 1;

  //
  // Loop until the end of file (EOF) is reached.
  // Blank lines indicate a new event.
  //
  ncoeff = nperiod = 0;  // These are counters local to the current file (not the appended GLM).
  event_id = icoeff = ishape = itype = found = -1;  // avoid compiler warnings
  while ( get_line_with_comment(file,line,LEN_STR) )
    {
      if ( sscanf(line,"%d",&i1) < 0 )  // ... then this is a blank line
	{
	  // The next line must be blank or contain an identifier.
	  while ( get_line_with_comment(file,line,LEN_STR) )
	    {
	      // The next line must contain exactly 3 entries:
	      // 1) event identifier as a single character (1-9, a-z, A-Z)
	      // 2) event type: R1, k2, k2a
	      // 3) event shape: constant, gamma, sigmoid, table
	      if ( (nread=sscanf(line,"%s %s %s",arg1,arg2,arg3)) != 3 )
		{
		  printf("Error on line %d: only %d values were read with value %s.\n",iline,nread,arg1);
		  printf("\nEvents should be identified with 3 arguments:\n");
		  printf("1) event identifier: 1-9, a-z, A-Z\n");
		  printf("2) event type      : R1, k2, k2a\n");
		  printf("3) event shape     : constant, gamma, sigmoid, table\n");
		}
	      sscanf(arg1,"%c",&event_char);
	      if ( !(event_id=SRTM_EventNumberFromChar(event_char)) )
		{
		  printf("Event number (%d) for event (%c) is not allowed on line %d in file %s\n",
			 event_id,event_char,iline+1,file_name);
		  return(1);
		}
	      // Define the event type
	      if ( !strcmp("R1",arg2) )
		itype = TYPE_R1;
	      else if ( !strcmp("k2",arg2) || !strcmp("K2",arg2) )
		itype = TYPE_k2;
	      else if ( !strcmp("k2a",arg2) || !strcmp("K2a",arg2) )
		itype = TYPE_k2a;
	      else
		{
		  printf("Error on line %d: allowed types are R1, k2, and k2a\n",iline);
		  return(1);
		}
	      // Define the event shape
	      if ( !strcmp("constant",arg3) )
		ishape = SHAPE_CONSTANT;
	      else if ( !strcmp("gamma",arg3) )
		ishape = SHAPE_GAMMA;
	      else if ( !strcmp("gamma-integral",arg3) )
		ishape = SHAPE_GAMMAINT;
	      else if ( !strcmp("sigmoid",arg3) )
		ishape = SHAPE_SIGMOID;
	      else if ( !strcmp("table",arg3) )
		ishape = SHAPE_TABLE;
	      else
		{
		  printf("Error: shape %s is unknown. Use 'constant', 'gamma', 'sigmoid', or 'table'.\n",arg3);
		  return(1);
		}
	      iline++;

	      // Decide whether or not the event has been defined previously.
	      found = 0;
	      for (jcoeff=0; jcoeff<SRTM.ncoeff; jcoeff++)
		{
		  if ( event_id == SRTM.event_id[jcoeff] )
		    { // This event already exists
		      icoeff = jcoeff;
		      found = 1;
		      if ( itype != SRTM.event_type[icoeff] )
			{
			  printf("Error: event %c does not have a consistent type.\n",event_char);
			  if ( SRTM.event_type[icoeff] == TYPE_R1 )
			    strcpy(event_type,"R1");
			  else if ( SRTM.event_type[icoeff] == TYPE_k2 )
			    strcpy(event_type,"k2");
			  else if ( SRTM.event_type[icoeff] == TYPE_k2a )
			    strcpy(event_type,"k2a");
			  printf("The type is %s in the 1st run ",event_type);
			  if ( itype == TYPE_R1 )
			    strcpy(event_type,"R1");
			  else if ( itype == TYPE_k2 )
			    strcpy(event_type,"k2");
			  else if ( itype == TYPE_k2a )
			    strcpy(event_type,"k2a");
			  printf("and %s in run %d.\n",event_type,irun);
			  return(1);
			}
		      if ( ishape != SRTM.event_shape[icoeff] )
			{
			  printf("Error: event %c does not have a consistent shape.\n",event_char);
			  if ( SRTM.event_shape[icoeff] == SHAPE_CONSTANT )
			    strcpy(event_shape,"constant");
			  else if ( SRTM.event_shape[icoeff] == SHAPE_GAMMA )
			    strcpy(event_shape,"gamma");
			  else if ( SRTM.event_shape[icoeff] == SHAPE_GAMMAINT )
			    strcpy(event_shape,"gamma-integral");
			  else if ( SRTM.event_shape[icoeff] == SHAPE_SIGMOID )
			    strcpy(event_shape,"sigmoid");
			  else if ( SRTM.event_shape[icoeff] == SHAPE_TABLE )
			    strcpy(event_shape,"table");
			  printf("The shape is %s in the 1st run ",event_shape);
			  if ( ishape == SHAPE_CONSTANT )
			    strcpy(event_shape,"constant");
			  else if ( ishape == SHAPE_GAMMA )
			    strcpy(event_shape,"gamma");
			  else if ( ishape == SHAPE_GAMMAINT )
			    strcpy(event_shape,"gamma-integral");
			  else if ( ishape == SHAPE_SIGMOID )
			    strcpy(event_shape,"sigmoid");
			  else if ( ishape == SHAPE_TABLE )
			    strcpy(event_shape,"table");
			  printf("and %s in run %d.\n",event_shape,irun);
			  return(1);
			}
		    }
		}
	      if ( !found )
		{ // This is a new event.
		  SRTM.ncoeff++;    // ... so increment the number of coefficients
		  if ( SRTM.ncoeff > MAX_COEFF )
		    {
		      printf("Error: # of events (%d) > max dimension in program (%d).\n",
			     ncoeff,MAX_COEFF);
		      exit(1);
		    }
		  icoeff = SRTM.ncoeff-1;
		  SRTM.event_id[icoeff]    = event_id;
		  SRTM.event_type[icoeff]  = itype;
		  SRTM.event_shape[icoeff] = ishape;
		  if ( SRTM.event_type[icoeff] == TYPE_k2 && SRTM.k2prime[irun] != 0. )
		    {
		      printf("Error: use either a variable k2' (no k2 event), or a fixed one (k2'=%g in run %d).\n",
			     SRTM.k2prime[irun],irun);
		      return(1);
		    }
		}
	      // Update local counter.
	      ncoeff++;
	      nperiod = 0;
	      break;
	    }
	}
      else
	{ // The line was NOT blank, and it is not an identifier
	  // The next line depends upon the shape.
	  // Assign start, stop, column, and tau
	  icolumn = 0;
	  start   = 0.;
	  //
	  SRTM.nruns++;
	  run_start = SRTM_time_total_minutes(SRTM.run_istart[irun]);
	  run_stop  = SRTM_time_total_minutes(SRTM.run_istop[irun]);
	  SRTM.nruns--;
	  //
	  stop      = run_stop;
	  tau     = 0.;
	  if ( ishape == SHAPE_CONSTANT )
	    { // [start] [stop]
	      if ( (nread=sscanf(line,"%g %g",&start,&stop) != 2) )
		{
		  if ( nread == 1 )
		    stop = 0.; // placeholder
		  else
		    {
		      printf("Programming error 1: nread = %d in SRTM_read_GLM_file\n",nread);
		      exit(1);
		    }
		}
	    }
	  else if ( ishape == SHAPE_GAMMA )
	    { // [on] [optional magnitude]
	      if ( (nread=sscanf(line,"%g %g",&start,&tau) != 2) )
		{
		  printf("Error: a gamma event should be followed by start time and tau.\n");
		  return(1);
		}
	      if ( found && tau != SRTM.tau[icoeff] )
		{
		  printf("Error: event %c is a gamma event with tau=%g in run %d not equal to previous tau (%g).\n",
			 SRTM_EventChar(event_id),tau,irun,SRTM.tau[icoeff]);
		  return(1);
		}
	      stop = 0.;  // to be replaced by end of run below
	    }
	  else if ( ishape == SHAPE_GAMMAINT )
	    { // [on] [optional magnitude]
	      if ( (nread=sscanf(line,"%g %g",&start,&tau) != 2) )
		{
		  printf("Error: a gamma-integral event should be followed by start time and tau.\n");
		  return(1);
		}
	      if ( found && tau != SRTM.tau[icoeff] )
		{
		  printf("Error: event %c is a gamma event with tau=%g in run %d not equal to previous tau (%g).\n",
			 SRTM_EventChar(event_id),tau,irun,SRTM.tau[icoeff]);
		  return(1);
		}
	      stop = 0.;  // to be replaced by end of run below
	    }
	  else if ( ishape == SHAPE_SIGMOID )
	    { // [on] [optional magnitude]
	      if ( (nread=sscanf(line,"%g %g",&start,&tau) != 2) )
		{
		  printf("Error: a sigmoid event should be followed by start time and tau.\n");
		  return(1);
		}
	      if ( found && tau != SRTM.tau[icoeff] )
		{
		  printf("Error: event %c is a sigmoid event with tau=%g in run %d not equal to previous tau (%g).\n",
			 SRTM_EventChar(event_id),tau,irun,SRTM.tau[icoeff]);
		  return(1);
		}
	      stop = 0.;  // to be replaced by end of run below
	    }
	  else if ( ishape == SHAPE_TABLE )
	    {
	      if ( (nread=sscanf(line,"%d",&icolumn) != 1) )
		{
		  printf("Programming error 2: nread = %d in SRTM_read_GLM_file\n",nread);
		  exit(1);
		}
	    }
	  //
	  // Handle other error conditions:
	  //
	  // 1) Each ON time must be greater than 0.
	  if ( start < 0. )
	    {
	      printf("Error: ON time must be > 0 for period %d of event %c.\n",
		     nperiod,SRTM_EventChar(event_id));
	      return(1);
	    }
	  // 2) The OFF time must be later than the ON time
	  if ( stop != 0. && stop <= start )
	    {
	      printf("Error: OFF time must be > ON time for period %d of event %c.\n",
		     nperiod,SRTM_EventChar(event_id));
	      return(1);
	    }
	  // Assign values
	  SRTM.icolumn[icoeff] = icolumn;
	  SRTM.tau[icoeff]     = tau;
	  SRTM.nruns++;
	  SRTM.period_start[icoeff][SRTM.nperiods[icoeff]]  = start + run_start;
	  if ( stop != 0. )
	    SRTM.period_stop[icoeff][SRTM.nperiods[icoeff]] = stop  + run_start;
	  else
	    SRTM.period_stop[icoeff][SRTM.nperiods[icoeff]] = run_stop;
	  SRTM.nruns--;
	  SRTM.nperiods[icoeff]++;
	  nperiod++;
	  if ( SRTM.nperiods[icoeff] > MAX_PERIODS )
	    {
	      printf("Error: the number of periods (%d) for event %c > max dimension in program (%d).\n",
		     SRTM.nperiods[icoeff],SRTM_EventChar(ncoeff),MAX_PERIODS);
	    }
	}
      iline++;
    }
  fclose( file );

  // Further brief error checking, and table checking.
  for (jcoeff=0; jcoeff<SRTM.ncoeff; jcoeff++)
    {
      event_id = SRTM.event_id[jcoeff];
      if ( SRTM_EventCharFromInteger(event_id,&event_char) )
	{
	  printf("Programming error: event number (%d) apparenty not defined.?!\n",event_id);
	  exit(1);
	}
      else if ( SRTM.nperiods[jcoeff] < 1 )
	{
	  printf("Error checking problem: # periods < 1 for event %c.\n",event_char);
	  printf("Check format of event description file.\n");
	  exit(1);
	}
      if ( SRTM.event_shape[jcoeff] != SHAPE_CONSTANT &&
	   SRTM.event_shape[jcoeff] != SHAPE_GAMMA    &&
	   SRTM.event_shape[jcoeff] != SHAPE_GAMMAINT &&
	   SRTM.event_shape[jcoeff] != SHAPE_SIGMOID  &&
	   SRTM.event_shape[jcoeff] != SHAPE_TABLE )
	{
	  printf("Programming error: event shape (%d) is unknown.\n",SRTM.event_shape[jcoeff]);
	  exit(1);
	}
      if ( SRTM.event_type[jcoeff] == TYPE_k2 )
	{
	  if ( SRTM.event_shape[jcoeff] != SHAPE_CONSTANT )
	    {
	      printf("Error: type 'k2' events (ID=%c) must be constant in time.\n",SRTM_EventChar(SRTM.event_id[jcoeff]));
	      return(1);
	    }
	}
      // Define maximum table value.
      if ( SRTM.event_shape[jcoeff] == SHAPE_TABLE )
	{
	  icolumn = SRTM.icolumn[jcoeff];  // table events only
	  for (jrun=0, ValueMax=0.; jrun<SRTM.nruns; jrun++)
	    {
	      istart = SRTM.run_istart[jrun];
	      for (jt=0; jt<SRTM.nt[jrun]; jt++)
		{
		  itime = istart + jt;
		  Value = SRTM.table_value[jrun][icolumn][jt];
		  if ( fabs((double)Value) > ValueMax )
		    {
		      ValueMax = Value;
		      SRTM.iTimeMax[jcoeff] = itime;
		    }
		}
	    }
	}
    }

  return(0);
}

int SRTM_read_table_file( char *file_name, int irun )
{
  FILE *file;
  char line[LEN_STR], string[LEN_STR], *ptr;
  int jcolumn, jt, nread;
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

  // Read the 1st line of the table, which contains column labels
  if ( !get_line_with_comment(file, line, LEN_STR) ) return(1);
  // Count the number of strings on the 1st line.
  ptr = line;
  nread = 0;
  while ( sscanf(ptr,"%s",string)==1 )
    {
      ptr = next_non_white( ptr + jip_int_count((JIP_INDEX)strlen(string)) );
      // Assign the column names based upon the 1st run.
      if ( irun == 0 )
	strcpy(SRTM.table_column_name[nread],string);
      else
	{ // For other runs, make sure the column names are consistent.
	  if ( strcmp(SRTM.table_column_name[nread],string) )
	    printf("Warning: runs 1 & %d have mismatched names (%s & %s) in column %d.\n",
		   irun+1,SRTM.table_column_name[nread],string,nread);
	}
      nread++;
    }
  SRTM.table_ncolumns[irun] = nread;  // # of columns in this table

  // Now determine the number of time points in the table.
  SRTM.table_nt[irun] = 0;
  while ( get_line_with_comment(file,line,LEN_STR) )
    (SRTM.table_nt[irun])++;

  rewind(file);
  get_line_with_comment(file,line,LEN_STR); // read 1st line again
  //  get_line_with_comment(file,line,LEN_STR); // read 2nd line again

  // Allocate the table memory.
  SRTM.table_value[irun]      = float_matrix(SRTM.table_ncolumns[irun],SRTM.table_nt[irun]);
  SRTM.reference_region[irun] = float_vector(SRTM.table_nt[irun]);
  SRTM.ref_region_deriv[irun] = float_vector(SRTM.table_nt[irun]);
  SRTM.target_region[irun]    = float_vector(SRTM.table_nt[irun]);

  // Read the table values for each column.
  for (jt=0; jt<SRTM.table_nt[irun]; jt++)
    {
      get_line_with_comment(file,line,LEN_STR); // read each line again.
      for (jcolumn=0, ptr=line; jcolumn<SRTM.table_ncolumns[irun]; jcolumn++)
	{
	  error =  sscanf(ptr,"%s",string) != 1;
	  error &= sscanf(ptr,"%f",&(SRTM.table_value[irun][jcolumn][jt])) != 1;
	  if ( jcolumn == 0 )
	    SRTM.table_value[irun][0][jt] /= 60.;  // convert seconds to minutes
	  if ( error )
	    {
	      printf("Error reading line %d of TABLE file %s\n",jt+3,file_name);
	      return(1);
	    }
	  else if ( isnan(SRTM.table_value[irun][jcolumn][jt]) )
	    {
	      printf("Error: table entry %d in column %d of run %d is not a number.\n",
		     jt,jcolumn,irun);
	      exit(1);
	    }
	  ptr = next_non_white( ptr + jip_int_count((JIP_INDEX)strlen(string)) );
	}
    }
  fclose(file);
  return(0);
}

int read_ROIs_from_file( BOOL verbose,  int iRun, char *SRTM_FileName )
{
  struct XD_HEADER header;
  float *data, weight_sum, rMax;
  int jt;JIP_INDEX j3d;
  static IPOINT3D dim;
  static BOOL first_time=TRUE;

  // Read the data file header for dimensions.
  if ( read_volume_header(verbose, SRTM_FileName, &header) )
    return(1);

  if ( header.nt != SRTM.nt[iRun] )
    {
      printf("Error: the data file (%s) and table file disagree on the number of time points (%d and %d)\n",
	     SRTM_FileName, header.nt, SRTM.nt[iRun]);
      return(1);
    }
  if ( first_time )
    { // Read the overlay file.
      first_time = FALSE;
      dim = header.dim;
    }
  else
    {
      if ( (header.dim.x != dim.x) || (header.dim.y != dim.y) || (header.dim.z != dim.z) )
	{
	  printf("Error: image dimensions for file %s (%d x %d x %d) are different from previous dimensions (%d x %d x %d)\n",
		 SRTM_FileName,header.dim.x,header.dim.y,header.dim.z,dim.x,dim.y,dim.z);
	  return(1);
	}
    }

  // Allocate and read overlay masks.
  SRTM.Mask_Reference = float_vector(((JIP_INDEX)(dim.x))*dim.y*dim.z);
  SRTM.Mask_Target    = float_vector(((JIP_INDEX)(dim.x))*dim.y*dim.z);
  if ( read_overlay_mask(SRTM_input.reference_region_file, dim, SRTM.Mask_Reference) )
    return(1);
  if ( read_overlay_mask(SRTM_input.target_region_file, dim, SRTM.Mask_Target) )
    return(1);

  ////////////////////////////////
  // Create ROIs from this file.
  ////////////////////////////////

  // Open the binary data file. */
  // Allocate a 4d data array.
  data = float_vector(header.n4d);
  if ( read_xdisplay_data_file(SRTM_FileName, header, data) )
    return(1);

  // Define the ROI vector for the reference region
  for (j3d=0, weight_sum=0.; j3d<header.n3d; j3d++)
    weight_sum += SRTM.Mask_Reference[j3d];
  for (jt=0; jt<header.nt; jt++)
    {
      SRTM.reference_region[iRun][jt] = 0.;
      for (j3d=0; j3d<header.n3d; j3d++)
	SRTM.reference_region[iRun][jt] += SRTM.Mask_Reference[j3d]/weight_sum * data[j3d+jt*header.n3d];
    }
  // Define the ROI vector for the target region
  for (j3d=0, weight_sum=0.; j3d<header.n3d; j3d++)
    weight_sum += SRTM.Mask_Target[j3d];
  for (jt=0; jt<header.nt; jt++)
    {
      SRTM.target_region[iRun][jt] = 0.;
      for (j3d=0; j3d<header.n3d; j3d++)
	SRTM.target_region[iRun][jt] += SRTM.Mask_Target[j3d]/weight_sum * data[j3d+jt*header.n3d];
    }
  for (jt=0, rMax=0.; jt<header.nt; jt++)
    if ( SRTM.target_region[iRun][jt] > rMax ) rMax = SRTM.target_region[iRun][jt];

  free_float_vector(data);
  return(0);
}

int read_xdisplay_data_file( char *FileName, struct XD_HEADER header, float *data )
{
  FILE *file;
  JIP_INDEX offset_volume, j;
  float *fdata;
  short *idata;
  unsigned short *iudata;
  unsigned char *i8data;
  int *iidata;
  int swap_bytes, jt;JIP_INDEX output_info, error;

  swap_bytes  = machine_byte_order() != header.byte_order;
  output_info = header.n4d > 1000000 && header.nt > 10;

  if ( header.storage_type == BFLOAT )      // bfloat
    // floating point data could be read into fSpaceTimeData directly,
    // but allocate a temporary array for consistency with other formats (below).
    fdata = float_vector(header.n3d);
  else if ( header.storage_type == BSHORT ) // bshort
    idata = short_vector(header.n3d);
  else if ( header.storage_type == BLONG )  // blong
    iidata = int_vector(header.n3d);
  else if ( header.storage_type == USHORT ) // unsigned short
    iudata = unsigned_short_vector(header.n3d);
  else if ( header.storage_type == UINT8 )  // unsigned integer with 1 byte (8 bits)
    i8data = unsigned_char_vector(header.n3d);

  // Open file file.
  if ( !(file = fopen(FileName,"r")) )
    {
      printf("\nError: cannot open file %s.\n",FileName);
      return(1);
    }
  // Go to the first data voxel
  error = jip_seek(file, (JIP_INDEX)(header.voxel_offset), SEEK_SET);
  if ( error )
    {
      printf("Error stepping past header into data for file %s\n",FileName);
      return(1);
    }

  // Read the array.
  for (jt=0; jt<header.nt; jt++)
    {
      if ( output_info && jt%(header.nt/10) == 0 )
	{
	  printf("\r%s: %d %c",FileName,10*jt/(header.nt/10),37);
	  fflush(stdout);
	}
      if ( header.storage_type == BFLOAT )      // bfloat
	error = fread((char *)fdata,sizeof(float),header.n3d,file) != header.n3d;
      else if ( header.storage_type == BSHORT ) // bshort
	error = fread((char *)idata,sizeof(short),header.n3d,file) != header.n3d;
      else if ( header.storage_type == USHORT ) // unsigned short
	error = fread((char *)iudata,sizeof(unsigned short),header.n3d,file) != header.n3d;
      else if ( header.storage_type == BLONG )  // blong
	error = fread((char *)iidata,sizeof(int),header.n3d,file) != header.n3d;
      else if ( header.storage_type == UINT8 ) // unsigned char
	error = fread((char *)i8data,sizeof(char),header.n3d,file) != header.n3d;
      if ( error )
	{
	  printf("Error reading time point %d for file %s\n",jt,FileName);
	  return(1);
	}
      // Copy the integer array into the stack floating point array.
      offset_volume = jt * header.n3d;
      for (j=0; j<header.n3d; j++)
	{
	  if ( header.storage_type == BFLOAT )      // bfloat
	    {
	      if ( swap_bytes ) swap_32( &(fdata[j]) );
	      data[j+offset_volume] = fdata[j];
	    }
	  else if ( header.storage_type == BSHORT ) // bshort
	    {
	      if ( swap_bytes ) swap_16( &(idata[j]) );
	      data[j+offset_volume] = idata[j];
	    }
	  else if ( header.storage_type == BLONG )  // blong
	    {
	      if ( swap_bytes ) swap_32( &(iidata[j]) );
	      data[j+offset_volume] = iidata[j];
	    }
	  else if ( header.storage_type == USHORT ) // unsigned short
	    {
	      if ( swap_bytes ) swap_16( &(iudata[j]) );
	      data[j+offset_volume] = iudata[j];
	    }
	  else if ( header.storage_type == UINT8 ) // unsigned char
	    {
	      // no need to swap, since only 1 byte+
	      data[j+offset_volume] = i8data[j];
	    }
	  // Scale the data if necessary.
	  if ( header.scale_slope != 0. )
	    {
	      data[j+offset_volume] *= header.scale_slope;
	      data[j+offset_volume] += header.scale_offset;
	    }
	}
    }
  fclose(file);

  // Done reading, so free array and close file.
  if ( header.storage_type == BFLOAT )      // bfloat
    free_float_vector(fdata);
  else if ( header.storage_type == BSHORT ) // bshort
    free_short_vector(idata);
  else if ( header.storage_type == BLONG )  // blong
    free_int_vector(iidata);
  else if ( header.storage_type == USHORT ) // unsigned short
    free_unsigned_short_vector(iudata);
  else if ( header.storage_type == UINT8 ) // unsigned char
    free_unsigned_char_vector(i8data);
  if ( output_info )
    {
      printf("\r%s: 100 %c\n",FileName,37);
      fflush(stdout);
    }
  return(0);
}

int read_overlay_mask( char *FileName, IPOINT3D dim, float *weight_vector )
{
  FILE *file;
  char whole_line[LEN_STR];
  IPOINT3D voxel;
  JIP_INDEX j;JIP_INDEX index;
  float weight;
  
  // Open the ovl file.
  if ( !(file = fopen(FileName,"r")) )
    {
      printf("Error opening file %s\n",FileName);
      return(1);
    }
  else
    { // initialize
      for (j=0; j<((JIP_INDEX)(dim.x))*dim.y*dim.z; j++)
	weight_vector[j] = 0.;
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
      else if ( voxel.x < 0 || voxel.x >= dim.x ||
		voxel.y < 0 || voxel.y >= dim.y ||
		voxel.z < 0 || voxel.z >= dim.z )
	{
	  printf("Error: the overlay in file .ovl does not agree with the image dimensions.\n");
	  return(1);
	}
      else
	{
	  index = ((JIP_INDEX)(voxel.z))*dim.x*dim.y + ((JIP_INDEX)(voxel.y))*dim.x + voxel.x;
	  weight_vector[index] = weight;
	}
    }
  fclose(file);
  return(0);
}

int SRTM_define_static_basis_functions()
// Define basis vectors that depend upon the reference region.
{
  int itype, ishape, icolumn, istart_run, itime, nincluded;
  int jcoeff, jcoeff1, jt, jt1, jrun, jperiod;
  float tau, sum1, sum2, dt, multiplier, time, time0, time1, time_stop, gmax;
  BOOL first_time = TRUE;

  if ( first_time )
    {
      // Define initial basis functions using the target region ROI.
      // Also define the inclusion vector.
      SRTM.target_t   = float_vector(SRTM.ntime);
      SRTM.include_time = int_vector(SRTM.ntime);
      for (jrun=0, nincluded=0; jrun<SRTM.nruns; jrun++)
	{
	  istart_run  = SRTM.run_istart[jrun];
	  for (jt=0; jt<SRTM.nt[jrun]; jt++)
	    {
	      SRTM.target_t[jt+istart_run] = SRTM.target_region[jrun][jt];
	      if ( jt < SRTM.inclusion_cutoff[jrun] )
		{
		  SRTM.include_time[jt+istart_run] = 1;
		  nincluded++;
		}
	      else
		SRTM.include_time[jt+istart_run] = 0;
	    }
	}
      first_time = FALSE;
    }

  for (jcoeff=0; jcoeff<SRTM.ncoeff; jcoeff++)
    {
      SRTM.Beta_c[jcoeff] = 0.;
      for (jcoeff1=0; jcoeff1<SRTM.ncoeff; jcoeff1++)
	SRTM.XTXm1_cc[jcoeff][jcoeff1] = 0.;
      for (jt=0; jt<SRTM.ntime; jt++)
	SRTM.Xorig_tc[jt][jcoeff] = SRTM.Xmix_tc[jt][jcoeff] = SRTM.XTXm1XT_ct[jcoeff][jt] = 0.;
    }
  SRTM.dof = nincluded - SRTM.ncoeff;

  // Define static basis functions.
  for (jcoeff=0; jcoeff<SRTM.ncoeff; jcoeff++)
    {
      itype   = SRTM.event_type[jcoeff];
      ishape  = SRTM.event_shape[jcoeff];
      icolumn = SRTM.icolumn[jcoeff];  // table events only
      tau     = SRTM.tau[jcoeff];      // gamma & sigmoid events only
      if ( SRTM.event_type[jcoeff] == TYPE_R1 )
	{ // coefficient = [R1] ;   basis function = [reference region]
	  for (jrun=0; jrun<SRTM.nruns; jrun++)
	    {
	      istart_run = SRTM.run_istart[jrun];
	      for (jt=0; jt<SRTM.nt[jrun]; jt++)
		{
		  itime = istart_run + jt;
		  time  = SRTM_time_total_minutes(itime);
		  SRTM.Xorig_tc[itime][jcoeff] = SRTM.Xmix_tc[itime][jcoeff] = 0.;  // default
		  for (jperiod=0; jperiod<SRTM.nperiods[jcoeff]; jperiod++)
		    {
		      time0 = SRTM.period_start[jcoeff][jperiod];
		      if ( time >= time0 && time <= SRTM.period_stop[jcoeff][jperiod] )
			{
			  if ( SRTM.event_shape[jcoeff] == SHAPE_CONSTANT )
			    {
			      multiplier = 1.;
			      for  (jt1=0, sum1=sum2=0.; jt1<=jt; jt1++)
				{
				  dt    = SRTM.table_value[jrun][0][jt1];
				  sum1 += dt * multiplier * SRTM.ref_region_deriv[jrun][jt1];
				  sum2 += dt * multiplier * SRTM.k2prime[jrun] * SRTM.reference_region[jrun][jt1];
				}
			    }
			  else if ( SRTM.event_shape[jcoeff] == SHAPE_GAMMA )
			    {
			      for  (jt1=0, sum1=sum2=0.; jt1<=jt; jt1++)
				{
				  time1 = SRTM_time_total_minutes(istart_run+jt1);
				  dt    = SRTM.table_value[jrun][0][jt1];
				  if ( time1 > time0 )
				    multiplier = (time1-time0)/tau * exp((double)(-(time1-time0)/tau)) / exp(-1.);
				  else
				    multiplier = 0.;
				  sum1 += dt * multiplier * SRTM.ref_region_deriv[jrun][jt1];
				  sum2 += dt * multiplier * SRTM.k2prime[jrun] * SRTM.reference_region[jrun][jt1];
				}
			    }
			  else if ( SRTM.event_shape[jcoeff] == SHAPE_GAMMAINT )
			    {
			      time_stop = SRTM.period_stop[jcoeff][jperiod];
			      gmax      = (time0+tau) * exp(-(double)(time0/tau)) - (time_stop+tau) * exp(-(double)(time_stop/tau));
			      for  (jt1=0, sum1=sum2=0.; jt1<=jt; jt1++)
				{
				  time1 = SRTM_time_total_minutes(istart_run+jt1);
				  dt    = SRTM.table_value[jrun][0][jt1];
				  if ( time1 > time0 )
				    multiplier = (time0+tau) * exp(-(double)(time0/tau)) - (time1+tau) * exp(-(double)(time1/tau));
				  else
				    multiplier = 0.;
				  multiplier /= gmax;
				  sum1 += dt * multiplier * SRTM.ref_region_deriv[jrun][jt1];
				  sum2 += dt * multiplier * SRTM.k2prime[jrun] * SRTM.reference_region[jrun][jt1];
				}
			    }
			  else if ( SRTM.event_shape[jcoeff] == SHAPE_SIGMOID )
			    {
			      for  (jt1=0, sum1=sum2=0.; jt1<=jt; jt1++)
				{
				  time1 = SRTM_time_total_minutes(istart_run+jt1);
				  dt   = SRTM.table_value[jrun][0][jt1];
				  if ( time1 > time0 )
				    multiplier = (time1-time0)/tau / sqrt((double)(1+SQR((time1-time0)/tau)));
				  else
				    multiplier = 0.;
				  sum1 += dt * multiplier * SRTM.ref_region_deriv[jrun][jt1];
				  sum2 += dt * multiplier * SRTM.k2prime[jrun] * SRTM.reference_region[jrun][jt1];
				}
			    }
			  else if ( SRTM.event_shape[jcoeff] == SHAPE_TABLE )
			    {
			      for  (jt1=0, sum1=sum2=0.; jt1<=jt; jt1++)
				{
				  multiplier = SRTM.table_value[jrun][icolumn][jt1];
				  dt   = SRTM.table_value[jrun][0][jt1];
				  sum1 += dt * multiplier * SRTM.ref_region_deriv[jrun][jt1];
				  sum2 += dt * multiplier * SRTM.k2prime[jrun] * SRTM.reference_region[jrun][jt1];
				}
			    }
			  SRTM.Xorig_tc[itime][jcoeff] = sum1 + sum2;  // correct basis functions for fixed or variable k2prime
			  SRTM.Xmix_tc[itime][jcoeff]  = sum1;         // only used in SRTM_binding_fit()
			}
		    } // loop over periods
		}
	    }
	}
      else if ( SRTM.event_type[jcoeff] == TYPE_k2 )
	{ // coefficient = k2  ;    basis function = [reference region integral]
	  for (jrun=0; jrun<SRTM.nruns; jrun++)
	    {
	      istart_run = SRTM.run_istart[jrun];
	      for (jt=0; jt<SRTM.nt[jrun]; jt++)
		{
		  itime = istart_run + jt;
		  time  = SRTM_time_total_minutes(itime);
		  SRTM.Xorig_tc[itime][jcoeff] = 0.;  // default
		  for (jperiod=0; jperiod<SRTM.nperiods[jcoeff]; jperiod++)
		    {
		      time0 = SRTM.period_start[jcoeff][jperiod];
		      if ( time >= time0 && time <= SRTM.period_stop[jcoeff][jperiod] )
			{
			  if ( SRTM.event_shape[jcoeff] == SHAPE_CONSTANT )
			    {
			      for  (jt1=0, sum1=0.; jt1<=jt; jt1++)
				{
				  dt   = SRTM.table_value[jrun][0][jt1];
				  sum1 += dt * SRTM.reference_region[jrun][jt1];
				}
			    }
			  else
			    {
			      printf("Programming error: event type k2 (ID=%c) is not constant in time.\n",
				     SRTM_EventChar(SRTM.event_id[jcoeff]));
			      exit(1);
			    }
			  SRTM.Xorig_tc[itime][jcoeff] = sum1;
			}
		    } // loop over periods
		} // loop over time
	    } // loop over runs
	}
    } // loop over coeff

  if ( SRTM_define_dynamic_basis_functions(SRTM.ntime, SRTM.target_t) )
    return(1);

  return(0);
}

int SRTM_define_dynamic_basis_functions( int ntime, float *tissue_vector )
// Define basis vectors that depend upon the reference region.
{
  int itype, istart_run, icolumn, ishape, itime, iRowFail;
  int jt, jt1, jcoeff, jcoeff1, jrun, jperiod, jcond, jc1, jc2;
  float sum, max, dt, tau, multiplier, time, time0, time1, time_stop, gmax;

  if ( ntime != SRTM.ntime )
    {
      printf("Error: the length of the tissue vector (%d) != time dimension of GLM (%d)\n",
	     ntime,SRTM.ntime);
      return(1);
    }

  // Assign weights as the 1/sqrt(n)
  for (jt=0, max=0.; jt<ntime; jt++)
    {
      if ( tissue_vector[jt] == 0. )
	SRTM.Weight_t[jt] = 0.;
      else
	SRTM.Weight_t[jt] = 1./sqrt((double)tissue_vector[jt]);
      /*
	{
	  time = SRTM_time_run_minutes(jt);
	  SRTM.Weight_t[jt] = time/20. * exp(-(double)(time/20.));
	}
      */
      if ( SRTM.Weight_t[jt] > max )
	max = SRTM.Weight_t[jt];
    }
  if ( max != 0. )
    {
      // Normalize weights
      for (jt=0; jt<ntime; jt++)
	//	SRTM.Weight_t[jt] /= max;
	SRTM.Weight_t[jt] = 1.;
    }

  // Define dynamic (tissue-dependent) basis functions.
  // Define static basis functions.
  for (jcoeff=0; jcoeff<SRTM.ncoeff; jcoeff++)
    {
      itype   = SRTM.event_type[jcoeff];
      ishape  = SRTM.event_shape[jcoeff];
      icolumn = SRTM.icolumn[jcoeff];  // table events only
      tau     = SRTM.tau[jcoeff];      // gamma & sigmoid events only
      if ( SRTM.event_type[jcoeff] == TYPE_k2a )
	{ // coefficient = k2a  ;    basis function = -[integral of target]
	  for (jrun=0; jrun<SRTM.nruns; jrun++)
	    {
	      istart_run = SRTM.run_istart[jrun];
	      for (jt=0; jt<SRTM.nt[jrun]; jt++)
		{
		  itime = istart_run + jt;
		  time  = SRTM_time_total_minutes(itime);
		  SRTM.Xorig_tc[itime][jcoeff] = 0.;  // default
		  for (jperiod=0; jperiod<SRTM.nperiods[jcoeff]; jperiod++)
		    {
		      time0 = SRTM.period_start[jcoeff][jperiod];
		      if ( time >= time0 && time <= SRTM.period_stop[jcoeff][jperiod] )
			{
			  if ( SRTM.event_shape[jcoeff] == SHAPE_CONSTANT )
			    {
			      multiplier = 1.;
			      for  (jt1=0, sum=0.; jt1<=jt; jt1++)
				{
				  dt   = SRTM.table_value[jrun][0][jt1];
				  sum += dt * multiplier * tissue_vector[istart_run + jt1];
				}
			    }
			  else if ( SRTM.event_shape[jcoeff] == SHAPE_GAMMA )
			    {
			      for  (jt1=0, sum=0.; jt1<=jt; jt1++)
				{
				  time1 = SRTM_time_total_minutes(istart_run+jt1);
				  // printf("time0 = %g, time1[%d] = %g, tau = %g\n",time0,istart_run+jt1,time1,tau);
				  dt    = SRTM.table_value[jrun][0][jt1];
				  if ( time1 > time0 )
				    multiplier = (time1-time0)/tau * exp((double)(-(time1-time0)/tau)) / exp(-1.);
				  else
				    multiplier = 0.;
				  sum += dt * multiplier * tissue_vector[istart_run + jt1];
				}
			    }
			  else if ( SRTM.event_shape[jcoeff] == SHAPE_GAMMAINT )
			    {
			      time_stop = SRTM.period_stop[jcoeff][jperiod];
			      gmax      = (time0+tau) * exp(-(double)(time0/tau)) - (time_stop+tau) * exp(-(double)(time_stop/tau));
			      for  (jt1=0, sum=0.; jt1<=jt; jt1++)
				{
				  time1 = SRTM_time_total_minutes(istart_run+jt1);
				  // printf("time0 = %g, time1[%d] = %g, tau = %g\n",time0,istart_run+jt1,time1,tau);
				  dt    = SRTM.table_value[jrun][0][jt1];
				  if ( time1 > time0 )
				    multiplier = (time0+tau) * exp(-(double)(time0/tau)) - (time1+tau) * exp(-(double)(time1/tau));
				  else
				    multiplier = 0.;
				  multiplier /= gmax;
				  sum += dt * multiplier * tissue_vector[istart_run + jt1];
				}
			    }
			  else if ( SRTM.event_shape[jcoeff] == SHAPE_SIGMOID )
			    {
			      for  (jt1=0, sum=0.; jt1<=jt; jt1++)
				{
				  time1 = SRTM_time_total_minutes(istart_run+jt1);
				  dt   = SRTM.table_value[jrun][0][jt1];
				  if ( time1 > time0 )
				    multiplier = (time1-time0)/tau / sqrt((double)(1+SQR((time1-time0)/tau)));
				  else
				    multiplier = 0.;
				  sum += dt * multiplier * tissue_vector[istart_run + jt1];
				}
			    }
			  else if ( SRTM.event_shape[jcoeff] == SHAPE_TABLE )
			    {
			      for  (jt1=0, sum=0.; jt1<=jt; jt1++)
				{
				  multiplier = SRTM.table_value[jrun][icolumn][jt1];
				  dt   = SRTM.table_value[jrun][0][jt1];
				  sum += dt * multiplier * tissue_vector[istart_run + jt1];
				}
			    }
			  SRTM.Xorig_tc[itime][jcoeff] = -sum;
			}
		    } // loop over periods
		} // loop over time
	    } // loop over runs
	}
    } // loop over coeff

  // Multiply X_tc by the inclusion vector
  for (jcoeff=0; jcoeff<SRTM.ncoeff; jcoeff++)
    {
      for (jt=0; jt<SRTM.ntime; jt++)
	{
	  SRTM.X_tc[jt][jcoeff] = SRTM.Xorig_tc[jt][jcoeff] * SRTM.include_time[jt];
	}
    }

  // Calculate (X_transpose * X) by summing over the time index.
  for (jcoeff=0; jcoeff<SRTM.ncoeff; jcoeff++)
    {
      for (jcoeff1=0; jcoeff1<SRTM.ncoeff; jcoeff1++)
	{
	  SRTM.XTXm1_cc[jcoeff][jcoeff1] = 0.;
	  for (jt=0; jt<SRTM.ntime; jt++)
	    SRTM.XTXm1_cc[jcoeff][jcoeff1] += SRTM.X_tc[jt][jcoeff] * SRTM.Weight_t[jt] * SRTM.X_tc[jt][jcoeff1];
	}
    }

  // Invert to get (X_transpose * X)^-1
  rInvertSquareMatrix(SRTM.ncoeff, SRTM.XTXm1_cc, &iRowFail);

  // Calculate (X_transpose * X)^-1 * X_transpose
  // by summing over the shared coefficient index.
  for (jcoeff=0; jcoeff<SRTM.ncoeff; jcoeff++)
    {
      for (jt=0; jt<SRTM.ntime; jt++)
	{
	  SRTM.XTXm1XT_ct[jcoeff][jt] = 0.;
	  for (jcoeff1=0; jcoeff1<SRTM.ncoeff; jcoeff1++)
	    SRTM.XTXm1XT_ct[jcoeff][jt] += SRTM.XTXm1_cc[jcoeff][jcoeff1] * SRTM.X_tc[jt][jcoeff1];
	}
    }

  // Update the covariance matrix used in the statistical tests.
  for (jcond=0; jcond<SRTM.nconditions; jcond++)
    {
      // Define the F convariance matrix for each condition.
      for (jc1=0; jc1<SRTM.ncontrasts[jcond]; jc1++)
	{
	  for (jc2=0; jc2<SRTM.ncontrasts[jcond]; jc2++)
	    {
	      SRTM.FCovar[jcond][jc1][jc2] = 0.;
	      for (jcoeff=0; jcoeff<SRTM.ncoeff; jcoeff++)
		{
		  for (jcoeff1=0; jcoeff1<SRTM.ncoeff; jcoeff1++)
		    {
		      SRTM.FCovar[jcond][jc1][jc2]  +=   SRTM.contrast_matrix[jcond][jc1][jcoeff]
			* SRTM.XTXm1_cc[jcoeff][jcoeff1] * SRTM.contrast_matrix[jcond][jc2][jcoeff1];
		    }
		}
	    }
	}
      // Make a copy for the inverse
      for (jc1=0; jc1<SRTM.ncontrasts[jcond]; jc1++)
	{
	  for (jc2=0; jc2<SRTM.ncontrasts[jcond]; jc2++)
	    SRTM.FCovarInv[jcond][jc1][jc2] = SRTM.FCovar[jcond][jc1][jc2];
	}
      // Now invert the matrix.
      rInvertSquareMatrix( SRTM.ncontrasts[jcond], SRTM.FCovarInv[jcond], &iRowFail );
    }

  SRTM.initialized = 2;
  return(0);
}

int SRTM_define_conditions( char *condition_string )
     // DefineContrasts: 0 = just get strings, 1 = define contrast matrix
{ // must be called AFTER SRTM_define_basis_functions()
  char *cptr, *c_event_id, cond_str[LEN_STR], event_char;
  int event_id, isign, length, found, iRowFail;
  int jchar, jcoeff, jcoeff1, jcon, jc1, jc2, icondition, icontrast;
  static int ncoeff_save=0;

  if ( SRTM.initialized != 2 ) // then the basis functions have been defined
    { // SRTM.initialized != 2
      printf("Error: SRTM_define_conditions called but SRTM.initialized = %d\n",SRTM.initialized);
      return(1);
    }

  // For the 1st pass, ncoeff_save will be 0, so set it.
  if ( ncoeff_save == 0 ) ncoeff_save = SRTM.ncoeff;

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
  SRTM.nconditions = 0;
  for (jcon=0; jcon<MAX_CONDITIONS; jcon++)
    {
      // Free contrast matrix for each condition.
      if ( SRTM.contrast_matrix[jcon] != NULL )
	free_int_matrix(SRTM.contrast_matrix[jcon],SRTM.ncontrasts[jcon],ncoeff_save);
      // Free F inverse-covariance matrix for each condition.
      if ( SRTM.FCovar[jcon] != NULL )
	free_float_matrix(SRTM.FCovar[jcon], SRTM.ncontrasts[jcon], SRTM.ncontrasts[jcon]);
      if ( SRTM.FCovarInv[jcon] != NULL )
	free_float_matrix(SRTM.FCovarInv[jcon], SRTM.ncontrasts[jcon], SRTM.ncontrasts[jcon]);
      // C*Beta vector for each condition
      // Reset contrasts to 0
      SRTM.ncontrasts[jcon]       = 0;
      SRTM.BP_condition[jcon]     = FALSE;
    }

  // Save number of coefficients, since this info is needed to release arrays above.
  ncoeff_save = SRTM.ncoeff;

  // If the condition string is blank, return
  if ( sscanf(condition_string,"%d",&isign) < 0 )
    {
      SRTM.nconditions = 0;
      return(0);
    }

  // Count the number of elements in the condition string.
  cptr=condition_string;
  while ( sscanf(cptr,"%s",cond_str) == 1 )
    {
      // For each condition, count the number of ',' characters, indicating new matrix rows.
      length = jip_int_count((JIP_INDEX)strlen(cond_str));
      for (jchar=0, SRTM.ncontrasts[SRTM.nconditions]=0;
	   jchar<length; jchar++)
	{ // Each new appearance of ',' indicates a new row in the contrast matrix
	  if ( *(cptr+jchar) == ',' ) SRTM.ncontrasts[SRTM.nconditions]++;
	}
      SRTM.ncontrasts[SRTM.nconditions]++;         // There is 1 row, even if no ',' was found
      
      // Allocate the contrast matrix for this condition, and initialize it.
      SRTM.contrast_matrix[SRTM.nconditions] = int_matrix(SRTM.ncontrasts[SRTM.nconditions],SRTM.ncoeff);
      for (jcon=0; jcon<SRTM.ncontrasts[SRTM.nconditions]; jcon++)
	{
	  for (jcoeff=0; jcoeff<SRTM.ncoeff; jcoeff++)
	    SRTM.contrast_matrix[SRTM.nconditions][jcon][jcoeff] = 0;
	}
      // Update the number of conditions.
      SRTM.nconditions++;
      cptr = next_non_white(cptr+jip_int_count((JIP_INDEX)strlen(cond_str)));
    }
  
  // Pick apart each condition and analyze the string.
  icondition = 0;
  cptr=condition_string;
  while ( sscanf(cptr,"%s",cond_str) == 1 )
    {
      icontrast = 0;
      // cond_str is the individual condition to be analyzed.
      strcpy(SRTM.condition_name[icondition],cond_str); // save string for identification
      length = jip_int_count((JIP_INDEX)strlen(cond_str));
      isign = 1;
      for (jchar=0; jchar<length; jchar++)
	{
	  c_event_id = cond_str+jchar;
	  event_id = SRTM_EventNumberFromChar(*c_event_id);
	  if ( *c_event_id == ',' )
	    {
	      icontrast++;  // end this contrast, and make a new row in the contrast matrix
	      isign = 1;    // reset the sign for the new contrast vector
	    }
	  else if ( *c_event_id == '-' )
	    isign = -1;   // change the sign of the event in the test contrast
	  else
	    {
	      // Loop over the number of stimulus events.
	      for (jcoeff=found=0; jcoeff<SRTM.ncoeff; jcoeff++)
		{
		  if ( SRTM.event_id[jcoeff] == event_id )
		    {
		      found = 1;
		      // If an event occurs n times in a condition, the value will be n.
		      SRTM.contrast_matrix[icondition][icontrast][jcoeff] += isign;
		    }
		}
	      if ( !found )
		{ // Make sure the specified event was found.
		  if ( SRTM_EventCharFromInteger(event_id, &event_char) )
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
      // Update the condition counter and go to the next condition string.
      icondition++;
      cptr = next_non_white(cptr+jip_int_count((JIP_INDEX)strlen(cond_str)));
      
      // The conditions have been defined. Now calculate the covariance matrix.
      for (jcon=0; jcon<SRTM.nconditions; jcon++)
	{
	  // Allocate the F covariance matrix for each condition.
	  SRTM.FCovar[jcon]    = float_matrix(SRTM.ncontrasts[jcon],SRTM.ncontrasts[jcon]);
	  SRTM.FCovarInv[jcon] = float_matrix(SRTM.ncontrasts[jcon],SRTM.ncontrasts[jcon]);
	  // Define the F convariance matrix for each condition.
	  for (jc1=0; jc1<SRTM.ncontrasts[jcon]; jc1++)
	    {
	      for (jc2=0; jc2<SRTM.ncontrasts[jcon]; jc2++)
		{
		  SRTM.FCovar[jcon][jc1][jc2] = 0.;
		  for (jcoeff=0; jcoeff<SRTM.ncoeff; jcoeff++)
		    {
		      for (jcoeff1=0; jcoeff1<SRTM.ncoeff; jcoeff1++)
			{
			  SRTM.FCovar[jcon][jc1][jc2]  +=   SRTM.contrast_matrix[jcon][jc1][jcoeff]
			    * SRTM.XTXm1_cc[jcoeff][jcoeff1] * SRTM.contrast_matrix[jcon][jc2][jcoeff1];
			}
		    }
		}
	    }
	  // Make a copy for the inverse
	  for (jc1=0; jc1<SRTM.ncontrasts[jcon]; jc1++)
	    {
	      for (jc2=0; jc2<SRTM.ncontrasts[jcon]; jc2++)
		SRTM.FCovarInv[jcon][jc1][jc2] = SRTM.FCovar[jcon][jc1][jc2];
	    }
	  // Now invert the matrix.
	  rInvertSquareMatrix( SRTM.ncontrasts[jcon], SRTM.FCovarInv[jcon], &iRowFail );
	}
    }
  for (jcon=0; jcon<SRTM.nconditions; jcon++)
    // Determine if this condition can have an associated BP calculation.
    SRTM.BP_condition[jcon] = SRTMInternal_BP_condition(jcon);

  return(0);
}

void SRTMInternal_describe_glm_format()
{ // This is printed as the results of input error
  printf("\nExample glm input file:\n\n");
  printf("conditions 1 2 12 1-2           # mandatory; Statistical tests MUST be included\n");
  printf("runs:                           # mandatory; This means that runs follow.\n");
  printf("../srtm1.nii srtm1.glm srtm1.table # start listing runs, each with a GLM description and table\n");
  printf("../srtm2.nii srtm2.glm srtm2.table # ... and so on\n");
}

int SRTM_fit_data( int ntime, float *data_vector, float *fit_vector, float *sigma2  )
     // input:
     //        ntime      : # points in time series; dimension for data_vector/fit_vector
     //        data_vector: Y in the equation Y = X * Beta
     // output: 
     //        fit_vector : fit to data (X * Beta);       [this is just for display]
     //        sigma2     : sum of (data-fit)^2 / DOF     [IMPORTANT: this is used in statistics]
{
  float y_prime, fit;
  int jt, jcoeff;

  // Make sure the GLM has been initialized.
  if ( SRTM.initialized != 2 )
    {
      printf("Error in SRTM_fit_data: first initialize the GLM (%d) and define stimuli\n",
	     SRTM.initialized);
      return(1);
    }
  // Make sure the GLM model and the input data are consistent.
  if ( ntime != SRTM.ntime )
    {
      printf("Error: the number of data time points (%d) does not match the number in the GLM (%d).\n",
	     ntime,SRTM.ntime);
      exit(1);
    }

  // Calculate the coefficients of the hemodynamic function Beta_c.
  for (jcoeff=0; jcoeff<SRTM.ncoeff; jcoeff++)
    {
      SRTM.Beta_c[jcoeff] = 0.;
      // Calculate Beta, summing over time
      for (jt=0; jt<SRTM.ntime; jt++)
	{
	  y_prime = data_vector[jt] * SRTM.Weight_t[jt] * SRTM.include_time[jt];
	  SRTM.Beta_c[jcoeff] += SRTM.XTXm1XT_ct[jcoeff][jt] * y_prime;
	}
    }
  // Compute the fit vector and the variance between the data and fit.
  *sigma2 = 0.;
  for (jt=0; jt<SRTM.ntime; jt++)
    {
      fit = 0.;             // this must done correctly for stats
      fit_vector[jt] = 0.;  // this is just for display
      for (jcoeff=0; jcoeff<SRTM.ncoeff; jcoeff++)
	{
	  fit            += SRTM.X_tc[jt][jcoeff]     * SRTM.Beta_c[jcoeff];
	  fit_vector[jt] += SRTM.Xorig_tc[jt][jcoeff] * SRTM.Beta_c[jcoeff];
	}
      y_prime = data_vector[jt] * SRTM.include_time[jt];
      *sigma2 += SQR(y_prime - fit);
    }
  // The variance is divided the degrees of freedom.
  *sigma2 /= SRTM.dof;

  return(0);
}

BOOL SRTMInternal_BP_condition( int iCondition )
{
// "iCondition" must satisfy these criteria:
// 1) All parameters in the condition are TYPE_k2a,
// 2) There is exactly 1 time-overlapping constant k2 value.
// IF SO, return TRUE because BP can be calculated for this condition.
  char condition_name[LEN_STR];
  int irun, iCoeff_k2, iCoeff_R1, nk2, nR1, lCondition;
  int jcoeff, jcoeff1, jperiod, jperiod1;
  float start, stop;
  BOOL overlap;

  SRTM.k2_condition_for_BP[iCondition] = SRTM.R1_condition_for_BP[iCondition] = -1;
  SRTM_get_condition_name( iCondition, condition_name );

  if ( SRTM.ncontrasts[iCondition] != 1 )  // don't allow F tests
    return(FALSE);
  else
    {
      iCoeff_k2 = iCoeff_R1 = -1;
      nk2 = nR1 = 0;
      // Make sure every parameter in the condition is TYPE_k2a.
      for (jcoeff=0; jcoeff<SRTM.ncoeff; jcoeff++)
	{
	  if ( SRTM.contrast_matrix[iCondition][0][jcoeff] != 0 )
	    { // coeff is in contrast, so test type and presence of k2
	      if ( SRTM.event_type[jcoeff] != TYPE_k2a )
		return(FALSE);
	      // Ensure 1 k2 value.
	      for (jperiod=0; jperiod<SRTM.nperiods[jcoeff]; jperiod++)
		{
		  start = SRTM.period_start[jcoeff][jperiod];
		  stop  = SRTM.period_stop [jcoeff][jperiod];
		  irun = SRTM_get_run_number_from_time(start);
		  if ( SRTM.k2prime[irun] != 0. )
		    {
		      nk2 = 1;  // no other k2 values should exist in this run
		      for (jcoeff1=0; jcoeff1<SRTM.ncoeff; jcoeff1++)
			{
			  if ( SRTM.event_type[jcoeff1] == TYPE_R1 )
			    {
			      for (jperiod1=0; jperiod1<SRTM.nperiods[jcoeff1]; jperiod1++)
				{
				  overlap  = (start >= SRTM.period_start[jcoeff1][jperiod1] &&
					      start <= SRTM.period_stop[jcoeff1][jperiod1] );
				  overlap |= (stop  >= SRTM.period_start[jcoeff1][jperiod1] &&
					      stop  <= SRTM.period_stop[jcoeff1][jperiod1] );
				  if ( overlap )
				    {
				      if ( SRTM.event_shape[jcoeff1] == SHAPE_CONSTANT )
					{ // require a single and constant R1
					  if ( jcoeff1 != iCoeff_R1 )
					    nR1++;
					  iCoeff_R1 = jcoeff1;
					}
				    }
				} // period1
			    }
			} // coeff1
		    }
		  else
		    {
		      nR1 = 1;  // For a variable k2, R1 is not needed for BP error bars, so r1 is irrelevant
		      for (jcoeff1=0; jcoeff1<SRTM.ncoeff; jcoeff1++)
			{
			  if ( SRTM.event_type[jcoeff1] == TYPE_k2 )
			    {
			      for (jperiod1=0; jperiod1<SRTM.nperiods[jcoeff1]; jperiod1++)
				{
				  overlap  = (start >= SRTM.period_start[jcoeff1][jperiod1] &&
					      start <= SRTM.period_stop[jcoeff1][jperiod1] );
				  overlap |= (stop  >= SRTM.period_start[jcoeff1][jperiod1] &&
					      stop  <= SRTM.period_stop[jcoeff1][jperiod1] );
				  if ( overlap )
				    {
				      if ( SRTM.event_shape[jcoeff1] != SHAPE_CONSTANT )
					{
					  printf("Programming error in SRTMInternal_BP_condition()\n");
					  exit(1);
					}
				      if ( jcoeff1 != iCoeff_k2 )
					nk2++;
				      iCoeff_k2 = jcoeff1;
				    }
				} // period1
			    }
			} // coeff1
		    }
		} // period
	    }
	} // coeff

      if ( nk2 == 1 && nR1 == 1 )
	{
	  if ( SRTM.k2prime[irun] != 0. )
	    { // The error in k2 comes from R1, since k2' is fixed
	      if ( (lCondition=SRTM_lSingleCondition(iCoeff_R1)) >= 0 )
		SRTM.R1_condition_for_BP[iCondition] = lCondition;
	      else
		printf("BP can be computed for condition %s, but there will be no error bar (check R1).\n",condition_name);
	    }
	  else
	    { // The error in k2 comes from k2, since k2 is spatially variable
	      if ( (lCondition=SRTM_lSingleCondition(iCoeff_k2)) >= 0 )
		SRTM.k2_condition_for_BP[iCondition] = lCondition;
	      else
		printf("BP can be computed for condition %s, but there will be no error bar (check k2).\n",condition_name);
	    }
	  return(TRUE);
	}
      else
	{
	  if ( nk2 != 1 )
	    printf("Condition %s is of type 'k2a', but BP cannot be computed for it (check k2).\n",condition_name);
	  else if ( nR1 != 1 )
	    printf("Condition %s is of type 'k2a', but BP cannot be computed for it (check R1).\n",condition_name);
	  return(FALSE);
	}
    }
}

BOOL SRTM_BP_condition( int iCondition )
{
  return(SRTM.BP_condition[iCondition]);
}

float SRTM_calculate_BP( int iCondition, float sigma2, float *BP_error )
// "effect_size" is the signal value for the condition (some combination of Beta_c values).
// "iCondition" must satisfy these criteria:
// 1) All parameter in the condition are TYPE_k2a,
// 2) There is at least one time-overlapping constant k2 value (should always be true)
// IF SO, calculate and return BP.
// IF NOT, return 0
{
  int jcoeff, iTime_total, irun, lCondition;
  float Time, k2a, k2, r1, BP, k2a_err, k2_err;

  if ( !SRTM.BP_condition[iCondition] )
    return(0.);
  else
    {
      k2a = 0.;
      for (jcoeff=0; jcoeff<SRTM.ncoeff; jcoeff++)
	{
	  if ( SRTM.contrast_matrix[iCondition][0][jcoeff] != 0 ) // contrast "0" because F tests not allowed here
	    {
	      if ( SRTM.event_shape[jcoeff] == SHAPE_CONSTANT )
		Time = SRTM.period_start[jcoeff][0];
	      else if ( SRTM.event_shape[jcoeff] == SHAPE_GAMMA )
		Time = SRTM.period_start[jcoeff][0] + SRTM.tau[jcoeff];
	      else if ( SRTM.event_shape[jcoeff] == SHAPE_GAMMAINT )
		Time = SRTM.period_stop[jcoeff][0];
	      else if ( SRTM.event_shape[jcoeff] == SHAPE_SIGMOID )
		Time = SRTM.period_stop[jcoeff][0];
	      else if ( SRTM.event_shape[jcoeff] == SHAPE_TABLE )
		Time = SRTM_time_total_minutes(SRTM.iTimeMax[jcoeff]);  // table events only
	      k2a += SRTM.contrast_matrix[iCondition][0][jcoeff] * SRTM.Beta_c[jcoeff];
	      iTime_total = SRTM_get_itime_from_time(Time);  // any time should do, since k2 is always constant
	    }
	}
      k2a_err = sqrt(fabs((double)(SRTM.FCovar[iCondition][0][0] * sigma2)));
      k2 = SRTM_get_parameter_value(-TYPE_k2,iTime_total);
      if ( k2 == 0. )
	{ // k2' is fixed, so get k2 from k2' * R1 and get k2 error from k2' * delta_R1
	  irun = SRTM_get_run_number(iTime_total);
	  if ( SRTM.k2prime[irun] != 0. )
	    {
	      r1 = SRTM_get_parameter_value(TYPE_R1,iTime_total);
	      k2 = SRTM.k2prime[irun] * r1;
	      lCondition = SRTM.R1_condition_for_BP[iCondition];
	      if ( lCondition >= 0 )
		k2_err = SRTM.k2prime[irun] * sqrt(fabs((double)(SRTM.FCovar[lCondition][0][0] * sigma2)));
	      else
		k2_err = 0.;
	    }
	  else
	    {
	      printf("Programming error: k2 = 0 in SRTM_calculate_BP()\n");
	      exit(1);
	    }
	}
      else
	{ // k2' is variable, not fixed, so get k2 error directly from k2 condition
	  lCondition = SRTM.k2_condition_for_BP[iCondition];
	  if ( lCondition >= 0 )
	    k2_err = sqrt(fabs((double)(SRTM.FCovar[lCondition][0][0] * sigma2)));
	  else
	    k2_err = 0.;
	}
      if ( k2a == 0 )
	BP = *BP_error = 0.;
      else
	{
	  BP = k2 / k2a - 1.;
	  if ( k2 != 0. && k2_err != 0. )
	    *BP_error = BP * sqrt((double)(SQR(k2a_err/k2a) + SQR(k2_err/k2)));
	  else
	    *BP_error = 0.;
	}
      return(BP);
    }
}

int SRTM_calculate_F(int ntime,          // input: number of time points
		    float sigma2,       // input: variance between data and fit
		    JIP_INDEX resels,         // resolution elements (for multiple comparisons)
		    int lCondition,     // an index between 0 and SRTM.nconditions-1
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
  if ( lCondition < 0 || lCondition >= SRTM.nconditions )
    {
      printf("Error: the specified condition (%d) is out of range (0-%d)\n",
	     lCondition,SRTM.nconditions);
      return(1);
    }
  else if ( ntime != SRTM.ntime )
    { // Make sure that the number of time points is consistent with the number used to fit the SRTM.
      printf("Error: # time points != # used in GLM fit.\n");
      return(1);
    }

  CBeta = float_vector(SRTM.ncontrasts[lCondition]);
  // Calculate the product CBeta
  for (jc1=0, lContrastMax=0, CBetaMax=0.;
       jc1<SRTM.ncontrasts[lCondition];
       jc1++)
    {
      CBeta[jc1] = 0;
      for (jcoeff=0; jcoeff<SRTM.ncoeff; jcoeff++)
	CBeta[jc1] += SRTM.contrast_matrix[lCondition][jc1][jcoeff] * SRTM.Beta_c[jcoeff];
      if ( fabs((double)CBeta[jc1]) > fabs((double)CBetaMax) )
	{
	  CBetaMax = CBeta[jc1];
	  lContrastMax = jc1;
	}
    }
  // Determine the effect size and standard deviation.
  *effect_size = CBetaMax;
  *effect_sdev = sqrt(fabs((double)(SRTM.FCovar[lCondition][lContrastMax][lContrastMax] * sigma2)));

  // Compute the numerator for F
  numerator = 0.;
  for (jc1=0; jc1<SRTM.ncontrasts[lCondition]; jc1++)
    {
      for (jc2=0; jc2<SRTM.ncontrasts[lCondition]; jc2++)
	numerator += CBeta[jc1]
	  * SRTM.FCovarInv[lCondition][jc1][jc2] * CBeta[jc2];
    }
  free_float_vector(CBeta);

  // Degrees of freedom
  df1 = SRTM.dof;
  df2 = SRTM.ncontrasts[lCondition];

  Fstat = numerator / sigma2 / df2;
  *cnr = sqrt((double)fabs((double)Fstat));

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

int SRTM_event_is_defined( int event_id )
{
  int jcoeff, found;

  for (jcoeff=0, found=0; jcoeff<SRTM.ncoeff; jcoeff++)
    {
      if ( SRTM.event_id[jcoeff] == event_id )
	found = 1;
    }
  return(found);
}

void SRTM_define_residue_from_inclusion(char *include_string, char *residue_string)
{ // Put all defined table events into a residue integer.
  char event_string[LEN_STR];
  int jcoeff, event_id;

  strcpy(residue_string,"");
  for (jcoeff=0; jcoeff<SRTM.ncoeff; jcoeff++)
    { // Loop over all events.
      if ( !SRTM_event_is_in_string(include_string, jcoeff) )
	{
	  event_id = SRTM.event_id[jcoeff];
	  SRTM_EventStringFromInteger(event_id,event_string);
	  strcat(residue_string,event_string);
	}
    }
}

int SRTM_invalid_residue(char *residue_string)
     // return the invalid event number if one exists in residue_string
{
  int event_id;JIP_INDEX j;
  char event_char;

  if ( jip_int_count((JIP_INDEX)strlen(residue_string)) == 0 ) return(0);
  for (j=0; j<jip_int_count((JIP_INDEX)strlen(residue_string)); j++)
    {
      event_char = residue_string[j];
      event_id   = SRTM_EventNumberFromChar(event_char);
      // Make sure the event is valid.
      if ( event_id != 0 && !SRTM_event_is_defined(event_id) < 0 )
	return(event_id);
    }
  return(0);
}

int SRTM_event_is_in_string(char *string, int index_event)
{
  int event_id;JIP_INDEX j;
  char event_char;

  if ( SRTM_invalid_residue(string) )
    {
      printf("Programming error: test for residue validity first.\n");
      exit(1);
    }

  // Find the event corresponding to the given coefficient.
  if ( jip_int_count((JIP_INDEX)strlen(string)) == 0 ) return(0);
  for (j=0; j<jip_int_count((JIP_INDEX)strlen(string)); j++)
    {
      event_char   = string[j];
      event_id = SRTM_EventNumberFromChar(event_char);
      if ( event_id == SRTM.event_id[index_event] ) return(1);
    }
  return(0);
}

int SRTM_create_residue( int ntime, float *data_vector, float *fit_vector )
{
  int jcoeff, jtime;

  if ( ntime != SRTM.ntime )
    {
      printf("Error: # time points (%d) does not match GLM (%d) in GLM_adjust_event()\n",
	     ntime,SRTM.ntime);
      return(1);
    }
  else if ( SRTM.ncoeff < 1 )
    {
      printf("Error: no coefficients assigned yet in GLM_adjust.\n");
      return(1);
    }

  // Loop over drift terms, removing each from all the runs.
  for (jtime=0; jtime<SRTM.ntime; jtime++)
    {
      for ( jcoeff=0; jcoeff<SRTM.ncoeff; jcoeff ++ )
	{
	  fit_vector[jtime]  -= SRTM.X_tc[jtime][jcoeff] * SRTM.Beta_c[jcoeff];
	  data_vector[jtime] -= SRTM.X_tc[jtime][jcoeff] * SRTM.Beta_c[jcoeff];
	}
    }
  return(0);
}

int SRTM_create_selective_residue( char *residue_string, int ntime, float *data_vector, float *fit_vector )
     // residue_string: "" = remove no events, "1" = remove event 1, "12" = remove events 1 & 2, ...
{
  int event_id, remove_coefficient;
  int jcoeff, jtime;

  // ignore event numbers below zero.
  if ( jip_int_count((JIP_INDEX)strlen(residue_string)) == 0 )
    return(0);
  else
    {
      if ( (event_id=SRTM_invalid_residue(residue_string)) )
	{
	  printf("Error: event %c is not valid in the residue (SRTM_create_selective_residue)\n",
		 SRTM_EventChar(event_id));
	  return(1);
	}
    }

  if ( ntime != SRTM.ntime )
    {
      printf("Error: # time points (%d) does not match SRTM (%d) in SRTM_adjust_event()\n",
	     ntime,SRTM.ntime);
      return(1);
    }
  else if ( SRTM.ncoeff <= 0 )
    {
      printf("Error: no coefficients assigned yet in SRTM_adjust.\n");
      return(1);
    }

  // Remove all other stimuli.
  for ( jcoeff=0; jcoeff < SRTM.ncoeff; jcoeff ++ )
    {
      // remove this coefficient from both the data and the fit if it's in the residue
      remove_coefficient = SRTM_event_is_in_string(residue_string, jcoeff);
      if ( remove_coefficient )
	{
	  for (jtime=0; jtime<SRTM.ntime; jtime++)
	    {
	      data_vector[jtime] -= SRTM.Xorig_tc[jtime][jcoeff] * SRTM.Beta_c[jcoeff];
	      fit_vector[jtime]  -= SRTM.Xorig_tc[jtime][jcoeff] * SRTM.Beta_c[jcoeff];
	    }
	}
    }
  return(0);
}

int SRTM_get_number_events_inside_residue( char *residue_string )
{
  int found_id[MAX_COEFF];
  int jcoeff, event_id, nevents;

  if ( SRTM_invalid_residue(residue_string) )
    {
      printf("Programming error: test for residue validity first.\n");
      exit(1);
    }

  for (jcoeff=0; jcoeff<SRTM.ncoeff; jcoeff++)
    found_id[jcoeff] = 0;

  // Set values in the found_event array.
  for (jcoeff=0, nevents=0; jcoeff<SRTM.ncoeff; jcoeff++)
    {
      if ( SRTM_event_is_in_string(residue_string, jcoeff) )
	{
	  event_id = SRTM.event_id[jcoeff];
	  found_id[event_id] = 1;
	}
    }
  // Count the number of values in the found_event array.
  for (jcoeff=0, nevents=0; jcoeff<MAX_COEFF; jcoeff++)
    if ( found_id[jcoeff] ) nevents++;
  return(nevents);
}

int SRTM_increment_event_id( int event_id )
{
  // This function is designed for an interactive program
  // that steps events based upon the existing events.
  int jcoeff, found, event_id_new;JIP_INDEX index_coeff;

  if ( SRTM.ncoeff == 0 )
    return(0);  // error condition: nothing defined
  else
    {
      // Check to see if the event exists.
      for ( jcoeff=0, found=0; jcoeff<SRTM.ncoeff; jcoeff++ )
	{
	  if ( event_id == SRTM.event_id[jcoeff] )
	    {
	      found = 1;
	      index_coeff = jcoeff;
	    }
	}
      if ( !found )
	return(0);
      else
	{
	  // If this is not the last event in the list, go to the next one.
	  if ( index_coeff < SRTM.ncoeff-1 )
	    event_id_new = SRTM.event_id[index_coeff+1];
	  else // return the 1st one
	    event_id_new = SRTM.event_id[0];
	}
      return(event_id_new);
    }
}
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
///////////// Functions that describe the GLM ////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

int SRTM_binding_fit( int ibind, int itime, float *fit )
// ibind=0 means non-specific binding: R1 term
// ibind=1 means specific binding: total fit - R1 term
{
  int jcoeff;

  if ( itime >= SRTM.ntime )
    {
      printf("Error: time index (%d) >= SRTM time points (%d) in SRTM_binding_fit()\n",
	     itime,SRTM.ntime);
      return(1);
    }
  else if ( SRTM.ncoeff <= 0 )
    {
      printf("Error: no coefficients assigned yet in SRTM_binding_fit.\n");
      return(1);
    }

  if ( ibind == 0 )
    { // non-specific binding, so just R1 term
      *fit = 0;
      for ( jcoeff=0; jcoeff<SRTM.ncoeff; jcoeff++)
	{
	  if ( SRTM.event_type[jcoeff] == TYPE_R1 )
	    *fit += SRTM.Xmix_tc[itime][jcoeff] * SRTM.Beta_c[jcoeff];
	}
    }
  else
    { // specific binding: total fit minus R1 term
      *fit = 0;
      for ( jcoeff=0; jcoeff<SRTM.ncoeff; jcoeff++)
	{
	  *fit += SRTM.Xorig_tc[itime][jcoeff] * SRTM.Beta_c[jcoeff];  // total fit
	  if ( SRTM.event_type[jcoeff] == TYPE_R1 )
	    *fit -= SRTM.Xmix_tc[itime][jcoeff] * SRTM.Beta_c[jcoeff];
	}
    }
  return(0);
}

int SRTM_coeff_fit( int icoeff, int itime, float *fit )
{
  if ( itime >= SRTM.ntime )
    {
      printf("Error: time index (%d) >= SRTM time points (%d) in SRTM_coeff_fit()\n",
	     itime,SRTM.ntime);
      return(1);
    }
  else if ( SRTM.ncoeff <= 0 )
    {
      printf("Error: no coefficients assigned yet in SRTM_coeff_fit.\n");
      return(1);
    }
  *fit = SRTM.Xorig_tc[itime][icoeff] * SRTM.Beta_c[icoeff];

  return(0);
}

void SRTM_describe_input_files()
{
  int jrun;

  printf("\nFiles: DATA          GLM          TABLE\n");
  printf("-----------------------------------------\n");
  for (jrun=0; jrun<SRTM.nruns; jrun++)
    printf("%d %s %s %s\n",jrun+1,SRTM_input.run_name[jrun],SRTM_input.glm_name[jrun],SRTM_input.table_name[jrun]);
  printf("\n");
}

void SRTM_describe_basis_functions()
{
  FILE *plot_file;
  int jcoeff, jt;

  if ( !(plot_file = fopen("basis.dat","w")) )
    {
      printf("Cannot open basis function output file ...\n");
      return;
    }
  fprintf(plot_file,"x ");
  for (jcoeff=0; jcoeff<SRTM.ncoeff; jcoeff++)
    fprintf(plot_file,"column_%d ",jcoeff);
  fprintf(plot_file,"\n");
  for (jt=0; jt<SRTM.ntime; jt++)
    {
      fprintf(plot_file,"%d ",jt);
      for (jcoeff=0; jcoeff<SRTM.ncoeff; jcoeff++)
	{
	  fprintf(plot_file,"%f ",SRTM.Xorig_tc[jt][jcoeff]);
	}
      fprintf(plot_file,"\n");
    }
  fclose(plot_file);
}

void SRTM_describe_conditions()
{
  int jcond, jcont, jcoeff;

  printf("Conditions to be tested:\n");
  // Describe each decoded condition
  for (jcond=0; jcond<SRTM.nconditions; jcond++)
    {
      if ( SRTM.ncontrasts[jcond] == 1 )
	printf("%s (T test): ",SRTM.condition_name[jcond]);
      else
	printf("%s (F test): ",SRTM.condition_name[jcond]);
      for (jcont=0; jcont<SRTM.ncontrasts[jcond]; jcont++)
	{
	  for (jcoeff=0; jcoeff<SRTM.ncoeff; jcoeff++)
	    printf("%d ",SRTM.contrast_matrix[jcond][jcont][jcoeff]);
	  if ( SRTM.ncontrasts[jcond] != 0 && jcont != SRTM.ncontrasts[jcond]-1 )
	    printf("; ");
	}
      printf("\n");
    }
}

void SRTM_describe_fit()
{
  int jcoeff;

  for (jcoeff=0; jcoeff<SRTM.ncoeff; jcoeff++)
    printf("amplitude of basis function %d = %f\n",jcoeff,SRTM.Beta_c[jcoeff]);
}

int SRTM_get_condition_name( int ncondition, char *condition_name )
{
  if ( ncondition < 0 || ncondition >= SRTM.nconditions )
    return(1);
  else
    strcpy(condition_name,SRTM.condition_name[ncondition]);
  return(0);
}

int SRTM_is_initialized()
{
  return(SRTM.initialized);
}

int SRTM_get_nruns()
{
  return(SRTM.nruns);
}

int SRTM_get_nconditions()
{
  return(SRTM.nconditions);
}

int SRTM_get_number_coefficients()
{
  return(SRTM.ncoeff);
}

int SRTM_get_time_points()
{
  return(SRTM.ntime);
}

float SRTM_get_effect_size(int index_coeff )
{
  return(SRTM.Beta_c[index_coeff]);
}

int SRTM_get_ignored_cutoff( int iRun )
{
  if ( iRun < 0 || iRun >= SRTM.nruns )
    {
      printf("Programming error: run %d out of range in SRTM_get_ignored_cutoff()\n",iRun);
      exit(1);
    }
  else
    return(SRTM.inclusion_cutoff[iRun]);
}

int SRTM_get_run_number( int iTime_total )
{
  int jrun;
  int iRun=-1;

  if ( iTime_total < 0 )  // could happen when using "-e" option to attach anatomy
    iRun = 0;
  else
    {
      for (jrun=0; jrun<SRTM.nruns; jrun++)
	{
	  if ( iTime_total >= SRTM.run_istart[jrun] && iTime_total <= SRTM.run_istop[jrun] )
	    iRun = jrun;
	}
    }
  if ( iRun == -1 )
    {
      printf("Programming error: iRun undefined by SRTM_get_run_number()\n");
      exit(1);
    }
  else
    return(iRun);
}

int SRTM_get_run_number_from_time( float Time_total )
{
  int jrun;
  int iRun=-1;

  if ( Time_total < 0. )  // could happen when using "-e" option to attach anatomy
    iRun = 0;
  else
    {
      for (jrun=0; jrun<SRTM.nruns; jrun++)
	{
	  if ( Time_total >= SRTM_time_total_minutes(SRTM.run_istart[jrun]) && Time_total <= SRTM_time_total_minutes(SRTM.run_istop[jrun]) )
	    iRun = jrun;
	}
    }
  if ( iRun == -1 )
    {
      printf("Programming error: iRun undefined by SRTM_get_run_number_from_time()\n");
      exit(1);
    }
  else
    return(iRun);
}

int SRTM_get_itime_in_run( int iTime_total )
{
  int iTime_run, iRun, jrun;

  // Find the run
  iRun = SRTM_get_run_number(iTime_total);
  // Find the time point in the run.
  for (jrun=0, iTime_run=iTime_total; jrun<=iRun; jrun++)
    iTime_run -= SRTM.run_istart[jrun];
  return(iTime_run);
}

int SRTM_get_itime_in_run_from_time( float Time_total )
{
  int iTime_total, iRun;
  float time_total;

  // Find the run
  //  printf("problem starts, Time_total = %g\n",Time_total);
  iRun        = SRTM_get_run_number_from_time(Time_total);
  //  printf("iRun = %d\n",iRun);
  iTime_total = SRTM.run_istart[iRun];
  //  printf("iTime_total = %d\n",iTime_total);
  time_total  = SRTM_time_total_minutes(iTime_total);
  //  printf("time_total = %g\n",time_total);
  while ( time_total < Time_total )
    {
      iTime_total++;
      //      printf("iTime_total = %d\n",iTime_total);
      time_total = SRTM_time_total_minutes(iTime_total);
      //      printf("time_total = %g\n",time_total);
    }
  //  printf("done: itime [%g] = %d\n",Time_total,iTime_total-SRTM.run_istart[iRun]);
  return(iTime_total-SRTM.run_istart[iRun]);
}

int SRTM_get_itime_from_time( float Time_total )
{
  int iTime_total, iTime_run, irun, jrun;

  irun = SRTM_get_run_number_from_time(Time_total);
  for (jrun=0, iTime_total=0; jrun<irun; jrun++)
    iTime_total += SRTM.nt[jrun];
  iTime_run = SRTM_get_itime_in_run_from_time(Time_total);
  iTime_total += iTime_run;
  return(iTime_total);
}

float SRTM_get_parameter_value( int iParameter, int iTime_total )
// dCt/dt = R1 * dCr/dt    +    k2       * Cr    -    k2a * Ct
// dCt/dt = R1 * dCr/dt    +    R1/tau2' * Cr    -    R1/tau2'/(1+BP) * Ct
//
// iParameter: 0=R1        ,    1=tau2',-1=k2         2=BP,-2=k2a
{
  int itype, ishape, icolumn, irun, iTime_run;
  int jcoeff, jperiod;
  float Time_total, tau, multiplier, value, time, time0, time_stop, gmax, return_value, r1_value, tau2_value;

  irun      = SRTM_get_run_number(iTime_total);
  iTime_run = SRTM_get_itime_in_run(iTime_total);
  Time_total = SRTM_time_total_minutes(iTime_total);

  for ( jcoeff = 0, value=0.; jcoeff<SRTM.ncoeff; jcoeff++ )
    {
      itype   = SRTM.event_type[jcoeff];
      ishape  = SRTM.event_shape[jcoeff];
      icolumn = SRTM.icolumn[jcoeff];  // table events only
      tau     = SRTM.tau[jcoeff];      // gamma & sigmoid events only
      if ( abs(iParameter) == itype )
	{
	  for (jperiod=0; jperiod<SRTM.nperiods[jcoeff]; jperiod++)
	    {
	      time0 = SRTM.period_start[jcoeff][jperiod];
	      if ( Time_total >= time0 && Time_total <= SRTM.period_stop[jcoeff][jperiod] )
		{ // parameter of correct type exists at this time point
		  if ( ishape == SHAPE_CONSTANT )
		    multiplier = 1.;
		  else if ( ishape == SHAPE_GAMMA )
		    {
		      time  = Time_total;
		      multiplier = (time-time0)/tau * exp((double)(-(time-time0)/tau)) / exp(-1.);
		    }
		  else if ( ishape == SHAPE_GAMMAINT )
		    {
		      time  = Time_total;
		      time_stop = SRTM.period_stop[jcoeff][jperiod];
		      multiplier = (time0+tau) * exp(-(double)(time0/tau)) - (time+tau) * exp(-(double)(time/tau));
		      gmax       = (time0+tau) * exp(-(double)(time0/tau)) - (time_stop+tau) * exp(-(double)(time_stop/tau));
		      multiplier /= gmax;
		    }
		  else if ( ishape == SHAPE_SIGMOID )
		    {
		      time  = Time_total;
		      multiplier = (time-time0)/tau / sqrt((double)(1+SQR((time-time0)/tau)));
		    }
		  else if ( ishape == SHAPE_TABLE )
		    multiplier = SRTM.table_value[irun][icolumn][iTime_run];
		  value += SRTM.Beta_c[jcoeff] * multiplier;
		}
	    }
	}
    }
  if ( iParameter == TYPE_R1 || iParameter == -TYPE_k2 || iParameter == -TYPE_k2a )
    return_value = value;
  else if ( iParameter == TYPE_k2 )
    {
      if ( SRTM.k2prime[irun] != 0. )
	return_value = 1./ SRTM.k2prime[irun];
      else
	{
	  r1_value = SRTM_get_parameter_value( TYPE_R1, iTime_total );
	  if ( value == 0. )
	    {
	      printf("Error 1: parameter value k2 = 0 in SRTM_get_parameter_value()\n");
	      exit(1);
	    }
	  return_value = r1_value / value;  // tau2' = R1 / (R1*k2')
	}
    }
  else if ( iParameter == TYPE_k2a )
    { // value = R1/tau2' / (1 + B) => B = R1/tau2'/value - 1.
      r1_value   = SRTM_get_parameter_value( TYPE_R1, iTime_total );
      tau2_value = SRTM_get_parameter_value( TYPE_k2, iTime_total );
      if ( tau2_value == 0. )
	{
	  printf("Error 2: parameter value tau2' = 0 in SRTM_get_parameter_value()\n");
	  exit(1);
	}
      if ( value == 0. ) // k2a can be zero if all data points are zero
	return_value = 0.;
      else
	return_value = r1_value / tau2_value / value - 1.;
    }
  return(return_value);
}

float SRTM_get_parameter_value_from_time( int iParameter, float Time_total )
{
  int iTime_total;
  float return_value;

  iTime_total = SRTM_get_itime_from_time(Time_total);
  return_value=SRTM_get_parameter_value( iParameter, iTime_total );
  return(return_value);
}

int SRTM_get_target_region( int ntime, float *target_region )
{
  int j;

  if ( ntime != SRTM.ntime )
    {
      printf("Error: the specified # of time points (%d) does not match the GLM (%d).\n",
	     ntime,SRTM.ntime);
      return(1);
    }
  else
    {
      for (j=0; j<SRTM.ntime; j++)
	target_region[j] = SRTM.target_t[j];
      return(0);
    }
}

float SRTM_get_maximal_effect_size(int lCondition )
{
  int jt, jc, jcoeff;
  float *CBeta, y, max;

  CBeta = float_vector(SRTM.ncontrasts[lCondition]);
  // Loop over time points, and find the maximal value for given condition.
  for (jt=0, max=0.; jt<SRTM.ntime; jt++)
    {
      // Calculate the product CBeta for the given condition.
      for (jc=0; jc<SRTM.ncontrasts[lCondition]; jc++)
	{
	  // Find C*Beta
	  CBeta[jc] = 0;
	  for (jcoeff=0; jcoeff<SRTM.ncoeff; jcoeff++)
	    CBeta[jc] += SRTM.contrast_matrix[lCondition][jc][jcoeff] * SRTM.Beta_c[jcoeff];
	}
      // Now find the maximum value of y = X * C*Beta
      for (jc=0; jc<SRTM.ncontrasts[lCondition]; jc++)
	{
	  y = SRTM.X_tc[jt][jc] * CBeta[jc];
	  if ( fabs((double)y) > fabs((double)max) ) max = y;
	}
    }
  free_float_vector(CBeta);
  return(max);
}

float SRTM_time_total_minutes( int itime_total )
{ // compute SRTM time
  float time, dt;
  int jrun, jt, it;

  time = 0;
  if ( SRTM.nruns == 0 )
    { // This could happen when reading the GLM file for the 1st run
      for (jt=0, it=0; jt<SRTM.table_nt[0]; jt++, it++)
	{
	  dt = SRTM.table_value[0][0][jt];
	  time += dt;
	  if ( it == itime_total )
	    {
	      return(time-dt/2.);  // time centered on bins
	    }
	}
      printf("Programming error: time point (%d) for 1st run not found in SRTM_time_total_minutes()\n",itime_total);
      exit(1);
    }
  else
    {
      for (jrun=0, it=0; jrun<SRTM.nruns; jrun++)
	{
	  for (jt=0; jt<SRTM.table_nt[jrun]; jt++, it++)
	    {
	      dt = SRTM.table_value[jrun][0][jt];
	      time += dt;
	      if ( it == itime_total )
		return(time-dt/2.);  // time centered on bins
	    }
	}
      if ( itime_total < 0 ) // could be anatomical using "-e' option
	return(0.);
      else
	{
	  printf("nruns = %d, it = %d\n",SRTM.nruns,it);
	  printf("Programming error: time point (%d) not found in SRTM_time_total_minutes()\n",itime_total);
	  exit(1);
	}
    }
}

float SRTM_time_run_minutes( int itime_total )
{ // compute SRTM time
  float time_run;
  int jrun, irun;

  for (jrun=0, irun=-1; jrun<SRTM.nruns; jrun++)
    {
      if ( itime_total >= SRTM.run_istart[jrun] && itime_total <= SRTM.run_istop[jrun] )
	irun = jrun;
    }
  if ( irun == -1 )
    return(0.);
  time_run = SRTM_time_total_minutes(itime_total);
  if ( irun > 0 )
    time_run -= SRTM_time_total_minutes(SRTM.run_istop[irun-1]);
  return(time_run);
}

void SRTM_get_start_stop_times( int iRun, int *istart, int *istop)
{
  if ( iRun >= SRTM.nruns )
    {
      printf("Programming error: run %d does not exist\n",iRun);
      exit(1);
    }
  *istart = SRTM.run_istart[iRun];
  *istop  = SRTM.run_istop[iRun];
}

int SRTM_get_DOF()
{
  int DOF;

  DOF = SRTM.ntime - SRTM.ncoeff;
  return(DOF);
}

int SRTM_EventNumberFromChar(char event_char)
     // character (in)  event integer (out)  ascii
     // -------------   -------------------  -----
     //      1-9              1-9            49-57
     //      A-Z             36-61           65-90
     //      a-z             10-35           97-122
     //
     // error returns 0
{
  int event_id;

  event_id = event_char;  // convert character to integer ascii code
  if ( (event_id<49 && event_id!=44) ||
       (event_id>57 && event_id<65)  ||
       (event_id>90 && event_id<97)  ||
       (event_id>122) )
    return(0);

  if ( event_id == 44 )   // ',' is special case
    event_id = -1;
  else if ( event_id >= 49 && event_id <= 57 )      // 1-9 characters
    event_id -= 48;  // numbers 1-9   (subtract 49, add 1)
  else if ( event_id >= 97 && event_id <= 122 ) // a-z characters
    event_id -= 87;  // numbers 10-35 (subtract 97, add 10)
  else if ( event_id >= 65 && event_id <= 90 )  // A-Z characters
    event_id -= 29;  // numbers 10-35 (subtract 65, add 36)
  return(event_id);
}

int SRTM_EventCharFromInteger(int event_id, char *event_char)
{
  int ascii_code;

  if ( event_id>=1 && event_id<=9 )
    ascii_code = event_id + 48;  // add 49-1
  else if ( event_id>=10 && event_id<=35 )
    ascii_code = event_id + 87;  // add 97-10
  else if ( event_id>=36 && event_id<=61 )
    ascii_code = event_id + 29;  // add 65-36
  else
    return(1);
  *event_char = ascii_code;
  return(0);
}

char SRTM_EventChar(int event_id)
{
  char event_char;

  if ( SRTM_EventCharFromInteger(event_id, &event_char) )
    {
      printf("Programming error: event number %d apparently is not defined!\n",event_id);
      exit(1);
    }
  return event_char;
}

int SRTM_EventStringFromInteger(int event_id, char *event_string)
{
  char event_char, event_string_temp[LEN_STR];
  if ( SRTM_EventCharFromInteger(event_id,&event_char) )
    {
      printf("Error: event %d is not defined.\n",event_id);
      return(1);
    }
  strcpy(event_string_temp," ");
  event_string_temp[0] = event_char;
  strcpy(event_string,event_string_temp);
  return(0);
}

int SRTM_set_inclusion_cutoff(int irun, int icutoff)
{
  int istart_run, nincluded;
  int jrun, jt;
  
  if ( irun < 0 || irun >= SRTM.nruns )
    return(1);
  else if ( icutoff < 0 || icutoff > SRTM.nt[irun] )
    return(1);
  else
    {
      // Assign the new cutoff.
      SRTM.inclusion_cutoff[irun] = icutoff;
      // Update the include_time vector;
      for (jrun=0, nincluded=0; jrun<SRTM.nruns; jrun++)
	{
	  istart_run  = SRTM.run_istart[jrun];
	  for (jt=0; jt<SRTM.nt[jrun]; jt++)
	    {
	      if ( jt < SRTM.inclusion_cutoff[jrun] )
		{
		  SRTM.include_time[jt+istart_run] = 1;
		  nincluded++;
		}
	      else
		SRTM.include_time[jt+istart_run] = 0;
	    }
	}
      // Update the basis functions.
      SRTM_define_static_basis_functions();
    }
  return(0);
}

BOOL SRTM_k2_condition( int iCondition )
{
  int nk2, jcoeff;

  if ( SRTM.ncontrasts[iCondition] != 1 )  // don't allow F tests
    return(FALSE);
  else
    {
      // Make sure every parameter in the condition is TYPE_k2.
      for (jcoeff=0, nk2=0; jcoeff<SRTM.ncoeff; jcoeff++)
	{
	  if ( SRTM.contrast_matrix[iCondition][0][jcoeff] != 0 )
	    { // coeff is in contrast, so test type and presence of k2
	      if ( SRTM.event_type[jcoeff] != TYPE_k2 )
		return(FALSE);
	      else
		nk2++;
	    }
	} // coeff
      if ( nk2 != 1 )
      	return(FALSE);
      else
	return(TRUE);
    }
}

int SRTM_lSingleCondition( int iCoeff )
// See if the coefficient "iCoeff" is included as a single test condition.
// If so, an error bar will be attached to it.
{
  int lCoeff, nCoeff;
  int jcond, jcoeff;

  for (jcond=0; jcond<SRTM.nconditions; jcond++)
    {
      if ( SRTM.ncontrasts[jcond] == 1 )
	{
	  for (jcoeff=0, nCoeff=0; jcoeff<SRTM.ncoeff; jcoeff++)
	    {
	      if ( SRTM.contrast_matrix[jcond][0][jcoeff] != 0 )
		{
		  nCoeff++;
		  lCoeff = jcoeff;
		}
	    }
	  if ( nCoeff == 1 && lCoeff == iCoeff )
	    return(jcond);
	}
    }
  return(-1);
}
