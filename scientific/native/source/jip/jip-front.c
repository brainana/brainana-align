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
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include "util.h"
#include "jip.h"
#include "../jip-align/common-align.h"
#include "glm-prototypes.h"

static char CNULL = '\0';

#define MAX_COMMANDS 150    // # of allowed commands; currently 128
struct COMMAND_LIST
{
  char command[LEN_STR];
};

//
// Define all the commands (in lower case).  For each entry, also make an
// entry in the enumerate block below.
//
struct COMMAND_LIST command_list[] = {

  // Alignment:
  "mirror-left-right",      // mirror-left-right <volume list>
  "mirror-up-down",         // mirror-up-down <volume list>
  "register",               // forward transformation using registration parameters
  "inverse-register",       // inverse transformation using registration parameters
  "register-histology",     // corresponding command for histological slices
  "down-sample",            // down-sample <stack list>
  "shift-pixels",           // shift-pixels [#x pixels] [#y pixels] [#z pixels] <stack list>
  // Arithmetic functions:
  "abs",                    // abs                <volume list> 
  "add",                    // add      [image 1] <volume list> 
  "add-constant",           // add-constant      [constant] <volume list> 
  "add-stacks",             // add-stacks      [stack 1] <stack list> 
  "create-fCBV",            // create-fCBV [sname] [pre volume] [post volume] <volume list>
  "create-fCBV1",           // create-fCBV [sname] [pre volume] [post volume] [base volume] <volume list>
  "divide",                 // divide   [volume 1] <volume list> 
  "divide-constant",        // divide-constant   [constant] <volume list> 
  "divide-stacks",          // divide-stacks   [stack 1] <stack list> 
  "exp",                    // exp               [constant] <volume list> 
  "ln",                     // ln                [constant] <volume list> 
  "atan",                   // atan  <volume list> 
  "multiply",               // multiply [volume 1] <volume list> 
  "multiply-constant",      // multiply-constant [constant] <volume list> 
  "multiply-stacks",        // multiply-stacks [stack 1] <stack list> 
  "subtract",               // subtract [volume 1] <volume list> 
  "subtract-constant",      // subtract-constant [constant] <volume list> 
  "subtract-stacks",        // subtract-stacks [stack 1] <stack list> 

  // Averaging:
  "average",                // average [new stack] <volume list> 
  "average-by",             // average-by [# volumes] [new stack] <vlist> 
  "average-epochs",         // average-epochs [# volumes] [new stack] <vlist> 
  "average-stacks",         // average-stacks [new stack] <stack list> 
  "smooth-3d",              // smooth-3d [fwhm in mm] <stack list> 
  "smooth-2d",              // smooth-2d [fwhm in mm] <stack list>
  "flatten",                // flatten [fov fraction] <stack list>
  "weighted-average",       // weighted-average [new stack name] <vlist> 
  "create-variances",       // create a variance map for each volume in a series

  // Input/output:
  "read",                   // read [file name] <stack name> 
  "read-loop",              // read-loop [file name] <stack name> 
  "end-loop",               // end-loop 
  "mode",                   // mode [file name] 
  "write",                  // write [file name with extension] <volume list> 
  "write-short",            // write-short [file.nii] <volume list> 
  "write-float",            // write-float [file.nii] <volume list> 
  "write-slices",           // write slices to put slices into separate files
  "write-overlay",          // write an overlay file
  "delete-overlay",         // delete overlay from memory
  "create-bitmap-from-overlay", // create-bitmap-from-overlay [stack name] <volume_list>
  "create-overlay-from-bitmap", // create-overlay-from-bitmap <volume_list>
  "mirror-overlay-x",       // mirror-overlay-x [x dimension]
  // Magnitude corrections:
  "baseline-correct",       // baseline-correct <volume list> 
  "baseline-correct-volumes",// baseline-correct-volumes <stack list> 
  "magnitude-window",       // magnitude-window <volume list> 
  "magnitude-window-together",// magnitude-window-together <volume list> 
  "scale",                  // scale [target max.] <volume list> 
  "normalize",              // normalize [target max.] <volume list> 

  // Program control and monitoring:
  "bye","quit","stop","exit",// terminate the program 
  "help",                   // help 
  "set",                    // set [variable name] [value] 
  "show",                   // show <variable> 
  "show-stacks",            // show-stacks <stack list> 
  // Reordering of data:
  "convert-complex",        // convert-complex <stack list> 
  "convert-to-real",        // convert-to-real <stack list> 
  "copy",                   // copy [new stack] <volume list> 
  "delete",                 // delete <stack list> 
  "clip-x",                 // clip-x <stack list> 
  "clip-y",                 // clip-y <stack list> 
  "clip-z",                 // clip-z <stack list> 
  "clip-x-to",              // clip-x-to [int] <stack list> 
  "clip-y-to",              // clip-y-to [int] <stack list> 
  "clip-z-to",              // clip-z-to [int] <stack list> 
  "pad-x",                  // pad-x  <stack list> 
  "pad-y",                  // pad-y <stack list> 
  "pad-z",                  // pad-z <stack list> 
  "pad-xy",                 // pad-xy <stack list> 
  "pad-to",                 // pad-to <stack list> 
  "pad-x-to",               // pad-x-to [int] <stack list> 
  "pad-y-to",               // pad-x-to [int] <stack list> 
  "pad-xyz",                // pad-xyz <stack list> 
  "pad-2^n",                // pad-2^n <stack list> 
  "reorder-slices",         // reorder-slices <stack list> 
  "shuffle",                // shuffle [new stack] <stack list> 
  "slice",                  // slice [int] [new stack] <volume list> 
  "reverse-slice-order",    // reverse-slice-order <stack list> 
  "reorder-time&slice",     // reorder-time&slice <stack list> 

  // Transformations and filters:
  "inverse-transform",      // inverse-transform   <volume list> 
  "inverse-transform-x",    // inverse-transform-x <volume list> 
  "inverse-transform-y",    // inverse-transform-y <volume list> 
  "inverse-transform-xyz",  // inverse-transform-xyz <volume list> 
  "transform",              // transform   <volume list> 
  "transform-x",            // transform-x <volume list> 
  "transform-y",            // transform-y <volume list> 
  "transform-xyz",          // transform-xyz <volume list> 
  "transform-time",         // transform-time <volume list> 
  "hamming-window",         // hamming-window <volume list> 
  "hamming-window-xyz",     // hamming-window-xyz <volume list> 
  "exp-filter-1d",          // exp-filter-1d [tau] <volume list> 
  "edge-filter",            // edge-filter <volume list>
  "analyze-lectin",         // analyze-lectin <volume list>

  // maps
  "map-snr",                // map-snr [stack name] <volume list>
  "map-t2",                 // map-t2  [stack name] <volume list>
  "map-diffusion",          // map-diffusion [stack name] <volume list>
  "map-t1-from-flip-angles",// map-t1-from-flip-angles [tr in ms (int)] [stack name] <volume list>

  // GLM
  "read-GLM",               // read-glm [glm control file]
  "GLM-remove-baseline",    // GLM-remove-baseline
  "GLM-correct-baseline",   // GLM-correct-baseline
  "GLM-histogram",          // GLM-histogram
  "GLM-residue",            // GLM-residue [residue_string] [stack name]
  "GLM-fit-events",         // GLM-fit-events [event] [stack name]
  "GLM-find-maximal-effect",// GLM-find-maximal-effect [condition] [stack name]
  "GLM-residual-variance",  // GLM-residual-variance [residue] [new stack name] [stack name]
  "GLM-map-AR",             // GLM-map-AR [new stack name] [stack name]
  "GLM-vary-tau",           // GLM-vary-tau [event number] [new stack name]
  "GLM-vary-taus",          // GLM-vary-tau [# time constants] [new stack name] [F volume] [glm]

  // Stats
  "histogram",              // histogram <volume list>
  "weighted-histogram",     // weighted-histogram <volume list>

  // miscellaneous
  "scatter-plot",           // scatter-plot [file name] [image 1] <image 2>
  "calculate-overlay-overlap", // calculate_overlay_overlap [smoothing fwhm] [volume1 volume2]
  "\0" };

enum
{
  // Alignment:
  MIRROR_LEFT_RIGHT, MIRROR_UP_DOWN, REGISTER, INVERSE_REGISTER, REGISTER_HIST,
  DOWNSAMPLE, SHIFT_PIXELS,
  // Arithmetic functions:
  ABS, ADD, ADD_CONST, ADD_STACKS, CREATE_FCBV, CREATE_FCBV1,
  DIVIDE, DIVIDE_CONST, DIVIDE_STACKS, EXP, LN, ATAN, MULTIPLY, MULTIPLY_CONST,
  MULTIPLY_STACKS, SUBTRACT, SUBTRACT_CONST, SUBTRACT_STACKS,
  // Averaging
  AVERAGE, AVERAGE_BY, AVERAGE_EPOCHS, AVERAGE_STACKS, SMOOTH_3D, SMOOTH_2D, FLATTEN, AVERAGE_WEIGHTED,
  CREATE_VARS,
  // Input/output:
  READ, READ_LOOP, END_READ_LOOP, MODE, WRITE, WRITE_SHORT, WRITE_FLOAT, WRITE_SLICES, WRITE_OVL, DELETE_OVL,
  CREATE_BITMAP_FROM_OVL, CREATE_OVL_FROM_BITMAP, MIRROR_OVERLAY_X,
  // Magnitude corrections:
  BASELINE_CORRECT, BASELINE_CORRECT_VOLUMES, MAGNITUDE_WINDOW, MAGNITUDE_WINDOW_TOGETHER,
  SCALE, NORMALIZE,
  // Program control and monitoring:
  BYE, QUIT, STOP, EXIT, HELP, SET, SHOW, SHOW_STACKS,
  // Reordering of data:
  CONVERT_COMPLEX, CONVERT_TO_REAL, COPY, DELETE,
  CLIP_X, CLIP_Y, CLIP_Z, CLIP_X_TO, CLIP_Y_TO, CLIP_Z_TO,
  PAD_X, PAD_Y, PAD_Z, PAD_XY, PAD_TO, PAD_X_TO, PAD_Y_TO, PAD_XYZ, PAD_2N,
  REORDER_SLICES, SHUFFLE, SLICE, REVERSE_SLICE_ORDER, REORDER_TIME_SLICE,
  // Transformation and filters:
  INVERSE_TRANSFORM, INVERSE_TRANSFORM_X, INVERSE_TRANSFORM_Y, INVERSE_TRANSFORM_XYZ,
  TRANSFORM, TRANSFORM_X, TRANSFORM_Y, TRANSFORM_XYZ, TRANSFORM_TIME,
  HAMMING_WINDOW, HAMMING_WINDOW_XYZ, EXP_FILTER_1D, EDGE_FILTER, ANALYZE_LECTIN,
  // maps
  MAP_SNR, MAP_T2, MAP_DIFFUSION, MAP_T1_FLIP,
  // GLM
  READ_GLM, GLM_REMOVE_BASELINE, GLM_CORRECT_BASELINE, GLM_HISTOGRAM, GLM_RESIDUE, GLM_FIT_EVENTS,
  GLM_MAX_EFFECT, GLM_RESIDUAL_VARIANCE, GLM_MAP_AR, GLM_VARY_TAU, GLM_VARY_TAUS,
  // Stats
  FILL_HISTOGRAM, FILL_WEIGHTED_HISTOGRAM,
  // Miscellaneous
  SCATTER_PLOT, CALC_OVL_OVERLAP
} command_choices;

/*
** Define macros.  For each entry, include an entry in the
** enumerate block below.
*/
struct COMMAND_LIST macro_list[] =
{
  "reorder-interleaved-slices",/* reorder-interleaved-slices [start] [stack name] */
  "\0"
};
enum
{
  REORDER_INT_SLICES
} macro_choices;

/*
** Define a set of variables (in lower case) to be used in conjunction with
** the SET command. For each entry, also make an entry in the enumerate block
** below.
*/
struct COMMAND_LIST variable_list[] =
{
  "overlay-logic",
  "complex-output",
  "phase-output",
  "io-increment",
  "slice-order",
  "map-array",
  "filter-range",
  "nifti-method",
  "histogram",
  "interpolation-kernel",

  "registration-matrix",
  "target-matrix",
  "source-matrix",

  "registration-resolution",
  "target-resolution",
  "source-resolution",

  "registration-origin",
  "target-origin",
  "source-origin",

  "registration-direction",
  "target-direction",
  "source-direction",

  "registration-type",
  "registration-space",
  "registration-orientation",
  "registration-wrapping",
  "registration-translations",
  "registration-rotations",
  "registration-inflations",
  "registration-skews",
  "registration-distortions",
  "histology-slices",
  "histology-slice",

  "target-voxel-affine",
  "source-voxel-affine",
  "time-step",
  "\0"
};
enum
{
  OVERLAY_LOGIC, COMPLEX_OUTPUT, PHASE_OUTPUT, IO_INCREMENT, SLICE_ORDER,
  MAP_ARRAY, FILTER_RANGE, NIFTI_METHOD, HISTOGRAM_VARS, INTERP_KERNEL,

  REG_MATRIX, TARGET_MATRIX, SOURCE_MATRIX,
  REG_RESOLUTION, TARGET_RESOLUTION, SOURCE_RESOLUTION,
  REG_ORIGIN, TARGET_ORIGIN, SOURCE_ORIGIN,
  REG_DIRECTION, TARGET_DIRECTION, SOURCE_DIRECTION,

  REG_TYPE, REG_SPACE, REG_ORIENT, REG_WRAPPING,
  REG_TRANSLATIONS, REG_ROTATIONS, REG_INFLATIONS, REG_SKEWS, REG_DISTORT,
  HIST_SLICES, HIST_SLICE, TARGET_VOXEL_AFFINE, SOURCE_VOXEL_AFFINE, TIME_STEP
} variable_choices;

