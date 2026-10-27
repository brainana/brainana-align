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



#ifndef JIP_MEMORY_H
#define JIP_MEMORY_H
/* All code sharing util.h must be rebuilt. This project requires a 64-bit ABI.
   Coordinates/API enums stay int; counts, byte offsets and flat indices do not. */
#include <stddef.h>
#include <stdint.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
typedef ptrdiff_t JIP_INDEX;
_Static_assert(sizeof(void*)==8 && sizeof(size_t)==8 && sizeof(JIP_INDEX)==8,
               "JIP requires a 64-bit target and 64-bit size/index types");
#ifndef JIP_SYSTEM_MALLOC
#define JIP_SYSTEM_MALLOC malloc
#endif
static inline void jip_memory_error(const char *reason) {
#ifdef JIP_MEMORY_FAILURE
  JIP_MEMORY_FAILURE(reason);
#else
  fprintf(stderr,"JIP size/allocation error: %s\n",reason);exit(EXIT_FAILURE);
#endif
}
static inline JIP_INDEX jip_product(JIP_INDEX a,JIP_INDEX b) {
  if(a<0||b<0||(b && a>PTRDIFF_MAX/b))jip_memory_error("negative or overflowing image size");
  return a*b;
}
static inline size_t jip_bytes(JIP_INDEX count,size_t element_size) {
  if(count<0||!element_size||(uintmax_t)count>(uintmax_t)PTRDIFF_MAX/element_size)
    jip_memory_error("allocation size exceeds the addressable object range");
  return (size_t)count*element_size;
}
static inline void *jip_allocate(JIP_INDEX count,size_t element_size) {
  size_t bytes=jip_bytes(count,element_size);void *result=JIP_SYSTEM_MALLOC(bytes?bytes:1);
  if(!result)jip_memory_error("cannot allocate requested buffer");
  return result;
}
static inline int jip_int_count(JIP_INDEX value) {
  if(value<0||value>INT_MAX)jip_memory_error("size exceeds a bounded 32-bit external API");
  return (int)value;
}
static inline int jip_coordinate(JIP_INDEX value) {
  if(value<INT_MIN||value>INT_MAX)jip_memory_error("coordinate exceeds a bounded 32-bit display API");
  return (int)value;
}
static inline int jip_seek(FILE *file,JIP_INDEX offset,int whence) {
#if defined(_WIN32) && !defined(__CYGWIN__)
  return _fseeki64(file,(__int64)offset,whence);
#else
  _Static_assert(sizeof(off_t)>=8,"64-bit file offsets required; define _FILE_OFFSET_BITS=64");
  return fseeko(file,(off_t)offset,whence);
#endif
}
static inline JIP_INDEX jip_tell(FILE *file) {
#if defined(_WIN32) && !defined(__CYGWIN__)
  return (JIP_INDEX)_ftelli64(file);
#else
  return (JIP_INDEX)ftello(file);
#endif
}
#endif
