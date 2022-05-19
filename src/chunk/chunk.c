#include "chunk.h"
#include <assert.h>

static struct line_encode chunk_get_encoding(chunk_t *chunk, size_t idx);

struct line_encode line_encode_diff(int begin_pos, int end_pos)
{
	assert(("end_pos must be greater than begin_pos", end_pos > begin_pos));

	return (struct line_encode){ end_pos - begin_pos, 1 };
}

size_t chunk_write_code(chunk_t *chunk, code_t code, int line)
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

size_t chunk_write_code_bulk(chunk_t *chunk, code_t code, int line,
			     const void *__restrict__ data, size_t data_cnt)
{
	chunk_write_code(chunk, code, line);

	assert(("A code must be written before bulk insertion of data",
		chunk->lines.cnt));

	struct line_encode *encoding = (struct line_encode *)list_get(
		&chunk->lines, chunk->lines.cnt - 1);

	encoding->count += data_cnt;

	return list_write_bulk(&chunk->code, data, data_cnt);
}

size_t chunk_write_const(chunk_t *chunk, lox_val_t const_val)
{
	return list_write_to(&(chunk->consts), &const_val);
}

code_t chunk_get_code(chunk_t *chunk, size_t offset)
{
	return *((code_t *)list_get(&chunk->code, offset));
}

int chunk_get_line(chunk_t *chunk, size_t offset)
{
	int cur_idx = 0;
	int counted_code = 0;
	int line_cnt = 0;

	while (counted_code <= offset) {
		assert(("Offset is not valid", cur_idx < chunk->lines.cnt));

		struct line_encode encoding =
			chunk_get_encoding(chunk, cur_idx);

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
	list_free(&(chunk->code));
	list_free(&(chunk->consts));
	list_free(&(chunk->lines));
	chunk->prev_line = 0;
}

static struct line_encode chunk_get_encoding(chunk_t *chunk, size_t idx)
{
	return *((struct line_encode *)list_get(&chunk->lines, idx));
}