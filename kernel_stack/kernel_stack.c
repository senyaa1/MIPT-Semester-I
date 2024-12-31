#include <linux/device.h>
#include <linux/fs.h> 
#include <linux/module.h>
#include <linux/seq_file.h>
#include <linux/slab.h>
#include <linux/miscdevice.h>
#include <linux/random.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/ioctl.h>
#include <linux/mm.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/cdev.h>

#include "kernel_stack.h"

MODULE_AUTHOR("your mom");
MODULE_LICENSE("GPLv10");

static unsigned int crc_table[256];
static bool is_table_initialized = false;

static void init_crc32_table(void) 
{
	unsigned int polynomial = 0xEDB88320;
	for (unsigned int i = 0; i < 256; i++) 
	{
		unsigned int crc = i;
		for (unsigned char j = 0; j < 8; j++) 
		{
			if (crc & 1)
				crc = (crc >> 1) ^ polynomial;
			else
				crc = crc >> 1;
		}
	        crc_table[i] = crc;
	}

	is_table_initialized = true;
}

static unsigned int crc32(const char *data, unsigned long length) 
{
	if(!is_table_initialized)
		init_crc32_table();
	
	unsigned int crc = 0xFFFFFFFF;  
	for (unsigned long i = 0; i < length; i++) 
	{
		unsigned char index = (crc ^ data[i]) & 0xFF;
		crc = (crc >> 8) ^ crc_table[index];
	}
	return crc ^ 0xFFFFFFFF;  
}


#define MAX_STACKS 10000
static const unsigned long MIN_STACK_SIZE = 16;
static stack_data_t stacks[MAX_STACKS] = { 0 };
static unsigned int stacks_created = 0;

#ifdef STACK_ENABLE_CANARIES
static unsigned long canaryval = 0x0;

static void init_canary(void* canaryptr, unsigned long sz)
{
	get_random_bytes(canaryptr, sz);
}

static stack_status_t check_canary(stack_data_t* st)
{
	if(!st->buf) return STACK_ERR_UNINITIALIZED;
	if(canaryval == 0x0) init_canary(&canaryval, sizeof(canaryval));

	if(st->canary1 != canaryval || st->canary2 != canaryval)
		return STACK_ERR_CANARY;

	if(*(canary_t*)((char*)st->buf) != canaryval || *((canary_t*)((char*)st->buf + st->allocated_size + sizeof(canary_t))) != canaryval)
		return STACK_ERR_CANARY;

	return STACK_OK;
}

static void set_canary(stack_data_t* st)
{
	if(canaryval == 0x0) init_canary(&canaryval, sizeof(canaryval));

	st->canary1 = canaryval;
	st->canary2 = canaryval;

	*((canary_t*)((char*)st->buf)) = canaryval;
	*((canary_t*)((char*)st->buf + st->allocated_size + sizeof(canary_t))) = canaryval;
}
#endif


static void* get_buf_ptr(stack_data_t* st)
{
#ifdef STACK_ENABLE_CANARIES
	return (char*)st->buf + sizeof(canary_t);
#else
	return st->buf;
#endif
}

#ifdef STACK_CRC
static uint32_t stack_get_crc(stack_data_t* st)
{
	return crc32((char*)st->buf + sizeof(canary_t) * 2, st->allocated_size);
}

static void stack_recalc_crc(stack_data_t* st)
{
	st->crc = stack_get_crc(st);
}

static stack_status_t check_crc(stack_data_t* st)
{	
	if(st->crc != stack_get_crc(st)) return STACK_ERR_CRC;

	return STACK_OK;
}
#endif

static stack_status_t stack_chk(stack_data_t* st)
{
	if(!st) return STACK_ERR_ARGNULL;
	
#ifdef STACK_ENABLE_CANARIES
	if(check_canary(st)) return STACK_ERR_CANARY;
#endif

#ifdef STACK_CRC
	if(check_crc(st)) return STACK_ERR_CRC;
#endif

	return STACK_OK;
}

