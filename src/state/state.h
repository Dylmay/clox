#ifndef __CLOX_STATE_H__
#define __CLOX_STATE_H__

#include "interner/interner.h"
#include "lookup/lookup.h"

struct state {
	interner_t strings;
	lookup_t lookup;
};

static inline struct state state_new()
{
	return (struct state){
		.strings = intern_new(),
		.lookup = lookup_new(),
	};
}

static inline void state_free(struct state *state)
{
	intern_free(&state->strings);
	lookup_free(&state->lookup);
}

#endif // __CLOX_CHUNK_STATE_H__