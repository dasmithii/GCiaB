#ifndef GCiaB_GLOBAL
#define GCiaB_GLOBAL
#include "GCiaB.h"


#define GCiaB_primitive_g(type)        GCiaB_object_g(type, NULL)
#define GCiaB_object_g(type, foreach)  GCiaB_allocation_g(sizeof(type), GCiaB_ALIGNMENT_OF(type), foreach)
#define GCiaB_buffer_g(type, size)     GCiaB_allocation_g((size) * sizeof(type), GCiaB_ALIGNMENT_OF(type), NULL)


void GCiaB_sweep_g(void);  
void GCiaB_debug_g(void);
size_t GCiaB_unfreed_g(void);
void GCiaB_root_g(void *allocation);
void GCiaB_unroot_g(void *allocation);
void GCiaB_include_g(GCiaB *other);
void *GCiaB_allocation_g(size_t size
	                   , size_t alignment
	                   , void (*foreach)(void*, void(*)(const void*)));
void GCiaB_print_g(void);


#endif