/* Shared integer arithmetic for CPU validation, Metal and CUDA. Image storage
   remains float32. Software binary64 accumulation preserves the serial JIP
   weighted sum even on GPUs without native float64. No fast-math is used. */
#ifndef JIP_ACCEL_KERNEL_H
#define JIP_ACCEL_KERNEL_H
#ifdef __METAL_VERSION__
#include <metal_stdlib>
using namespace metal;
typedef uint jip_u32;typedef ulong jip_u64;typedef int jip_i32;
#define JIP_DEVICE device
#define JIP_CONSTANT constant
#define JIP_INLINE inline
#elif defined(__CUDACC_RTC__)
typedef unsigned int jip_u32;typedef unsigned long long jip_u64;typedef int jip_i32;
#define JIP_DEVICE
#define JIP_CONSTANT
#define JIP_INLINE __device__ inline
#else
#include <stdint.h>
typedef uint32_t jip_u32;typedef uint64_t jip_u64;typedef int32_t jip_i32;
#define JIP_DEVICE
#define JIP_CONSTANT
#ifdef __CUDACC__
#define JIP_INLINE __host__ __device__ inline
#else
#define JIP_INLINE static inline
#endif
#endif
#define JIP_KERNEL_TAPS 34
typedef struct {jip_i32 low[3];jip_u32 length[3],coeff[3*JIP_KERNEL_TAPS],valid;} jip_sample;
typedef struct {jip_u32 dim[3],wrap[3],count;} jip_kernel_params;

