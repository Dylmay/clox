/**
 * @file list.h
 * @author Dylan Mayor
 * @brief header file for generic arraylist implementation
 *
 */
// TODO: move in to macro list
#ifndef __CLOX_UTIL_LIST_H__
#define __CLOX_UTIL_LIST_H__

#include <stddef.h>
#include "util/mem/mem.h"

/**
 * @brief list struct. Created using list_new() or list_of_type(). contents must be freed after use by parser_free()
 *
 * @see list_new()
 * @see list_free()
 *
 */
typedef struct __list {
	size_t cnt;
	size_t cap;
	size_t type_sz;
	uint8_t *data;
	uint8_t *head;
} list_t;

//! @brief for each function implementation
typedef void (*for_each_fn)(void *);

/**
 * @brief creates a new list to hold the given type
 *
 * @param elem_type the element type to hold
 *
 * @return list_t the created list
 *
 */
#define list_of_type(elem_type) (list_new(sizeof(elem_type)))

/**
 * @brief gets a value from the list. Indexing begins at zero
 * Note: Index bounds are only checked when the debug flag is set
 *
 * @param lst the list
 * @param idx the index of the item
 * @return void* the pointer to the index item
 */
void *list_get(list_t *lst, size_t idx);

/**
 * @brief sets the capacity of the list.
 * Any items that are outside of the new cap are removed
 *
 * @param lst the list
 * @param cap the new capacity of the list
 */
void list_set_cap(list_t *lst, size_t cap);

/**
 * @brief sets the item count of the list.
 * Any items that are outside of the new count are removed
 *
 * @param lst the list
 * @param cnt the new item count of the list
 */
void list_set_cnt(list_t *lst, size_t cnt);

/**
 * @brief frees the passe list and its' related items
 *
 * @param lst the list to free
 */
void list_free(list_t *lst);

/**
 * @brief pushes a number of records in to the list from the 'value' pointer
 *
 * @param lst the list to push to
 * @param value the value(s) to push
 * @param cnt the number of values that the passed pointer references
 * @return size_t the new list size
 */
size_t list_push_bulk(list_t *lst, const void *restrict value, size_t cnt);

/**
 * @brief pushes a single item to the list
 *
 * @param lst the list to push to
 * @param value the value to push
 */
static inline size_t list_push(list_t *lst, const void *restrict value)
{
	return list_push_bulk(lst, value, 1);
}

/**
 * @brief pops a number of records from the list
 *
 * @param lst the list to pop from
 * @param cnt the number of values to pop
 * @return void* the pointer of the last item popped. Valid until the list is updated/pushed to
 */
void *list_pop_bulk(list_t *lst, size_t cnt);

/**
 * @brief pops a single item from the list
 *
 * @param lst the list to pop from
 * @return void* the pointer of the item popped. Valid until the list is updated/pushed to
 */
static inline void *list_pop(list_t *lst)
{
	return list_pop_bulk(lst, 1);
}

/**
 * @brief peeks the value at an offset from the list head
 * NULL is returned when the list is empty
 *
 * @param lst the list to peek
 * @param offset the offset to peek at
 * @return void* the value at the position
 */
void *list_peek_offset(list_t *lst, size_t offset);

/**
 * @brief peeks the last value of the list
 * NULL is returned when the list is empty
 *
 * @param lst the list to peek
 * @return void* the last value
 */
static inline void *list_peek(list_t *lst)
{
	return list_peek_offset(lst, 0);
}

/**
 * @brief resets/clears the given list
 *
 * @param lst the list to clear
 */
void list_reset(list_t *lst);

/**
 * @brief creates a new arraylist to hold items at the given data size
 *
 * @param data_sz the size of the data item
 * @return list_t the new list
 */
static inline list_t list_new(size_t data_sz)
{
	return (list_t){
		.cnt = 0,
		.cap = 0,
		.type_sz = data_sz,
		.data = NULL,
		.head = NULL,
	};
}

/**
 * @brief gets the list size
 *
 * @param lst the list
 * @return size_t the list size/count
 */
static inline size_t list_size(const list_t *lst)
{
	return lst->cnt;
}

/**
 * @brief runs the for each function on each item within the list
 *
 * @param lst the list to process
 * @param fn list for each function
 */
void list_for_each (list_t *lst, for_each_fn fn);

#endif // __CLOX_UTIL_LIST_H__