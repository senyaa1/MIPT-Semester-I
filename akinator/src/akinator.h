#pragma once

#include <jason/json.h>

#define JSON_DB_PATH "db.json"
#define GRAPH_IMG_PATH "graph.png"

#define MIN(a, b) ((a < b) ? (a) : (b))

typedef struct qualities
{
	char** strs;
	size_t cnt;
	size_t allocated;
} qualities_t;

void guess(json_value_t* val);
void compare(json_object_t* root);
void definition(json_object_t* root);
