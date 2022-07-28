/**
 * @file hash_util.h
 * @author Dylan Mayor
 * @brief header file for hash gen utility functions
 *
 */
#ifndef __CLOX_UTIL_HASH_UTIL_H__
#define __CLOX_UTIL_HASH_UTIL_H__

#include <stddef.h>

#include "hash.h"

/**
 * @brief generates a hash for the passed c string. str_sz defines the max length of the string
 *
 * @param chars c string chars
 * @param str_sz expected size of the c string
 * @return hash_t the string hash
 */
hash_t c_str_gen_hash(const char *chars, size_t str_sz);

/**
 * @brief generates a hash for an asciiz string
 *
 * @param chars asciiz string
 * @return hash_t the string hash
 */
hash_t asciiz_gen_hash(const char *chars);

#endif // __CLOX_UTIL_HASH_UTIL_H__
