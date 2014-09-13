#include <kit/greatest.h>
#include "../sources/Global.h"



// ensures that all allocations are freed when no roots exist
TEST GCiaB_rootless() 
{
	int *a1 = GCiaB_primitive_g(int);
	int *a2 = GCiaB_primitive_g(int);
	int *a3 = GCiaB_primitive_g(int);
	ASSERT_EQ(GCiaB_unfreed_g(), 3);
	GCiaB_sweep_g();
	ASSERT_EQ(GCiaB_unfreed_g(), 0);
	PASS();
}


TEST GCiaB_roots()
{
	int *a1 = GCiaB_primitive_g(int);
	int *a2 = GCiaB_primitive_g(int);
	ASSERT_EQ(GCiaB_unfreed_g(), 2);
	GCiaB_root_g(a1);
	GCiaB_sweep_g();
	ASSERT_EQ(GCiaB_unfreed_g(), 1);
	GCiaB_unroot_g(a1);
	GCiaB_sweep_g();
	ASSERT_EQ(GCiaB_unfreed_g(), 0);
	PASS();
}





typedef struct {
	void *p1;
	void *p2;
} PointerContainer;


static void PointerContainer_eachReference(void *addr, void (*func)(const void*))
{
	PointerContainer *self = (PointerContainer*) addr;
	if(self->p1)
		func(self->p1);
	if(self->p2)
		func(self->p2);
}



TEST GCiaB_multi()
{

	int *i1 = GCiaB_primitive_g(int);
	int *i2 = GCiaB_primitive_g(int);
	int *i3 = GCiaB_primitive_g(int);
	PointerContainer *cont = GCiaB_object_g(PointerContainer, PointerContainer_eachReference);
	PointerContainer *root = GCiaB_object_g(PointerContainer, PointerContainer_eachReference);
	GCiaB_root_g(root);
	root->p1 = cont;
	root->p2 = i1;
	cont->p1 = i2;
	cont->p2 = cont;
//  |
// root-------  ----
//  |        | |   |
//  |      cont-----
//  |        |      
// i1   i2  i3
	ASSERT_EQ(GCiaB_unfreed_g(), 5);
	GCiaB_sweep_g();
//  |
// root-------  ----
//  |        | |   |
//  |      cont-----
//  |        |      
// i1       i3
	ASSERT_EQ(GCiaB_unfreed_g(), 4);
	root->p1 = NULL;
//  |
// root      -  ----
//  |        | |   |
//  |      cont-----
//  |        |      
// i1       i3
	GCiaB_sweep_g();
	ASSERT_EQ(GCiaB_unfreed_g(), 2);
//  |
// root    
//  |      
//  |
//  |
// i1
	GCiaB_unroot_g(root);
	GCiaB_sweep_g();
	ASSERT_EQ(GCiaB_unfreed_g(), 0);
	PASS();
}


GREATEST_SUITE(GCiaB_suite) 
{
    RUN_TEST(GCiaB_rootless);
    RUN_TEST(GCiaB_roots);
    RUN_TEST(GCiaB_multi);
}