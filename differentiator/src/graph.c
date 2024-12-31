#include <stdlib.h>

#include <graphviz/gvc.h>

#include "graph.h"
#include "parser.h"

#include "color.h"

static Agnode_t* create_node(Agraph_t* g)
{
	Agnode_t* node = agnode(g, 0, 1);
	agsafeset(node, "color", "white", "");
	agsafeset(node, "style", "rounded", "");
	agsafeset(node, "shape", "rect", "");
	agsafeset(node, "fontcolor", "white", "");

	return node;
}

static Agnode_t* render_node(Agraph_t* g, diff_node_t* node)
{
	if(!node) return 0;
	Agnode_t* root = create_node(g);

	char* label = 0;
	switch(node->type)
	{
		case NODE_NUMBER:
			asprintf(&label, "%g", node->value.number);
			agsafeset(root, "color", "green", "");
			break;
		case NODE_FUNCTION:
			asprintf(&label, "%s", match_operation(node->value.op_type));
			agsafeset(root, "color", "blue", "");
			break;
		case NODE_VARIABLE:
			asprintf(&label, "var: %s", node->value.variable);
			agsafeset(root, "color", "red", "");
			break;
		default:
			asprintf(&label, "[INVALID]");
			break;
	}

	agset(root, "label", label);

	if(node->left)
	{
		Agnode_t* left_node = render_node(g, node->left);
		Agedge_t* l_edge = agedge(g, root, left_node, 0, 1);
		agsafeset(l_edge, "color", "white", "");
	}

	if(node->right)
	{
		Agnode_t* right_node = render_node(g, node->right);
		Agedge_t* r_edge = agedge(g, root, right_node, 0, 1);
		agsafeset(r_edge, "color", "white", "");
	}

	return root;
}

void draw_tree(diff_node_t* root, const char* output_filename)
{
	GVC_t *gvc = gvContext();

	Agraph_t *g = agopen("G", Agdirected, 0);

	agsafeset(g, "bgcolor", "gray12", "");

	render_node(g, root);
	// render_obj(g, json->value.obj);

	gvLayout(gvc, g, "dot");

	FILE *file = fopen(output_filename, "wb");
	gvRender(gvc, g, "png", file);

	agclose(g);
	gvFreeContext(gvc);
}
