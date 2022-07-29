/**
 * @file timer.h
 * @author Dylan Mayor
 * @brief header file holding timing utilities
 *
 */
#ifndef __CLOX_DEBUG_TIMER_H__
#define __CLOX_DEBUG_TIMER_H__

#include "util/common.h"
#include <time.h>

/**
 * @brief gets the time taken to run the passed function
 *
 * @param func the function to run
 * @return struct timespec the time taken to complete
 */
struct timespec time_function(void (*func)(void));

/**
 * @brief gets the mean time taken to run the passed function run_cnt times
 *
 * @param func the function to run
 * @param run_cnt the number of times to run the function
 * @return struct timespec the mean time taken to complete
 */
struct timespec time_function_mean(void (*func)(void), size_t run_cnt);

/**
 * @brief starts the given timer
 *
 * @param timer the timer to start recording
 */
void timer_start(struct timespec *timer);
/**
 * @brief gets the end time of the given timer
 *
 * @param end the timer to get the end time of
 * @return struct timespec the time span of the timer
 */
struct timespec timer_end(const struct timespec end);

/**
 * @brief gets the timespan/diff between the two timers
 *
 * @param start the start timer
 * @param end the end timer
 * @return struct timespec the difference between the two timers
 */
struct timespec timespec_diff(struct timespec start, struct timespec end);
/**
 * @brief averages the time between two passed timers
 *
 * @param a timer a
 * @param b timer b
 * @return struct timespec the average time of the two timers
 */
struct timespec timespec_avg(struct timespec a, struct timespec b);
/**
 * @brief prints the time to stdout
 *
 * @param time the time to print
 * @param formatTime whether to format the time as ss,sss,sss(s) nn,nnn,nnn(ns)
 */
void timespec_print(struct timespec time, bool formatTime);

#endif // __CLOX_DEBUG_TIMER_H__