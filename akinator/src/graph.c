#include <stdlib.h>

#include <graphviz/gvc.h>
#include <jason/json.h>

#include "graph.h"
#include "color.h"

static Agnode_t* render_node(Agraph_t* g, json_value_t* val, Agnode_t* parent)
{
	static size_t cnt = 0;

	if(!val)
		return 0;

	if(val->type == JSON_STRING)
	{
		Agnode_t* end_node = agnode(g, 0, 1);
		agsafeset(end_node, "color", "white", "");
		agsafeset(end_node, "style", "rounded", "");
		agsafeset(end_node, "shape", "box", "");
		agsafeset(end_node, "fontcolor", "white", "");
		agsafeset(end_node, "label", val->value.str, "");


		return end_node;
	}

	if(val->type != JSON_OBJECT)
		return 0;

	json_object_t* obj = val->value.obj;

	if(obj->elem_cnt == 1 && obj->elements[0].value.type == JSON_OBJECT)
	{
		Agnode_t* obj_root = agnode(g, 0, 1);
		agsafeset(obj_root, "color", "white", "");
		agsafeset(obj_root, "style", "rounded", "");
		agsafeset(obj_root, "shape", "oval", "");
		agsafeset(obj_root, "fontcolor", "white", "");

		char* label = 0;
		asprintf(&label, "is it %s?", obj->elements[0].key);
		agsafeset(obj_root, "label", label, "");

		Agnode_t* next_node = render_node(g, &obj->elements[0].value, obj_root);

		if(obj_root && next_node)
			agedge(g, obj_root, next_node, 0, 1);

		return obj_root;
	}
	
	if(obj->elem_cnt == 2)
	{
		Agnode_t* yes_node = render_node(g, json_obj_get(obj, "yes"), 0);
		if(yes_node && parent)
		{
			Agedge_t* yes_edge = agedge(g, parent, yes_node, 0, 1);
			agsafeset(yes_edge, "color", "green", "");
		}

		
		Agnode_t* no_n = render_node(g, json_obj_get(obj, "no"), 0);
		if(parent && no_n)
		{
			Agedge_t* no_edge = agedge(g, parent, no_n, 0, 1);
			agsafeset(no_edge, "color", "red", "");
		}

		return 0;
	}

	return 0;
}

void render_graph(json_value_t* json, const char* output_filename)
{
	GVC_t *gvc = gvContext();

	Agraph_t *g = agopen("G", Agdirected, 0);

	agsafeset(g, "bgcolor", "gray12", "");

	render_node(g, json, 0);
	// render_obj(g, json->value.obj);

	gvLayout(gvc, g, "dot");

	FILE *file = fopen(output_filename, "wb");
	gvRender(gvc, g, "png", file);

	agclose(g);
	gvFreeContext(gvc);
}
