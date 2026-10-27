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
#include "util.h"
#include "jip.h"

extern char error_string[LEN_STR];


int map_snr( char *new_stack_name )
     // Make a T2 map from a list of image stacks.
{
  IPOINT3D dim;
  int nt, jt;JIP_INDEX jv, i1, i2, i3;
  float *data_vector, *fit_vector;
  float mean, stdev;
  struct data_stack *stack, *new_stack;
  struct volume_list  *volume;
  extern struct volume_list *volume_list_root;
  extern int flags;

  // Make sure that all volumes have the same dimensions.
  if ( inconsistent_3d_dimensions(&dim,&nt) ) return(1);

  if ( nt < 2 )
    {
      printf("Error: there must be >= 2 images to compute an average and stdev.\n");
      return(1);
    }
  // temporal working arrays:
  data_vector = float_vector(nt);
  fit_vector  = float_vector(nt);

  // Install the new_stack in memory - 3 volumes for S/N, S, N
  stack = volume_list_root->stack;
  new_stack = install_stack( new_stack_name, stack, dim, 3 );
  new_stack->data_type = 2;
  new_stack->status = new_stack->status | MAG_PHASE_BIT;

  // Begin a loop over voxels.
  for ( jv=0; jv<new_stack->n3d; jv++ )
    {
      // Define the data array for this point.
      for (volume=volume_list_root, jt=0; volume; volume=volume->next, jt++)
	data_vector[jt] = volume_amp(volume,jv);
      // Find the mean.
      for (jt=0, mean=0.; jt<nt; jt++)
	mean += data_vector[jt];
      mean /= (float)nt;
      // Find the standard deviation.
      for (jt=0, stdev=0.; jt<nt; jt++)
	stdev += SQR(mean - data_vector[jt]);
      stdev = sqrt( (double)(stdev/(float)(nt-1)) );
      // Fill the new stack.
      i1 = jv;
      i2 = i1 + new_stack->n3d;
      i3 = i2 + new_stack->n3d;
      new_stack->SpaceTime.fcData[i1].imag
	= new_stack->SpaceTime.fcData[i2].imag = new_stack->SpaceTime.fcData[i3].imag = 0;
      if ( stdev != 0. )
	new_stack->SpaceTime.fcData[i1].real = mean / stdev;
      else
	new_stack->SpaceTime.fcData[i1].real = 0.;
      new_stack->SpaceTime.fcData[i2].real = mean;
      new_stack->SpaceTime.fcData[i3].real = stdev;
    }

  if ( !(flags&MUTE_BIT) )
    printf("The map stack contains 3 images in the order SNR, S, N.\n");

  free_float_vector(data_vector);
  free_float_vector(fit_vector);

  return(0);
}

int map_snr_old( char *new_stack_name )
     // Make a T2 map from a list of image stacks.
{
  IPOINT3D dim;
  int nt, jt;JIP_INDEX jv, i1, i2, i3;
  int *include_vector;
  float *data_vector, *fit_vector;
  float mean, stdev;
  struct data_stack *stack, *new_stack;
  struct volume_list  *volume;
  extern struct volume_list *volume_list_root;
  extern int flags;

  // Make sure that all volumes have the same dimensions.
  if ( inconsistent_3d_dimensions(&dim,&nt) ) return(1);

  if ( nt < 5 )
    {
      printf("Error: there must be >= 5 images to fit the 3 baseline + 2 snr parameters.\n");
      return(1);
    }
  // Include all points
  include_vector = int_vector(nt);
  for (jt=0; jt<nt; jt++)
    include_vector[jt] = 1;
  // Define a polynomial GLM for removing baseline drift.
  if ( PolynomialDefineGLM(3, nt, include_vector) )
    {
      printf("Error defining polynomial GLM in function map_snr.\n");
      return(1);
    }
  // temporal working arrays:
  data_vector = float_vector(nt);
  fit_vector  = float_vector(nt);

  // Install the new_stack in memory - 3 volumes for S/N, S, N
  stack = volume_list_root->stack;
  new_stack = install_stack( new_stack_name, stack, dim, 3 );
  new_stack->data_type = 2;
  new_stack->status = new_stack->status | MAG_PHASE_BIT;

  // Begin a loop over voxels.
  for ( jv=0; jv<new_stack->n3d; jv++ )
    {
      // Define the data array for this point.
      for (volume=volume_list_root, jt=0; volume; volume=volume->next, jt++)
	data_vector[jt] = volume_amp(volume,jv);
      // Remove baseline drift.
      if ( PolynomialFitGLM(      nt, data_vector, fit_vector) ) return(1);
      if ( PolynomialRemoveGLM(1, nt, data_vector, fit_vector) ) return(1);
      // Find the mean.
      for (jt=0, mean=0.; jt<nt; jt++)
	mean += data_vector[jt];
      mean /= (float)nt;
      // Find the standard deviation.
      for (jt=0, stdev=0.; jt<nt; jt++)
	stdev += SQR(mean - data_vector[jt]);
      stdev = sqrt( (double)(stdev/(float)(nt-1)) );
      // Fill the new stack.
      i1 = jv;
      i2 = i1 + new_stack->n3d;
      i3 = i2 + new_stack->n3d;
      new_stack->SpaceTime.fcData[i1].imag
	= new_stack->SpaceTime.fcData[i2].imag = new_stack->SpaceTime.fcData[i3].imag = 0;
      if ( stdev == 0. )
	new_stack->SpaceTime.fcData[i1].real
	  = new_stack->SpaceTime.fcData[i2].real = new_stack->SpaceTime.fcData[i3].real = 0;
      else
	{
	  new_stack->SpaceTime.fcData[i1].real = mean / stdev;
	  new_stack->SpaceTime.fcData[i2].real = mean;
	  new_stack->SpaceTime.fcData[i3].real = stdev;
	}
    }

  if ( !(flags&MUTE_BIT) )
     printf("The map stack contains 3 images in the order SNR, S, N.\n");

  free_int_vector(include_vector);
  free_float_vector(data_vector);
  free_float_vector(fit_vector);

  return(0);
}

