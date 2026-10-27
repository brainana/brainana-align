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

void define_basis_functions( int which )
// which: 0 = x, 1 = y, 2 = z
{
  int nBasis, nSpace, iDown, nBasis1, nBasis2, nBasis4, nBasis8, nBasis16;
  JIP_INDEX jBasis, jSpace, iRel;
  float **mat, **mat1_bs, **mat2_bs, **mat4_bs, **mat8_bs, **mat16_bs, **mix_lh;

  if ( which == 0 )
    {
      mat     = AlignNL.BasisX;
      nBasis  = AlignNL.NBasis1D.x;
      nSpace  = target.dim.x;
      iDown   = AlignNL.GridDown.x;
    }
  else if ( which == 1 )
    {
      mat    = AlignNL.BasisY;
      nBasis = AlignNL.NBasis1D.y;
      nSpace = target.dim.y;
      iDown  = AlignNL.GridDown.y;
    }
  else if ( which == 2 )
    {
      mat    = AlignNL.BasisZ;
      nBasis = AlignNL.NBasis1D.z;
      nSpace = target.dim.z;
      iDown  = AlignNL.GridDown.z;
    }

  ////////////////// Level-1 Basis Functions /////////////////////
  nBasis1 = number_NL_basis_functions(nSpace,1);
  // Define the level-1 matrix with no down-sampling.
  mat1_bs = float_matrix(nBasis1,nSpace);
  for (jBasis=0; jBasis<nBasis1; jBasis++)
    {
      for (jSpace=0; jSpace<nSpace; jSpace++)
	{
	  iRel = jip_int_count(llabs(jSpace-jBasis));
	  //	  if ( iRel <= 2 )
	  //	    mat1_bs[jBasis][jSpace] = 1.-(float)iRel/2.;
	  if ( jBasis == jSpace )
	    mat1_bs[jBasis][jSpace] = 1.;
	  else
	    mat1_bs[jBasis][jSpace] = 0.;
	}
    }
  if ( iDown == 1 )
    {
      if ( nBasis != nBasis1 )
	{
	  printf("Programming error in define_basis_functions():\n");
	  printf("# of basis functions (%d) != # of level-1 basis functions (%d)\n",nBasis,nBasis1);
	  exit(1);
	}
      for (jBasis=0; jBasis<nBasis; jBasis++)
	{
	  for (jSpace=0; jSpace<nSpace; jSpace++)
	    mat[jBasis][jSpace] = mat1_bs[jBasis][jSpace];
	}
      free_float_matrix(mat1_bs,nBasis,nSpace);
      return;
    }

  ////////////////// Level-2 Basis Functions /////////////////////
  nBasis2 = number_NL_basis_functions(nSpace,2);
  // Define the mixing matrix from level 1 to level 2.
  mix_lh = float_matrix(nBasis2,nBasis1);   // mix_lh = mix_lowDim_highDim
  compute_mixing_matrix(nBasis2,nBasis1,mix_lh);
  // Compute the level-2 matrix (2x down-sampling): mat2_bs = mix_lh * mat1_bs
  mat2_bs = float_matrix(nBasis2,nSpace);
  //               X1*X2    L1       L2       X1    t   L12      X2      t
  Matrix_Product( mat2_bs, nBasis2, nSpace, mix_lh, 0, nBasis1, mat1_bs, 0);
  free_float_matrix(mat1_bs,nBasis1,nSpace);
  free_float_matrix(mix_lh,nBasis2,nBasis1);
  if ( iDown == 2 )
    {
      if ( nBasis != nBasis2 )
	{
	  printf("Programming error in define_basis_functions():\n");
	  printf("# of basis functions (%d) != # of level-1 basis functions (%d)\n",nBasis,nBasis2);
	  exit(1);
	}
      for (jBasis=0; jBasis<nBasis; jBasis++)
	{
	  for (jSpace=0; jSpace<nSpace; jSpace++)
	    mat[jBasis][jSpace] = mat2_bs[jBasis][jSpace];
	}
      free_float_matrix(mat2_bs,nBasis,nSpace);
      return;
    }

  ////////////////// Level-4 Basis Functions /////////////////////
  nBasis4 = number_NL_basis_functions(nSpace,4);
  // Define the mixing matrix from level 2 to level 4.
  mix_lh = float_matrix(nBasis4,nBasis2);
  compute_mixing_matrix(nBasis4,nBasis2,mix_lh);
  // Compute the level-2 matrix (2x down-sampling): mat4_bs = mix_lh * mat2_bs
  mat4_bs = float_matrix(nBasis4,nSpace);
  //               X1*X2    L1       L2       X1    t   L12      X2      t
  Matrix_Product( mat4_bs, nBasis4, nSpace, mix_lh, 0, nBasis2, mat2_bs, 0);
  free_float_matrix(mix_lh,nBasis4,nBasis2);
  free_float_matrix(mat2_bs,nBasis2,nSpace);
  if ( iDown == 4 )
    {
      if ( nBasis != nBasis4 )
	{
	  printf("Programming error in define_basis_functions():\n");
	  printf("# of basis functions (%d) != # of level-1 basis functions (%d)\n",nBasis,nBasis4);
	  exit(1);
	}
      for (jBasis=0; jBasis<nBasis; jBasis++)
	{
	  for (jSpace=0; jSpace<nSpace; jSpace++)
	    mat[jBasis][jSpace] = mat4_bs[jBasis][jSpace];
	}
      free_float_matrix(mat4_bs,nBasis,nSpace);
      return;
    }

  ////////////////// Level-8 Basis Functions /////////////////////
  nBasis8 = number_NL_basis_functions(nSpace,8);
  // Define the mixing matrix from level 2 to level 4.
  mix_lh = float_matrix(nBasis8,nBasis4);
  compute_mixing_matrix(nBasis8,nBasis4,mix_lh);
  // Compute the level-2 matrix (2x down-sampling): mat8_bs = mix_lh * mat4_bs
  mat8_bs = float_matrix(nBasis8,nSpace);
  //               X1*X2    L1       L2       X1    t   L12      X2      t
  Matrix_Product( mat8_bs, nBasis8, nSpace, mix_lh, 0, nBasis4, mat4_bs, 0);
  free_float_matrix(mix_lh,nBasis8,nBasis4);
  free_float_matrix(mat4_bs,nBasis4,nSpace);
  if ( iDown == 8 )
    {
      if ( nBasis != nBasis8 )
	{
	  printf("Programming error in define_basis_functions():\n");
	  printf("# of basis functions (%d) != # of level-1 basis functions (%d)\n",nBasis,nBasis8);
	  exit(1);
	}
      for (jBasis=0; jBasis<nBasis; jBasis++)
	{
	  for (jSpace=0; jSpace<nSpace; jSpace++)
	    mat[jBasis][jSpace] = mat8_bs[jBasis][jSpace];
	}
      free_float_matrix(mat8_bs,nBasis,nSpace);
      return;
    }

  ////////////////// Level-16 Basis Functions /////////////////////
  nBasis16 = number_NL_basis_functions(nSpace,16);
  // Define the mixing matrix from level 2 to level 4.
  mix_lh = float_matrix(nBasis16,nBasis8);
  compute_mixing_matrix(nBasis16,nBasis8,mix_lh);
  // Compute the level-2 matrix (2x down-sampling): mat8_bs = mix_lh * mat4_bs
  mat16_bs = float_matrix(nBasis16,nSpace);
  //               X1*X2    L1       L2       X1    t   L12      X2      t
  Matrix_Product( mat16_bs, nBasis16, nSpace, mix_lh, 0, nBasis8, mat8_bs, 0);
  free_float_matrix(mix_lh,nBasis16,nBasis8);
  free_float_matrix(mat8_bs,nBasis8,nSpace);
  if ( iDown == 16 )
    {
      if ( nBasis != nBasis16 )
	{
	  printf("Programming error in define_basis_functions():\n");
	  printf("# of basis functions (%d) != # of level-1 basis functions (%d)\n",nBasis,nBasis16);
	  exit(1);
	}
      for (jBasis=0; jBasis<nBasis; jBasis++)
	{
	  for (jSpace=0; jSpace<nSpace; jSpace++)
	    mat[jBasis][jSpace] = mat16_bs[jBasis][jSpace];
	}
      free_float_matrix(mat16_bs,nBasis,nSpace);
      return;
    }
  else
    {
      printf("Programming error in define_basis_functions():\n");
      printf("Down-sampling for dimension %d == %d, but 16 is the largest implementation.\n",which,iDown);
      exit(1);
    }
}
