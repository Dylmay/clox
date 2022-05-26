#ifndef __CLOX_COMPILER_ERROR_HANDLER_H__
#define __CLOX_COMPILER_ERROR_HANDLER_H__

#include "compiler/parser.h"

void report_error(parser_t *prsr, const token_t *tkn, const char *msg);

static inline void report_error_at_current(parser_t *prsr, const char *msg)
{
	report_error(prsr, &(prsr->current), msg);
}

static inline void report_error_at_previous(parser_t *prsr, const char *msg)
{
	report_error(prsr, &(prsr->previous), msg);
}

#endif // __CLOX_COMPILER_ERROR_HANDLER_H__