#include <kit/greatest.h>
#include "../sources/GCiaB.h"



// ensures that all allocations are freed when no roots exist
TEST gc_rootless() 
{
	int *a1 = gc_object(int, NULL);
	int *a2 = gc_object(int, NULL);
	int *a3 = gc_object(int, NULL);
	ASSERT_EQ(gc_unfreed(), 3);
	gc_sweep();
	ASSERT_EQ(gc_unfreed(), 0);
	PASS();
}


// TEST gc_roots()
// {
// 	int *a1 = gc_object(int, NULL);
// 	gc_sweep();
// 	ASSERT_EQ(gc_unfreed(), 2);
// 	gc_root(a1);

// 	PASS();
// }


GREATEST_SUITE(gc_suite) 
{
    RUN_TEST(gc_rootless);
    // RUN_TEST(gc_roots);
}