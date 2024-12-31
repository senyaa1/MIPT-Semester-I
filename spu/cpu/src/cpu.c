#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>

#include "arch.h"
#include "fs.h"
#include "stack.h"
#include "cpu.h"
#include "io.h"


#define BINARY_OP(INST, OP)			\
case INST:					\
	stack_pop(&cpu->data_stack, &a);	\
	stack_pop(&cpu->data_stack, &b);	\
	OP;					\
	stack_push(&cpu->data_stack, &res);	\
break;						\

#define FLOAT_BINARY_OP(INST, OP)		\
case INST:					\
	stack_pop(&cpu->data_stack, &f1);	\
	stack_pop(&cpu->data_stack, &f2);	\
	OP;					\
	stack_push(&cpu->data_stack, &fres);	\
break;						\

#define FLOAT_UNARY_OP(INST, OP)		\
case INST:					\
	stack_pop(&cpu->data_stack, &f1);	\
	OP;					\
	stack_push(&cpu->data_stack, &fres);	\
break;						\



static int load_idt(cpu_t* cpu)
{
	cpu->idt.iv_cnt = cpu->mem[cpu->regs[IDTR]];
	free(cpu->idt.interrupt_vectors);

	cpu->idt.interrupt_vectors = calloc(cpu->idt.iv_cnt, sizeof(reg_t));

	for(int i = 0; i < cpu->idt.iv_cnt; i++)
		memcpy(&cpu->idt.interrupt_vectors[i], &cpu->mem[cpu->regs[IDTR] + 1 + i * sizeof(reg_t)], sizeof(reg_t));

	return 0;
}

static int load_gdt(cpu_t* cpu)
{
	cpu->gdt.gdt_entry_cnt = cpu->mem[cpu->regs[GDTR]];
	free(cpu->gdt.gdt_entries);

	cpu->gdt.gdt_entries = calloc(cpu->gdt.gdt_entry_cnt, sizeof(gdt_entry_t));

	for(int i = 0; i < cpu->gdt.gdt_entry_cnt; i++)
		memcpy(&cpu->gdt.gdt_entries[i], &cpu->mem[cpu->regs[GDTR] + 1 + i * sizeof(gdt_entry_t)], sizeof(gdt_entry_t));

	return 0;
}

static void cpu_dump(cpu_t* cpu)
{
	#define ENUMDMP(val)	printf(BLUE #val YELLOW "\t= 0x%X \n" RESET, cpu->regs[val]);
	#define FLAGDMP(val)	printf(((cpu->regs[FLAGS] & val) ? (GREEN UNDERLINE #val RESET " ") : (RED #val RESET " ")));

	printf("CPU\n");
	printf(WHITE UNDERLINE "Registers\n" RESET);
	
	ENUMDMP(AX)
	ENUMDMP(BX)
	ENUMDMP(CX)
	ENUMDMP(DX)
	ENUMDMP(EX)
	ENUMDMP(IP)
	ENUMDMP(IDTR)
	ENUMDMP(GDTR)
	ENUMDMP(FLAGS)

	printf("\t  ");
	FLAGDMP(ZF)
	FLAGDMP(SF)
	FLAGDMP(IF)

	printf("\n\n");

	printf(WHITE UNDERLINE "Call Stack\n" RESET);
	stack_print(&cpu->call_stack);

	printf(WHITE UNDERLINE "Data Stack\n" RESET);
	stack_print(&cpu->data_stack);

	#undef ENUMDMP
	#undef FLAGDMP
}

static void set_flags(cpu_t* cpu, reg_t res)
{
	if(res == 0)	cpu->regs[FLAGS] |= ZF;
	else		cpu->regs[FLAGS] &= ~ZF;

	if(res < 0)	cpu->regs[FLAGS] |= SF;
	else		cpu->regs[FLAGS] &= ~SF;
}

static void set_flags_float(cpu_t* cpu, float res)
{
	if(fabs(res) < EPS)	cpu->regs[FLAGS] |= ZF;
	else			cpu->regs[FLAGS] &= ~ZF;

	if(res < 0)		cpu->regs[FLAGS] |= SF;
	else			cpu->regs[FLAGS] &= ~SF;
}

