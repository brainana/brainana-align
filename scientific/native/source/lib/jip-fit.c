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
#include "lmmin.h"
#include "lm_eval.h"
#include "util.h"

struct SimpleGLM
{
  int ntime;
  int ncoeff;
  int *include_time;
  float **X_tc;
  float **XTXm1_cc;
  float **XTXm1XT_ct;
  float *Beta_c;
};

struct SimpleGLM PolyGLM;
struct SimpleGLM CubicBSplineGLM;
struct SimpleGLM GeneralGLM;

float tr_for_t1_fit;
float flip_angle_for_t1_fit;


int PolynomialDefineGLM( int ncoeff, int ntime, int *include )
     // Define the basis vectors of a general linear model (GLM)
     // for fitting a polynomial with evenly spaced points in x.
{
  int iRowFail;
  int jtime, jorder, jorder1;
  static int first_time = 1;

  if ( ncoeff < 1 )
    {
      printf("Error: the order of the polynomial (%d) must be >= 1.\n",ncoeff);
      return(1);
    }
  else if ( ncoeff > 4 )
    {
      printf("Error: the order of the polynomial (%d) must be <= 3.\n",ncoeff);
      return(1);
    }
  else if ( ntime < 2 )
    {
      printf("Error: the # of points (%d) must be >= 2.\n",ntime);
      return(1);
    }

  // Free existing arrays.
  if ( ! first_time )
    {
      free_float_matrix(PolyGLM.X_tc,PolyGLM.ntime,PolyGLM.ncoeff);
      free_float_matrix(PolyGLM.XTXm1_cc,PolyGLM.ncoeff,PolyGLM.ncoeff);
      free_float_matrix(PolyGLM.XTXm1XT_ct,PolyGLM.ncoeff,PolyGLM.ntime);
      free_float_vector(PolyGLM.Beta_c);
    }
  else
    first_time = 0;

  // Set the number of ordericients in the GLM:
  PolyGLM.ntime  = ntime;
  PolyGLM.ncoeff = ncoeff;

  // Allocate and initialize new arrays.
  PolyGLM.X_tc       = float_matrix(PolyGLM.ntime,PolyGLM.ncoeff);
  PolyGLM.XTXm1_cc   = float_matrix(PolyGLM.ncoeff,PolyGLM.ncoeff);
  PolyGLM.XTXm1XT_ct = float_matrix(PolyGLM.ncoeff,PolyGLM.ntime);
  PolyGLM.Beta_c     = float_vector(PolyGLM.ncoeff);

  for (jorder=0; jorder<PolyGLM.ncoeff; jorder++)
    {
      PolyGLM.Beta_c[jorder] = 0.;
      for (jorder1=0; jorder1<PolyGLM.ncoeff; jorder1++)
	PolyGLM.XTXm1_cc[jorder][jorder1] = 0.;
      for (jtime=0; jtime<PolyGLM.ntime; jtime++)
	PolyGLM.X_tc[jtime][jorder] = PolyGLM.XTXm1XT_ct[jorder][jtime] = 0.;
    }

  // Create the basis functions.
  // The constant offset values for each run ...
  if ( PolyGLM.ncoeff >= 1 )
    {
      for ( jtime=0; jtime<PolyGLM.ntime; jtime++ )
	PolyGLM.X_tc[jtime][0] = 1.;
    }
  // The drift corrections for each run ...
  // Give each drift correction an average value of zero.
  // 1st order: set range 0->1, then subtract 1/2 to make zero mean.
  if ( PolyGLM.ncoeff >= 2 )
    {
      for ( jtime=0; jtime<PolyGLM.ntime; jtime++ )
	PolyGLM.X_tc[jtime][1] = (float)jtime / (float)ntime - 0.5;
    }
  // 2nd order drift, if included ...
  // Set range 0->1, then subtract 1/3 to make zero mean.
  if ( PolyGLM.ncoeff >= 3 )
    {
      for ( jtime=0; jtime<PolyGLM.ntime; jtime++ )
	PolyGLM.X_tc[jtime][2] = (float)(jtime*jtime) / (float)(ntime*ntime) - 1./3.;
    }
  // 3rd order drift, if included ...
  if ( PolyGLM.ncoeff == 4 )
    {
      for ( jtime=0; jtime<PolyGLM.ntime; jtime++ )
	PolyGLM.X_tc[jtime][3] = (float)(jtime*jtime*jtime) / (float)(ntime*ntime*ntime) - 1./4.;
    }

  // Define the exclusion vector
  if ( PolyGLM.include_time )
    free_int_vector(PolyGLM.include_time);
  PolyGLM.include_time = int_vector(PolyGLM.ntime);
  // Initialize the vector with 1's.
  for (jtime=0; jtime<PolyGLM.ntime; jtime++)
    PolyGLM.include_time[jtime] = include[jtime];

  // Calculate (X_transpose * X) by summing over the time index.
  for (jorder=0; jorder<PolyGLM.ncoeff; jorder++)
    {
      for (jorder1=0; jorder1<PolyGLM.ncoeff; jorder1++)
	{
	  for (jtime=0; jtime<PolyGLM.ntime; jtime++)
	    PolyGLM.XTXm1_cc[jorder][jorder1] += PolyGLM.include_time[jtime]
	      * PolyGLM.X_tc[jtime][jorder] * PolyGLM.X_tc[jtime][jorder1];
	}
    }

  // Invert to get (X_transpose * X)^-1
  rInvertSquareMatrix(PolyGLM.ncoeff, PolyGLM.XTXm1_cc, &iRowFail);

  // Calculate (X_transpose * X)^-1 * X_transpose
  // by summing over the shared index.
  for (jorder=0; jorder<PolyGLM.ncoeff; jorder++)
    {
      for (jtime=0; jtime<PolyGLM.ntime; jtime++)
	{
	  for (jorder1=0; jorder1<PolyGLM.ncoeff; jorder1++)
	    PolyGLM.XTXm1XT_ct[jorder][jtime] += PolyGLM.include_time[jtime]
	      * PolyGLM.XTXm1_cc[jorder][jorder1] * PolyGLM.X_tc[jtime][jorder1];
	}
    }

  return(0);
}

