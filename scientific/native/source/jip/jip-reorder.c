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


void clip_stacks( int idim )
     // *** NOT TESTED ***
// idim:  0 = clip x
//        1 = clip y
//        2 = clip z
{
  int last_ny, last_nx, last_nz, lNext;
  struct data_stack *stack, *stack_save;
  struct volume_list  *volume;
  extern struct volume_list *volume_list_root;

  // Cycle through the volume list, beginning with the root. 
  stack_save = 0;
  for ( volume=volume_list_root; volume; volume=volume->next )
    {
      if ( (stack=volume->stack) == stack_save ) continue;
      stack_save = stack;
      is_power_of_2(stack->dim.x, &last_nx, &lNext);
      // Allow the following case for spectroscopy. 
      if ( stack->dim.y == 1 )
	last_ny = 1;
      else
	is_power_of_2(stack->dim.y, &last_ny, &lNext);
      if ( stack->dim.z == 1 )
	last_nz = 1;
      else
	is_power_of_2(stack->dim.z, &last_nz, &lNext);
      if ( idim == 0 )
	clip_stack( stack, last_nx, stack->dim.y, stack->dim.z );
      else if ( idim == 1 )
	clip_stack( stack, stack->dim.x, last_ny, stack->dim.z );
      else
	clip_stack( stack, stack->dim.x, stack->dim.y, last_nz );
    }
}

int clip_stacks_to_n( int idim, int nclip)
     // *** NOT TESTED ***
//
// idim:  0 = clip x
//        1 = clip y
//        2 = clip z
{
  struct data_stack *stack, *stack_save;
  struct volume_list  *volume;
  extern struct volume_list *volume_list_root;

  if ( nclip%2 != 0 )
    {
      printf("Error: clip only to even numbers!\n");
      return(1);
    }

  // Cycle through the volume list, beginning with the root. 
  stack_save = 0;
  for ( volume=volume_list_root; volume; volume=volume->next )
    {
      if ( (stack=volume->stack) == stack_save ) continue;
      stack_save = stack;
      if ( idim == 0 )
	clip_stack( stack, nclip, stack->dim.y, stack->dim.z );
      else if ( idim == 1 )
	clip_stack( stack, stack->dim.x, nclip, stack->dim.z );
      else
	clip_stack( stack, stack->dim.x, stack->dim.y, nclip );
    }
  return(0);
}

void clip_stack( struct data_stack *stack, int smaller_nx, int smaller_ny, int smaller_nz )
     // *** NOT TESTED ***
{
  FCOMPLEX *temp_data;
  int offset_x, offset_y, offset_z;JIP_INDEX index, index_smaller;
  int jx, jy, jz, jt;
  BOOL inside;

  temp_data = fcomplex_vector(smaller_nx*smaller_ny*smaller_nz*stack->nt);
  offset_x = ( stack->dim.x - smaller_nx ) / 2;
  offset_y = ( stack->dim.y - smaller_ny ) / 2;
  offset_z = ( stack->dim.z - smaller_nz ) / 2;

  index_smaller = index = 0;
  for ( jt=0; jt<stack->nt; jt++ )
    {
      for (jz=0; jz<stack->dim.z; jz++)
	{
	  for (jy=0; jy<stack->dim.y; jy++)
	    {
	      for (jx=0; jx<stack->dim.x; jx++)
		{
		  if ( offset_x == 0 || !(stack->dim.x%2) )
		    inside = (jx >= offset_x && jx < stack->dim.x - offset_x);
		  else
		    inside = (jx >  offset_x && jx < stack->dim.x - offset_x);
		  if ( offset_y == 0 || !(stack->dim.y%2) )
		    inside &= (jy >= offset_y && jy < stack->dim.y - offset_y);
		  else
		    inside &= (jy >  offset_y && jy < stack->dim.y - offset_y);
		  if ( offset_z==0 || !(stack->dim.z%2) )
		    inside &= (jz >= offset_z && jz < stack->dim.z - offset_z);
		  else
		    inside &= (jz >  offset_z && jz < stack->dim.z - offset_z);
		  if ( inside )
		    {
		      temp_data[index_smaller] = stack->SpaceTime.fcData[index];
		      index_smaller++;
		    }
		  index++ ;
		}
	    }
	}
    }
  printf("Stack %s was clipped from %d x %d to %d x %d.\n",stack->name,
	 stack->dim.y,stack->dim.x,smaller_ny,smaller_nx);

  free_fcomplex_vector(stack->SpaceTime.fcData); 
  stack->dim.x  = smaller_nx;
  stack->dim.y  = smaller_ny;
  stack->dim.z  = smaller_nz;
  stack->n2d    = jip_product(smaller_nx,smaller_ny);
  stack->n3d    = jip_product(stack->n2d,smaller_nz);
  stack->n4d    = jip_product(stack->n3d,stack->nt);
  stack->npts_volume = jip_product(2,stack->n4d);
  stack->SpaceTime.fcData   = temp_data;

  //////////// Shift the coordinates //////////////////////
  stack->origin.x += offset_x*stack->resolution.x*stack->direction.x;
  stack->origin.y += offset_y*stack->resolution.y*stack->direction.y;
  stack->origin.z += offset_z*stack->resolution.z*stack->direction.z;
  stack->ijk_to_xyz = scale_and_shift_to_mat44(stack->resolution,stack->origin,stack->direction);
  stack->xyz_to_ijk = invert_mat44( stack->ijk_to_xyz );
}

