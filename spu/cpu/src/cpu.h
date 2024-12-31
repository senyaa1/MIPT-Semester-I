#pragma once

#include "arch.h"
#include "stack.h"
#include "io.h"


typedef struct idt
{
	uint8_t iv_cnt;
	reg_t* interrupt_vectors;
} idt_t;

typedef struct gdt_entry
{
	reg_t start;
	reg_t end;
	uint8_t prot_mode;
} gdt_entry_t;

typedef struct gdt
{
	uint8_t gdt_entry_cnt;
	gdt_entry_t* gdt_entries;
} gdt_t;

typedef struct cpu
{
	reg_t regs[16];
	stack_t data_stack;
	stack_t call_stack;
	uint8_t* mem;
	uint8_t priv;

	idt_t idt;
	gdt_t gdt;
} cpu_t;


int execute(cpu_t* cpu, framebuf_t* fb);