int map_t2( int t2_map, char *new_stack_name )
// create maps using S = S0 * exp(-t * R), where R has units of 1/t
//
// t2_map: 1 => T2 map          S = S0 * exp(-TE * R2)
//         0 => diffusion map   S = S0 * exp(-b  * D )
//
// map m and b using a linear function: y = mx + b
//                                      y = ln(S)
//                                      b = ln(S0)
//                                      x =  TE (T2 map) or  b (diffusion map)
//                                      m = -R2 (T2 map) or -D (diffusion map)
{
  IPOINT3D dim;
  int nt, jt;JIP_INDEX jv, i1, i2, i3;
  int nfits, nfilters, number_fit;
  BOOL allzero, anyzero;
  static int first_time = 1;
  float *lnmag, sigma;
  float s_time, mr2, lns0, t2, s0;
  struct data_stack *stack, *new_stack;
  struct volume_list  *volume;
  extern struct volume_list *volume_list_root;
  extern int flags;
  extern POINT3D filter_range;
  extern int nmap_array;
  extern float *map_array;

  // Make sure that all volumes have the same dimensions.
  if ( inconsistent_3d_dimensions(&dim,&nt) ) return(1);

  // Warn the user to set the `filter-range' variable.
  if ( nt != nmap_array )
    {
      if ( t2_map )
	printf("Error: # of time points (%d) != number of TE values (%d).\n",nt,nmap_array);
      else
	printf("Error: # of time points (%d) != number of b values (%d).\n",nt,nmap_array);
      printf("Use `set map-array ...'\n");
      return(1);
    }
  else if ( nt < 2 )
    {
      printf("Error: there must be >= 2 images to fit the 2 parameters.\n");
      return(1);
    }
  else if ( first_time )
    {
      if ( t2_map )
	printf("\nRemember: the `filter-range' variable is always applied to T2 maps.\n\n");
      else
	printf("\nRemember: the `filter-range' variable is always applied to ADC maps.\n\n");
      first_time = 0;
    }

  if ( nt == 2 )
    {
      if ( t2_map )
	printf("The T2 map will be calculated (not fit) from 2 points.\n");
      else
	printf("The ADC map will be calculated (not fit) from 2 points.\n");
    }
  // The T2/ADC map will be attempted, so allocate room for lnmag[nt]
  lnmag   = float_vector(nt);

  // Install the new_stack in memory - 3 volumes for T2, R2, S0.
  stack = volume_list_root->stack;
  new_stack = install_stack( new_stack_name, stack, dim, 3 );
  new_stack->data_type = 2;
  new_stack->status = new_stack->status | MAG_PHASE_BIT;

  /*
  ** Begin a loop over voxels.
  */
  nfits = nfilters = 0;
  for ( jv=0; jv<new_stack->n3d; jv++ )
    {
      for (jt=0, volume=volume_list_root; volume; jt++, volume=volume->next)
	{
	  s_time = volume_amp(volume,jv); // signal as a function of time
	  if ( s_time <= 0. )
	    lnmag[jt] = 0.;
	  else
	    lnmag[jt] = log( (double)s_time );
	}
      // Now fit T2/ADC for this pixel.
      // The fit is  ln[S(t)] = (-1/t2)*te + ln[S(0)]
      //         or  lnmag    =    mr2 *te + lns0
      //         or  lnmap    = (-D)   * b + lns0
      //
      // Data might be pre-filtered using the "magnitude-window" or "magnitude-window-together"
      // commands, which set points to zero if they fall outside the "filter-range".
      // Do not perform a fit if every magnitude value is equal to zero.
      // If SOME but not ALL points are zero, limit the number of the points
      // to the first set of non-zero points in the series. This approach allows
      // different number of values to be used at each voxel so that the noise floor
      // will not corrupt T2/T2* values.
      allzero = TRUE;
      anyzero = FALSE;
      for (jt=0, number_fit=0; jt<nt; jt++)
	{
	  allzero &= (lnmag[jt] == 0.);
	  anyzero |= (lnmag[jt] == 0.);
	  if ( !anyzero )
	    number_fit++;
	}
      if ( allzero || number_fit < 2 )
	t2 = 0.;
      else
	{
	  fit_line( nt,map_array,lnmag,&mr2,&lns0,&sigma );
	  t2 = -1./mr2;
	  s0 = exp((double)lns0);
	  /* Remove values which are not in a reasonable range. */
	  nfits++;
	  if ( filter_range.x < filter_range.y &&
	       ( t2 < filter_range.x || t2 > filter_range.y ) )
	    {
	      t2 = 0.;
	      nfilters++;
	    }
	}
      i1 = jv;
      i2 = i1 + new_stack->n3d;
      i3 = i2 + new_stack->n3d;
      new_stack->SpaceTime.fcData[i1].imag = new_stack->SpaceTime.fcData[i2].imag
	= new_stack->SpaceTime.fcData[i3].imag = 0;
      if ( t2 == 0. )
	new_stack->SpaceTime.fcData[i1].real = new_stack->SpaceTime.fcData[i2].real
	  = new_stack->SpaceTime.fcData[i3].real = 0;
      else
	{
	  new_stack->SpaceTime.fcData[i1].real = 1.0/t2;
	  new_stack->SpaceTime.fcData[i2].real = t2;
	  new_stack->SpaceTime.fcData[i3].real = s0;
	}
    }
  /*
  ** Free the temporary arrays used in the fitting procedure.
  */
  free_float_vector(lnmag);
  /*
  ** Report fitting errors.
  */
  if ( !(flags&MUTE_BIT) )
    {
      printf("Of %d fits, %f %c were filtered.\n",nfits,
	     100.*nfilters/(float)nfits,37);
      if ( t2_map )
	printf("The map stack contains 3 images in the order R2, T2, S0.\n");
      else
	printf("The map stack contains 3 images in the order ACD, 1/ADC, S0.\n");
    }
  return(0);
}

