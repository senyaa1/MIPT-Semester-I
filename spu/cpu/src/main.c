#include <stdio.h>

#include "arch.h"
#include "fs.h"
#include "stack.h"
#include "cpu.h"
#include "io.h"


int main(int argc, char** argv)
{
	if(argc < 2)
	{
		fprintf(stderr, "no binary provided!\n");
		return -1;
	}

	cpu_t cpu = { 0 };
	STACK_INIT(&cpu.data_stack, sizeof(reg_t), 16);
	STACK_INIT(&cpu.call_stack, sizeof(reg_t), 16);

	size_t sz = read_file(argv[1], (char**)&cpu.mem); 
	if(sz <= 0)
	{
		fprintf(stderr,"unable to read the binary!\n");
		return -1;
	}

	cpu.mem = realloc(cpu.mem, sz + 0xFF + FB_SIZE);
	
	// print_buf(cpu.mem, sz);

	framebuf_t fb = { 0 };
	framebuffer_initialize(&fb);

	int retcode = execute(&cpu, &fb);

	framebuffer_destroy(&fb);
	free(cpu.mem);
	free(cpu.idt.interrupt_vectors);
	stack_dtor(&cpu.call_stack);
	stack_dtor(&cpu.data_stack);

	return retcode;
}
