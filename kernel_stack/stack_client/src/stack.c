#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#include "colors.h"
#include "stack.h"

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
		STATSTR(STACK_ERR_LIMIT)
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


stack_num stack_init(size_t elem_size, size_t initial_size)
{
	int fd = open("/dev/" STACK_NAME, O_RDWR);
	if(fd == -1)
	{
		printf("Can't open stack device!\nAre you sure driver is loaded?\n");
		abort();
		return -1;
	}
	printf("%d\n", fd);

	stack_ioctl_packet_t packet = { .elem_size = elem_size, .initial_size = initial_size};
 
	ioctl(fd, STACK_INIT, &packet);
	close(fd);

	return packet.st_num;
}


stack_status_t stack_destroy(stack_num st_num)
{
	int fd = open("/dev/" STACK_NAME, O_RDWR);
	if(fd == -1)
	{
		printf("Can't open stack device!\nAre you sure driver is loaded?\n");
		abort();
	}

	stack_ioctl_packet_t packet = { .st_num = st_num };
	ioctl(fd, STACK_DESTROY, &packet);
	close(fd);

	return packet.status;
}

stack_status_t stack_push(stack_num st_num, const void* input_data)
{
	int fd = open("/dev/" STACK_NAME, O_RDWR);
	if(fd == -1)
	{
		printf("Can't open stack device!\nAre you sure driver is loaded?\n");
		abort();
	}

	stack_ioctl_packet_t packet = { .st_num = st_num, .dataptr = (void*)input_data };
	ioctl(fd, STACK_PUSH, &packet);
	close(fd);

	return packet.status;
}

stack_status_t stack_pop(stack_num st_num, void* resulting_data)
{
	int fd = open("/dev/" STACK_NAME, O_RDWR);
	if(fd == -1)
	{
		printf("Can't open stack device!\nAre you sure driver is loaded?\n");
		abort();
	}

	stack_ioctl_packet_t packet = { .st_num = st_num, .dataptr = resulting_data };
	ioctl(fd, STACK_POP, &packet);
	close(fd);

	return packet.status;
}

stack_status_t stack_get(stack_num st_num, stack_data_t* stack_data)
{
	int fd = open("/dev/" STACK_NAME, O_RDWR);
	if(fd == -1)
	{
		printf("Can't open stack device!\nAre you sure driver is loaded?\n");
		abort();
	}

	stack_ioctl_packet_t packet = { .st_num = st_num, .dataptr = stack_data };
	ioctl(fd, STACK_GET, &packet);
	close(fd);

	return packet.status;
}

stack_status_t stack_get_buf(stack_num st_num, void* buf_ptr)
{
	int fd = open("/dev/" STACK_NAME, O_RDWR);
	if(fd == -1)
	{
		printf("Can't open stack device!\nAre you sure driver is loaded?\n");
		abort();
	}

	stack_ioctl_packet_t packet = { .st_num = st_num, .dataptr = buf_ptr };
	ioctl(fd, STACK_GET_BUF, &packet);
	close(fd);

	return packet.status;
}


stack_status_t stack_print(stack_data_t* stack, void* buf)
{
	if(!stack) return STACK_ERR_ARGNULL;

	printf("stack_data_t\n");

	size_t cap = stack->allocated_size / stack->elem_size;

	printf("\tcnt \t\t= " YELLOW "%lu\n" RESET, stack->cur_index);
	printf("\tcapacity \t= " YELLOW "%lu\n" RESET, cap);

	printf("\tdata" BLUE " [%p]\n" RESET, buf);

	for(size_t i = 0; i < cap; i++)
	{
		if(i < stack->cur_index)
			printf(RED "\t\t*\t[%lu]\t" YELLOW "= %d\n" RESET, i, ((int*)(buf))[i]);
		else
			printf(GREEN "\t\t\t[%lu]\t" YELLOW "= %d\n" RESET, i, ((int*)(buf))[i]);
	}

	return STACK_OK;
}

stack_status_t stack_get_and_print(stack_num num)
{
	stack_data_t st;
	stack_status_t res = stack_get(num, &st);
	if(res) return res;

	void* buf = calloc(1, st.allocated_size + 3 * sizeof(canary_t));
	res = stack_get_buf(num, buf);
	if(res) return res;
	stack_print(&st, buf + sizeof(canary_t));

	free(buf);

	return STACK_OK;
}
