#include <stdio.h>
#include <math.h>

#include "parser.h"
#include "differentiator.h"
#include "tex.h"

#define D(NODE)					differentiate(NODE, writer)
#define C(NODE)					node_copy(NODE)
#define NUM(NUM)				node_create_num_d(NUM)
#define OP(OPERATION, LEFT, RIGHT)		op(#OPERATION, (LEFT), (RIGHT))
#define F(FUNCTION, LEFT, RIGHT)		f(#FUNCTION, (LEFT), (RIGHT))

static diff_node_t* f(char* func, diff_node_t* left, diff_node_t* right)
{
	diff_node_t* node = node_create_func(func);
	node->left = left;
	node->right = right;
	return node;
}

static diff_node_t* op(char* op, diff_node_t* left, diff_node_t* right)
{
	diff_node_t* node = node_create_op(op);
	node->left = left;
	node->right = right;
	return node;
}

static diff_node_t* diff_function(diff_node_t* node, buf_writer_t* writer)
{
	math_func_t type = node->value.op_type;
	diff_node_t* LEFT = node->left, *RIGHT = node->right;

	switch(type)
	{

		case ADD:
			return OP(+, D(LEFT), D(RIGHT));

		case SUB:
			return OP(-, D(LEFT), D(RIGHT));

		case MUL:
			return OP(+,	OP(*, D(LEFT), C(RIGHT)), 	
					OP(*, C(LEFT), D(RIGHT)));

		case DIV:
			return OP(/,	
					OP(-, 
						OP(*, D(LEFT), C(RIGHT)),		
						OP(*, C(LEFT), D(RIGHT))
				),
				OP(^, C(RIGHT), NUM(2))
			);

		case SIN:
			return OP(*, F(cos, C(LEFT), 0), D(LEFT));

		case COS:
			return OP(*,
					OP(*, 
						F(sin, C(LEFT), 0), 
						D(LEFT)
				),
				NUM(-1)
			);

		case TG:
			return OP(*,  
					OP(/, 
						NUM(1), 
						OP(^, F(cos, C(LEFT), 0), 
							NUM(2))),
					D(LEFT));
		case CTG:
			return OP(*,	OP(/, 
					NUM(-1), 
					OP(^, F(sin, C(LEFT), 0),
						NUM(2))),
				D(LEFT));
		case LN:
			return	OP(/,	D(LEFT), 
					C(LEFT));
		case LOG:
			return OP(/,	F(ln, C(LEFT), 0), 
					F(ln, C(RIGHT), 0)
				);
		case POW:
			if(LEFT->type == NODE_VARIABLE && RIGHT->type == NODE_NUMBER)
			{
				return OP(*,	
						C(RIGHT), 
						OP(^, C(LEFT), 
							OP(-, C(RIGHT), NUM(1))
						)
					);
			}

			if(LEFT->type == NODE_FUNCTION && RIGHT->type == NODE_NUMBER)
			{
				return OP(*,	C(RIGHT),
						OP(*,	
							D(LEFT),
							OP(^, 
								C(LEFT),
								node_create_num_d(RIGHT->value.number - 1)
							)
						)
					);
			}

			return OP(*,	
					C(node),
					OP(+, 
						OP(*,
							D(RIGHT),
							F(ln, C(LEFT), 0)),
						OP(*, 
							C(RIGHT),
							OP(/, D(LEFT),
								C(LEFT))
						)
					)
				);
			break;
		case SQRT:
			return OP(/,	D(LEFT),
					OP(*,	NUM(2), 
						C(node)
					)
				);

		case ARCSIN:
			return OP(/,	D(LEFT),
						F(sqrt, OP(-,	
								node_create_num_d(1),
								OP(^, C(LEFT), node_create_num_d(2))
							),0
						)
					);
							
		case ARCCOS:
			return OP(/,	OP(*, node_create_num_d(-1), D(LEFT)),
						F(sqrt, OP(-,	
								node_create_num_d(1),
								OP(^, C(LEFT), node_create_num_d(2))
							),0
						)
					);
		case ARCTG:
			return OP(/,	D(LEFT),
						OP(+,	
								node_create_num_d(1),
								OP(^, C(LEFT), node_create_num_d(2))
						)
					);
		case ARCCTG:
			return OP(/,	OP(*, node_create_num_d(-1), D(LEFT)),
						OP(+,	
								node_create_num_d(1),
								OP(^, C(LEFT), node_create_num_d(2))
						)
					);

		case SH:
			return OP(*, F(ch, C(LEFT), 0), D(LEFT));
		case CH:
			return OP(*, F(sh, C(LEFT), 0), D(LEFT));

		case TH:
			return OP(/,	D(LEFT),
					OP(^, F(ch, C(LEFT), 0), node_create_num_d(2))
					);
		case CTH:
			return OP(*, node_create_num_d(-1), 
					OP(/,	D(LEFT),
					OP(^, F(ch, C(LEFT), 0), node_create_num_d(2))
					));
		default:
			return 0;
	}
}

