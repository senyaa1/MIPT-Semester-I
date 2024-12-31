#pragma once

#include <stdlib.h>
#include "json.h"

json_number_t json_deserialize_number(char* json_text, size_t len, size_t num_start, size_t* new_ptr);
json_char_t* json_deserialize_string(char* json_text, size_t len, size_t str_start, size_t* new_ptr);
json_value_t json_deserialize_value(char* json_text, size_t len, size_t value_start, size_t *new_ptr);
json_object_t* json_deserialize_object(char* json_text, size_t len, size_t object_start, size_t* new_ptr);
json_value_t json_deserialize(char* json_text);
