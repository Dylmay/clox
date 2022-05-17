#include "list.h"
#include <string.h>

#define GROW_LIST(data_sz, pointer, old_cnt, new_cnt)                          \
	(reallocate(pointer, (data_sz)*old_cnt, (data_sz)*new_cnt))

#define FREE_LIST(data_sz, pointer, old_cnt)                                   \
	(reallocate(pointer, (data_sz)*old_cnt, 0))

static void list_init(list_t *);

size_t list_write_to(list_t *lst, void *val)
{
	return list_write_bulk(lst, val, 1);
}

size_t list_write_bulk(list_t *lst, void *val, size_t cnt)
{
	size_t idx = lst->cnt;

	if (lst->cap < lst->cnt + cnt) {
		size_t old_cap = lst->cap;

		lst->cap = GROW_CAPACITY_AT_LEAST(lst->cnt + cnt, old_cap);
		lst->data =
			GROW_LIST(lst->type_sz, lst->data, old_cap, lst->cap);
	}
	memcpy(lst->data + (lst->cnt * lst->type_sz), val, lst->type_sz * cnt);

	lst->cnt += cnt;

	return idx;
}

void list_free(list_t *lst)
{
	FREE_LIST(lst->type_sz, lst->data, lst->cap);
	list_init(lst);
}

char *list_get(list_t *lst, size_t idx)
{
	return lst->data + (idx * (lst->type_sz));
}

static void list_init(list_t *lst)
{
	lst->cap = 0;
	lst->cnt = 0;
	lst->data = NULL;
}