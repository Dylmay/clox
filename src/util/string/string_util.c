#include "string_util.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>

#define SIZEOF_STRING(len) (sizeof(string_t) + (sizeof(char) * (len)))

static size_t __c_str_count_len(const char *c_str, size_t max_len)
{
	if (!c_str) {
		return 0;
	}

	size_t cnt = 0;
	const char *cur_char = c_str;

	while (*(cur_char++) != '\0' && cnt++ < max_len - 1)
		;

	return cnt;
}

string_t *string_new(const char *c_str, size_t len)
{
	assert(("string cannot have a null c_string", c_str));
	size_t max_len = __c_str_count_len(c_str, len);

	string_t *str_ptr = malloc(SIZEOF_STRING(max_len) + sizeof(char));

	strncpy(str_ptr->c_str, c_str, max_len);
	str_ptr->len = max_len;
	str_ptr->c_str[str_ptr->len] = '\0';

	return str_ptr;
}

void string_free(string_t *str)
{
	if (str) {
		free(str);
	}
}

bool string_equals(const string_t *a, const string_t *b)
{
	return a && b && a->len == b->len && strcmp(a->c_str, b->c_str) == 0;
}

string_t *string_concat(const string_t *a, const string_t *b)
{
	if (!a && !b) {
		return NULL;
	}

	if (!a) {
		return string_copy(b);
	}

	if (!b) {
		return string_copy(a);
	}

	size_t len = a->len + b->len;

	string_t *str_ptr = malloc(SIZEOF_STRING(len) + sizeof(char));
	str_ptr->len = len;
	strncpy(str_ptr->c_str, a->c_str, a->len);
	strncpy(str_ptr->c_str + a->len, b->c_str, b->len);
	str_ptr->c_str[str_ptr->len] = '\0';

	return str_ptr;
}

string_t *string_copy(const string_t *str)
{
	assert(("string cannot be null", str));

	return string_new(str->c_str, str->len);
}

string_t *string_c_append(string_t *str, const char *chars, size_t c_len)
{
	assert(("chars cannot be null", chars));

	if (!str) {
		return string_new(chars, c_len);
	}

	size_t c_max_len = __c_str_count_len(chars, c_len);
	size_t len = str->len + c_max_len;
	string_t *str_ptr = malloc(SIZEOF_STRING(len) + sizeof(char));
	str_ptr->len = len;
	strncpy(str_ptr->c_str, str->c_str, str->len);
	strncpy(str_ptr->c_str + str->len, chars, c_max_len);
	str_ptr->c_str[str_ptr->len] = '\0';

	string_free(str);
	return str_ptr;
}

hash_t string_gen_hash(const string_t *str)
{
	assert(("string cannot be null", str));

	return c_str_gen_hash(str->c_str, str->len);
}

char string_char_at(const string_t *str, size_t offset)
{
	assert(("string cannot be null", str));
	assert(("offset cannot be larger than len", offset < str->len));

	return str->c_str[offset];
}