#include <stdio.h>
#include <stdlib.h>
#include <kit/base/bits.h>
#include "GCiaB.h"


// For accessing metadata in allocation header. Its format is analogous
// to the following:
//   struct {
//     unsigned char marked : 1
//  	           , rooted : 1
//  	           , barrier : 1;
//  	           , padding : CHAR_BIT - 3
//   }
#define MARKED (1 << (CHAR_BIT - 1))
#define ROOTED (1 << (CHAR_BIT - 2))
#define BARRICADED (1 << (CHAR_BIT - 3))
#define PADDING_BITS (CHAR_BIT - 3)
#define MAX_PADDING (1 << PADDING_BITS) - 1
#define PADDING(n) ((n) & MAX_PADDING)

#define MARK(h)      h->meta |= MARKED
#define UNMARK(h)    h->meta &= ~MARKED
#define IS_MARKED(h) ((h->meta) & MARKED)? true:false
#define NOT_MARKED(h) !IS_MARKED(h)


#define ROOT(h)      h->meta |= ROOTED
#define UNROOT(h)    h->meta &= ~ROOTED
#define IS_ROOTED(h) ((h->meta) & ROOTED)? true:false
#define NOT_ROOTED(h) !IS_ROOTED(h)

#define IS_BARRICADED(h) ((h->meta) & BARRICADED)? true:false
#define NOT_BARRICADED(h) !IS_BARRICADED(h)

#define SET_PADDING(h, v)        \
	h->meta &= ~MAX_PADDING;     \
	h->meta |= PADDING(v)

#define GET_PADDING(h) ((h->meta) & MAX_PADDING)




static void printHeader(MSHeader *self)
{
	printf("Header @%p\n", self);
	printf("- meta: %d\n", self->meta);
	printf("- marked: %d\n", IS_MARKED(self));
	printf("- rooted: %d\n", IS_ROOTED(self));
	// printf("- padding: %d\n", GET_PADDING(self));
	// printf("- barricade: %d\n", IS_BARRICADED(self));
	// printf("- next: %p\n", self->next);
	// printf("- foreach: %p\n", self->foreach);
}


// backtracks to find header of assumed allocation
static void *getDataHeader(const void *ptr)
{
	char *byte = ((char*) ptr) - 1;
	while(*byte == 0)
		--byte;
	return (void*) (byte - offsetof(MSHeader, meta));
}


// returns pointer to first data byte in allocation
static void *getHeaderData(MSHeader *self)
{
	char *ptr = (char*) self;
	return ptr + sizeof(MSHeader) + GET_PADDING(self);
}


static void unmarkAll(MSCollector *self)
{
	MSHeader *i = self->firstHeader;
	while(i){
		UNMARK(i);
		i = i->next;
	}
}


static void markDataRecursive(const void *data);
static void markRecursive(MSHeader *root)
{
	if(NOT_MARKED(root)){
		MARK(root);
		if(root->foreach)
			root->foreach(getHeaderData(root), markDataRecursive);
	}
}

static void markDataRecursive(const void *data)
{
	MSHeader *header = getDataHeader(data);
	markRecursive(header);
}


static void markRootsRecursive(MSCollector *self)
{
	MSHeader *i = self->firstHeader;
	while(i){
		if(IS_ROOTED(i))
			markRecursive(i);
		i = i->next;
	}
}


static void filterUnmarked(MSCollector *self)
{
	MSHeader *i = self->firstHeader;
	while(i && i->next){
		if(NOT_MARKED(i->next)){
			MSHeader *temp = i->next->next;
			free(i->next);
			i->next = temp;
			self->unfreedAllocations -= 1;
		} else {
			i = i->next;
		}
	}


	// check first
	if(self->firstHeader && NOT_MARKED(self->firstHeader)){
		i = self->firstHeader->next;
		free(self->firstHeader);
		self->firstHeader = i;
		self->unfreedAllocations -= 1;
	}


	// ensure last == first if size is 1
	if(self->unfreedAllocations == 1)
		self->lastHeader = self->firstHeader;
}


