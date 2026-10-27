/* Force-include only for the engine and its private legacy support library.
   The X11/legacy CLI builds keep their original error/allocator behavior. */
#include <stdlib.h>
#include <stdio.h>
#include "engine_runtime.h"
#include "private_symbols.h"
#define malloc(n) jip_runtime_malloc(n)
#define calloc(n,s) jip_runtime_calloc(n,s)
#define realloc(p,n) jip_runtime_realloc(p,n)
#define free(p) jip_runtime_free(p)
#define exit(s) jip_runtime_exit(s)
#define JIP_SYSTEM_MALLOC(n) jip_runtime_malloc(n)
#define JIP_MEMORY_FAILURE(s) jip_runtime_raise(JIP_MEMORY,s,0)