/*
** Define the roots of three linked lists:
** a) volume stacks stored in memory,
** b) lists of volumes generated from the argument of a user-input command,
** c) a 2nd volume list used for volume comparisons (contains 0 or 1 volumes).
** These are pointers to structures and are initialized to NULL.
*/
struct data_stack *stack_root           = NULL;
struct volume_list  *volume_list_root     = NULL;
struct volume_list  *comparison_list_root = NULL;

char error_string[LEN_STR]; /* string to place error text into for jip_text*/

/*
** The following global variables are controlled with the "set" command.
*/
FILE *mode_file1, *mode_file2;
int flags;                       /* each bit represents a different flag */
                                 /* (see volume.h) */
int macro_running=0;             /* 1 when a macro is running, 0 otherwise */
int read_loop_running=0;         /* 1 when collecting commands inside a read-loop, 0 otherwise */
int fatal_error=0;               /* Controls whether program exits on error. */
int io_increment=0;              /* # to add to read or write list: "bshort>10-19" -> bshort>20-29" when inc=10 */
int nslice_order=0;              /* # of elements in array `slice_order' */
int *slice_order;                /* array of values for reordering volumes */
int nmap_array=0;                /* # of elements in array `map_array' */
float *map_array;                /* array of values to apply to volume maps */
// Filter variables
POINT3D filter_range;
// Overlay
OVERLAY overlay;
/* Replacement variables for JIP_COM files that function like argc and argv. */
int com_argc;
char *com_argv[10];
char argv0[LEN_STR], argv1[LEN_STR], argv2[LEN_STR], argv3[LEN_STR], argv4[LEN_STR];
char argv5[LEN_STR], argv6[LEN_STR], argv7[LEN_STR], argv8[LEN_STR], argv9[LEN_STR];
// histogram
float hist_start, hist_end, hist_bin_size;
int hist_nbins;JIP_INDEX *histogram;
// nifti method
int nifti_method=3;
// time step between volumes
float time_step=0.;


int main( int argc, char *argv[])
{
  char command_line[LEN_STR];
  int program_entry;

  /*
  ** Initialize global variables.
  */
  initialize_global_variables();

  /*
  ** The first argument on the command line should be a mode file name.
  */
  if ( argc >= 2 )
    {
      if ( !(mode_file1 = fopen(argv[1],"r")) )
	{
	  sprintf(error_string,"I cannot open the jip command file %s.",argv[1]);
	  jip_text(3,error_string);
	}
      else
	{
	  flags = flags | MODE_BIT1;
	  program_entry = 1;
	}
      fatal_error = 1;
    }
  /*
  ** Begin an endless loop interpretting commands and acting upon these.
  ** The loop is terminated by a 'quit', 'stop', or 'exit' command.
  */
  while (1)
    {
      if ( !(flags&MODE_BIT1 || flags&MODE_BIT2) )
	{
	  program_entry = 0;
	  /* Read the command from standard input. */
	  do fprintf(stdout,"jip > ");             /* prompt for command */
	  while ( !get_line_with_comment(stdin,command_line,LEN_STR) );        /* read the command   */
	}
      else
	{
	  if ( flags&MODE_BIT2 )
	    {
	      /* Read the command from a file. */
	      if ( !get_line_with_comment(mode_file2,command_line,LEN_STR) )
		{
		  fclose( mode_file2 );
		  flags = flags & ~MODE_BIT2;
		  continue;
		}
	      /*
	      ** Replace any passed arguments in the command line.
	      */
	      if ( replace_passed_arguments(command_line,com_argc,com_argv) ) continue;
	    }
	  else if ( flags&MODE_BIT1 )
	    {
	      /* Read the command from a file. */
	      if ( !get_line_with_comment(mode_file1,command_line,LEN_STR) )
		{
		  fclose( mode_file1 );
		  flags = flags & ~MODE_BIT1;
		  continue;
		}
	      /*
	      ** Replace any passed arguments in the command line.
	      */
	      if ( program_entry )
		{
		  if ( replace_passed_arguments(command_line,argc,argv) )
		    {
		      program_entry = 0;
		      continue;
		    }
		}
	      else
		if ( replace_passed_arguments(command_line,com_argc,com_argv) ) continue;
	    }
	  /* Remove the trailing carriage return. */
	  /* *(command_line+jip_int_count((JIP_INDEX)strlen(command_line))-1)='\0'; */
	  /* Echo the command with the prompt. */
	  fprintf(stdout,"jip > %s",command_line);
	}
      
      do_command( 0,command_line );
    }
}

int do_command(int imacro, char *command_line)
{
  char command[LEN_STR], variable[LEN_STR];
  char argument[LEN_STR], *next_non_white();
  int icommand, ivariable, nerrors = 0;
  int interpret_command();

  /*
  ** This command may be called either by user input
  ** or by c function "macros" which exploit other commands.
  */
  if ( imacro == 1 )
    {
      fprintf(stdout,"macro>> %s",command_line);
      macro_running = 1;
    }
  else
    macro_running = 0;

  /* Extract the command. */
  if ( sscanf(command_line,"%s",command) < 0 ) return(0);

  /*
  ** Allow comments to be included after the following characters:
  **                      '('  or '#'
  */
  if ( command[0]=='(' || command[0]=='#' ) return(0);

  /*
  ** The argument of the command is the rest of the command line after
  ** the first contiguous piece.
  */
  /* argument = next_non_white( command_line+jip_int_count((JIP_INDEX)strlen(command)) ); */
  strcpy(argument,next_non_white(command_line+jip_int_count((JIP_INDEX)strlen(command))));

  /*
  ** Interpet the command.  Commands must be entered exactly.  The
  ** position of the command in the structure array is returned.
  */
  icommand = interpret_command( 0, command );

  if ( read_loop_running && icommand != END_READ_LOOP && icommand != MODE )
    {
      store_read_loop_command(command_line);
      return(0);
    }
  /*
  ** If the command is
  ** `set' or `show': call the appropriate set/show routine,
  ** else: call the action routine for all other commands.
  */
  if ( icommand == SET || icommand == SHOW )
    {
      /* Pick off the variable name (if one is given). */
      variable[0] = CNULL;
      /* if ( *argument != NULL ) sscanf(argument,"%s",variable); */
      if ( argument[0] != CNULL ) sscanf(argument,"%s",variable);
      
      /* The value of the variable is the rest of the argument. */
      /* argument = next_non_white( argument + jip_int_count((JIP_INDEX)strlen(variable)) );*/
      {char *rest=next_non_white(argument+jip_int_count((JIP_INDEX)strlen(variable)));memmove(argument,rest,jip_int_count((JIP_INDEX)strlen(rest))+1);}
      
      /* Null the carriage return for variable names read from files. */
      if ( icommand == SET && (flags&MODE_BIT1||flags&MODE_BIT2) )
	argument[jip_int_count((JIP_INDEX)strlen(argument))-1] = CNULL;
      
      /* Interpet the variable name, which must be entered exactly. */
      ivariable = interpret_command( 1, variable );
      
      /* Finally, set the variable to its input value. */
      if ( icommand == SET )
	{
	  if ( variable[0] != CNULL )
	    set_variable( ivariable, argument );
	  else
	    jip_text(2,"Syntax = `set [variable name] [value]'.");
	}
      else
	show_variable( ivariable );
    }
  else
    {
      /*
      ** Act on the command.  The command number is passed together with
      ** the argument of the command.
      */
      if ( icommand < MAX_COMMANDS )
	{
	  if ( act_on_command(icommand, argument) )
	    nerrors++;
	}
      else
	{
	  if ( act_on_macro(icommand-MAX_COMMANDS, argument) )
	    nerrors++;
	}
      /*
      ** Prevent a runaway process, which could occur if a bad file is
      ** passed to this program.
      */
      if ( nerrors>0 && (flags&MODE_BIT1||flags&MODE_BIT2) ) exit(1);
    }
  return(0);
}

int strip_com_arguments(char *string)
{
  char *cptr, string1[LEN_STR], file_name[LEN_STR];
  char *next_non_white();

  strcpy(string1,string);
  com_argc = 2;
  strcpy(argv0,"read");
  com_argv[0] = argv0;

  cptr = string1;
  sscanf(cptr,"%s",file_name);
  cptr = next_non_white(cptr+jip_int_count((JIP_INDEX)strlen(file_name)));
  strcpy(argv1,file_name);
  com_argv[1] = argv1;

  if ( sscanf(cptr,"%s",argv2) == 0 ) return(0);
  com_argv[com_argc++] = argv2;
  cptr = next_non_white(cptr+jip_int_count((JIP_INDEX)strlen(argv2)));

  if ( sscanf(cptr,"%s",argv3) == 0 ) return(0);
  com_argv[com_argc++] = argv3;
  cptr = next_non_white(cptr+jip_int_count((JIP_INDEX)strlen(argv3)));

  if ( sscanf(cptr,"%s",argv4) == 0 ) return(0);
  com_argv[com_argc++] = argv4;
  cptr = next_non_white(cptr+jip_int_count((JIP_INDEX)strlen(argv4)));

  if ( sscanf(cptr,"%s",argv5) == 0 ) return(0);
  com_argv[com_argc++] = argv5;
  cptr = next_non_white(cptr+jip_int_count((JIP_INDEX)strlen(argv5)));

  if ( sscanf(cptr,"%s",argv6) == 0 ) return(0);
  com_argv[com_argc++] = argv6;
  cptr = next_non_white(cptr+jip_int_count((JIP_INDEX)strlen(argv6)));

  if ( sscanf(cptr,"%s",argv7) == 0 ) return(0);
  com_argv[com_argc++] = argv7;
  cptr = next_non_white(cptr+jip_int_count((JIP_INDEX)strlen(argv7)));

  if ( sscanf(cptr,"%s",argv8) == 0 ) return(0);
  com_argv[com_argc++] = argv8;
  cptr = next_non_white(cptr+jip_int_count((JIP_INDEX)strlen(argv8)));

  if ( sscanf(cptr,"%s",argv9) == 0 ) return(0);
  com_argv[com_argc++] = argv9;
  cptr = next_non_white(cptr+jip_int_count((JIP_INDEX)strlen(argv9)));
  return(0);
}

int replace_passed_arguments(char *command_line,int argc,char *argv[])
{
  char result[LEN_STR];size_t i=0,n=0,j;int arg;
  while(command_line[i]) {
    if(command_line[i]=='$') {
      i++;
      if(command_line[i]<'0'||command_line[i]>'9')goto invalid;
      arg=command_line[i++]-'0'+1;
      if(arg>=argc||!argv[arg])goto invalid;
      for(j=0;argv[arg][j];j++) {
        if(n>=sizeof(result)-1)goto invalid;
        result[n++]=argv[arg][j];
      }
    } else {
      if(n>=sizeof(result)-1)goto invalid;
      result[n++]=command_line[i++];
    }
  }
  result[n]=0;memcpy(command_line,result,n+1);return 0;
invalid:
  jip_text(2,"Invalid command argument substitution or command exceeds buffer size.");
  return 1;
}

int interpret_command( int ilist, char *command )
/*
** Interpret a command, which is passed as a character string pointer,
** and return the location of the command within the command list,
** which is an array of structures.  If the command is not found, the value
** returned is -1.  The present implementation requires that commands are
** entered exactly (except for case).
*/
{
  struct COMMAND_LIST *comptr;
  int icommand, icount;

  // convert_to_lower( command );
  icommand = -1;
  icount = 0;
  /*
  ** Select the command list or the variable list.
  */
  if ( ilist == 0 )
    comptr = command_list;
  else
    comptr = variable_list;
  while ( *comptr->command != CNULL )
    {
      if ( !strcmp(comptr->command,command) )
	{
	  icommand = icount;
	  break;
	}
      icount++;
      comptr++;
    }
  /*
  ** If the command was not found in the command list, check the macro list.
  */
  if ( icommand==-1 && ilist==0 )
    {
      comptr = macro_list;
      icount = 0;
      while ( *comptr->command != CNULL )
	{
	  if ( !strcmp(comptr->command,command) )
	    {
	      icommand = icount + MAX_COMMANDS;
	      break;
	    }
	  icount++;
	  comptr++;
	}
    }
  return( icommand );
}

