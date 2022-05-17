#include "chunk/chunk.h"

size_t chunk_write_code(chunk_t *chunk, code_t code, int line)
{
	list_write_to(&chunk->lines, &line);
	return list_write_to(&chunk->code, &code);
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
	return *((int *)list_get(&chunk->lines, offset));
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
}