#include <kit/greatest.h>
#include "../sources/GCiaB.h"



// ensures that all allocations are freed when no roots exist
TEST gc_rootless() 
{
	int *a1 = gc_primitive(int);
	int *a2 = gc_primitive(int);
	int *a3 = gc_primitive(int);
	ASSERT_EQ(gc_unfreed(), 3);
	gc_sweep();
	ASSERT_EQ(gc_unfreed(), 0);
	PASS();
}


TEST gc_roots()
{
	int *a1 = gc_primitive(int);
	int *a2 = gc_primitive(int);
	ASSERT_EQ(gc_unfreed(), 2);
	gc_root(a1);
	gc_sweep();
	ASSERT_EQ(gc_unfreed(), 1);
	PASS();
}


GREATEST_SUITE(gc_suite) 
{
    RUN_TEST(gc_rootless);
    RUN_TEST(gc_roots);
}