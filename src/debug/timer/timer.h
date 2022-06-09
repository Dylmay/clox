#ifndef __CLOX_DEBUG_TIMER_H__
#define __CLOX_DEBUG_TIMER_H__

#include "util/common.h"
#include <time.h>

struct timespec time_function(void (*func)(void));

struct timespec time_function_mean(void (*func)(void), size_t run_cnt);

struct timespec timer_start(struct timespec *timer);
struct timespec timer_end(const struct timespec end);

struct timespec timespec_diff(struct timespec start, struct timespec end);
struct timespec timespec_avg(struct timespec a, struct timespec b);
void timespec_print(struct timespec time, bool formatTime);

#endif // __CLOX_DEBUG_TIMER_H__