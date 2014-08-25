#ifndef GCIAB_API
#define GCIAB_API


// GC algorithms.
#define MARK_AND_SWEEP 0
#define REFERENCE_COUNTING 1
#define THREE_COLOR 2


// Change this line for a different default collector. Note
// that each alorithm is available regardless, but X GC type
// is more conveniently usable if specified here.
#define DEFAULT_COLLECTOR MARK_AND_SWEEP


// Arrange some convenience functions for the default collector.
#if DEFAULT_COLLECTOR == MARK_AND_SWEEP
#include "mark-and-sweep.h"
#elif DEFAULT_COLLECTOR == REFERENCE_COUNTING
#include "reference-counting.h"
#elif DEFAULT_COLLECTOR == THREE_COLOR
#include "three-color.h"
#endif


// End include gaurd.
#endif