void pad_stacks( int idim )
     // *** NOT TESTED ***

// idim:  0 = pad read
//        1 = pad phase
//        2 = pad 3rd dimension (3-d phase encode)
//        3 = pad both read and phase but not 3rd dimension
//        4 = pad all dimensions
{
  int next_nz, next_ny, next_nx, last;
  struct data_stack *stack, *stack_save;
  struct volume_list  *volume;
  extern struct volume_list *volume_list_root;

  // Cycle through the volume list, beginning with the root. 
  stack_save = 0;
  for ( volume=volume_list_root; volume; volume=volume->next )
    {
      if ( (stack=volume->stack) == stack_save ) continue;
      stack_save = stack;
	{
	  is_power_of_2(stack->dim.x,  &last, &next_nx);
	  is_power_of_2(stack->dim.y,  &last, &next_ny);
	  is_power_of_2(stack->dim.z,  &last, &next_nz);
	  if ( idim == 0 )
	    pad_stack( stack, next_nx, stack->dim.y, stack->dim.z, 0 );
	  else if ( idim == 1 )
	    pad_stack( stack, stack->dim.x, next_ny, stack->dim.z, 0 );
	  else if ( idim == 2 )
	    pad_stack( stack, stack->dim.x, stack->dim.y, next_nz, 0 );
	  else if ( idim == 3 )
	    pad_stack( stack, next_nx, next_ny, stack->dim.z, 0 );
	  else
	    pad_stack( stack, next_nx, next_ny, next_nz, 0 );
	}
    }
}

void pad_stack( struct data_stack *stack, int larger_nx, int larger_ny, int larger_nz, int partial_ky )
     // *** NOT TESTED ***
{
  FCOMPLEX *temp_data;
  int offset_z, offset_y, offset_x, top_y;
  int jt, jz, jy, jx;
  int stack_size;JIP_INDEX index, index_larger;
  float factor;
  //
  // A single volume in a stack cannot be padded without affecting other
  // data in that stack, so instead every volume in the stack is padded.
  // Allocate a new data array for this purpose.
  
  stack_size = stack->nt * larger_nx * larger_ny * larger_nz;
  temp_data = fcomplex_vector(stack_size);
  offset_x = ( larger_nx - stack->dim.x ) / 2;
  offset_z = ( larger_nz - stack->dim.z ) / 2;
  if ( partial_ky )
    {
      offset_y = larger_ny - stack->dim.y;
      top_y    = larger_ny + offset_y;
    }
  else
    {
      offset_y = ( larger_ny - stack->dim.y ) / 2;
      top_y    = larger_ny;
    }
  //
  // Assume that the purpose of padding is to interpolate (after a Fourier
  // transform).  To preserve the data values, the volume must be scaled
  // up so that the inverse-transform will not change the scale.
  
  factor = stack_size / (float)(stack->n3d);
  
  index_larger = index = 0;
  for ( jt=0; jt<stack->nt; jt++ )
    {
      for ( jz=0; jz<larger_nz; jz++ )
	{
	  for ( jy=0; jy<larger_ny; jy++ )
	    {
	      for ( jx=0; jx<larger_nx; jx++ )
		{
		  if (   jz >= offset_z && jz < larger_nz - offset_z &&
			 jy >= offset_y && jy < top_y - offset_y &&
			 jx >= offset_x && jx < larger_nx - offset_x )
		    {
		      temp_data[index_larger].real=stack->SpaceTime.fcData[index].real*factor;
		      temp_data[index_larger].imag=stack->SpaceTime.fcData[index].imag*factor;
		      index++;
		    }
		  else
		    {
		      temp_data[index_larger].real = 0.;
		      temp_data[index_larger].imag = 0.;
		    }
		  index_larger++ ;
		}
	    }
	}
    }
  printf("Stack %s was padded from %d x %d x %d to %d x %d x %d.\n",stack->name,
	 stack->dim.x,stack->dim.y,stack->dim.z,larger_nx,larger_ny,larger_nz);
  
  free_fcomplex_vector(stack->SpaceTime.fcData);
  stack->SpaceTime.fcData = temp_data;
  stack->dim.x = larger_nx;
  stack->dim.y = larger_ny;
  stack->dim.z = larger_nz;
  stack->n2d   = jip_product(stack->dim.x,stack->dim.y);
  stack->n3d   = jip_product(stack->n2d,stack->dim.z);
  stack->n4d   = jip_product(stack->n3d,stack->nt);
  stack->npts_volume = jip_product(2,stack->n4d);
}

