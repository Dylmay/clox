#include "list_iterator.h"

#define NEXT_ITER(iter) (						\
	iter->forward	? iter->cur->next			\
					: iter->cur->prev			\
)

list_iterator_t list_iter(linked_list_t list, bool iter_forward)
{
	if (!list.head)
		return (list_iterator_t){ list, NULL, NULL, false, false};

	if (iter_forward)
		return (list_iterator_t){ list, list.head, list.head, iter_forward, false};
	else
		return (list_iterator_t){ list, list.head->prev, list.head->prev, iter_forward, false};
}

bool list_iter_has_next(list_iterator_t* iter)
{
	return (iter && iter->cur) && (!iter->used || iter->cur != iter->start);
}

void* list_iter_next(list_iterator_t* iter)
{
	if (!list_iter_has_next(iter))
		return NULL;

	return list_iter_next_circ(iter);
}

void* list_iter_next_circ(list_iterator_t* iter)
{
	if (!iter)
		return NULL;

	if (!iter->cur)
		return NULL;

	void* data = iter->cur->data;

	iter->used = true;

	iter->cur = NEXT_ITER(iter);

	return data;
}


void* list_iter_find(list_iterator_t* iter, bool (*func)(void*))
{
	while (list_iter_has_next(iter)) {
		void* data = list_iter_next(iter);

		if (func(data))
			return data;
	}

	return NULL;
}

int list_iter_find_idx(list_iterator_t* iter, bool (*func)(void*))
{
	int idx = 0;

	while (list_iter_has_next(iter)) {
		void* data = list_iter_next(iter);

		if (func(data))
			return idx;

		idx++;
	}

	return -1;
}

bool list_iter_any(list_iterator_t* iter, bool (*func)(void*))
{
	return list_iter_find(iter, func) != NULL;
}

bool list_iter_all(list_iterator_t* iter, bool (*func)(void*))
{
	void* data;
	while ( (data = list_iter_next(iter)) ) if (!func(data)) return false;

	return true;
}

void list_iter_for_each(list_iterator_t* iter, void (*func)(void*))
{
	void* data;
	while ( (data = list_iter_next(iter)) ) func(data);
}