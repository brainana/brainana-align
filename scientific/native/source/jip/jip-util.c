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
#include <string.h>
#include <math.h>
#include "util.h"
#include "jip.h"

extern char error_string[LEN_STR];

/*
** Changes desired to reduce memory requirements:
** convert-to-derivative - Copy only each time vector.
*/


JIP_INDEX index2d(int ix, int iy, struct volume_list *volume)
{
  return((JIP_INDEX)iy * volume->dim.x + ix);
}

JIP_INDEX index3d(int ix, int iy, int iz, struct volume_list *volume)
{
  return((JIP_INDEX)iz * volume->n2d + ((JIP_INDEX)(iy)) * volume->dim.x + ix);
}

JIP_INDEX index4d(int ix, int iy, int iz, int it, struct data_stack *stack)
{
  return((JIP_INDEX)it * stack->n3d + iz * stack->n2d + ((JIP_INDEX)(iy)) * stack->dim.x + ix);
}

float volume_amp( struct volume_list *volume, JIP_INDEX index )
{
  struct data_stack *stack;
  float amp;

  stack=volume->stack;
  // The stack may be null, in principle
  if ( stack && (stack->status&MAG_PHASE_BIT || stack->status&MAP_BIT) )
    amp = volume->SpaceTime.fcData[index].real;
  else
    amp = sqrt((double)(volume->SpaceTime.fcData[index].real*volume->SpaceTime.fcData[index].real +
			volume->SpaceTime.fcData[index].imag*volume->SpaceTime.fcData[index].imag ));
  return( amp );
}

float volume_phase( struct volume_list *volume, JIP_INDEX index )
{
  struct data_stack *stack;
  float real, imag, amp, phase;

  stack = volume->stack;
  // The stack may not be null, principal
  if ( stack && (stack->status&MAG_PHASE_BIT || stack->status&MAP_BIT) )
    {
      phase = volume->SpaceTime.fcData[index].imag;
      return(phase);
    }
  else
    {
      real = volume->SpaceTime.fcData[index].real;
      imag = volume->SpaceTime.fcData[index].imag;
    }
  polar_from_complex(real, imag, &amp, &phase);
  return( phase );
}	      

float stack_amp( struct data_stack *stack, JIP_INDEX index )
{
  float amp;

  if ( stack->status&MAG_PHASE_BIT || stack->status&MAP_BIT )
    amp = stack->SpaceTime.fcData[index].real;
  else
    amp = sqrt((double)(stack->SpaceTime.fcData[index].real*stack->SpaceTime.fcData[index].real +
			stack->SpaceTime.fcData[index].imag*stack->SpaceTime.fcData[index].imag ));
  return( amp );
}

void polar_from_complex(float real, float imag, float *amp, float *phase)
{
  double ratio;

  *amp = sqrt((double)(real*real + imag*imag));
  if ( real == 0. )
    {
      if ( imag == 0. )
	*phase = 0.;
      else if ( imag > 0. )
	*phase =  PI/2.;
      else
	*phase = -PI/2.;
    }
  else
    {
      ratio = imag / real;
      *phase = atan( ratio );
      if ( real  < 0. )  *phase += PI;
      if ( *phase > PI ) *phase -= 2.*PI;
      /*      if ( real  < 0. )  *phase = PI - *phase; */
    }
}

void delete_stacks()
{
  struct data_stack *stack, *stack_save;
  struct volume_list  *volume;
  extern struct volume_list *volume_list_root;

  /* Cycle through the volume list, beginning with the root. */
  stack_save = 0;
  for ( volume=volume_list_root; volume; volume=volume->next )
    {
      if ( (stack=volume->stack) == stack_save ) continue;
      stack_save = stack;
      free_stack( stack );
    }
}

