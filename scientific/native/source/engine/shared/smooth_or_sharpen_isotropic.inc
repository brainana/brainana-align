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

void smooth_or_sharpen_isotropic( int sharpen, POINT3D fwhm, struct data_stack *stack )
     // fwhm is in units of mm (or whatever spatial units are used)
{
  POINT3D resolution, fov2, sigma2, sigma2_k;
  float x, y, z, scale, amp, phase;
  int jx, jy, jz, jt;JIP_INDEX jp, npts, index;
  struct data_stack stack_temp;

  stack_temp.dim = stack->dim;
  stack_temp.n2d = stack->n2d;
  stack_temp.n3d = stack->n3d;
  stack_temp.n4d = stack->n4d;
  stack_temp.nt  = stack->nt;
  stack_temp.resolution = stack->resolution;
  npts = stack->n4d;
  stack_temp.SpaceTime.fcData = fcomplex_vector(npts);
  for (jp=0; jp<npts; jp++)
    {
      stack_temp.SpaceTime.fcData[jp].real = stack->SpaceTime.fData[jp];
      stack_temp.SpaceTime.fcData[jp].imag = 0.;
    }

  // inverse FT in x, y, and z
  ft_1d( 1, -1, &stack_temp ); // x
  ft_1d( 2, -1, &stack_temp ); // y
  ft_1d( 3, -1, &stack_temp ); // z

  // Determine the 3D sigma_k
  // Multiply data by a k-space filter
  sigma2_k.x = 2.355*2.355/4./PI/PI /fwhm.x/fwhm.x;
  sigma2_k.y = 2.355*2.355/4./PI/PI /fwhm.y/fwhm.y;
  sigma2_k.z = 2.355*2.355/4./PI/PI /fwhm.z/fwhm.z;

  // Smooth data by multiplication in k-space
  for ( jt=0; jt<stack->nt; jt++ )
    {
      // Work on 1 volume at a time
      resolution = stack->resolution;
      fov2.x = resolution.x*resolution.x * stack->dim.x*stack->dim.x;
      fov2.y = resolution.y*resolution.y * stack->dim.y*stack->dim.y;
      fov2.z = resolution.z*resolution.z * stack->dim.z*stack->dim.z;
      sigma2.x = sigma2_k.x * fov2.x; // use normalized fov in each dimension
      sigma2.y = sigma2_k.y * fov2.y;
      sigma2.z = sigma2_k.z * fov2.z;
      for ( jz=0; jz<stack->dim.z; jz++ ) // these are steps in k-space
	{
	  z = jz - stack->dim.z/2 + 0.5;
	  for ( jy=0; jy<stack->dim.y; jy++ )
	    {
	      y = jy - stack->dim.y/2 + 0.5;
	      for ( jx=0; jx<stack->dim.x; jx++ )
		{
		  x = jx - stack->dim.x/2 + 0.5;
		  scale
		    = exp(-(double)(x*x/2/sigma2.x))
		    * exp(-(double)(y*y/2/sigma2.y))
		    * exp(-(double)(z*z/2/sigma2.z));
		  if ( sharpen )
		    scale = 1. - scale; // switch low-pass to high-pass
		  index = index4d_stack(jx,jy,jz,jt,&stack_temp);
		  stack_temp.SpaceTime.fcData[index].real *= scale;
		  stack_temp.SpaceTime.fcData[index].imag *= scale;
		}
	    }
	}
    }

  // forward FT in x, y, and z
  ft_1d( 1, 1, &stack_temp ); // x
  ft_1d( 2, 1, &stack_temp ); // y
  ft_1d( 3, 1, &stack_temp ); // z

  // The data should be real, but make sure here.
  for ( jp=0; jp<npts; jp++ )
    {
      amp   = compute_amplitude(stack_temp.SpaceTime.fcData[jp]);
      phase = compute_phase(stack_temp.SpaceTime.fcData[jp]);
      //      if ( phase < 0. ) amp = -amp;
      //      stack->SpaceTime.fData[jp] = amp;
      stack->SpaceTime.fData[jp] = stack_temp.SpaceTime.fcData[jp].real;
    }
  free_fcomplex_vector(stack_temp.SpaceTime.fcData);
}
