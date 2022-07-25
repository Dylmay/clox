#ifndef __CLOX_CHUNK_LOOKUP_H__
#define __CLOX_CHUNK_LOOKUP_H__

#include "util/common.h"
#include "util/map/map.h"
#include "val/val.h"

typedef uint8_t var_flags_t;
#define LOOKUP_VAR_INVALID (0)
#define LOOKUP_VAR_NO_FLAGS (0)

#define LOOKUP_VAR_IMMUTABLE (0 << 0)
#define LOOKUP_VAR_MUTABLE (1 << 0)

#define LOOKUP_VAR_NOT_DEFINED (0 << 1)
#define LOOKUP_VAR_DEFINED (1 << 1)

#define LOOKUP_VAR_NOT_ASSIGNED (0 << 2)
#define LOOKUP_VAR_ASSIGNED (1 << 2)

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
lookup_var_t lookup_declare(lookup_t *lookup, const char *chars, size_t len,
			    bool mutable);
lookup_var_t lookup_define(lookup_t *lookup, const char *name, size_t len,
			   bool mutable);
lox_str_t *lookup_scope_find(const lookup_t *lookup, uint32_t idx);
lookup_var_t lookup_find_name(lookup_t *lookup, const char *name, size_t len);
bool lookup_has_defined(lookup_t *lookup, const char *name, size_t len);
lox_str_t *lookup_scope_find(const lookup_t *lookup, uint32_t idx);
hashmap_t *lookup_scope_at_depth(lookup_t *lookup, size_t idx);
hashmap_t *lookup_cur_scope(lookup_t *lookup);
void lookup_end_scope(lookup_t *lookup);
bool lookup_scope_has_name(lookup_t *lookup, const char *name, size_t len);

static inline bool lookup_var_is_mutable(lookup_var_t var)
{
	return var.var_flags & LOOKUP_VAR_MUTABLE;
}

static inline bool lookup_var_defined(lookup_var_t var)
{
	return var.var_flags & LOOKUP_VAR_DEFINED;
}

static inline bool lookup_var_is_assigned(lookup_var_t var)
{
	return var.var_flags & LOOKUP_VAR_ASSIGNED;
}

#endif // __CLOX_CHUNK_LOOKUP_H__
