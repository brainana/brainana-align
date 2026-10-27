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


#ifndef JIP_WARP_SAFETY_H
#define JIP_WARP_SAFETY_H
#include <math.h>
#include <stddef.h>
/* Continuous extension of the original sampled target->source displacement.
   No change to the basis, field coefficients or registration objective. */
typedef struct {
  const float *component[3];
  size_t stride;
  int dim[3];
  double spacing[3],origin[3];
} JipWarpField;

static int jip_warp_sample(const JipWarpField *f,const double x[3],double u[3],double du[3][3])
{
  int lo[3], hi[3], a, c, bits, k[3], count[3];double t[3],weight[3],derivative[3],v,w,d;
  size_t index;
  for(a=0;a<3;a++) {
    if(f->dim[a]<1||!isfinite(x[a])||!isfinite(f->spacing[a])||f->spacing[a]<=0)return 0;
    d=(x[a]-f->origin[a])/f->spacing[a];
    if(d < -1.e-6 || d > f->dim[a]-1+1.e-6)return 0;
    d=fmax(0.,fmin(f->dim[a]-1.,d));
    lo[a]=(int)floor(d);hi[a]=lo[a]+1;
    if(f->dim[a]==1){lo[a]=hi[a]=0;t[a]=0;count[a]=1;}
    else {if(hi[a]>=f->dim[a]){lo[a]=f->dim[a]-2;hi[a]=f->dim[a]-1;}t[a]=d-lo[a];count[a]=2;}
  }
  for(c=0;c<3;c++){u[c]=0;for(a=0;a<3;a++)du[c][a]=0;}
  for(k[2]=0;k[2]<count[2];k[2]++)for(k[1]=0;k[1]<count[1];k[1]++)for(k[0]=0;k[0]<count[0];k[0]++) {
    index=0;
    for(a=0;a<3;a++) {
      weight[a]=count[a]==1?1.:k[a]?t[a]:1.-t[a];
      derivative[a]=count[a]==1?0.:(k[a]?1.:-1.)/f->spacing[a];
    }
    index=(size_t)(k[0]?hi[0]:lo[0])+(size_t)f->dim[0]*((k[1]?hi[1]:lo[1])+(size_t)f->dim[1]*(k[2]?hi[2]:lo[2]));
    w=weight[0]*weight[1]*weight[2];
    for(c=0;c<3;c++) {
      v=f->component[c][index*f->stride];if(!isfinite(v))return 0;
      if(w!=0)u[c]+=v*w;
      for(a=0;a<3;a++){d=derivative[a];for(bits=0;bits<3;bits++)if(bits!=a)d*=weight[bits];if(d!=0)du[c][a]+=v*d;}
    }
  }
  return 1;
}
static int jip_warp_solve3(double j[3][3],const double residual[3],double step[3])
{
  double m[3][4],scale=0,t,factor;int i, k, r, p;
  for(i=0;i<3;i++){for(k=0;k<3;k++){m[i][k]=j[i][k];scale=fmax(scale,fabs(j[i][k]));}m[i][3]=residual[i];}
  if(!isfinite(scale)||scale==0)return 0;
  for(k=0;k<3;k++) {
    p=k;for(r=k+1;r<3;r++)if(fabs(m[r][k])>fabs(m[p][k]))p=r;
    if(!isfinite(m[p][k])||fabs(m[p][k])<1.e-12*scale)return 0;
    for(i=k;i<4;i++){t=m[k][i];m[k][i]=m[p][i];m[p][i]=t;}
    t=m[k][k];for(i=k;i<4;i++)m[k][i]/=t;
    for(r=0;r<3;r++)if(r!=k){factor=m[r][k];for(i=k;i<4;i++)m[r][i]-=factor*m[k][i];}
  }
  for(i=0;i<3;i++){step[i]=m[i][3];if(!isfinite(step[i]))return 0;}return 1;
}
static int jip_warp_evaluate(const JipWarpField *f,const float affine[4][4],const double x[3],const double y[3],double r[3],double j[3][3],double *error)
{
  double u[3],du[3][3];int c, a;
  if(!jip_warp_sample(f,x,u,du))return 0;
  *error=0;
  for(c=0;c<3;c++) {
    r[c]=affine[c][3]+u[c]-y[c];
    for(a=0;a<3;a++){r[c]+=affine[c][a]*x[a];j[c][a]=affine[c][a]+du[c][a];}
    *error+=r[c]*r[c];
  }
  return isfinite(*error);
}
static double jip_warp_determinant(double j[3][3])
{
  return j[0][0]*(j[1][1]*j[2][2]-j[1][2]*j[2][1])-j[0][1]*(j[1][0]*j[2][2]-j[1][2]*j[2][0])+j[0][2]*(j[1][0]*j[2][1]-j[1][1]*j[2][0]);
}
/* Sampled orientation check, matching the registration's grid-based policy.
   This is not a proof of global invertibility between grid samples. */
