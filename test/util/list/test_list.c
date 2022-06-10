#include "test_list.h"
#include "util/list/list.h"

#include <assert.h>
#include <string.h>
#include <stdio.h>

struct value {
	int val;
	size_t len;
	char *str;
};

void list_test_all()
{
	list_test_new();
	list_test_push();
	list_test_pop();
	list_test_reset();
	list_test_peek_offset();
	list_test_write_to_bulk();
}

void list_test_new()
{
	list_t lst = list_of_type(struct value);

	assert(("Size of value is incorrect",
		lst.type_sz == sizeof(struct value)));
	assert(("Count is not 0", !lst.cnt));
}

void list_test_push()
{
	list_t lst = list_of_type(struct value);
	struct value val = (struct value){ .val = 1,
					   .len = sizeof("Hello world!!") + 1,
					   .str = "Hello world!!" };

	list_push(&lst, &val);
	assert(("Count has not increased", lst.cnt == 1));
	const struct value *retval = (struct value *)list_peek(&lst);

	assert(("Strings do not match",
		strncmp(retval->str, "Hello world!!", retval->len) == 0));

	list_free(&lst);
}

void list_test_pop()
{
	list_t lst = list_of_type(struct value);
	struct value val = (struct value){ .val = 1,
					   .len = sizeof("Hello world!!") + 1,
					   .str = "Hello world!!" };

	list_push(&lst, &val);
	const struct value *retval = (struct value *)list_pop(&lst);

	assert(("Strings do not match",
		strncmp(retval->str, "Hello world!!", retval->len) == 0));

	assert(("Count has not decreased", lst.cnt == 0));

	list_free(&lst);
}

void list_test_reset()
{
	list_t lst = list_of_type(struct value);
	struct value val = (struct value){
		.val = 1,
	};
	list_push(&lst, &val);
	list_push(&lst, &val);
	list_push(&lst, &val);
	assert(("Count is not 3", lst.cnt == 3));
	list_reset(&lst);
	list_push(&lst, &val);
	assert(("Count is not 1", lst.cnt == 1));

	list_free(&lst);
}

void list_test_peek_offset()
{
	list_t lst = list_of_type(struct value);
	struct value val = (struct value){
		.val = 1,
	};

	list_push(&lst, &val);
	val.val = 2;
	list_push(&lst, &val);
	val.val = 3;
	list_push(&lst, &val);

	for (int i = 0; i < lst.cnt; i++) {
		assert(("offset peek incorrect",
			3 - i == ((struct value *)list_peek_offset(&lst, i))
					 ->val));
	}

	list_free(&lst);
}

void list_test_write_to_bulk()
{
	list_t lst = list_of_type(struct value);

	struct value val_arr[] = {
		{
			.val = 1,
		},
		{
			.val = 2,
		},
		{
			.val = 3,
		},
		{
			.val = 4,
		},
		{
			.val = 5,
		},
		{
			.val = 6,
		},
		{
			.val = 7,
		},
		{
			.val = 8,
		},
		{
			.val = 9,
		},
	};

	list_write_bulk(&lst, val_arr, sizeof(val_arr) / sizeof(struct value));
	assert(("Unexpected count", lst.cnt == 9));

	list_free(&lst);
}
