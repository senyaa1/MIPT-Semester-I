#pragma once
#include <stdint.h>
#include <stdlib.h>

#include "assembler.h"

int read_value(char* valuestr, size_t n, uint64_t* val);
size_t strim(char** s, size_t line_len);
operand_t parse_operand(char* operand_ptr, size_t len);
