#ifndef __CLOX_UTIL_DTOA_H__
#define __CLOX_UTIL_DTOA_H__

#include "common.h"

double strtod(const char *s00, char **se);
char *dtoa(double dd, int mode, int ndigits, int *decpt, int *sign, char **rve);
void freedtoa(char *s);

#endif // __CLOX_UTIL_DTOA_H__