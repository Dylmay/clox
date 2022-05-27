#include "list.h"
#include <string.h>

#define GROW_LIST(data_sz, pointer, old_cnt, new_cnt)                          \
	(reallocate(pointer, (data_sz)*old_cnt, (data_sz)*new_cnt))

#define FREE_LIST(data_sz, pointer, old_cnt)                                   \
	(reallocate(pointer, (data_sz)*old_cnt, 0))

static void __list_init(list_t *);
static void __list_adj_head(list_t *lst, int cnt);

size_t list_write_to(list_t *lst, const void *__restrict__ val)
{
	return list_write_bulk(lst, val, 1);
}

size_t list_write_bulk(list_t *lst, const void *__restrict__ val, size_t cnt)
{
	size_t idx = lst->cnt;

	if (lst->cap < lst->cnt + cnt) {
		list_set_cap(lst,
			     GROW_CAPACITY_AT_LEAST(lst->cnt + cnt, lst->cnt));
	}
	memcpy(lst->head, val, lst->type_sz * cnt);

	__list_adj_head(lst, (int)cnt);

	return idx;
}

void list_push(list_t *lst, const void *__restrict__ value)
{
	list_write_to(lst, value);
}

uint8_t *list_pop(list_t *lst)
{
	__list_adj_head(lst, -1);

	return lst->head;
}

uint8_t *list_peek_offset(list_t *lst, size_t offset)
{
	return lst->head - ((offset + 1) * lst->type_sz);
}

void list_reset(list_t *lst)
{
	lst->cnt = 0;
	lst->head = lst->data;
}

void list_free(list_t *lst)
{
	FREE_LIST(lst->type_sz, lst->data, lst->cap);
	__list_init(lst);
}

uint8_t *list_get(list_t *lst, size_t idx)
{
	return lst->data + (idx * (lst->type_sz));
}

void list_set_cap(list_t *lst, size_t cap)
{
	size_t old_cap = lst->cap;
	lst->cap = cap;
	lst->data = GROW_LIST(lst->type_sz, lst->data, old_cap, lst->cap);
	lst->head = lst->data + (lst->type_sz * lst->cnt);
}

void list_set_cnt(list_t *lst, size_t cnt)
{
	lst->cnt = cnt;
}

void list_for_each (list_t *lst, const void (*func)(void *))
{
	for (size_t idx = 0; idx < lst->cnt; idx++) {
		func(list_get(lst, idx));
	}
}

static void __list_init(list_t *lst)
{
	lst->cap = 0;
	lst->cnt = 0;
	lst->data = NULL;
	lst->head = NULL;
}

static void __list_adj_head(list_t *lst, int cnt)
{
	lst->cnt += cnt;
	lst->head += lst->type_sz * cnt;
}
