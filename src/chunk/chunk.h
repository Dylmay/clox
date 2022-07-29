/**
 * @file chunk.h
 * @author Dylan Mayor
 * @brief header file holding the chunk struct definition. See chunk_func.h for chunk operations
 *
 * @see chunk_func.h
 *
 */
#ifndef __CLOX_CHUNK_H__
#define __CLOX_CHUNK_H__

#include "util/common.h"
#include "util/list/list.h"

//! @brief vm opcode typedef
typedef uint8_t code_t;

/**
 * @brief chunk struct definition. See chunk_func.h for usage
 * Created using chunk_new(). Must be freed after use by using chunk_free()
 *
 */
typedef struct __chunk {
	list_t code;
	list_t lines;
	list_t consts;
	uint32_t prev_line;
} chunk_t;

#endif // __CLOX_CHUNK_H__