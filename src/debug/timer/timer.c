#include "timer.h"

double time_function(void (*func)(void))
{
	timer_t start, end;
	time(&start);
	func();
	time(&end);

	return difftime(end, start);
}