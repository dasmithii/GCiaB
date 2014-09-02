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
	gc_unroot(a1);
	gc_sweep();
	ASSERT_EQ(gc_unfreed(), 0);
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



TEST gc_multi()
{

	int *i1 = gc_primitive(int);
	int *i2 = gc_primitive(int);
	int *i3 = gc_primitive(int);
	PointerContainer *cont = gc_object(PointerContainer, PointerContainer_eachReference);
	PointerContainer *root = gc_object(PointerContainer, PointerContainer_eachReference);
	gc_root(root);
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
	ASSERT_EQ(gc_unfreed(), 5);
	gc_sweep();
//  |
// root-------  ----
//  |        | |   |
//  |      cont-----
//  |        |      
// i1       i3
	ASSERT_EQ(gc_unfreed(), 4);
	root->p1 = NULL;
//  |
// root      -  ----
//  |        | |   |
//  |      cont-----
//  |        |      
// i1       i3
	gc_sweep();
	ASSERT_EQ(gc_unfreed(), 2);
//  |
// root    
//  |      
//  |
//  |
// i1
	gc_unroot(root);
	gc_sweep();
	ASSERT_EQ(gc_unfreed(), 0);
	PASS();
}


GREATEST_SUITE(gc_suite) 
{
    RUN_TEST(gc_rootless);
    RUN_TEST(gc_roots);
    RUN_TEST(gc_multi);
}