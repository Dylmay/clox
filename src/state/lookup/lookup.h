/**
 * @file lookup.h
 * @author Dylan Mayor
 * @brief header file for lookup table implementation for lox variables
 *
 */
#ifndef __CLOX_STATE_LOOKUP_H__
#define __CLOX_STATE_LOOKUP_H__

#include "util/common.h"
#include "util/map/map.h"
#include "lookup_var.h"

typedef struct __lookup {
	hashmap_t table;
} lookup_t;

/**
 * @brief creates a new lookup table
 *
 * @return lookup_t the new lookup table
 */
lookup_t lookup_new();

/**
 * @brief frees all related memory held by the lookup table
 *
 * @param lookup the lookup table to free
 */
void lookup_free(lookup_t *lookup);

/**
 * @brief declares a variable within the lookup at a given scope.
 * Declared variables must also be defined before being able to be used
 *
 * @param lookup the lookup table
 * @param scope_depth the depth to declare
 * @param chars the variable name
 * @param name_sz the name length
 * @param mutable if the variable can be modified
 * @return lookup_var_t the created lookup variable
 */
lookup_var_t lookup_declare(lookup_t *lookup, const char *chars, size_t name_sz,
			    uint32_t idx, var_flags_t flags);

/**
 * @brief declares and/or defines a variable within the lookup at the current scope.
 *
 * @param lookup the lookup table
 * @param name the variable name
 * @param name_sz the name length
 * @param mutable if the variable can be modified
 * @return lookup_var_t the created lookup variable
 */
lookup_var_t lookup_define(lookup_t *lookup, const char *name, size_t name_sz,
			   uint32_t idx, var_flags_t flags);

/**
 * @brief looks through the current scope for the given name
 *
 * @param lookup the lookup table
 * @param idx the variable index
 * @return lox_str_t* the name of the variable
 */
lookup_var_t lookup_find_name(const lookup_t *lookup, const char *name,
			      size_t name_sz);

/**
 * @brief returns whether the current scope has the given name
 *
 * @param lookup the lookup table
 * @param name the variable name
 * @param name_sz name size
 * @return true the lookup scope does have the name
 * @return false the lookup scope deos not have the name
 */
bool lookup_has_name(const lookup_t *lookup, const char *name, size_t name_sz);

static inline uint32_t lookup_get_size(const lookup_t *lookup)
{
	return (uint32_t)map_size(&lookup->table);
}

bool lookup_remove(lookup_t *lookup, uint32_t idx);

#endif // __CLOX_STATE_LOOKUP_H__
