#ifndef __CLOX_UTIL_LIST_H__
#define __CLOX_UTIL_LIST_H__

#include <stddef.h>
#include "util/mem/mem.h"

typedef struct list {
	size_t cnt;
	size_t cap;
	size_t type_sz;
	uint8_t *data;
	uint8_t *head;
} list_t;

#define list_new(elem_type) ((list_t){ 0, 0, sizeof(elem_type), NULL, NULL })

size_t list_write_to(list_t *lst, const void *__restrict__ value);

size_t list_write_bulk(list_t *lst, const void *__restrict__ value, size_t cnt);

uint8_t *list_get(list_t *lst, size_t idx);
void list_set_cap(list_t *lst, size_t cap);
void list_set_cnt(list_t *lst, size_t cnt);
void list_free(list_t *lst);

void list_push(list_t *lst, const void *__restrict__ value);
uint8_t *list_pop(list_t *lst);
uint8_t *list_peek_offset(list_t *lst, size_t offset);
void list_reset(list_t *lst);

static inline uint8_t *list_peek(list_t *lst)
{
	return list_peek_offset(lst, 0);
}

void list_for_each (list_t *lst, const void (*func)(void *));

#endif // __CLOX_UTIL_LIST_H__