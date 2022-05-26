#ifndef __CLOX_COMPILER_H__
#define __CLOX_COMPILER_H__

#include "chunk/chunk.h"

bool compile(const char *src, chunk_t *chunk);

#endif // __CLOX_COMPILER_H__