int PolynomialFitGLM( int ntime, float *data, float *fit )
{
  float y_prime;
  int jtime, jorder;
  
  // Make sure the GLM has been initialized.
  if ( PolyGLM.ncoeff < 1 )
    {
      printf("Error in PolynomialFitGLM: order of polynomial (%d) < 1.\n",PolyGLM.ncoeff);
      return(1);
    }
  // Make sure the GLM model and the input data are consistent.
  if ( ntime != PolyGLM.ntime )
    {
      printf("Error: the number of data time points (%d) does not match the number in the GLM (%d).\n",
	     ntime,PolyGLM.ntime);
      exit(1);
    }

  // Calculate the ordericients of the hemodynamic function Beta_c.
  for (jorder=0; jorder<PolyGLM.ncoeff; jorder++)
    {
      PolyGLM.Beta_c[jorder] = 0.;
      // Calculate X_transpose * Y, summing over time
      for (jtime=0; jtime<PolyGLM.ntime; jtime++)
	{
	  y_prime = PolyGLM.include_time[jtime] * data[jtime];
	  PolyGLM.Beta_c[jorder] += PolyGLM.XTXm1XT_ct[jorder][jtime] * y_prime;
	}
    }
  // Compute the fit.
  for (jtime=0; jtime<PolyGLM.ntime; jtime++)
    {
      for (jorder=0, fit[jtime]=0.; jorder<PolyGLM.ncoeff; jorder++)
	fit[jtime] += PolyGLM.X_tc[jtime][jorder] * PolyGLM.Beta_c[jorder];
    }

  return(0);
}

