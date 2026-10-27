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

int ParabolicInterpolation(float *X3Point, float *Y3Point, float *XMax, float *YMax)
// return: 0 = ok, 1 = error (not parabolic)
{
  float XMatrix[3][3], XMatrix_T[3][3], **XTX, XTY[3], Beta[3];
  float XNew[3], YNew[3];
  float X0, Y0, amp, maxX, maxY;
  int j1, j2, jsum, iRowFail;

  X0 = X3Point[1];
  Y0 = Y3Point[1];
  // Find the maximum of x, and then scale x to 1.
  for (j1=0, maxX = 0.; j1<3; j1++)
    {
      amp = fabs((double)(X3Point[j1]-X0));
      if ( amp > maxX )
	maxX = amp;
    }
  if ( maxX == 0. )
    {
      printf("Error: maxX = 0 in ParabolicInterpolation()\n");
      return(1);
    }
  for (j1=0; j1<3; j1++)
    XNew[j1] = (X3Point[j1]-X0)/maxX;
  // Find the maximum of y, and then scale y to 1.
  for (j1=0, maxY = 0.; j1<3; j1++)
    {
      amp = fabs((double)(Y3Point[j1]-Y0));
      if ( amp > maxY )
	maxY = amp;
    }
  if ( maxY == 0. )
    { // not parabolic: linear with all y = 0
      *XMax = *YMax = 0.;
      return(1);
    }
  for (j1=0; j1<3; j1++)
    YNew[j1] = (Y3Point[j1]-Y0)/maxY;

  XTX = float_matrix(3,3);
  // y = a*x^2 + b*x + c
  // ... can be solved for 3 points in matrix form: Y = X * Beta

  XMatrix[0][0] = XNew[0] * XNew[0];
  XMatrix[0][1] = XNew[0];
  XMatrix[0][2] = 1.;
  XMatrix[1][0] = XNew[1] * XNew[1];
  XMatrix[1][1] = XNew[1];
  XMatrix[1][2] = 1.;
  XMatrix[2][0] = XNew[2] * XNew[2];
  XMatrix[2][1] = XNew[2];
  XMatrix[2][2] = 1.;

  // Form the transpose of this matrix
  for (j1=0; j1<3; j1++)
    {
      for (j2=0; j2<3; j2++)
	XMatrix_T[j1][j2] = XMatrix[j2][j1];
    }
  // Calculate XTX
  for (j1=0; j1<3; j1++)
    {
      for (j2=0; j2<3; j2++)
	{
	  XTX[j1][j2] = 0;
	  for (jsum=0; jsum<3; jsum++)
	    XTX[j1][j2] += XMatrix_T[j1][jsum] * XMatrix[jsum][j2];
	}
    }
  // Invert XTX
  if ( rInvertSquareMatrix(3,XTX,&iRowFail) )
    {
      printf("maximize:\n");
      printf("X: %f %f %f\n",XNew[0],XNew[1],XNew[2]);
      printf("Y: %f %f %f\n",YNew[0],YNew[1],YNew[2]);
      printf("Error: singular matrix in ParabolicInterpolation\n");
      return(1);
    }
  // Calculate XTY
  for (j1=0; j1<3; j1++)
    {
      XTY[j1] = 0;
      for (jsum=0; jsum<3; jsum++)
	XTY[j1] += XMatrix_T[j1][jsum] * YNew[jsum];
    }
  // Finally, calculate Beta as (XTX)^1 XT*Y
  for (j1=0; j1<3; j1++)
    {
      Beta[j1] = 0;
      for (jsum=0; jsum<3; jsum++)
	Beta[j1] += XTX[j1][jsum] * XTY[jsum];
    }
  free_float_matrix(XTX,3,3);
  // y = a*x^2 + b*x + c has a maximum or minimum at x = -b/(2a), unless a = 0
  if ( Beta[0] == 0. )
    {
      *XMax = *YMax = 0.;
      return(1);
    }
  else
    {
      *XMax = -Beta[1]/2./Beta[0];
      *YMax = Beta[0]*(*XMax)*(*XMax) + Beta[1]*(*XMax) + Beta[2];
      *XMax *= maxX;
      *YMax *= maxY;
      *XMax += X0;
      *YMax += Y0;
      return(0);
    }
}
