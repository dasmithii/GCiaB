#include <stdio.h>
#include <stdlib.h>
#include "mark-and-sweep.h"


// backtracks to find header of assumed allocation
static void *getDataHeader(void *ptr)
{
	char *byte = ((char*) ptr) - 1;
	while(*byte == 0)
		--byte;
	return (void*) (byte - offsetof(MSHeader, internals) - sizeof(MSInternals));
	// TODO: fix the above line, which depends on an undefined bahavior (!)
}


// returns pointer to first data byte in allocation
static void *getHeaderData(MSHeader *self)
{
	char *ptr = (char*) self;
	ptr += sizeof(MSHeader);
	ptr += self->padding;
	return (void*) ptr;
}


static void *getReferenceData(MSHeader *self, size_t i)
{
	char *ptr = getHeaderData(self);
	ptr += self->internals->offsets[i];
	return *((void**) ptr);
}


static void *getReferenceHeader(MSHeader *self, size_t i)
{
	void *data = getReferenceData(self, i);
	return getDataHeader(data);
}


static void unmarkAll(MSCollector *self)
{
	MSHeader *i = self->firstHeader;
	printf("\n\n%p\n\n", i);
	printf("\n\n%p\n\n", i->next);
	// while(i){
	// 	i->marked = 0;
	// 	i = i->next;
	// }
}

static void markRecursive(MSHeader*);
static void markReferenceHeaders(MSHeader *self)
{
	if(self->internals){
		for(int i = 0; i < self->internals->count; ++i)
			markRecursive(getReferenceHeader(self, i));
	}
}


static void markRecursive(MSHeader *root)
{
	if(root->marked == 0){
		root->marked = 1;
		markReferenceHeaders(root);
	}
}


static void markRootsRecursive(MSCollector *self)
{
	MSHeader *i = self->firstHeader;
	while(i){
		if(i->rooted)
			markRecursive(i);
		i = i->next;
	}
}


static void filterUnmarked(MSCollector *self)
{
	MSHeader *i = self->firstHeader;
	while(i){
		if(i->next){
			if(i->next->marked == 0){
				MSHeader *temp = i->next->next;
				free(i->next);
				i->next = temp;
				self->unfreedAllocations -= 1;
			}
		}
		i = i->next;
	}


	// check first
	if(self->firstHeader && self->firstHeader->marked == 0){
		i = self->firstHeader->next;
		free(self->firstHeader);
		self->firstHeader = i;
		self->unfreedAllocations -= 1;
	}


	// ensure last == first if size is 1
	if(self->unfreedAllocations == 1)
		self->lastHeader = self->firstHeader;
}


void ms_sweep_(MSCollector *self)
{
	unmarkAll(self);
	// markRootsRecursive(self);
	// filterUnmarked(self);
}  


void ms_debug_(MSCollector *self)
{
	printf("GC Report:\n");
	printf(" - unfreed allocations: %u\n", (unsigned int) self->unfreedAllocations);
}


static MSHeader *newAllocation(size_t size
	                         , size_t alignment
	                         , MSInternals *internals)
{
	size_t prefixedBytes = sizeof(MSHeader) > alignment? sizeof(MSHeader):alignment;
	size_t required = prefixedBytes + size;
	MSHeader *ret = calloc(1, required);
	ret->next = NULL;
	ret->marked = 0;
	ret->rooted = 0;
	ret->padding = prefixedBytes - sizeof(MSHeader);
	ret->internals = internals;
	ret->barrier = 1;
	return ret;
}


void *ms_allocate_(MSCollector *self
	             , size_t size
	             , size_t alignment
	             , MSInternals *internals)
{
	MSHeader *header = newAllocation(size, alignment, internals);

	if(self->unfreedAllocations == 0){
		self->firstHeader = header;
		self->lastHeader = header;
	} else {
		self->lastHeader->next = header;
		self->lastHeader = header;
	}
	printf("next address: %p\n", &(header->next));
	printf("variable address: %p\n", &(self->unfreedAllocations));
	self->unfreedAllocations++;

	return getHeaderData(header);	
}


void ms_clear_(MSCollector *self)
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


void ms_root(void *allocation)
{
	MSHeader *header = getDataHeader(allocation);
	header->rooted = 1;
}


void ms_unroot(void *allocation)
{
	MSHeader *header = getDataHeader(allocation);
	header->rooted = 0;
}


static void MSCollector_init(MSCollector *self)
{
	self->firstHeader = NULL;
	self->lastHeader  = NULL;
	self->unfreedAllocations = 0;
}


static MSCollector *ms_g = NULL;
static void prepareGC()
{
	if(!ms_g){
		ms_g = malloc(sizeof(MSCollector));
		MSCollector_init(ms_g);
	}
}


void ms_sweep()
{
	prepareGC();
	ms_sweep_(ms_g);
}


void ms_debug()
{
	prepareGC();
	ms_debug_(ms_g);
}


void *ms_allocate(size_t size, size_t alignment, MSInternals *internals)
{
	prepareGC();
	return ms_allocate_(ms_g, size, alignment, internals);
}


void ms_clear()
{
	prepareGC();
	ms_clear_(ms_g);
}


size_t ms_unfreed()
{
	prepareGC();
	return ms_g->unfreedAllocations;
}
