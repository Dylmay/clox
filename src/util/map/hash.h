#ifndef __UTIL_MAP_HASH_H__
#define __UTIL_MAP_HASH_H__

#include <stdint.h>

typedef uint32_t hash_t;
typedef hash_t (*hash_fn)(const void *a);

#endif // __UTIL_MAP_HASH_H__