void fit_line( int npoints, float *x, float *y, float *slope, float *offset, float *sigma )
// It is expected that the (npoints < 2 ) will be handled elsewhere.
// For npoints == 2, sigma = 0.
{
  int j;
  float av_x, av_y, num, denom;

  // Error out on npoints < 2
  if ( npoints < 2 )
    {
      printf("Programming error: npoints (%d) < 2 in fit_line.\n",npoints);
      exit(1);
    }

  // Compute averages
  for (j=0, av_x=av_y=0.; j<npoints; j++)
    {
      av_x += x[j];
      av_y += y[j];
    }
  av_x /= (float)npoints;
  av_y /= (float)npoints;

  // Compute the slope & offset
  for (j=0, num=denom=0.; j<npoints; j++)
    {
      num   += (x[j]-av_x)*(y[j]-av_y);
      denom += SQR(x[j]-av_x);
    }
  *slope  = num / denom;
  *offset = av_y - (*slope) * av_x;

  // Compute sigma.
  if ( npoints == 2 )
    *sigma = 0;
  else
    {
      for (j=0, *sigma=0.; j<npoints; j++)
	*sigma += SQR(y[j]-av_y) - (*slope)*(x[j]-av_x)*(y[j]-av_y);
      *sigma = sqrt((double)(*sigma))/(float)(npoints-2);
    }
}

