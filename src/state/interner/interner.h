/**
 * @file interner.h
 * @author Dylan Mayor
 * @brief header file for lox string interning implementation
 *
 */
#ifndef __CLOX_CHUNK_INTERNER_H__
#define __CLOX_CHUNK_INTERNER_H__

#include "util/map/set.h"
#include "val/val.h"

/**
 * @brief Interner struct. Must be created using intern_new(), and freed using intern_free() after use
 *
 * @see intern_new()
 * @see intern_free()
 *
 */
typedef hashset_t interner_t;

/**
 * @brief the interner to create
 *
 * @return interner_t the new interner
 */
interner_t intern_new();

/**
 * @brief interns the passed string.
 * If it is already present, the pointer to the existing lox string is returned
 *
 * @param interner the interner
 * @param chars the string chars
 * @param len the string length
 * @return struct object_str* the interned string
 */
struct object_str *intern_string(interner_t *interner, const char *chars,
				 size_t len);

/**
 * @brief inserts the given object_str into the interner
 *
 * @param interner the interner
 * @param str the lox string to insert
 * @return true the insert succeeded
 * @return false the insert failed due to the item already being present
 */
static inline bool intern_insert(interner_t *interner, struct object_str *str)
{
	return hashset_insert(interner, str);
}

/**
 * @brief gets the lox string pointer for the given string and length.
 * If no string related to the given string and length is found, NULL is returned
 *
 * @param interner the interner
 * @param chars the string chars
 * @param len the string length
 * @return struct object_str* the lox string pointer or NULL
 */
struct object_str *intern_get_str(const interner_t *interner, const char *chars,
				  size_t len);

/**
 * @brief frees the passed interner
 *
 * @param interner the interner to free
 */
void intern_free(interner_t *interner);

#endif // __CLOX_CHUNK_INTERNER_H__