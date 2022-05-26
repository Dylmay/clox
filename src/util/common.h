#ifndef __CLOX_UTIL_COMMON_H__
#define __CLOX_UTIL_COMMON_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define DEBUG_TRACE_EXECUTION
#define DEBUG_PRINT_CODE

#if defined DEBUG_TRACE_EXECUTION || defined DEBUG_PRINT_CODE
#define NDEBUG
#endif

#endif // __CLOX_UTIL_COMMON_H__