int scatter_plot( char *plot_file_name )
{
  FILE *plot_file;
  short *mask_ovl;
  IPOINT3D dim;
  int nt, mask_default, ix, iy, iz;JIP_INDEX index, j;
  struct volume_list  *volume1, *volume2;
  extern struct volume_list *volume_list_root;
  extern OVERLAY overlay;

  // Make sure that all volumes have the same dimensions.
  if ( inconsistent_3d_dimensions(&dim,&nt) ) return(1);

  if ( nt != 2 )
    {
      printf("Error: there must be only 2 volumes supplied to this command.\n");
      return(1);
    }
  volume1 = volume_list_root;
  volume2 = volume1->next;

  mask_ovl = short_vector(((JIP_INDEX)(dim.x))*dim.y*dim.z);
  if ( overlay.nvoxels != 0 )  // Overlay active
    mask_default = 0;          // All off but overlay
  else
    mask_default = 1;          // all off
  // Set a default mask.
  for (j=0; j<((JIP_INDEX)(dim.x))*dim.y*dim.z; j++)
    mask_ovl[j] = mask_default;
  // Now set a mask from an overlay if one is active.
  if ( overlay.nvoxels != 0 )
    {
      for (j=0; j<overlay.nvoxels; j++)
	{
	  ix = overlay.voxel[j].x;
	  iy = overlay.voxel[j].y;
	  iz = overlay.voxel[j].z;
	  index = index3d(ix,iy,iz,volume1);
	  if ( index > ((JIP_INDEX)(dim.x))*dim.y*dim.z )
	    {
	      printf("(x,y,z) = (%d,%d,%d), index = %td, total = %td\n",
		     ix,iy,iz,index,((JIP_INDEX)(dim.x))*dim.y*dim.z);
	      jip_text(2,"The overlay falls outside the volume boundaries.");
	      return(1);
	    }
	  else
	    mask_ovl[index] = 1;
	}
    }

  // Open the plot file.
  if ( !(plot_file = fopen(plot_file_name,"w")) )
    {
      sprintf(error_string,"I cannot open the output file %s.",plot_file_name);
      jip_text(2,error_string);
      return(1);
    }
  // Write out a scatter plot of volume1 (x) versus volume2 (y) amplitudes. */
  for (j=0; j<((JIP_INDEX)(dim.x))*dim.y*dim.z; j++)
    {
      if ( mask_ovl[j] == 1 )
	fprintf(plot_file,"%f %f\n",volume_amp(volume1,j),volume_amp(volume2,j));
    }
  fclose(plot_file);
  free_short_vector(mask_ovl);
  return(0);
}