int PolynomialRemoveGLM( int ibasis, int ntime, float *data, float *fit )
     // ibasis: starting basis function (0 removes all, 1 removes drift terms)
{
  int jtime, jcoeff;

  if ( ntime != PolyGLM.ntime )
    {
      printf("Error: # time points (%d) does not match GLM (%d) in PolynomialRemoveGLM()\n",
	     ntime,PolyGLM.ntime);
      return(1);
    }
  /*
  ** Remove all effects of no interest (e.g., baseline terms).
  */
  for (jtime=0; jtime<ntime; jtime++)
    {
      for ( jcoeff=ibasis; jcoeff<PolyGLM.ncoeff; jcoeff ++ )
	{
	  fit[jtime]  -= PolyGLM.X_tc[jtime][jcoeff] * PolyGLM.Beta_c[jcoeff];
	  data[jtime] -= PolyGLM.X_tc[jtime][jcoeff] * PolyGLM.Beta_c[jcoeff];
	}
    }
  return(0);
}

int GeneralDefineGLM( int ncoeff, int ntime, int *include, float **X_tc )
     // Define the basis vectors of a general linear model (GLM)
     // based upon a hemodynamic response function (IRF) and
     // information about the times for the stimuli.
{
  int jtime, jcoeff, jcoeff1, iRowFail;
  static int first_time = 1;

  if ( ncoeff < 1 )
    {
      printf("Error: the # of coefficients in the GLM (%d) must be >= 1.\n",ncoeff);
      return(1);
    }
  else if ( ntime < 2 )
    {
      printf("Error: the # of points (%d) must be >= 2.\n",ntime);
      return(1);
    }

  // Free existing arrays.
  if ( ! first_time )
    {
      free_float_matrix(GeneralGLM.X_tc,GeneralGLM.ntime,GeneralGLM.ncoeff);
      free_float_matrix(GeneralGLM.XTXm1_cc,GeneralGLM.ncoeff,GeneralGLM.ncoeff);
      free_float_matrix(GeneralGLM.XTXm1XT_ct,GeneralGLM.ncoeff,GeneralGLM.ntime);
      free_float_vector(GeneralGLM.Beta_c);
      free_int_vector(GeneralGLM.include_time);
    }
  else
    first_time = 0;

  // Copy input to the GLM structure.
  GeneralGLM.ntime  = ntime;
  GeneralGLM.ncoeff = ncoeff;

  // Allocate and initialize new arrays.
  GeneralGLM.X_tc       = float_matrix(GeneralGLM.ntime,GeneralGLM.ncoeff);
  GeneralGLM.XTXm1_cc   = float_matrix(GeneralGLM.ncoeff,GeneralGLM.ncoeff);
  GeneralGLM.XTXm1XT_ct = float_matrix(GeneralGLM.ncoeff,GeneralGLM.ntime);
  GeneralGLM.Beta_c     = float_vector(GeneralGLM.ncoeff);

  for (jcoeff=0; jcoeff<GeneralGLM.ncoeff; jcoeff++)
    {
      GeneralGLM.Beta_c[jcoeff] = 0.;
      for (jcoeff1=0; jcoeff1<GeneralGLM.ncoeff; jcoeff1++)
	GeneralGLM.XTXm1_cc[jcoeff][jcoeff1] = 0.;
      for (jtime=0; jtime<GeneralGLM.ntime; jtime++)
	{
	  // Copy the input design matrix into the GLM structure
	  GeneralGLM.X_tc[jtime][jcoeff] = X_tc[jtime][jcoeff];
	  GeneralGLM.XTXm1XT_ct[jcoeff][jtime] = 0.;
	}
    }

  // Define the exclusion vector
  GeneralGLM.include_time = int_vector(GeneralGLM.ntime);
  // Initialize the vector with 1's.
  for (jtime=0; jtime<GeneralGLM.ntime; jtime++)
    GeneralGLM.include_time[jtime] = include[jtime];

  // Calculate (X_transpose * X) by summing over the time index.
  for (jcoeff=0; jcoeff<GeneralGLM.ncoeff; jcoeff++)
    {
      for (jcoeff1=0; jcoeff1<GeneralGLM.ncoeff; jcoeff1++)
	{
	  for (jtime=0; jtime<GeneralGLM.ntime; jtime++)
	    GeneralGLM.XTXm1_cc[jcoeff][jcoeff1] += GeneralGLM.include_time[jtime]
	      * GeneralGLM.X_tc[jtime][jcoeff] * GeneralGLM.X_tc[jtime][jcoeff1];
	}
    }

  // Invert to get (X_transpose * X)^-1
  rInvertSquareMatrix(GeneralGLM.ncoeff, GeneralGLM.XTXm1_cc, &iRowFail);

  // Calculate (X_transpose * X)^-1 * X_transpose
  // by summing over the shared index.
  for (jcoeff=0; jcoeff<GeneralGLM.ncoeff; jcoeff++)
    {
      for (jtime=0; jtime<GeneralGLM.ntime; jtime++)
	{
	  for (jcoeff1=0; jcoeff1<GeneralGLM.ncoeff; jcoeff1++)
	    GeneralGLM.XTXm1XT_ct[jcoeff][jtime] += GeneralGLM.include_time[jtime]
	      * GeneralGLM.XTXm1_cc[jcoeff][jcoeff1] * GeneralGLM.X_tc[jtime][jcoeff1];
	}
    }

  return(0);
}

