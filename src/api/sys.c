#include "sys.h"

#include <time.h>
#include <stdio.h>

#include "val/func/val_func.h"
#include "val/func/object_func.h"
#include "util/string/string_util.h"

static lox_val_t __clock_native(int arg_cnt, lox_val_t *args);
static lox_val_t __print_native(int arg_cnt, lox_val_t *args);
static lox_val_t __to_str(int arg_cnt, lox_val_t *args);
static lox_val_t __assert(int arg_cnt, lox_val_t *args);
static lox_val_t __input(int arg_cnt, lox_val_t *args);

#define CREATE_FUNC_DEF(func_name, func_def)                                   \
	{                                                                      \
		.fn_name = func_name, .name_sz = sizeof(func_name) - 1,        \
		.fn = &func_def,                                               \
	}

static struct native_import imports[] = {
	CREATE_FUNC_DEF("clock", __clock_native),
	CREATE_FUNC_DEF("print", __print_native),
	CREATE_FUNC_DEF("str", __to_str),
	CREATE_FUNC_DEF("assert", __assert),
	CREATE_FUNC_DEF("input", __input),
};

#undef CREATE_FUNC_DEF

#define FILE_READ_ERR_MSG "File Read Error"
#define ASSERT_ERR_MSG "Assert Error"

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

static lox_val_t __assert(int arg_cnt, lox_val_t *args)
{
	if (!arg_cnt || val_is_falsey(args[0])) {
		if (arg_cnt > 1) {
			lox_val_t err_msg = __to_str(1, &args[1]);
			err_msg.type = VAL_ERR;
			return err_msg;
		}

		return VAL_CREATE_ERR(object_str_new(
			ASSERT_ERR_MSG, sizeof(ASSERT_ERR_MSG) - 1));
	}

	return VAL_CREATE_NIL;
}

static lox_val_t __input(int arg_cnt, lox_val_t *args)
{
	// while no newline
	// write to buffer, BUF_SZ len
	// on read of BUF_SZ, recall
	char buf[INPUT_BUF_SZ];
	struct string *read_input = NULL;

	if (arg_cnt) {
		val_print(args[0]);
	}

	char last_char = '\0';
	do {
		char *ret_str = fgets(buf, sizeof(buf), stdin);

		if (!ret_str) {
			return VAL_CREATE_ERR(
				object_str_new(FILE_READ_ERR_MSG,
					       sizeof(FILE_READ_ERR_MSG) - 1));
		}

		read_input = string_c_append(read_input, buf, sizeof(buf));

		last_char =
			string_char_at(read_input, string_get_len(read_input));

	} while (last_char != '\n' && last_char != '\0');

	char *read_cstring = string_get_cstring(read_input);
	size_t read_len = string_get_len(read_input) - 1;

	lox_val_t str_obj =
		VAL_CREATE_OBJ(object_str_new(read_cstring, read_len));

	string_free(read_input);

	return str_obj;
}