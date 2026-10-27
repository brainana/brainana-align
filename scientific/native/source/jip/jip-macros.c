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
#include "jip.h"
#include "glm-prototypes.h"

char loopfile_name[LEN_STR], loopstack_name[LEN_STR], looplist_name[LEN_STR];
int number_time_points;JIP_INDEX number_bytes_per_point;
struct loop_commands
{
  char command[LEN_STR];
  struct loop_commands *next;
};
struct loop_commands *loop_root;
  

extern char error_string[LEN_STR];


int initialize_read_loop( int file_type, char *file_name, char *stack_name, char *list_name )
{
  struct loop_commands *loop_member, *free_me;
  struct XD_HEADER header;
  IPOINT3D dim;

  /*
  ** If there is an existing set of loop commands, get rid of these.
  */
  loop_member = loop_root;
  while ( loop_member )
    {
      free_me = loop_member;
      loop_member = loop_member->next;
      free( (char *)free_me );
    }
  loop_root = NULL;

  /*
  ** Figure out how many total volumes (or time points) are in the file.
  */
  /* Read the dimensions from the xdisplay header file. */
  if ( read_volume_header(TRUE, file_name, &header) ) return(1);
  dim = header.dim;

  number_time_points     = header.nt;
  number_bytes_per_point = ((JIP_INDEX)(dim.x)) * dim.y * dim.z * 8;  // complex bfloats = 8 bytes each 

  strcpy(loopfile_name,file_name);
  strcpy(loopstack_name,stack_name);
  strcpy(looplist_name,list_name);

  return(0);
}

int store_read_loop_command( char *command_line )
{
  struct loop_commands *loop_member, *last_loop_member;
  int ncommands;

  ncommands = 1;
  loop_member = loop_root;
  while ( loop_member )
    {
      last_loop_member = loop_member;
      loop_member = loop_member->next;
      ncommands++;
    }
  /*
  ** Allocate space for a new member, and put it into the list.
  */
  if ( (loop_member=(struct loop_commands *)malloc(sizeof(struct loop_commands))) )
    {
      strcpy( loop_member->command, command_line );
      loop_member->next = NULL;
      if ( ncommands == 1 )
	loop_root = loop_member;
      else
	last_loop_member->next = loop_member;
    }
  else
    {
      printf("Error allocating command space in `store_read_loop_command'.\n");
      exit(1);
    }
  return(0);
}

int flush_read_loop_commands()
{
  char command[LEN_STR];
  struct loop_commands *loop_member;
  int number_points_per_iteration, ndone;
  int nset, set[MAX_NSET];
  float Mbytes;

  /*
  ** Determine the set of volumes based upon the list passed by the read-loop command.
  ** Note that this can differ from the number actually stored in the file.
  */
  if ( decode_subset(looplist_name,number_time_points,&nset,set) )
    return(1);
  if ( nset < 1 )
    {
      printf("Programming error in routine flush_read_loop_commands.\n");
      printf("# points = %d, list name = %s, nset = %d\n",number_time_points,looplist_name,nset);
      exit(1);
    }

  /*
  ** Determine the number of iterations based upon the number of volumes
  ** and memory demands.  Shoot for 10 megabytes per read, but read data
  ** in multiples of 20 time points.
  */
  Mbytes = (float)number_bytes_per_point/1.e6;
  number_points_per_iteration = 20 * ceil( (double)(10./Mbytes/20.));
  /*                                                ^ number of megabytes */
  /*
  ** Begin the loop.
  */
  ndone = 0;
  while ( ndone < nset )
    {
      create_read_command(nset, set, number_points_per_iteration, &ndone, command);
      if ( do_command(1,command) ) return(1);
      for (loop_member=loop_root; loop_member; loop_member=loop_member->next)
	if ( do_command(1,loop_member->command) ) return(1);
    }

  return(0);
}

int create_read_command(int nset, int *set, int number_points_per_interation, int *ndone, char *command)
{
  char listname[LEN_STR], dummy[LEN_STR];
  int contiguous;JIP_INDEX index_start, index_end, j;

  index_start = *ndone;
  index_end = *ndone+number_points_per_interation;
  if ( index_end > nset ) index_end = nset;
  
  contiguous = TRUE;
  for (j=index_start+1; j<index_end; j++)
    contiguous = contiguous && set[j] == set[j-1]+1;
  if ( contiguous )
    sprintf(listname,"%d-%d",set[index_start],set[index_end-1]);
  else
    {
      sprintf(listname,"%d",set[index_start]);
      for (j=index_start+1; j<index_end; j++)
	{
	  sprintf(dummy,",%d",set[j]);
	  strcat(listname,dummy);
	}
    }
  *ndone = jip_int_count(index_end);
  /*
  ** Assemble the full command line.
  */
  strcpy(command,"read ");
  strcat(command,loopfile_name);
  strcat(command,">");
  strcat(command,listname);
  strcat(command," ");
  strcat(command,loopstack_name);
  strcat(command,"\n");
  return(0);
}