int GeneralFitGLM( int ntime, float *data, float *fit )
{
  float y_prime;
  int jtime, jcoeff;
  
  // Make sure the GLM has been initialized.
  if ( GeneralGLM.ncoeff < 1 )
    {
      printf("Error in GeneralFitGLM: order of polynomial (%d) < 1.\n",GeneralGLM.ncoeff);
      return(1);
    }
  // Make sure the GLM model and the input data are consistent.
  if ( ntime != GeneralGLM.ntime )
    {
      printf("Error: the number of data time points (%d) does not match the number in the GLM (%d).\n",
	     ntime,GeneralGLM.ntime);
      exit(1);
    }

  // Calculate the ordericients of the hemodynamic function Beta_c.
  for (jcoeff=0; jcoeff<GeneralGLM.ncoeff; jcoeff++)
    {
      GeneralGLM.Beta_c[jcoeff] = 0.;
      // Calculate X_transpose * Y, summing over time
      for (jtime=0; jtime<GeneralGLM.ntime; jtime++)
	{
	  y_prime = GeneralGLM.include_time[jtime] * data[jtime];
	  GeneralGLM.Beta_c[jcoeff] += GeneralGLM.XTXm1XT_ct[jcoeff][jtime] * y_prime;
	}
    }
  // Compute the fit.
  for (jtime=0; jtime<GeneralGLM.ntime; jtime++)
    {
      for (jcoeff=0, fit[jtime]=0.; jcoeff<GeneralGLM.ncoeff; jcoeff++)
	fit[jtime] += GeneralGLM.X_tc[jtime][jcoeff] * GeneralGLM.Beta_c[jcoeff];
    }

  return(0);
}

int GeneralRemoveGLM( int ibasis, int ntime, float *data, float *fit )
     // ibasis: starting basis function (0 removes all, 1 removes drift terms)
{
  int jtime, jcoeff;

  if ( ntime != GeneralGLM.ntime )
    {
      printf("Error: # time points (%d) does not match GLM (%d) in GeneralRemoveGLM()\n",
	     ntime,GeneralGLM.ntime);
      return(1);
    }
  // Remove the GLM fit, which is computed in GeneralFitGLM
  for (jtime=0; jtime<ntime; jtime++)
    {
      for ( jcoeff=ibasis; jcoeff<GeneralGLM.ncoeff; jcoeff ++ )
	{
	  fit[jtime]  -= GeneralGLM.X_tc[jtime][jcoeff] * GeneralGLM.Beta_c[jcoeff];
	  data[jtime] -= GeneralGLM.X_tc[jtime][jcoeff] * GeneralGLM.Beta_c[jcoeff];
	}
    }
  return(0);
}

