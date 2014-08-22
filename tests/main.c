#include <kit/greatest.h>
extern SUITE(ms_suite);


GREATEST_MAIN_DEFS();
int main(int argc, char *argv[]) {
	GREATEST_MAIN_BEGIN();   
	RUN_SUITE(ms_suite);
	GREATEST_MAIN_END();
	return 0;
}