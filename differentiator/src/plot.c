#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "parser.h"
#include "buffer.h"
#include "plot.h"


static diff_node_t* plot_dump_recursive(buf_writer_t* writer, diff_node_t* node)
{
	#define PLOT_OP(OP_NAME, OP)					\
		case OP_NAME:						\
			plot_dump_recursive(writer, node->left);	\
			bufcpy(writer, OP);				\
			plot_dump_recursive(writer, node->right);	\
			break;						\

	#define PLOT_FUNC(FUNC_NAME, FUNC)				\
		case FUNC_NAME:						\
			bufcpy(writer, #FUNC "(");			\
			plot_dump_recursive(writer, node->left);	\
			bufcpy(writer, ")");				\
			break;						\


	if(!node) return 0;

	char* num_str = 0;
	switch(node->type)
	{
		case NODE_VARIABLE:
			bufcpy(writer, node->value.variable);
			break;
		case NODE_NUMBER:
			asprintf(&num_str, "%g", node->value.number);
			bufcpy(writer, num_str);
			free(num_str);
			break;
		case NODE_FUNCTION:
			switch(node->value.op_type)
			{
				PLOT_OP(ADD, " + ")
				PLOT_OP(SUB, " - ")
				PLOT_OP(MUL, " * ")
				PLOT_OP(DIV, " / ")
				PLOT_OP(POW, " ** ")
				case LOG:
					bufcpy(writer, "log(");
					plot_dump_recursive(writer, node->left);
					bufcpy(writer, ")/log(");
					plot_dump_recursive(writer, node->right);
					bufcpy(writer, ")");
					break;
				PLOT_FUNC(SIN, sin)
				PLOT_FUNC(COS, cos)
				PLOT_FUNC(TG, tan)
				PLOT_FUNC(CTG, tan)
				PLOT_FUNC(LN, ln)
				PLOT_FUNC(SQRT, sqrt)
				PLOT_FUNC(ARCSIN, asin)
				PLOT_FUNC(ARCCOS, acos)
				PLOT_FUNC(ARCTG, atan)
				PLOT_FUNC(ARCCTG, atan)

				PLOT_FUNC(SH, sinh)
				PLOT_FUNC(CH, cosh)
				PLOT_FUNC(TH, tanh)
				PLOT_FUNC(CTH, tanh)
			}
			break;
		default:
			return 0;
	}
	
	return 0;
}

int plot_dump(diff_node_t* node, char* graph_name, char* color, char* filename)
{
	buf_writer_t writer = { writer.buf = (char*)calloc(DEFAULT_PLOT_ALLOC, sizeof(char)), .buf_len = DEFAULT_PLOT_ALLOC};

	plot_dump_recursive(&writer, node);
	writer.buf[writer.cursor] = '\x00';

	FILE *gnuplotPipe = popen("gnuplot -persistent", "w");
	if (gnuplotPipe == NULL) 
	{
		fprintf(stderr, "Could not open pipe to gnuplot.\n");
		return -1;
	}
	fprintf(gnuplotPipe, "set title '%s'\n", graph_name);
	fprintf(gnuplotPipe, "set xlabel 'x'\n");
	fprintf(gnuplotPipe, "set ylabel 'y'\n");
	fprintf(gnuplotPipe, "set grid\n");
	fprintf(gnuplotPipe, "set samples 10000\n");

	fprintf(gnuplotPipe, "set terminal png size 800,600 enhanced\n");
	fprintf(gnuplotPipe, "set output '%s'\n", filename);

	fprintf(gnuplotPipe, "plot (%s) with lines linecolor rgb \"%s\"\n", writer.buf, color);
	fprintf(stderr, "plotting: %s\n", writer.buf);


	pclose(gnuplotPipe);
	free(writer.buf);
	return 0;
}
