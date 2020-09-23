#pragma once
#ifndef ERROR_H_INCLUDED
#define ERROR_H_INCLUDED

#include <stdio.h>

typedef enum {
    ERR_SUCCESS = 0,
    ERR_ARGC,
    ERR_NULL_PTR,
    ERR_OPEN_FILE,
    ERR_EOF,
    ERR_READ,
    ERR_NOMEM,
    ERR_PARAM,
    ERR_UNKNOWN
} ErrorType;

void PrintError(FILE* output, ErrorType errorType, char const* filename, int line);

#endif // ERROR_H_INCLUDED
