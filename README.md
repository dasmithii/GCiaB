# Overview
GCiaB provides a simple GC algorithm designed to make life easier during the implementation of new programming languages. It isn't highly performant, employing mark & sweep collection atop malloc calls, but the API is super simple. If you're prototyping a language, or building a toy language, where GC hangs aren't detrimental, something like this may fit your needs. 

However, if X-language has high ambitions, this library should be replaced later on, once core features are functional and the prototyping stage is complete.



# Tutorial
The GCiaB interface consists of three primary functions: allocation, root specification, and sweeping. 

Basic datatypes are allocated using `GCiaB_primitive_g(<type>)`:
```C
int *my_integer = GCiaB_primitive_g(int);
```

Compound objects are created via `GCiaB_object_g(<type>, <foreach>)`. Argument two permits access to internally-stored references.
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

Array *my_array = GCiaB_object_g(Array, foreach);
```

In and out of scope values are marked as such in the rooting process, as explained [here](http://en.wikipedia.org/wiki/Tracing_garbage_collection#Reachability_of_an_object).
```C
int *v1 = GCiaB_primitive_g(int);
int *v2 = GCiaB_primitive_g(int);
GCiaB_root_g(v1);
GCiaB_sweep_g()   // will delete v2, but not v1, since it's rooted
```
`GCiaB_sweep_g()` collects all allocations unreachable through root values and their reference networks (as defined by foreach functions).



# Non-global GC
Note that each function postfixed with a `_g` is applied to the global garbage collector. For a self-contained, non-global GC, declare `GCiaB gc;`, initialize it, and apply the same functions with a GCiaB pointer prepended to the argument list. In example:
```
int *i = GCiaB_primitive_g(int);
```
could be performed like this:
```
GCiaB gc;
GCiaB_init(&gc);
int *i = GCiaB_primitive(&gc, int);
```



# Installation
This project is managed by [Kit](https://github.com/dasmithii/Kit). If you haven't already, install kit via [this link](https://github.com/dasmithii/Kit#installation), then execute the following:
```
kit fetch GCiaB
```
Once complete, `#include <kit/GCiaB.h>` will import all API definitions when built with Kit. For those uninterested in a new project manager, just copy GCiaB sources directly.
