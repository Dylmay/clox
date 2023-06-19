#include "sys.h"
#include "util/dtoa.h"

#include <time.h>
#include <stdio.h>
#include <math.h>

#include "val/func/val_func.h"
#include "val/func/object_func.h"
#include "util/string/string_util.h"

/* Natives */
static lox_val_t __clock_native(int arg_cnt, lox_val_t *args);
static lox_val_t __print_native(int arg_cnt, lox_val_t *args);
static lox_val_t __to_str(int arg_cnt, lox_val_t *args);
static lox_val_t __to_num(int arg_cnt, lox_val_t *args);
static lox_val_t __assert(int arg_cnt, lox_val_t *args);
static lox_val_t __read(int arg_cnt, lox_val_t *args);
static lox_val_t __round_val(int arg_cnt, lox_val_t *args);
static lox_val_t __len(int arg_cnt, lox_val_t *args);
/* Helpers */
static lox_val_t __round_double(double value, int round_to);
static lox_val_t __str_to_num(const lox_str_t *str);

#define CREATE_FUNC_DEF(func_name, func_def)                                   \
	{                                                                      \
		.fn_name = func_name, .name_sz = sizeof(func_name) - 1,        \
		.fn = &func_def,                                               \
	}

static struct native_import imports[] = {
	CREATE_FUNC_DEF("clock", __clock_native),
	CREATE_FUNC_DEF("print", __print_native),
	CREATE_FUNC_DEF("str", __to_str),
	CREATE_FUNC_DEF("num", __to_num),
	CREATE_FUNC_DEF("assert", __assert),
	CREATE_FUNC_DEF("read", __read),
	CREATE_FUNC_DEF("round", __round_val),
	CREATE_FUNC_DEF("len", __len),
};

#undef CREATE_FUNC_DEF

#define NAN_ERR_MSG "Argument is not a number"

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
		return VAL_CREATE_OBJ(LITERAL_OBJECT_STRING(""));
	}
}

static lox_val_t __len(int arg_cnt, lox_val_t *args)
{
	if (arg_cnt != 1) {
		return VAL_CREATE_ERR(
			LITERAL_OBJECT_STRING("Invalid number of arguments"));
	}

	if (!OBJECT_IS_STRING(args[0])) {
		return VAL_CREATE_ERR(
			LITERAL_OBJECT_STRING("len() only accepts strings"));
	}

	lox_str_t *str = OBJECT_AS_STRING(args[0]);
	return VAL_CREATE_NUMBER(str->len);
}

static lox_val_t __assert(int arg_cnt, lox_val_t *args)
{
	if (!arg_cnt || val_is_falsey(args[0])) {
		if (arg_cnt > 1) {
			lox_val_t err_msg = __to_str(1, &args[1]);
			err_msg.type = VAL_ERR;
			return err_msg;
		}

		return VAL_CREATE_ERR(LITERAL_OBJECT_STRING("Assert Error"));
	}

	return VAL_CREATE_NIL;
}

static lox_val_t __read(int arg_cnt, lox_val_t *args)
{
	char buf[INPUT_BUF_SZ];
	struct string *read_input = NULL;

	if (arg_cnt) {
		val_print(args[0]);
	}

	char last_char = '\n';
	do {
		const char *ret_str = fgets(buf, sizeof(buf), stdin);

		if (!ret_str) {
			return VAL_CREATE_ERR(
				LITERAL_OBJECT_STRING("Read Error"));
		}

		read_input = string_c_append(read_input, buf, sizeof(buf));
		size_t char_pos = string_get_len(read_input) - 1;

		last_char = string_char_at(read_input, char_pos);

	} while (last_char != '\n');

	const char *read_cstring = string_get_cstring(read_input);
	size_t read_len = string_get_len(read_input) - 1;

	lox_val_t str_obj =
		VAL_CREATE_OBJ(object_str_new(read_cstring, read_len));

	string_free(read_input);

	return str_obj;
}

static lox_val_t __round_val(int arg_cnt, lox_val_t *args)
{
	if (arg_cnt) {
		lox_val_t val_to_round = args[0];
		int round_amt = 0;

		if (!VAL_IS_NUMBER(val_to_round)) {
			return VAL_CREATE_ERR(
				LITERAL_OBJECT_STRING(NAN_ERR_MSG));
		}

		if (arg_cnt > 1) {
			lox_val_t round_val = args[1];

			if (!VAL_IS_NUMBER(round_val)) {
				return VAL_CREATE_ERR(
					LITERAL_OBJECT_STRING(NAN_ERR_MSG));
			}

			double rounded_val = floor(VAL_AS_NUMBER(round_val));

			if (rounded_val < 0) {
				return VAL_CREATE_ERR(LITERAL_OBJECT_STRING(
					"round amount cannot be negative"));
			}

			round_amt = (int)rounded_val;
		}

		if (round_amt == 0) {
			return VAL_CREATE_NUMBER(
				round(VAL_AS_NUMBER(val_to_round)));
		}

		return __round_double(VAL_AS_NUMBER(val_to_round), round_amt);
	}

	return VAL_CREATE_ERR(
		LITERAL_OBJECT_STRING("round() requires a number"));
}

static lox_val_t __to_num(int arg_cnt, lox_val_t *args)
{
	if (arg_cnt) {
		lox_val_t arg = args[0];

		if (VAL_IS_BOOL(arg)) {
			return VAL_CREATE_NUMBER(VAL_AS_BOOL(arg) ? 1 : 0);
		}

		if (VAL_IS_NUMBER(arg)) {
			return arg;
		}

		if (OBJECT_IS_STRING(arg)) {
			return __str_to_num(OBJECT_AS_STRING(arg));
		}
	}

	return VAL_CREATE_ERR(
		LITERAL_OBJECT_STRING("invalid literal for num()"));
}

static lox_val_t __round_double(double value, int round_to)
{
#define SMALL_BUF_LEN 100
	char *buf, *buf_end;
	char small_buf[SMALL_BUF_LEN];
	size_t temp_buf_sz = SMALL_BUF_LEN;
	char *temp_buf = small_buf;
	int sign, decpt;

	buf = dtoa(value, 3, round_to, &decpt, &sign, &buf_end);

	if ((buf_end - buf) + 9 > SMALL_BUF_LEN) {
		temp_buf_sz = (buf_end - buf) + 8;
		temp_buf = reallocate(NULL, 0, temp_buf_sz);
	}

	snprintf(temp_buf, temp_buf_sz, "%s0%se%d", (sign ? "-" : ""), buf,
		 decpt - (int)(buf_end - buf));

	lox_val_t val = VAL_CREATE_NUMBER(strtod(temp_buf, NULL));

	if (temp_buf != small_buf) {
		reallocate(temp_buf, temp_buf_sz, 0);
	}
	freedtoa(buf);

	return val;
#undef SMALL_BUF_LEN
}

static lox_val_t __str_to_num(const lox_str_t *str)
{
	char *read_end = NULL;
	double val = strtod(str->chars, &read_end);
	size_t counted_chars = read_end - str->chars;

	// check to stop mixed strings returning partial numbers
	if (counted_chars != str->len && val != INFINITY) {
		val = NAN;
	}

	return VAL_CREATE_NUMBER(val);
}