static stack_status_t increase_alloc(stack_data_t *st)
{
	STACK_CHK_RET(st)

	st->allocated_size *= 2;

#ifdef STACK_ENABLE_CANARIES
	st->buf = krealloc(st->buf, st->allocated_size + sizeof(canary_t) * 2, GFP_KERNEL);
	set_canary(st);
#else
	st->buf = krealloc(st->buf, st->allocated_size, GFP_KERNEL);
#endif
	if(!st->buf) return STACK_ERR_ALLOC;

	st->last_allocation_index = st->allocated_size / st->elem_size;

#ifdef STACK_CRC
	stack_recalc_crc(st);
#endif

	return STACK_OK;
}

static stack_status_t decrease_alloc(stack_data_t *st)
{
	STACK_CHK_RET(st)

	st->allocated_size /= 2;

#ifdef STACK_ENABLE_CANARIES
	st->buf = krealloc(st->buf, st->allocated_size + sizeof(canary_t) * 2, GFP_KERNEL);
	set_canary(st);
#else
	st->buf = krealloc(st->buf, st->allocated_size, GFP_KERNEL);
#endif
	if(!st->buf) return STACK_ERR_ALLOC;

	st->last_allocation_index = st->allocated_size / st->elem_size;

#ifdef STACK_CRC
	stack_recalc_crc(st);
#endif

	return STACK_OK;
}

inline static stack_status_t maybe_decrease_alloc(stack_data_t *st)
{
	if (st->cur_index <= (st->last_allocation_index / 2) - 1 && st->allocated_size / st->elem_size / 2 - 1 > MIN_STACK_SIZE)
		return decrease_alloc(st);

	return STACK_OK;
}


static stack_status_t stack_ctor(stack_data_t* st, unsigned long elem_size, unsigned long initial_size)
{
	if(!st) return STACK_ERR_ARGNULL;
	if(st->buf) return STACK_ERR_INITIALIZED;

	st->elem_size = elem_size;
	
	unsigned long to_allocate = ((initial_size < MIN_STACK_SIZE) ? MIN_STACK_SIZE : initial_size) * st->elem_size;
	st->allocated_size = to_allocate;

#ifdef STACK_ENABLE_CANARIES
	st->buf = kcalloc(1, to_allocate + (sizeof(canary_t) * 2), GFP_KERNEL);
	set_canary(st);
#else
	st->buf = kcalloc(1, to_allocate, GFP_KERNEL);
#endif

	if(!get_buf_ptr(st)) return STACK_ERR_ALLOC;

#ifdef STACK_CRC
	stack_recalc_crc(st);
#endif

	return STACK_OK;
}

static stack_status_t stack_dtor(stack_data_t* st)
{
	STACK_CHK_RET(st)

	kfree(st->buf);
	memset(st, 0, sizeof(stack_data_t));

	return STACK_OK;
}

static stack_status_t stack_push(stack_data_t* st, const void* data)	
{
	STACK_CHK_RET(st)

	if((st->cur_index * st->elem_size) >= st->allocated_size)
		if(increase_alloc(st)) return STACK_ERR_ALLOC;

	memcpy(((char*)get_buf_ptr(st) + (st->cur_index++) * st->elem_size), data, st->elem_size);

#ifdef STACK_CRC
	stack_recalc_crc(st);
#endif

	return STACK_OK;
}

static stack_status_t stack_pop(stack_data_t* st, void* resulting_data)	
{
	STACK_CHK_RET(st)

	if(st->cur_index <= 0) return STACK_ERR_EMPTY;
	if(maybe_decrease_alloc(st)) return STACK_ERR_ALLOC;

	memcpy((char*)resulting_data, ((char*)get_buf_ptr(st) + (--st->cur_index) * st->elem_size), st->elem_size);

#ifdef STACK_CRC
	stack_recalc_crc(st);
#endif

	return STACK_OK;
}


