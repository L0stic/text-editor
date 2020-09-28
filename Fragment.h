#pragma once
#ifndef FRAGMENT_H_INCLUDED
#define FRAGMENT_H_INCLUDED

// #include <stdlib.h>
// #include <assert.h>

#include "List.h"

typedef struct FragmentData_tag {
    size_t len;
    size_t pos;
} FragmentData_t;

LIST_TEMPLATE(Fragment, FragmentData_t)

#endif // FRAGMENT_H_INCLUDED
