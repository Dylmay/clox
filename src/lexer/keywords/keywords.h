#ifndef __CLOX_LEXER_KEYWORDS_H__
#define __CLOX_LEXER_KEYWORDS_H__

#include "lexer/tokens.h"

enum tkn_type keyword_traverse(const char *identifier, int id_len);

#endif // __CLOX_LEXER_KEYWORDS_H__