#pragma once

#include "parser.h"
#include "buffer.h"

static const size_t DEFAULT_PLOT_ALLOC = 256;

int plot_dump(diff_node_t* node, char* graph_name, char* color, char* filename);
