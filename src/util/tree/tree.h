#ifndef __CLOX_UTIL_TREE_H__
#define __CLOX_UTIL_TREE_H__

#include "tree_entry.h"

#define tree_of_type(data_type) (tree_new(sizeof(data_type)))

typedef struct __tree {
	tree_entry_t *root;
} tree_t;

static inline tree_t tree_new(size_t data_sz)
{
	return (tree_t){ .root = tree_entry_new(data_sz) };
}

static inline tree_entry_t *tree_root(tree_t tree)
{
	return tree.root;
}

static inline void tree_free(tree_t *tree)
{
	tree_entry_free(tree->root);
	reallocate(tree->root, sizeof(tree_entry_t) + tree->root->data_sz, 0);
}

#endif // __CLOX_UTIL_TREE_H__