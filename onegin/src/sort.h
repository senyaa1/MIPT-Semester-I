#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

int str_comparator(const void* in_a, const void* in_b);

void random_sort(void* array, size_t cnt, size_t elsize, int(*comparator)(const void*, const void*));

void quick_sort(void *array, size_t cnt, size_t elsize, int(*comparator)(const void*, const void*));
