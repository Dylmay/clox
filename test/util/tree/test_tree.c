#include "test_tree.h"
#include "tree.h"

#include <assert.h>

void tree_test_all()
{
	tree_test_new();
	tree_test_entry_val();
	tree_test_insert();
	tree_test_get();
	tree_test_traverse();
}

void tree_test_new()
{
	tree_t tree = tree_of_type(int);
	tree_free(&tree);
}

void tree_test_entry_val()
{
	tree_t tree = tree_of_type(int);
	int tree_value = 123;

	tree_entry_set_value(tree.root, &tree_value);
	assert(("Tree value incorrect",
		*((int *)tree_entry_get_value(tree.root)) == tree_value));

	tree_value = 456;

	tree_entry_set_value(tree.root, &tree_value);
	assert(("Tree value incorrect",
		*((int *)tree_entry_get_value(tree.root)) == tree_value));

	tree_free(&tree);
}

void tree_test_insert()
{
	tree_t tree = tree_of_type(int);

	for (int i = 0; i < ITEM_CNT; i++) {
		tree_entry_t *child = tree_entry_new_child(tree.root, &i);

		assert(("Tree value incorrect",
			*((int *)tree_entry_get_value(child)) == i));
	}

	assert(("Unexpected child count",
		tree_entry_child_cnt(tree.root) == ITEM_CNT));

	tree_free(&tree);
}

void tree_test_get()
{
	tree_t tree = tree_of_type(int);

	for (int i = 0; i < ITEM_CNT; i++) {
		tree_entry_new_child(tree.root, &i);
	}

	assert(("Unexpected child count",
		tree_entry_child_cnt(tree.root) == ITEM_CNT));

	for (int i = 0; i < ITEM_CNT; i++) {
		tree_entry_t *child = tree_entry_child_at(tree.root, i);

		assert(("Tree get incorrect",
			*((int *)tree_entry_get_value(child)) == i));
	}

	tree_free(&tree);
}

void tree_test_traverse()
{
	tree_entry_t *d_one;
	tree_entry_t *d_two;
	tree_entry_t *d_three;

	tree_t tree = tree_of_type(int);

	for (int depth = 1, val = 0; depth <= ITEM_CNT; depth++, val++) {
		switch (depth % 4) {
		case 1:
			tree_entry_new_child(tree.root, &val);
			break;
		case 2: {
			d_one = tree_entry_last_child(tree.root);
			tree_entry_new_child(d_one, &val);
		} break;
		case 3: {
			d_two = tree_entry_last_child(d_one);
			tree_entry_new_child(d_two, &val);
		} break;
		case 0: {
			d_three = tree_entry_last_child(d_two);
			tree_entry_new_child(d_three, &val);
		} break;
		default:
			assert(("Unexpected depth modulo", 0));
			break;
		}
	}

	tree_free(&tree);
}