int map_t1_from_flip_angles( int tr, char *new_stack_name )
// create t1 map using S = S0 * sin(theta) * (1-exp(-tr/t1)) / (1-cos(theta)*exp(-tr/t1))
// maps: s0, r1, t1, relative_angle:
//
// where these are input: tr (fixed, in ms), angle (in map-array), series of volumes
//   and these are fit  : s0, r1, rel_angle
//
// equations:
//
// ex = exp(-tr*r1)
// phi = angle * rel_angle,
// s = s0 * sin(phi) * (1 - ex)/(1-ex*cos(phi))
//
{
  IPOINT3D dim;
  int nt, jt;JIP_INDEX jv, i1, i2, i3, i4;
  int nfits, nfilters;
  BOOL allzero;
  static int first_time = 1;
  double *flip_array, *signal_array;
  float s0, t1, r1, rel_angle, ex, fraction;
  float tr_sec, max_signal, max_angle;
  struct data_stack *stack, *new_stack;
  struct volume_list  *volume;
  extern struct volume_list *volume_list_root;
  extern int flags;
  extern POINT3D filter_range;
  extern int nmap_array;
  extern float *map_array;

  // Make sure that all volumes have the same dimensions.
  if ( inconsistent_3d_dimensions(&dim,&nt) ) return(1);

  // Warn the user to set the `filter-range' variable.
  if ( nt != nmap_array )
    {
      printf("Error: # of time points (%d) != number of flip angles (%d).\n",nt,nmap_array);
      printf("Use `set map-array ...'\n");
      exit(1);
    }
  else if ( nt < 4 )
    {
      printf("Error: there must be > 3 images to fit the 3 parameters.\n");
      return(1);
    }
  else if ( first_time )
    {
      printf("\nRemember: the `filter-range' variable is always applied to T1 maps.\n\n");
      first_time = 0;
    }

  printf("nt = %d, nmap_array = %d\n",nt,nmap_array);

  flip_array   = double_vector(nt);
  signal_array = double_vector(nt);
  for (jt=0; jt<nmap_array; jt++)
    flip_array[jt] = map_array[jt];

  // Install the new_stack in memory - 4 volumes for T1, R1, S0, rel_angle
  stack = volume_list_root->stack;
  new_stack = install_stack( new_stack_name, stack, dim, 4 );
  new_stack->data_type = 2;
  new_stack->status = new_stack->status | MAG_PHASE_BIT;

  tr_sec = tr/1000.;

  // Begin a loop over voxels.
  nfits = nfilters = 0;
  printf("n3d = %td\n",new_stack->n3d);
  for ( jv=0; jv<new_stack->n3d; jv++ )
    {
      if ( jv%1000 == 0 )
	printf("jv = %td\n",jv);
      if ( jv == ((JIP_INDEX)(20))*new_stack->dim.x + 50 )
	printf("**************\n");
      //      if ( jv == 20*new_stack->dim.x + 51 )
      //	exit(1);
      for (jt=0, max_signal=-1.e10, max_angle=90, volume=volume_list_root;
	   volume;
	   jt++, volume=volume->next)
	{
	  signal_array[jt] = volume_amp(volume,jv); // signal as a function of time
	  //	  printf("flip[%d] = %g, signal[%d] = %g\n",jt,flip_array[jt],jt,signal_array[jt]);
	  if ( signal_array[jt] > max_signal )
	    {
	      max_signal = signal_array[jt];
	      max_angle  = flip_array[jt];
	    }
	}
      printf("max signal = %g, max angle = %g\n",max_signal,max_angle);
      // Now fit r1 for this pixel.
      //
      // Data might be pre-filtered using the "magnitude-window" or "magnitude-window-together"
      // commands, which set points to zero if they fall outside the "filter-range".
      // Do not perform a fit if every magnitude value is equal to zero.
      // If SOME but not ALL points are zero, limit the number of the points
      // to the first set of non-zero points in the series. This approach allows
      // different number of values to be used at each voxel so that the noise floor
      // will not corrupt r1 values.
      for (jt=0; jt<nt; jt++)
	allzero &= (signal_array[jt] == 0.);
      if ( allzero )
	t1 = 0.;
      else
	{
	  ex  = exp((double)(-tr_sec*1.));
	  fraction = sin((double)(PI/180.*max_angle)) * (1. - ex)/(1.-ex*cos((double)(PI/180.*max_angle)));
	  s0 = max_signal / fraction;
	  r1 = 1.;  // initialize with t1 = 1 sec
	  rel_angle = 1.;
	  fit_r1_from_flip_angles(nt, flip_array, signal_array, tr_sec, // input
				  &s0, &r1, &rel_angle);                    // i/o
	  t1 = 1./r1;
	  /* Remove values which are not in a reasonable range. */
	  nfits++;
	  if ( filter_range.x < filter_range.y &&
	       ( t1 < filter_range.x || t1 > filter_range.y ) )
	    {
	      t1 = 0.;
	      nfilters++;
	    }
	}
      i1 = jv;
      i2 = i1 + new_stack->n3d;
      i3 = i2 + new_stack->n3d;
      i4 = i3 + new_stack->n3d;
      new_stack->SpaceTime.fcData[i1].imag = new_stack->SpaceTime.fcData[i2].imag
	= new_stack->SpaceTime.fcData[i3].imag = new_stack->SpaceTime.fcData[i4].imag = 0;
      if ( t1 == 0. )
	new_stack->SpaceTime.fcData[i1].real = new_stack->SpaceTime.fcData[i2].real
	  = new_stack->SpaceTime.fcData[i3].real = new_stack->SpaceTime.fcData[i4].real = 0;
      else
	{
	  new_stack->SpaceTime.fcData[i1].real = r1;
	  new_stack->SpaceTime.fcData[i2].real = t1;
	  new_stack->SpaceTime.fcData[i3].real = s0;
	  new_stack->SpaceTime.fcData[i4].real = rel_angle;
	}
    }
  /*
  ** Free the temporary arrays used in the fitting procedure.
  */
  free_double_vector(flip_array);
  free_double_vector(signal_array);
  /*
  ** Report fitting errors.
  */
  if ( !(flags&MUTE_BIT) )
    {
      printf("Of %d fits, %f %c were filtered.\n",nfits,
	     100.*nfilters/(float)nfits,37);
      printf("The map stack contains 4 images in the order R1, T1, S0, relative_angle.\n");
    }
  return(0);
}

