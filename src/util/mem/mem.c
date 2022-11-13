#include <stdlib.h>
#include "mem.h"

void *reallocate(void *pointer, size_t old_sz, size_t new_sz)
{
	if (new_sz == 0) {
		free(pointer);

		return NULL;
	}

	void *result = realloc(pointer, new_sz);

	if (result == NULL)
		exit(100);

	return result;
}