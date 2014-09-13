#include <stdlib.h>
#include "Global.h"


static GCiaB *gc = NULL;
static inline void prepare()
{
	if(!gc){
		gc = malloc(sizeof(GCiaB));
		GCiaB_init(gc);
	}
}


void GCiaB_sweep_g(void)
{
	prepare();
	GCiaB_sweep(gc);
}


void GCiaB_debug_g(void)
{
	prepare();
	GCiaB_debug(gc);
}


size_t GCiaB_unfreed_g(void)
{
	prepare();
	return GCiaB_unfreed(gc);
}


void GCiaB_root_g(void *allocation)
{
	prepare();
	GCiaB_root(gc, allocation);
}


void GCiaB_unroot_g(void *allocation)
{
	prepare();
	GCiaB_unroot(gc, allocation);
}


void GCiaB_include_g(GCiaB *other)
{
	prepare();
	GCiaB_include(gc, other);
}


void *GCiaB_allocation_g(size_t size
	                   , size_t alignment
	                   , void (*foreach)(void*, void(*)(const void*)))
{
	prepare();
	return GCiaB_allocation(gc, size, alignment, foreach);
}


void GCiaB_print_g(void)
{
	prepare();
	GCiaB_print(gc);
}
