#include "hash_util.h"

hash_t c_str_gen_hash(const char *chars, size_t str_sz)
{
	uint32_t hash = 216613626UL;

	for (size_t i = 0; i < str_sz; i++) {
		hash ^= (uint8_t)chars[i];
		hash *= 16777619;
	}

	return hash;
}

hash_t asciiz_gen_hash(const char *chars)
{
	uint32_t hash = 216613626UL;

	while (chars && *chars != '\0') {
		hash ^= (uint8_t)*chars;
		hash *= 16777619;

		chars++;
	}

	return hash;
}