void pad_2n_stacks()
     // *** NOT TESTED ***
     // Force each dimensions in the stack to be an integer power of 2 by padding
     // the data with zeroes, if necessary.
{
  int next_ny, next_nx, last;
  struct data_stack *stack, *stack_save;
  struct volume_list  *volume;
  extern struct volume_list *volume_list_root;

  // Cycle through the volume list, beginning with the root. 
  stack_save = 0;
  for ( volume=volume_list_root; volume; volume=volume->next )
    {
      if ( (stack=volume->stack) == stack_save ) continue;
      stack_save = stack;
      if ( is_power_of_2(stack->dim.x, &last, &next_nx) ) next_nx /= 2;
      if ( is_power_of_2(stack->dim.y, &last, &next_ny) ) next_ny /= 2;
      
      if ( next_nx * next_ny != ((JIP_INDEX)(stack->dim.x)) * stack->dim.y )
	pad_stack( stack, next_nx, next_ny, stack->dim.z, 0 );
    }
}

void pad_stacks_to_n( int idim, int npad)
     // *** NOT TESTED ***
// idim:  0 = pad read
//        1 = pad phase
//        2 = pad both read and phase
{
  struct data_stack *stack, *stack_save;
  struct volume_list  *volume;
  extern struct volume_list *volume_list_root;

  // Cycle through the volume list, beginning with the root. 
  stack_save = 0;
  for ( volume=volume_list_root; volume; volume=volume->next )
    {
      if ( (stack=volume->stack) == stack_save ) continue;
      stack_save = stack;
      if ( idim == 0 )
	pad_stack( stack, npad, stack->dim.y, stack->dim.z, 0 );
      else if ( idim == 1 )
	pad_stack( stack, stack->dim.x, npad, stack->dim.z, 0 );
      else
	pad_stack( stack, npad, npad, stack->dim.z, 0 );
    }
}

