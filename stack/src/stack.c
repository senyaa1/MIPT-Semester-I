#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/mman.h>
#include <time.h>

#include "stack.h"
#include "crc.h"
#include "colors.h"

static const size_t MIN_STACK_SIZE = 16;


#ifdef STACK_ENABLE_CANARIES
static uint64_t canaryval = 0x0;

static void init_canary(void* canaryptr, size_t sz)
{
	srand(time(NULL));

	for (size_t i = 0; i < sz; i++) 
		((char*)canaryptr)[i] = (char)rand() % 256;
}

static stack_status_t check_canary(stack_t* stack)
{
	if(!stack->buf) return STACK_ERR_UNINITIALIZED;
	if(canaryval == 0x0) init_canary(&canaryval, sizeof(canaryval));

	if(stack->canary2 != canaryval)
		return STACK_ERR_CANARY;

	if(*((canary_t*)((char*)stack->buf + stack->allocated_size + sizeof(canary_t))) != canaryval)
		return STACK_ERR_CANARY;

	return STACK_OK;
}

static void set_canary(stack_t* stack)
{
	if(canaryval == 0x0) init_canary(&canaryval, sizeof(canaryval));

	stack->canary1 = canaryval;
	stack->canary2 = canaryval;

	*((canary_t*)((char*)stack->buf)) = canaryval;
	*((canary_t*)((char*)stack->buf + stack->allocated_size + sizeof(canary_t))) = canaryval;
}
#endif


static void* get_buf_ptr(stack_t* stack)
{
#ifdef STACK_ENABLE_CANARIES
	return (char*)stack->buf + sizeof(canary_t);
#else
	return stack->buf;
#endif
}


#ifdef STACK_CRC
static uint32_t stack_get_crc(stack_t* stack)
{
	return crc32((char*)stack->buf + sizeof(canary_t) * 2, stack->allocated_size);
}

static void stack_recalc_crc(stack_t* stack)
{
	stack->crc = stack_get_crc(stack);
}

static stack_status_t check_crc(stack_t* stack)
{	
	if(stack->crc != stack_get_crc(stack)) return STACK_ERR_CRC;

	return STACK_OK;
}
#endif


static stack_status_t increase_alloc(stack_t *stack)
{
	STACK_CHK_RET(stack)

	stack->allocated_size *= 2;

#ifdef STACK_ENABLE_CANARIES
	stack->buf = realloc(stack->buf, stack->allocated_size + sizeof(canary_t) * 2);
	set_canary(stack);
#else
	stack->buf = realloc(stack->buf, stack->allocated_size);
#endif
	if(!stack->buf) return STACK_ERR_ALLOC;

	stack->last_allocation_index = stack->allocated_size / stack->elem_size;

#ifdef STACK_CRC
	stack_recalc_crc(stack);
#endif

	return STACK_OK;
}


static stack_status_t decrease_alloc(stack_t *stack)
{
	STACK_CHK_RET(stack)

	stack->allocated_size /= 2;

#ifdef STACK_ENABLE_CANARIES
	stack->buf = realloc(stack->buf, stack->allocated_size + sizeof(canary_t) * 2);
	set_canary(stack);
#else
	stack->buf = realloc(stack->buf, stack->allocated_size);
#endif
	if(!stack->buf) return STACK_ERR_ALLOC;

	stack->last_allocation_index = stack->allocated_size / stack->elem_size;

#ifdef STACK_CRC
	stack_recalc_crc(stack);
#endif

	return STACK_OK;
}

inline static stack_status_t maybe_decrease_alloc(stack_t *stack)
{
	if (stack->cur_index <= (stack->last_allocation_index / 2) - 1 && stack->allocated_size / stack->elem_size / 2 - 1 > MIN_STACK_SIZE)
		return decrease_alloc(stack);

	return STACK_OK;
}

