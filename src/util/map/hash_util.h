#ifndef __CLOX_UTIL_HASH_UTIL_H__
#define __CLOX_UTIL_HASH_UTIL_H__

#include <stddef.h>

#include "hash.h"

hash_t c_str_gen_hash(const char *chars, size_t str_sz);
hash_t asciiz_gen_hash(const char *chars);

#endif // __CLOX_UTIL_HASH_UTIL_H__
