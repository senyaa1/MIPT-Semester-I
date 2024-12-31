#pragma once

#include <stdlib.h>

static const size_t DEFAULT_TOKEN_ALLOC = 128;

typedef struct lexer
{
	char **tokens;
	size_t size;
	size_t pos;
	size_t allocated;
} lexer_t;


lexer_t* lex(const char* expression);
char *current_token(lexer_t* lexer);
void free_lexer(lexer_t* lexer);
