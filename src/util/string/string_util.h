#ifndef __CLOX_UTIL_STRING_H__
#define __CLOX_UTIL_STRING_H__

#include "util/common.h"
#include "util/map/hash_util.h"

/**
 * @brief struct to hold information about an immutable fixed-length string
 * Note. All newly created strings must be de-allocated using string_free()
 * @see string_new()
 * @see string_free()
 */
struct string {
	size_t len;
	char c_str[];
};

/**
 * @brief Constructs a new immutable string handled in memory
 *
 * @param c_str the c string, size len or '\0' delimited
 * @param len the expected/maximum length of the c string. set to SIZE_MAX if the length is unknown and the entire string is required
 * @return struct string* newly allocated pointer to a handled string struct
 */
struct string *string_new(const char *c_str, size_t len);

/**
 * @brief Frees the passed string pointer
 *
 * @param str the string to free
 */
void string_free(struct string *str);

/**
 * @brief Checks whether string a equals string b
 *
 * @param a string a
 * @param b string b
 * @return true when both strings equal
 * @return false when strings are not equal
 */
bool string_equals(const struct string *a, const struct string *b);

/**
 * @brief generates a hash for the passed string
 *
 * @param str the string to hash
 * @return hash_t the generated hash value
 */
hash_t string_gen_hash(const struct string *str);

/**
 * @brief copies the passed string
 *
 * @param str the string to copy
 * @return struct string* the newly allocated copy
 */
struct string *string_copy(const struct string *str);

/**
 * @brief concats two strings.
 * If a and b are NULL, NULL is returned.
 * If either a or b is null, b or a is copied and returned
 *
 * @param a string a
 * @param b string b
 * @return struct string* the concatenated string
 */
struct string *string_concat(const struct string *a, const struct string *b);

/**
 * @brief appends a c string to the end of the given string
 *
 * @param str the string struct
 * @param chars the c string to append
 * @param len the length of the c string
 * @return struct string* the appended c string
 */
struct string *string_c_append(struct string *str, const char *chars,
			       size_t len);

/**
 * @brief gets the length of the passed string
 *
 * @param str the string struct
 * @return size_t the length of the string
 */
static inline size_t string_get_len(const struct string *str)
{
	return str->len;
}

/**
 * @brief gets the character at the given position. Index bounds is only checked in debug mode
 *
 * @param str the string to select from
 * @param offset the offset/index of the character
 * @return char the character at the given offset
 */
char string_char_at(const struct string *str, size_t offset);

/**
 * @brief gets the underlying c string of the passed struct
 *
 * @param str the string struct
 * @return char* the c string underlying the string struct
 */
static inline char *string_get_cstring(struct string *str)
{
	return str->c_str;
}

#endif // __CLOX_UTIL_STRING_H__