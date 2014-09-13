#include <kit/greatest.h>
extern SUITE(GCiaB_suite);


GREATEST_MAIN_DEFS();
int main(int argc, char *argv[]) {
	GREATEST_MAIN_BEGIN();   
	RUN_SUITE(GCiaB_suite);
	GREATEST_MAIN_END();
	return 0;
}