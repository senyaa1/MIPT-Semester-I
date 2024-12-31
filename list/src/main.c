#include <stdio.h>
#include <stdlib.h>

#include "list.h"

int main()
{
	list_t list = { 0 };
	list_ctor(&list, 32);

	list_insert_head(&list, 0);

	list_insert_after(&list, 1, 123);
	list_insert_after(&list, 2, 456);

	list_insert_after(&list, 3, 567);
	list_insert_after(&list, 4, 789);

	list_insert_tail(&list, 4);
	list_insert_tail(&list, 5);
	list_insert_tail(&list, 6);

	list_insert_head(&list, 1);
	list_insert_head(&list, 2);

	list_insert_tail(&list, 10);

	list_remove_at(&list, 1);

	list_remove_at(&list, 5);

	// list.elements[7].next = 3;

	list_dump(&list);

	list_dtor(&list);
	return 0;
}
