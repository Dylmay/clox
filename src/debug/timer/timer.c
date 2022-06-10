#include <stdio.h>

#include "timer.h"
#include "list/list.h"

static void __print_fmtd_time(time_t time);

void timer_start(struct timespec *timer)
{
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, timer);
}

struct timespec timer_end(const struct timespec timer)
{
	struct timespec end;
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);

	return timespec_diff(timer, end);
}

struct timespec time_function(void (*func)(void))
{
	struct timespec timer;

	timer_start(&timer);
	func();
	return timer_end(timer);
}

struct timespec time_function_mean(void (*func)(void), size_t run_cnt)
{
	if (run_cnt == 0) {
		return (struct timespec){ 0 };
	}

	struct timespec avg_time = time_function(func);

	while (--run_cnt > 0) {
		struct timespec this_run = time_function(func);

		avg_time = timespec_avg(avg_time, this_run);
	}

	return avg_time;
}

void timespec_print(struct timespec time, bool formatTime)
{
	if (formatTime) {
		__print_fmtd_time(time.tv_sec);
		printf(" (s), ");
		__print_fmtd_time(time.tv_nsec);
		printf(" (ns)");
	} else {
		printf("%lu (s), %lu (ns)", time.tv_sec, time.tv_nsec);
	}
}

static void __print_fmtd_time(time_t time)
{
	list_t time_list = list_of_type(uint8_t);
	do {
		uint8_t temp_time = time % 10;
		time /= 10;

		list_push(&time_list, &temp_time);
	} while (time);

	while (time_list.cnt) {
		printf("%d", *(list_pop(&time_list)));
		if (time_list.cnt && time_list.cnt % 3 == 0) {
			printf(",");
		}
	}

	list_free(&time_list);
}

struct timespec timespec_diff(struct timespec start, struct timespec end)
{
	struct timespec temp;

	if ((end.tv_nsec - start.tv_nsec) < 0) {
		temp.tv_sec = end.tv_sec - start.tv_sec - 1;
		temp.tv_nsec = (end.tv_nsec - start.tv_nsec) + 1000000000;
	} else {
		temp.tv_sec = end.tv_sec - start.tv_sec;
		temp.tv_nsec = (end.tv_nsec - start.tv_nsec);
	}

	return temp;
}

struct timespec timespec_avg(struct timespec a, struct timespec b)
{
	return (struct timespec){
		.tv_nsec = (a.tv_nsec + b.tv_nsec) / 2,
		.tv_sec = (a.tv_sec + b.tv_sec) / 2,
	};
}