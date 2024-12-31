#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>

#include "io.h"
#include "assembler.h"
#include "parser.h"

size_t strim(char** s, size_t line_len)
{
	char* trimmed_ptr = *s;

	// left trim
	size_t i = 0;
	while(i < line_len)
	{
		if(!isspace(*trimmed_ptr))
			break;
		trimmed_ptr++;
	}

	size_t new_len = line_len - (trimmed_ptr - *s);

	// right trim
	while(new_len > 0)
	{
		if(!isspace(trimmed_ptr[new_len - 1]))
			break;
		new_len--;
	}

	*s = trimmed_ptr;
	return new_len;
}

int read_value(char* valuestr, size_t n, uint64_t* val)
{
	int res = 0;
	char* terminated_str = (char*)calloc(n + 1, sizeof(char));
	memcpy(terminated_str, valuestr, n);

	char *end = 0;
	errno = 0;

	// if (strchr(terminated_str, '.') != 0 && terminated_str[n - 1] == 'f')		// parse as float
	if (strchr(terminated_str, '.') != 0)
		*val = strtof(terminated_str, &end);
	else
		*val = strtoll(terminated_str, &end, 0);

	bool range_error = errno == ERANGE;

	if(range_error || (end - terminated_str) != n)
		goto exit;

	res = 1;
exit:
	free(terminated_str);
	return res;
}

operand_t parse_operand(char* operand_ptr, size_t len)
{
	size_t operand_len = strim(&operand_ptr, len);
	// printf("encountered operand (%d): \"%.*s\"\n", operand_len, operand_len, operand_ptr);

	operand_t op = { 
		.type = OP_INVALID,
		.length = 0, 
		.value = 0,
	};

	if (operand_ptr[0] == '[' && operand_ptr[operand_len - 1] == ']')	// recursively parse!
	{
		op = parse_operand(operand_ptr + 1, operand_len - 2);
		if(!op.type)
		{
			print_error("can't parse operand value!");
			op.type = OP_INVALID;
			return op;
		}
		// printf("parsed [ptr] operand as type %d\n", op.type);
		op.type |= OPERAND_PTR_BIT;

		return op;
	}

	if (operand_ptr[0] == '"' && operand_ptr[operand_len - 1] == '"')
	{
		op.length = operand_len - 2;
		op.type = OP_STR;
		op.operand_text = operand_ptr + 1;
		op.operand_text_len = operand_len - 2;

		return op;
	}
	
	#define REGCMP(reg_name)										\
		if(strlen(#reg_name) == operand_len && strncasecmp(operand_ptr, #reg_name, operand_len) == 0)	\
		{												\
			op.value = reg_name;									\
		}												\

	REGCMP(AX)
	REGCMP(BX)
	REGCMP(CX)
	REGCMP(DX)
	REGCMP(EX)
	REGCMP(IP)
	REGCMP(FLAGS)
	REGCMP(IDTR)
	REGCMP(GDTR)

	if(op.value != 0)
	{
		op.type = OP_REG;
		op.length = 1;
		return op;
	}


	uint64_t value = 0;
	if(read_value(operand_ptr, operand_len, &value))
	{
		op.type = OP_VALUE;
		op.length = sizeof(reg_t);
		op.value = value;
		return op;
	}

	for(int i = 0; i < operand_len; i++)		// no spaces in labels
	{
		if((ispunct(operand_ptr[i]) && operand_ptr[i] != '_') || isspace(operand_ptr[i]))
		{
			op.type = OP_INVALID;
			return op;
		}
	}

	// if can't be parsed as register, memory ptr or value, then consider it a label
	op.type = OP_LABEL;
	op.length = sizeof(reg_t);
	op.value = 0xDEADBEEF;
	op.operand_text = operand_ptr;
	op.operand_text_len = operand_len;
	
	return op;
	#undef REGCMP
}
