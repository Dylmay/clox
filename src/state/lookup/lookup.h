#ifndef __CLOX_CHUNK_LOOKUP_H__
#define __CLOX_CHUNK_LOOKUP_H__

#include "util/common.h"
#include "util/map/map.h"
#include "val/val.h"

typedef uint8_t var_flags_t;
#define LOOKUP_VAR_NO_FLAGS (0)
#define LOOKUP_VAR_MUTABLE (1 << 1)
#define LOOKUP_VAR_NOT_DEFINED (1 << 2)
#define LOOKUP_VAR_NOT_ASSIGNED (1 << 3)
#define LOOKUP_VAR_GLOBAL (1 << 4)

typedef struct {
	uint32_t idx;
	var_flags_t var_flags;
} lookup_var_t;

typedef struct {
	list_t scopes;
	uint32_t cur_idx;
	uint32_t glbl_idx;
} lookup_t;

lookup_t lookup_new();
void lookup_free(lookup_t *lookup);

void lookup_begin_scope(lookup_t *lookup);
size_t lookup_cur_depth(const lookup_t *lookup);
lookup_var_t lookup_define(lookup_t *lookup, const char *name, size_t len,
			   var_flags_t flags);
lox_str_t *lookup_scope_find(const lookup_t *lookup, uint32_t idx);
lookup_var_t lookup_find_name(lookup_t *lookup, const char *name, size_t len);
bool lookup_has_defined(lookup_t *lookup, const char *name, size_t len);
lox_str_t *lookup_scope_find(const lookup_t *lookup, uint32_t idx);
hashmap_t *lookup_scope_at_depth(lookup_t *lookup, size_t idx);
hashmap_t *lookup_cur_scope(lookup_t *lookup);
void lookup_end_scope(lookup_t *lookup);

static inline lookup_var_t lookup_reserve(lookup_t *lookup, const char *name,
					  size_t len, var_flags_t flags)
{
	return lookup_define(lookup, name, len,
			     flags | LOOKUP_VAR_NOT_ASSIGNED);
}

static inline hashmap_t *lookup_global_scope(lookup_t *lookup)
{
	return list_get(&lookup->scopes, 0);
}

static inline bool lookup_is_scoped(lookup_t *lookup)
{
	return list_size(&lookup->scopes) > 1;
}

static inline bool lookup_var_is_mutable(lookup_var_t var)
{
	return var.var_flags & LOOKUP_VAR_MUTABLE;
}

static inline bool lookup_var_not_defined(lookup_var_t var)
{
	return var.var_flags & LOOKUP_VAR_NOT_DEFINED;
}

static inline bool lookup_var_is_assigned(lookup_var_t var)
{
	return !(var.var_flags & LOOKUP_VAR_NOT_ASSIGNED);
}

static inline bool lookup_var_is_global(lookup_var_t var)
{
	return var.var_flags & LOOKUP_VAR_GLOBAL;
}

#endif // __CLOX_CHUNK_LOOKUP_H__
