#pragma once

#define STACK_NAME "megastack"
#define STACK_SHMEM_NAME "megastack_shmem"

#define STACK_ENABLE_CANARIES
#define STACK_CRC

#ifdef STACK_ENABLE_CANARIES
typedef unsigned long canary_t;
#endif

typedef int stack_num;

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
	STACK_ERR_LIMIT		= (1 << 8),
} stack_status_t;

typedef enum STACK_FOPS {
	STACK_INIT	= 0xbadbabe1,
	STACK_PUSH	= 0xbadbabe2,
	STACK_POP	= 0xbadbabe3,
	STACK_GET	= 0xbadbabe4,
	STACK_WRITE_BUF	= 0xbadbabe7,
	STACK_DESTROY	= 0xbadbabe5,
	STACK_GET_BUF	= 0xbadbabe6,
} stack_fops_t;

typedef struct stack_data {
#ifdef STACK_ENABLE_CANARIES
	canary_t canary1;
#endif

#ifdef STACK_CRC
	unsigned long crc;
#endif

	void*		buf;
	unsigned long	cur_index;
	unsigned long	allocated_size;
	unsigned long	elem_size;
	unsigned long	last_allocation_index;

#ifdef STACK_ENABLE_CANARIES
	canary_t canary2;
#endif

} stack_data_t;

typedef struct stack_ioctl_packet {
	stack_num st_num;
	void* dataptr;
	unsigned int elem_size;
	unsigned int initial_size;
	stack_status_t status;
} stack_ioctl_packet_t;

#define STACK_CHK_RET(s)	stack_status_t stack_chk_res = stack_chk(s);	\
				if(stack_chk_res)				\
					return stack_chk_res;
