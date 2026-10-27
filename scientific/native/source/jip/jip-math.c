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


int arithmetic_function_constant( int itype, float constant )
/*
** itype: 0) subtract
**        1) add
**        2) divide
**        3) multiply
**        4) ln
**        5) exp
*/
{
  JIP_INDEX j;
  struct data_stack *stack;
  struct volume_list  *volume;
  extern struct volume_list *volume_list_root;

  /*
  ** Do not allow division by zero.
  */
  if ( (itype==2 || itype==4 || itype==5) && constant==0. )
    {
      jip_text(2,"You are attempting division by zero.");
      return(1);
    }
  /*
  ** Perform the arithmetic function on all volumes in the volume list.
  */
  for ( volume=volume_list_root; volume ; volume=volume->next )
    {
      stack = volume->stack;
      for ( j=0; j<volume->n3d; j++ )
	{
	  if ( itype == 0 )       /* subtract */
	    {
	      volume->SpaceTime.fcData[j].real -= constant;
	      if ( !(stack->status&MAG_PHASE_BIT) )
		volume->SpaceTime.fcData[j].imag -= constant;
	    }
	  else if ( itype == 1 )  /* add */
	    {
	      volume->SpaceTime.fcData[j].real += constant;
	      if ( !(stack->status&MAG_PHASE_BIT) )
		volume->SpaceTime.fcData[j].imag += constant;
	    }
	  else if ( itype == 2 )  /* divide */
	    {
	      volume->SpaceTime.fcData[j].real /= constant;
	      if ( !(stack->status&MAG_PHASE_BIT) )
		volume->SpaceTime.fcData[j].imag /= constant;
	    }
	  else if ( itype == 3 )  /* multiply */
	    {
	      volume->SpaceTime.fcData[j].real *= constant;
	      if ( !(stack->status&MAG_PHASE_BIT) )
		volume->SpaceTime.fcData[j].imag *= constant;
	    }
	  else if ( itype == 4 )  /* ln */
	    {
	      if ( volume->SpaceTime.fcData[j].real/constant > 0. )
		volume->SpaceTime.fcData[j].real = log( (double)
					   (volume->SpaceTime.fcData[j].real/constant) );
	      else
		volume->SpaceTime.fcData[j].real = 0.;
	      if ( !(stack->status&MAG_PHASE_BIT) )
		{
		  if ( volume->SpaceTime.fcData[j].imag/constant > 0. )
		    volume->SpaceTime.fcData[j].imag = log( (double)
						(volume->SpaceTime.fcData[j].imag/constant) );
		  else
		    volume->SpaceTime.fcData[j].imag = 0.;
		}
	    }
	  else if ( itype == 5 )  /* exp */
	    {
	      volume->SpaceTime.fcData[j].real = exp( (double)(volume->SpaceTime.fcData[j].real/constant) );
	      if ( !(stack->status&MAG_PHASE_BIT) )
		volume->SpaceTime.fcData[j].imag = exp( (double)(volume->SpaceTime.fcData[j].imag/constant) );
	    }
	}
    }
  return(0);
}

