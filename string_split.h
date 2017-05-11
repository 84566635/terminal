#ifndef _STRING_A_H
#define _STRING_A_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

typedef struct{
	uint8_t argc;
	char** argv;
}string_a_Arg;

typedef string_a_Arg args;

void ArgDestroy(args* src);

void split(char* strSrc, const char* separator, args* dst);

#endif // STRING_A_H
