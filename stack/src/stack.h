#pragma once

#include <stdlib.h>
#include <stdint.h>

#define STACK_ENABLE_CANARIES
#define STACK_CRC


#ifdef STACK_ENABLE_CANARIES
typedef uint64_t canary_t;
#endif

#ifndef STACK_NDEBUG
	#define STACK_PRINT(s)				stack_print(s, __FILE_NAME__, __LINE__, __PRETTY_FUNCTION__);
	#define STACK_INIT(s, elem_size, initial_size)	stack_ctor(s, elem_size, initial_size,  __FILE_NAME__, __LINE__, #s);
#elif
	#define STACK_INIT(s, elem_size, initial_size)	stack_ctor(s, elem_size, initial_size);
#endif


typedef enum stack_status
{
	STACK_OK		= 0,
	STACK_ERR_ALLOC		= (1 << 1),
	STACK_ERR_ARGNULL	= (1 << 2),
	STACK_ERR_INITIALIZED	= (1 << 3),
	STACK_ERR_UNINITIALIZED	= (1 << 4),
	STACK_ERR_EMPTY		= (1 << 5),
	STACK_ERR_CANARY	= (1 << 6),
	STACK_ERR_CRC		= (1 << 7),
} stack_status_t;


typedef struct stack
{
#ifdef STACK_ENABLE_CANARIES
	canary_t canary1;
#endif

#ifdef STACK_CRC
	uint64_t crc;
#endif

	void*	buf;
	size_t	cur_index;
	size_t	allocated_size;
	size_t	elem_size;
	size_t	last_allocation_index;

#ifndef STACK_NDEBUG
	char*	instantiated_with_name;
	char*	instantiated_at_file;
	int	instantiated_at_line;
#endif

#ifdef STACK_ENABLE_CANARIES
	canary_t canary2;
#endif
} stack_t;


#ifndef STACK_NDEBUG
stack_status_t	stack_ctor(stack_t* s, size_t elem_size, size_t initial_size, const char* file, const int line, const char* name);
#elif
stack_status_t	stack_ctor(stack_t* s, size_t elem_size, size_t initial_size);
#endif

stack_status_t	stack_dtor(stack_t* s);
stack_status_t	stack_push(stack_t* s, const void* data);
stack_status_t	stack_pop(stack_t* s, void* resulting_data);
stack_status_t	stack_print_err(stack_status_t status);
stack_status_t	stack_print(stack_t* s, const char* file, const int line, const char* function);

stack_status_t stack_chk(stack_t* stack);
#define STACK_CHK_RET(s)	stack_status_t stack_chk_res = stack_chk(s);	\
				if(stack_chk_res)				\
					return stack_chk_res;