void show_stacks()
{
  char type[LEN_STR];
  float min, max, test;
  JIP_INDEX j;
  struct data_stack *stack, *stack_save;
  struct volume_list  *volume;
  extern struct volume_list *volume_list_root;

  /* Cycle through the volume list, beginning with the root. */
  printf("stack name\r\t\t# x\r\t\t\t# y\r\t\t\t\t# z\r\t\t\t\t\t# t\r");
  printf("\t\t\t\t\t\ttype\r\t\t\t\t\t\t\t\trange\n");
  printf("__________\r\t\t___\r\t\t\t___\r\t\t\t\t___\r\t\t\t\t\t___\r");
  printf("\t\t\t\t\t\t____\r\t\t\t\t\t\t\t\t_____\n");
  stack_save = 0;
  for ( volume=volume_list_root; volume; volume=volume->next )
    {
      if ( (stack=volume->stack) == stack_save ) continue;
      stack_save = stack;

      if ( stack->status & MAP_BIT )
	strcpy(type,"map      ");
      else
	{
	  if ( stack->status&MAG_PHASE_BIT )
	    strcpy(type,"mag-phase");
	  else
	    strcpy(type,"real-imag");
	}

      /* Determine the min and max for the entire file. */
      min =  1.e10;
      max = -1.e10;
      for ( j=0; j<stack->n4d; j++ )
	{
	  if ( (test=stack_amp(stack,j)) < min ) min = test;
	  if (  test                     > max ) max = test;
	}
      printf("%s\r\t\t%d\r\t\t\t%d\r\t\t\t\t%d\r\t\t\t\t\t%d\r\t\t\t\t\t\t%s\r\t\t\t\t\t\t\t\t%g\n",
	     stack->name,stack->dim.x,stack->dim.y,stack->dim.z,stack->nt,type,min);
      printf("\t\t%g\r\t\t\t%g\r\t\t\t\t%g\r\t\t\t\t\t%g\r\t\t\t\t\t\t\t\t%g\n",
	     stack->resolution.x,stack->resolution.y,stack->resolution.z,stack->resolution_t,max);
    }
}

void convert_complex()
{
  char error_string[LEN_STR];
  JIP_INDEX j;
  float amp, phase;
  struct data_stack *stack, *stack_save;
  struct volume_list  *volume;
  extern struct volume_list *volume_list_root;

  stack_save = 0;
  for ( volume=volume_list_root; volume; volume=volume->next )
    {
      if ( (stack=volume->stack) == stack_save ) continue;
      stack_save = stack;

      if ( stack->status&MAP_BIT )
	{
	  sprintf(error_string,"stack %s is a map!\n",stack->name);
	  jip_text(1,error_string);
	  continue;
	}
      else if ( stack->status&MAG_PHASE_BIT )
	{
	  /*
	  ** Convert polar complex data to real and imaginary.
	  */
	  for (j=0; j<stack->n4d; j++)
	    {
	      amp   = stack->SpaceTime.fcData[j].real;
	      phase = stack->SpaceTime.fcData[j].imag;
	      stack->SpaceTime.fcData[j].real = amp * cos((double)(phase*PI/180.));
	      stack->SpaceTime.fcData[j].imag = amp * sin((double)(phase*PI/180.));
	    }
	  stack->status = stack->status & ~MAG_PHASE_BIT;
	  sprintf(error_string,"`%s' was converted from polar complex data to complex data.\n",stack->name);
	  jip_text(0,error_string);
	}
      else
	{
	  /*
	  ** Convert real and imaginary data to polar complex data.
	  */
	  for (j=0; j<stack->n4d; j++)
	    {
	      polar_from_complex(stack->SpaceTime.fcData[j].real,stack->SpaceTime.fcData[j].imag,
				 &amp,&phase);
	      stack->SpaceTime.fcData[j].real = amp;
	      stack->SpaceTime.fcData[j].imag = phase*180./PI;
	    }
	  stack->status = stack->status | MAG_PHASE_BIT;
	  sprintf(error_string,"`%s' was converted from complex data to polar complex data.\n",stack->name);
	  jip_text(0,error_string);
	}
    }
}

void convert_complex_stack( struct data_stack *stack )
{
  char error_string[LEN_STR];
  JIP_INDEX j;
  float amp, phase;

  if ( stack->status&MAG_PHASE_BIT )
    {
      // Convert polar complex data to real and imaginary.
      for (j=0; j<stack->n4d; j++)
	{
	  amp   = stack->SpaceTime.fcData[j].real;
	  phase = stack->SpaceTime.fcData[j].imag;
	  stack->SpaceTime.fcData[j].real = amp * cos((double)(phase*PI/180.));
	  stack->SpaceTime.fcData[j].imag = amp * sin((double)(phase*PI/180.));
	}
      stack->status = stack->status & ~MAG_PHASE_BIT;
      sprintf(error_string,"`%s' was converted from polar complex data to complex data.\n",stack->name);
      jip_text(0,error_string);
    }
  else
    {
      // Convert real and imaginary data to polar complex data.
      for (j=0; j<stack->n4d; j++)
	{
	  polar_from_complex(stack->SpaceTime.fcData[j].real,stack->SpaceTime.fcData[j].imag,
			     &amp,&phase);
	  stack->SpaceTime.fcData[j].real = amp;
	  stack->SpaceTime.fcData[j].imag = phase*180./PI;
	}
      stack->status = stack->status | MAG_PHASE_BIT;
      sprintf(error_string,"`%s' was converted from complex data to polar complex data.\n",stack->name);
      jip_text(0,error_string);
    }
}

