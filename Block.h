#pragma once
#ifndef BLOCK_H_INCLUDED
#define BLOCK_H_INCLUDED

#include <stdlib.h>
#include <assert.h>

#include "List.h"
#include "Fragment.h"

typedef struct BlockData_tag {
    size_t len;                 // a length of a string that a block covers
    ListFragment* fragments;    // pointer to fragments of a string
} BlockData_t;

// template for list of blocks
LIST_TEMPLATE(Block, BlockData_t)

#endif // BLOCK_H_INCLUDED
