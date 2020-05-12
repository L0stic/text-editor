#ifndef BLOCK_H_INCLUDED
#define BLOCK_H_INCLUDED

#include <stdlib.h>
#include <assert.h>

#include "List.h"

typedef struct BlockData_tag {
    size_t len;
    size_t pos;
    char* start;
} BlockData_t;

LIST_TEMPLATE(Block, BlockData_t)

#endif // BLOCK_H_INCLUDED
