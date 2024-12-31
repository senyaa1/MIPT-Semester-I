#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

#include "json.h"
#include "deserializer.h"
#include "color.h"

static inline bool is_whitespace(char sym)
{
	return sym == ' ' || sym == '\t' || sym == '\r' || sym == '\n';
}

static inline size_t get_text_start(char* json_text, size_t len, size_t start)
{
	size_t actual_start = start;
	while(actual_start < len)
	{
		if(!is_whitespace(json_text[actual_start++]))
		{
			actual_start--;
			break;
		}
	}

	return actual_start;
}

json_number_t json_deserialize_number(char* json_text, size_t len, size_t num_start, size_t* new_ptr)
{
	size_t actual_start = get_text_start(json_text, len, num_start);

	char *end = 0;
	long double num = strtold(json_text + actual_start, &end);

	if (errno == ERANGE || isnan(num) || !isfinite(num))
	{
		fprintf(stderr, "error during number parsing!\n");
		return NAN;
	}

	*new_ptr = end - json_text - 1;
	return num;
}


json_char_t* json_deserialize_string(char* json_text, size_t len, size_t str_start, size_t* new_ptr)
{
	bool str_started = false, str_ended = false;
	int begin_index = 0, end_index = 0;

	size_t str_len = 0, str_allocated = JSON_STR_DEFAULT_ALLOC_SZ;
	json_char_t* str = (char*)calloc(str_allocated, sizeof(char));

	bool prev_was_escaped_backslash = false;

	int i = str_start;
	for(; i < len - 1; i++)
	{
		if(str_ended) break;

		if(str_len + 1 >= str_allocated)
		{
			str_allocated *= 2;
			str = (char*)realloc(str, sizeof(json_char_t) * str_allocated);
		}

		char sym = json_text[i];

		if(is_whitespace(sym) && !str_started)
			continue;

		if(!is_whitespace(sym) && !str_started && sym != '"')
		{
			// fprintf(stderr, "Encountered symbols before string start (index: %d)!\n", i);
			goto error;
		}

		bool escaped = i > 1 && json_text[i - 1] == '\\' && !prev_was_escaped_backslash;
		if (!escaped && sym == L'\\')
		{
			prev_was_escaped_backslash = false;
			continue;
		}

		if (escaped && sym == L'\\')
		{
			prev_was_escaped_backslash = true;
			str[str_len++] = '\\';
			continue;
		}

		prev_was_escaped_backslash = false;
		
		if(escaped)
		{
			switch(sym)
			{
				default:
					fprintf(stderr, "invalid escape sequence! (\\%c)\n", sym);
					goto error;
					break;
				case 'r':
					str[str_len++] = L'\r';
					break;
				case 'n':
					str[str_len++] = L'\n';
					break;
				case 't':
					str[str_len++] = L'\t';
					break;
				case 'b':
					str[str_len++] = L'\b';
					break;
				case 'f':
					str[str_len++] = L'\f';
					break;
				case '/':
				case '"':
					str[str_len++] = sym;
					break;

				case 'u':			// deserialize things such as \u002F (/)
					errno = 0;
					char hex[5] = { 0 };
					char *end = 0;

					if(len - i - 1 < 4)
					{
			            		fprintf(stderr, "can't deserialize unicode code point cause string is out of bounds!\n");
						goto error;
					}

					memcpy(hex, json_text + i + 1, sizeof(char) * 5);

					char val = strtol(json_text + i + 1, &end, 16);

				        if (json_text + i + 1 == end || errno == ERANGE)
					{
			            		fprintf(stderr, "unable to deserialize unicode code point!\n");
						goto error;
					}

					str[str_len++] = val;
					i += 4;
					break;
			}

			continue;
		}

		switch(sym)
		{
			case '"':
				if(!str_started)
				{
					str_started = true;
					begin_index = i;
				}
				else 
				{
					str_ended = true;
					end_index = i;
				}
				break;

			default:
				str[str_len++] = sym;
		}
	}
	str[str_len] = '\x00';

	str_allocated = str_len + 1;
	str = (char*)realloc(str, sizeof(json_char_t) * str_allocated);

	*new_ptr = --i;

	return str;

error:
	free(str);
	return 0;
}