int execute(cpu_t* cpu, framebuf_t* fb)
{
	cpu->priv = 0;	// start in ring 0
	while(1)
	{
		uint8_t opcode = cpu->mem[cpu->regs[IP]];
		instruction_t inst = opcode & 0b00111111;
		operand_t operands[MAX_OPERAND_CNT] = { 0 };
		uint8_t operand_cnt = opcode >> 6;

		cpu->regs[IP] += sizeof(instruction_t);

		// printf("current insturction: %x\n", inst);
		// printf("operand cnt: %d\n", operand_cnt);
		// printf("ip: %d\n", cpu->regs[IP]);
		
		for(int i = 0; i < operand_cnt; i++)
		{
			operands[i].type = cpu->mem[cpu->regs[IP]] >> 4;
			operands[i].length = cpu->mem[cpu->regs[IP]] & 0b1111;
			cpu->regs[IP]++;

			reg_t data = 0;

			switch(operands[i].type)
			{
				case OP_VALUE:
					memcpy(&operands[i].value, cpu->mem + cpu->regs[IP], sizeof(reg_t));
					memcpy(&operands[i].actual_value, &operands[i].value, sizeof(reg_t));
					break;

				case OP_VALUEPTR:
					memcpy(&operands[i].value, cpu->mem + cpu->regs[IP], sizeof(reg_t));
					// operands[i].actual_value = cpu->mem[operands[i].value];
					memcpy(&operands[i].actual_value, &cpu->mem[operands[i].value], sizeof(reg_t));
					break;

				case OP_REG:
					operands[i].value = ((reg_name_t*)cpu->mem)[cpu->regs[IP]];
					// operands[i].actual_value = cpu->regs[operands[i].value];
					memcpy(&operands[i].actual_value, &cpu->regs[operands[i].value], sizeof(reg_t));
					break;

				case OP_REGPTR:
					operands[i].value = ((reg_name_t*)cpu->mem)[cpu->regs[IP]];
					// operands[i].actual_value = cpu->mem[cpu->regs[operands[i].value]];
					memcpy(&operands[i].actual_value, &cpu->mem[cpu->regs[operands[i].value]], sizeof(reg_t));
					break;

				default:
					break;
			}

			// check if allowed by gdt
			
			// printf("is ptr: %d\t", ((operands[i].type & OPERAND_PTR_BIT) != 0));
			// printf("type: %d,\tlength: %d\tvalue: 0x%x\t\n", operands[i].type, operands[i].length, operands[i].value);
			
			cpu->regs[IP] += operands[i].length;

		}

		
		reg_t a = 0, b = 0, res = 0;
		float f1 = 0, f2 = 0, fres = 0;
		switch(inst)
		{
			case PUSH:
				stack_push(&cpu->data_stack, &operands[0].actual_value);
				break;
			case POP:
				stack_pop(&cpu->data_stack, &cpu->regs[operands[0].value]);
				break;
			case MOV:
				if(operand_cnt != 2)
				{
					fprintf(stderr, "mov should have 2 operands!\n");
					return -1;
				}
				// printf(RED "val: %d\n" RESET, operands[1].actual_value);
				switch(operands[0].type)
				{
					case OP_VALUE:
						fprintf(stderr, RED "can't move i to a fucking value!!\n" RESET);
						return -1;
					case OP_REG:
						cpu->regs[operands[0].value] = operands[1].actual_value;
						break;
					case OP_REGPTR:
						cpu->mem[cpu->regs[operands[0].value]] = operands[1].actual_value;
						break;

					case OP_VALUEPTR:
						cpu->mem[operands[0].value] = operands[1].actual_value;
						break;
				}
				break;
			case XCHG:
				a = cpu->regs[operands[0].value];
				cpu->regs[operands[0].value] = cpu->regs[operands[1].value];
				cpu->regs[operands[1].value] = a;
				break;
			case SLEEP:
				sleep(operands[0].actual_value);
				break;

			BINARY_OP(ADD, res = a + b);
			BINARY_OP(SUB, res = b - a; set_flags(cpu, res);)
			BINARY_OP(MUL, res = a * b);
			BINARY_OP(DIV, res = b / a);

			BINARY_OP(AND, res = a & b);
			BINARY_OP(OR, res = a | b);
			BINARY_OP(XOR, res = a ^ b);
			BINARY_OP(MOD, res = b % a);

			case CALL:
				stack_push(&cpu->call_stack, &cpu->regs[IP]);
				cpu->regs[IP] = operands[0].actual_value;
				break;
			case RET:
				stack_pop(&cpu->call_stack, &cpu->regs[IP]);
				break;
			case DUMP:
				cpu_dump(cpu);
				break;
			case CMP:
				if(operand_cnt == 0)
				{
					stack_pop(&cpu->data_stack, &a);
					stack_pop(&cpu->data_stack, &b);
				}
				else
				{
					a = operands[1].actual_value;
					b = operands[0].actual_value;
				}

				set_flags(cpu, b - a);
				break;
			case DRAW:
				if(fb->addr == 0)
				{
					fprintf(stderr, "framebuffer is not initialized!\n");
					return -1;
				}

				uint8_t* dataptr = (uint8_t*)(cpu->mem + operands[0].actual_value);
				framebuffer_draw(fb, dataptr);

				usleep(40000);	// too fast
				break;
			case HLT:
				// printf(GREEN "halting!\n" RESET);
				return 0;
			case JMP:
				cpu->regs[IP] = operands[0].value;
				break;
			case JZ:
				if(cpu->regs[FLAGS] & 1)					cpu->regs[IP] = operands[0].value;
				break;
			case JNZ:
				if(!(cpu->regs[FLAGS] & 1))					cpu->regs[IP] = operands[0].value;
				break;
			case JG:
				if(!(cpu->regs[FLAGS] & (1 << 1)))				cpu->regs[IP] = operands[0].value;
				break;
			case JGE:
				if(!(cpu->regs[FLAGS] & (1 << 1)) || (cpu->regs[FLAGS] & 1))	cpu->regs[IP] = operands[0].value;
				break;
			case JL:
				if(cpu->regs[FLAGS] & (1 << 1))					cpu->regs[IP] = operands[0].value;
				break;
			case JLE:
				if(cpu->regs[FLAGS] & (1 << 1) || (cpu->regs[FLAGS] & 1))	cpu->regs[IP] = operands[0].value;
				break;

			case OUT:
				switch(operand_cnt)
				{
					case 0:
						stack_pop(&cpu->data_stack, &a);
						printf(GREEN "%d " RESET, a);
						break;
					case 1:
						printf(GREEN "%d " RESET, operands[0].actual_value);
						break;
				}
				break;
			case IN:
				printf(BLUE "input: ");
				scanf("%d", &a);
				printf(RESET);

				switch(operand_cnt)
				{
					case 0:
						stack_push(&cpu->data_stack, &a);
						break;
					case 1:
						cpu->regs[operands[0].value] = a;
						break;
				}
				break;
			case PRINT:
				switch(operand_cnt)
				{
					case 0:
						stack_pop(&cpu->data_stack, &a);
						printf(GREEN "%d" RESET, a);
						break;
					case 1:
						printf(GREEN "%s" RESET, cpu->mem + operands[0].actual_value);
						break;
				}
				break;
			case LIDT:
				if(cpu->priv != 0)
				{
					fprintf(stderr, RED "No privs to load IDT!\n" RESET);
					return -1;
				}

				cpu->regs[IDTR] = operands[0].value;
				if(load_idt(cpu))
				{
					fprintf(stderr, RED "Unable to load IDT!\n" RESET);
					return -1;
				}

				printf(BLUE "loaded interrupt descriptor table at %d: sz: %d\n" RESET, cpu->regs[IDTR], cpu->idt.iv_cnt);
				break;
			case LGDT:
				if(cpu->priv != 0)
				{
					fprintf(stderr, RED "No privs to load GDT!\n" RESET);
					return -1;
				}

				cpu->regs[GDTR] = operands[0].value;

				if(load_gdt(cpu))
				{
					fprintf(stderr, RED "Unable to load GDT!\n" RESET);
					return -1;
				}

				printf(BLUE "loaded global descriptor table, entry cnt: %d\n" RESET, cpu->gdt.gdt_entry_cnt);
				break;
			case IRET:
				if(cpu->priv != 0)
				{
					fprintf(stderr, RED "No privs to IRET!\n" RESET);
					return -1;
				}
				cpu->priv = 1;

				switch(operand_cnt)
				{
					case 1:
						cpu->regs[IP] = operands[0].actual_value;
						break;
					default:
						stack_pop(&cpu->call_stack, &cpu->regs[IP]);
						break;
				}

				break;
			case INT:
				stack_push(&cpu->call_stack, &cpu->regs[IP]);
				printf("INTERRUPTING!\n");
				if(operands[0].actual_value - 1 > cpu->idt.iv_cnt)
				{
					fprintf(stderr, RED "Interrupt handler not found!\n" RESET);
					return -1;
				}
				cpu->regs[IP] = cpu->idt.interrupt_vectors[operands[0].actual_value - 1];
				cpu->priv = 0;
				break;

			case FCMP:
				stack_pop(&cpu->data_stack, &f1);
				stack_pop(&cpu->data_stack, &f2);
				set_flags_float(cpu, f2 - f1);
				break;

			FLOAT_BINARY_OP(FADD,	fres = f1 + f2);
			FLOAT_BINARY_OP(FSUB,	fres = f2 - f1; set_flags_float(cpu, fres))
			FLOAT_BINARY_OP(FMUL,	fres = f1 * f2);
			FLOAT_BINARY_OP(FDIV,	fres = f2 / f1);
			FLOAT_UNARY_OP(SQRT,	fres = sqrtf(f1))
			FLOAT_UNARY_OP(SIN,	fres = sinf(f1))
			FLOAT_UNARY_OP(COS,	fres = cosf(f1))

			case FIN:
				printf(BLUE "input: ");
				scanf("%f", &f1);
				printf(RESET);

				switch(operand_cnt)
				{
					case 0:
						stack_push(&cpu->data_stack, &f1);
						break;
					case 1:
						memcpy(&cpu->regs[operands[0].value], &f1, sizeof(float));
						// cpu->regs[operands[0].value] = f1;
						break;
				}
				break;
				// stack_push(&cpu->data_stack, &f1);
				break;
			case FOUT:
				switch(operand_cnt)
				{
					case 0:
						stack_pop(&cpu->data_stack, &f1);
						break;
					case 1:
						memcpy(&f1, &cpu->regs[operands[0].value],sizeof(float));
						// cpu->regs[operands[0].value] = f1;
						break;
				}
				// stack_pop(&cpu->data_stack, &f1);
				printf(GREEN "%f " RESET, f1);
				break;

			default:
				fprintf(stderr, RED "Encountered INVALID instruction!: %d (%x) \n" RESET, inst, inst);
				return -1;
		}
	}
	return 0;
}
