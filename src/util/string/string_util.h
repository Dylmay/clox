#ifndef __CLOX_UTIL_STRING_H__
#define __CLOX_UTIL_STRING_H__

#include "util/common.h"
#include "util/map/hash_util.h"

struct string {
	size_t len;
	char c_str[];
};

size_t c_str_count_len(const char *c_str, size_t max_len);

struct string *string_new(const char *c_str, size_t len);
void string_free(struct string *str);

bool string_equals(const struct string *a, const struct string *b);

hash_t string_gen_hash(const struct string *str);

struct string *string_concat(const struct string *a, const struct string *b);

struct string *string_copy(const struct string *str);

struct string *string_c_append(const struct string *str, const char *chars,
			       size_t len);

static inline size_t string_get_len(const struct string *str)
{
	return str->len;
}

char string_char_at(const struct string *str, size_t offset);

static inline char *string_get_cstring(struct string *str)
{
	return str->c_str;
}

#endif // __CLOX_UTIL_STRING_H__