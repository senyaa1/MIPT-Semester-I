#include <stdlib.h>
#include <string.h>
#include "json.h"

void json_obj_add_entry(json_object_t* obj, json_char_t* key, json_value_t val)
{
	obj->elem_cnt++;
	obj->elements = (json_pair_t*)realloc(obj->elements, obj->elem_cnt * sizeof(json_pair_t));
	obj->elements[obj->elem_cnt - 1].key = key;
	obj->elements[obj->elem_cnt - 1].value = val;
}

// int json_obj_remove_entry(json_object_t* obj, json_char_t* key)
// {
// 	for(int i = 0; i < obj->elem_cnt; i++)
// 	{
// 		if(strcmp(key, obj->elements[i].key))	continue;
//
// 		for(int j = i; j < obj->elem_cnt; j++)
// 		{
// 			obj->elements[
// 		}
//
// 		obj->elements[i] = 0;
// 		obj->elem_cnt--;
// 		return 1;
// 	}
//
// 	return 0;
// }

json_value_t* json_obj_get(json_object_t* obj, json_char_t* key)
{
	for(int i = 0; i < obj->elem_cnt; i++)
	{
		if(strcmp(key, obj->elements[i].key))	continue;

		return &obj->elements[i].value;
	}

	return 0;
}

void json_free_object(json_object_t* obj)
{
	for(int i = 0; i < obj->elem_cnt; i++)
	{
		json_free_val(&obj->elements[i].value);
		free(obj->elements[i].key);
	}

	free(obj->elements);
	free(obj);
}

void json_free_val(json_value_t* val)
{
	switch(val->type)
	{
		case JSON_STRING:
			free(val->value.str);
			break;
		case JSON_ARRAY:
			json_free_array(val->value.arr);
			break;
		case JSON_OBJECT:
			json_free_object(val->value.obj);
			break;
		default:
			break;
	}
}

void json_free_array(json_array_t* arr)
{
	for(int i = 0; i < arr->length; i++)
		json_free_val(&arr->arr[i]);

	free(arr->arr);
	free(arr);
}

void json_free(json_value_t val)
{
	json_free_val(&val);
}

