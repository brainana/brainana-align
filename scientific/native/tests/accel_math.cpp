#include "accel_kernel.h"
#include <cmath>
#include <cstdio>
#include <cstring>
#include <random>
static uint32_t bits(float x){uint32_t b;std::memcpy(&b,&x,4);return b;}
static uint64_t dbits(double x){uint64_t b;std::memcpy(&b,&x,8);return b;}
static float value(uint32_t b){float f;std::memcpy(&f,&b,4);return f;}
static double dvalue(uint64_t b){double f;std::memcpy(&f,&b,8);return f;}
#define CHECK(x) do{if(!(x)){std::fprintf(stderr,"MATH_FAIL line %d case %d\n",__LINE__,i);return 1;}}while(0)
int main(){std::mt19937 random(8917);for(int i=0;i<1000000;i++) {
 uint32_t a=random(),b=random(),c=random();if((a&0x7f800000)==0x7f800000)a&=~0x00800000;if((b&0x7f800000)==0x7f800000)b&=~0x00800000;if((c&0x7f800000)==0x7f800000)c&=~0x00800000;
 float x=value(a),y=value(b);float product=x*y;
 CHECK(jip_mul32(a,b)==bits(product));CHECK(jip_from32(a)==dbits((double)x));
 uint64_t p=jip_product64(a,b);CHECK(p==dbits((double)x*y));
 double total=dvalue(p)+(double)value(c);CHECK(jip_add64(p,jip_from32(c))==dbits(total));
 CHECK(jip_to32(p)==bits((float)dvalue(p)));
 if(y!=0&&x!=0){uint64_t q=jip_div64(jip_from32(a),jip_from32(b));CHECK(q==dbits((double)x/y));CHECK(jip_to32(q)==bits((float)((double)x/y)));}
 }
 std::puts("ACCEL_MATH_RESULT million finite cases: exact multiply/add/divide/conversion; includes subnormal float32");return 0;}