int map_t1_from_multiple_tr( int theta, char *new_stack_name )
// create t1 map using S = S0 * sin(theta) * (1-exp(-tr/t1)) / (1-cos(theta)*exp(-tr/t1))
// maps: s0, r1, t1, relative_angle:
//
// where these are input: tr (fixed, in ms), angle (in map-array), series of volumes
//   and these are fit  : s0, r1, rel_angle
//
// equations:
//
// ex = exp(-tr*r1)
// phi = angle * rel_angle,
// s = s0 * sin(phi) * (1 - ex)/(1-ex*cos(phi))
//
{
  IPOINT3D dim;
  int nt, jt;JIP_INDEX jv, i1, i2, i3, i4;
  int nfits, nfilters;
  BOOL allzero;
  static int first_time = 1;
  double *tr_array, *signal_array;
  float s0, t1, r1, rel_angle;
  float flip_angle;
  struct data_stack *stack, *new_stack;
  struct volume_list  *volume;
  extern struct volume_list *volume_list_root;
  extern int flags;
  extern POINT3D filter_range;
  extern int nmap_array;
  extern float *map_array;

  // Make sure that all volumes have the same dimensions.
  if ( inconsistent_3d_dimensions(&dim,&nt) ) return(1);

  // Warn the user to set the `filter-range' variable.
  if ( nt != nmap_array )
    {
      printf("Error: # of time points (%d) != number of TR values (%d).\n",nt,nmap_array);
      printf("Use `set map-array ...'\n");
      exit(1);
    }
  else if ( nt < 4 )
    {
      printf("Error: there must be > 3 images to fit the 3 parameters.\n");
      return(1);
    }
  else if ( first_time )
    {
      printf("\nRemember: the `filter-range' variable is always applied to T1 maps.\n\n");
      first_time = 0;
    }

  printf("nt = %d, nmap_array = %d\n",nt,nmap_array);

  tr_array     = double_vector(nt);
  signal_array = double_vector(nt);
  for (jt=0; jt<nmap_array; jt++)
    tr_array[jt] = map_array[jt];

  // Install the new_stack in memory - 4 volumes for T1, R1, S0, rel_angle
  stack = volume_list_root->stack;
  new_stack = install_stack( new_stack_name, stack, dim, 4 );
  new_stack->data_type = 2;
  new_stack->status = new_stack->status | MAG_PHASE_BIT;

  flip_angle = (float)theta * PI/180.;
  printf("fixed flip_angle = %g\n",flip_angle);

  // Begin a loop over voxels.
  nfits = nfilters = 0;
  printf("n3d = %td\n",new_stack->n3d);
  for ( jv=0; jv<new_stack->n3d; jv++ )
    {
      if ( jv%1000 == 0 )
	printf("jv = %td\n",jv);
      if ( jv == ((JIP_INDEX)(20))*new_stack->dim.x + 50 )
	printf("**************\n");
      //      if ( jv == 20*new_stack->dim.x + 51 )
      //	exit(1);
      for (jt=0, volume=volume_list_root; volume; jt++, volume=volume->next)
	signal_array[jt] = volume_amp(volume,jv); // signal as a function of time
      // Now fit r1 for this pixel.
      //
      // Data might be pre-filtered using the "magnitude-window" or "magnitude-window-together"
      // commands, which set points to zero if they fall outside the "filter-range".
      // Do not perform a fit if every magnitude value is equal to zero.
      // If SOME but not ALL points are zero, limit the number of the points
      // to the first set of non-zero points in the series. This approach allows
      // different number of values to be used at each voxel so that the noise floor
      // will not corrupt r1 values.
      allzero = TRUE;
      for (jt=0; jt<nt; jt++)
	allzero &= (signal_array[jt] == 0.);
      if ( allzero )
	t1 = 0.;
      else
	{
	  s0 = signal_array[nt-1];
	  r1 = 1.;  // initialize with t1 = 1 sec
	  rel_angle = 1.;
	  fit_r1_from_multiple_tr(nt, tr_array, signal_array, flip_angle, // input
				  &s0, &r1, &rel_angle);                          // i/o
	  t1 = 1./r1;
	  /* Remove values which are not in a reasonable range. */
	  nfits++;
	  if ( filter_range.x < filter_range.y &&
	       ( t1 < filter_range.x || t1 > filter_range.y ) )
	    {
	      t1 = 0.;
	      nfilters++;
	    }
	}
      i1 = jv;
      i2 = i1 + new_stack->n3d;
      i3 = i2 + new_stack->n3d;
      i4 = i3 + new_stack->n3d;
      new_stack->SpaceTime.fcData[i1].imag = new_stack->SpaceTime.fcData[i2].imag
	= new_stack->SpaceTime.fcData[i3].imag = new_stack->SpaceTime.fcData[i4].imag = 0;
      if ( t1 == 0. )
	new_stack->SpaceTime.fcData[i1].real = new_stack->SpaceTime.fcData[i2].real
	  = new_stack->SpaceTime.fcData[i3].real = new_stack->SpaceTime.fcData[i4].real = 0;
      else
	{
	  new_stack->SpaceTime.fcData[i1].real = r1;
	  new_stack->SpaceTime.fcData[i2].real = t1;
	  new_stack->SpaceTime.fcData[i3].real = s0;
	  new_stack->SpaceTime.fcData[i4].real = rel_angle;
	}
    }
  /*
  ** Free the temporary arrays used in the fitting procedure.
  */
  free_double_vector(tr_array);
  free_double_vector(signal_array);
  /*
  ** Report fitting errors.
  */
  if ( !(flags&MUTE_BIT) )
    {
      printf("Of %d fits, %f %c were filtered.\n",nfits,
	     100.*nfilters/(float)nfits,37);
      printf("The map stack contains 4 images in the order R1, T1, S0, relative_angle.\n");
    }
  return(0);
}

