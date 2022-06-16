#ifndef __CLOX_CHUNK_LOOKUP_H__
#define __CLOX_CHUNK_LOOKUP_H__

#include "util/common.h"
#include "util/map/map.h"
#include "val/val.h"

#define lookup_for_each(lookup, fn) (map_entry_for_each(lookup, fn))
#define lookup_remove(lookup, name) (map_remove(lookup, name))
#define lookup_size(lookup) (map_size(lookup))
#define lookup_free(lookup) (map_free(lookup))
#define lookup_is_empty(lookup) (map_size(lookup) == 0)

typedef hashmap_t lookup_t;

lookup_t lookup_new();

uint32_t lookup_by_name(lookup_t *lookup, const lox_str_t *name);
lox_str_t *lookup_find(const lookup_t *lookup, uint32_t idx);
bool lookup_has_name(const lookup_t *lookup, const lox_str_t *name);

lookup_t *lookup_root(lookup_t *lookup);
lookup_t *lookup_new_scope(lookup_t *lookup);
lookup_t *lookup_end_scope(lookup_t *lookup);

#endif // __CLOX_CHUNK_LOOKUP_H__
