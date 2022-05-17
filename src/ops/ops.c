#include "ops.h"
#include <assert.h>

void op_write_const(chunk_t *chunk, lox_val_t const_val, int line)
{
	size_t const_offset = chunk_write_const(chunk, const_val);

	if (const_offset <= __UINT8_MAX__) {
		chunk_write_code(chunk, OP_CONSTANT, line);
		chunk_write_code(chunk, const_offset, line);
	} else {
		assert(("Max number of consts is 256 * 24",
			const_offset < 256 * 24));

		chunk_write_code_bulk(chunk, OP_CONSTANT_LONG, line,
				      &const_offset, 3);
	}
}

void op_write_return(chunk_t *chunk, int line)
{
	chunk_write_code(chunk, OP_RETURN, line);
}