static noinline long handler(struct file *file, unsigned int cmd, unsigned long arg) 
{
	stack_ioctl_packet_t packet = { 0 };

	if(copy_from_user(&packet, (void*)arg, sizeof(packet)))
	{
		return EFAULT;
	}

	switch(cmd)
	{
		case STACK_INIT:
			pr_info("stack init\n");
			if(stacks_created >= MAX_STACKS)
			{
				packet.status = STACK_ERR_LIMIT;
				break;
			}
			packet.status = stack_ctor(&stacks[stacks_created], packet.elem_size, packet.initial_size);
			packet.st_num = stacks_created++;
			break;

		case STACK_PUSH:
			pr_info("stack push\n");
			if(packet.st_num < 0 || packet.st_num >= stacks_created)
			{
				packet.status = STACK_ERR_UNINITIALIZED;
				break;
			}

			void* kernel_pushptr = kzalloc(stacks[packet.st_num].elem_size, GFP_KERNEL);

			if(copy_from_user(kernel_pushptr, packet.dataptr, stacks[packet.st_num].elem_size))
			{
				packet.status = STACK_ERR_ARGNULL;
				kfree(kernel_pushptr);
				break;
			}

			packet.status = stack_push(&stacks[packet.st_num], kernel_pushptr);
			kfree(kernel_pushptr);
			break;

		case STACK_POP:
			pr_info("stack pop\n");
			if(packet.st_num < 0 || packet.st_num >= stacks_created)
			{
				packet.status = STACK_ERR_UNINITIALIZED;
				break;
			}

			void* kernel_popptr = kzalloc(stacks[packet.st_num].elem_size, GFP_KERNEL);
			packet.status = stack_pop(&stacks[packet.st_num], kernel_popptr);

			if(copy_to_user(packet.dataptr, kernel_popptr, stacks[packet.st_num].elem_size))
			{
				packet.status = STACK_ERR_ARGNULL;
				kfree(kernel_popptr);
				break;
			}

			kfree(kernel_popptr);
			break;

		case STACK_GET:
			pr_info("stack get\n");
			if(packet.st_num < 0 || packet.st_num >= stacks_created)
			{
				packet.status = STACK_ERR_UNINITIALIZED;
				break;
			}
			if(copy_to_user((void*)packet.dataptr, &stacks[packet.st_num], sizeof(stack_data_t)))
			{
				packet.status = STACK_ERR_ARGNULL;
				break;
			}

			packet.status = STACK_OK;
			break;

		case STACK_GET_BUF:
			pr_info("stack get buf\n");
			if(packet.st_num < 0 || packet.st_num >= stacks_created)
			{
				packet.status = STACK_ERR_UNINITIALIZED;
				break;
			}

			if(copy_to_user((void*)packet.dataptr, stacks[packet.st_num].buf, stacks[packet.st_num].allocated_size))
			{
				packet.status = STACK_ERR_ARGNULL;
				break;
			}

			packet.status = STACK_OK;
			break;
		case STACK_WRITE_BUF:
			if(packet.st_num < 0 || packet.st_num >= stacks_created)
			{
				packet.status = STACK_ERR_UNINITIALIZED;
				break;
			}

			if(copy_from_user(stacks[packet.st_num].buf, (void*)packet.dataptr, stacks[packet.st_num].allocated_size))
			{
				packet.status = STACK_ERR_ARGNULL;
				break;
			}

			packet.status = stack_chk(&stacks[packet.st_num]);
			break;

		case STACK_DESTROY:
			pr_info("stack destroy\n");
			if(packet.st_num < 0 || packet.st_num >= stacks_created)
			{
				packet.status = STACK_ERR_UNINITIALIZED;
				break;
			}
			packet.status = stack_dtor(&stacks[packet.st_num]);
			break;
	}

	if(copy_to_user((void*)arg, &packet, sizeof(packet)))
	{
		return EFAULT;
	}

	return 0;
}

static struct file_operations fops = {.unlocked_ioctl = handler};

static struct miscdevice stack_dev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = STACK_NAME,
	.fops = &fops,
};

static int __init based_init(void) 
{
	pr_info("registering dev\n");
	if(misc_register(&stack_dev))
	{
		pr_err("failed registering the device!\n");
		return -1;
	}
	pr_info("registered dev\n");

	pr_info("ready to handle stack for you, mate\n");
	return 0;
}


static void __exit based_exit(void) 
{
	misc_deregister(&stack_dev);

	pr_info(KERN_INFO "no more stack :(  \n");
}

module_init(based_init);
module_exit(based_exit);
