#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>

#include "assembler.h"
#include "io.h"
#include "fixups.h"
#include "parser.h"


static size_t assemble_instruction(const char* line, size_t line_len, uint8_t* buf, reg_t ip, size_t line_num, asm_info_t* asm_info)
{
	// printf("line: \"%.*s\"\n", line_len, line);
	char* t_line = (char*)line;
	size_t t_len = strim(&t_line, line_len);
	// printf("trimmed line: \"%.*s\"\n", t_len, t_line);

	if(t_len == 0) return 0;

	if(t_line[t_len - 1] == ':')	// label
	{
		// printf("encountered label named %.*s at 0x%x\n", t_len - 1, t_line, ip);
		add_to_labels(asm_info, t_line, t_len - 1, ip);
		return 0;
	}


	size_t instruction_word_len = t_len;
	// match first word (instruction)
	for(size_t i = 0; i < t_len; i++)
	{
		if(isspace(t_line[i]))
		{
			instruction_word_len = i;
			break;
		}
	}

	expression_t expr = UNDEFINED;

	#define EXPRCMP(expr_name)						\
		if(strncasecmp(t_line, #expr_name, instruction_word_len) == 0)	\
		{								\
			expr = expr_name;					\
		}								\

	EXPRCMP(DB)
	EXPRCMP(DD)
	EXPRCMP(DW)
	EXPRCMP(DQ)

	#undef EXPRCMP

	// printf("instruction: %.*s\n", instruction_word_len, t_line);

	instruction_t inst = INST_INVALID;
	
	#define INSTCMP(inst_name)												\
		if(strncasecmp(t_line, #inst_name, instruction_word_len) == 0 && instruction_word_len == strlen(#inst_name))	\
		{														\
			inst = inst_name;											\
		}														\

	INSTCMP(PUSH)
	INSTCMP(POP)
	INSTCMP(MOV)
	INSTCMP(ADD)
	INSTCMP(SUB)
	INSTCMP(MUL)
	INSTCMP(DIV)
	INSTCMP(DUMP)
	INSTCMP(DRAW)
	INSTCMP(IN)
	INSTCMP(OUT)
	INSTCMP(HLT)
	INSTCMP(SLEEP)
	INSTCMP(JMP)
	INSTCMP(JZ)
	INSTCMP(JNZ)
	INSTCMP(JG)
	INSTCMP(JGE)
	INSTCMP(JL)
	INSTCMP(JLE)
	INSTCMP(CALL)
	INSTCMP(RET)
	INSTCMP(AND)
	INSTCMP(OR)
	INSTCMP(XOR)
	INSTCMP(XCHG)
	INSTCMP(INT)
	INSTCMP(PRINT)
	INSTCMP(LIDT)
	INSTCMP(LGDT)
	INSTCMP(IRET)
	INSTCMP(CMP)

	INSTCMP(SIN)
	INSTCMP(COS)
	INSTCMP(SQRT)
	INSTCMP(FADD)
	INSTCMP(FSUB)
	INSTCMP(FMUL)
	INSTCMP(FDIV)
	INSTCMP(FCMP)
	INSTCMP(FIN)
	INSTCMP(FOUT)
	INSTCMP(MOD)


	#undef INSTCMP

	if (!inst && !expr)
	{
		print_error("encountered invalid instruction!");
		print_line(line, line_len, line_num);
		return -1;
	}


	// no operands

	// operands
	// printf("operands: %.*s\n", t_len - instruction_word_len, t_line + instruction_word_len);

	operand_t operands[MAX_OPERAND_CNT] = { 0 };

	size_t cur_operand_len = 0;
	size_t operand_cnt = 0;
	for(size_t i = instruction_word_len; i <= t_len; i++)
	{
		if(t_line[i] != ',' && i != t_len)
		{
			cur_operand_len++;
			continue;
		}
		// printf("cur operand len: %d\n", cur_operand_len);

		operand_t op = parse_operand(t_line + i - cur_operand_len, cur_operand_len);
		operands[operand_cnt++] = op;

		if(cur_operand_len == 0) operand_cnt = 0;
		cur_operand_len = 0;

		if(!op.type)
		{
			print_error("can't parse operand!");
			print_line(line, line_len, line_num);
			return -1;
		}

		// printf("type: %d,\tlength: %d\tvalue: 0x%x\t\n", op.type, op.length, op.value);
	}

	if(inst)
	{
		if(operand_cnt > MAX_OPERAND_CNT) 
		{
			print_error("too many operands!");
			print_line(line, line_len, line_num);
			return -1;
		}

		buf[ip] = inst | (operand_cnt << 6);
		// printf("operand_cnt: %d\n", operand_cnt);
		ip += sizeof(instruction_t);
		if(instruction_word_len == t_len)
			return sizeof(instruction_t);
	}

	// check if specified operand and instruction combination is allowed
	// make a list of expected operands


	// for (int i = 0; i < operand_cnt; i++)
	// 	printf("type: %d,\tlength: %d\tvalue: 0x%x\t\n", operands[i].type, operands[i].length, operands[i].value);

	if(expr)
	{
		reg_t saved_ip = ip;
		for(int i = 0; i < operand_cnt; i++)
		{
			if(operands[i].type == OP_STR && expr != DB)
			{
				print_error("unable to define non-single byte str!");
				print_line(line, line_len, line_num);
				return -1;
			}

			if(operands[i].type == OP_LABEL && expr != DD)
			{
				print_error("can't use label with incorrect size!");
				print_line(line, line_len, line_num);
			}

			if(operands[i].type == OP_LABEL) add_to_fixups(asm_info, &operands[i], ip);

			switch(expr)
			{
				case DB:
					if(operands[i].type == OP_STR)
					{
						for(int j = 0; j < operands[i].operand_text_len; j++)
							buf[ip++] = operands[i].operand_text[j];
						
					}
					else
					{
						memcpy(buf + ip, &operands[i].value, sizeof(uint8_t));
						ip += sizeof(uint8_t);
					}
					break;
				case DW:
					memcpy(buf + ip, &operands[i].value, sizeof(uint16_t));
					ip += sizeof(uint16_t);
					break;
				case DD:
					memcpy(buf + ip, &operands[i].value, sizeof(uint32_t));
					ip += sizeof(uint32_t);
					break;
				case DQ:
					memcpy(buf + ip, &operands[i].value, sizeof(uint64_t));
					ip += sizeof(uint64_t);
					break;
			}
		}

		return ip - saved_ip;
	}

	size_t operands_len_sum = 0;
	for(size_t i = 0; i < operand_cnt; i++)
	{
		buf[ip] = ((((operands[i].type & ~OPERAND_PTR_BIT) ==	OP_LABEL ? 
									OP_VALUE :
									operands[i].type) << 4) | (operands[i].length));

		if(operands[i].type & OPERAND_PTR_BIT)	buf[ip] |= (OPERAND_PTR_BIT << 4);

		ip += sizeof(uint8_t);
		operands_len_sum++;

		memcpy(buf + ip, &operands[i].value, operands[i].length);
		operands_len_sum += operands[i].length;

		if((operands[i].type & ~OPERAND_PTR_BIT) == OP_LABEL)
		{
			add_to_fixups(asm_info, &operands[i], ip);
		}

		ip += operands[i].length;
	}

	return sizeof(instruction_t) + operands_len_sum;
}



size_t assemble(asm_info_t* asm_info)
{
	if(asm_info->state != NOT_ASMED)
		return -1;

	asm_info->asm_buf_sz = INITIAL_ASM_BUF_SIZE;
	asm_info->asm_buf = (uint8_t*)calloc(asm_info->asm_buf_sz, sizeof(uint8_t));

	reg_t cur_ptr = 0;

	size_t cur_len = 0, comment_len = 0, line_num = 0;
	bool is_comment_started = false;
	for(size_t i = 0; i < asm_info->src_buf_sz; i++)
	{	
		if(asm_info->src_buf[i] != '\n')
		{
			if((asm_info->src_buf[i] == ';' || asm_info->src_buf[i] == '#') && !is_comment_started) 
				is_comment_started = true;	// starting from here every symbol in line is considered comment
			
			if (!is_comment_started)	cur_len++;			// Skip comments starting with ;
			else				comment_len++;

			continue;
		}

		line_num++;

		if(cur_ptr + 256 > asm_info->asm_buf_sz)
		{
			asm_info->asm_buf_sz *= 2;
			uint8_t* new_buf = realloc(asm_info->asm_buf, asm_info->asm_buf_sz);
			if(!new_buf)
			{
				print_error("memory allocation error!");
				return -1;
			}
			asm_info->asm_buf = new_buf;
		}

		size_t instruction_sz = assemble_instruction(asm_info->src_buf + i - cur_len - comment_len, cur_len, asm_info->asm_buf, cur_ptr, line_num, asm_info);

		if(instruction_sz < 0) 
			return -1;

		cur_ptr += instruction_sz;

		cur_len = 0;
		comment_len = 0;
		is_comment_started = false;
	}

	asm_info->asm_buf_sz = cur_ptr;
	asm_info->state = ASM_PASS1;
	return cur_ptr;
}