int act_on_command( int icommand, char *argument )
/*
** Act upon a command which has already been decoded into an integer and
** an argument.
*/
{
  int number, file_type;
  float real_number, real_number1, real_number2;
  char file_name[LEN_STR], stack_name[LEN_STR], list_name[LEN_STR];
  char string1[LEN_STR];

  initialize_volume_list();

  /*
  ** Loop over all commands.  In most cases, each case is written out
  ** explicitly (rather than combining similar cases) so that the
  ** order of command in the "enum" statement need not be maintained
  ** in a fixed way.
  */
  switch( icommand )
    {
 // Program control
    case QUIT:  case EXIT:  case STOP:  case BYE:
      exit(0);
      break;
    case HELP:
      help_menu(0,"jip-menu.txt");
      break;
    case SET:
      jip_text(3,"Command = 'set'; this path should never occur.");
      exit(1);
    case SHOW:
      jip_text(3,"Command = 'show'; this path should never occur.");
      exit(1);
    case SHOW_STACKS:         /* "syntax: show-stacks <stack list>" */
      if ( decode_volume_list(0,argument) ) return(1);
      show_stacks();
      break;
 // Input/output
    case READ:
      if ( decode_read_argument(argument,file_name,&file_type,
				stack_name,list_name,&number) )
	return(1);
      if ( file_type == JIP_COM )
	{
	  /* argument[jip_int_count((JIP_INDEX)strlen(argument))-1] = CNULL; */
	  strip_com_arguments(argument);
	  sscanf(argument,"%s",file_name);
	  if ( flags&MODE_BIT2 )
	    printf("Error: nested jip command files are not yet allowed.\n");
	  else if ( flags&MODE_BIT1 )
	    {
	      if ( !(mode_file2 = fopen(file_name,"r")) )
		printf("Error: cannot open the jip command file %s.\n",file_name);
	      else
		flags = flags | MODE_BIT2;
	    }
	  else
	    {
	      if ( !(mode_file1 = fopen(file_name,"r")) )
		printf("Error: cannot open the jip command file %s.\n",file_name);
	      else
		flags = flags | MODE_BIT1;
	    }
	}
      else if ( file_type == OVL )
	read_ovl_file( file_name );
      else
	read_xdisplay_file( file_name, stack_name, list_name, time_step );
      break;
    case READ_LOOP:
      if ( decode_read_argument(argument,file_name,&file_type,
				stack_name,list_name,&number) )
	return(1);
      if ( file_type==BSHORT    || file_type==BFLOAT || file_type==BLONG ||
	   file_type==USHORT    || file_type==UINT8 ||
	   file_type==NIFTI_NII || file_type==NIFTI_IMG ||
	   file_type == FREESURFER ||
	   file_type == -1 ) // unknown
	initialize_read_loop(file_type, file_name, stack_name, list_name );
      read_loop_running = TRUE;
      break;
    case END_READ_LOOP:
      read_loop_running = FALSE;
      flush_read_loop_commands();
      break;
    case MODE:  /* Equivalent to read file.com, but this command works with read-loop */
      strip_com_arguments(argument);
      sscanf(argument,"%s",file_name);
      if ( flags&MODE_BIT2 )
	printf("Error: nested jip command files are not yet allowed.\n");
      else if ( flags&MODE_BIT1 )
	{
	  if ( !(mode_file2 = fopen(file_name,"r")) )
	    printf("Error: cannot open the jip command file %s.\n",file_name);
	  else
	    flags = flags | MODE_BIT2;
	}
      else
	{
	  if ( !(mode_file1 = fopen(file_name,"r")) )
	    printf("Error: cannot open the jip command file %s.\n",file_name);
	  else
	    flags = flags | MODE_BIT1;
	}
      break;
    case WRITE:
      if ( decode_string_and_list(1,argument,file_name) ) return(1);
      write_output_file( -1, file_name );
      break;
    case WRITE_SHORT:
      if ( decode_string_and_list(1,argument,file_name) ) return(1);
      write_output_file( BSHORT, file_name );
      break;
    case WRITE_FLOAT:
      if ( decode_string_and_list(1,argument,file_name) ) return(1);
      write_output_file( BFLOAT, file_name );
      break;
    case WRITE_SLICES:
      if ( decode_string_and_list(1,argument,file_name) ) return(1);
      write_slices( file_name );
      break;
    case WRITE_OVL:
      write_ovl( argument );
      break;
    case DELETE_OVL:
      delete_overlay( argument );
      break;
    case CREATE_BITMAP_FROM_OVL:
      if ( decode_string_and_list(0,argument,stack_name) ) return(1);
      create_bitmap_from_overlay( stack_name );
      break;
    case CREATE_OVL_FROM_BITMAP:
      if ( decode_volume_list(0,argument) ) return(1);
      create_overlay_from_bitmap();
      break;
    case MIRROR_OVERLAY_X:
      if ( decode_int_and_list(argument,&number) ) return(1);
      mirror_overlay_in_x( number );
      break;
 // Averaging
    case AVERAGE:          /* "syntax: average [stack name] <volume list>" */
      if ( decode_string_and_list(0,argument,stack_name) ) return(1);
      average_volumes( stack_name );
      break;
    case AVERAGE_BY:      /*"syntax:average-by [int] [s_name] <i_list>"*/
      if (decode_int_string_and_list(0,argument,&number,stack_name)) return(1);
      average_by_n( stack_name, number );
      break;
    case AVERAGE_EPOCHS:  /*"syntax:average-epochs [int] [s_name] <i_list>"*/
      if (decode_int_string_and_list(0,argument,&number,stack_name)) return(1);
      average_epochs( stack_name, number );
      break;
    case AVERAGE_STACKS:  /*"syntax:average-stacks [stack name] <volume list>"*/
      if ( decode_string_and_list(0,argument,stack_name) ) return(1);
      average_stacks( stack_name );
      break;
    case SMOOTH_2D:       /*"syntax:smooth-2d [ fwhm in pixels] <stack list>"*/
      if ( decode_real_and_list(argument,&real_number) ) return(1);
      smooth_in_kspace( real_number, 2 );
      break;
    case SMOOTH_3D:       /*"syntax:smooth-3d [fwhm in mm] <stack list>"*/
      if ( decode_real_and_list(argument,&real_number) ) return(1);
      smooth_in_kspace( real_number, 3 );
      //      sharpen_in_kspace( real_number, 3 );
      break;
    case FLATTEN:       /*"syntax:flatten [fov fraction] <stack list>"*/
      if ( decode_real_and_list(argument,&real_number) ) return(1);
      flatten_volumes( real_number );
      //      sharpen_in_kspace( real_number, 3 );
      break;
    case AVERAGE_WEIGHTED:  /*"syntax:weighted-average [stack name] <i_list>"*/
      if ( decode_string_and_list(0,argument,stack_name) ) return(1);
      weighted_average( stack_name );
      break;
    case CREATE_VARS:  /*"syntax:create-variance [stack name] <volume list>"*/
      if ( decode_string_and_list(0,argument,stack_name) ) return(1);
      create_variances( stack_name );
      break;
 // Arithmetic functions
    case ABS: /* "syntax: abs <volume list>" */
      if ( decode_volume_list(0,argument) ) return(1);
      absolute_value();
      break;
    case ADD:              /* "syntax: add      <ref volume> <volume list>" */
      if ( decode_comparison_list(argument) ) return(1);
      arithmetic_function( 1 );
      break;
    case ADD_CONST:         /* "syntax: add-constant      <volume list>" */
      if ( decode_real_and_list(argument,&real_number) ) return(1);
      arithmetic_function_constant( 1, real_number );
      break;
    case ADD_STACKS:       /* "syntax: add_stacks      <ref stack> <s list>" */
      if ( decode_comparison_list(argument) ) return(1);
      arithmetic_function_on_stacks( 1 );
      break;
    case CREATE_FCBV:             /* "syntax: create-fCBV [stack name] [pre volume] [post volume] <volume list>" */
    case CREATE_FCBV1:            /* "syntax: create-fCBV [stack name] [pre volume] [post volume] [base volume] <volume list>" */
      if ( decode_string_and_list(0,argument,stack_name) ) return(1);
      create_fCBV( icommand-CREATE_FCBV, stack_name );
      break;
    case DIVIDE:           /* "syntax: divide   <ref volume> <volume list>" */
      if ( decode_comparison_list(argument) ) return(1);
      arithmetic_function( 2 );
      break;
    case MULTIPLY:         /* "syntax: multiply <ref volume> <volume list>" */
      if ( decode_comparison_list(argument) ) return(1);
      arithmetic_function( 3 );
      break;
    case ATAN:                /* "syntax: atan <volume list>" */
      if ( decode_comparison_list(argument) ) return(1);
      arithmetic_function( 4 );
      break;
    case DIVIDE_CONST:      /* "syntax: divide-constant   <volume list>" */
      if ( decode_real_and_list(argument,&real_number) ) return(1);
      arithmetic_function_constant( 2, real_number );
      break;
    case DIVIDE_STACKS:    /* "syntax: divide_stacks   <ref stack> <s list>" */
      if ( decode_comparison_list(argument) ) return(1);
      arithmetic_function_on_stacks( 2 );
      break;
    case EXP:               /* "syntax: exp               <volume list>" */
      if ( decode_real_and_list(argument,&real_number) ) return(1);
      arithmetic_function_constant( 5, real_number );
      break;
    case LN:                /* "syntax: ln [constant] <volume list>" */
      if ( decode_real_and_list(argument,&real_number) ) return(1);
      arithmetic_function_constant( 4, real_number );
      break;
    case MULTIPLY_CONST:    /* "syntax: multiply-constant <volume list>" */
      if ( decode_real_and_list(argument,&real_number) ) return(1);
      arithmetic_function_constant( 3, real_number );
      break;
    case MULTIPLY_STACKS:  /* "syntax: multiply_stacks <ref stack> <s list>" */
      if ( decode_comparison_list(argument) ) return(1);
      arithmetic_function_on_stacks( 3 );
      break;
    case SUBTRACT:         /* "syntax: subtract <ref volume> <volume list>" */
      if ( decode_comparison_list(argument) ) return(1);
      arithmetic_function( 0 );
      break;
    case SUBTRACT_CONST:    /* "syntax: subtract-constant <volume list>" */
      if ( decode_real_and_list(argument,&real_number) ) return(1);
      arithmetic_function_constant( 0, real_number );
      break;
    case SUBTRACT_STACKS:  /* "syntax: subtract_stacks <ref stack> <s list>" */
      if ( decode_comparison_list(argument) ) return(1);
      arithmetic_function_on_stacks( 0 );
      break;
 // Magnitude corrections
    case BASELINE_CORRECT: /* "syntax: baseline-correct <volume list>" */
      if ( decode_volume_list(0,argument) ) return(1);
      baseline_correct();
      break;
    case BASELINE_CORRECT_VOLUMES: /* "baseline-correct-volumes <stack list>" */
      if ( decode_volume_list(0,argument) ) return(1);
      baseline_correct_volumes();
      break;
    case MAGNITUDE_WINDOW: /*"syntax:magnitude-window <volume list>"*/
      if ( decode_volume_list(0,argument) ) return(1);
      magnitude_window();
      break;
    case MAGNITUDE_WINDOW_TOGETHER:/*"magnitude-window-together <volume list>"*/
      if ( decode_volume_list(0,argument) ) return(1);
      magnitude_window_together();
      break;
    case SCALE:          /*"syntax:scale <volume list>"*/
      if ( decode_real_and_list(argument,&real_number) ) return(1);
      scale_volumes(real_number);
      break;
    case NORMALIZE:          /*"syntax:normalize <volume list>"*/
      if ( decode_real_and_list(argument,&real_number) ) return(1);
      normalize_volumes(real_number);
      break;
 // Reordering of data
    case CONVERT_COMPLEX:/*"syntax:convert-complex <stack list>"*/
      if ( decode_volume_list(0,argument) ) return(1);
      convert_complex();
      break;
    case CONVERT_TO_REAL:/*"syntax:convert-to-real <stack list>"*/
      if ( decode_volume_list(0,argument) ) return(1);
      convert_to_real();
      break;
    case COPY:             /* "syntax: copy [stack name] <volume list>" */
      if ( decode_string_and_list(0,argument,stack_name) ) return(1);
      copy_volumes( stack_name );
      break;
    case DELETE:          /*"syntax: delete <stack list>"*/
      if ( decode_volume_list(0,argument) ) return(1);
      delete_stacks();
      break;
    case CLIP_X:        /* "syntax: clip-x <stack list>" */
    case CLIP_Y:        /* "syntax: clip-y <stack list>" */
    case CLIP_Z:        /* "syntax: clip-z <stack list>" */
      if ( decode_volume_list(0,argument) ) return(1);
      clip_stacks(icommand-CLIP_X);
      break;
    case CLIP_X_TO:     /* "syntax: clip-x-to [int] <stack list>" */
    case CLIP_Y_TO:     /* "syntax: clip-y-to [int] <stack list>" */
    case CLIP_Z_TO:     /* "syntax: clip-z-to [int] <stack list>" */
      if ( decode_int_and_list(argument,&number) ) return(1);
      clip_stacks_to_n( icommand-CLIP_X_TO,number );
      break;
    case PAD_X:         /* "syntax: pad-x   <stack list>" */
    case PAD_Y:         /* "syntax: pad-y   <stack list>" */
    case PAD_Z:         /* "syntax: pad-z   <stack list>" */
    case PAD_XY:        /* "syntax: pad     <stack list>" */
    case PAD_XYZ:       /* "syntax: pad-xyz <stack list>" */
      if ( decode_volume_list(0,argument) ) return(1);
      pad_stacks(icommand-PAD_X);
      break;
    case PAD_TO:       /* "syntax: pad-to   [int] <stack list>" */
      if ( decode_int_and_list(argument,&number) ) return(1);
      pad_stacks_to_n( 2, number );
      break;
    case PAD_X_TO:     /* "syntax: pad-x-to [int] <stack list>" */
      if ( decode_int_and_list(argument,&number) ) return(1);
      pad_stacks_to_n( 0, number );
      break;
    case PAD_Y_TO:     /* "syntax: pad-y-to [int] <stack list>" */
      if ( decode_int_and_list(argument,&number) ) return(1);
      pad_stacks_to_n( 1, number );
      break;
    case PAD_2N:             /* "syntax: pad-2^n <stack list>" */
      if ( decode_volume_list(0,argument) ) return(1);
      pad_2n_stacks();
      break;
    case REORDER_SLICES:     /* "syntax: reorder-slices <stack list>" */
      if ( decode_volume_list(0,argument) ) return(1);
      reorder_slices();
      break;
    case REVERSE_SLICE_ORDER:
      if ( decode_volume_list(0,argument) ) return(1);
      reverse_slice_order();
      break;
    case SHUFFLE:        /*"syntax: shuffle [new stack] <stack list>"*/
      if ( decode_string_and_list(0,argument,stack_name) ) return(1);
      shuffle_stacks( stack_name );
      break;
    case SLICE:           /*"syntax:slice [int] [s_name] <i_list>"*/
      if (decode_int_string_and_list(0,argument,&number,stack_name)) return(1);
      slice_volumes( stack_name, number );
      break;
    case REORDER_TIME_SLICE:     /* "syntax: reorder-time&slice <stack list>" */
      if ( decode_volume_list(0,argument) ) return(1);
      reorder_time_and_slices();
      break;
 // Transformations and filters
    case TRANSFORM:          /* "syntax: transform <volume list>" */
      if ( decode_volume_list(0,argument) ) return(1);
      ft_1d( 1, -1 ); // x
      ft_1d( 2, -1 ); // y
      break;
    case INVERSE_TRANSFORM:  /* "syntax: inverse-transform <volume list>" */
      if ( decode_volume_list(0,argument) ) return(1);
      ft_1d( 1, 1 ); // x
      ft_1d( 2, 1 ); // y
      break;
    case TRANSFORM_X:    /* "syntax: transform-x <volume list>" */
      if ( decode_volume_list(0,argument) ) return(1);
      ft_1d(1,-1);
      break;
    case INVERSE_TRANSFORM_X:  /* "inverse-transform-x <volume list>"  */
      if ( decode_volume_list(0,argument) ) return(1);
      ft_1d(1,1);
      break;
    case TRANSFORM_Y:          /* "syntax: transform-y <volume list>" */
      if ( decode_volume_list(0,argument) ) return(1);
      ft_1d(2,-1);
      break;
    case INVERSE_TRANSFORM_Y: /* "inverse-transform-y <volume list>" */
      if ( decode_volume_list(0,argument) ) return(1);
      ft_1d(2,1);
      break;
    case TRANSFORM_XYZ:      /* "syntax: transform-xyz <volume list>" */
      if ( decode_volume_list(0,argument) ) return(1);
      ft_1d( 1, -1 ); // x
      ft_1d( 2, -1 ); // y
      ft_1d( 3, -1 ); // z
      break;
    case INVERSE_TRANSFORM_XYZ: /*"syntax: inverse-transform-xyz <volume list>" */
      if ( decode_volume_list(0,argument) ) return(1);
      ft_1d( 1, 1 ); // x
      ft_1d( 2, 1 ); // y
      ft_1d( 3, 1 ); // z
      break;
    case TRANSFORM_TIME:      /* "syntax: transform-time <stack list>" */
      if ( decode_volume_list(0,argument) ) return(1);
      ft_time();
      break;
    case HAMMING_WINDOW:
      if ( decode_volume_list(0,argument) ) return(1);
      hamming_window( 0 );
      break;
    case HAMMING_WINDOW_XYZ:
      if ( decode_volume_list(0,argument) ) return(1);
      hamming_window( 1 );
      break;
    case EXP_FILTER_1D:
      if ( decode_real_and_list(argument,&real_number) ) return(1);
      exponential_filter_1d( real_number );
      break;
    case EDGE_FILTER:
      if ( decode_volume_list(0,argument) ) return(1);
      edge_filter();
      break;
    case ANALYZE_LECTIN:
      if ( decode_volume_list(0,argument) ) return(1);
      analyze_lectin();
      break;
 // Alignment
    case MIRROR_LEFT_RIGHT:    /* "syntax: mirror-left-right <volume list>" */
      if ( decode_volume_list(0,argument) ) return(1);
      swap_quadrants_in_volume_list( 1,0,1,1 );
      break;
    case MIRROR_UP_DOWN:       /* "syntax: mirror-up-down <volume list>" */
      if ( decode_volume_list(0,argument) ) return(1);
      swap_quadrants_in_volume_list( 1,1,1,1 );
      break;
    case REGISTER:
      if ( decode_volume_list(0,argument) ) return(1);
      register_volume(0);
      break;
    case INVERSE_REGISTER:
      if ( decode_volume_list(0,argument) ) return(1);
      inverse_register_volume(0);
      break;
    case REGISTER_HIST:
      if ( decode_volume_list(0,argument) ) return(1);
      if ( register_histology(0) ) return(1);
      break;
    case DOWNSAMPLE:
      if ( decode_volume_list(0,argument) ) return(1);
      downsample_stacks();
      break;
    case SHIFT_PIXELS:   /* "syntax: shift [x pix.] [y pix.] <stack list>" */
      if ( decode_3reals_and_list(argument,&real_number,&real_number1,&real_number2) )
	return(1);
      ishift_pixels( (int)real_number, (int)real_number1, (int)real_number2 );
      break;
 // Maps
    case MAP_SNR:
      if ( decode_string_and_list(0,argument,stack_name) ) return(1);
      map_snr( stack_name );
      break;
    case MAP_T2:
      if ( decode_string_and_list(0,argument,stack_name) ) return(1);
      map_t2( 1, stack_name );
      break;
    case MAP_DIFFUSION:
      if ( decode_string_and_list(0,argument,stack_name) ) return(1);
      map_t2( 0, stack_name );
      break;
    case MAP_T1_FLIP:
      //      if (decode_int_string_and_list(0,argument,&number,stack_name)) return(1);
      //      map_t1_from_multiple_tr( number, stack_name );
      if ( decode_string_and_list(0,argument,stack_name) ) return(1);
      map_t1_Barral_MRM2010( stack_name );
      break;
 // GLM
    case READ_GLM:
      macro_read_glm_control_file(argument);
      break;
    case GLM_REMOVE_BASELINE:
      strcpy(list_name,"glm");
      if ( decode_volume_list(0,list_name) ) return(1);
      glm_remove_or_correct_baseline(0);
      break;
    case GLM_CORRECT_BASELINE:
      strcpy(list_name,"glm");
      if ( decode_volume_list(0,list_name) ) return(1);
      glm_remove_or_correct_baseline(1);
      break;
    case GLM_HISTOGRAM:
      //      strcpy(list_name,argument);
      //      strcat(list_name,"  glm");
      //      if ( decode_comparison_list(list_name) ) return(1);
      strcpy(list_name,"glm");
      if ( decode_volume_list(0,list_name) ) return(1);
      glm_histogram_unbiased();
      break;
    case GLM_RESIDUE:
      if (decode_2strings_and_list(0,argument,string1,stack_name)) return(1);
      strcpy(list_name,"glm");
      if ( decode_volume_list(0,list_name) ) return(1);
      glm_create_residue( string1, stack_name );
      break;
    case GLM_RESIDUAL_VARIANCE:
      if (decode_string_and_list(0,argument,stack_name)) return(1);
      strcpy(list_name,"glm");
      if ( decode_volume_list(0,list_name) ) return(1);
      glm_create_residual_variance( stack_name );
      break;
    case GLM_MAP_AR:
      if (decode_string_and_list(0,argument,stack_name)) return(1);
      strcpy(list_name,"glm");
      if ( decode_volume_list(0,list_name) ) return(1);
      glm_create_AR_maps( stack_name );
      break;
    case GLM_FIT_EVENTS:
      if (decode_string_and_list(0,argument,stack_name)) return(1);
      strcpy(list_name,"glm");
      if ( decode_volume_list(0,list_name) ) return(1);
      glm_fit_events( stack_name );
      break;
    case GLM_MAX_EFFECT:
      if (decode_2strings_and_list(0,argument,string1,stack_name)) return(1);
      strcpy(list_name,"glm");
      if ( decode_volume_list(0,list_name) ) return(1);
      glm_maximal_effect( string1, stack_name );
      break;
    case GLM_VARY_TAU: // glm-vary-tau [event number] [new stack name]
      if (decode_int_string_and_list(0,argument,&number,stack_name)) return(1);
      strcpy(list_name,"glm");
      if ( decode_volume_list(0,list_name) ) return(1);
      glm_vary_time_constant( number, stack_name );
      break;
    case GLM_VARY_TAUS: // GLM-vary-tau [# time constants] [new stack name] [F volume] [glm]
      if (decode_int_string_and_list(0,argument,&number,stack_name)) return(1);
      glm_vary_time_constants( number, stack_name );
      break;
 // stats
    case FILL_HISTOGRAM: // "histogram 
      if ( decode_volume_list(0,argument) ) return(1);
      fill_histogram();
      break;
    case FILL_WEIGHTED_HISTOGRAM: // "weighted-histogram
      if ( decode_volume_list(0,argument) ) return(1);
      fill_weighted_histogram();
      break;
 // miscellaneous
    case SCATTER_PLOT:   // "syntax: scatter-plot [file name] [image 1] [image 2]"
      if ( decode_string_and_list(1,argument,file_name) ) return(1);
      scatter_plot( file_name );
      break;
    case CALC_OVL_OVERLAP:
      if ( decode_real_and_list(argument,&real_number) ) return(1);
      calculate_overlay_volume_overlap(real_number);
      break;
    default:
      jip_text(2,"The command was not found.");
    }
  return(0);
}

