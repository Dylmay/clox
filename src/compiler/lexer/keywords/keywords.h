#ifndef __CLOX_LEXER_KEYWORDS_H__
#define __CLOX_LEXER_KEYWORDS_H__

#include "util/common.h"
#include "lexer/tokens.h"

enum tkn_type keyword_traverse(const char *identifier, size_t id_len);

#endif // __CLOX_LEXER_KEYWORDS_H__