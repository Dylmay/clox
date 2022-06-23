#include "tree_entry.h"

#include <string.h>

#include "util/mem/mem.h"

#define SIZEOF_ENTRY(data_sz) (sizeof(tree_entry_t) + (data_sz))

static inline tree_entry_t tree_entry_create(size_t data_sz)
{
	return (tree_entry_t){
		.parent = NULL,
		.children = list_new(SIZEOF_ENTRY(data_sz)),
		.data_sz = data_sz,
	};
}

tree_entry_t *tree_entry_new(size_t data_sz)
{
	tree_entry_t *entry = reallocate(NULL, 0, SIZEOF_ENTRY(data_sz));

	entry->parent = NULL;
	entry->children = list_new(SIZEOF_ENTRY(data_sz));
	entry->data_sz = data_sz;

	return entry;
}

tree_entry_t *tree_entry_new_child(tree_entry_t *entry,
				   const void *restrict child_data)
{
	tree_entry_t new_entry = tree_entry_create(entry->data_sz);
	new_entry.parent = entry;
	list_push(&entry->children, &new_entry);

	tree_entry_t *list_entry = (tree_entry_t *)list_peek(&entry->children);
	memcpy(list_entry->data, child_data, list_entry->data_sz);

	return list_entry;
}

void tree_entry_set_value(tree_entry_t *restrict entry,
			  const void *restrict value)
{
	memcpy(entry->data, value, entry->data_sz);
}

void tree_entry_free(tree_entry_t *entry)
{
	list_for_each (&entry->children, (for_each_fn)&tree_entry_free)
		;
	list_free(&entry->children);
}
