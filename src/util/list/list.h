#ifndef __CLOX_UTIL_LIST_H__
#define __CLOX_UTIL_LIST_H__

#include <stddef.h>
#include "util/mem/mem.h"

typedef struct list {
	size_t cnt;
	size_t cap;
	size_t type_sz;
	char *data;
} list_t;

#define list_new(elem_type) ((list_t){ 0, 0, sizeof(elem_type), NULL })

size_t list_write_to(list_t *lst, void *value);

size_t list_write_bulk(list_t *lst, void *value, size_t cnt);

char *list_get(list_t *lst, size_t idx);
void list_free(list_t *lst);

#endif // __CLOX_UTIL_LIST_H__