int shuffle_stacks( char *new_stack_name )
     // *** NOT TESTED ***
{
  IPOINT3D dim;
  int nt, number_stacks;JIP_INDEX index_new;
  int jt, jstack;JIP_INDEX jv;
  struct data_stack *stack, *stack_save, *new_stack;
  struct volume_list  *volume;
  extern struct volume_list *volume_list_root;
  
  //
  // Shuffle a list of stacks which all have the same number of volumes to
  // create a new stack in which the volumes are shuffled as
  // stack0.0 stack1.0 stack2.0 stack0.1 stack1.1 stack2.1 ...
  
  // First find the number of stacks, the number of volumes per stack, and the
  // total number of volume points.
  
  number_stacks = 0;
  stack_save = 0;
  for ( volume=volume_list_root; volume; volume=volume->next )
    {
      if ( (stack=volume->stack) == stack_save ) continue;
      stack_save = stack;
      number_stacks++;
      if ( volume == volume_list_root )
	{
	  dim = stack->dim;
	  nt = stack->nt;
	}
      else
	{
	  if ( stack->nt != nt )
	    {
	      printf("Stacks %s and %s have different numbers of volumes.",
		     (volume_list_root->stack)->name,stack->name);
	      return(1);
	    }
	  else if ( stack->dim.x != dim.x || stack->dim.y != dim.y || stack->dim.z != dim.z )
	    {
	      printf("Stacks %s and %s have different dimensions.",
		     (volume_list_root->stack)->name,stack->name);
	      return(1);
	    }
	}
    }
  // Install the new_stack in memory.
  new_stack = install_stack( new_stack_name, volume_list_root->stack,
			     dim, number_stacks*nt );

  stack_save = volume_list_root->stack;
  for ( volume=volume_list_root, jt=jstack=0; volume; volume=volume->next, jt++ )
    { // loop over all volumes
      if ( (stack=volume->stack) != stack_save )
	{
	  jt=0;
	  jstack++;
	}
      stack_save = stack;
      // Put the current volume in the proper location in the new stack.
      for (jv=0; jv<stack->n3d; jv++) // stack volume
	{
	  index_new = jv + new_stack->n3d * (jstack + number_stacks*jt);
	  new_stack->SpaceTime.fcData[index_new] = volume->SpaceTime.fcData[jv];
	}
    }
  return(0);
}

int slice_volumes( char *new_stack_name, int nslice )
     // *** NOT TESTED ***
//
// Slice and copy a list of volumes, and put the result in a new stack with the
// name given by `new_stack_name'.
{
  IPOINT3D dim;
  int nt;JIP_INDEX index_copy;
  int jslice, jv;JIP_INDEX jp;
  struct data_stack *stack, *new_stack;
  struct volume_list  *volume;
  extern struct volume_list *volume_list_root;

  // Make sure that all volumes have the same dimensions.  Also, count
  // the number of volumes.

  // Make sure that all volumes have the same dimensions.
  for ( volume=volume_list_root, nt = 0;
	volume;
	volume=volume->next )
    {
      nt++;
      // Make sure all stacks have the same dimensions.
      if ( volume == volume_list_root )
	dim = volume->dim;
      else
	{
	  if ( volume->dim.x!=dim.x || volume->dim.y!=dim.y || volume->dim.z!=dim.z )
	    {
	      sprintf(error_string,"Stacks %s and %s have different dimensions.",
		      volume_list_root->stack->name,volume->stack->name);
	      jip_text(2,error_string);
	      return(1);
	    }
	}
      for ( jslice=0; jslice<nslice-1; jslice++ )
	{
	  volume=volume->next;
	  if ( !volume ) break;
	}
      if ( !volume ) break;
    }
  // Install the new_stack in memory.
  stack = volume_list_root->stack;
  new_stack = install_stack( new_stack_name, stack, dim, nt );

  // Begin a loop over volumes.  Copy the data for each volume into the newly
  // created volume stack.
  for (jv=0, volume=volume_list_root; volume; jv++, volume=volume->next)
    {
      stack = volume->stack;
      // Copy the volume data.
      for ( jp=0; jp<stack->n3d; jp++ )
	{
	  index_copy  = jv * new_stack->n3d + jp;
	  new_stack->SpaceTime.fcData[index_copy] = volume->SpaceTime.fcData[jp];
	}
      // Increment the slice number.
      for ( jslice=0; jslice<nslice-1; jslice++ )
	{
	  volume=volume->next;
	  if ( !volume ) break;
	}
      if ( !volume ) break;
    }
  return(0);
}

void reverse_slice_order()
{
  int jz, jz1;JIP_INDEX jp, jv;
  JIP_INDEX iswap1, iswap2;
  FCOMPLEX *temp_data;
  struct volume_list  *volume;
  extern struct volume_list *volume_list_root;
  
  // Cycle through the volume list, beginning with the root. 
  for ( volume=volume_list_root; volume; volume=volume->next )
    {
      // Allocate temporary memory, then copy this into the stack array.
      temp_data = fcomplex_vector(volume->n3d);
      // Reorder each set of slices separately.
      for ( jz=0; jz<volume->dim.z; jz++ )
	{
	  jz1 = volume->dim.z - jz - 1;
	  for (jp=0; jp<volume->n2d; jp++)
	    {
	      iswap1 = jz  * volume->n2d + jp;
	      iswap2 = jz1 * volume->n2d + jp;
	      temp_data[iswap2] = volume->SpaceTime.fcData[iswap1];
	    }
	}
      for (jv=0; jv<volume->n3d; jv++)
	volume->SpaceTime.fcData[jv] = temp_data[jv];
      free_fcomplex_vector(temp_data);
    }
}

