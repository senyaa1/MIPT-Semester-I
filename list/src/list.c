#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

#include "list.h"


static list_status_t list_increase_alloc(list_t* list)
{
	list_elem_t* new_alloc = (list_elem_t*)realloc(list->elements, list->size * 2 * sizeof(list_elem_t));

	if(!new_alloc) return LIST_ERR_ALLOC;

	list->elements = new_alloc;
	list->size *= 2;

	return LIST_OK;
}

static list_status_t list_maybe_increase_alloc(list_t* list)
{
	if(list->cnt < list->size - 1) return LIST_OK;
	
	return list_increase_alloc(list);
}


list_status_t list_ctor(list_t* list, size_t initial_size)
{
	if(!list) return  LIST_ERR_ARGNULL;
	if(list->elements) return LIST_ERR_ALLOC;

	list->size = initial_size;
	list->elements = (list_elem_t*)calloc(list->size + 1, sizeof(list_elem_t));
	list->cnt = 0;

	for(int i = 1; i < initial_size; i++)
	{
		list->elements[i].next = i + 1;
		list->elements[i].prev = 0;
	}

	list->elements[0].data = 0xbabecafe;
	list->elements[0].next = 0;
	list->elements[0].prev = 0;
	list->elements[0].used = 1;
	list->free = 1;

	return LIST_OK;
}

list_status_t list_dtor(list_t* list)
{
	if(!list) return LIST_ERR_ARGNULL;

	free(list->elements);
	memset(list, 0, sizeof(list_t));

	return LIST_OK;
}

list_elem_t* list_next(list_t* list, list_elem_t* elem)
{
	if(elem->next == 0)
		return 0;

	return &list->elements[elem->next];
}

list_elem_t* list_prev(list_t* list, list_elem_t* elem)
{
	if(elem->prev == 0)
		return 0;

	return &list->elements[elem->prev];
}

list_elem_t* list_begin(list_t* list)
{
	return &list->elements[list->elements[0].next];
}

list_elem_t* list_end(list_t* list)
{
	return &list->elements[list->elements[0].prev];
}

list_status_t list_chk(list_t* list)
{
	if(!list->elements)		return LIST_ERR_ALLOC;
	if(list->cnt > list->size)	return LIST_ERR_ALLOC;

	// for(list_elem_t elem = *list_begin(list); list_next(list, &elem) != 0; elem = *list_next(list, &elem))
	for(int i = 0; i < list->size; i++)
	{
		list_elem_t elem = list->elements[i];
		if(elem.prev == elem.next) return LIST_ERR_NOTLINKED;
		if(i != list->elements[elem.next].prev && elem.used) return LIST_ERR_NOTLINKED;
	}

	
	return LIST_OK;
}

