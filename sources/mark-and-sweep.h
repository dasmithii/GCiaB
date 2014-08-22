#ifndef GC_MARK_AND_SWEEP
#define GC_MARK_AND_SWEEP
#include <stddef.h>
#include <stdint.h>
#include <limits.h>
// This file implements mark and sweep garbage collection on top of
// malloc and free, such that all standard libc allocation functions
// may be used alongside ms_allocate functions without interference.
//
// Note: at this point, functions provided here are neither reentrant 
// nor thread safe. Do not expect otherwise.
//
// Note: if I/you/we were to re-implement a malloc-like function and
// build in GC directly, this code could be optimized greatly. TODO (!)


// describes internal references of allocation
typedef struct {
	size_t count;
	size_t *offsets;
} MSInternals;


// data prepended before every allocation
typedef struct MSHeader {
	struct MSHeader *next;
	MSInternals *internals;
	unsigned int marked : 1;
	unsigned int rooted : 1;
	unsigned int padding : CHAR_BIT - 3;
	unsigned int barrier : 1;
} MSHeader;


// represents a full garbage collect
typedef struct {
	MSHeader *firstHeader;
	MSHeader *lastHeader;
	size_t unfreedAllocations;
} MSCollector;


// not sure if portable
#define MS_ALIGNMENT_OF(type)  \
	offsetof( struct {         \
			  	char x;        \
			  	type member    \
		      }, member)


// main interface
#define ms_allocation(type, internals)  ms_allocate(sizeof(type), MS_ALIGNMENT_OF(type), internals)
#define ms_array(type, size, internals) ms_allocate(size * sizeof(type), MS_ALIGNMENT_OF(type), internals)
void ms_sweep();  
void ms_clear();  
void ms_debug();
void ms_root(void*);
void ms_unroot(void*);


// interface for non-global allocators
#define ms_allocation_(gc, type, internals)  ms_allocate_(gc, sizeof(type), MS_ALIGNMENT_OF(type), internals)
#define ms_array_(gc, type, size, internals) ms_allocate_(gc, size * sizeof(type), MS_ALIGNMENT_OF(type), internals)
void ms_sweep(MSCollector*);  
void ms_clear(MSCollector*);  
void ms_debug(MSCollector*);
void ms_init(MSCollector*);



// allocate with given size and alignment
void *ms_allocate(size_t, size_t, MSInternals*);
void *ms_allocate_(MSCollector*, size_t, size_t, MSInternals*);



#endif