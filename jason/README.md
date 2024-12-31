# libjason

RFC 8259-compliant C library for serializing and deserializing JSON.

*This library is still in development and **is not suitable for production** use yet!!*

## Examples

```c
#include <jason/json.h>
#include <jason/serializer.h>
#include <jason/deserializer.h>

int main(int argc, char** argv)
{
	char* json_text = 0;
	read_file(argv[1], &json_text);

	json_value_t json = json_deserialize(json_text);

	char* serialized_buf = json_serialize(&json);
	printf("Serialized buf: \n%s\n", serialized_buf);

	json_free(json);
	free(json_text);
	free(serialized_buf);
	return 0;
}

```

### Getting started

##### Types
The following value types are implemented:
 - `JSON_NONE`
 - `JSON_OBJECT` 
 - `JSON_ARRAY`
 - `JSON_NUMBER` 
 - `JSON_STRING` 
 - `JSON_TRUE`
 - `JSON_FALSE`

JSON value struct:
```c
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
```

##### Functions
```c
char* json_serialize(json_value_t* val);
```

```c
json_value_t json_deserialize(char* json_text);
```

```c
void json_free(json_value_t val);
```
