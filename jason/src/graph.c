#include <stdlib.h>

#include <graphviz/gvc.h>

#include "graph.h"
#include "json.h"
#include "color.h"

static Agnode_t* render_arr(Agraph_t* g, json_array_t* arr);
static Agnode_t* render_obj(Agraph_t* g, json_object_t* obj);

static Agnode_t* render_val(Agraph_t* g, json_value_t* obj)
{
	static size_t val_cnt = 0;

	switch(obj->type)
	{
		case JSON_OBJECT:
			return render_obj(g, obj->value.obj);
		case JSON_ARRAY:
			return render_arr(g, obj->value.arr);
		default:
			break;
	}

	char* node_name = 0;
	asprintf(&node_name, "value_%ld", val_cnt++);
	Agnode_t* node = agnode(g, node_name, 1);

	agsafeset(node, "color", "red", "");
	agsafeset(node, "shape", "rect", "");
	char* num_str = 0;

	switch(obj->type)
	{
		case JSON_STRING:
			agset(node, "label", obj->value.str);
			break;
		case JSON_NUMBER:
			asprintf(&num_str, "%lg", obj->value.num);
			agset(node, "label", num_str);
			break;
		case JSON_TRUE:
			agset(node, "label", "true");
			break;
		case JSON_FALSE:
			agset(node, "label", "false");
			break;
		case JSON_NULL:
			agset(node, "label", "null");
			break;
		default:
			agset(node, "label", "[INVALID]");
			break;
	}

	return node;
}

static Agnode_t* render_arr(Agraph_t* g, json_array_t* arr)
{
	static size_t arr_cnt = 0;

	char* node_name = 0;
	asprintf(&node_name, "array_%ld", arr_cnt++);

	Agnode_t* arr_root = agnode(g, node_name, 1);
	agsafeset(arr_root, "color", "cyan", "");
	agsafeset(arr_root, "shape", "box", "");
	agsafeset(arr_root, "style", "filled", "");
	agsafeset(arr_root, "label", "[array]", "");

	// agsafeset(arr_root, "style", "filled", "");
	
	for(int i = 0; i < arr->length; i++)
	{
		agedge(g, arr_root, render_val(g, &arr->arr[i]), 0, 1);
	}

	return arr_root;
}

static Agnode_t* render_obj(Agraph_t* g, json_object_t* obj)
{
	static size_t obj_cnt = 0;

	Agnode_t* obj_root = agnode(g, 0, 1);
	agsafeset(obj_root, "color", "green", "");
	agsafeset(obj_root, "style", "filled", "");
	agsafeset(obj_root, "shape", "box", "");
	agsafeset(obj_root, "label", "[object]", "");

	for(int i = 0; i < obj->elem_cnt; i++)
	{
		if(!obj->elements[i].key || strlen(obj->elements[i].key) == 0)
		{
			fprintf(stderr, RED "NULL KEY\n" RESET);
			continue;
		}

		Agnode_t* entry = agnode(g, 0, 1);
		agset(entry, "label", obj->elements[i].key);
		agedge(g, obj_root, entry, 0, 1);

		Agnode_t* val = render_val(g, &obj->elements[i].value);
		agedge(g, entry, val, 0, 1);
	}

	return obj_root;
}

void render_graph(json_value_t* json, const char* output_filename)
{
	GVC_t *gvc = gvContext();

	Agraph_t *g = agopen("G", Agdirected, 0);

	render_val(g, json);

	gvLayout(gvc, g, "dot");

	FILE *file = fopen(output_filename, "wb");
	gvRender(gvc, g, "png", file);

	agclose(g);
	gvFreeContext(gvc);
}
