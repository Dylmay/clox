#ifndef __CLOX_COMPILER_H__
#define __CLOX_COMPILER_H__

#include "chunk/chunk.h"
#include "chunk/state.h"

chunk_t *compile(const char *src, struct state *state);

#endif // __CLOX_COMPILER_H__