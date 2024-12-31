#pragma once

#include "parser.h"
#include "tex.h"

diff_node_t* differentiate(diff_node_t* node, buf_writer_t* writer);
diff_node_t* optimize(diff_node_t* tree, buf_writer_t* writer);
