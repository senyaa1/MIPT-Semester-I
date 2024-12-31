#pragma once

#include <stdint.h>
#include <stdlib.h>

/*
//	IO
//
//	check if allowed by gdt

- MATH	6
float operations
sqrt

	sin
	cos
		using taylor polynomials
*/

typedef enum EXPRESSIONS : uint8_t
{
	DB		= 1,
	DW		= 2,
	DD		= 3,
	DQ		= 4,
	UNDEFINED	= 0
} expression_t;

typedef int32_t reg_t;
#define MAX_OPERAND_CNT 4

typedef enum INSTRUCTIONS : uint8_t
{
	PUSH	= 1,
	POP	= 2,
	MOV	= 3,
	ADD	= 4,
	SUB	= 5,
	MUL	= 6,
	DIV	= 7,
	SIN	= 8,
	COS	= 9,
	DUMP	= 10,
	DRAW	= 11,
	IN	= 12,
	OUT	= 13,
	HLT	= 14,
	SLEEP	= 15,
	JMP	= 16,
	JZ	= 17,
	JNZ	= 18,
	JG 	= 19,
	JGE	= 20,
	JL 	= 21,
	JLE	= 22,
	CALL	= 23,
	RET	= 24,
	AND	= 25,
	OR	= 26,
	XOR	= 27,
	XCHG	= 28,
	CMP	= 29,
	INT	= 30,
	PRINT	= 31,
	LIDT	= 32,
	LGDT	= 33,
	IRET	= 34,
	SQRT	= 35,
	FADD	= 36,
	FSUB	= 37,
	FMUL	= 38,
	FDIV	= 39,
	FCMP	= 40,
	FIN	= 41,
	FOUT	= 42,
	MOD	= 43,
	INST_INVALID	= 0
} instruction_t;

static const float EPS = 0.001;

typedef enum REGISTERS : uint8_t
{
	AX		= 1,
	BX		= 2,
	CX		= 3,
	DX		= 4,
	EX		= 5,
	IP		= 6,
	FLAGS		= 7,
	IDTR		= 8,
	GDTR		= 9,
	REG_INVALID	= 0
} reg_name_t;

enum FLAGS : uint8_t
{
	ZF = 1 << 0,
	SF = 1 << 1,
	IF = 1 << 2,
};

#define OPERAND_PTR_BIT  (1 << 2)

typedef enum OPERANDS : uint8_t 
{
	OP_VALUE		= 1,
	OP_VALUEPTR		= 1	| OPERAND_PTR_BIT,
	OP_REG			= 2,
	OP_REGPTR		= 2	| OPERAND_PTR_BIT,
	OP_LABEL		= 32,
	OP_STR			= 33,
	OP_INVALID		= 0,
} operand_type_t;

typedef struct operand
{
	operand_type_t type;
	size_t length;
	uint64_t value;
	reg_t actual_value;

	char* operand_text;
	size_t operand_text_len;
} operand_t;

