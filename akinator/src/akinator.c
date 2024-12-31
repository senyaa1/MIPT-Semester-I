#include <stdbool.h>
#include <stdio.h>

#include <jason/json.h>
#include <jason/serializer.h>
#include <jason/deserializer.h>

#include "akinator.h"
#include "color.h"


static json_object_t* add_new_diff(json_object_t* parent, char* diff, char* name)
{
	char* copied_name = (char*)calloc(strlen(name) + 1, sizeof(char));
	memcpy(copied_name, name, strlen(name));

	json_value_t val = (json_value_t){.type = JSON_OBJECT, .value.obj = (json_object_t*)calloc(1, sizeof(json_object_t)) };
	json_object_t* next_obj = (json_object_t*)calloc(1, sizeof(json_object_t));

	json_obj_add_entry(val.value.obj, "yes", (json_value_t){.type = JSON_STRING, .value.str = copied_name});
	json_obj_add_entry(val.value.obj, "no", (json_value_t){.type = JSON_OBJECT, .value.obj = next_obj});

	json_obj_add_entry(parent, diff, val);
	return next_obj;
}

static void add_qual(json_char_t* key, qualities_t* qualities)
{
	if(!qualities) return;

	qualities->strs[qualities->cnt++] = key;

	if(qualities->cnt >= qualities->allocated)
	{
		qualities->allocated *= 2;
		qualities->strs = (char**)realloc(qualities->strs, qualities->allocated * sizeof(char*));
	}
}

static bool search(json_object_t* obj, char* to_find, char* prev, qualities_t* qualities)
{
	for(int i = 0; i < obj->elem_cnt; i++)
	{
		json_value_t elem = obj->elements[i].value;
		json_char_t* key = obj->elements[i].key;

		if(elem.type == JSON_OBJECT)
		{
			if(search(elem.value.obj, to_find, key, qualities))
			{
				add_qual(key, qualities);
				return true;
			}
		}

		if(elem.type == JSON_STRING && strcmp(elem.value.str, to_find) == 0)
		{
			add_qual(key, qualities);
			return true;
		}
	}

	return false;
}

void guess(json_value_t* val)
{
	if(!val) return;
	if(val->type == JSON_STRING)
	{
		printf("It is " GREEN "%s\n" RESET, val->value.str);
		return;
	}

	if(val->type != JSON_OBJECT)
		return;

	json_object_t* elem = val->value.obj;

	if(elem->elem_cnt == 0)
	{
		printf(RED "No questions left! Let's add new object!\n" RESET);
		printf(WHITE "Enter name> " BLUE);
		char* item = (char*)calloc(256, sizeof(char));
		char* difference = (char*)calloc(256, sizeof(char));
		scanf("%255s", item);
		printf(RESET WHITE "Enter difference> " BLUE);
		scanf("%255s", difference);
		printf(RESET);

		json_object_t* next = add_new_diff(elem, item, difference);
		return;
	}

	while(1)
	{
		printf(WHITE "Is it %s?> " RESET BLUE, elem->elements[0].key);
		char str[256] = {0};
		scanf("%255s", (char*)str);
		printf(RESET);

		if(strcasecmp(str, "yes") == 0)
		{
			guess(json_obj_get(elem->elements[0].value.value.obj, "yes"));
			return;
		}

		if(strcasecmp(str, "no") == 0)
		{
			guess(json_obj_get(elem->elements[0].value.value.obj, "no"));
			return;
		}

		printf("yes/no\n");
	}
}
static qualities_t get_quals(json_object_t* root, char* to_find)
{
	qualities_t qual = { .cnt = 0, .allocated = 16, .strs =  (char**)calloc(16, sizeof(char*))};

	if(!search(root, to_find, 0, &qual))
	{
		printf(RED "Can't find this object!\n" RESET);
		return (qualities_t){0};
	}

	return qual;
}

static qualities_t get_and_print_quals(json_object_t* root, char* to_find)
{
	qualities_t qual = get_quals(root, to_find);

	for(int i = qual.cnt - 1; i > 0; i -= 2)
	{
		char* key = qual.strs[i];
		char* state = qual.strs[i - 1];

		if(strcmp(state, "yes") == 0)
		{
			printf(GREEN "Is %s   " RESET, key);
		}
		else if(strcmp(state, "no") == 0)
		{
			printf(RED "Not %s    " RESET, key);
		}
	}

	printf("\n");

	return qual;
}

void definition(json_object_t* root)
{
	printf(WHITE "Enter object name> " RESET BLUE );

	char to_find[256] = {0};
	scanf("%255s", (char*)to_find);
	printf(RESET);

	qualities_t quals = get_and_print_quals(root, to_find);
	free(quals.strs);
}

void compare(json_object_t* root)
{
	char to_find1[256] = {0};
	char to_find2[256] = {0};

	printf(WHITE "First object> " RESET BLUE );
	scanf("%255s", (char*)to_find1);
	printf(RESET);

	printf(WHITE "Second object> " RESET BLUE );
	scanf("%255s", (char*)to_find2);
	printf(RESET);

	qualities_t qual1 = get_quals(root, to_find1);
	qualities_t qual2 = get_quals(root, to_find2);

	for(int i = 1; i < qual1.cnt; i += 2)
	{
		bool found_matching = false, has_info = false;
		for(int j = 1; j < qual2.cnt; j += 2)
		{
			if(strcmp(qual1.strs[i], qual2.strs[j]) == 0)
			{ 
				has_info = true;
				if(strcmp(qual1.strs[i - 1], qual2.strs[j - 1]) != 0)
					break;

				found_matching = true;

				printf(BLUE "Similarities: Both ");
				if(!strcmp(qual1.strs[i - 1], "yes"))
 				{
					printf(GREEN "are %s" RESET, qual1.strs[i]);
					break;
				}
				printf(RED "aren't %s\n" RESET, qual1.strs[i]);

				break;
			}
		}
		if(!found_matching && has_info)
		{
			printf(YELLOW "Difference: %s " RESET, to_find1);
			if(!strcmp(qual1.strs[i - 1], "yes"))
			{
				printf(GREEN "is %s, while %s isn't!\n" RESET, qual1.strs[i], to_find2);
			}
			else 
			{
				printf(RED "is not %s, while %s is!\n" RESET, qual1.strs[i], to_find2);
			}


		}
		// printf(BLUE "Differences: ");
	}

	free(qual1.strs);
	free(qual2.strs);
}
