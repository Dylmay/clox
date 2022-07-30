#include "sys.h"

#include <time.h>
#include <stdio.h>

#include "val/func/val_func.h"
#include "val/func/object_func.h"

static lox_val_t __clock_native(int arg_cnt, lox_val_t *args);
static lox_val_t __print_native(int arg_cnt, lox_val_t *args);

static struct native_import imports[] = {
	{
		.fn_name = "clock",
		.name_sz = sizeof("clock") - 1,
		.fn = &__clock_native,
	},
	{
		.fn_name = "print",
		.name_sz = sizeof("print") - 1,
		.fn = &__print_native,
	}
};

static struct import_list import_list = {
	.import_name = "sys",
	.import_cnt = sizeof(imports) / sizeof(imports[0]),
	.import_arr = imports,
};

struct import_list sys_get_import_list()
{
	return import_list;
}

static lox_val_t __clock_native(int arg_cnt, lox_val_t *args)
{
	return VAL_CREATE_NUMBER((double)clock() / CLOCKS_PER_SEC);
}

static lox_val_t __print_native(int arg_cnt, lox_val_t *args)
{
	if (!arg_cnt) {
		return VAL_CREATE_NIL;
	}

	val_print(args[0]);
	puts("");

	return VAL_CREATE_NIL;
}