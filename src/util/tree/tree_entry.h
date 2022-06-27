#ifndef __CLOX_UTIL_TREE_ENTRY_H__
#define __CLOX_UTIL_TREE_ENTRY_H__

#include "util/common.h"
#include "util/list/list.h"

#define DEFAULT_TREE_ENTRY_CNT 4

typedef struct __tree_entry {
	struct __tree_entry *parent;
	list_t children;
	size_t data_sz;
	uint8_t data[];
} tree_entry_t;

tree_entry_t *tree_entry_new(size_t data_sz);

tree_entry_t *tree_entry_new_child(tree_entry_t *entry,
				   const void *restrict child_data);

static inline void tree_entry_set_parent(tree_entry_t *restrict entry,
					 tree_entry_t *restrict parent)
{
	entry->parent = parent;
}

static inline tree_entry_t *
tree_entry_get_parent(const tree_entry_t *restrict entry)
{
	return entry->parent;
}

void tree_entry_free(tree_entry_t *entry);

void tree_entry_set_value(tree_entry_t *restrict entry,
			  const void *restrict value);

static inline void *tree_entry_get_value(tree_entry_t *entry)
{
	return entry->data;
}

static inline tree_entry_t *tree_entry_child_at(tree_entry_t *restrict entry,
						size_t idx)
{
	if (!entry || idx >= list_size(&entry->children)) {
		return NULL;
	}

	return (tree_entry_t *)list_get(&entry->children, idx);
}

static inline size_t tree_entry_child_cnt(const tree_entry_t *restrict entry)
{
	return list_size(&entry->children);
}

static inline tree_entry_t *tree_entry_last_child(tree_entry_t *restrict entry)
{
	return tree_entry_child_at(entry, tree_entry_child_cnt(entry) - 1);
}

static inline bool tree_entry_has_child(const tree_entry_t *restrict entry)
{
	return entry && tree_entry_child_cnt(entry);
}

#endif // __CLOX_UTIL_TREE_ENTRY_H__