int act_on_macro( int imacro, char *argument )
/*
** Act upon a command which has already been decoded into an integer and
** an argument.
*/
{
  int number;

  initialize_volume_list();

  switch( imacro )
    {
    case REORDER_INT_SLICES:
      if ( decode_int_and_list(argument,&number) ) return(1);
      macro_reorder_interleaved_slices(number);
      break;
    default:
      jip_text(2,"The macro was not found.");
    }
  return(0);
}

void set_variable( int ivariable, char *argument )
/*
** Set a variable to its input value.
*/
{
  char string1[LEN_STR], file_name[LEN_STR], stack_name[LEN_STR], list_name[LEN_STR];
  int file_type, number;
  float h1, h2, h3, h4, fnumber;
  static int number_histology_slices=0;

  /* Convert the argument to a string for miscellaneous comparisons. */
  sscanf(argument,"%s",string1);

  /* Set the variable. */
  switch( ivariable )
    {
    case OVERLAY_LOGIC:
      if ( !strcmp("replace",string1) )
	overlay.io_logic = 0;
      else if ( !strcmp("or",string1) )
	overlay.io_logic = 1;
      else if ( !strcmp("and",string1) )
	overlay.io_logic = 2;
      else if ( !strcmp("not",string1) )
	overlay.io_logic = 3;
      else
	jip_text(2,"Use keywords `replace', `or', `and', or `not' to set this variable.");
      break;
    case COMPLEX_OUTPUT:
      if ( !strcmp("on",string1) )
	flags = flags | COMPLEX_OUTPUT_BIT;
      else if ( !strcmp("off",string1) )
	flags = flags & ~COMPLEX_OUTPUT_BIT;
      else
	jip_text(2,"Use keywords `on' and `off' to set program flags.");
      break;
    case PHASE_OUTPUT:
      if ( !strcmp("on",string1) )
	flags = flags | PHASE_OUTPUT_BIT;
      else if ( !strcmp("off",string1) )
	flags = flags & ~PHASE_OUTPUT_BIT;
      else
	jip_text(2,"Use keywords `on' and `off' to set program flags.");
      break;
    case IO_INCREMENT:
      if ( sscanf(argument,"%d",&io_increment) != 1 )
	jip_text(2,"Syntax = `set io-increment <number>'.");
      if ( io_increment < 0 )
	{
	  sprintf(error_string,"%d is an invalid value for `io_increment'.",
		  io_increment);
	  jip_text(2,error_string);
	  io_increment = 0;
	}
      break;
    case NIFTI_METHOD:
      if ( sscanf(argument,"%d",&nifti_method) != 1 )
	jip_text(2,"Syntax = `set nifti-method <number>'.");
      if ( nifti_method != 2 && nifti_method != 3 )
	{
	  sprintf(error_string,"%d is an invalid value for `nifti-method'; use 2 or 3.",
		  nifti_method);
	  jip_text(2,error_string);
	  nifti_method = 3;
	}
      break;
    case SLICE_ORDER:
      read_slice_order( argument );
      break;
    case MAP_ARRAY:
      read_map_array( argument );
      break;
    case FILTER_RANGE:
      if ( sscanf(argument,"%f %f %f",&filter_range.x, &filter_range.y, &filter_range.z) != 3 )
	{
	  if ( sscanf(argument,"%f %f",&filter_range.x, &filter_range.y) != 2 )
	    {
	      if ( sscanf(argument,"%f",&filter_range.x) != 1 )
		jip_text(2,"Use a syntax like `set filter-range 1. 1. 1.'\n               or `set filter-range 1.'");
	      else
		filter_range.y = filter_range.z = filter_range.x;
	    }
	  else
	    filter_range.z = 1.;
	}
      break;
    case TARGET_VOXEL_AFFINE:
    case SOURCE_VOXEL_AFFINE:
      {
        mat44 m={0};float *a=&m.m[0][0];int r, c, valid=1;double det;
        if(sscanf(argument,"%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f",a,a+1,a+2,a+3,a+4,a+5,a+6,a+7,a+8,a+9,a+10,a+11,a+12,a+13,a+14,a+15)!=16)
          {jip_text(2,"Native voxel affine requires 16 finite row-major values.");break;}
        for(r=0;r<4;r++)for(c=0;c<4;c++)if(!isfinite(m.m[r][c]))valid=0;
        det=(double)m.m[0][0]*(m.m[1][1]*m.m[2][2]-m.m[1][2]*m.m[2][1])-(double)m.m[0][1]*(m.m[1][0]*m.m[2][2]-m.m[1][2]*m.m[2][0])+(double)m.m[0][2]*(m.m[1][0]*m.m[2][1]-m.m[1][1]*m.m[2][0]);
        if(!valid||!isfinite(det)||fabs(det)<1.e-12||m.m[3][0]!=0||m.m[3][1]!=0||m.m[3][2]!=0||m.m[3][3]!=1){jip_text(2,"Native geometry must be a nonsingular affine.");break;}
        if(ivariable==TARGET_VOXEL_AFFINE){Alignment.voxel_affineT=m;Alignment.has_voxel_affineT=TRUE;}
        else {Alignment.voxel_affineS=m;Alignment.has_voxel_affineS=TRUE;}
      }
      break;
    case REG_TYPE:
      if ( !strcmp("deck-of-cards",string1) )
	Alignment.RigidBody = 0;
      else
	Alignment.RigidBody = 1;
      break;
    case REG_SPACE:
      if ( !strcmp("paxinos-rat",string1) )
	Alignment.registration = REG_PW_RAT_OLD;
      else
	Alignment.registration = REG_PW_RAT;
      break;
    case REG_MATRIX:
    case TARGET_MATRIX:
      if ( sscanf(argument,"%d %d %d",
		  &Alignment.dimensionT.x,&Alignment.dimensionT.y,&Alignment.dimensionT.z) != 3 )
	jip_text(2,"Use a syntax like `set target-matrix <3 int variables>'.");
      break;
    case SOURCE_MATRIX:
      if ( sscanf(argument,"%d %d %d",
		  &Alignment.dimensionS.x,&Alignment.dimensionS.y,&Alignment.dimensionS.z) != 3 )
	jip_text(2,"Use a syntax like `set source-matrix <3 int variables>'.");
      break;
    case REG_RESOLUTION:
    case TARGET_RESOLUTION:
      if ( sscanf(argument,"%f %f %f",
		  &Alignment.resolutionT.x,&Alignment.resolutionT.y,&Alignment.resolutionT.z) != 3 )
	jip_text(2,"Use a syntax like `set target-resolution <3 variables>'.");
      break;
    case SOURCE_RESOLUTION:
      if ( sscanf(argument,"%f %f %f",
		  &Alignment.resolutionS.x,&Alignment.resolutionS.y,&Alignment.resolutionS.z) != 3 )
	jip_text(2,"Use a syntax like `set source-resolution <3 variables>'.");
      break;
    case REG_ORIGIN:
    case TARGET_ORIGIN:
      if ( sscanf(argument,"%f %f %f",
		  &Alignment.originT.x,&Alignment.originT.y,&Alignment.originT.z) != 3 )
	jip_text(2,"Use a syntax like `set target-origin <3 variables>'.");
      break;
    case SOURCE_ORIGIN:
      if ( sscanf(argument,"%f %f %f",
		  &Alignment.originS.x,&Alignment.originS.y,&Alignment.originS.z) != 3 )
	jip_text(2,"Use a syntax like `set source-origin <3 variables>'.");
      break;
    case REG_DIRECTION:
    case TARGET_DIRECTION:
      if ( sscanf(argument,"%f %f %f",
		  &Alignment.directionT.x,&Alignment.directionT.y,&Alignment.directionT.z) != 3 )
	jip_text(2,"Use a syntax like `set target-direction <3 variables>'.");
      break;
    case SOURCE_DIRECTION:
      if ( sscanf(argument,"%f %f %f",
		  &Alignment.directionS.x,&Alignment.directionS.y,&Alignment.directionS.z) != 3 )
	jip_text(2,"Use a syntax like `set source-direction <3 variables>'.");
      break;
    case REG_ORIENT:
      if ( sscanf(argument,"%d %d %d %d",
		  &Alignment.OrientRotation.x,&Alignment.OrientRotation.y,&Alignment.OrientRotation.z,
		  &Alignment.OrientSwapZ) != 4 )
	jip_text(2,"Use a syntax like `set registration-orientation <4 int variables>'.");
      break;
    case REG_WRAPPING:
      if ( sscanf(argument,"%d %d %d",
		  &Alignment.WrapAround.x,&Alignment.WrapAround.y,&Alignment.WrapAround.z) != 3 )
	jip_text(2,"Use a syntax like `set registration-wrapping <3 int variables>'.");
      break;
    case REG_TRANSLATIONS:
      if ( sscanf(argument,"%f %f %f",
		  Alignment.value,Alignment.value+1,Alignment.value+2) != 3 )
	jip_text(2,"Use a syntax like `set translations <3 variables>'.");
      break;
    case REG_ROTATIONS:
      if ( sscanf(argument,"%f %f %f",
		  Alignment.value+3,Alignment.value+4,Alignment.value+5) != 3 )
	jip_text(2,"Use a syntax like `set rotations <3 variables>'.");
      break;
    case REG_INFLATIONS:
      if ( sscanf(argument,"%f %f %f",
		  Alignment.value+6,Alignment.value+7,Alignment.value+8) != 3 )
	jip_text(2,"Use a syntax like `set inflations <3 variables>'.");
      break;
    case REG_SKEWS:
      if ( sscanf(argument,"%f %f %f",
		  Alignment.value+9,Alignment.value+10,Alignment.value+11) != 3 )
	jip_text(2,"Use a syntax like `set skews <3 variables>'.");
      break;
    case HIST_SLICES:
      if ( sscanf(argument,"%d",&number) != 1 )
	{
	  printf("Error reading the value of `set histology-slices'\n");
	  return;
	}
      if ( number != Alignment.dimensionT.z )
	{
	  printf("Warning: the number of histological slices (%d) != Aligment.dim.z (%d).\n",
		 number,Alignment.dimensionT.z);
	  Alignment.dimensionT.z = number;
	}
      if ( number_histology_slices != 0 )
	free_float_matrix(Alignment.slice_value,number_histology_slices,4);
      number_histology_slices = number;
      Alignment.slice_value = float_matrix(number_histology_slices,4);
      break;
    case HIST_SLICE:
      if ( sscanf(argument,"%d %f %f %f %f",&number,&h1,&h2,&h3,&h4) != 5 )
	{
	  printf("Error reading the value of `set histology-slice'\n");
	  printf("There should be 5 values: [slice index] [x] [y] [theta] [skew]\n");
	  return;
	}
      if ( number >= number_histology_slices )
	{
	  printf("Error: the listed histology slice (%d) is > # of slices (%d).\n",
		 number,number_histology_slices);
	  return;
	}
      Alignment.slice_value[number][0] = h1;
      Alignment.slice_value[number][1] = h2;
      Alignment.slice_value[number][2] = h3;
      Alignment.slice_value[number][3] = h4;
      break;
    case REG_DISTORT:
      if ( decode_read_argument(argument,file_name,&file_type,
				stack_name,list_name,&number) )
	return;
      if ( number_histology_slices == 0 )
	{
	  strcpy(list_name,"0-2");
	  strcpy(stack_name,"registration_distortions");
	}
      else
	{
	  strcpy(list_name,"0-1");
	  strcpy(stack_name,"histology_distortions");
	}
      read_xdisplay_file( file_name, stack_name, list_name, time_step );
      break;
    case HISTOGRAM_VARS:
      if ( sscanf(argument,"%f %f %f",&hist_start,&hist_end,&hist_bin_size) != 3 ||
	   hist_end <= hist_start || hist_bin_size <= 0. )
	jip_text(2,"Use a syntax like `set histogram <start> <end> <bin size>'.");
      else
	{
	  hist_nbins = ceil((double)( (hist_end - hist_start)/hist_bin_size ));
	  if ( histogram != NULL )
	    free(histogram);
	  histogram = index_vector(hist_nbins);
	  printf("Allocate histogram: %g %g %g (%d bins)\n",
	     hist_start,hist_end,hist_bin_size,hist_nbins);
	}
      break;
    case INTERP_KERNEL:
      if ( !strcmp("gaussian",string1) || !strcmp("Gaussian",string1) )
	{
	  Alignment.interpolation = INTERP_GAUSSIAN;
	  printf("A Gaussian interpolation kernel will smooth your data\n");
	  printf("by 1.5 x the voxel dimension, but it will not introduce ringing.\n"); 
	}
      else if ( !strcmp("lanczos",string1) || !strcmp("Lanczos",string1) )
	{
	  Alignment.interpolation = INTERP_LANCZOS;
	  printf("A Lanczos interpolation kernel will not smooth your data\n");
	  printf("but it will not introduce some ringing.\n");
	  printf("For downsampling > 2x, a Gaussian kernel will be used instead.\n");
	}
      else
	{
	  printf("Use a syntax like `set interpolation-kernel [type]\n");
	  printf("   where the type is Gaussian or Lanczos.\n");
	}      
      break;
    case TIME_STEP:
      if ( sscanf(argument,"%g",&fnumber) != 1 )
	{
	  printf("Error reading the value of `set time-step'\n");
	  return;
	}
      if ( fnumber < 0. )
	fnumber = 1.;
      time_step = fnumber;
      break;
    default:
      jip_text(2,"That variable name is not used in this program.");
    }
}