int reorder_slices()
     // Use the `slice-order' array to specify the reordering scheme.
{
  IPOINT3D dim;
  int nt;JIP_INDEX iswap1, iswap2;
  int jz, jz_new;JIP_INDEX jp;
  FCOMPLEX *temp_data;
  struct volume_list  *volume;
  extern struct volume_list *volume_list_root;
  extern int nslice_order, *slice_order;

  // Make sure all volumes have the same dimensions.
  if ( inconsistent_3d_dimensions(&dim,&nt) ) return(1);
  // Make sure that the `volume-order' has been set.
  if ( nslice_order == 0 || slice_order == NULL )
    {
      printf("Set the `slice-order' array. You may want to use `reorder-interleaved-slices'\n");
      return(1);
    }
  else if ( dim.z != nslice_order )
    {
      printf("The number of slices (%d) must match the length of the `slice-order' array (%d).\n",
	     dim.z,nslice_order);
      return(1);
    }
  else if ( dim.z == 1 )
    {
      printf("Only 1 slice; nothing done.\n");
      return(0);
    }

  // Reordering is facilited with temporary arrays.
  temp_data = fcomplex_vector(((JIP_INDEX)(dim.x))*dim.y*dim.z); // holds the reordered image data.
  for (jp=0; jp<((JIP_INDEX)(dim.x))*dim.y*dim.z; jp++)
    temp_data->real = temp_data->imag = 0.;
  
  // Cycle through the volume list, beginning with the root. 
  for ( volume=volume_list_root; volume; volume=volume->next )
    {
      // Loop over the slices.
      for ( jz=0; jz<dim.z; jz++ )
	{
	  jz_new = slice_order[jz];
	  // Reorder the volume data.
	  for ( jp=0; jp<volume->n2d; jp++ )
	    {
	      iswap1 = jz     * volume->n2d + jp;
	      iswap2 = jz_new * volume->n2d + jp;
	      temp_data[iswap2] = volume->SpaceTime.fcData[iswap1];
	    }
	}
      // Copy the data into the volume array.
      for ( jp=0; jp<volume->n3d; jp++ )
	volume->SpaceTime.fcData[jp] = temp_data[jp];
    }
  free_fcomplex_vector(temp_data);
  return(0);
}

int reorder_time_and_slices()
// This function rearranges data with the time index faster than the slice index.
// This can happen, for instance, using multi-echo sequences.
{
  IPOINT3D dim;
  int nt, iold, jz, jt;JIP_INDEX inew;JIP_INDEX j;
  FCOMPLEX *temp_data;
  struct data_stack *stack, *stack_save;
  struct volume_list  *volume;
  extern struct volume_list *volume_list_root;

  // Make sure all volumes have the same dimensions.
  if ( inconsistent_3d_dimensions(&dim,&nt) ) return(1);
  // Make sure that the `volume-order' has been set.
  if ( dim.z == 1 )
    {
      printf("Only 1 slice; nothing done.\n");
      return(0);
    }
  else if ( nt == 1 )
    {
      printf("Only 1 time point; nothing done.\n");
      return(0);
    }

  printf("This function assumes correct labeling of # slices (%d) and # time (%d)\n\n",dim.z,nt);

  // Cycle through the volume list, beginning with the root. 
  stack_save = 0;
  for ( volume=volume_list_root; volume; volume=volume->next )
    {
      if ( (stack=volume->stack) == stack_save ) continue;
      stack_save = stack;
      printf("n3d = %td, nt = %d, n4d = %td\n",stack->n3d, stack->nt, stack->n4d);
      temp_data = fcomplex_vector(stack->n4d);
      for (jz=0; jz<stack->dim.z; jz++)
	{
	  for (jt=0; jt<stack->nt; jt++)
	    {
	      iold = jz * stack->nt    + jt;   // time  index faster
	      inew = ((JIP_INDEX)(jt)) * stack->dim.z + jz;   // slice index faster
	      for (j=0; j<stack->n2d; j++)
		temp_data[inew*stack->n2d+j] = stack->SpaceTime.fcData[iold*stack->n2d+j];
	    }
	}
      for (j=0; j<stack->n4d; j++)
	stack->SpaceTime.fcData[j] = temp_data[j];
      free_fcomplex_vector(temp_data);
    }
  return(0);
}

