#include <kit/greatest.h>
extern SUITE(gc_suite);


GREATEST_MAIN_DEFS();
int main(int argc, char *argv[]) {
	GREATEST_MAIN_BEGIN();   
	RUN_SUITE(gc_suite);
	GREATEST_MAIN_END();
	return 0;
}