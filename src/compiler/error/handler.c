#include <stdio.h>
#include "handler.h"

void report_error(parser_t *prsr, const token_t *tkn, const char *msg)
{
	if (prsr->panic_mode) {
		return;
	}

	fprintf(stderr, "[line %d] Error", tkn->line);

	if (tkn->type == TKN_EOF) {
		fprintf(stderr, " at end");
	} else if (tkn->type != TKN_ERR) {
		fprintf(stderr, " at '%.*s'", tkn->len, tkn->start);
	}

	fprintf(stderr, ": %s\n", msg);
	prsr->had_err = true;
	prsr->panic_mode = true;
}