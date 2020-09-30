#include "Error.h"

static char* errorMessages[ERR_UNKNOWN + 1] = {
    "success",
    "command line argument not specified (name of processed file)",
    "NULL pointer passed",
    "error opening file",
    "unexpected end of file while reading",
    "error reading file",
    "not enough memory",
    "parameter is not defined",
    "unknown error"
};

void PrintError(FILE* output, ErrorType errorType, const char* filename, int line) {
    if (!output) { output = stderr; }
    if (!filename) {
        fprintf(output, "ERROR: %s\n", errorMessages[errorType]);
    } else {
        fprintf(output, "ERROR: %s\nFILE: %s\nLINE: %i\n", errorMessages[errorType], filename, line);
    }
}
