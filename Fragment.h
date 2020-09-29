#pragma once
#ifndef FRAGMENT_H_INCLUDED
#define FRAGMENT_H_INCLUDED

#include <stdlib.h>
#include <assert.h>

#include "List.h"

typedef struct FragmentData_tag {
    size_t len;     // a length of a string part
    size_t pos;     // start position of a string part
} FragmentData_t;

// template for list of fragments
LIST_TEMPLATE(Fragment, FragmentData_t)

#endif // FRAGMENT_H_INCLUDED
