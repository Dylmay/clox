#ifndef __CLOX_UTIL_MEM_H__
#define __CLOX_UTIL_MEM_H__

/**
 * @file mem.h
 * @author Dylan Mayor
 * @brief header file for general memory management
 *
 */

#include "util/common.h"

/**
 * @brief the minimum capacity for an array
 *
 */
#define MIN_CAP 8

/**
 * @brief returns a capacity value which is at least min. min must be at least MIN_CAP
 *
 * @see MIN_CAP
 *
 * @param min the minimum capacity
 * @param cap the capacity to grow
 *
 * @return the new capacity
 */
#define GROW_CAPACITY_AT_LEAST(min, cap)                                       \
	(min < GROW_CAPACITY(cap) ? GROW_CAPACITY(cap) : min)

/**
 * @brief grows the given capacity by at least MIN_CAP or double of its current value
 *
 * @see MIN_CAP
 *
 * @param cap the capacity to grow
 *
 * @return the new capacity
 *
 */
#define GROW_CAPACITY(cap) ((cap) < MIN_CAP ? MIN_CAP : (cap)*2)

/**
 * @brief frees the given pointer
 *
 * @param type the base type of the pointer
 * @param pointer the pointer to free
 *
 *
 */
#define FREE(type, pointer) (reallocate(pointer, sizeof(type), 0))

/**
 * @brief reallocates the passed pointer from the old size to the new size
 *
 * @param pointer the pointer to reallocate. NULL if a new allocation is needed
 * @param old_sz the old size
 * @param new_sz the new size
 * @return void* the newly allocated pointer
 */
void *reallocate(void *pointer, size_t old_sz, size_t new_sz);

#endif // __CLOX_UTIL_MEM_H__