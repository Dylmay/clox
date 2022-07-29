#include "chunk.h"
#include "val/func/object_func.h"

#include <assert.h>
#include <string.h>

struct line_encode {
	uint32_t offset;
	uint32_t count;
};

static struct line_encode __chunk_get_line_encode(chunk_t *chunk, size_t idx);
static struct line_encode __line_encode_diff(uint32_t begin_pos,
					     uint32_t end_pos);

chunk_t chunk_new()
{
	return (chunk_t){
		.code = list_of_type(code_t),
		.consts = list_of_type(lox_val_t),
		.lines = list_of_type(struct line_encode),
		.prev_line = 0,
	};
}

size_t chunk_cur_instr(const chunk_t *chunk)
{
	return list_size(&chunk->code);
}

void chunk_reserve_code(chunk_t *chunk, uint32_t count)
{
	list_push_bulk(&chunk->code, NULL, count);

	struct line_encode *encoding = list_peek(&chunk->lines);
	encoding->count += count;
}

void chunk_patch_code(chunk_t *chunk, size_t offset, const void *data,
		      uint32_t count)
{
	void *dest = list_get(&chunk->code, offset);
	memcpy(dest, data, count);
}

size_t chunk_write_code(chunk_t *chunk, code_t code, uint32_t line)
{
	assert(("passed line can not be zero", line != 0));

	if (chunk->prev_line != line) {
		struct line_encode encoding =
			__line_encode_diff(chunk->prev_line, line);

		list_push(&chunk->lines, &encoding);

		chunk->prev_line = line;
	} else {
		struct line_encode *encoding = list_peek(&chunk->lines);
		encoding->count++;
	}

	return list_push(&chunk->code, &code);
}

size_t chunk_write_code_extended(chunk_t *chunk, code_t code, uint32_t line,
				 const void *restrict data, size_t data_cnt)
{
	chunk_write_code(chunk, code, line);

	assert(("A code must be written before bulk insertion of data",
		chunk->lines.cnt));

	struct line_encode *encoding = list_peek(&chunk->lines);
	encoding->count += data_cnt;

	return list_push_bulk(&chunk->code, data, data_cnt);
}

size_t chunk_write_const(chunk_t *chunk, lox_val_t const_val)
{
	return list_push(&(chunk->consts), &const_val);
}

code_t chunk_get_code(chunk_t *chunk, size_t offset)
{
	return *((code_t *)list_get(&chunk->code, offset));
}

size_t chunk_get_line(chunk_t *chunk, size_t offset)
{
	size_t cur_idx = 0;
	size_t counted_code = 0;
	size_t line_cnt = 0;

	while (counted_code <= offset) {
		assert(("Offset is not valid", cur_idx < chunk->lines.cnt));

		struct line_encode encoding =
			__chunk_get_line_encode(chunk, cur_idx);

		line_cnt += encoding.offset;
		counted_code += encoding.count;

		cur_idx++;
	}

	return line_cnt;
}

lox_val_t chunk_get_const(chunk_t *chunk, size_t offset)
{
	return *((lox_val_t *)list_get(&chunk->consts, offset));
}

void chunk_free(chunk_t *chunk)
{
	list_free(&chunk->code);
	list_free(&chunk->lines);
	list_free(&chunk->consts);
	chunk->prev_line = 0;
}
static struct line_encode __chunk_get_line_encode(chunk_t *chunk, size_t idx)
{
	return *((struct line_encode *)list_get(&chunk->lines, idx));
}

static struct line_encode __line_encode_diff(uint32_t begin_pos,
					     uint32_t end_pos)
{
	assert(("end_pos must be greater than begin_pos", end_pos > begin_pos));

	return (struct line_encode){
		.offset = end_pos - begin_pos,
		.count = 1,
	};
}