json_value_t json_deserialize_value(char* json_text, size_t len, size_t value_start, size_t* new_ptr)
{
	json_value_t val = (json_value_t){JSON_NONE, 0};
	size_t actual_start = get_text_start(json_text, len, value_start);
	json_char_t starting_sym = json_text[actual_start];

	if(starting_sym == '"')
	{
		json_char_t* str = json_deserialize_string(json_text, len, actual_start, new_ptr);
		// fprintf(stderr, "encountered string! %ls\n", str);
		val.type = JSON_STRING;
		val.value.str = str;
		return val;
	}
	
	if(starting_sym == '-' || isdigit(starting_sym))
	{
		json_number_t num = json_deserialize_number(json_text, len, actual_start, new_ptr);
		// fprintf(stderr, "encountered number! %Lf\n", num);
		val.type = JSON_NUMBER;
		val.value.num = num;
		return val;
	}

	if(starting_sym == ']')
	{
		*new_ptr = actual_start - 1;
		return val;	// end of array
	}


	if(starting_sym == '[')
	{
		// fprintf(stderr, "encountered array!\n");
		size_t array_ptr = actual_start;

		json_array_t* arr = (json_array_t*)calloc(sizeof(json_array_t), 1);
		arr->length = JSON_ARR_DEFAULT_ALLOC_SZ;
		arr->arr = (json_value_t*)calloc(sizeof(json_value_t), arr->length);

		size_t elem_cnt = 0;
		while(1)
		{
			if(elem_cnt >= arr->length)
			{
				arr->length *= 2;
				arr->arr = (json_value_t*)realloc(arr->arr, sizeof(json_value_t) * arr->length);
			}

			json_value_t elem = json_deserialize_value(json_text, len, array_ptr + 1, new_ptr);

			if(!elem.type)
			{
				fprintf(stderr, RED "Unable to deserialize array element! Probably trailing comma\n" RESET);
				json_free_array(arr);
				return val;
			}

			arr->arr[elem_cnt++] = elem;
			
			array_ptr = get_text_start(json_text, len, *new_ptr + 1);

			if(json_text[array_ptr] != L',')
				break;
		}

		arr->length = elem_cnt;
		arr->arr = (json_value_t*)realloc(arr->arr, sizeof(json_value_t) * arr->length);

		val.type = JSON_ARRAY;
		val.value.arr = arr;
		*new_ptr = array_ptr;
		return val;
	}

	if(starting_sym == '{')
	{
		// fprintf(stderr, "encountered object!\n");
		json_object_t* obj = json_deserialize_object(json_text, len, actual_start, new_ptr);
		val.type = JSON_OBJECT;
		val.value.obj = obj;
		return val;
	}


	if(len - actual_start < 5) return val;

	// printf("%ls\n", json_text + actual_start);

	#define VAL_CMP(val_str, enum_val)								\
		if(strncmp(json_text + actual_start, val_str, strlen(val_str)) == 0)			\
		{											\
			val.type = enum_val;								\
			return val;									\
		}											\


	VAL_CMP("true", JSON_TRUE)
	VAL_CMP("false", JSON_FALSE)
	VAL_CMP("null", JSON_NULL)
	
	#undef VAL_CMP

	return val;
}

json_object_t* json_deserialize_object(char* json_text, size_t len, size_t object_start, size_t* new_ptr)
{
	bool object_started = false, object_ended = false, deserialized_key = false, waiting_for_new_entry = false, failed_to_deserialize_key = false;
	json_object_t* obj = (json_object_t*)calloc(sizeof(json_object_t), 1);

	obj->elem_cnt = JSON_OBJ_DEFAULT_ALLOC_SZ;
	obj->elements = (json_pair_t*)calloc(sizeof(json_pair_t), obj->elem_cnt);

	size_t cnt = 0;
	for(size_t i = object_start; i < len; i++)
	{
		if (cnt >= obj->elem_cnt)
		{
			obj->elem_cnt *= 2;
			obj->elements = (json_pair_t*)realloc(obj->elements, sizeof(json_pair_t) * obj->elem_cnt);
		}

		char sym = json_text[i];
		if(is_whitespace(sym))
			continue;


		if(object_started && !deserialized_key && !failed_to_deserialize_key)
		{
			// fprintf(stderr, "was at index \"%d\"\n", i);
			size_t prev = i; 
			obj->elements[cnt].key = json_deserialize_string(json_text, len, i, &i);

			if(prev == i || !obj->elements[cnt].key)
			{
				failed_to_deserialize_key = true;
				if(cnt == 0)
				{
					continue;
				}
				// fprintf(stderr, RED "unable to deserialize key! at %d\n (probably trailing comma)\n" RESET, i);
				// goto error;
			}

			deserialized_key = true;
			waiting_for_new_entry = false;
			// fprintf(stderr, "deserialized object key: \"%s\"\n", obj->elements[cnt].key);
			// fprintf(stderr, "now at\"%d\"\n", i);
			continue;
		}

		switch(sym)
		{
		     case '{':
				object_started = true;
				// fprintf(stderr, "object started!\n");
				break;
		     case '}':
				if(waiting_for_new_entry)
				{
					fprintf(stderr, RED "Trailing comma!\n" RESET);
					goto error;
				}

				*new_ptr = i;
				// fprintf(stderr, "object ended!\n");
				object_ended = true;
				goto done;
				break;
		     case ':':
				// fprintf(stderr, "beginning to deserialize \"%s\" value\n", obj->elements[cnt].key);
				obj->elements[cnt].value = json_deserialize_value(json_text, len, i + 1, &i);
				// fprintf(stderr, "deserialized %s\n", obj->elements[cnt].value.value.str);
				cnt++;
				break;
		     case ',':
				waiting_for_new_entry = true;
				deserialized_key = false;
				failed_to_deserialize_key = false;
				// fprintf(stderr, "waiting for NEW entry!\n");
			     	break;
		}
	}

done:
	obj->elem_cnt = cnt;
	obj->elements = (json_pair_t*)realloc(obj->elements, sizeof(json_pair_t) * obj->elem_cnt);

	return obj;

error:
	json_free_object(obj);
	return 0;
}


json_value_t json_deserialize(char* json_text)
{
	size_t len = strlen(json_text);
	size_t end_ptr = 0;

	return json_deserialize_value(json_text, len, 0, &end_ptr);
}


