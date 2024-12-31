#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#include "parser.h"
#include "lexer.h"

void node_free(diff_node_t* node)
{
	if(!node)	return;

	node_free(node->left);
	node_free(node->right);

	free(node);
}

char* match_operation(math_func_t op)
{
	#define OP_ENTRY(NAME)	\
	case NAME:		\
		return #NAME;	\

	switch(op)
	{
		OPERATIONS
		default:
	 		return 0;
	}

	#undef OP_ENTRY
}

math_func_t get_std_func(char* func)
{
	#define OP_ENTRY(NAME)				\
		if(strcasecmp(func, #NAME) == 0)	\
			return NAME;			\

	OPERATIONS

	#undef OP_ENTRY
	#undef OP_STR

	return INVALID;
}

diff_node_t* node_create()
{
	diff_node_t* node = (diff_node_t*)calloc(1, sizeof(diff_node_t));
	return node;
}

diff_node_t* node_copy(diff_node_t* node)
{
	if(!node) return 0;

	diff_node_t* new_node = (diff_node_t*)calloc(1, sizeof(diff_node_t));
	memcpy(new_node, node, sizeof(diff_node_t));
	node->left = node_copy(node->left);
	node->right = node_copy(node->right);
	return new_node;
}

diff_node_t* node_create_num_d(double value)
{
	diff_node_t* node = node_create();
	node->type = NODE_NUMBER;
	node->value.number = value;
	return node;
}

diff_node_t* node_create_num(char *value)
{
	char* end = 0;
	return node_create_num_d(strtod(value, &end));
}

diff_node_t* node_create_var(char *value)
{
	diff_node_t* node = node_create();

	node->type = NODE_VARIABLE;
	node->value.variable = value;

	return node;
}

diff_node_t* node_create_func(char *func)
{
	diff_node_t* node = node_create();

	node->type = NODE_FUNCTION;
	node->value.op_type = get_std_func(func);

	return node;
}

diff_node_t* node_create_op(char *op)
{
	diff_node_t* node = node_create();

	node->type = NODE_FUNCTION;

	#define OP(OP_SYM, FUNC)			\
		case OP_SYM:				\
			node->value.op_type = FUNC;	\
			break;				\


	switch(op[0])
	{
		OP('+', ADD)
		OP('-', SUB)
		OP('*', MUL)
		OP('/', DIV)
		OP('^', POW)
		default:
			fprintf(stderr, "Unknown operation (%c)\n", op[0]);
			break;
	}
	
	#undef OP

	return node;
}

diff_node_t* parse(lexer_t *lexer) 
{
	char *token = current_token(lexer);

	if (!token) 
	{
		fprintf(stderr, "Unexpected end of input\n");
		return 0;
	}

	if (isdigit(token[0]) || token[0] == '.') 
	{
		lexer->pos++;
		return node_create_num(token);
	} 

	if (strcmp(token, "(") == 0) {
		lexer->pos++;
		diff_node_t* node = parse_expression(lexer);

		char* next_token = current_token(lexer);
		if (next_token && strcmp(next_token, ")") != 0) 
		{
			fprintf(stderr, "Expected ')'\n");
			return 0;
		}

		lexer->pos++;
		return node;
	} 


	if (isalpha(token[0])) 
	{
		char *func = token;
		lexer->pos++;

		// no ( => variable
		char* next_token = current_token(lexer);
		if (next_token && strcmp(next_token, "(") != 0 || !next_token) 
		{
			printf("var: %s\n", func);
			return node_create_var(token);
		}

		lexer->pos++;
		diff_node_t* arg = parse_expression(lexer);

		next_token = current_token(lexer);
		if (next_token && strcmp(next_token, ")") != 0) 
		{
			fprintf(stderr, "Expected ')'\n");
			return 0;
		}

		if(!get_std_func(func))
		{
			fprintf(stderr, "Unkown function! (%s)\n", func);
			return 0;
		}

		lexer->pos++;

		diff_node_t* node = node_create_func(func);
		node->left = arg;

		return node;
	}

	fprintf(stderr, "Unexpected token: %s\n", token);
	return 0;
}

#define STRCMP(STR) (strcmp(current_token(lexer), #STR) == 0)
#define PARSEFUNC(NAME, COMPARISON, NEXT_FUNC)							\
diff_node_t* NAME(lexer_t *lexer)								\
{												\
	diff_node_t* node = NEXT_FUNC(lexer);							\
	while (current_token(lexer) && (COMPARISON))						\
	{											\
		char *op = current_token(lexer);						\
		lexer->pos++;									\
		diff_node_t* right = NEXT_FUNC(lexer);						\
		diff_node_t* newNode = node_create_op(op);					\
		newNode->left = node;								\
		newNode->right = right;								\
		node = newNode;									\
	}											\
	return node;										\
}												\

PARSEFUNC(parseFactor,		STRCMP(^),			parse)
PARSEFUNC(parseTerm,		(STRCMP(*) || STRCMP(/)),	parseFactor)
PARSEFUNC(parse_expression,	(STRCMP(+) || STRCMP(-)),	parseTerm)

#undef STRCMP
#undef PARSEFUNC
