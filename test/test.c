#include <stdio.h>

#include "util/map/test_map.h"
#include "util/list/list.h"

void print_num(void *val)
{
	printf("%i\n", *((int *)val));
}

int main(int argc, const char *argv[])
{
	printf("Hwllo world!!");
	list_t lst = list_of_type(int);

	for (int i = 0; i < 10; i++) {
		list_push(&lst, &i);
	}

	list_for_each (&lst, &print_num)
		;
	list_free(&lst);
	test_map();
}
