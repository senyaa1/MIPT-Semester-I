#pragma once

#include <stdlib.h>

typedef double json_number_t;
typedef char json_char_t;

static const size_t JSON_STR_DEFAULT_ALLOC_SZ = 32;
static const size_t JSON_ARR_DEFAULT_ALLOC_SZ = 16;
static const size_t JSON_OBJ_DEFAULT_ALLOC_SZ = 16;

typedef enum JSON_VALUE_TYPE : char
{
	JSON_NONE	= 0,
	JSON_OBJECT	= 1,
	JSON_ARRAY	= 2,
	JSON_NUMBER	= 3,
	JSON_STRING	= 4,
	JSON_TRUE	= 5,
	JSON_FALSE	= 6,
	JSON_NULL	= 7,
} json_value_type_t;

typedef struct json_value
{
	json_value_type_t type;
	union {
		json_number_t	num;
		json_char_t*	str;
		struct json_object*	obj;
		struct json_array*	arr;
	} value;
} json_value_t;

typedef struct json_pair
{
	json_char_t* key;
	json_value_t value;
} json_pair_t;

typedef struct json_object
{
	size_t elem_cnt;
	json_pair_t* elements;
} json_object_t;

typedef struct json_array 
{
	size_t length;
	json_value_t* arr;
} json_array_t;

void json_free_val(json_value_t* val);
void json_free_object(json_object_t* obj);
void json_free_array(json_array_t* arr);
void json_free(json_value_t val);


void json_obj_add_entry(json_object_t* obj, json_char_t* key, json_value_t val);
json_value_t* json_obj_get(json_object_t* obj, json_char_t* key);