int CubicBSplineGLM_DefineGLM( int ntime )
     // Define the basis vectors of a general linear model (GLM)
     // for fitting a cubic B-spline with ncoeff = ntime
{
  int iRowFail;
  int jtime, jcoeff, jcoeff1;
  static int first_time = 1;

  if ( ntime < 4 )
    {
      printf("Error: the # of points (%d) must be >= 4.\n",ntime);
      return(1);
    }

  // Free existing arrays.
  if ( ! first_time )
    {
      free_float_matrix(CubicBSplineGLM.X_tc,CubicBSplineGLM.ntime,CubicBSplineGLM.ncoeff);
      free_float_matrix(CubicBSplineGLM.XTXm1_cc,CubicBSplineGLM.ncoeff,CubicBSplineGLM.ncoeff);
      free_float_matrix(CubicBSplineGLM.XTXm1XT_ct,CubicBSplineGLM.ncoeff,CubicBSplineGLM.ntime);
      free_float_vector(CubicBSplineGLM.Beta_c);
    }
  else
    first_time = 0;

  // Set the number of ordericients in the GLM:
  CubicBSplineGLM.ntime  = ntime;
  CubicBSplineGLM.ncoeff = ntime/1;

  // Allocate and initialize new arrays.
  CubicBSplineGLM.X_tc       = float_matrix(CubicBSplineGLM.ntime,CubicBSplineGLM.ncoeff);
  CubicBSplineGLM.XTXm1_cc   = float_matrix(CubicBSplineGLM.ncoeff,CubicBSplineGLM.ncoeff);
  CubicBSplineGLM.XTXm1XT_ct = float_matrix(CubicBSplineGLM.ncoeff,CubicBSplineGLM.ntime);
  CubicBSplineGLM.Beta_c     = float_vector(CubicBSplineGLM.ncoeff);

  // Initialize
  for (jcoeff=0; jcoeff<CubicBSplineGLM.ncoeff; jcoeff++)
    {
      CubicBSplineGLM.Beta_c[jcoeff] = 0.;
      for (jcoeff1=0; jcoeff1<CubicBSplineGLM.ncoeff; jcoeff1++)
	CubicBSplineGLM.XTXm1_cc[jcoeff][jcoeff1] = 0.;
      for (jtime=0; jtime<CubicBSplineGLM.ntime; jtime++)
	CubicBSplineGLM.X_tc[jtime][jcoeff] = CubicBSplineGLM.XTXm1XT_ct[jcoeff][jtime] = 0.;
    }

  // Create the basis functions.
  for ( jcoeff=0; jcoeff<CubicBSplineGLM.ncoeff; jcoeff++)
    {
      for (jtime=0; jtime<CubicBSplineGLM.ntime; jtime++)
	{
	  /*
	  if ( jtime == 4*jcoeff )
	    CubicBSplineGLM.X_tc[jtime][jcoeff] = 1.;
	  else if ( jtime == 4*jcoeff-1 || jtime == 4*jcoeff+1 )
	    CubicBSplineGLM.X_tc[jtime][jcoeff] = 0.918;
	  else if ( jtime == 4*jcoeff-2 || jtime == 4*jcoeff+2 )
	    CubicBSplineGLM.X_tc[jtime][jcoeff] = 0.71875;
	  else if ( jtime == 4*jcoeff-3 || jtime == 4*jcoeff+3 )
	    CubicBSplineGLM.X_tc[jtime][jcoeff] = 0.473;
	  else if ( jtime == 4*jcoeff-4 || jtime == 4*jcoeff+4 )
	    CubicBSplineGLM.X_tc[jtime][jcoeff] = 0.25;
	  else if ( jtime == 4*jcoeff-5 || jtime == 4*jcoeff+5 )
	    CubicBSplineGLM.X_tc[jtime][jcoeff] = 0.105;
	  else if ( jtime == 4*jcoeff-6 || jtime == 4*jcoeff+6 )
	    CubicBSplineGLM.X_tc[jtime][jcoeff] = 0.03125;
	  else if ( jtime == 4*jcoeff-7 || jtime == 4*jcoeff+7 )
	    CubicBSplineGLM.X_tc[jtime][jcoeff] = 0.0039;
	  */
	  //
	  /*
	  if ( jtime == 2*jcoeff )
	    CubicBSplineGLM.X_tc[jtime][jcoeff] = 1.;
	  else if ( jtime == 2*jcoeff-1 || jtime == 2*jcoeff+1 )
	    CubicBSplineGLM.X_tc[jtime][jcoeff] = 0.71875;
	  else if ( jtime == 2*jcoeff-2 || jtime == 2*jcoeff+2 )
	    CubicBSplineGLM.X_tc[jtime][jcoeff] = 0.25;
	  else if ( jtime == 2*jcoeff-3 || jtime == 2*jcoeff+3 )
	    CubicBSplineGLM.X_tc[jtime][jcoeff] = 0.03125;
	  */
	  //
	  if ( jtime == jcoeff )
	    CubicBSplineGLM.X_tc[jtime][jcoeff] = 1.;
	  else if ( jtime == jcoeff-1 || jtime == jcoeff+1 )
	  CubicBSplineGLM.X_tc[jtime][jcoeff] = 0.25;
	}
    }

  // Calculate (X_transpose * X) by summing over the time index.
  for (jcoeff=0; jcoeff<CubicBSplineGLM.ncoeff; jcoeff++)
    {
      for (jcoeff1=0; jcoeff1<CubicBSplineGLM.ncoeff; jcoeff1++)
	{
	  for (jtime=0; jtime<CubicBSplineGLM.ntime; jtime++)
	    CubicBSplineGLM.XTXm1_cc[jcoeff][jcoeff1] += CubicBSplineGLM.X_tc[jtime][jcoeff] * CubicBSplineGLM.X_tc[jtime][jcoeff1];
	}
    }

  // Invert to get (X_transpose * X)^-1
  rInvertSquareMatrix(CubicBSplineGLM.ncoeff, CubicBSplineGLM.XTXm1_cc, &iRowFail);

  // Calculate (X_transpose * X)^-1 * X_transpose
  // by summing over the shared index.
  for (jcoeff=0; jcoeff<CubicBSplineGLM.ncoeff; jcoeff++)
    {
      for (jtime=0; jtime<CubicBSplineGLM.ntime; jtime++)
	{
	  for (jcoeff1=0; jcoeff1<CubicBSplineGLM.ncoeff; jcoeff1++)
	    CubicBSplineGLM.XTXm1XT_ct[jcoeff][jtime] += CubicBSplineGLM.XTXm1_cc[jcoeff][jcoeff1] * CubicBSplineGLM.X_tc[jtime][jcoeff1];
	}
    }

  return(0);
}