int map_t1_Barral_MRM2010( char *new_stack_name )
//
// Map T1 values following Barrel ... Nishimura, Magn Reson Med 2010
//
// The signal model is S(TI) = a + b*exp(-TI/T1) for a series of TI values
// where a and b are complex (5 parameters: a, b, TI)
//
// input:
//    TI values are stored in map_array
//    data are assumed to complex (real, imag)
// output:
//    a new stack named "new_stack_name" containing
//    0) T1
//    1) R1
//    2) Sinf = |S(infinity)| = sqrt( (a+b)(a+b)* )
//
{
  IPOINT3D dim;
  int nt, jt;JIP_INDEX jv, i1, i2, i3;
  int nfits;
  BOOL allzero;
  FCOMPLEX *signal_array, a_complex, b_complex;
  float T1, R1, Sinf;
  struct data_stack *stack, *new_stack;
  struct volume_list  *volume;
  extern struct volume_list *volume_list_root;
  extern int flags;
  extern POINT3D filter_range;
  extern int nmap_array;
  extern float *map_array;
  static BOOL first_time = TRUE;

  // Make sure that all volumes have the same dimensions.
  if ( inconsistent_3d_dimensions(&dim,&nt) ) return(1);

  // Warn the user to set the `filter-range' variable.
  if ( nt != nmap_array )
    {
      printf("Error: # of time points (%d) != number of TI values (%d).\n",nt,nmap_array);
      return(1);
    }
  else if ( nt < 3 )
    {
      printf("Error: there must be >= 3 complex images to fit the 5 parameters.\n");
      return(1);
    }
  else if ( first_time )
    {
      if ( filter_range.x < filter_range.y )
	printf("\nT1 values will be tested from %d to %d ms with 1 ms steps.\n",
	       (int)filter_range.x,(int)filter_range.y);
      else
	printf("\nT1 values will be tested from %d to %d ms with 1 ms steps.\n",0,5000);
      printf("The range can be adjusted by setting the `filter-range' variable.\n");
      first_time = FALSE;
    }

  // Allocate TI and S(TI)
  signal_array = fcomplex_vector(nt);

  // Install the new_stack in memory - 3 volumes for T1, R1, Sinf
  stack = volume_list_root->stack;
  new_stack = install_stack( new_stack_name, stack, dim, 3 );
  new_stack->data_type = 2;
  new_stack->status = new_stack->status | MAG_PHASE_BIT;

  // Begin a loop over voxels
  nfits = 0;
  for ( jv=0; jv<new_stack->n3d; jv++ )
    {
      if ( jv%(new_stack->n3d/20) == 0 )
	{
	  fprintf(stdout,"\rcomputing T1 map: %td %c done",5*jv/(new_stack->n3d/20),37);
	  fflush(stdout);
	}
      for (jt=0, volume=volume_list_root; volume; jt++, volume=volume->next)
	signal_array[jt] = volume->SpaceTime.fcData[jv];
      // Data might be pre-filtered using the "magnitude-window" or "magnitude-window-together"
      // commands, which set points to zero if they fall outside the "filter-range".
      // Do not perform a fit if every magnitude value is equal to zero.
      allzero = TRUE;
      for (jt=0, volume=volume_list_root; volume; jt++, volume=volume->next)
	allzero &= (volume_amp(volume,jv) == 0.);
      if ( allzero )
	T1 = 0.;
      else
	{
	  // Determine the 5 parameters: a, b, T1
	  grid_search_t1(0, nt, signal_array, &a_complex, &b_complex, &T1);
	  grid_search_t1(1, nt, signal_array, &a_complex, &b_complex, &T1);
	  R1   = 1000./T1;
	  Sinf = sqrt((double)(a_complex.real*a_complex.real + a_complex.imag*a_complex.imag));
	  // Remove values which are not in a reasonable range.
	  nfits++;
	  if ( filter_range.x < filter_range.y &&
	       ( T1 < filter_range.x || T1 > filter_range.y ) )
	    T1 = 0.;
	}
      i1 = jv;
      i2 = i1 + new_stack->n3d;
      i3 = i2 + new_stack->n3d;
      new_stack->SpaceTime.fcData[i1].imag = new_stack->SpaceTime.fcData[i2].imag
	= new_stack->SpaceTime.fcData[i3].imag = 0;
      if ( T1 == 0. )
	new_stack->SpaceTime.fcData[i1].real = new_stack->SpaceTime.fcData[i2].real
	  = new_stack->SpaceTime.fcData[i3].real = 0;
      else
	{
	  new_stack->SpaceTime.fcData[i1].real = R1;
	  new_stack->SpaceTime.fcData[i2].real = T1;
	  new_stack->SpaceTime.fcData[i3].real = Sinf;
	}
    }
  // Free the temporary arrays used in the fitting procedure.
  free_fcomplex_vector(signal_array);
  printf("\n");
  return(0);
}