void convert_to_real()
{
  char error_string[LEN_STR];
  JIP_INDEX j;
  float amp, phase;
  struct data_stack *stack, *stack_save;
  struct volume_list  *volume;
  extern struct volume_list *volume_list_root;

  stack_save = 0;
  for ( volume=volume_list_root; volume; volume=volume->next )
    {
      if ( (stack=volume->stack) == stack_save ) continue;
      stack_save = stack;

      if ( stack->status&MAP_BIT )
	{
	  sprintf(error_string,"stack %s is a map!\n",stack->name);
	  jip_text(1,error_string);
	  continue;
	}
      else if ( stack->status&MAG_PHASE_BIT )
	{
	  /*
	  ** Convert polar complex data to (real,0).
	  */
	  for (j=0; j<stack->n4d; j++)
	    {
	      amp   = stack->SpaceTime.fcData[j].real;
	      phase = stack->SpaceTime.fcData[j].imag;
	      stack->SpaceTime.fcData[j].real = amp * cos((double)(phase*PI/180.));
	      stack->SpaceTime.fcData[j].imag = 0.;
	    }
	  stack->status = stack->status | MAG_PHASE_BIT;
	  sprintf(error_string,"`%s' was converted from polar complex data to signed polar complex data.\n",stack->name);
	  jip_text(0,error_string);
	}
      else
	{
	  /*
	  ** Convert real and imaginary data (real,0.).  Flag the
	  ** data as polar complex.
	  */
	  for (j=0; j<stack->n4d; j++)
	    stack->SpaceTime.fcData[j].imag = phase*180./PI;
	  stack->status = stack->status | MAG_PHASE_BIT;
	  sprintf(error_string,"`%s' was converted from complex data to signed polar complex data.\n",stack->name);
	  jip_text(0,error_string);
	}
    }
}

int copy_volumes( char *new_stack_name )
/*
** Copy a list of volumes, and put the result in a new stack with the
** name given by `new_stack_name'.
*/
{
  IPOINT3D dim;
  int nt, jt;JIP_INDEX jv, index;
  struct data_stack *stack, *new_stack;
  struct volume_list  *volume;
  extern struct volume_list *volume_list_root;

  /*
  ** Make sure that all volumes have the same dimensions.
  */
  nt = 0;
  for ( volume=volume_list_root; volume; volume=volume->next )
    {
      nt++;
      stack = volume->stack;
      /* Make sure all stacks have the same dimensions. */
      if ( volume == volume_list_root )
	dim = stack->dim;
      else
	{
	  if ( stack->dim.x != dim.x || stack->dim.y != dim.y || stack->dim.z != dim.z )
	    {
	      sprintf(error_string,"Stacks %s and %s have different dimensions.",
		      volume_list_root->stack->name,stack->name);
	      jip_text(2,error_string);
	      return(1);
	    }
	}
    }

  stack = volume_list_root->stack;
  /*
  ** Install the new_stack in memory.
  */
  new_stack = install_stack( new_stack_name, stack, dim, nt );
  /*
  ** Begin a loop over volumes.  Copy the data for each volume into the newly
  ** created volume stack.
  */
  for (volume=volume_list_root, jt=0; 
       volume;
       volume=volume->next, jt++)
    {
      for ( jv=0; jv<new_stack->n3d; jv++ )
	{
	  index  = jt * volume->n3d + jv;
	  new_stack->SpaceTime.fcData[index] = volume->SpaceTime.fcData[jv];
	}
    }
  printf("New stack %s contains %d volumes of dimensions (%d,%d,%d)\n",
	 new_stack->name,new_stack->nt,new_stack->dim.x,new_stack->dim.y,new_stack->dim.z);
  return(0);
}

int inconsistent_3d_dimensions( IPOINT3D *dim, int *nt )
{
  struct volume_list  *volume;
  extern struct volume_list *volume_list_root;

  //
  // Make sure that all volumes have the same dimensions.
  //
  for ( volume=volume_list_root, *nt = 0;
	volume;
	volume=volume->next, (*nt)++ )
    {
      // Make sure all stacks have the same dimensions.
      if ( volume == volume_list_root )
	{
	  dim->x = volume->dim.x;
	  dim->y = volume->dim.y;
	  dim->z = volume->dim.z;
	}
      else
	{
	  if ( volume->dim.x!=dim->x || volume->dim.y!=dim->y || volume->dim.z!=dim->z )
	    {
	      sprintf(error_string,"Stacks %s and %s have different dimensions.",
		      volume_list_root->stack->name,volume->stack->name);
	      jip_text(2,error_string);
	      return(1);
	    }
	}
    }
  return(0);
}
