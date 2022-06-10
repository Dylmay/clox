#ifndef __TEST_UTIL_MAP_H__
#define __TEST_UTIL_MAP_H__

#define ITEM_CNT 10000

void map_test_all();
void map_test_new();
void map_test_insert();
void map_test_remove();
void map_test_get();
void map_test_find();

#define BENCHMARK_ITERATIONS 100

void map_bench_all();
void map_bench_insert();
void map_bench_traffic();
void map_bench_remove();
void map_bench_get();
void map_bench_find();

#endif // __TEST_UTIL_MAP_H__