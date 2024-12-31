#include <unistd.h>
#include <stdio.h>
#include <wchar.h>

#include "io.h"
#include "ptrs.h"


void print_strings(const wchar_t** arr, size_t size, const char* title)
{
	printf("%s: \n", title);

	for(size_t i = 0; i < size; i++)
	{
		printf("\t%ls\n", (wchar_t*)PTR_STRIP(arr[i]));
	}
}
