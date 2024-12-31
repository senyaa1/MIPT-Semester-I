#pragma once
#include "arch.h"

static const size_t INITIAL_ASM_BUF_SIZE = 256;
static const size_t MAX_INSTRUCTION_SIZE = 256;

typedef struct label {
	char* name;
	size_t name_len;
	reg_t value;
} label_t;


typedef struct fixup {
	char* name;
	size_t name_len;
	reg_t location;
} fixup_t;

typedef enum ASMSTATE {
	NOT_ASMED = 0,
	ASM_PASS1,
	ASMED
} asm_state_t;

typedef struct asm_info
{
	asm_state_t state;

	label_t* labels;
	size_t label_cnt;
	size_t labels_sz;

	fixup_t* fixups;
	size_t fixup_cnt;
	size_t fixups_sz;

	char* src_buf;
	size_t src_buf_sz;

	uint8_t* asm_buf;
	size_t asm_buf_sz;

} asm_info_t;

size_t assemble(asm_info_t* asm_info);
