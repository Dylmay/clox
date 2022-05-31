#ifndef __CLOX_CHUNK_H__
#define __CLOX_CHUNK_H__

#include "util/common.h"
#include "val/val.h"
#include "util/list/list.h"

typedef uint8_t code_t;

struct line_encode {
	uint32_t offset;
	uint32_t count;
};

typedef struct {
	list_t code;
	list_t lines;
	list_t consts;
	uint32_t prev_line;
} chunk_t;

#define chunk_new()                                                            \
	((chunk_t){ list_new(code_t), list_new(struct line_encode),            \
		    list_new(lox_val_t), 0 })

size_t chunk_write_code(chunk_t *chunk, code_t code, int line);
size_t chunk_write_code_bulk(chunk_t *chunk, code_t code, int line,
			     const void *__restrict__ data, size_t data_cnt);

size_t chunk_write_const(chunk_t *chunk, lox_val_t const_val);

void chunk_free(chunk_t *chunk);

code_t chunk_get_code(chunk_t *chunk, size_t offset);
int chunk_get_line(chunk_t *chunk, size_t offset);
lox_val_t chunk_get_const(chunk_t *chunk, size_t offset);

#endif // __CLOX_CHUNK_H__