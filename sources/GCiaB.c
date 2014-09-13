#include <stdio.h>
#include <stdlib.h>
#include <kit/base/bits.h>
#include "GCiaB.h"


// // data prepended before every allocation
// typedef struct MSHeader {
// 	struct MSHeader *next;
// 	void (*foreach)(void*, void(*)(const void*));
// 	unsigned char meta;
// } MSHeader;


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


static void unmarkAll(GCiaB *self)
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


static void markRootsRecursive(GCiaB *self)
{
	MSHeader *i = self->firstHeader;
	while(i){
		if(IS_ROOTED(i))
			markRecursive(i);
		i = i->next;
	}
}


static void filterUnmarked(GCiaB *self)
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


size_t GCiaB_unfreed(GCiaB *self)
{
	return self->unfreedAllocations;
}


void GCiaB_sweep(GCiaB *self)
{
	unmarkAll(self);
	markRootsRecursive(self);
	filterUnmarked(self);
}  


void GCiaB_debug(GCiaB *self)
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


void *GCiaB_allocation(GCiaB *self
	                 , size_t size
	                 , size_t alignment
	                 , void (*foreach)(void*, void(*)(const void*)))
{
	MSHeader *header = newAllocation(size, alignment, foreach);

	if(self->unfreedAllocations == 0){
		self->firstHeader = header;
		self->lastHeader = header;
	} else {
		((MSHeader*) self->lastHeader)->next = header;
		self->lastHeader = header;
	}

	self->unfreedAllocations++;

	return getHeaderData(header);	
}


void GCiaB_clean(GCiaB *self)
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


void GCiaB_root(GCiaB *self, void *allocation)
{
	MSHeader *header = getDataHeader(allocation);
	ROOT(header);
}


void GCiaB_unroot(GCiaB *self, void *allocation)
{
	MSHeader *header = getDataHeader(allocation);
	UNROOT(header);
}


void GCiaB_init(GCiaB *self)
{
	self->firstHeader = NULL;
	self->lastHeader  = NULL;
	self->unfreedAllocations = 0;
}


void GCiaB_print(GCiaB *self)
{
	printf("GC ALLOCATION LIST\n");
	MSHeader *header = self->firstHeader;
	while(header){
		printHeader(header);
		header = header->next;
	}
}


void GCiaB_include(GCiaB *self, GCiaB *other)
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
