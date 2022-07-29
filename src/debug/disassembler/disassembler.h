#ifndef __CLOX_DISASSEMBLER_H__
#define __CLOX_DISASSEMBLER_H__

#include "util/common.h"
#include "chunk/chunk.h"

/**
 * @brief entirely disassembles the passed chunk
 *
 * @param chunk the chunk to disassemble
 * @param name the name of the function/chunk
 */
void disassem_chunk(struct chunk *chunk, const char *name);

/**
 * @brief disassembles the instruction at the chosen offset, within the chunk
 *
 * @param chunk the chunk to disassemble
 * @param ip the ip of the instruction to disassemble
 * @return size_t the next instruction offset
 */
size_t disassem_inst(struct chunk *chunk, size_t ip);

#endif // __CLOX_UTIL_DISASSEMBLER_H__