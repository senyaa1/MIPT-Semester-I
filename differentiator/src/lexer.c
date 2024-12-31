#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "lexer.h"

lexer_t* lex(const char* expression) 
{
	lexer_t *lexer = (lexer_t*)calloc(1, sizeof(lexer_t));

	lexer->allocated = DEFAULT_TOKEN_ALLOC;
	lexer->tokens = (char**)calloc(lexer->allocated, sizeof(char*));

	char buffer[128] = { 0 };
	int idx = 0;
	const char *ptr = expression;

	while (*ptr) 
	{
		if(lexer->size >= lexer->allocated)
		{
			lexer->allocated *= 2;
			lexer->tokens = (char**)realloc(lexer->tokens, lexer->allocated *= sizeof(char*));
		}

		if (isspace(*ptr)) 
		{
			ptr++;
			continue;
		}

		if (isdigit(*ptr) || *ptr == '.') 
		{
			idx = 0;
			while (isdigit(*ptr) || *ptr == '.') 
			{
				buffer[idx++] = *ptr++;
			}
			buffer[idx] = '\0';
			lexer->tokens[lexer->size++] = strdup(buffer);
			continue;
		} 

		if (isalpha(*ptr)) 
		{
			idx = 0;
			while (isalpha(*ptr)) 
				buffer[idx++] = *ptr++;
			buffer[idx] = '\0';
			lexer->tokens[lexer->size++] = strdup(buffer);
			continue;
		} 

		buffer[0] = *ptr++;
		buffer[1] = '\0';
		lexer->tokens[lexer->size++] = strdup(buffer);
	}

	return lexer;
}

void free_lexer(lexer_t* lexer)
{
	for(int i = 0; i < lexer->size; i++)
		free(lexer->tokens[i]);

	free(lexer->tokens);
	free(lexer);
}

char *current_token(lexer_t *lexer) 
{
    return (lexer->pos < lexer->size) ? lexer->tokens[lexer->pos] : 0;
}

