#ifndef __CLOX_CHUNK_H__
#define __CLOX_CHUNK_H__

#include "util/common.h"
#include "util/list/list.h"

typedef uint8_t code_t;

struct chunk {
	list_t code;
	list_t lines;
	list_t consts;
	uint32_t prev_line;
};

#endif // __CLOX_CHUNK_H__