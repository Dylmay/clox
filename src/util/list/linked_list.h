#ifndef __LINKED_LIST_H__
#define __LINKED_LIST_H__

#include "util/common.h"

typedef struct node {
	struct node *prev;
	struct node *next;
	uint8_t data[];
} node_t;

typedef struct linked_list {
	size_t elem_size;
	struct node *head;
} linked_list_t;

#define linked_list_of_type(elem_type)                                         \
	((linked_list_t){ sizeof(elem_type), NULL })

void *linked_list_insert_front(linked_list_t *list, const void *data);

void *linked_list_insert_back(linked_list_t *list, const void *data);

void *linked_list_insert_after(linked_list_t *list, const void *data,
			       size_t idx);

void *linked_list_pop_front(linked_list_t *list);

void *linked_list_pop_back(linked_list_t *list);

void *linked_list_remove_at(linked_list_t *list, size_t idx);

size_t linked_list_size(linked_list_t *list);

#define linked_list_free(list)                                                 \
	do {                                                                   \
		while (linked_list_pop_front(&list))                           \
			;                                                      \
	} while (false)

#endif //__LINKED_LIST_H__