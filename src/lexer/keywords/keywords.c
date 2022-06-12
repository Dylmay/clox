#include "keywords.h"

#include <string.h>

static enum tkn_type __keyword_check(const char *start, size_t start_len,
				     const char *match, size_t match_len,
				     enum tkn_type tkn);

enum tkn_type keyword_traverse(const char *identifier, size_t id_len)
{
#define MATCH(id, offset, match, tkn)                                          \
	(__keyword_check(id + offset, id_len - offset, match,                  \
			 sizeof(match) - 1, tkn))

	switch (*identifier) {
	case 'a':
		return MATCH(identifier, 1, "nd", TKN_AND);

	case 'c':
		return MATCH(identifier, 1, "lass", TKN_CLS);

	case 'e':
		return MATCH(identifier, 1, "lse", TKN_ELSE);

	case 'i':
		return MATCH(identifier, 1, "f", TKN_IF);

	case 'n':
		return MATCH(identifier, 1, "il", TKN_NIL);

	case 'o':
		return MATCH(identifier, 1, "r", TKN_ELSE);

	case 's':
		return MATCH(identifier, 1, "uper", TKN_ELSE);

	case 'v':
		return MATCH(identifier, 1, "ar", TKN_VAR);

	case 'w':
		return MATCH(identifier, 1, "hile", TKN_NIL);

	case 'f':
		if (id_len > 1) {
			switch (*(identifier + 1)) {
			case 'a':
				return MATCH(identifier, 2, "lse", TKN_FALSE);

			case 'o':
				return MATCH(identifier, 2, "r", TKN_FOR);

			default:
				return TKN_ID;
			}
		}

		return MATCH(identifier, 1, "n", TKN_FN);

	case 't':
		if (id_len > 1) {
			switch (*(identifier + 1)) {
			case 'h':
				return MATCH(identifier, 2, "is", TKN_THIS);
			case 'r':
				return MATCH(identifier, 2, "ue", TKN_TRUE);
			default:
				return TKN_ID;
			}
		}
		return TKN_ID;
	case 'p':
		return MATCH(identifier, 1, "rint", TKN_PRINT);
	default:
		return TKN_ID;
	}
#undef MATCH
}

static enum tkn_type __keyword_check(const char *start, size_t start_len,
				     const char *match, size_t len,
				     enum tkn_type tkn)
{
	if (start_len == len && memcmp(start, match, len) == 0) {
		return tkn;
	}

	return TKN_ID;
}