void show_variable( int ivariable )
/*
** Show that value of a variable (or all variables).
*/
{
  char column1[LEN_STR], line[LEN_STR];
  int j;

  switch( ivariable )
    {
    case COMPLEX_OUTPUT:
      if ( flags & COMPLEX_OUTPUT_BIT )
	strcpy(column1,"\tcomplex-output  ON");
      else
	strcpy(column1,"\tcomplex-output  OFF");
      strcpy(line,column1);
      printf("%s\n",line);
      break;
    case PHASE_OUTPUT:
      if ( flags & PHASE_OUTPUT_BIT )
	strcpy(column1,"\tphase-output    ON   (ignored when complex-output = ON)");
      else
	strcpy(column1,"\tphase-output    OFF  (ignored when complex-output = ON)");
      strcpy(line,column1);
      printf("%s\n",line);
      break;
    case IO_INCREMENT:
      sprintf(column1,"\tio-increment    %d",io_increment);
      strcpy(line,column1);
      printf("%s\n",line);
      break;
    case OVERLAY_LOGIC:
      if ( overlay.io_logic == 0 )
	sprintf(column1,"\toverlay-logic   replace");
      else if ( overlay.io_logic == 1 )
	sprintf(column1,"\toverlay-logic   or");
      else if ( overlay.io_logic == 2 )
	sprintf(column1,"overlay-logic     and");
      strcpy(line,column1);
      printf("%s\n",line);
      break;
    case NIFTI_METHOD:
      sprintf(column1,"\tnifti-method    %d",nifti_method);
      strcpy(line,column1);
      printf("%s\n",line);
      break;
    case REG_SPACE:
    case REG_TRANSLATIONS:
    case REG_ROTATIONS:
    case REG_INFLATIONS:
    case REG_SKEWS:
    case INTERP_KERNEL:
      printf("\n");
      if ( Alignment.RigidBody )
	printf("\t\t\tAlignment variables (rigid body)\n");
      else
	printf("\t\t\tAlignment variables (deck-of-cards)\n");
      printf("\t\t\t------------------\n");
      if ( Alignment.interpolation == INTERP_GAUSSIAN )
	printf("\tinterpolation-kernel Gaussian\n");
      else
	printf("\tinterpolation-kernel Lanczos\n");
      printf("\ttranslations         %g %g %g\n",Alignment.value[0],Alignment.value[1],Alignment.value[2]);
      printf("\trotations            %g %g %g\n",Alignment.value[3],Alignment.value[4],Alignment.value[5]);
      printf("\tinflations           %g %g %g\n",Alignment.value[6],Alignment.value[7],Alignment.value[8]);
      printf("\tskews                %g %g %g\n",Alignment.value[9],Alignment.value[10],Alignment.value[11]);
      printf("\n");
      break;
    case HISTOGRAM_VARS:
      printf("\thistogram       %g %g %g (%d bins)\n",
	     hist_start,hist_end,hist_bin_size,hist_nbins);
      break;
    case SLICE_ORDER:
      if ( nslice_order != 0 )
	{
	  fprintf(stdout,"\tslice-order     ");
	  for (j=0; j<nslice_order; j++)
	    fprintf(stdout,"%d ",slice_order[j]);
	  fprintf(stdout,"\n");
	}
      else
	fprintf(stdout,"\tslice-order     \n");
      break;
    case MAP_ARRAY:
      if ( nmap_array != 0 )
	{
	  fprintf(stdout,"\tmap-array       ");
	  for (j=0; j<nmap_array; j++)
	    fprintf(stdout,"%g ",map_array[j]);
	  fprintf(stdout,"\n");
	}
      else
	fprintf(stdout,"\tmap-array      \n");
      break;
    case FILTER_RANGE:
      fprintf(stdout,"\tfilter-range    %g %g (%g)\n",
	      filter_range.x, filter_range.y, filter_range.z);
      break;
    case TIME_STEP:
      fprintf(stdout,"\ttime-step       %g\n",time_step);
      break;
    default:
      show_variable( COMPLEX_OUTPUT );
      show_variable( PHASE_OUTPUT );
      show_variable( OVERLAY_LOGIC );
      show_variable( NIFTI_METHOD );
      printf("\n");
      show_variable( HISTOGRAM_VARS );
      show_variable( SLICE_ORDER );
      show_variable( MAP_ARRAY );
      show_variable( FILTER_RANGE );
      show_variable( REG_SPACE );
      show_variable( TIME_STEP );
    }
}

void initialize_global_variables()
{
  int j;

  for (j=0; j<NAFFINE; j++)
    {
      if ( j>=6 && j<9 ) // inflations
	Alignment.value[j] = 1.;
      else // set all other values to zero
	Alignment.value[j] = 0.;
    }
  registration_distortions.nt = 0;
  registration_distortions.dim.x = registration_distortions.dim.y = registration_distortions.dim.z = 0;
  registration_distortions.SpaceTime.fcData = NULL;

  flags = flags & ~COMPLEX_OUTPUT_BIT;
  flags = flags & ~PHASE_OUTPUT_BIT;
  flags = flags & ~MODE_BIT1;
  flags = flags & ~MODE_BIT2;

  io_increment = 0;

  filter_range.x = filter_range.y = filter_range.z = 1.;

  overlay.nvoxels = overlay.io_logic = 0;
  overlay.weight = NULL;
  overlay.voxel  = NULL;

  Alignment.RigidBody = 1;
  Alignment.dimensionT.x  = Alignment.dimensionT.y  = Alignment.dimensionT.z  = 0;
  Alignment.resolutionT.x = Alignment.resolutionT.y = Alignment.resolutionT.z = 0.;
  Alignment.originT.x     = Alignment.originT.y     = Alignment.originT.z     = 1.e10;
  Alignment.directionT.x  = Alignment.directionT.y  = Alignment.directionT.z  = 0.;
  Alignment.OrientRotation.x = Alignment.OrientRotation.y = Alignment.OrientRotation.z = 0.;
  Alignment.OrientSwapZ = 0;
  Alignment.WrapAround.x = Alignment.WrapAround.y = Alignment.WrapAround.z = 0;

  Alignment.value[0]=Alignment.value[1]=Alignment.value[2]=Alignment.value[3]
    =Alignment.value[4]=Alignment.value[5] = 0.;
  Alignment.value[6]=Alignment.value[7] =Alignment.value[8]  = 1.;
  Alignment.value[9]=Alignment.value[10]=Alignment.value[11] = 0.;
  Alignment.interpolation = INTERP_LANCZOS;

  // histogram
  hist_start = hist_end = hist_bin_size = 0.;
  hist_nbins = 0;
  histogram = NULL;
  // nifti method (2 or 3) for determining coordinates from voxels
  nifti_method = 3;

  com_argc = 0;
}

