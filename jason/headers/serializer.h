#pragma once

#include <string.h>
#include "json.h"

static const size_t JSON_SERIALIZED_BUF_DEFAULT_ALLOC_SZ = 1024;

typedef struct json_buf_writer
{
	char* buf;
	size_t buf_len;
	size_t cursor;
	size_t indent;
} json_buf_writer_t;

void json_serialize_val(json_buf_writer_t* writer, json_value_t* val);
void json_serialize_obj(json_buf_writer_t* writer, json_object_t* obj);
void json_serialize_arr(json_buf_writer_t* writer, json_array_t* arr);
void json_serialize_str(json_buf_writer_t* writer, json_char_t* str);
void json_serialize_num(json_buf_writer_t* writer, json_number_t num);

char* json_serialize(json_value_t* val);
