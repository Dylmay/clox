#include "list.h"
#include <string.h>
#include <assert.h>

#define GROW_LIST(data_sz, pointer, old_cnt, new_cnt)                          \
	(reallocate(pointer, (data_sz)*old_cnt, (data_sz)*new_cnt))

#define FREE_LIST(data_sz, pointer, old_cnt)                                   \
	(reallocate(pointer, (data_sz)*old_cnt, 0))

static void __list_init(list_t *);
static void __list_inc_head(list_t *lst, size_t cnt);
static void __list_dec_head(list_t *lst, size_t cnt);

size_t list_push_bulk(list_t *lst, const void *restrict val, size_t cnt)
{
	size_t idx = lst->cnt;

	if (lst->cap < lst->cnt + cnt) {
		list_set_cap(lst,
			     GROW_CAPACITY_AT_LEAST(lst->cnt + cnt, lst->cnt));
	}

	if (val) {
		memcpy(lst->head, val, lst->type_sz * cnt);
	} else {
		memset(lst->head, 0, lst->type_sz * cnt);
	}

	__list_inc_head(lst, cnt);

	return idx;
}

void *list_pop_bulk(list_t *lst, size_t cnt)
{
	__list_dec_head(lst, cnt);

	return lst->head;
}

void *list_peek_offset(list_t *lst, size_t offset)
{
	if (lst->head == NULL) {
		return NULL;
	}

	assert(("Offset will fall out of array", offset + 1 <= lst->cnt));

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

void *list_get(list_t *lst, size_t idx)
{
	assert(("Index cannot be greater than list size", idx < lst->cnt));

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

	if (cnt > lst->cap) {
		list_set_cap(lst, GROW_CAPACITY_AT_LEAST(cnt, lst->cap));
	} else {
		lst->head = lst->data + (lst->type_sz * lst->cnt);
	}
}

void list_for_each(list_t *lst, for_each_fn func)
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

static void __list_inc_head(list_t *lst, size_t cnt)
{
	assert(("head will be outside of list alloc",
		lst->cnt + cnt <= lst->cap));

	lst->cnt += cnt;
	lst->head += lst->type_sz * cnt;
}

static void __list_dec_head(list_t *lst, size_t cnt)
{
	assert(("adjustment will cause overflow error", cnt <= lst->cnt));

	lst->cnt -= cnt;
	lst->head -= lst->type_sz * cnt;
}
