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

void calculate_orientation_rotation()
{
  float Rx[3][3],   Ry[3][3],     Rz[3][3];
  float RyRz[3][3], RxRyRz[3][3], Sz[3][3];
  float cs, sn;
  int j1, j2, j12;

  Rx[0][0] = Ry[1][1] = Rz[2][2] = 1.;
  Rx[0][1] = Rx[1][0] = Rx[0][2] = Rx[2][0] = 0.;
  Ry[0][1] = Ry[1][0] = Ry[1][2] = Ry[2][1] = 0.;
  Rz[0][2] = Rz[2][0] = Rz[1][2] = Rz[2][1] = 0.;

  define_cosine_sine(Alignment.OrientRotation.x, &cs, &sn);
  Rx[1][1] = cs;   Rx[1][2] = -sn;
  Rx[2][2] = cs;   Rx[2][1] =  sn;
  
  define_cosine_sine(Alignment.OrientRotation.y, &cs, &sn);
  Ry[0][0] = cs;   Ry[0][2] =  sn;
  Ry[2][2] = cs;   Ry[2][0] = -sn;
  
  define_cosine_sine(Alignment.OrientRotation.z, &cs, &sn);
  Rz[0][0] = cs;   Rz[0][1] = -sn;
  Rz[1][1] = cs;   Rz[1][0] =  sn;

  // Compute Ry * Rz
  for (j1=0; j1<3; j1++)
    {
      for (j2=0; j2<3; j2++)
	{
	  RyRz[j1][j2] = 0.;
	  for (j12=0; j12<3; j12++)
	    RyRz[j1][j2] += Ry[j1][j12]*Rz[j12][j2]; // inner product
	}
    }
  // Compute Ry * Rx * Rz
  for (j1=0; j1<3; j1++)
    {
      for (j2=0; j2<3; j2++)
	{
	  RxRyRz[j1][j2] = 0.;
	  for (j12=0; j12<3; j12++)
	    RxRyRz[j1][j2] += Rx[j1][j12]*RyRz[j12][j2]; // inner product
	}
    }
  // Compute Ry * Rx * Rz * Swap_z = Alignment.Orient
  for (j1=0; j1<3; j1++)
    {
      for (j2=0; j2<3; j2++)
	{
	  // Set up swap matrix as identity
	  if ( j1 == j2 )
	    Sz[j1][j2] = 1;
	  else
	    Sz[j1][j2] = 0;
	}
    }
  if ( Alignment.OrientSwapZ )
    Sz[2][2] = -1.; // swap the z coordinate
  for (j1=0; j1<3; j1++)
    {
      for (j2=0; j2<3; j2++)
	{
	  Alignment.Orient[j1][j2] = 0.;
	  for (j12=0; j12<3; j12++)
	    Alignment.Orient[j1][j2] += RxRyRz[j1][j12]*Sz[j12][j2]; // inner product
	}
    }
}
