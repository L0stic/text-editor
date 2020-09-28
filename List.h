#pragma once
#ifndef LIST_H_INCLUDED
#define LIST_H_INCLUDED

#include <stdlib.h>
#include <assert.h>

#define NODE(T, D) \
    typedef struct T##_tag {    \
        struct T##_tag* prev;   \
        struct T##_tag* next;   \
        D data;                 \
    } T;                        \
// END_NODE

#define LIST(T) \
    typedef struct List##T##_tag {  \
        size_t len;                 \
        T* nodes;                   \
        T* last;                    \
    } List##T;                      \
// END_LIST

#define CREATE_NODE(T, D)   T* Create##T(T* prev, const D* data)
#define DESTROY_NODE(T)     void Destroy##T(T** node)
#define CREATE_LIST(T)      List##T* CreateList##T()
#define DESTROY_LIST(T)     void DestroyList##T(List##T** list)
#define ADD_DATA(T, D)      int Add##T##Data(List##T* list, const D* data)
#define INSERT_NODE(T)      void Insert##T(List##T* list, T* node)

#define LIST_TEMPLATE(T, D) \
    NODE(T, D)          \
    LIST(T)             \
    \
    CREATE_NODE(T, D);  \
    DESTROY_NODE(T);    \
    \
    CREATE_LIST(T);     \
    DESTROY_LIST(T);    \
    \
    ADD_DATA(T, D);     \
    INSERT_NODE(T);     \
// END_LIST_TEMPLATE

#endif // LIST_H_INCLUDED
