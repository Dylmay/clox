/**
 * @file hash.h
 * @author Dylan Mayor
 * @brief header file for table hash function and value
 *
 */
#ifndef __UTIL_MAP_HASH_H__
#define __UTIL_MAP_HASH_H__

#include <stdint.h>

//! @brief hash result
typedef uint32_t hash_t;

//! @brief hash function typedef
typedef hash_t (*hash_fn)(const void *a);

#endif // __UTIL_MAP_HASH_H__