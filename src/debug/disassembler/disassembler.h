#ifndef __CLOX_DISASSEMBLER_H__
#define __CLOX_DISASSEMBLER_H__

#include "util/common.h"
#include "chunk/chunk.h"

void disassem_chunk(struct chunk *chunk, const char *name);

size_t disassem_inst(struct chunk *chunk, size_t offset);

#endif // __CLOX_UTIL_DISASSEMBLER_H__