int arithmetic_function( int itype )
/*
** itype: 0) subtract
**        1) add
**        2) divide
**        3) multiply
**        4) atan
*/
{
  int nvolume;JIP_INDEX j;
  float a, b, c_a, c_b, div;
  struct data_stack *stack, *comparison_stack;
  struct volume_list  *volume, *comparison_volume;
  extern struct volume_list *volume_list_root, *comparison_list_root;

  /*
  ** Make sure that the comparison list contains just one volume.
  */
  nvolume = 0;
  for ( volume=comparison_list_root; volume ; volume=volume->next ) nvolume++;
  if ( nvolume != 1 )
    {
      jip_text(2,"The first argument to an arithmetic function must be a SINGLE volume.");
      return(1);
    }
  comparison_volume = comparison_list_root;
  comparison_stack  = comparison_volume->stack;

  /*
  ** Make sure that all volumes in the volume list have the same dimensions as
  ** the comparison volume.
  */
  for ( volume=volume_list_root; volume ; volume=volume->next )
    {
      stack = volume->stack;
      if ( volume->dim.x!=comparison_volume->dim.x ||
	   volume->dim.y!=comparison_volume->dim.y ||
	   volume->dim.z!=comparison_volume->dim.z)
	{
	  sprintf(error_string,"Stacks %s and %s have different dimensions.",
		  stack->name, comparison_stack->name);
	  jip_text(2,error_string);
	  return(1);
	}
    }

  /*
  ** Perform the arithmetic function for all volumes in the volume list.
  */
  for ( volume=volume_list_root; volume ; volume=volume->next )
    {
      for ( j=0; j<volume->n3d; j++ )
	{
	  if ( itype == 0 )       /* subtract */
	    {
	      volume->SpaceTime.fcData[j].real -= comparison_volume->SpaceTime.fcData[j].real;
	      volume->SpaceTime.fcData[j].imag -= comparison_volume->SpaceTime.fcData[j].imag;
	    }
	  else if ( itype == 1 )  /* add */
	    {
	      volume->SpaceTime.fcData[j].real += comparison_volume->SpaceTime.fcData[j].real;
	      volume->SpaceTime.fcData[j].imag += comparison_volume->SpaceTime.fcData[j].imag;
	    }
	  else if ( itype == 2 )  /* divide */
	    {
	      if ( stack->status&MAG_PHASE_BIT )
		{
		  if ( (a=comparison_volume->SpaceTime.fcData[j].real) !=0. )
		    volume->SpaceTime.fcData[j].real /= a;
		  else
		    volume->SpaceTime.fcData[j].real = 0;
		  if ( (a=comparison_volume->SpaceTime.fcData[j].imag) !=0. )
		    volume->SpaceTime.fcData[j].imag /= a;
		  else
		    volume->SpaceTime.fcData[j].imag = 0;
		}
	      else /* complex in the form (real,imag) */
		{
		  a   = volume->SpaceTime.fcData[j].real;
		  b   = volume->SpaceTime.fcData[j].imag;
		  c_a = comparison_volume->SpaceTime.fcData[j].real;
		  c_b = comparison_volume->SpaceTime.fcData[j].imag;
		  div = c_a * c_a + c_b * c_b;
		  if ( div == 0. )
		    {
		      volume->SpaceTime.fcData[j].real = 0.;
		      volume->SpaceTime.fcData[j].imag = 0.;
		    }
		  else
		    {
		      volume->SpaceTime.fcData[j].real  = ( a * c_a +   b * c_b ) / div;
		      volume->SpaceTime.fcData[j].imag  = ( b * c_a -   a * c_b ) / div;
		    }
		}
	    }
	  else if ( itype == 3 )  /* multiply */
	    {
	      if ( stack->status&MAG_PHASE_BIT )
		{
		  volume->SpaceTime.fcData[j].real *= comparison_volume->SpaceTime.fcData[j].real;
		  volume->SpaceTime.fcData[j].imag *= comparison_volume->SpaceTime.fcData[j].imag;
		}
	      else /* complex in the form (real,imag) */
		{
		  a   = volume->SpaceTime.fcData[j].real;
		  b   = volume->SpaceTime.fcData[j].imag;
		  c_a = comparison_volume->SpaceTime.fcData[j].real;
		  c_b = comparison_volume->SpaceTime.fcData[j].imag;
		  volume->SpaceTime.fcData[j].real = a * c_a - b * c_b;
		  volume->SpaceTime.fcData[j].imag = a * c_b + b * c_a;
		}
	    }
	  else if ( itype == 4 )  /* atan */
	    {
	      // atan [a = sine amp] [b = cosine amp]
	      a = volume_amp(comparison_volume,j);
	      b = volume_amp(volume,j);
	      // The next operation produces a phase shift between -90 & 90
	      volume->SpaceTime.fcData[j].real = 180./PI * atan((double)(-a/b));
	      volume->SpaceTime.fcData[j].imag = 0.;
	    }
	}
    }
  return(0);
}

