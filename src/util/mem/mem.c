#include <stdio.h>
#include <stdlib.h>
#include "mem.h"

void *reallocate(void *pointer, size_t old_sz, size_t new_sz)
{
	if (new_sz == 0) {
		free(pointer);

		return NULL;
	}

	void *result = realloc(pointer, new_sz);

	if (result == NULL) {
		printf("Rlox Memory Error: Unable to allocate additional memory (Prev size %lu; New size %lu)\n",
		       old_sz, new_sz);
		exit(100);
	}

	return result;
}