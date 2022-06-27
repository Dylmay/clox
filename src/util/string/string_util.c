#include "string_util.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>

#define SIZEOF_STRING(len) (sizeof(struct string) + (sizeof(char) * (len)))

size_t c_str_count_len(const char *c_str, size_t max_len)
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

struct string *string_new(const char *c_str, size_t len)
{
	assert(("string cannot have a null c_string", c_str));
	size_t max_len = c_str_count_len(c_str, len);

	struct string *str_ptr = malloc(SIZEOF_STRING(max_len) + sizeof(char));

	strncpy(str_ptr->c_str, c_str, max_len);
	str_ptr->len = max_len;
	str_ptr->c_str[str_ptr->len] = '\0';

	return str_ptr;
}

void string_free(struct string *str)
{
	if (str) {
		free(str);
	}
}

bool string_equals(const struct string *a, const struct string *b)
{
	return a && b && a->len == b->len && strcmp(a->c_str, b->c_str) == 0;
}

struct string *string_concat(const struct string *a, const struct string *b)
{
	if (!a && !b) {
		return NULL;
	}

	if (!a) {
		return string_copy(a);
	}

	if (!b) {
		return string_copy(b);
	}

	size_t len = a->len + b->len;

	struct string *str_ptr = malloc(SIZEOF_STRING(len) + sizeof(char));
	str_ptr->len = len;
	strncpy(str_ptr->c_str, a->c_str, a->len);
	strncpy(str_ptr->c_str + a->len, b->c_str, b->len);
	str_ptr->c_str[str_ptr->len] = '\0';

	return str_ptr;
}

struct string *string_copy(const struct string *str)
{
	assert(("string cannot be null", str));

	return string_new(str->c_str, str->len);
}

struct string *string_c_append(const struct string *str, const char *chars,
			       size_t c_len)
{
	assert(("chars cannot be null", chars));

	if (!str) {
		return string_new(chars, c_len);
	}

	size_t c_max_len = c_str_count_len(chars, c_len);
	size_t len = str->len + c_max_len;
	struct string *str_ptr = malloc(SIZEOF_STRING(len) + sizeof(char));
	str_ptr->len = len;
	strncpy(str_ptr->c_str, str->c_str, str->len);
	strncpy(str_ptr->c_str + str->len, chars, c_max_len);
	str_ptr->c_str[str_ptr->len] = '\0';

	return str_ptr;
}

hash_t string_gen_hash(const struct string *str)
{
	assert(("string cannot be null", str));

	return c_str_gen_hash(str->c_str, str->len);
}

char string_char_at(const struct string *str, size_t offset)
{
	assert(("string cannot be null", str));
	assert(("offset cannot be larger than len", offset >= str->len));

	return str->c_str[offset];
}