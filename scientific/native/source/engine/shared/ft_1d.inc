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


/* Shared numerical source: legacy X11 frontend and context-based engine. */

int ft_1d( int idim, int forward, struct data_stack *stack )
{
  // idim: 1 -> x,
  //       2 -> y,
  //       3 -> z
  // forward: 1 -> forward  (k-space to image space)
  //         -1 -> backward (image space to k-space)
  fftw_plan plan_ft1d;
  fftw_complex *fftwData;
  FCOMPLEX *Data1d;
  float div;
  int lInner, lOuter1, lOuter2, direction;
  int j1, j2, jIn, jt;JIP_INDEX index;

  if ( forward == 1 )
    direction = FFTW_FORWARD;
  else
    direction = FFTW_BACKWARD;
  
  // Set the dimensions based upon the FT direction.
  if ( idim == 1 )
    {
      lInner  = stack->dim.x;
      lOuter1 = stack->dim.y;
      lOuter2 = stack->dim.z;
    }
  else if ( idim == 2 )
    {
      lOuter1 = stack->dim.x;
      lInner  = stack->dim.y;
      lOuter2 = stack->dim.z;
    }
  else if ( idim == 3 )
    {
      lOuter1 = stack->dim.x;
      lOuter2 = stack->dim.y;
      lInner  = stack->dim.z;
    }
  if ( forward == 1 )
    div = (float)lInner;
  else
    div = 1.;

  // Allocate the 1d arrays
  Data1d   = fcomplex_vector(lInner); // floats
  fftwData = (fftw_complex *)fftw_malloc((size_t)jip_product(sizeof(fftw_complex),lInner)); // doubles
  // Loop over time + the 3 spatial dimensions
  for (jt=0; jt<stack->nt; jt++)
    {
      for (j1=0; j1<lOuter1; j1++)
	{
	  for (j2=0; j2<lOuter2; j2++)
	    {
	      for (jIn=0; jIn<lInner; jIn++)
		{
		  if ( idim == 1 )
		    index = index4d_stack(jIn,j1,j2,jt,stack);
		  else if ( idim == 2 )
		    index = index4d_stack(j1,jIn,j2,jt,stack);
		  else if ( idim == 3 )
		    index = index4d_stack(j1,j2,jIn,jt,stack);
		  Data1d[jIn] = stack->SpaceTime.fcData[index];
		}
	      // On the 1st line of data, define the FT "plan".
	      if ( jt == 0 && j1==0 && j2==0 )
		plan_ft1d = fftw_plan_dft_1d(lInner,fftwData,fftwData,direction,FFTW_MEASURE);
	      
	      // swap on input
	      swap_x( Data1d, lInner, 0 );
	      // Assign the fftw directly (rather than use pointers) in case
	      // the byte size differs for the 2 arrays.
	      for (jIn=0; jIn<lInner; jIn++)
		{
		  fftwData[jIn][0] = Data1d[jIn].real;
		  fftwData[jIn][1] = Data1d[jIn].imag;
		}
	      // Transform the data.
	      fftw_execute(plan_ft1d);
	      // Use the original array.
	      for ( jIn=0; jIn<lInner; jIn++ )
		{
		  Data1d[jIn].real = fftwData[jIn][0];
		  Data1d[jIn].imag = fftwData[jIn][1];
		}
	      // The data comes out of the fft routine in wrap-around order.
	      swap_x( Data1d, lInner, 1 );
	      // Put the transformed data back into the original matrix.
	      for (jIn=0; jIn<lInner; jIn++)
		{
		  if ( idim == 1 )
		    index = index4d_stack(jIn,j1,j2,jt,stack);
		  else if ( idim == 2 )
		    index = index4d_stack(j1,jIn,j2,jt,stack);
		  else if ( idim == 3 )
		    index = index4d_stack(j1,j2,jIn,jt,stack);
		  stack->SpaceTime.fcData[index].real = Data1d[jIn].real/div;
		  stack->SpaceTime.fcData[index].imag = Data1d[jIn].imag/div;
		}
	    }
	}
    }

  // Free data and plan allocation
  free_fcomplex_vector(Data1d);
  fftw_destroy_plan(plan_ft1d);
  fftw_free(fftwData);

  return(0);
}