void grid_search_t1( int mode, int nt, FCOMPLEX *signal_array, FCOMPLEX *A, FCOMPLEX *B, float *T1_opt)
// mode: 0 = search 1 to 5000 ms in steps of 100 ms
//       1 = search from T1_opt-100 to T1_opt+100 ms in steps of 1 ms
{
  FCOMPLEX gamma;
  double T1, psi, arg, max;
  int iStart, iStop, iStep, jt1;
  extern POINT3D filter_range;

  if ( mode == 0 )
    {
      if ( filter_range.x < filter_range.y )
	{
	  iStart = filter_range.x;
	  iStop  = filter_range.y;
	}
      else
	{
	  iStart = 0;
	  iStop  = 5000;
	}
      iStep  = 100;
    }
  else
    {
      iStart = *T1_opt - 100;
      iStop  = *T1_opt + 100;
      iStep  = 1;
    }

  max = *T1_opt = 0.;
  for (jt1=iStart; jt1<iStop; jt1+=iStep)
    {
      T1    = MAX(1,jt1);
      gamma = Barral_gamma( nt, signal_array, T1 );
      psi   = Barral_psi(   nt, signal_array, T1 );
      arg   = (gamma.real*gamma.real + gamma.imag*gamma.imag) / psi;
      if ( arg > max )
	{
	  max    = arg;
	  *T1_opt = T1;
	}
    }
  // The best T1 value has been found. Calculate A and B.
  gamma = Barral_gamma( nt, signal_array, *T1_opt );
  psi   = Barral_psi(   nt, signal_array, *T1_opt );
  B->real = gamma.real/psi;
  B->imag = gamma.imag/psi;
  // Compute A
  *A = Barral_A( nt, signal_array, *T1_opt, *B);
}

FCOMPLEX Barral_gamma( int nt, FCOMPLEX *signal_array, double T1 )
{
  FCOMPLEX gamma, sum1, sum3, zero={0.,0.};
  double TI, sum2;
  int jt;
  extern int nmap_array;
  extern float *map_array;

  if ( nt != nmap_array )
    {
      printf("Programming error in Barrel_psi()\n");
      exit(1);
    }

  sum1 = sum3 = gamma = zero;
  sum2 = 0.;
  // Compute the sums
  for (jt=0; jt<nt; jt++)
    {
      TI = map_array[jt];
      sum1.real += signal_array[jt].real * exp(-TI/T1);
      sum1.imag += signal_array[jt].imag * exp(-TI/T1);
      sum2      += exp(-TI/T1);
      sum3.real += signal_array[jt].real;
      sum3.imag += signal_array[jt].imag;
    }
  gamma.real = sum1.real - 1./(double)nt * sum2 * sum3.real;
  gamma.imag = sum1.imag - 1./(double)nt * sum2 * sum3.imag;
  return(gamma);
}

double Barral_psi( int nt, FCOMPLEX *signal_array, double T1 )
{
  double TI, psi, sum1, sum2;
  int jt;
  extern int nmap_array;
  extern float *map_array;

  if ( nt != nmap_array )
    {
      printf("Programming error in Barrel_psi()\n");
      exit(1);
    }
  sum1 = sum2 = 0.;
  // Compute the sums
  for (jt=0; jt<nt; jt++)
    {
      TI = map_array[jt];
      sum1 += exp(-2*TI/T1);
      sum2 += exp(-TI/T1);
    }
  psi = sum1 - 1./(double)nt * sum2*sum2;
  return(psi);
}

FCOMPLEX Barral_A( int nt, FCOMPLEX *signal_array, double T1, FCOMPLEX B )
{
  FCOMPLEX sum1, A;
  double TI, sum2;
  int jt;
  extern int nmap_array;
  extern float *map_array;

  if ( nt != nmap_array )
    {
      printf("Programming error in Barrel_psi()\n");
      exit(1);
    }

  sum1.real = sum1.imag = sum2 = 0.;
  // Compute the sums
  for (jt=0; jt<nt; jt++)
    {
      TI = map_array[jt];
      sum1.real += signal_array[jt].real;
      sum1.imag += signal_array[jt].imag;
      sum2      += exp(-TI/T1);
    }
  A.real = 1./(double)nt * ( sum1.real - B.real*sum2 );
  A.imag = 1./(double)nt * ( sum1.imag - B.imag*sum2 );
  return(A);
}
