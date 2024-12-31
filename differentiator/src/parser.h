#pragma once

#include "lexer.h"

// codegen is useful, because these are used in parser.c to cast enumerated entries to their string representation
#define OPERATIONS	\
	OP_ENTRY(ADD)	\
	OP_ENTRY(SUB)	\
	OP_ENTRY(MUL)	\
	OP_ENTRY(DIV)	\
	OP_ENTRY(POW)	\
	OP_ENTRY(LOG)	\
	OP_ENTRY(SQRT)	\
	OP_ENTRY(SIN)	\
	OP_ENTRY(COS)	\
	OP_ENTRY(TG)	\
	OP_ENTRY(CTG)	\
	OP_ENTRY(LN)	\
	OP_ENTRY(ARCSIN)	\
	OP_ENTRY(ARCCOS)	\
	OP_ENTRY(ARCTG)		\
	OP_ENTRY(ARCCTG)	\
	OP_ENTRY(SH)		\
	OP_ENTRY(CH)		\
	OP_ENTRY(TH)		\
	OP_ENTRY(CTH)		\


#define OP_ENTRY(NAME) NAME,
typedef enum MATH_FUNCS
{
    INVALID = 0,
    OPERATIONS
} math_func_t;
#undef OP_ENTRY

typedef enum DIFF_NODE_TYPE : char
{
	NODE_INVALID 	= 0,
	NODE_NUMBER	= 1,
	NODE_FUNCTION	= 2,
	NODE_VARIABLE	= 3,
} diff_node_type_t;

typedef struct diff_node diff_node_t;
struct diff_node
{
	diff_node_type_t type;
	union 
	{
		math_func_t op_type;
		double number;
		char* variable;
	} value;
	diff_node_t* left;
	diff_node_t* right;
};

diff_node_t* parse_expression(lexer_t *lexer);
char* match_operation(math_func_t op);

diff_node_t* node_create();
diff_node_t* node_copy(diff_node_t* node);
diff_node_t* node_create_num(char *value);
diff_node_t* node_create_func(char *value);
diff_node_t* node_create_op(char *op);
diff_node_t* node_create_num_d(double value);
void node_free(diff_node_t* node);