JIP_INLINE int jip_top(jip_u64 x) {
  int n=0;if(x>>32){x>>=32;n+=32;}if(x>>16){x>>=16;n+=16;}if(x>>8){x>>=8;n+=8;}
  if(x>>4){x>>=4;n+=4;}if(x>>2){x>>=2;n+=2;}if(x>>1)n++;return n;
}
JIP_INLINE jip_u64 jip_jam(jip_u64 x,int n) {
  if(n<=0)return x;if(n>=64)return x!=0;return (x>>n)|((x<<((64-n)&63))!=0);
}
JIP_INLINE jip_u64 jip_round(jip_u64 x,int n) {
  if(n<=0)return x<<(-n);if(n>=64)return 0;
  /* This header is also Metal source: `half` is a reserved scalar type there. */
  jip_u64 q=x>>n,r=x&((((jip_u64)1)<<n)-1),midpoint=((jip_u64)1)<<(n-1);
  return q+(r>midpoint||(r==midpoint&&(q&1)));
}
JIP_INLINE int jip_float_exp(jip_u32 x) {
  int e=(int)((x>>23)&255);if(e)return e-127;return jip_top(x&0x7fffff)-149;
}
JIP_INLINE jip_u64 jip_float_mant(jip_u32 x) {
  jip_u32 m=x&0x7fffff;if(x&0x7f800000)return m|0x800000;
  return ((jip_u64)m)<<(23-jip_top(m));
}
JIP_INLINE jip_u32 jip_mul32(jip_u32 a,jip_u32 b) {
  jip_u32 sign=(a^b)&0x80000000;
  if(!(a&0x7fffffff)||!(b&0x7fffffff))return sign;
  jip_u64 product=jip_float_mant(a)*jip_float_mant(b);int top=jip_top(product);
  int e=jip_float_exp(a)+jip_float_exp(b)+top-46;
  if(e>127)return sign|0x7f800000;
  jip_u64 m=jip_round(product,top-23+(e< -126?-126-e:0));
  if(e< -126)return sign|(jip_u32)m;
  if(m>=0x1000000){m>>=1;e++;}
  if(e>127)return sign|0x7f800000;
  return sign|((jip_u32)(e+127)<<23)|((jip_u32)m&0x7fffff);
}
JIP_INLINE jip_u64 jip_from32(jip_u32 x) {
  jip_u64 sign=((jip_u64)(x&0x80000000))<<32;if(!(x&0x7fffffff))return sign;
  return sign|((jip_u64)(jip_float_exp(x)+1023)<<52)|((jip_float_mant(x)&0x7fffff)<<29);
}
JIP_INLINE jip_u64 jip_product64(jip_u32 a,jip_u32 b) {
  jip_u64 sign=((jip_u64)((a^b)&0x80000000))<<32;
  if(!(a&0x7fffffff)||!(b&0x7fffffff))return sign;
  jip_u64 p=jip_float_mant(a)*jip_float_mant(b);int top=jip_top(p);
  int e=jip_float_exp(a)+jip_float_exp(b)+top-46;
  return sign|((jip_u64)(e+1023)<<52)|((p<<(52-top))&((((jip_u64)1)<<52)-1));
}
JIP_INLINE jip_u64 jip_add64(jip_u64 a,jip_u64 b) {
  const jip_u64 signbit=((jip_u64)1)<<63,mask=signbit-1,hidden=((jip_u64)1)<<52;
  if(!(a&mask)&&!(b&mask))return (a&b)&signbit;
  if(!(a&mask))return b;if(!(b&mask))return a;
  if((a&mask)<(b&mask)){jip_u64 swap=a;a=b;b=swap;}
  int e=(int)((a>>52)&2047),shift=e-(int)((b>>52)&2047);
  jip_u64 ma=((a&(hidden-1))|hidden)<<3,mb=jip_jam(((b&(hidden-1))|hidden)<<3,shift);
  jip_u64 m=((a^b)&signbit)?ma-mb:ma+mb;if(!m)return 0;
  if(m&(((jip_u64)1)<<56)){m=jip_jam(m,1);e++;}
  while(!(m&(((jip_u64)1)<<55))){m<<=1;e--;}
  jip_u64 rounded=(m>>3)+((m&7)>4||((m&7)==4&&((m>>3)&1)));
  if(rounded&(((jip_u64)1)<<53)){rounded>>=1;e++;}
  return (a&signbit)|((jip_u64)e<<52)|(rounded&(hidden-1));
}
JIP_INLINE jip_u64 jip_div64(jip_u64 a,jip_u64 b) {
  const jip_u64 signbit=((jip_u64)1)<<63,hidden=((jip_u64)1)<<52;
  jip_u64 sign=(a^b)&signbit;if(!(a&(signbit-1)))return sign;
  jip_u64 ma=(a&(hidden-1))|hidden,mb=(b&(hidden-1))|hidden;
  int e=(int)((a>>52)&2047)-(int)((b>>52)&2047)+1023;
  if(ma<mb){ma<<=1;e--;}
  jip_u64 r=ma-mb,q=1;
  for(int i=0;i<52;i++){r<<=1;q<<=1;if(r>=mb){r-=mb;q|=1;}}
  if((r<<1)>mb||((r<<1)==mb&&(q&1)))q++;
  if(q&(((jip_u64)1)<<53)){q>>=1;e++;}
  return sign|((jip_u64)e<<52)|(q&(hidden-1));
}
JIP_INLINE jip_u32 jip_to32(jip_u64 a) {
  jip_u32 sign=(jip_u32)(a>>32)&0x80000000;const jip_u64 hidden=((jip_u64)1)<<52;
  if(!(a&((((jip_u64)1)<<63)-1)))return sign;
  int e=(int)((a>>52)&2047)-1023;if(e>127)return sign|0x7f800000;
  jip_u64 m=jip_round((a&(hidden-1))|hidden,29+(e< -126?-126-e:0));
  if(e< -126)return sign|(jip_u32)m;
  if(m>=0x1000000){m>>=1;e++;}if(e>127)return sign|0x7f800000;
  return sign|((jip_u32)(e+127)<<23)|((jip_u32)m&0x7fffff);
}
JIP_INLINE jip_i32 jip_wrap(jip_i32 i,jip_u32 size,jip_u32 wrap) {
  if(wrap){if(i<0)i+=(jip_i32)size;else if((jip_u32)i>=size)i-=(jip_i32)size;}return i;
}
JIP_INLINE jip_u32 jip_sample_bits(JIP_DEVICE const jip_sample *job,JIP_DEVICE const jip_u32 *source,JIP_CONSTANT const jip_kernel_params *p) {
  if(!job->valid)return 0;jip_u64 sum=0,denom=0;
  for(jip_u32 z=0;z<job->length[2];z++) {
    jip_i32 iz=jip_wrap(job->low[2]+(jip_i32)z,p->dim[2],p->wrap[2]);if(iz<0||(jip_u32)iz>=p->dim[2])continue;
    for(jip_u32 y=0;y<job->length[1];y++) {
      jip_i32 iy=jip_wrap(job->low[1]+(jip_i32)y,p->dim[1],p->wrap[1]);if(iy<0||(jip_u32)iy>=p->dim[1])continue;
      jip_u64 row=((jip_u64)(jip_u32)iz*p->dim[1]+(jip_u32)iy)*p->dim[0];
      for(jip_u32 x=0;x<job->length[0];x++) {
        jip_i32 ix=jip_wrap(job->low[0]+(jip_i32)x,p->dim[0],p->wrap[0]);if(ix<0||(jip_u32)ix>=p->dim[0])continue;
        jip_u32 w=jip_mul32(jip_mul32(job->coeff[x],job->coeff[JIP_KERNEL_TAPS+y]),job->coeff[2*JIP_KERNEL_TAPS+z]);
        sum=jip_add64(sum,jip_product64(w,source[row+(jip_u32)ix]));denom=jip_add64(denom,jip_from32(w));
      }
    }
  }
  return !(denom&((((jip_u64)1)<<63)-1))?0:jip_to32(jip_div64(sum,denom));
}
#endif
