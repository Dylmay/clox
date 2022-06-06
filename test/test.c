#include <stdio.h>
#include <stddef.h>

#include "util/map/test_map.h"
#include "util/list/test_list.h"
#include "util/map/test_map.h"

void print_num(void *val)
{
	printf("%i\n", *((int *)val));
}

int main(int argc, const char *argv[])
{
	list_test_all();
	map_test_all();
}
