#pragma once
#ifndef STRING_H_INCLUDED
#define STRING_H_INCLUDED

#define BASE_STRING_SIZE 50
#define DIV_WITH_ROUND_UP(op1, op2) ((op1) / (op2) + (((op1) % (op2)) ? 1 : 0))

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "Error.h"

typedef struct String_tag {
    size_t size;    // reserved size
    size_t len;     // length of string
    char* data;     // pointer to start of string
} String;

/**
 * Creates String object.
 * IN:
 * @param src - pointer to string
 * 
 * OUT:
 * @return pStr - pointer to a String object 
 */
String* CreateString(const char* src);

/**
 * Destroys String object.
 * IN:
 * @param ppStr - pointer to pointer to String object
 * 
 * OUT:
 * *ppStr - filled with NULL value
 */
void DestroyString(String** ppStr);

/**
 * Reserves the size to store the string.
 * IN:
 * @param str - pointer to a String object
 * @param size - a size to be reserved
 * 
 * OUT:
 * @return err - error value
 */
int ReserveSize(String* str, size_t size);

/**
 * Adds string to a String object.
 * IN:
 * @param str - pointer to a String object
 * @param src - pointer to string that should be added
 * 
 * OUT:
 * @return err - error value
 */
int AddString(String* str, const char* src);

/**
 * Adds char to a String object.
 * IN:
 * @param str - pointer to a String object
 * @param c - char that should be added
 * 
 * OUT:
 * @return err - error value
 */
int AddChar(String* str, char c);

/**
 * Prints string.
 * IN:
 * @param output - pointer to a stream. If it's NULL, then the string is printed to the stdout
 * @param c - char that should be added
 * 
 * OUT:
 * @return len - length of string
 */
size_t PrintString(FILE* output, const String* str);

#endif // STRING_H_INCLUDED