void gc_sweep_(MSCollector *self)
{
	unmarkAll(self);
	markRootsRecursive(self);
	filterUnmarked(self);
}  


void gc_debug_(MSCollector *self)
{
	printf("GC Report:\n");
	printf(" - unfreed allocations: %u\n", (unsigned int) self->unfreedAllocations);
}


static MSHeader *newAllocation(size_t size
	                         , size_t alignment
	                         , void (*foreach)(void*, void(*)(const void*)))
{
	size_t bloat;
	if(alignment >= sizeof(MSHeader))
		bloat = alignment - sizeof(MSHeader);
	else
		bloat = (1 + sizeof(MSHeader) / alignment) * alignment;
	size_t required = sizeof(MSHeader) + bloat + size;
	MSHeader *ret = calloc(1, required);
	ret->next = NULL;
	ret->foreach = foreach;
	ret->meta = BARRICADED | PADDING(bloat);
	return ret;
}


void *gc_allocate_(MSCollector *self
	             , size_t size
	             , size_t alignment
	             , void (*foreach)(void*, void(*)(const void*)))
{
	MSHeader *header = newAllocation(size, alignment, foreach);

	if(self->unfreedAllocations == 0){
		self->firstHeader = header;
		self->lastHeader = header;
	} else {
		self->lastHeader->next = header;
		self->lastHeader = header;
	}

	self->unfreedAllocations++;

	return getHeaderData(header);	
}


void gc_clear_(MSCollector *self)
{
	MSHeader *header = self->firstHeader;
	while(header){
		MSHeader *temp = header;
		header = header->next;
		free(temp);
	}
	self->unfreedAllocations = 0;
	self->firstHeader = NULL;
	self->lastHeader = NULL;
}


void gc_root(void *allocation)
{
	MSHeader *header = getDataHeader(allocation);
	ROOT(header);
}


void gc_unroot(void *allocation)
{
	MSHeader *header = getDataHeader(allocation);
	UNROOT(header);
}


static void MSCollector_init(MSCollector *self)
{
	self->firstHeader = NULL;
	self->lastHeader  = NULL;
	self->unfreedAllocations = 0;
}


static MSCollector *gc_g = NULL;
static void prepareGC()
{
	if(!gc_g){
		gc_g = malloc(sizeof(MSCollector));
		MSCollector_init(gc_g);
	}
}


void printGC_(MSCollector *self)
{
	printf("GC ALLOCATION LIST\n");
	MSHeader *header = self->firstHeader;
	while(header){
		printHeader(header);
		header = header->next;
	}
}

void printGC()
{
	prepareGC();
	printGC_(gc_g);
}


void gc_sweep()
{
	prepareGC();
	gc_sweep_(gc_g);
}


void gc_debug()
{
	prepareGC();
	gc_debug_(gc_g);
}


void *gc_allocate(size_t size, size_t alignment, void (*foreach)(void*, void(*)(const void*)))
{
	prepareGC();
	return gc_allocate_(gc_g, size, alignment, foreach);
}


void gc_clear()
{
	prepareGC();
	gc_clear_(gc_g);
}


size_t gc_unfreed()
{
	prepareGC();
	return gc_g->unfreedAllocations;
}


void gc_include_(MSCollector *self, MSCollector *other)
{
	if(self->unfreedAllocations == 0){
		self->firstHeader = other->firstHeader;
		self->lastHeader = other->lastHeader;
	} else {
		self->lastHeader->next = other->firstHeader;
		self->lastHeader = other->lastHeader;
	}
	self->unfreedAllocations += other->unfreedAllocations;
}


void gc_include(MSCollector *other)
{
	gc_include_(gc_g, other);
}


void gc_init_(MSCollector *self)
{
	MSCollector_init(self);
}

