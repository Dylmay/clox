/**
 * @file state.h
 * @author Dylan Mayor
 * @brief header file for lox run state struct
 *
 */
#ifndef __CLOX_STATE_H__
#define __CLOX_STATE_H__

#include "lookup/lookup.h"

/**
 * @brief lox state struct. To hold information between compile and vm runs.
 * Created using state_new(). Must be cleared after use by using state_free()
 *
 * @see state_new()
 * @see state_free()
 */
struct state {
	lookup_t globals;
};

//! @brief Construct a
static inline struct state state_new()
{
	return (struct state){
		.globals = lookup_new(0),
	};
}

/**
 * @brief frees the given state
 *
 * @param state the state to free
 */
static inline void state_free(struct state *state)
{
	lookup_free(&state->globals);
}

#endif // __CLOX_CHUNK_STATE_H__