#ifndef __CLOX_STATE_LOOKUP_VAR_H__
#define __CLOX_STATE_LOOKUP_VAR_H__

#include "util/common.h"

typedef uint8_t var_flags_t;
#define LOOKUP_VAR_INVALID (0)
#define LOOKUP_VAR_NO_FLAGS (0)

#define LOOKUP_VAR_IMMUTABLE (0 << 0)
#define LOOKUP_VAR_MUTABLE (1 << 0)

#define LOOKUP_VAR_NOT_DECLARED (0 << 1)
#define LOOKUP_VAR_DECLARED (1 << 1)

#define LOOKUP_VAR_NOT_DEFINED (0 << 2)
#define LOOKUP_VAR_DEFINED (1 << 2)

#define LOOKUP_VAR_LOCAL (0 << 3)
#define LOOKUP_VAR_GLOBAL (1 << 3)

//! @brief lookup variable
typedef struct {
	uint32_t idx;
	var_flags_t var_flags;
} lookup_var_t;

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