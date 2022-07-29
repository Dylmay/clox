/**
 * @file chunk_func.h
 * @author Dylan Mayor
 * @brief header file for chunk_t functions
 *
 */
#ifndef __CLOX_CHUNK_FUNC_H__
#define __CLOX_CHUNK_FUNC_H__

#include "chunk/chunk.h"
#include "val/val.h"

/**
 * @brief creates a new code chunk
 *
 * @return chunk_t the new code chunk
 */
chunk_t chunk_new();

/**
 * @brief frees the passed chunk
 *
 * @param chunk the chunk to free
 */
void chunk_free(chunk_t *chunk);

/**
 * @brief gets the current instruction pointer of the chunk
 *
 * @param chunk the chunk
 * @return size_t the current chunk ip
 */
size_t chunk_cur_ip(const chunk_t *chunk);

/**
 * @brief reserves a chunk of code so it can be modified or set later
 *
 * @param chunk the chunk
 * @param count the number of opcodes to reserve
 */
void chunk_reserve_code(chunk_t *chunk, uint32_t count);

/**
 * @brief patches a previous op code with the given data and count
 *
 * @param chunk the chunk to patch
 * @param ip the instruction pointer of the instruction
 * @param data the data to overwrite the instruction with
 * @param data_sz the number of code_t values which data holds
 */
void chunk_patch_code(chunk_t *chunk, size_t ip, const void *data,
		      uint32_t data_sz);

/**
 * @brief writes a code item in to the code stack
 *
 * @param chunk the chunk to write to
 * @param code the code to write
 * @param line the line in which the code was defined
 * @return size_t the current ip
 */
size_t chunk_write_code(chunk_t *chunk, code_t code, uint32_t line);

/**
 * @brief writes a code item to the code stack and then writes the extended data after the code
 *
 * @param chunk the chunk to write to
 * @param code the code to write
 * @param line the line in which the code was defined
 * @param data the extended code data
 * @param data_cnt the extended code data count, in number of code_t items
 * @return size_t the current ip
 */
size_t chunk_write_code_extended(chunk_t *chunk, code_t code, uint32_t line,
				 const void *restrict data, size_t data_cnt);

/**
 * @brief writes a const value to the chunk
 *
 * @param chunk the chunk to write to
 * @param const_val the constant to write
 * @return size_t
 */
size_t chunk_write_const(chunk_t *chunk, lox_val_t const_val);

/**
 * @brief gets the code at the given ip
 *
 * @param chunk the chunk to read
 * @param ip the ip of the code
 * @return code_t the code at the given ip
 */
code_t chunk_get_code(chunk_t *chunk, size_t ip);

/**
 * @brief gets the line the given instruction was defined at
 *
 * @param chunk the chunk to read
 * @param offset the instruction offset
 * @return size_t the instruction line
 */
size_t chunk_get_line(chunk_t *chunk, size_t offset);

/**
 * @brief gets the constant at the given offset
 *
 * @param chunk the chunk to read
 * @param offset th instruction offset
 * @return lox_val_t the instruction constant
 */
lox_val_t chunk_get_const(chunk_t *chunk, size_t offset);

#endif //__CLOX_CHUNK_FUNC_H__