list_status_t list_dump(list_t* list)
{
	FILE *dot_file = fopen(GRAPH_SRC_FILENAME, "w");

	fprintf(dot_file, "digraph G {\n");
	fprintf(dot_file, "node [shape=Mrecord];\n");
	fprintf(dot_file, "rankdir=LR;\n") ;
	fprintf(dot_file, "bgcolor=\"grey12\";\n");

	fprintf(dot_file, "\"0\"[style=\"filled\";color=\"#FFFFFF\";fontcolor=\"#000000\";label=\"root\"];\n");

	fprintf(dot_file, "\"free\"[style=\"filled\";color=\"#FFFFFF\";fontcolor=\"#000000\";label=\"free\"];\n");
	fprintf(dot_file, "\"free\" -> \"_%d\"[color=\"#444444\"; fontcolor=\"green\"];\n", list->free);

	for(int i = 1; i <= list->size; i++)
	{
		list_elem_t elem = list->elements[i];
		if(!elem.used)
			fprintf(dot_file, "\"_%d\"[color=\"#444444\";fontcolor=\"#888888\";label=\"ind: %d | val: %d | next: %d | prev: %d \"];\n", i, i, elem.data, elem.next, elem.prev);
		else
			fprintf(dot_file, "\"%d\"[color=\"#FFFFFF\";fontcolor=\"#FFFFFF\";label=\"ind: %d | val: %d | next: %d | prev: %d \"];\n", i, i, elem.data, elem.next, elem.prev);
	}

	
	// used nodes
	for(int i = 0; i < list->size; i++)
	{
		list_elem_t elem = list->elements[i];

		if(!elem.used)
		{
			fprintf(dot_file, "\"_%d\" -> \"_%d\"[color=\"#444444\"; fontcolor=\"green\"];\n", i, list->elements[i].next);
			continue;
		}

		if(list_next(list, &elem) != 0)
			fprintf(dot_file, "\"%d\" -> \"%d\"[color=green; fontcolor=\"green\"];\n", i, list->elements[i].next);

		if(list_prev(list, &elem) != 0)
			fprintf(dot_file, "\"%d\" -> \"%d\"[color=red; fontcolor=\"red\"];\n", i, list->elements[i].prev);


	}

	fprintf(dot_file, "\"physical_0\"[style=filled; color=\"#FFFFFF\";fontcolor=\"#000000\";label=\"root\"];\n");
	fprintf(dot_file, "\"physical_free\"[style=filled; color=\"#FFFFFF\";fontcolor=\"#000000\";label=\"free\"];\n");
	fprintf(dot_file, "\"physical_free\" -> \"physical_%d\"[color=gray];\n", list->free);

	for(int i = 1; i <= list->size; i++)
	{
		if(list->elements[i].used)
			fprintf(dot_file, "\"physical_%d\"[color=\"#FFFFFF\";fontcolor=\"#FFFFFF\";label=\"ind: %d | val: %d | next: %d | prev: %d\"];\n", i, i, list->elements[i].data, list->elements[i].next, list->elements[i].prev);
		else
			fprintf(dot_file, "\"physical_%d\"[color=\"#444444\";fontcolor=\"#888888\";label=\"ind: %d | val: %d | next: %d | prev: %d\"];\n", i, i, list->elements[i].data, list->elements[i].next, list->elements[i].prev);

		fprintf(dot_file, "\"physical_%d\" -> \"physical_%d\"[color=invis; thickness=1000; weight=\"10000\"];\n", i - 1, i);
	}

	for(int i = 1; i < list->size; i++)
	{
		if(!list->elements[i].used)
		{
			fprintf(dot_file, "\"physical_%d\" -> \"physical_%d\"[color=gray];\n", i, list->elements[i].next);
		}
		else
		{
			if(i == list->elements[list->elements[i].next].prev)
			{
				fprintf(dot_file, "\"physical_%d\" -> \"physical_%d\"[color=cyan];\n", i, list->elements[i].next);
			}
			else
			{
				fprintf(dot_file, "\"physical_%d\" -> \"physical_%d\"[color=red;style=bold];\n", list->elements[i].next, i);
				fprintf(dot_file, "\"physical_%d\" -> \"physical_%d\"[color=green;style=bold];\n", list->elements[i].prev, i);
			}
			
		}
	}


	fprintf(dot_file, "}\n");
	fclose(dot_file);

	system("dot -Tpng " GRAPH_SRC_FILENAME " -o " GRAPH_FILENAME);

	FILE *log_file = fopen(LOG_FILENAME, "w");

	fprintf(log_file, "<pre>\n");
	fprintf(log_file, "List [0x%p]: \n", list);
	fprintf(log_file, "\telements: [0x%p]\n", list->elements);
	fprintf(log_file, "\tsize: %ld\n", list->size);
	fprintf(log_file, "\tcnt: %ld\n", list->cnt);
	fprintf(log_file, "</pre>\n");
	fprintf(log_file, "<img src=\"" GRAPH_FILENAME "\">");

	fclose(log_file);

	char* cmd_buf = (char*)calloc(1024, sizeof(char));
	char* cwd = (char*)calloc(512, sizeof(char));
	getcwd(cwd, 512);

	sprintf(cmd_buf, "$BROWSER file:///%s/%s", cwd, LOG_FILENAME);

	system(cmd_buf);

	free(cmd_buf);
	free(cwd);

	return LIST_OK;
}

list_status_t list_insert_before(list_t* list, int index, list_data_t data)
{
	if(list_maybe_increase_alloc(list)) 
		return LIST_ERR_ALLOC;

	int insertion_index = list->free;
	list->free = list->elements[insertion_index].next;

	list_elem_t* elem = &list->elements[insertion_index];

	elem->data = data;
	elem->used = true;

	list_elem_t* next = &list->elements[index];
	list_elem_t* prev = &list->elements[next->prev];

	elem->next = index;
	elem->prev = next->prev;

	next->prev = insertion_index;
	prev->next = insertion_index;

	list->cnt++;
	
	return LIST_OK;
}


list_status_t list_remove_at(list_t* list, int index)
{
	list_elem_t* elem = &list->elements[index];

	list->elements[elem->prev].next = elem->next;
	list->elements[elem->next].prev = elem->prev;

	memset(elem, 0, sizeof(list_elem_t));

	elem->next = list->free;
	list->free = index;

	list->cnt--;

	return LIST_OK;
}

list_status_t list_insert_after(list_t* list, int index, list_data_t data)
{
	return list_insert_before(list, list->elements[index].next, data);
}

list_status_t list_insert_head(list_t* list, list_data_t data)
{
	return list_insert_before(list, list->elements[0].next, data);
}

list_status_t list_insert_tail(list_t* list, list_data_t data)
{
	return list_insert_after(list, list->elements[0].prev, data);
}

list_status_t list_remove_head(list_t* list)
{
	return list_remove_at(list, list->elements[0].next);
}

list_status_t list_remove_tail(list_t* list)
{
	return list_remove_at(list, list->elements[0].prev);
}

int list_find_val(list_t* list, list_data_t val)
{
	int counter = 0;
	int cur = list->elements[0].next, end = list->elements[0].prev;
	
	while(cur != end)
	{
		cur = list->elements[cur].next;
		if(list->elements[cur].data == val) return cur;
	}


	return -1;
}

int list_index(list_t* list, int index)
{
	int counter = 0, elem = list->elements[0].next;
	
	while(elem != list->elements[0].prev)
	{
		counter++;
		if(counter == index) return counter;
	}

	return -1;
}