static int jip_warp_validate(const JipWarpField *f,const float affine[4][4])
{
  double base[3][3],j[3][3],x[3],y[3]={0},r[3],e,det,relative;int a, c, i, k, l;
  for(a=0;a<3;a++)for(c=0;c<3;c++)base[a][c]=affine[a][c];
  det=jip_warp_determinant(base);if(!isfinite(det)||fabs(det)<1.e-12)return 0;
  for(l=0;l<f->dim[2];l++)for(k=0;k<f->dim[1];k++)for(i=0;i<f->dim[0];i++) {
    x[0]=f->origin[0]+i*f->spacing[0];x[1]=f->origin[1]+k*f->spacing[1];x[2]=f->origin[2]+l*f->spacing[2];
    if(!jip_warp_evaluate(f,affine,x,y,r,j,&e))return 0;
    relative=jip_warp_determinant(j)/det;if(!isfinite(relative)||relative<=1.e-8)return 0;
  }
  return 1;
}
/* Invert A*x+u(x), rather than A alone or a separately fitted reverse field.
   Return failure for a root outside the field domain or no convergence. */
static int jip_warp_inverse(const JipWarpField *f,const float affine[4][4],const float inverse[4][4],const double y[3],double result[3])
{
  double x[3],trial[3],r[3],j[3][3],rt[3],jt[3][3],step[3],error,next,alpha,low,high;
  int a, c, iteration, attempt, accepted;
  for(a=0;a<3;a++) {
    if(f->dim[a]<1||!isfinite(f->spacing[a])||f->spacing[a]<=0||!isfinite(y[a]))return 0;
    low=f->origin[a];high=low+(f->dim[a]-1)*f->spacing[a];
    x[a]=inverse[a][3];for(c=0;c<3;c++)x[a]+=inverse[a][c]*y[c];
    if(!isfinite(x[a]))return 0;x[a]=fmax(low,fmin(high,x[a]));
  }
  for(iteration=0;iteration<64;iteration++) {
    if(!jip_warp_evaluate(f,affine,x,y,r,j,&error))return 0;
    if(error<=1.e-8){for(a=0;a<3;a++)result[a]=x[a];return 1;}
    if(!jip_warp_solve3(j,r,step))return 0;
    accepted=0;
    for(attempt=0,alpha=1.;attempt<16;attempt++,alpha*=.5) {
      for(a=0;a<3;a++)trial[a]=fmax(f->origin[a],fmin(f->origin[a]+(f->dim[a]-1)*f->spacing[a],x[a]-alpha*step[a]));
      if(jip_warp_evaluate(f,affine,trial,y,rt,jt,&next)&&next<error){accepted=1;break;}
    }
    if(!accepted)return 0;for(a=0;a<3;a++)x[a]=trial[a];
  }
  return 0;
}
#endif
