#pragma once

#include "parser.h"
#include "buffer.h"

static const size_t DEFAULT_TEX_ALLOC = 256;

buf_writer_t tex_init(diff_node_t* tree);
char* tex_end(buf_writer_t* writer);
char* tex_dump(diff_node_t* tree);
void bufcpy(buf_writer_t* writer, const char* string);
void tex_dump_equation(buf_writer_t* writer, diff_node_t* node, char* preamble);
