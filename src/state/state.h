/**
 * @file state.h
 * @author Dylan Mayor
 * @brief header file for lox run state struct
 *
 */
#ifndef __CLOX_STATE_H__
#define __CLOX_STATE_H__

#include "interner/interner.h"
#include "lookup/lookup.h"

/**
 * @brief lox state struct. To hold information between compile and vm runs.
 * Created using state_new(). Must be cleared after use by using state_free()
 *
 * @see state_new()
 * @see state_free()
 */
struct state {
	interner_t strings;
	lookup_t lookup;
};

//! @brief Construct a
static inline struct state state_new()
{
	return (struct state){
		.strings = intern_new(),
		.lookup = lookup_new(),
	};
}

/**
 * @brief frees the given state
 *
 * @param state the state to free
 */
static inline void state_free(struct state *state)
{
	intern_free(&state->strings);
	lookup_free(&state->lookup);
}

#endif // __CLOX_CHUNK_STATE_H__