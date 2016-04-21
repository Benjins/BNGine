#ifndef LEXER_H
#define LEXER_H

#pragma once

#include "vector.h"

typedef struct{
	const char* start;
	int length;
} Token;

int TokenEqual(Token a, Token b);

Vector<Token> LexString(const char* string);

#define COMPARE_STRING(ptr, reference) (memcmp(ptr, reference, (sizeof(reference) - 1)) == 0)
#define COMPARE_STRING_N(ptr, reference, len) (memcmp(ptr, reference, BNS_MIN(sizeof(reference) - 1, len)) == 0)

#define TOKEN_IS(token, str) (COMPARE_STRING_N(token.start, str, token.length) && token.length == (sizeof(str)-1))

#endif
