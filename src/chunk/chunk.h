#ifndef __CLOX_CHUNK_H__
#define __CLOX_CHUNK_H__

#include "util/common.h"
#include "val/val.h"
#include "val/object.h"
#include "util/list/list.h"
#include "interner/interner.h"

typedef uint8_t code_t;

struct line_encode {
	uint32_t offset;
	uint32_t count;
};

struct state {
	interner_t strings;
};

typedef struct {
	list_t code;
	list_t lines;
	list_t consts;
	struct state vals;
	uint32_t prev_line;
} chunk_t;

static inline struct state state_new()
{
	return (struct state){
    .strings = intern_new(),
	};
}

static inline void state_free(struct state *state)
{
	intern_free(&state->strings);
}

chunk_t chunk_new();

chunk_t chunk_using_state(struct state state);
bool chunk_has_state(chunk_t *chunk);

size_t chunk_write_code(chunk_t *chunk, code_t code, int line);
size_t chunk_write_code_bulk(chunk_t *chunk, code_t code, int line,
			     const void *restrict data, size_t data_cnt);

size_t chunk_write_const(chunk_t *chunk, lox_val_t const_val);

void chunk_free(chunk_t *chunk, bool free_state);

struct object_str *chunk_intern_string(chunk_t *chunk, const char *chars,
				       size_t len);

code_t chunk_get_code(chunk_t *chunk, size_t offset);
int chunk_get_line(chunk_t *chunk, size_t offset);
lox_val_t chunk_get_const(chunk_t *chunk, size_t offset);

#endif // __CLOX_CHUNK_H__