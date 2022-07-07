#ifndef __CLOX_COMPILER_H__
#define __CLOX_COMPILER_H__

#include "chunk/chunk.h"
#include "state/state.h"

struct chunk *compile(const char *src, struct state *state);

#endif // __CLOX_COMPILER_H__