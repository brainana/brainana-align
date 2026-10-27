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

JIP_INDEX number_invalid_distortions(int all_voxels,int ilevel,int ipass)
{
#ifdef JIP_USE_CONTEXT
  JIP_CHECKPOINT();
#endif

  IPOINT3D p,start,end;
  int c, a, k, lo[3], hi[3], q[3], at[3];JIP_INDEX bad=0, checked=0;JIP_INDEX n,low_index,high_index;
  double j[3][3],det,base,ratio,minimum=INFINITY,maximum=-INFINITY,h,low,high;
  float *field[3];
  field[0]=distort_x.SpaceTime.fData;field[1]=distort_y.SpaceTime.fData;field[2]=distort_z.SpaceTime.fData;
  base=Alignment.Affine[0][0]*(Alignment.Affine[1][1]*Alignment.Affine[2][2]-Alignment.Affine[1][2]*Alignment.Affine[2][1])
      -Alignment.Affine[0][1]*(Alignment.Affine[1][0]*Alignment.Affine[2][2]-Alignment.Affine[1][2]*Alignment.Affine[2][0])
      +Alignment.Affine[0][2]*(Alignment.Affine[1][0]*Alignment.Affine[2][1]-Alignment.Affine[1][1]*Alignment.Affine[2][0]);
  if (all_voxels) {start.x=start.y=start.z=0;end=target.dim;}
  else {
    start=AlignNL.Index3DStart[AlignNL.iBasis];end=AlignNL.Index3DEnd[AlignNL.iBasis];
    /* A changed sample also affects a neighboring finite-difference stencil. */
    start.x=MAX(0,start.x-1);start.y=MAX(0,start.y-1);start.z=MAX(0,start.z-1);
    end.x=MIN(target.dim.x,end.x+1);end.y=MIN(target.dim.y,end.y+1);end.z=MIN(target.dim.z,end.z+1);
  }
  for(p.z=start.z;p.z<end.z;p.z++)for(p.y=start.y;p.y<end.y;p.y++)for(p.x=start.x;p.x<end.x;p.x++) {
    n=voxel3D_to_1D(p);ratio=NAN;
    q[0]=p.x;q[1]=p.y;q[2]=p.z;
    for(a=0;a<3;a++) {
      k=a==0?target.dim.x:a==1?target.dim.y:target.dim.z;
      h=a==0?target.resolution.x:a==1?target.resolution.y:target.resolution.z;
      lo[a]=q[a];hi[a]=q[a]+1<k?q[a]+1:q[a];
      if(hi[a]==lo[a]&&k>1)lo[a]--;
      /* Each component uses the same stencil. Resolve its two checked wide
         indices once per axis, preserving the original derivative arithmetic,
         boundary policy, finite tests and determinant evaluation order. */
      at[0]=q[0];at[1]=q[1];at[2]=q[2];at[a]=lo[a];
      low_index=index4d(at[0],at[1],at[2],0);
      at[a]=hi[a];high_index=index4d(at[0],at[1],at[2],0);
      for(c=0;c<3;c++) {
        low=field[c][low_index];high=field[c][high_index];
        j[c][a]=Alignment.Affine[c][a];
        if(!isfinite(h)||h<=0||!isfinite(low)||!isfinite(high)||!isfinite(field[c][n]))j[c][a]=NAN;
        else if(k>1)j[c][a]+=(high-low)/h;
      }
    }
    det=j[0][0]*(j[1][1]*j[2][2]-j[1][2]*j[2][1])
       -j[0][1]*(j[1][0]*j[2][2]-j[1][2]*j[2][0])
       +j[0][2]*(j[1][0]*j[2][1]-j[1][1]*j[2][0]);
    /* The pull map is A*x+u(x), NOT A*(x+u(x)). Keep the legacy display's
       volume-change convention relative to the affine, with correct derivative. */
    if(isfinite(base)&&fabs(base)>1.e-12&&isfinite(det))ratio=det/base;
    if(!isfinite(ratio)||ratio<=0||!isfinite(AlignNL.Jacob_limits)||AlignNL.Jacob_limits<1||
       ratio<1./AlignNL.Jacob_limits||ratio>AlignNL.Jacob_limits)bad++;
    if(isfinite(ratio)){minimum=MIN(minimum,ratio);maximum=MAX(maximum,ratio);}
    Jac_deter.SpaceTime.fData[n]=(float)ratio;checked++;
  }
  /* Local validity queries must NEVER reset the objective's ProbeInit caches. */
  if(all_voxels)printf("Relative full-map Jacobian: min=%g max=%g, invalid=%td/%td (level %d, pass %d)\n",minimum,maximum,bad,checked,ilevel,ipass);
  return bad;
}
