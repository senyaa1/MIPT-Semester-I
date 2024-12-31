#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <wchar.h>
#include <wctype.h>

#include "ptrs.h"

#define SWAPBUF_LEN 4096

static inline void memswap(void* a, void* b, size_t len)
{
	static char buf[SWAPBUF_LEN] = {0};

	size_t offset = 0;

	for(int i = 0; i < len / SWAPBUF_LEN + 1; i++)
	{
		size_t cur_len = len - offset;
		if(cur_len > SWAPBUF_LEN) cur_len = SWAPBUF_LEN;

		memcpy(buf, b + offset, cur_len);
		memcpy(b + offset, a + offset, cur_len);
		memcpy(a + offset, buf, cur_len);

		offset += cur_len;
	}
}

static inline void comp_swap(void* a, void* b, size_t len, int(*comparator)(const void*, const void*))
{
	if(comparator(a, b) != 1) return;

	memswap(a, b, len);
}

static inline bool is_punctuation(wchar_t c) 
{
	return wcschr(L"!\"#$%&'()*+,./:;<=>?@[\\]-—^_`{|}~ «»", c) != NULL;
}

static size_t partition(void *arr, size_t elsize, size_t l, size_t r, int(*comparator)(const void*, const void*)) 
{
	void* pivot = arr + r * elsize;
	size_t i = l - 1;

	for (size_t j = l; j < r; j++) {
		// if (arr[j] > pivot)
		if (comparator(arr + j * elsize, pivot) == 1)
			continue;

		memswap(arr + ++i * elsize, arr + j * elsize, sizeof(int));
	}

	memswap(arr + ++i * elsize, arr + r * elsize, sizeof(int));
	return i;
}

static void quicksort(void *arr, size_t elsize, int l, int r, int(*comparator)(const void*, const void*))
{
	if (l >= r)
		return;

	if (r - l <= 3) // Sedgewick
	{
		comp_swap(arr + ((l >> 2) + (r >> 2) + (l & r & 1)) * elsize, arr + (r - 1) * elsize, elsize, comparator);
		comp_swap(arr + (l) * elsize, arr + (r - 1) * elsize, elsize, comparator);
		comp_swap(arr + (l) * elsize, arr + (r) * elsize, elsize, comparator);
		comp_swap(arr + (r - 1) * elsize, arr + (r) * elsize, elsize, comparator);
	}

        size_t pivot_index = partition(arr, elsize, l, r, comparator);

        quicksort(arr, elsize, l, pivot_index - 1, comparator);
        quicksort(arr, elsize, pivot_index + 1, r, comparator);
}

void quick_sort(void *array, size_t cnt, size_t elsize, int(*comparator)(const void*, const void*))
{
	quicksort(array, elsize, 0, cnt - 1, comparator);
}

// best sorting algo
// in the best case works faster than any other sorting algorithm
// patent pending
void random_sort(void* array, size_t cnt, size_t elsize, int(*comparator)(const void*, const void*))
{
	srand(time(NULL)); // the seed better be good for this one

	bool is_sorted = false;
	while(!is_sorted)
	{
		is_sorted = true;

		for(size_t i = 0; i < (cnt - 1); i++)
		{
			if(comparator(array + i * elsize, array + (i+1) * elsize) == 1)
			{
				is_sorted = false;
				break;
			}
		}

		if(is_sorted) break;

		void* r1 = (array + (rand() % cnt) * elsize);
		void* r2 = (array + (rand() % cnt) * elsize);

		memswap(r1, r2, elsize);
	}
}

int str_comparator(const void* in_a, const void* in_b)
{
	wchar_t  *a = *(wchar_t**)(in_a),
		 *b = *(wchar_t**)(in_b);

	uint16_t len_a = PTR_GETLEN(a),
		 len_b = PTR_GETLEN(b);

	a = PTR_STRIP(a);
	b = PTR_STRIP(b);

	size_t	a_ptr = 0, 
		b_ptr = 0;

	size_t min_len = ((len_a < len_b) ? len_a : len_b);
	while(a_ptr < min_len && b_ptr < min_len)
	{
		while(is_punctuation(a[a_ptr]) && a_ptr < min_len)
			a_ptr++;

		while(is_punctuation(b[b_ptr]) && b_ptr < min_len)
			b_ptr++;

		wchar_t chr_a = towlower(a[a_ptr]),  // ispunct
			chr_b = towlower(b[b_ptr]);

		if(chr_a == chr_b)
		{
			a_ptr++;
			b_ptr++;
			continue;
		}

		return (chr_a < chr_b) ? -1 : 1;
	}
	return 0;
}
