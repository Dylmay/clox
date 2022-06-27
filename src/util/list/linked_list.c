#include <stdlib.h>
#include <string.h>

#include "linked_list.h"

static node_t *__node_alloc(const void *data, size_t data_sz);
static node_t *__node_get(linked_list_t *list, size_t idx);
static void *__node_free(node_t *node, size_t data_sz);

void *linked_list_insert_front(linked_list_t *list, const void *data)
{
	if (!list)
		return NULL;

	node_t *node = __node_alloc(data, list->elem_size);
	node_t *head = list->head;

	if (!node)
		return NULL;

	if (head) {
		node->next = head;
		node->prev = head->prev;
		head->prev = node;
		node->prev->next = node;
	}

	list->head = node;

	return node->data;
}

void *linked_list_insert_back(linked_list_t *list, const void *data)
{
	if (!list)
		return NULL;

	node_t *head = list->head;

	if (!head)
		return linked_list_insert_front(list, data);

	node_t *node = __node_alloc(data, list->elem_size);
	node_t *back = head->prev;

	if (!node)
		return NULL;

	node->prev = back;
	head->prev = node;
	back->next = node;
	node->next = head;

	return node->data;
}

void *linked_list_insert_after(linked_list_t *list, const void *data,
			       size_t idx)
{
	if (!list)
		return NULL;

	if (!list->head) {
		if (!idx)
			return linked_list_insert_front(list, data);
		else
			return NULL;
	}

	node_t *ret_node = __node_get(list, idx);

	if (!ret_node)
		return NULL;

	node_t *new = __node_alloc(data, list->elem_size);
	node_t *next_node = ret_node->next;

	new->prev = ret_node;
	ret_node->next = new;

	new->next = next_node;
	next_node->prev = new;

	return new->data;
}

void *linked_list_pop_front(linked_list_t *list)
{
	if (!list)
		return NULL;

	node_t *head = list->head;

	if (!head)
		return NULL;

	if (head == head->next) {
		list->head = NULL;
	} else {
		node_t *tail = head->prev;

		list->head = head->next;
		tail->next = list->head;
		list->head->prev = tail;
	}

	return __node_free(head, list->elem_size);
}

void *linked_list_pop_back(linked_list_t *list)
{
	if (!list)
		return NULL;

	node_t *head = list->head;

	if (!head)
		return NULL;

	if (head == head->next)
		return linked_list_pop_front(list);

	node_t *tail = head->prev;

	head->prev = tail->prev;
	tail->prev->next = head;

	return __node_free(tail, list->elem_size);
}

void *linked_list_remove_at(linked_list_t *list, size_t idx)
{
	if (!list || !list->head)
		return NULL;

	if (!idx)
		return linked_list_pop_front(list);

	node_t *ret_node = __node_get(list, idx);

	if (!ret_node)
		return NULL;

	ret_node->prev->next = ret_node->next;
	ret_node->next->prev = ret_node->prev;

	return __node_free(ret_node, list->elem_size);
}

size_t linked_list_size(linked_list_t *list)
{
	if (!list || !list->head)
		return 0;

	int i = 1;
	node_t *cur_node = list->head;
	while ((cur_node = cur_node->next) != list->head)
		i++;

	return i;
}

static node_t *__node_alloc(const void *data, size_t data_sz)
{
	node_t *node = malloc(sizeof(node_t) + data_sz);

	if (node) {
		memmove(node->data, data, data_sz);
		node->next = node;
		node->prev = node;

		return node;
	} else {
		return NULL;
	}
}

static void *__node_free(node_t *node, size_t data_sz)
{
	void *data = malloc(data_sz);
	memmove(data, node->data, data_sz);

	free(node);

	return data;
}

static node_t *__node_get(linked_list_t *list, size_t idx)
{
	node_t *cur_node = list->head;
	for (size_t i = 0; i != idx && (cur_node = cur_node->next); i++)
		if (cur_node == list->head)
			return NULL;

	return cur_node;
}