#ifndef __CLOX_UTIL_HASH_UTIL_H__
#define __CLOX_UTIL_HASH_UTIL_H__

#include <stddef.h>

#include "hash.h"

hash_t str_gen_hash(const char *chars, size_t str_sz);

#endif // __CLOX_UTIL_HASH_UTIL_H__