int absolute_value()
{
  JIP_INDEX j;
  float amp;
  struct data_stack *stack;
  struct volume_list  *volume;
  extern struct volume_list *volume_list_root;

  // Perform the arithmetic function for all volumes in the volume list.
  for ( volume=volume_list_root; volume ; volume=volume->next )
    {
      stack = volume->stack;
      for ( j=0; j<volume->n3d; j++ )
	{
	  if ( stack->status&MAG_PHASE_BIT )
	    {
	      volume->SpaceTime.fcData[j].real = fabs((double)volume->SpaceTime.fcData[j].real); // magnitude
	      volume->SpaceTime.fcData[j].imag = 0;  // phase
	    }
	  else
	    {
	      amp = volume_amp(volume,j);
	      volume->SpaceTime.fcData[j].real = amp;   // real
	      volume->SpaceTime.fcData[j].imag = 0.;    // imag
	    }
	}
    }
  return(0);
}

int arithmetic_function_on_stacks( int itype )
/*
** itype: 0) subtract
**        1) add
**        2) divide
**        3) multiply
*/
{
  int error;JIP_INDEX j;
  float a, b, c_a, c_b, div;
  struct data_stack *stack, *comparison_stack, *stack_save;
  struct volume_list  *volume;
  extern struct volume_list *volume_list_root, *comparison_list_root;

  comparison_stack = comparison_list_root->stack;

  /*
  ** Make sure that all stacks in the stack list have the same dimensions as
  ** the comparison stack.
  */
  stack_save = 0;
  for ( volume=volume_list_root; volume ; volume=volume->next )
    {
      if ( (stack=volume->stack) == stack_save ) continue;
      stack_save = stack;

      /* Make sure all stacks have the same dimensions. */
      error = stack->nt != comparison_stack->nt
	|| stack->dim.x     != comparison_stack->dim.x
	|| stack->dim.y     != comparison_stack->dim.y
	|| stack->dim.z     != comparison_stack->dim.z;
      if ( error )
	{
	  sprintf(error_string,"Stacks %s and %s have different dimensions.",
		  volume_list_root->stack->name,stack->name);
	  jip_text(2,error_string);
	  return(1);
	}
    }

  /*
  ** Perform the arithmetic function for all stacks in the stack list.
  */
  stack_save = 0;
  for ( volume=volume_list_root; volume ; volume=volume->next )
    {
      if ( (stack=volume->stack) == stack_save ) continue;
      stack_save = stack;

      for ( j=0; j<stack->n4d; j++ )
	{
	  if ( itype == 0 )       /* subtract */
	    {
	      stack->SpaceTime.fcData[j].real -= comparison_stack->SpaceTime.fcData[j].real;
	      stack->SpaceTime.fcData[j].imag -= comparison_stack->SpaceTime.fcData[j].imag;
	    }
	  else if ( itype == 1 )  /* add */
	    {
	      stack->SpaceTime.fcData[j].real += comparison_stack->SpaceTime.fcData[j].real;
	      stack->SpaceTime.fcData[j].imag += comparison_stack->SpaceTime.fcData[j].imag;
	    }
	  else if ( itype == 2 )  /* divide */
	    {

	      a   = stack->SpaceTime.fcData[j].real;
	      b   = stack->SpaceTime.fcData[j].imag;
	      c_a = comparison_stack->SpaceTime.fcData[j].real;
	      c_b = comparison_stack->SpaceTime.fcData[j].imag;
	      if ( stack->status&MAG_PHASE_BIT )
		{
		  if ( c_a != 0. )
		    stack->SpaceTime.fcData[j].real = a / c_a;
		  else
		    stack->SpaceTime.fcData[j].real = 0.;
		  if ( c_b != 0. )
		    stack->SpaceTime.fcData[j].imag = b / c_b;
		  else
		    stack->SpaceTime.fcData[j].imag = 0.;
		}
	      else /* complex in the form (real,imag) */
		{
		  div = c_a * c_a + c_b * c_b;
		  if ( div == 0. )
		    {
		      stack->SpaceTime.fcData[j].real = 0.;
		      stack->SpaceTime.fcData[j].imag = 0.;
		    }
		  else
		    {
		      stack->SpaceTime.fcData[j].real  = ( a * c_a +   b * c_b ) / div;
		      stack->SpaceTime.fcData[j].imag  = ( b * c_a -   a * c_b ) / div;
		    }
		}
	    }
	  else if ( itype == 3 )  /* multiply */
	    {
	      a   = stack->SpaceTime.fcData[j].real;
	      b   = stack->SpaceTime.fcData[j].imag;
	      c_a = comparison_stack->SpaceTime.fcData[j].real;
	      c_b = comparison_stack->SpaceTime.fcData[j].imag;
	      stack->SpaceTime.fcData[j].real = a * c_a - b * c_b;
	      stack->SpaceTime.fcData[j].imag = a * c_b + b * c_a;
	    }
	}
    }
  return(0);
}

