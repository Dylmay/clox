#ifndef __LINKED_LIST_ITERATOR_H
#define __LINKED_LIST_ITERATOR_H

#include <stdbool.h>

#include "linked_list.h"

typedef struct list_iterator {
	linked_list_t list;
	node_t *start;
	node_t *cur;
	bool forward;
	bool used;
} list_iterator_t;

list_iterator_t list_iter(linked_list_t list, bool iter_forward);

bool list_iter_has_next(list_iterator_t *iter);

void *list_iter_next(list_iterator_t *iter);

void *list_iter_next(list_iterator_t *iter);

void *list_iter_next_circ(list_iterator_t *iter);

void *list_iter_find(list_iterator_t *iter, bool (*func)(void *));

int list_iter_find_idx(list_iterator_t *iter, bool (*func)(void *));

bool list_iter_any(list_iterator_t *iter, bool (*func)(void *));

bool list_iter_all(list_iterator_t *iter, bool (*func)(void *));

void list_iter_for_each(list_iterator_t *iter, void (*func)(void *));

#endif //__LINKED_LIST_ITERATOR_H