stack_status_t stack_print(stack_t* stack, const char* file, const int line, const char* function)
{
	if(!stack) return STACK_ERR_ARGNULL;

#ifndef STACK_NDEBUG

	printf( "stack_t %s" BLUE " [%p]. " RESET 
		"Instantiated at " GREEN UNDERLINE "%s:%d" RESET 
		", printing from: " CYAN UNDERLINE "%s:%d (%s)\n" RESET,
		stack->instantiated_with_name, stack, stack->instantiated_at_file, stack->instantiated_at_line,
		file, line, function);

	size_t cap = stack->allocated_size / stack->elem_size;

	printf("\tcnt \t\t= " YELLOW "%lu\n" RESET, stack->cur_index);
	printf("\tcapacity \t= " YELLOW "%lu\n" RESET, cap);

#ifdef STACK_CRC
	if(check_crc(stack))
		printf("\tCRC\t\t= " RED UNDERLINE "0x%lx" RESET RED "  <<-- INCORRECT!!\n" RESET, stack->crc);
	else
		printf("\tCRC\t\t=" GREEN " 0x%lx\n" RESET, stack->crc);
#endif
	printf("\tdata" BLUE " [%p]\n" RESET, get_buf_ptr(stack));

#ifdef STACK_ENABLE_CANARIES
	printf(BLUE "\tbuf canaries: %lx %lx\n" RESET, *(canary_t*)stack->buf, *(canary_t*)((char*)stack->buf + stack->allocated_size + sizeof(canary_t)));
#endif

	for(size_t i = 0; i < cap; i++)
	{
		if(i < stack->cur_index)
			printf(RED "\t\t*\t[%lu]\t" YELLOW "= %d\n" RESET, i, ((int*)get_buf_ptr(stack))[i]);
		else
			printf(GREEN "\t\t\t[%lu]\t" YELLOW "= %d\n" RESET, i, ((int*)get_buf_ptr(stack))[i]);
	}
#endif
	return STACK_OK;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#pragma GCC diagnostic ignored "-Wwrite-strings"
stack_status_t stack_print_err(stack_status_t status)
{
	// if(status == STACK_OK) return status;

	#define STATSTR(stat)			\
		case stat:			\
			cur_status = #stat;	\
			break;			\

	char* cur_status = NULL;
	switch(status)
	{
		STATSTR(STACK_OK)
		STATSTR(STACK_ERR_ALLOC)
		STATSTR(STACK_ERR_ARGNULL)
		STATSTR(STACK_ERR_INITIALIZED)
		STATSTR(STACK_ERR_EMPTY)
		STATSTR(STACK_ERR_UNINITIALIZED)
		STATSTR(STACK_ERR_CRC)
		STATSTR(STACK_ERR_CANARY)
		default:
			cur_status = "STACK_ERR_UNHANDLED";
		break;
	}

	if(status)
		printf("Stack status: " RED "%s" RESET "\n", cur_status);
	else
		printf("Stack status: " GREEN "%s" RESET "\n", cur_status);


	return status;

	#undef STATSTR	
}
#pragma GCC diagnostic pop


#ifndef STACK_NDEBUG
stack_status_t	stack_ctor(stack_t* stack, size_t elem_size, size_t initial_size, const char* file, const int line, const char* name)
#elif
stack_status_t	stack_ctor(stack_t* stack, size_t elem_size, size_t initial_size)
#endif
{
	if(!stack) return STACK_ERR_ARGNULL;
	if(stack->buf) return STACK_ERR_INITIALIZED;

	stack->elem_size = elem_size;
	
	size_t to_allocate = ((initial_size < MIN_STACK_SIZE) ? MIN_STACK_SIZE : initial_size) * stack->elem_size;
	stack->allocated_size = to_allocate;

#ifdef STACK_ENABLE_CANARIES
	stack->buf = calloc(1, to_allocate + (sizeof(canary_t) * 2));;
	set_canary(stack);
#else
	stack->buf = calloc(1, to_allocate);
#endif

	if(!get_buf_ptr(stack)) return STACK_ERR_ALLOC;

#ifdef STACK_CRC
	stack_recalc_crc(stack);
#endif

#ifndef STACK_NDEBUG
	stack->instantiated_at_file = (char*)file;
	stack->instantiated_at_line = (int)line;
	stack->instantiated_with_name = (char*)name;
#endif

	return STACK_OK;
}

stack_status_t stack_dtor(stack_t* stack)
{
	STACK_CHK_RET(stack)

	free(stack->buf);
	memset(stack, 0, sizeof(stack_t));

	return STACK_OK;
}

stack_status_t stack_push(stack_t* stack, const void* data)	
{
	STACK_CHK_RET(stack)

	if((stack->cur_index * stack->elem_size) >= stack->allocated_size)
		if(increase_alloc(stack)) return STACK_ERR_ALLOC;

	memcpy(((char*)get_buf_ptr(stack) + (stack->cur_index++) * stack->elem_size), data, stack->elem_size);

#ifdef STACK_CRC
	stack_recalc_crc(stack);
#endif

	return STACK_OK;
}

stack_status_t stack_pop(stack_t* stack, void* resulting_data)	
{
	STACK_CHK_RET(stack)

	if(stack->cur_index <= 0) return STACK_ERR_EMPTY;
	if(maybe_decrease_alloc(stack)) return STACK_ERR_ALLOC;

	memcpy((char*)resulting_data, ((char*)get_buf_ptr(stack) + (--stack->cur_index) * stack->elem_size), stack->elem_size);

#ifdef STACK_CRC
	stack_recalc_crc(stack);
#endif

	return STACK_OK;
}


stack_status_t stack_chk(stack_t* stack)
{
	if(!stack) return STACK_ERR_ARGNULL;
	
#ifdef STACK_ENABLE_CANARIES
	if(check_canary(stack)) return STACK_ERR_CANARY;
#endif

#ifdef STACK_CRC
	if(check_crc(stack)) return STACK_ERR_CRC;
#endif

	return STACK_OK;
}
