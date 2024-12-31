#include <stdio.h>
#include <stdlib.h>

#include "stack.h"

int main()
{
	stack_t s = {};
	STACK_INIT(&s, sizeof(int), 5);

	for(int i = 0; i < 10; i++)
	{
		stack_print_err(stack_push(&s, &i));
	}

	STACK_PRINT(&s)

	int data = 0;

	for(int i = 0; i < 10; i++)
	{
		stack_print_err(stack_pop(&s, &data));
	}

	stack_print_err(stack_chk(&s));

	STACK_PRINT(&s)

	stack_dtor(&s);
	return 0;
}
