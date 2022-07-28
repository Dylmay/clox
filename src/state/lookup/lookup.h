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
#include "val/val.h"
#include "lookup_var.h"

//! @brief global scope depth
#define LOOKUP_GLOBAL_DEPTH 1

//! @brief lookup table for variables
typedef struct {
	list_t scopes;
	uint32_t cur_idx;
	uint32_t glbl_idx;
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
 * @brief begins a new lookup scope
 *
 * @param lookup the lookup table to scope
 */
void lookup_begin_scope(lookup_t *lookup);

/**
 * @brief gets the current scope depth of the lookup table
 *
 * @param lookup the lookup table
 * @return size_t the current lookup depth
 */
static inline size_t lookup_cur_depth(const lookup_t *lookup)
{
	return list_size(&lookup->scopes);
}

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
lookup_var_t lookup_declare(lookup_t *lookup, size_t scope_depth,
			    const char *chars, size_t name_sz, bool mutable);

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
			   bool mutable);

/**
 * @brief looks through the current scope for the given index
 *
 * @param lookup the lookup table
 * @param idx the variable index
 * @return lox_str_t* the name of the variable
 */
lox_str_t *lookup_scope_find(const lookup_t *lookup, uint32_t idx);

/**
 * @brief looks through the lookup table (inner scope to outer scope), to match the first variable
 * which has the given name
 *
 * @param lookup the lookup table
 * @param name the variable name
 * @param name_sz name length
 * @return lookup_var_t the variable lookup info
 */
lookup_var_t lookup_find_name(lookup_t *lookup, const char *name,
			      size_t name_sz);

/**
 * @brief checks whether the given name (inner scope to outer scope), has been defined
 * Will match the first variable with the given name and return
 *
 * @param lookup the lookup table
 * @param name the variable name
 * @param name_sz name length
 * @return true variable has been defined
 * @return false variable has not been defined
 */
bool lookup_has_defined(lookup_t *lookup, const char *name, size_t name_sz);

/**
 * @brief gets the lookup scope at the given depth. Depth starts at one
 *
 * @param lookup the lookup table
 * @param depth The scope depth
 * @return hashmap_t* the lookup scope
 */
hashmap_t *lookup_scope_at_depth(lookup_t *lookup, size_t depth);

/**
 * @brief gets the current scope of the lookup table
 *
 * @param lookup the lookup table
 * @return hashmap_t* the current lookup scope
 */
static inline hashmap_t *lookup_cur_scope(lookup_t *lookup)
{
	return (hashmap_t *)list_peek(&lookup->scopes);
}

/**
 * @brief ends the current scope of the lookup table
 *
 * @param lookup the lookup table
 */
void lookup_end_scope(lookup_t *lookup);

/**
 * @brief returns whether the current scope has the given name
 *
 * @param lookup the lookup table
 * @param name the variable name
 * @param name_sz name size
 * @return true the lookup scope does have the name
 * @return false the lookup scope deos not have the name
 */
bool lookup_scope_has_name(lookup_t *lookup, const char *name, size_t name_sz);

#endif // __CLOX_STATE_LOOKUP_H__
