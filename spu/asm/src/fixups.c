#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "io.h"
#include "assembler.h"
#include "fixups.h"


void add_to_fixups(asm_info_t* asm_info, operand_t* op, reg_t location)
{
	if(asm_info->fixups_sz == 0)	
	{
		asm_info->fixups_sz = 10 * sizeof(fixup_t);
		asm_info->fixups = (fixup_t*)calloc(asm_info->fixups_sz, sizeof(fixup_t));
	}

	if(asm_info->fixups_sz < (asm_info->fixup_cnt + 2) * sizeof(fixup_t))
	{
		asm_info->fixups_sz *= 2;
		asm_info->fixups = realloc(asm_info->fixups, asm_info->fixups_sz);
	}

	asm_info->fixups[asm_info->fixup_cnt++] = (fixup_t){op->operand_text, op->operand_text_len, location};
}

void add_to_labels(asm_info_t* asm_info, char* name, size_t name_len, reg_t location)
{
	if(asm_info->labels_sz == 0)	
	{
		asm_info->labels_sz = 10 * sizeof(label_t);
		asm_info->labels = (label_t*)calloc(asm_info->labels_sz, sizeof(label_t));
	}

	if(asm_info->labels_sz < (asm_info->label_cnt + 2) * sizeof(label_t))
	{
		asm_info->labels_sz *= 2;
		asm_info->labels = realloc(asm_info->labels, asm_info->labels_sz);
	}

	asm_info->labels[asm_info->label_cnt++] = (label_t){name, name_len, location};
}

int fixup(asm_info_t* asm_info)
{
	if(asm_info->state != ASM_PASS1)
		return -1;

	for(int i = 0; i < asm_info->fixup_cnt; i++)
	{
		bool found = false;
		for(int j = 0; j < asm_info->label_cnt; j++)
		{
			#define min(a,b) ((a < b) ? a : b)
			size_t len1 = asm_info->labels[j].name_len, len2 = asm_info->fixups[i].name_len;
			if(len1 != len2 || strncmp(asm_info->fixups[i].name, asm_info->labels[j].name, len1) != 0)
				continue;
			#undef min

			memcpy(&asm_info->asm_buf[asm_info->fixups[i].location], &asm_info->labels[j].value, sizeof(reg_t));
			found = true;
			break;
		}

		if(!found)
		{
			char* error_msg = 0;

			asprintf(&error_msg, "label " WHITE UNDERLINE "%.*s" RESET BOLD " is used but not defined!", asm_info->fixups[i].name_len, asm_info->fixups[i].name);
			print_error(error_msg);

			free(error_msg);
			return -1;
		}
	}

	asm_info->state = ASMED;
	return 0;
}
