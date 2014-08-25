#include <kit/greatest.h>
#include "../sources/GCiaB.h"



// ensures that all allocations are freed when no roots exist
TEST ms_rootless() 
{
	int *a1 = ms_allocation(int, NULL);
	int *a2 = ms_allocation(int, NULL);
	int *a3 = ms_allocation(int, NULL);
	ASSERT_EQ(ms_unfreed(), 3);
	ms_sweep();
	ASSERT_EQ(ms_unfreed(), 0);
	PASS();
}


// TEST ms_roots()
// {
// 	int *a1 = ms_allocation(int, NULL);
// 	ms_sweep();
// 	ASSERT_EQ(ms_unfreed(), 2);
// 	ms_root(a1);

// 	PASS();
// }


GREATEST_SUITE(ms_suite) 
{
    RUN_TEST(ms_rootless);
    // RUN_TEST(ms_roots);
}