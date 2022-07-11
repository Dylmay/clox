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

typedef void (*for_each_fn)(void *);

#define list_of_type(elem_type) (list_new(sizeof(elem_type)))

size_t list_write_to(list_t *lst, const void *restrict value);

size_t list_write_bulk(list_t *lst, const void *restrict value, size_t cnt);

void *list_get(list_t *lst, size_t idx);
void list_set_cap(list_t *lst, size_t cap);
void list_set_cnt(list_t *lst, size_t cnt);
void list_adjust_cnt(list_t *lst, int adjust);
void list_free(list_t *lst);

void list_push(list_t *lst, const void *restrict value);
void *list_pop(list_t *lst);
void *list_peek_offset(list_t *lst, size_t offset);
void list_reset(list_t *lst);

static inline list_t list_new(size_t data_sz)
{
	return (list_t){
		0, 0, data_sz, NULL, NULL,
	};
}

static inline void *list_peek(list_t *lst)
{
	return list_peek_offset(lst, 0);
}

static inline size_t list_size(const list_t *lst)
{
	return lst->cnt;
}

static inline size_t list_cap(const list_t *lst)
{
	return lst->cap;
}

void list_for_each (list_t *lst, for_each_fn fn);

#endif // __CLOX_UTIL_LIST_H__