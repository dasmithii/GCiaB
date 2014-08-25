#include <stdio.h>
#include <stdlib.h>
#include "GCiaB.h"


// backtracks to find header of assumed allocation
static void *getDataHeader(const void *ptr)
{
	char *byte = ((char*) ptr) - 1;
	while(*byte == 0)
		--byte;
	return (void*) (byte - offsetof(MSHeader, foreach) - sizeof(void (*)(void*, void(*)(void*))));
	// TODO
}


// returns pointer to first data byte in allocation
static void *getHeaderData(MSHeader *self)
{
	char *ptr = (char*) self;
	ptr += sizeof(MSHeader);
	ptr += self->padding;
	return (void*) ptr;
}


static void unmarkAll(MSCollector *self)
{
	MSHeader *i = self->firstHeader;
	while(i){
		i->marked = 0;
		i = i->next;
	}
}


static void markDataRecursive(const void *data);
static void markRecursive(MSHeader *root)
{
	if(root->marked == 0){
		root->marked = 1;
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
		if(i->rooted)
			markRecursive(i);
		i = i->next;
	}
}


static void filterUnmarked(MSCollector *self)
{
	MSHeader *i = self->firstHeader;
	while(i && i->next){
		if(i->next->marked == 0){
			MSHeader *temp = i->next->next;
			free(i->next);
			i->next = temp;
			self->unfreedAllocations -= 1;
		} else {
			i = i->next;
		}
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
	size_t prefixedBytes = sizeof(MSHeader) > alignment? sizeof(MSHeader):alignment;
	size_t required = prefixedBytes + size;
	MSHeader *ret = calloc(1, required);
	ret->next = NULL;
	ret->marked = 0;
	ret->rooted = 0;
	ret->padding = prefixedBytes - sizeof(MSHeader);
	ret->foreach = foreach;
	ret->barrier = 1;
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
	header->rooted = 1;
}


void gc_unroot(void *allocation)
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


static MSCollector *gc_g = NULL;
static void prepareGC()
{
	if(!gc_g){
		gc_g = malloc(sizeof(MSCollector));
		MSCollector_init(gc_g);
	}
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
