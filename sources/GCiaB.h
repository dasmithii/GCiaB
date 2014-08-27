#ifndef GC_MARK_AND_SWEEP
#define GC_MARK_AND_SWEEP
#include <stddef.h>
#include <stdint.h>
#include <limits.h>
// This file implements mark and sweep garbage collection on top of
// malloc and free, such that all standard libc allocation functions
// may be used alongside gc_object functions without interference.
//
// Note: at this point, functions provided here are neither reentrant 
// nor thread safe. Do not expect otherwise.


// data prepended before every allocation
typedef struct MSHeader {
	struct MSHeader *next;
	void (*foreach)(void*, void(*)(const void*));
	unsigned char meta;
} MSHeader;


// represents a full garbage collect
typedef struct {
	MSHeader *firstHeader;
	MSHeader *lastHeader;
	size_t unfreedAllocations;
} MSCollector;


// not sure if portable
#define gc_ALIGNMENT_OF(type)  \
	offsetof( struct {         \
			  	char x;        \
			  	type member;   \
		      }, member)


// main interface
#define gc_primitive(type)        gc_object(type, NULL)
#define gc_object(type, foreach)  gc_allocate(sizeof(type), gc_ALIGNMENT_OF(type), foreach)
#define gc_buffer(type, size)     gc_allocate(size * sizeof(type), gc_ALIGNMENT_OF(type), NULL)
void gc_sweep();  
void gc_clear();  
void gc_debug();
size_t gc_unfreed();
void gc_root(void*);
void gc_unroot(void*);
void gc_include(MSCollector*);


// interface for non-global allocators
#define gc_object_(gc, type, foreach)  gc_allocate_(gc, sizeof(type), gc_ALIGNMENT_OF(type), foreach)
#define gc_array_(gc, type, size, foreach) gc_allocate_(gc, size * sizeof(type), gc_ALIGNMENT_OF(type), foreach)
void gc_sweep_(MSCollector*);  
void gc_clear_(MSCollector*);  
void gc_debug_(MSCollector*);
void gc_init_(MSCollector*);
void gc_include_(MSCollector*, MSCollector*);



// allocate with given size and alignment
void *gc_allocate(size_t, size_t, void (*)(void*, void(*)(const void*)));
void *gc_allocate_(MSCollector*, size_t, size_t, void (*)(void*, void(*)(const void*)));


// print headers of all allocations
void printGC_(MSCollector*);
void printGC();



#endif