#define main unused_api_fixture_main
#include "api_driver.c"
#undef main
/* Simulate an embedder that already links the legacy helper library. */
extern float *float_vector(ptrdiff_t);
extern void free_float_vector(float*);
int main(void) {
  float *v=float_vector(4);v[0]=123;free_float_vector(v);
  jip_engine *c=make(1,4);if(!c||jip_engine_register(c))return 1;
  jip_engine_destroy(c);puts("MIXED_LINK_RESULT private allocator isolation passed");return 0;
}