void swap_quadrants_in_volume_list( int mirror, int iswap, int idiv_x, int idiv_y )
{
  struct volume_list  *volume;
  extern struct volume_list *volume_list_root;

  // Cycle through the volume list, beginning with the root. 
  for ( volume=volume_list_root; volume; volume=volume->next )
    swap_quadrants( volume, mirror, iswap, idiv_x, idiv_y );
}

void swap_quadrants( struct volume_list *volume, int mirror, int iswap, int idiv_x, int idiv_y )
     // *** NOT TESTED ***
// iswap: 0) swap quadrants left and right (readout dimension)
//        1) swap quadrants up and down (phase-encode dimension)
//        2) swap quadrants left/right and up/down
// idiv_xx: factors by which to divide the data.  This is useful since this
//          routine is employed after fft's, and thus the data can be scaled
//          at the same time it is reordered.

{
  int jx, jy, jz;JIP_INDEX iswap1, iswap2;
  float div_x, div_y;
  FCOMPLEX buffer;

  div_x = idiv_x;
  div_y = idiv_y;

  if ( iswap == 0 || iswap == 2 )
    {
      // Swap quadrants left-right. 
      for (jz=0; jz<volume->dim.z; jz++)
	{
	  for (jy=0; jy<volume->dim.y; jy++)
	    {
	      for (jx=0; jx<volume->dim.x/2; jx++)
		{
		  iswap1 = volume->n2d*jz + ((JIP_INDEX)(volume->dim.x)) * jy + jx;
		  if ( mirror )  // jx -> -jx 
		    iswap2 = volume->n2d*jz + ((JIP_INDEX)(volume->dim.x)) * (jy+1) - jx - 1;
		  else
		    iswap2 = iswap1 + volume->dim.x/2;
		  buffer = volume->SpaceTime.fcData[iswap1];
		  volume->SpaceTime.fcData[iswap1].real = volume->SpaceTime.fcData[iswap2].real / div_x;
		  volume->SpaceTime.fcData[iswap1].imag = volume->SpaceTime.fcData[iswap2].imag / div_x;
		  volume->SpaceTime.fcData[iswap2].real = buffer.real               / div_x;
		  volume->SpaceTime.fcData[iswap2].imag = buffer.imag               / div_x;
		}
	    }
	}
    }

  if ( iswap == 1 || iswap == 2 )
    {
      // Swap quadrants top-bottom. 
      for (jz=0; jz<volume->dim.z; jz++)
	{
	  for (jy=0; jy<volume->dim.y/2; jy++)
	    {
	      for (jx=0; jx<volume->dim.x; jx++)
		{
		  iswap1 = volume->n2d*jz + ((JIP_INDEX)(volume->dim.x)) * jy + jx;
		  if ( mirror )  // jy -> -jy
		    {
		      iswap2 = volume->n2d*jz - ((JIP_INDEX)(volume->dim.x))*jy + jx; 
		      if ( iswap2 < volume->n2d*jz ) iswap2 += volume->n2d;
		    }
		  else
		    iswap2 = iswap1 + ((JIP_INDEX)(volume->dim.x)) * volume->dim.y / 2;
		  buffer = volume->SpaceTime.fcData[iswap1];
		  volume->SpaceTime.fcData[iswap1].real = volume->SpaceTime.fcData[iswap2].real / div_y;
		  volume->SpaceTime.fcData[iswap1].imag = volume->SpaceTime.fcData[iswap2].imag / div_y;
		  volume->SpaceTime.fcData[iswap2].real = buffer.real               / div_y;
		  volume->SpaceTime.fcData[iswap2].imag = buffer.imag               / div_y;
		}
	    }
	}
    }
}
