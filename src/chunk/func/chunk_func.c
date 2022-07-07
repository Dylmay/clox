#include "chunk.h"
#include "val/func/object_func.h"

#include <assert.h>
#include <string.h>

struct line_encode {
	uint32_t offset;
	uint32_t count;
};

static struct line_encode __chunk_get_line_encode(struct chunk *chunk,
						  size_t idx);

struct chunk chunk_new()
{
	return (struct chunk){
		.code = list_of_type(code_t),
		.consts = list_of_type(lox_val_t),
		.lines = list_of_type(struct line_encode),
		.prev_line = 0,
	};
}

struct line_encode line_encode_diff(int begin_pos, int end_pos)
{
	assert(("end_pos must be greater than begin_pos", end_pos > begin_pos));

	return (struct line_encode){ end_pos - begin_pos, 1 };
}

size_t chunk_cur_ip(const struct chunk *chunk)
{
	return list_size(&chunk->code);
}

void chunk_reserve_code(struct chunk *chunk, uint32_t count)
{
	list_adjust_cnt(&chunk->code, count);

	((struct line_encode *)list_get(&chunk->lines, chunk->lines.cnt - 1))
		->count += count;
}

void chunk_patch_code(struct chunk *chunk, size_t offset, const void *data,
		      uint32_t count)
{
	void *dest = list_get(&chunk->code, offset);
	memcpy(dest, data, count);
}

size_t chunk_write_code(struct chunk *chunk, code_t code, int line)
{
	assert(("passed line can not be zero", line != 0));

	if (chunk->prev_line != line) {
		struct line_encode encoding =
			line_encode_diff(chunk->prev_line, line);

		list_write_to(&chunk->lines, &encoding);

		chunk->prev_line = line;
	} else {
		((struct line_encode *)list_get(&chunk->lines,
						chunk->lines.cnt - 1))
			->count++;
	}

	return list_write_to(&chunk->code, &code);
}

size_t chunk_write_code_bulk(struct chunk *chunk, code_t code, int line,
			     const void *restrict data, size_t data_cnt)
{
	chunk_write_code(chunk, code, line);

	assert(("A code must be written before bulk insertion of data",
		chunk->lines.cnt));

	struct line_encode *encoding = (struct line_encode *)list_get(
		&chunk->lines, chunk->lines.cnt - 1);

	encoding->count += data_cnt;

	return list_write_bulk(&chunk->code, data, data_cnt);
}

size_t chunk_write_const(struct chunk *chunk, lox_val_t const_val)
{
	return list_write_to(&(chunk->consts), &const_val);
}

code_t chunk_get_code(struct chunk *chunk, size_t offset)
{
	return *((code_t *)list_get(&chunk->code, offset));
}

int chunk_get_line(struct chunk *chunk, size_t offset)
{
	int cur_idx = 0;
	int counted_code = 0;
	int line_cnt = 0;

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

lox_val_t chunk_get_const(struct chunk *chunk, size_t offset)
{
	return *((lox_val_t *)list_get(&chunk->consts, offset));
}

void chunk_free(struct chunk *chunk)
{
	list_free(&chunk->code);
	list_free(&chunk->lines);
	list_free(&chunk->consts);
	chunk->prev_line = 0;
}
static struct line_encode __chunk_get_line_encode(struct chunk *chunk,
						  size_t idx)
{
	return *((struct line_encode *)list_get(&chunk->lines, idx));
}
