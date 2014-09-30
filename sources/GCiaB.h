#ifndef GCiaB_MARK_AND_SWEEP
#define GCiaB_MARK_AND_SWEEP
#include <stddef.h>
#include <stdint.h>
#include <limits.h>
// This file implements mark and sweep garbage collection on top of
// malloc and free, such that all standard libc allocation functions
// may be used alongside GCiaB_object functions without interference.
//
// Note: at this point, functions provided here are neither reentrant 
// nor thread safe. Do not expect otherwise.


// data prepended before every allocation
typedef struct MSHeader {
	struct MSHeader *next;
	void (*foreach)(void*, void(*)(const void*));
	unsigned char meta;
} MSHeader;


// Represents self-contained garbage collector.
typedef struct {
	MSHeader *firstHeader;
	MSHeader *lastHeader;
	size_t unfreedAllocations;
	void (*onFree)(void*);
} GCiaB;


// Not sure if portable...
#define GCiaB_ALIGNMENT_OF(type)  \
	offsetof( struct {         \
			  	char x;        \
			  	type member;   \
		      }, member)


// Main interface.
#define GCiaB_primitive(self, type)        GCiaB_object(self, type, NULL)
#define GCiaB_object(self, type, foreach)  GCiaB_allocation(self, sizeof(type), GCiaB_ALIGNMENT_OF(type), foreach)
#define GCiaB_buffer(self, type, size)     GCiaB_allocation(self, (size) * sizeof(type), GCiaB_ALIGNMENT_OF(type), NULL)


void GCiaB_init(GCiaB *self);
void GCiaB_onFree(GCiaB*, void(*)(void*));
void GCiaB_sweep(GCiaB *self);  
void GCiaB_clean(GCiaB *self);  
void GCiaB_debug(GCiaB *self);
size_t GCiaB_unfreed(GCiaB *self);
void GCiaB_root(GCiaB *self, void *allocation);
void GCiaB_unroot(GCiaB *self, void *allocation);
void GCiaB_include(GCiaB *self
	             , GCiaB *other);
void *GCiaB_allocation(GCiaB *self
	                 , size_t size
	                 , size_t alignment
	                 , void (*foreach)(void*, void(*)(const void*)));
void GCiaB_print(GCiaB *self);


#endif