int macro_reorder_interleaved_slices( int istart )
{
  struct data_stack *stack, *stack_save;
  struct volume_list  *volume;
  int jslice;
  char string1[LEN_STR], string2[LEN_STR], command[LEN_STR];
  extern struct volume_list *volume_list_root;

  // 'istart' must be 0 or 1
  if ( istart != 0 && istart != 1 )
    {
      printf("Error: `reorder-interleaved-slices [start] [volume list]\n");
      printf("where istart (%d) must be either 0 or 1.\n",istart);
      return(1);
    }

  // The following method allows multiply stacks to be reversed.
  // One could in principle set a different volume order for each stack,
  // since each stack can have a different number of slices.  The problem
  // is then that the reorder-slices command will overwrite the current
  // volume list, and only the 1st stack will be reordered.  Therefore, this
  // is done by assuming that each stack has the same number of slices,
  // and function "reorder-slices" will return an error if this is not true.

  // Set the volume order based upon the 1st stack.
  stack = volume_list_root->stack;
  // Create a string that contains the volume order, then use the commands
  // 1) set slice-order [string]
  // 2) reorder-slices [stack name]
  sprintf(string2," ");
  for ( jslice=istart; jslice<stack->dim.z; jslice+=2 )
    {
      sprintf(string1,"%d ",jslice);
      strcat(string2,string1);
    }
  for ( jslice=!istart; jslice<stack->dim.z; jslice+=2 )
    {
      sprintf(string1,"%d ",jslice);
      strcat(string2,string1);
    }
  sprintf(command,"set slice-order %s\n",string2);
  if ( do_command(1,command) ) return(1);

  stack_save = 0;
  sprintf(string1," ");
  for (volume=volume_list_root; volume; volume=volume->next)
    {
      if ( (stack=volume->stack) == stack_save ) continue;
      stack_save = stack;
      strcat(string1," ");
      strcat(string1,stack->name);
    }
  sprintf(command,"reorder-slices %s\n",string1);
  if ( do_command(1,command) ) return(1);
  return(0);
}

int macro_read_glm_control_file( char * argument )
{
  char file_name[LEN_STR], command[LEN_STR], string[LEN_STR];
  int jrun;

  // The 1st (and only) argument should be the GLM control file name.
  if ( sscanf(argument,"%s",file_name) != 1 )
    {
      jip_text(2,"The argument to the read-glm command should be a file name.\n");
      return(1);
    }

  // Read the GLM control file.
  if ( GLM_read_control_file(file_name,0) )
    {
      sprintf(error_string,"Error reading the GLM control file %s\n",file_name);
      jip_text(2,error_string);
      return(1);
    }

  // Read each file in the GLM.
  for (jrun=0; jrun<GLM_input.nruns; jrun++)
    {
      // Read each run in the GLM control file.
      sprintf(command,"read %s glm_run%d\n",GLM_input.run_name[jrun],jrun);
      if ( do_command(1,command) ) return(1);
      if ( jrun == 0 )
	sprintf(string,"glm_run0");
      else
	{
	  sprintf(command," glm_run%d",jrun);
	  strcat(string,command);         // concatenate names of all runs
	}
    }
  // Copy all of the runs into a single stack.
  sprintf(command,"copy glm %s\n",string);
  if ( do_command(1,command) ) return(1);
  // Delete the individual runs.
  sprintf(command,"delete %s\n",string);
  if ( do_command(1,command) ) return(1);

  // Define the GLM basis functions.
  if ( GLM_get_number_events() )
    {
      // Define the basis functions.
      GLM_define_basis_functions();
      // Output info about the basis functions.
      if ( GLM_input.notify ) GLM_describe_basis_functions();
      return(0);
    }
  else
    {
      // Define the basis functions anyway (maybe just a drift correction).
      //      GLM_define_basis_functions();
      jip_text(1,"No events have been defined!\n");
      return(1);
    }
}
