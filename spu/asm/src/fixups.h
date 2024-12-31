#pragma once
#include "assembler.h"

void add_to_fixups(asm_info_t* asm_info, operand_t* op, reg_t location);
void add_to_labels(asm_info_t* asm_info, char* name, size_t name_len, reg_t location);
int fixup(asm_info_t* asm_info);
