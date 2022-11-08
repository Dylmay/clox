#include "sys.h"

#include <time.h>
#include <stdio.h>

#include "val/func/val_func.h"
#include "val/func/object_func.h"

static lox_val_t __clock_native(int arg_cnt, lox_val_t *args);
static lox_val_t __print_native(int arg_cnt, lox_val_t *args);
static lox_val_t __to_str(int arg_cnt, lox_val_t *args);

#define CREATE_FUNC_DEF(func_name, func_def)                                   \
	{                                                                      \
		.fn_name = func_name, .name_sz = sizeof(func_name) - 1,        \
		.fn = &func_def,                                               \
	}

static struct native_import imports[] = {
	CREATE_FUNC_DEF("clock", __clock_native),
	CREATE_FUNC_DEF("print", __print_native),
	CREATE_FUNC_DEF("str", __to_str),
};

#undef CREATE_FUNC_DEF

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
	if (arg_cnt) {
		val_print(args[0]);
	}

	puts("");

	return VAL_CREATE_NIL;
}

static lox_val_t __to_str(int arg_cnt, lox_val_t *args)
{
	if (arg_cnt) {
		return val_to_string(args[0]);
	} else {
		return VAL_CREATE_OBJ(object_str_new("", 0));
	}
}