int CubicBSplineGLM_FitGLM( int ntime, float *data, float *fit )
{
  float y_prime;
  int jtime, jcoeff;
  
  // Make sure the GLM has been initialized.
  if ( CubicBSplineGLM.ncoeff < 2 )
    {
      printf("Error in CubicBPSpline_FitGLM: # coefficients (%d) < 2.\n",CubicBSplineGLM.ncoeff);
      return(1);
    }
  // Make sure the GLM model and the input data are consistent.
  if ( ntime != CubicBSplineGLM.ntime )
    {
      printf("Error: the number of data time points (%d) does not match the number in the GLM (%d).\n",
	     ntime,CubicBSplineGLM.ntime);
      exit(1);
    }

  // Calculate the coeffients of the hemodynamic function Beta_c.
  for (jcoeff=0; jcoeff<CubicBSplineGLM.ncoeff; jcoeff++)
    {
      CubicBSplineGLM.Beta_c[jcoeff] = 0.;
      // Calculate X_transpose * Y, summing over time
      for (jtime=0; jtime<CubicBSplineGLM.ntime; jtime++)
	{
	  y_prime = data[jtime];
	  CubicBSplineGLM.Beta_c[jcoeff] += CubicBSplineGLM.XTXm1XT_ct[jcoeff][jtime] * y_prime;
	}
    }
  printf("overwrite data with fit\n");
  // Compute the fit.
  for (jtime=0; jtime<CubicBSplineGLM.ntime; jtime++)
    {
      for (jcoeff=0, fit[jtime]=0.; jcoeff<CubicBSplineGLM.ncoeff; jcoeff++)
	fit[jtime] += CubicBSplineGLM.X_tc[jtime][jcoeff] * CubicBSplineGLM.Beta_c[jcoeff];
    }

  return(0);
}

