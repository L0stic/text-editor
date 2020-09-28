#pragma once
#ifndef BLOCK_H_INCLUDED
#define BLOCK_H_INCLUDED

#include <stdlib.h>
#include <assert.h>

#include "List.h"
#include "Fragment.h"

typedef struct BlockData_tag {
    size_t len;
    ListFragment* fragments;
} BlockData_t;

LIST_TEMPLATE(Block, BlockData_t)

#endif // BLOCK_H_INCLUDED