diff_node_t* differentiate(diff_node_t* node, buf_writer_t* writer)
{
	switch(node->type)
	{
		case NODE_NUMBER:	
			return NUM(0);
		case NODE_FUNCTION:
			return diff_function(node, writer);
		case NODE_VARIABLE: 
			return NUM(1); 
		default:
			return 0;
	}
}

#define TRIV_NUM(NODE, VAL, ACTION)					\
if(NODE && NODE->type == NODE_NUMBER && NODE->value.number == VAL)	\
{									\
	(*optimization_cnt)++;						\
	free(node);							\
	node_free(NODE);						\
	ACTION;								\
}									\


#define PERFORM_OP(TYPE, OPERATION)					\
case TYPE:								\
	(*optimization_cnt)++;						\
	node_free(node);						\
	return NUM(left_num OPERATION right_num);			\


#define ISNUM(NODE)	(NODE && NODE->type == NODE_NUMBER)
#define ISFUNC(NODE)	(NODE && NODE->type == NODE_FUNCTION)

#define TRY_GET_NUM(NODE, CHILD1)				\
if(!second_const && NODE->CHILD1 && NODE->CHILD1->type == NODE_NUMBER)	\
{									\
	second_const = NODE->CHILD1->value.number;			\
	func = NODE;							\
	to_modify = &NODE->CHILD1->value.number;			\
}									\

#define OPT_OP(OPERATION_NAME, OPERATION)						\
case OPERATION_NAME:									\
	(*optimization_cnt)++;								\
	free(node);									\
	free(to_free);									\
	*to_modify = (first_const OPERATION second_const);				\
	return func;									\


static diff_node_t* optimize_recursive(diff_node_t* node, size_t* optimization_cnt)
{
	if(!node) return 0;

	if(node->type != NODE_FUNCTION)
		return node;

	diff_node_t* LEFT = node->left, *RIGHT = node->right;

	// Trivial (*0, +0, etc)
	switch(node->value.op_type)
	{
		case ADD:
		case SUB:
			TRIV_NUM(RIGHT, 0,	return LEFT);
			TRIV_NUM(LEFT, 0,	return RIGHT);
			break;

		case MUL:
			TRIV_NUM(RIGHT, 1,	return LEFT);
			TRIV_NUM(LEFT, 1,	return RIGHT);

			TRIV_NUM(RIGHT, 0,	node_free(LEFT); return NUM(0));
			TRIV_NUM(LEFT, 0,	node_free(RIGHT); return NUM(0));
			break;
		case POW:
			TRIV_NUM(RIGHT, 1,	return LEFT);
			break;
		case DIV:
			TRIV_NUM(RIGHT, 1,	return LEFT);
			break;
		default:
			break;
	}

	// Calculate constants (e.g. 2 + 2)
	if(ISNUM(LEFT) && ISNUM(RIGHT))
	{					
		double left_num = LEFT->value.number, right_num = RIGHT->value.number;
		switch(node->value.op_type)
		{
			PERFORM_OP(ADD, +)
			PERFORM_OP(SUB, -)
			PERFORM_OP(MUL, *)
			PERFORM_OP(DIV, /)
			case POW:
				(*optimization_cnt)++;
				node_free(node);
				return NUM(pow(left_num, right_num));
			default:
				break;
		}
	}

	// Optimize children of the same operations e.g. (mul -> mul)
	if(	(ISFUNC(LEFT) && ISNUM(RIGHT) && node->value.op_type == LEFT->value.op_type)	|| 
		(ISFUNC(RIGHT) && ISNUM(LEFT) && node->value.op_type == RIGHT->value.op_type))
	{
		double first_const = 0, second_const = 0, *to_modify = 0;
		diff_node_t* func = 0, *to_free = 0;

		if(ISNUM(RIGHT))
		{
			first_const = RIGHT->value.number;
			TRY_GET_NUM(LEFT, left)
			TRY_GET_NUM(LEFT, right)
			to_free = RIGHT;
		}
		else
		{
			first_const = LEFT->value.number;
			TRY_GET_NUM(RIGHT, left)
			TRY_GET_NUM(RIGHT, right)
			to_free = LEFT;
		}

		if(second_const)
		{
			switch(node->value.op_type)
			{
				OPT_OP(ADD, +)
				OPT_OP(SUB, -)
				OPT_OP(DIV, /)
				OPT_OP(MUL, *)
				default:
					break;
			}
		}
	}

	node->right = optimize_recursive(node->right, optimization_cnt);
	node->left = optimize_recursive(node->left, optimization_cnt);

	return node;
}

diff_node_t* optimize(diff_node_t* tree, buf_writer_t* writer)
{
	bufcpy(writer, "Давайте немного упростим данное выражение.\n");
	size_t optimization_cnt = 0;
	do 
	{
		optimization_cnt = 0;
		tree = optimize_recursive(tree, &optimization_cnt);
	} while (optimization_cnt > 0);
  
	return tree;
}

