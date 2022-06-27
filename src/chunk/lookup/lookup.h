#ifndef __CLOX_CHUNK_LOOKUP_H__
#define __CLOX_CHUNK_LOOKUP_H__

#include "util/common.h"
#include "util/map/map.h"
#include "val/val.h"

typedef uint8_t var_flags_t;
#define LOOKUP_VAR_NO_FLAGS (0)
#define LOOKUP_VAR_MUTABLE (1 << 1)
#define LOOKUP_VAR_INITIALIZED (1 << 2)
#define LOOKUP_VAR_INVALID (1 << 4)

typedef struct {
	uint32_t idx;
	var_flags_t var_flags;
} lookup_var_t;

typedef struct {
	list_t scopes;
	uint32_t cur_idx;
} lookup_t;

lookup_t lookup_new();
void lookup_free(lookup_t *lookup);

void lookup_begin_scope(lookup_t *lookup);
size_t lookup_cur_depth(const lookup_t *lookup);
lookup_var_t lookup_scope_define(lookup_t *lookup, const char *name, size_t len,
				 var_flags_t flags);
lox_str_t *lookup_scope_find(const lookup_t *lookup, uint32_t idx);
lookup_var_t lookup_find_name(lookup_t *lookup, const char *name, size_t len);
bool lookup_scope_has_name(lookup_t *lookup, const char *name, size_t len);
lox_str_t *lookup_scope_find(const lookup_t *lookup, uint32_t idx);
hashmap_t *lookup_scope_at_depth(lookup_t *lookup, size_t idx);
hashmap_t *lookup_cur_scope(lookup_t *lookup);
void lookup_end_scope(lookup_t *lookup);

static inline bool lookup_var_is_mutable(lookup_var_t var)
{
	return var.var_flags & LOOKUP_VAR_MUTABLE;
}

static inline bool lookup_var_is_initialized(lookup_var_t var)
{
	return var.var_flags & LOOKUP_VAR_INITIALIZED;
}

static inline bool lookup_var_is_invalid(lookup_var_t var)
{
	return var.var_flags & LOOKUP_VAR_INVALID;
}

#endif // __CLOX_CHUNK_LOOKUP_H__
