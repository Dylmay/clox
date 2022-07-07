#ifndef __CLOX_CHUNK_FUNC_H__
#define __CLOX_CHUNK_FUNC_H__

#include "chunk/chunk.h"
#include "val/val.h"
#include "val/object.h"

struct chunk chunk_new();
void chunk_free(struct chunk *chunk);

size_t chunk_cur_ip(const struct chunk *chunk);
void chunk_reserve_code(struct chunk *chunk, uint32_t count);
void chunk_patch_code(struct chunk *chunk, size_t offset, const void *data,
		      uint32_t count);
size_t chunk_write_code(struct chunk *chunk, code_t code, int line);
size_t chunk_write_code_bulk(struct chunk *chunk, code_t code, int line,
			     const void *restrict data, size_t data_cnt);

size_t chunk_write_const(struct chunk *chunk, lox_val_t const_val);

code_t chunk_get_code(struct chunk *chunk, size_t offset);
int chunk_get_line(struct chunk *chunk, size_t offset);
lox_val_t chunk_get_const(struct chunk *chunk, size_t offset);

#endif //__CLOX_CHUNK_FUNC_H__