int fit_gaussian(int npoints,
		 double *xarray,
		 double *yarray,
		 float *xcenter,  // guess & result for x center
		 float *amplitude,// guess & result for amplitude of Gaussian
		 float *sigma,    // sigma for Gaussian
		 float *yoffset)  // y offset of baseline
{
  double param[4];
  lm_control_type control;
  lm_data_type data;

  lm_initialize_control(&control);

  data.user_func = func_gaussian;
  data.user_x    = xarray;
  data.user_y    = yarray;
  
  //  param[0] = *yoffset;
  param[0] = *amplitude;
  param[1] = *xcenter;
  param[2] = *sigma;

  // perform the fit:
  lm_minimize(npoints, 3, param, lm_evaluate_default, lm_print_maps,
	      &data, &control);
  // print results:
  printf("status: %s after %d evaluations\n",
	 lm_shortmsg[control.info], control.nfev);
  //  *yoffset   = param[0];
  *amplitude = param[0];
  *xcenter   = param[1];
  *sigma     = param[2];

  return(0);
}

double func_gaussian(double x, double *p)
{
  double amp, xoff, sigma, ex, val;

  //  yoff  = p[0];
  amp   = p[0];
  xoff  = p[1];
  sigma = p[2];
  ex  = exp( (double)(-(x-xoff)*(x-xoff)/2./sigma/sigma) );
  //  val = yoff + amp * ex;
  val = amp * ex;
  return(val);
}

int fit_1_over_r2(int npoints,
		  double *xarray,
		  double *yarray,
		  float *xcenter,  // guess & result for x center
		  float *amplitude,// guess & result for amplitude of Gaussian
		  float *HWHM,     // sets HWHM of decay
		  float *yoffset)  // y offset of baseline
{
  double param[4];
  lm_control_type control;
  lm_data_type data;

  lm_initialize_control(&control);

  data.user_func = func_1_over_r2;
  data.user_x    = xarray;
  data.user_y    = yarray;
  
  //  param[0] = *yoffset;
  param[0] = *amplitude;
  param[1] = *xcenter;
  param[2] = *HWHM;

  // perform the fit:
  lm_minimize(npoints, 3, param, lm_evaluate_default, lm_print_maps,
	      &data, &control);
  // print results:
  printf("status: %s after %d evaluations\n",
	 lm_shortmsg[control.info], control.nfev);
  //  *yoffset   = param[0];
  *amplitude = param[0];
  *xcenter   = param[1];
  *HWHM      = param[2];

  return(0);
}

double func_1_over_r2(double x, double *p)
{
  double amp, xoff, HWHM, one_over_r2, val;

  amp   = p[0];
  xoff  = p[1];
  HWHM  = p[2]; // half-width at half-max = FWHM/2
  one_over_r2 = SQR(HWHM) / (SQR(x-xoff)+SQR(HWHM));
  //  val = yoff + amp * one_over_r2;
  val = amp * one_over_r2;
  return(val);
}

int fit_r1_from_flip_angles(int npoints,
			    double *xarray,   // INPUT: different flip angles in degrees
			    double *yarray,   // INPUT: signal intensities
			    float tr,         // INPUT: fixed TR used in study
			    float *s0,        // I/O:   s0 in signal equation
			    float *r1,        // 1/t1
			    float *rel_angle) // I/O:   fraction relative to "nominal" flip angle
{
  double param[3];
  lm_control_type control;
  lm_data_type data;
  int j;

  lm_initialize_control(&control);

  data.user_func = func_r1_from_flip_angles;
  data.user_x    = xarray;
  data.user_y    = yarray;

  for (j=0; j<npoints; j++)
    printf("for %d, flip = %g , signal = %g\n",j,xarray[j],yarray[j]);

  param[0] = *s0;
  param[1] = *r1;
  param[2] = *rel_angle;

  tr_for_t1_fit = tr;

  printf("\ninput : (s0,r1,ang) = (%g,%g,%g)\n",*s0,*r1,*rel_angle);

  // perform the fit:
  lm_minimize(npoints, 3, param, lm_evaluate_default, lm_print_maps,
	      &data, &control);
  // print results:
  //  printf("status: %s after %d evaluations\n",
  //	 lm_shortmsg[control.info], control.nfev);
  *s0        = param[0];
  *r1        = param[1];
  *rel_angle = param[2];

  printf("output: (s0,r1,ang) = (%g,%g,%g)\n",*s0,*r1,*rel_angle);

  return(0);
}

