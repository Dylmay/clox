#ifndef __CLOX_DEBUG_TIMER_H__
#define __CLOX_DEBUG_TIMER_H__

#include "util/common.h"
#include <time.h>

struct timespec time_function(void (*func)(void));

struct timespec time_function_mean(void (*func)(void), size_t run_cnt);

void print_time(struct timespec time, bool formatTime);

#endif // __CLOX_DEBUG_TIMER_H__