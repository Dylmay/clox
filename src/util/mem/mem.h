#ifndef __CLOX_UTIL_MEM_H__
#define __CLOX_UTIL_MEM_H__

#include "util/common.h"

#define GROW_CAPACITY_AT_LEAST(min, cap)                                       \
	(min < GROW_CAPACITY(cap) ? GROW_CAPACITY(cap) : min)

#define GROW_CAPACITY(cap) ((cap) < 8 ? 8 : (cap)*2)

#define GROW_ARRAY(type, pointer, old_cnt, new_cnt)                            \
	(type *)reallocate(pointer, sizeof(type) * (old_cnt),                  \
			   sizeof(type) * (new_cnt))

#define FREE(type, pointer) (reallocate(pointer, sizeof(type), 0))

#define FREE_ARRAY(type, pointer, old_cnt)                                     \
	(reallocate(pointer, sizeof(type) * (old_cnt), 0))

#define ALLOCATE(type, cnt) ((type *)reallocate(NULL, 0, sizeof(type) * cnt))

void *reallocate(void *pointer, size_t old_sz, size_t new_sz);

#endif // __CLOX_UTIL_MEM_H__