void initialize_volume_list()
{
  struct volume_list  *volume, *free_me;

  /* Free the main volume list. */
  volume = volume_list_root;
  while ( volume )
    {
      free_me = volume;
      volume = volume->next;
      free( (char *)free_me );
    }
  volume_list_root = NULL;

  /* Free the comparison volume list. */
  volume = comparison_list_root;
  while ( volume )
    {
      free_me = volume;
      volume = volume->next;
      free( (char *)free_me );
    }
  comparison_list_root = NULL;
}

int decode_volume_list( int which_list, char *argument )
     // which_list: 0 => volume_list_root  ,  1 => comparison_list_root
{
  char *next_non_white();
  int decode_stack_subset();
  char stack_and_volumes[LEN_STR];
  char *cptr;
  int nset, iset[MAX_NSET];
  int nt, jt;
  struct data_stack *stack;
  struct volume_list  *volume, *volume_last;

  nt = 0;
  if ( which_list == 0 ) volume = volume_list_root;
  else                   volume = comparison_list_root;
 /*
 ** Determine the number of text pieces in the argument string.
 */
  if ( sscanf(argument,"%s",stack_and_volumes) != 1 )
    {
      /* Put all volumes into the volume list */
      *(cptr=argument) = CNULL;
      for ( stack=stack_root; stack; stack=stack->next )
	{
	  /*
	  ** Alter the argument to reflect the full stack list.  This is done
	  ** so that multiple calls to this function during one command
	  ** execution will not change the volume list due to the creation of
	  ** new stacks.
	  */
	  strcat(cptr,stack->name);
	  cptr += jip_int_count((JIP_INDEX)strlen(stack->name));
	  strcat(cptr," ");
	  cptr++;
	  for ( jt=0; jt<stack->nt; jt++ )
	    {
	      if ( !(volume=(struct volume_list *)malloc(sizeof(struct volume_list))) )
		jip_text(3,"Out of memory in 'decode_volume_list'.");
	      volume->stack = stack;
	      volume->dim   = stack->dim;
	      volume->n2d   = stack->n2d;
	      volume->n3d   = stack->n3d;
	      volume->SpaceTime.fcData  = stack->SpaceTime.fcData + jt * stack->n3d;
	      volume->next  = NULL;
	      if ( nt == 0 )
		{
		  if ( which_list == 0 ) volume_list_root     = volume;
		  else                   comparison_list_root = volume;
		}
	      else
		volume_last->next = volume;
	      volume_last = volume;
	      nt++;
	    }
	}
      *cptr = CNULL;
      if ( nt == 0 )
	{
	  jip_text(1,"No volumes exist in memory.");
	  return(1);
	}
    }
  else
    {
      cptr = next_non_white( argument );
      do
	{
	  if ( decode_stack_subset(stack_and_volumes, &stack, &nset, iset) )
	    return(1);

	  /* Put the volumes from the given stack into the volume list. */
	  for ( jt=0; jt<nset; jt++ )
	    {
	      if ( !(volume=(struct volume_list *)malloc(sizeof(struct volume_list))) )
		jip_text(3,"Out of memory in `decode_volume_list'.");
	      volume->stack = stack;
	      volume->dim   = stack->dim;
	      volume->n2d   = stack->n2d;
	      volume->n3d   = stack->n3d;
	      volume->SpaceTime.fcData  = stack->SpaceTime.fcData + iset[jt] * stack->n3d;
	      volume->next  = NULL;
	      if ( nt == 0 )
		{
		  if ( which_list == 0 ) volume_list_root     = volume;
		  else                   comparison_list_root = volume;
		}
	      else
		volume_last->next = volume;
	      volume_last = volume;
	      nt++;
	    }
	  cptr = next_non_white( cptr+jip_int_count((JIP_INDEX)strlen(stack_and_volumes)) );
	}
      while ( sscanf(cptr,"%s",stack_and_volumes) == 1 );
    }

  return(0);
}

int decode_stack_subset( char *stack_and_volumes, struct data_stack **stack, int *nset, int *iset )
/*
** This routine decodes a syntax like "stack_name>1-4,8,10-12" into the
** stack name and the list of volumes within the stack.  The stack name is
** checked to be a valid one, and the corresponding stack pointer is returned.
** The list of volumes is checked to see that it falls within the existing set.
*/
{
  char stack_name[LEN_STR], list_name[LEN_STR];
  LOGICAL found;

  /*
  ** Separate the stack name and the volume list.
  */
  if ( separate_name_and_list(stack_and_volumes,stack_name,list_name) )
    return(1);
  /*
  ** Identify the stack pointer based upon the stack name.
  */
  found = 0;
  for ( *stack=stack_root; *stack; *stack=(*stack)->next )
    {
      if ( !strcmp((*stack)->name,stack_name) )
	found = 1;
      if ( found ) break;
    }
  if ( !found )
    {
      sprintf(error_string,"Stack %s does not exist.",stack_name);
      jip_text(2,error_string);
      return(1);
    }
  /*
  ** Finally, decode volume list.
  */
  if ( decode_subset(list_name,(*stack)->nt,nset,iset) )
    return(1);
  else
    return(0);
}

int separate_name_and_list( char *name_and_volumes, char *name, char *list_name )
{  
  char *cptr;
  int length, j;
  LOGICAL arrow;
  /*
  ** Pick off the stack name.  Volumes within a stack are selected by the user
  ** by including an volume list after a '>' following the stack name, so
  ** search for this character.
  */
  length = jip_int_count((JIP_INDEX)strlen( name_and_volumes ));
  arrow = 0;
  for ( cptr=name_and_volumes, j=0; j<length; j++, cptr++ )
    {
      if ( *cptr == '>' )
	{
	  name[j] = CNULL;
	  arrow = 1;
	  break;
	}
      else
	name[j] = *cptr;
    }
  name[length] = CNULL;   /*...for when an arrow is NOT found. */
  cptr++;

  /* A specification like `name>' is illegal. */
  if ( arrow && (length=jip_int_count((JIP_INDEX)strlen(cptr))) == 0 )
    {
      jip_text(2,"No volume list is given following a `>' delimeter.");
      return(1);
    }

  if ( !arrow )
    *list_name = CNULL;
  else
    sscanf(cptr,"%s",list_name);
  return(0);
}

int decode_subset( char *list_name, int ntotal, int *nset, int *iset )
{
  char *cptr, charN;
  int length, istart, iend;
  int j, jset;
  LOGICAL dash, last_dash;
  extern int io_increment;

  length = jip_int_count((JIP_INDEX)strlen( list_name ));
  /*
  ** Which elements are included in the list?  If the list name is NULL,
  ** then the full set is selected; otherwise, decode the list.
  */
  if ( length == 0 )
    {
      /* Include all elements in the list. */
      *nset = ntotal;
      for ( jset=0; jset<*nset; jset++ )
	iset[jset] = jset;
      return(0);            /* Made it with no errors! */
    }
  /*
  ** Decode the volume list.
  */
  *nset = dash = last_dash = 0;
  /*
  ** Go through each character in the list.
  */
  for ( j=0, cptr=list_name; j<=length; j++ )
    {
      /* Evaluate preceeding substrings when `,', `-', and end are found. */
      if ( list_name[j]==',' || list_name[j]=='-' || j==length )
	{
	  if ( list_name[j]=='-' )
	    dash = 1;
	  else
	    dash = 0;
	  list_name[j] = CNULL;   /* Replace the non-alpha with a NULL. */
	  if ( (sscanf(cptr,"%c",&charN))==1 && (charN=='n'||charN=='N') )
	    iset[*nset] = ntotal - 1;
	  else if ( (sscanf(cptr,"%c",&charN))==1 && (charN=='z'||charN=='Z') )
	    iset[*nset] = ntotal - 1;
	  else if ( (sscanf(cptr,"%c",&charN))==1 && (charN=='y'||charN=='Y') )
	    iset[*nset] = ntotal - 2;
	  else if ( (sscanf(cptr,"%c",&charN))==1 && (charN=='x'||charN=='X') )
	    iset[*nset] = ntotal - 3;
	  else if ( sscanf(cptr,"%d",&iset[*nset]) != 1 )
	    {
	      jip_text(2,"A bad syntax was used in the list string.");
	      return(1);
	    }
	  /*
	  ** "io-increment" is set using the "set" command and simply adds to
	  ** an explicitly defined set list.  This simplies repeated steps on a
	  ** stack that is split into multiple steps due to memory limitations.
	  */
	  iset[*nset] += io_increment;
	  *nset = *nset + 1;
	  cptr = list_name + j + 1;
	  /* Was the last non-alpha (before the current one) a dash? */ 
	  if ( last_dash )
	    {
	      /* Do not allow a syntax like `-4'. */
	      if ( *nset < 2 )
		{
		  jip_text(2,"bad syntax used in the list string.");
		  return(1);
		}
	      else
		{
		  /* Fill in intermediate numbers for a `-' (dash) character.*/
		  *nset  = *nset - 2;
		  istart = iset[*nset];
		  iend   = iset[*nset+1];
		  if ( iend >= istart )
		    {
		      for ( jset=istart; jset<=iend; jset++ )
			{
			  iset[*nset] = jset;
			  *nset = *nset + 1;
			}
		    }
		  else
		    {
		      for ( jset=istart; jset>=iend; jset-- )
			{
			  iset[*nset] = jset;
			  *nset = *nset + 1;
			}
		    }
		}
	    }
	  last_dash = dash;
	}
    }
  /*
  ** Lastly, make sure that all volume numbers in the list are within range.
  */
      
  if ( *nset > MAX_NSET )
    {
      jip_text(3,"The # of volumes exceeds the program dimension MAX_NSET.");
      return(1);
    }
  for ( jset=0; jset<*nset; jset++ )
    {
      if ( iset[jset] < 0 || iset[jset] >= ntotal )
	{
	  jip_text(2,"The list contains a number which is out of range.");
	  return(1);
	}
    }
  return(0);        /* Made it with no errors! */
}

int decode_comparison_list( char *argument )
{
  int decode_volume_list();
  char volume_comp_name[LEN_STR], *volume_list_name, *next_non_white();

  /*
  ** Pick off the name of the first volume or stack.
  */
  if ( sscanf(argument,"%s",volume_comp_name) != 1 )
    {
      jip_text(2,"At least one argument is mandatory here.");
      return(1);
    }

  /*
  ** Decode the 1st volume name with the normal routine.  This creates a
  ** comparison volume list, which should contain just one volume (this is
  ** checked elsewhere).
  */
  if ( decode_volume_list(1,volume_comp_name) ) return(1);

  /* Update the argument to the next string position. */
  volume_list_name = next_non_white( argument + jip_int_count((JIP_INDEX)strlen(volume_comp_name)) );

  /* Decode the rest of the argument as the main volume list. */
  if ( decode_volume_list(0,volume_list_name) )
    return(1);
  else
    return(0);
}

int decode_string_and_comparison_list(char *argument,char *string )
{
  int decode_volume_list();
  char *next_non_white();
  char *remaining_argument, *cptr;

  /*
  ** Pick off the first string.
  */
  if ( sscanf(argument,"%s",string) != 1 )
    {
      jip_text(2,"A name for the volume to create is mandatory here.");
      return(1);
    }
  
  /*
  ** Make sure that the stack name does not contain a '>'
  ** since these characters are reserve for specifying volume lists within
  ** a stack.
  */
  for ( cptr = string; *cptr != CNULL; cptr++ )
    {
      if ( *cptr == '>' )
	{
	  jip_text(2,"Do not use > in a stack name.");
	  return(1);
	}
    }

  /* Update the argument to the next string position. */
  remaining_argument = next_non_white( argument + jip_int_count((JIP_INDEX)strlen(string)) );

  /* Decode the rest of the argument as the main volume list. */
  if ( decode_comparison_list(remaining_argument) )
    return(1);
  else
    return(0);
}

int decode_int_and_comparison_list(char *argument, int *number)
{
  int decode_volume_list();
  char string[LEN_STR];
  char *next_non_white();
  char *remaining_argument;

  /*
  ** Pick off the first string.
  */
  if ( sscanf(argument,"%s",string) != 1 )
    {
      jip_text(2,"An integer is a mandatory argument here.");
      return(1);
    }
  sscanf(argument,"%d",number);
  
  /* Update the argument to the next string position. */
  remaining_argument = next_non_white( argument + jip_int_count((JIP_INDEX)strlen(string)) );

  /* Decode the rest of the argument as the main volume list. */
  if ( decode_comparison_list(remaining_argument) )
    return(1);
  else
    return(0);
}

int decode_read_argument( char *argument, char *file_name, int *file_type,
			  char *stack_name, char *list_name, int *nread )
{
  char file_and_list_name[LEN_STR], extension[LEN_STR], file_root[LEN_STR];
  char *cptr;
  int number_arguments;

  *file_type = -1;   /* initialization */
  *nread = 0;
  if ((number_arguments=sscanf(argument,"%s %s %d",file_and_list_name,
			       stack_name,nread))
      <= 0 )
    {
      jip_text(2,"No argument to read command.");
      return(1);
    }
  else
    {
      separate_name_and_list( file_and_list_name, file_name, list_name );
      strcpy(file_root,file_name);
      if ( file_name[jip_int_count((JIP_INDEX)strlen(file_name))-1] == '_' ) // no extension, file ends with "_"
	*file_type = FREESURFER;
      else if ( (cptr=strrchr(file_root,'.')) )
	{ // file has an extension
	  *cptr = CNULL;
	  sscanf(cptr+1,"%s",extension);
	  // Evaluate the file based upon its extension.
	  if ( !strcmp(extension,"com") )         *file_type = JIP_COM;
	  else if ( !strcmp(extension,"bfloat") ) *file_type = BFLOAT;
	  else if ( !strcmp(extension,"bshort") ) *file_type = BSHORT;
	  else if ( !strcmp(extension,"ushort") ) *file_type = USHORT;
	  else if ( !strcmp(extension,"blong") )  *file_type = BLONG;
	  else if ( !strcmp(extension,"nii") )    *file_type = NIFTI_NII;
	  else if ( !strcmp(extension,"img") )    *file_type = NIFTI_IMG;
	  else if ( !strcmp(extension,"ovl") )    *file_type = OVL;
	}
      else
	*file_type = -1; // unknown

      if ( number_arguments == 1 ) strcpy(stack_name,file_root);
    }

  /*
  ** Make sure that the stack name does not contain a '>'
  ** since these characters are reserved for specifying volume lists within
  ** a stack.
  */
  for ( cptr = stack_name; *cptr != CNULL; cptr++ )
    {
      if ( *cptr == '>' )
	{
	  jip_text(2,"Do not use > in a stack name.");
	  return(1);
	}
    }

  return(0);
}

