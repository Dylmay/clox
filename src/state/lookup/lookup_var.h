/**
 * @file lookup_var.h
 * @author Dylan Mayor
 * @brief header file for lookup variable definitions
 *
 */
#ifndef __CLOX_STATE_LOOKUP_VAR_H__
#define __CLOX_STATE_LOOKUP_VAR_H__

#include "util/common.h"

#pragma region lookup_var_flags
//! @brief flags related to lox variable definitions
typedef uint8_t var_flags_t;
/**
 * @brief constant to test whether all flags are off. I.E. the var is invalid
 *
 * @see var_flags_t
 * @see lookup_var_is_valid()
 *
 */
#define LOOKUP_VAR_INVALID_FLAG (0)
/**
 * @brief constant to set var_flags_t to zero
 *
 * @see var_flags_t
 *
 */
#define LOOKUP_VAR_NO_FLAGS (0)

/**
 * @brief flag to indicate lookup variable is not mutable. Default value
 *
 * @see var_flags_t
 * @see lookup_var_is_mutable()
 *
 */
#define LOOKUP_VAR_IMMUTABLE (0 << 0)
/**
 * @brief flag to indicate lookup variable is mutable
 *
 * @see var_flags_t
 * @see lookup_var_is_mutable()
 *
 */
#define LOOKUP_VAR_MUTABLE (1 << 0)

/**
 * @brief flag to indicate lookup variable is not declared. Default value
 *
 * @see var_flags_t
 * @see lookup_var_is_declared()
 */
#define LOOKUP_VAR_NOT_DECLARED (0 << 1)
/**
 * @brief flag to indicate lookup variable is declared.
 * lookup variables will always have at least this flag set if valid
 *
 * @see var_flags_t
 * @see lookup_var_is_declared()
 *
 */
#define LOOKUP_VAR_DECLARED (1 << 1)

/**
 * @brief flag to indicate lookup variable is not defined. Default value
 *
 * @see var_flags_t
 * @see lookup_var_is_defined()
 *
 */
#define LOOKUP_VAR_NOT_DEFINED (0 << 2)
/**
 * @brief flag to indicate lookup variable is defined
 *
 * @see var_flags_t
 * @see lookup_var_is_defined()
 *
 */
#define LOOKUP_VAR_DEFINED (1 << 2)

/**
 * @brief flag to indicate lookup variable is in local scope. Default value
 *
 * @see var_flags_t
 * @see lookup_var_is_global()
 *
 */
#define LOOKUP_VAR_LOCAL (0 << 3)
/**
 * @brief flag to indicate lookup variable is in global scope
 *
 * @see var_flags_t
 * @see lookup_var_is_global()
 *
 */
#define LOOKUP_VAR_GLOBAL (1 << 3)
#pragma endregion

//! @brief lookup variable
typedef struct __lookup_var {
	uint32_t idx;
	var_flags_t var_flags;
} lookup_var_t;

#define LOOKUP_VAR_TYPE_INVALID                                                \
	(lookup_var_t)                                                         \
	{                                                                      \
		.idx = 0, .var_flags = LOOKUP_VAR_INVALID_FLAG,                \
	}

static inline bool lookup_var_is_valid(lookup_var_t var)
{
	return var.var_flags & ~LOOKUP_VAR_INVALID_FLAG;
}

/**
 * @brief returns whether the passed variable is mutable
 *
 * @param var the variable to check
 * @return true variable is mutable
 * @return false variable is not mutable
 */
static inline bool lookup_var_is_mutable(lookup_var_t var)
{
	return var.var_flags & LOOKUP_VAR_MUTABLE;
}

/**
 * @brief returns whether the passed vairable is declared
 *
 * @param var the variable to check
 * @return true variable is declared
 * @return false variable is not declared
 */
static inline bool lookup_var_is_declared(lookup_var_t var)
{
	return var.var_flags & LOOKUP_VAR_DECLARED;
}

/**
 * @brief returns whether the passed variable is defined
 *
 * @param var the variable to check
 * @return true variable is defined
 * @return false variable is not defined
 */
static inline bool lookup_var_is_defined(lookup_var_t var)
{
	return var.var_flags & LOOKUP_VAR_DEFINED;
}

/**
 * @brief returns whether the variable is global
 *
 * @param var the variable to check
 * @return true variable is global
 * @return false variable is local
 */
static inline bool lookup_var_is_global(lookup_var_t var)
{
	return var.var_flags & LOOKUP_VAR_GLOBAL;
}
#endif // __CLOX_STATE_LOOKUP_VAR_H__