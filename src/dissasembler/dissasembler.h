#ifndef __CLOX_DISASSEMBLER_H__
#define __CLOX__DISASSEMBLER_H__

#include "chunk/chunk.h"

void disassem_chunk(chunk_t *chunk, const char *name);

size_t disassem_inst(chunk_t *chunk, size_t offset);

#endif // __CLOX_UTIL_DISASSEMBLER_H__