#ifndef GC_MARK_AND_SWEEP
#define GC_MARK_AND_SWEEP
#include <stddef.h>
#include <stdint.h>
#include <limits.h>
// This file implements mark and sweep garbage collection on top of
// malloc and free, such that all standard libc allocation functions
// may be used alongside ms_allocation functions without interference.
//
// Note: at this point, functions provided here are neither reentrant 
// nor thread safe. Do not expect otherwise.


// data prepended before every allocation
typedef struct MSHeader {
	struct MSHeader *next;
	void (*foreach)(void*, void(*)(const void*));
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
			  	type member;   \
		      }, member)


// main interface
#define ms_allocation(type, foreach)  ms_allocate(sizeof(type), MS_ALIGNMENT_OF(type), foreach)
#define ms_array(type, size, foreach) ms_allocate(size * sizeof(type), MS_ALIGNMENT_OF(type), foreach)
void ms_sweep();  
void ms_clear();  
void ms_debug();
size_t ms_unfreed();
void ms_root(void*);
void ms_unroot(void*);


// interface for non-global allocators
#define ms_allocation_(gc, type, foreach)  ms_allocate_(gc, sizeof(type), MS_ALIGNMENT_OF(type), foreach)
#define ms_array_(gc, type, size, foreach) ms_allocate_(gc, size * sizeof(type), MS_ALIGNMENT_OF(type), foreach)
void ms_sweep_(MSCollector*);  
void ms_clear_(MSCollector*);  
void ms_debug_(MSCollector*);
void ms_init_(MSCollector*);



// allocate with given size and alignment
void *ms_allocate(size_t, size_t, void (*)(void*, void(*)(const void*)));
void *ms_allocate_(MSCollector*, size_t, size_t, void (*)(void*, void(*)(const void*)));



#endif