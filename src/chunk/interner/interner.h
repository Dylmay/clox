#ifndef __CLOX_CHUNK_INTERNER_H__
#define __CLOX_CHUNK_INTERNER_H__

#include "util/map/set.h"

typedef hashset_t interner_t;

interner_t intern_new();

struct object_str *intern_string(interner_t *interner, const char *chars,
				 size_t len);

void intern_free(interner_t *interner);

#endif // __CLOX_CHUNK_INTERNER_H__