int decode_read_special( char *argument, char *file_name, char *stack_name )
{
  int decode_string_and_list();
  char *next_non_white(), *remaining_argument, *cptr;

  /*
  ** Pick off the number first.
  */
  if ( sscanf(argument,"%s",file_name) != 1 )
    {
      jip_text(0,"This command requires at least one argument.");
      jip_text(2,"read-GE [file name root] <stack name>.");
      return(1);
    }
  /* Update the argument to the next string position. */
  remaining_argument = next_non_white( argument + jip_int_count((JIP_INDEX)strlen(file_name)) );

  if ( sscanf(remaining_argument,"%s",stack_name) != 1 )
    strcpy(stack_name,file_name);
  else
    {
      /*
      ** Make sure that the stack name does not contain a '>'
      ** since these characters are reserved for specifying volume lists within
      ** a stack.
      */
      for ( cptr = stack_name; *cptr != CNULL; cptr++ )
	{
	  if ( *cptr == '>' )
	    {
	      jip_text(2,"Do not use > in a stack name.");
	      return(1);
	    }
	}
    }
  return(0);
}

int decode_string_and_list( int istring, char *argument, char *string )
     // istring: 0 = stack name, 1 = file name, 2 = parameter name
{
  int decode_volume_list();
  char *next_non_white();
  char *volume_list_name, *cptr;

  /*
  ** Pick off the name of the stack, which will then be created.
  */
  if ( sscanf(argument,"%s",string) != 1 )
    {
      if ( istring == 0 )
	{
	  jip_text(2,"A name for the volume to create is mandatory here.");
	  return(1);
	}
      else if ( istring == 1 )
	{
	  jip_text(2,"A file name root is mandatory here.");
	  return(1);
	}
      else
	{
	  jip_text(2,"A parameter name is mandatory here.");
	  return(1);
	}
    }
  
  if ( istring == 0 )
    {
      /*
      ** Make sure that the stack name does not contain a '>'
      ** since these characters are reserved for specifying volume lists within
      ** a stack.
      */
      for ( cptr = string; *cptr != CNULL; cptr++ )
	{
	  if ( *cptr == '>' )
	    {
	      jip_text(2,"Do not use > in a stack name.");
	      return(1);
	    }
	}
    }

  /* Update the argument to the next string position. */
  volume_list_name = next_non_white( argument + jip_int_count((JIP_INDEX)strlen(string)) );

  /* Decode the rest of the argument as the main volume list. */
  if ( decode_volume_list(0,volume_list_name) )
    return(1);
  else
    return(0);
}

int decode_int_and_list( char *argument, int *number )
{
  int decode_string_and_list();
  char string[LEN_STR];
  char *next_non_white();
  char *remaining_argument;

  /*
  ** Pick off the integer first.
  */
  if ( sscanf(argument,"%s",string) != 1 )
    {
      jip_text(2,"An integer is a mandatory argument here.");
      return(1);
    }
  sscanf(argument,"%d",number);
  
  /* Update the argument to the next string position. */
  remaining_argument = next_non_white( argument + jip_int_count((JIP_INDEX)strlen(string)) );

  /* Decode the rest of the argument as the main volume list. */
  if ( decode_volume_list(0,remaining_argument) )
    return(1);
  else
    return(0);
}

int decode_int_string_and_list( int istring, char *argument, int *number, char *string )
{
  int decode_string_and_list();
  char *next_non_white();
  char *remaining_argument;

  /*
  ** Pick off the number first.
  */
  if ( sscanf(argument,"%s",string) != 1 )
    {
      jip_text(2,"A name for the volume to create is mandatory here.");
      return(1);
    }
  else
    {
      sscanf(argument,"%d",number);
      /*
      if ( *number < 0 || *number > 100000 )
      {
      jip_text(2,"The number on the command line is missing or out of range.");
      return(1);
      }
      */
    }
  
  /* Update the argument to the next string position. */
  remaining_argument = next_non_white( argument + jip_int_count((JIP_INDEX)strlen(string)) );

  /* Decode the rest of the argument as the main volume list. */
  if ( decode_string_and_list(istring,remaining_argument,string) )
    return(1);
  else
    return(0);
}

int decode_2strings_and_list( int istring, char *argument, char *string1, char *string2 )
{
  int decode_string_and_list();
  char *next_non_white();
  char *remaining_argument;

  /*
  ** Pick off the number first.
  */
  if ( sscanf(argument,"%s",string1) != 1 )
    {
      jip_text(2,"A name for the volume to create is mandatory here.");
      return(1);
    }

  /* Update the argument to the next string position. */
  remaining_argument = next_non_white( argument + jip_int_count((JIP_INDEX)strlen(string1)) );

  /* Decode the rest of the argument as the main volume list. */
  if ( decode_string_and_list(istring,remaining_argument,string2) )
    return(1);
  else
    return(0);
}

int decode_real_string_and_list( char *argument, float *number, char *string )
{
  int decode_string_and_list();
  char *next_non_white();
  char *remaining_argument;

  /*
  ** Pick off the number first.
  */
  if ( sscanf(argument,"%s",string) != 1 )
    {
      jip_text(2,"A name for the volume to create is mandatory here.");
      return(1);
    }
  else
    sscanf(argument,"%f",number);
  
  /* Update the argument to the next string position. */
  remaining_argument = next_non_white( argument + jip_int_count((JIP_INDEX)strlen(string)) );

  /* Decode the rest of the argument as the main volume list. */
  if ( decode_string_and_list(0,remaining_argument,string) )
    return(1);
  else
    return(0);
}

int decode_real_string_and_comparison_list( char *argument, float *number, char *string )
{
  int decode_string_and_comparison_list();
  char *next_non_white();
  char *remaining_argument;

  /*
  ** Pick off the number first.
  */
  if ( sscanf(argument,"%s",string) != 1 )
    {
      jip_text(2,"A name for the volume to create is mandatory here.");
      return(1);
    }
  else
    sscanf(argument,"%f",number);
  
  /* Update the argument to the next string position. */
  remaining_argument = next_non_white( argument + jip_int_count((JIP_INDEX)strlen(string)) );

  /* Decode the rest of the argument as the main volume list. */
  if ( decode_string_and_comparison_list(remaining_argument,string) )
    return(1);
  else
    return(0);
}

int decode_real_and_list( char *argument, float *real_number )
{
  int decode_string_and_list();
  char string[LEN_STR];
  char *next_non_white();
  char *remaining_argument;

  /*
  ** Pick off the real number first.
  */
  if ( sscanf(argument,"%s",string) != 1 )
    {
      jip_text(2,"A real number is a mandatory argument here.");
      return(1);
    }
  sscanf(argument,"%f",real_number);
  
  /* Update the argument to the next string position. */
  remaining_argument = next_non_white( argument + jip_int_count((JIP_INDEX)strlen(string)) );

  /* Decode the rest of the argument as the main volume list. */
  if ( decode_volume_list(0,remaining_argument) )
    return(1);
  else
    return(0);
}

int decode_euler_and_list( char *argument, float *euler )
{
  int decode_string_and_list(), j;
  char string[LEN_STR];
  char *next_non_white();
  char *remaining_argument;

  /*
  ** Pick off the 6 real numbers first.
  */
  remaining_argument = argument;
  for (j=0; j<6; j++)
    {
      if ( sscanf(remaining_argument,"%s",string) != 1 )
	{
	  jip_text(2,"A real number is a mandatory argument here.");
	  return(1);
	}
      sscanf(remaining_argument,"%f",euler+j);
      remaining_argument = next_non_white( remaining_argument + jip_int_count((JIP_INDEX)strlen(string)) );
    }

  /* Decode the rest of the argument as the main volume list. */
  if ( decode_volume_list(0,remaining_argument) )
    return(1);
  else
    return(0);
}

int decode_2reals_and_list( char *argument, float *real_number1, float *real_number2 )
{
  int decode_string_and_list();
  char string[LEN_STR];
  char *next_non_white();
  char *remaining_argument;

  /*
  ** Pick off the two real number first.
  */
  if ( sscanf(argument,"%s",string) != 1 )
    {
      jip_text(2,"2 real numbers are mandatory arguments here.");
      return(1);
    }
  sscanf(argument,"%f",real_number1);
  /* Update the argument to the next string position. */
  remaining_argument = next_non_white( argument + jip_int_count((JIP_INDEX)strlen(string)) );
  if ( sscanf(remaining_argument,"%s",string) != 1 )
    {
      jip_text(2,"2 real numbers are mandatory arguments here.");
      return(1);
    }
  sscanf(remaining_argument,"%f",real_number2);
  /* Update the argument to the next string position. */
  remaining_argument = next_non_white( remaining_argument + jip_int_count((JIP_INDEX)strlen(string)) );

  /* Decode the rest of the argument as the main volume list. */
  if ( decode_volume_list(0,remaining_argument) )
    return(1);
  else
    return(0);
}

int decode_4reals_string_and_list( char *argument, float *real_number1, float *real_number2,
				   float *real_number3, float *real_number4, char *string )
{
  int decode_string_and_list();
  char *next_non_white();
  char *remaining_argument;

  /*
  ** Pick off the three real number first.
  */
  if ( sscanf(argument,"%s",string) != 1 )
    {
      jip_text(2,"4 real numbers are mandatory arguments here.");
      return(1);
    }
  sscanf(argument,"%f",real_number1);
  /* Update the argument to the next string position. */
  remaining_argument = next_non_white( argument + jip_int_count((JIP_INDEX)strlen(string)) );
  if ( sscanf(remaining_argument,"%s",string) != 1 )
    {
      jip_text(2,"4 real numbers are mandatory arguments here.");
      return(1);
    }
  sscanf(remaining_argument,"%f",real_number2);
  /* Update the argument to the next string position. */
  remaining_argument = next_non_white( remaining_argument + jip_int_count((JIP_INDEX)strlen(string)) );
  if ( sscanf(remaining_argument,"%s",string) != 1 )
    {
      jip_text(2,"4 real numbers are mandatory arguments here.");
      return(1);
    }
  sscanf(remaining_argument,"%f",real_number3);
  /* Update the argument to the next string position. */
  remaining_argument = next_non_white( remaining_argument + jip_int_count((JIP_INDEX)strlen(string)) );
  if ( sscanf(remaining_argument,"%s",string) != 1 )
    {
      jip_text(2,"4 real numbers are mandatory arguments here.");
      return(1);
    }
  sscanf(remaining_argument,"%f",real_number4);
  /* Update the argument to the next string position. */
  remaining_argument = next_non_white( remaining_argument + jip_int_count((JIP_INDEX)strlen(string)) );

  /* Decode the rest of the argument as the main volume list. */
  if ( decode_string_and_list(0,remaining_argument,string) )
    return(1);
  else
    return(0);
}

int decode_3reals_and_list( char *argument, float *real_number1, float *real_number2, float *real_number3 )
{
  int decode_string_and_list();
  char string[LEN_STR];
  char *next_non_white();
  char *remaining_argument;

  /*
  ** Pick off the three real number first.
  */
  if ( sscanf(argument,"%s",string) != 1 )
    {
      jip_text(2,"3 real numbers are mandatory arguments here.");
      return(1);
    }
  sscanf(argument,"%f",real_number1);
  /* Update the argument to the next string position. */
  remaining_argument = next_non_white( argument + jip_int_count((JIP_INDEX)strlen(string)) );
  if ( sscanf(remaining_argument,"%s",string) != 1 )
    {
      jip_text(2,"3 real numbers are mandatory arguments here.");
      return(1);
    }
  sscanf(remaining_argument,"%f",real_number2);
  /* Update the argument to the next string position. */
  remaining_argument = next_non_white( remaining_argument + jip_int_count((JIP_INDEX)strlen(string)) );
  if ( sscanf(remaining_argument,"%s",string) != 1 )
    {
      jip_text(2,"3 real numbers are mandatory arguments here.");
      return(1);
    }
  sscanf(remaining_argument,"%f",real_number3);
  /* Update the argument to the next string position. */
  remaining_argument = next_non_white( remaining_argument + jip_int_count((JIP_INDEX)strlen(string)) );

  /* Decode the rest of the argument as the main volume list. */
  if ( decode_volume_list(0,remaining_argument) )
    return(1);
  else
    return(0);
}

