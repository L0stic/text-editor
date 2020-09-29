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

/**
 * Prints a description of a error to output stream.
 * 
 * IN:
 * @param output - pointer to a stream. If it's NULL, then the description is printed to the stderr
 * @param errorType - pointer to a file name
 * @param filename - pointer to a file name
 * @param line - pointer to a file name
 */
void PrintError(FILE* output, ErrorType errorType, const char* filename, int line);

#endif // ERROR_H_INCLUDED
