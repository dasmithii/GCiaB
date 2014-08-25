# Overview
GCiaB provides a simple GC algorithm designed to make life easier during the implementation of new programming languages. It isn't highly performant, employing mark & sweep collection atop malloc calls, but the API is super simple. If you're prototyping a language, or building a toy language, where GC hangs aren't detrimental, something like this may fit your needs. 

However, if X-language has high ambitions, this library should be replaced later on, once core features are functional and the prototyping stage is complete.



# Tutorial
The GCiaB interface consists of three primary functions: allocation, root specification, and sweeping. 

Basic datatypes are allocated using `gc_primitive(<type>)`:
```C
int *my_integer = gc_primitive(int);
```

Compound objects are created via `gc_object(<type>, <foreach>)`. Argument two permits access to internally-stored references.
```C
typedef struct {
    size_t size;
    int *buffer;
} Array;

void foreach(void *self, void (*func)(const void*))
{
    Array *arr = (Array*) self;
    for(int i = 0; i < arr->size; ++i)
        func((void*) &arr->buffer[i]);
}

Array *my_array = gc_object(Array, foreach);
```

In and out of scope values are marked as such in the rooting process, as explained [here](http://en.wikipedia.org/wiki/Tracing_garbage_collection#Reachability_of_an_object).
```C
int *v1 = gc_primitive(int);
int *v2 = gc_primitive(int);
gc_root(v1);
gc_sweep()   // will delete v2, but not v1, since it's rooted
```



# Installation
This project is managed by [Kit](https://github.com/dasmithii/Kit). If you haven't already, install kit via [this link](https://github.com/dasmithii/Kit#installation), then execute the following:
```
kit fetch GCiaB
```
Once complete, `#include <kit/GCiaB.h>` will import all API definitions when built with Kit. For those uninterested in a new project manager, just copy GCiaB sources directly.