int decode_3reals_string_and_list( char *argument, float *real_number1, float *real_number2,
				   float *real_number3, char *string )
{
  int decode_string_and_list();
  char *next_non_white();
  char *remaining_argument;

  /*
  ** Pick off the three real number first.
  */
  if ( sscanf(argument,"%s",string) != 1 )
    {
      jip_text(2,"3 real numbers are mandatory arguments here.");
      return(1);
    }
  sscanf(argument,"%f",real_number1);
  /* Update the argument to the next string position. */
  remaining_argument = next_non_white( argument + jip_int_count((JIP_INDEX)strlen(string)) );
  if ( sscanf(remaining_argument,"%s",string) != 1 )
    {
      jip_text(2,"3 real numbers are mandatory arguments here.");
      return(1);
    }
  sscanf(remaining_argument,"%f",real_number2);
  /* Update the argument to the next string position. */
  remaining_argument = next_non_white( remaining_argument + jip_int_count((JIP_INDEX)strlen(string)) );
  if ( sscanf(remaining_argument,"%s",string) != 1 )
    {
      jip_text(2,"3 real numbers are mandatory arguments here.");
      return(1);
    }
  sscanf(remaining_argument,"%f",real_number3);
  /* Update the argument to the next string position. */
  remaining_argument = next_non_white( remaining_argument + jip_int_count((JIP_INDEX)strlen(string)) );

  /* Decode the rest of the argument as the main volume list. */
  if ( decode_string_and_list(0,remaining_argument,string) )
    return(1);
  else
    return(0);
}

int decode_2ints_string_and_list( char *argument, int *number1, int *number2, char *string )
{
  int decode_string_and_list();
  char *next_non_white();
  char *remaining_argument;

  /*
  ** Pick off the 2 real numbers first.
  */
  if ( sscanf(argument,"%s",string) != 1 )
    {
      jip_text(2,"2 integers are mandatory arguments here.");
      return(1);
    }
  sscanf(argument,"%d",number1);
  /* Update the argument to the next string position. */
  remaining_argument = next_non_white( argument + jip_int_count((JIP_INDEX)strlen(string)) );
  if ( sscanf(remaining_argument,"%s",string) != 1 )
    {
      jip_text(2,"2 integers are mandatory arguments here.");
      return(1);
    }
  sscanf(remaining_argument,"%d",number2);
  /* Update the argument to the next string position. */
  remaining_argument = next_non_white( remaining_argument + jip_int_count((JIP_INDEX)strlen(string)) );

  /* Decode the rest of the argument as the main volume list. */
  if ( decode_string_and_list(0,remaining_argument,string) )
    return(1);
  else
    return(0);
}

int read_slice_order( char *argument )
{
  char *next_non_white();
  char *cptr, number[LEN_STR];
  int ivalue;JIP_INDEX index;

  /*
  ** Re-intialize the volume order.
  */
  if ( nslice_order != 0 )
    {
      free_int_vector(slice_order);
      nslice_order = 0;
      slice_order = NULL;
    }

  /*
  ** Go through the array list and determine the number of values.
  */
  nslice_order = 0;
  cptr = argument;
  while ( sscanf(cptr,"%s",number) == 1 )
    {
      if ( sscanf(cptr,"%d",&ivalue) != 1 ) break;
      nslice_order++;
      cptr = next_non_white( cptr + jip_int_count((JIP_INDEX)strlen(number)) );
    }
  if ( nslice_order < 2 )
    {
      nslice_order = 0;
      jip_text(0,"Fewer than 2 values were successfully read; slice_order = NULL.");
      return(1);
    }
  /*
  ** Allocate space for the map array.
  */
  slice_order = int_vector(nslice_order);
  /*
  ** Read the input array values a second time, and this time store them.
  */
  for ( index=0, cptr=argument; index<nslice_order; index++ )
    {
      sscanf(cptr,"%d",slice_order+index);
      sscanf(cptr,"%s",number);
      cptr = next_non_white( cptr + jip_int_count((JIP_INDEX)strlen(number)) );
    }
  fprintf(stdout,"%d values were read for the slice order.\n",nslice_order);

  return(0);
}

int read_map_array( char *argument )
{
  char *next_non_white();
  char *cptr, number[LEN_STR];
  JIP_INDEX index;
  float value;

  /*
  ** Re-intialize the map array.
  */
  if ( nmap_array != 0 )
    {
      free_float_vector(map_array);
      nmap_array = 0;
    }

  /*
  ** Go through the array list and determine the number of values.
  */
  nmap_array = 0;
  cptr = argument;
  while ( sscanf(cptr,"%s",number) == 1 )
    {
      if ( sscanf(cptr,"%f",&value) != 1 ) break;
      nmap_array++;
      cptr = next_non_white( cptr + jip_int_count((JIP_INDEX)strlen(number)) );
    }
  if ( nmap_array < 2 )
    {
      nmap_array = 0;
      jip_text(2,"Fewer than 2 values were successfully read.");
      return(1);
    }
  /*
  ** Allocate space for the map array.
  */
  map_array = float_vector(nmap_array);
  /*
  ** Read the input array values a second time, and this time store them.
  */
  for ( index=0, cptr=argument; index<nmap_array; index++ )
    {
      sscanf(cptr,"%f",map_array+index);
      sscanf(cptr,"%s",number);
      cptr = next_non_white( cptr + jip_int_count((JIP_INDEX)strlen(number)) );
    }
  fprintf(stdout,"%d values were read for the map array.\n",nmap_array);

  return(0);
}

struct data_stack *install_stack( char *stack_name, struct data_stack *stack_template,
				    IPOINT3D dim, int nt )
     // Create a new stack named stack_name with dimensions dim and time points nt
     // Use stack_template as a template to copy additional information.
{
  struct data_stack *stack, *found_stack, *last_stack;
  int nstack;
  BOOL found;

  /*
  ** Compare all existing stack names against the input name.
  */
  found = FALSE;
  for ( stack=stack_root, nstack=0; stack; stack=stack->next, nstack++ )
    {
      if ( !(strcmp(stack_name,stack->name)) && !found )
	{
	  found_stack = stack;
	  found = TRUE;
	}
      last_stack = stack; // this is the last stack in memory
    }

  if ( found )
    {
      stack = found_stack;
      // If a stack was found, free the data allocation.
      if ( stack->SpaceTime.fcData != NULL )
	free_fcomplex_vector(stack->SpaceTime.fcData);
    }
  else
    {
      // If the name was not found (and thus this is a new stack to add to the
      // linked list of stacks), then allocate room for a stack.
      if ( (stack=(struct data_stack *)malloc(sizeof(struct data_stack))) )
	{
	  strcpy( stack->name, stack_name );
	  strcat( stack->name,"\0");
	}
      else
	jip_text(3,"Out of memory in routine `install_stack'.");
      // If this is a new stack (end of chain), then the next one is undefined.
      stack->next = NULL;
      if ( nstack == 0 )
	stack_root = stack;       // The first stack is the root
      else
	last_stack->next = stack; // The last stack points to this new one at the end of the chain.
    }

  stack->dim = dim;
  stack->nt  = nt;
  stack->n2d = jip_product(stack->dim.x,stack->dim.y);
  stack->n3d = jip_product(stack->n2d,stack->dim.z);
  stack->n4d = jip_product(stack->n3d,stack->nt);
  stack->npts_volume = jip_product(2,stack->n3d);
  stack->data_type = 1; // real, imag by default
  stack->SpaceTime.fcData = fcomplex_vector(stack->n4d);
  // If a template stack was given, use this to get the resolution, ...
  if ( stack_template != 0 )
    {
      stack->resolution = stack_template->resolution;
      stack->resolution_t = stack_template->resolution_t;
      stack->origin     = stack_template->origin;
      stack->direction  = stack_template->direction;
      stack->status     = stack_template->status;
      stack->registration = stack_template->registration;
      stack->lDOF = stack_template->lDOF;
    }
  else
    {
      stack->resolution.x = stack->resolution.y = stack->resolution.z = stack->resolution_t = 1.;
      stack->origin.x     = stack->origin.y     = stack->origin.z     = 0.;
      stack->direction.x  = stack->direction.y  = stack->direction.z  = 1.;
      stack->status = 0;
      stack->registration = 0;
      stack->lDOF = 0;
    }

  return( stack );
}

void free_stack( struct data_stack *free_me )
{
  struct data_stack *stack, *last_stack;

  for ( stack=stack_root; stack; stack=stack->next )
    {
      if ( stack == free_me )
	{
	  if ( stack == stack_root ) 
	    /* If the root is be freed, move the root address to next stack. */
	    stack_root = stack->next;
	  else
	    /* For other stacks, leapfrog the stack in the linked list. */
	    last_stack->next = stack->next;
	  /*
	  ** If memory has been allocated to pointers within the stack, free
	  ** it first, and then free the memory for the stack structure.
	  */
	  if ( stack->SpaceTime.fcData != NULL )
	    free_fcomplex_vector(stack->SpaceTime.fcData);
	  free( (char *)stack );
	  break;
	}
      else
	last_stack = stack;
    }
}

void jip_text(int type, char *text)
{
  if (type==0)
    fprintf(stdout,"%s\n",text);
  else if (type==1)
    fprintf(stderr,"Warning: %s\n",text);
  else
    {
      if (type>2 || fatal_error==1)
	{
	  fprintf(stderr,"Fatal error: %s\n",text);
	  fprintf(stderr,"Program aborted.\n");
	  exit(1);
	}
      else
	fprintf(stderr,"Error: %s\n",text);
    }
}

void print_help_page()
{
  char doc_file_name[LEN_STR], line[LEN_STR];
  int nline, ncontrol, nskip, nprint, icontrol, ilevel;
  int con[3], control_requested, input, j;
  int *control, *control_line;
  FILE *doc_file;

  strcpy(doc_file_name,"/homes/1/jbm/doc/volume/volume_doc_new.txt");
  if ( !(doc_file = fopen(doc_file_name,"r")) )
    {
      sprintf(error_string,"I cannot open the documentation file \n%s.",
	      doc_file_name);
      jip_text(2,error_string);
      jip_text(0,"E-mail Joe Mandeville about this.");
      exit(1);
    }

  /*
  ** Go through the documentation file and find all direction codes.
  */
  nline = ncontrol = 0;
  while ( get_line_with_comment(doc_file, line, LEN_STR) != NULL )
    {
      nline++;
      if ( line[0] == '!' )
	{
	  if ( sscanf(line+1,"%d %d %d",con+2,con+1,con) != 3 )
	    {
	      sprintf(error_string,"Error reading the direction integers on line %d",nline);
	      jip_text(2,error_string);
	      jip_text(0,"E-mail Joe Mandeville about this.");
	      fclose(doc_file);
	      return;
	    }
	  else
	    ncontrol++;
	}
    }
  ncontrol++;       /* This hold the end of the file. */

  /*
  ** Define the control integer array.
  */
  jip_seek(doc_file,0L,0);              /* Rewind the file. */
  control      = int_vector(ncontrol);
  control_line = int_vector(ncontrol);
  nline = ncontrol = 0;
  while ( get_line_with_comment(doc_file, line, LEN_STR) != NULL )
    {
      nline++;
      if ( line[0] == '!' )
	{
	  if ( sscanf(line+1,"%d %d %d",con+2,con+1,con) != 3 )
	    jip_text(3,"Programming error #1 in function print_help_page.");
	  else
	    {
	      control[ncontrol] = con[2]*10000 + con[1]*100 + con[0];
	      control_line[ncontrol] = nline;
	      ncontrol++;
	    }
	}
    }
  control[ncontrol] = (con[2]+1)*10000;
  control_line[ncontrol] = nline+1;
  ncontrol++;
  /*
  ** Begin help "menu mode".
  */
  icontrol = ilevel = 0;
  while (1)
    {
      jip_seek(doc_file,0L,0);              /* Rewind the file. */
      con[0] = control[icontrol]/10000;
      con[1] = (control[icontrol] - con[0]*10000) / 100;
      con[2] =  control[icontrol] - con[0]*10000 - con[1]*100;
      if ( con[0] == 0 )
	ilevel = 0;
      else if ( con[1] == 0 )
	ilevel = 1;
      else
	ilevel = 2;
/*
      printf("icontrol = %d, ilevel = %d\n",icontrol,ilevel);
      printf("control[icontrol] (%d %d %d)= %d\n",
	     con[0],con[1],con[2],control[icontrol]);
*/
      nskip = control_line[icontrol];
      nprint = control_line[icontrol+1]
	     - control_line[icontrol] - 1;
      for (j=0; j<nskip; j++)
	{
	  if ( get_line_with_comment(doc_file, line, LEN_STR) == NULL )
	    jip_text(3,"problem reading file #1\n");
	}
      for (j=0; j<nprint; j++)
	{
	  if ( get_line_with_comment(doc_file, line, LEN_STR) == NULL )
	    jip_text(3,"problem reading file #2\n");
	  else
	    if ( line[0] != 12 ) printf("%s",line);
	}
      printf("             _________________________________________________________ \n");
      printf("            |  q = quit     m = menu          n = next       b = back |\n");
      printf("            |__________________________<return> = next________________|\n");
      printf("volume help >>");
      get_line_with_comment(stdin,line,LEN_STR);
      sscanf(line,"%d",&input);
      switch ( line[0] )
	{
	case 'q':             /* quit help */
	  fclose(doc_file);
	  return;
	  break;
	  //	case NULL:            /* next */
	case 'n':
	  icontrol++;
	  if ( icontrol > ncontrol-2 ) icontrol = ncontrol-2;
	  break;
	case 'b':             /* back */
	  icontrol--;
	  if ( icontrol < 0 ) icontrol = 0;
	  break;
	case 'm':             /* menu */
	  if ( con[ilevel] == 0 ) ilevel--;
	  if ( ilevel < 0 ) ilevel = 0;
	  for (j=ilevel; j<3; j++)
	    con[j] = 0;
	  control_requested = con[0]*10000 + con[1]*100 + con[2];
	  icontrol = -1;
	  for (j=0; j<ncontrol; j++)
	    if ( control[j] == control_requested )
	      icontrol = j;
	  if ( icontrol == -1 )
	    jip_text(3,"Programming error #2 in function print_help_page.");
	  break;
	default:              /* integer selection */
	  con[ilevel] = input;
	  control_requested = con[0]*10000 + con[1]*100 + con[2];
	  icontrol = -1;
	  for (j=0; j<ncontrol; j++)
	    if ( control[j] == control_requested )
	      icontrol = j;
	  if ( icontrol == -1 )
	    jip_text(2,"The specified option is not in the menu.");
	  break;
	}
    }
}