double func_r1_from_flip_angles(double x, double *p)
{
  double flip, s0, r1, rel_angle, ex, val;

  flip      = x*PI/180.;
  s0        = p[0];
  r1        = p[1];
  rel_angle = p[2];

  ex  = exp((double)(-tr_for_t1_fit*r1));
  val = s0 * sin((double)(flip*rel_angle)) * (1. - ex)/(1.-ex*cos((double)(flip*rel_angle)));

  printf("flip=%g, s0=%g, r1=%g, relang=%g, ex=%g, val=%g\n",flip,s0,r1,rel_angle,ex,val);

  return(val);
}

int fit_r1_from_multiple_tr(int npoints,
			    double *xarray,   // INPUT: different tr values
			    double *yarray,   // INPUT: signal intensities
			    float flip,       // INPUT: fixed flip angle used in series
			    float *s0,        // I/O:   s0 in signal equation
			    float *r1,        // 1/t1
			    float *rel_angle) // I/O:   fraction relative to "nominal" flip angle
{
  double param[3];
  lm_control_type control;
  lm_data_type data;
  int j;

  lm_initialize_control(&control);

  data.user_func = func_r1_from_multiple_tr;
  data.user_x    = xarray;
  data.user_y    = yarray;

  for (j=0; j<npoints; j++)
    printf("for %d, tr = %g , signal = %g\n",j,xarray[j],yarray[j]);

  param[0] = *s0;
  param[1] = *r1;
  param[2] = *rel_angle;

  flip_angle_for_t1_fit = flip;
  printf("\ninput : (s0,r1,ang) = (%g,%g,%g)\n",*s0,*r1,*rel_angle);

  // perform the fit:
  lm_minimize(npoints, 3, param, lm_evaluate_default, lm_print_maps,
	      &data, &control);
  // print results:
  //  printf("status: %s after %d evaluations\n",
  //	 lm_shortmsg[control.info], control.nfev);
  *s0        = param[0];
  *r1        = param[1];
  *rel_angle = param[2];

  printf("output: (s0,r1,ang) = (%g,%g,%g)\n",*s0,*r1,*rel_angle);

  return(0);
}

double func_r1_from_multiple_tr(double x, double *p)
{
  double tr, flip, s0, r1, rel_angle, ex, val;

  tr        = x;
  s0        = p[0];
  r1        = p[1];
  rel_angle = p[2];
  flip      = rel_angle * flip_angle_for_t1_fit;

  ex  = exp((double)(-tr*r1));
  val = s0 * sin((double)flip) * (1. - ex)/(1.-ex*cos((double)flip));
  printf("tr=%g, s0=%g, r1=%g, relang=%g, flip=%g, ex=%g, val=%g\n",tr,s0,r1,rel_angle,flip,ex,val);

  return(val);
}

void lm_print_maps(int n_par, double *par, int m_dat, double *fvec,
		   void *data, int iflag, int iter, int nfev)
/*
 *       data  : for soft control of printout behaviour, add control
 *                 variables to the data struct
 *       iflag : 0 (init) 1 (outer loop) 2(inner loop) -1(terminated)
 *       iter  : outer loop counter
 *       nfev  : number of calls to *evaluate
 */
{
  return;
  /*
  double f, y, t;
  int i;
  lm_data_type *mydata;
  mydata = (lm_data_type *) data;
  
  if (iflag == -1)
    {
      printf("terminated after %d evaluations\n", nfev);
      printf(" fitting data as follows:\n");
      for (i = 0; i < m_dat; ++i)
	{
	  t = (mydata->user_x)[i];
	  y = (mydata->user_y)[i];
	  f = mydata->user_func(t, par);
	  printf("    t[%2d]=%12g y=%12g fit=%12g residue=%12g\n",
		 i, t, y, f, y - f);
	}
    }
  */
}
