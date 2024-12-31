#include <stdio.h>
#include "stack.h"


int main()
{
	stack_num num = stack_init(sizeof(int), 10);

	if(num < 0)
	{
		printf("Failed to initialize stack!\n");
		return -1;
	}

	for(int i = 0; i < 10; i++)
	{
		stack_print_err(stack_push(num, &i));
	}


	stack_get_and_print(num);

	int data = 0;
	for(int i = 0; i < 10; i++)
	{
		stack_print_err(stack_pop(num, &data));
		printf("%d\n", data);
	}

	stack_get_and_print(num);
	stack_destroy(num);
	return 0;
}