int create_fCBV( int mode, char *new_stack_name )
{
  IPOINT3D dim;
  int nt, jt;JIP_INDEX jv, index;
  double s_pre, s_post, s_base, s_time;
  struct data_stack *stack, *new_stack;
  struct volume_list  *volume, *volume_pre, *volume_post, *volume_base, *volume_start;
  extern struct volume_list *volume_list_root;

  /*
  ** Make sure that all volumes have the same dimensions.
  */
  if ( inconsistent_3d_dimensions(&dim,&nt) ) return(1);

  /*
  ** MODE = 0
  ** There should be at least three volumes in the list:
  ** 1) pre-injection
  ** 2) post-injection
  ** 3) functional volume series
  ** 
  ** MODE = 1
  ** There should be at least four volumes in the list:
  ** 1) pre-injection
  ** 2) post-injection
  ** 3) functional baseline
  ** 4) functional volume series
  */
  if ( mode == 0 && nt < 3 )
     {
      jip_text(0,"1) pre-injection, 2) post-injection, 3) functional volume series.");
      jip_text(2,"There should be >= 3 volumes passed to this command.");
      return(1);
    }
  else if ( mode == 1 && nt < 4 )
     {
      jip_text(0,"1) pre-injection, 2) post-injection, 3) functional baseline, 4) functional volume series.");
      jip_text(2,"There should be >= 4 volumes passed to this command.");
      return(1);
    }

  if ( mode == 0 )
    nt = nt - 2;
  else
    nt = nt - 3;
  /*
  ** Install the new_stack in memory.
  */
  stack = volume_list_root->stack;
  new_stack = install_stack( new_stack_name, stack, dim, nt );
  new_stack->status = new_stack->status | MAG_PHASE_BIT;

  /*
  ** mode=0  1)volume_pre  2)volume_post = volume_base  3)volume_time (series)
  ** mode=1  1)volume_pre  2)volume_post 3)volume_base  4)volume_time (series)
  */
  volume_pre  = volume_list_root;
  volume_post = volume_pre->next;
  if ( mode == 0 )
    volume_base = volume_post;
  else
    volume_base = volume_post->next;
  volume_start = volume_base->next;
  /*
  ** Begin a loop over volumes.
  */
  for ( jv=0; jv<new_stack->n3d; jv++ )
    {
      s_pre  = volume_amp(volume_pre,jv);
      s_post = volume_amp(volume_post,jv);
      s_base = volume_amp(volume_base,jv);
      for (jt=0, volume=volume_start; volume; jt++, volume=volume->next)
	{
	  s_time = volume_amp(volume,jv);
	  index  = jt * new_stack->n3d + jv;
	  if ( s_pre <= 0. || s_post <= 0. || s_base <= 0. || s_time <= 0. )
	    new_stack->SpaceTime.fcData[index].real = new_stack->SpaceTime.fcData[index].imag = 0.;
	  else
	    {
	      /* Units = 100*% */
	      new_stack->SpaceTime.fcData[index].real = 10000. * log(s_time/s_base) / log(s_post/s_pre);
	      new_stack->SpaceTime.fcData[index].imag = 0.;
	    }
	}
    }
  return(0);
}
